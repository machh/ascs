
#include <iostream>

//configuration
//#define ASCS_INPUT_QUEUE non_lock_queue
//file_server / file_client is a responsive system, before file_server send each message (except talking message,
//but file_server only receive talking message, don't send talking message proactively), the previous message has been
//sent to file_client, so sending buffer will always be empty, which means we will never operate sending buffer concurrently,
//so need no locks.
#if defined(_MSC_VER) && _MSC_VER <= 1800
#define ASCS_DEFAULT_PACKER packer2<shared_buffer<i_buffer>>
#else
#define ASCS_DEFAULT_PACKER packer2<>
#endif
//#define ASCS_RECV_BUFFER_TYPE std::vector<asio::mutable_buffer> //scatter-gather buffer, it's very useful under certain situations (for example, ring buffer).
//#define ASCS_SCATTERED_RECV_BUFFER //used by unpackers, not belongs to ascs
//note, these two macro are not requisite, i'm just showing how to use them.

//all other definitions are in the makefile, because we have two cpp files, defining them in more than one place is riskful (
// we may define them to different values between the two cpp files)
//configuration

#include "file_socket.h"

#define QUIT_COMMAND	"quit"
#define RESTART_COMMAND	"restart"
#define STATUS			"status"
#define STATISTIC		"statistic"
#define LIST_ALL_CLIENT	"list all client"

int main(int argc, const char* argv[])
{
	puts("this is a file transfer server.");
	printf("usage: %s [<port=%d> [ip=0.0.0.0]]\n", argv[0], ASCS_SERVER_PORT);
	if (argc >= 2 && (0 == strcmp(argv[1], "--help") || 0 == strcmp(argv[1], "-h")))
		return 0;
	else
		puts("type " QUIT_COMMAND " to end.");

	service_pump sp;
	server_base<file_socket> file_server_(sp);

	if (argc > 2)
		file_server_.set_server_addr(atoi(argv[1]), argv[2]);
	else if (argc > 1)
		file_server_.set_server_addr(atoi(argv[1]));

	sp.start_service();
	while(sp.is_running())
	{
		std::string str;
		std::getline(std::cin, str);
		if (str.empty())
			;
		else if (QUIT_COMMAND == str)
			sp.stop_service();
		else if (RESTART_COMMAND == str)
		{
			sp.stop_service();
			sp.start_service();
		}
		else if (STATISTIC == str)
		{
			printf("link #: " ASCS_SF ", invalid links: " ASCS_SF "\n\n", file_server_.size(), file_server_.invalid_object_size());
			puts(file_server_.get_statistic().to_string().data());
		}
		else if (STATUS == str)
			file_server_.list_all_status();
		else if (LIST_ALL_CLIENT == str)
			file_server_.list_all_object();
	}

	return 0;
}
