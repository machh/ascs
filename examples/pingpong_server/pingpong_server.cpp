
#include <iostream>

//configuration
#define ASCS_SERVER_PORT		9527
#define ASCS_REUSE_OBJECT //use objects pool
#define ASCS_DELAY_CLOSE		5 //define this to avoid hooks for async call (and slightly improve efficiency)
#define ASCS_MSG_BUFFER_SIZE	65536
#define ASCS_INPUT_QUEUE non_lock_queue
#define ASCS_INPUT_CONTAINER list
//if pingpong_client only send message in on_msg_handle(), which means a responsive system, a real pingpong test,
//then, before pingpong_server send each message, the previous message has been sent to pingpong_client,
//so sending buffer will always be empty, which means we will never operate sending buffer concurrently, so need no locks.
//
//if pingpong_client send message in on_msg_send(), then using non_lock_queue as input queue in pingpong_server will lead
//undefined behavior, please note.
#define ASCS_DEFAULT_UNPACKER stream_unpacker //non-protocol
#define ASCS_DECREASE_THREAD_AT_RUNTIME
//configuration

#include <ascs/ext/tcp.h>
using namespace ascs;
using namespace ascs::tcp;
using namespace ascs::ext::tcp;

#define QUIT_COMMAND	"quit"
#define LIST_ALL_CLIENT	"list_all_client"
#define STATISTIC		"statistic"
#define STATUS			"status"
#define INCREASE_THREAD	"increase_thread"
#define DECREASE_THREAD	"decrease_thread"

class echo_socket : public server_socket
{
public:
	echo_socket(i_server& server_) : server_socket(server_) {}

protected:
	//msg handling: send the original msg back(echo server)
	virtual bool on_msg_handle(out_msg_type& msg) {return direct_send_msg(std::move(msg));}
	//msg handling end
};

class echo_server : public server_base<echo_socket>
{
public:
	echo_server(service_pump& service_pump_) : server_base<echo_socket>(service_pump_) {}

protected:
	virtual bool on_accept(object_ctype& socket_ptr) {asio::ip::tcp::no_delay option(true); socket_ptr->lowest_layer().set_option(option); return true;}
};

int main(int argc, const char* argv[])
{
	printf("usage: %s [<service thread number=1> [<port=%d> [ip=0.0.0.0]]]\n", argv[0], ASCS_SERVER_PORT);
	if (argc >= 2 && (0 == strcmp(argv[1], "--help") || 0 == strcmp(argv[1], "-h")))
		return 0;
	else
		puts("type " QUIT_COMMAND " to end.");

	service_pump sp;
	echo_server echo_server_(sp);

	if (argc > 3)
		echo_server_.set_server_addr(atoi(argv[2]), argv[3]);
	else if (argc > 2)
		echo_server_.set_server_addr(atoi(argv[2]));

	auto thread_num = 1;
	if (argc > 1)
		thread_num = std::min(16, std::max(thread_num, atoi(argv[1])));

	sp.start_service(thread_num);
	while(sp.is_running())
	{
		std::string str;
		std::cin >> str;
		if (QUIT_COMMAND == str)
			sp.stop_service();
		else if (STATISTIC == str)
		{
			printf("link #: " ASCS_SF ", invalid links: " ASCS_SF "\n", echo_server_.size(), echo_server_.invalid_object_size());
			puts("");
			puts(echo_server_.get_statistic().to_string().data());
		}
		else if (STATUS == str)
			echo_server_.list_all_status();
		else if (LIST_ALL_CLIENT == str)
			echo_server_.list_all_object();
		else if (INCREASE_THREAD == str)
			sp.add_service_thread(1);
		else if (DECREASE_THREAD == str)
			sp.del_service_thread(1);
	}

	return 0;
}
