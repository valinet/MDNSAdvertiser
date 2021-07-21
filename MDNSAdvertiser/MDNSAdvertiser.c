#include <WinSock2.h>
#include <Ws2ipdef.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Shlwapi.lib")

#define APP_NAME "MDNSAdvertiser"
#define MDNS_ADDRESS "224.0.0.251"
#define MDNS_PORT 5353
#define WAIT_TIMEOUT 1000
#define MESSAGE_INIT_FAILED_BEGIN "The application failed to initialize properly "
#define MESSAGE_INIT_FAILED_END ".\r\nClick OK to terminate the application."

const uint8_t dns_r[] = {
	0x00, 0x00,           // id
	0x80, 0x00,           // flags
	0x00, 0x00,
	0x00, 0x01,           // ancount = 1
	0x00, 0x00,
	0x00, 0x00,
	7, 'v', 'a', 'l', 'i', 'n', 'e', 't',
	5, 'l', 'o', 'c', 'a', 'l',
	0,
	0x00, 0x01,           // type = A
	0x00, 0x01,           // class = IN
	0x00, 0x00, 0x00, 10, // TTL = 10 seconds
	0x00, 0x04
};

int WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
	WSADATA hWsa;
	SOCKET hSocket;
	struct sockaddr_in stServer;
	stServer.sin_family = AF_INET;
	stServer.sin_port = htons(MDNS_PORT);
	TCHAR buffer[BUFSIZ];
	uint8_t payload[sizeof(dns_r) + 4];
	char* szIp = PathGetArgsA(GetCommandLineA());
	struct in_addr localInterface;
	unsigned int res = 0;

	SetLastError(0);

	if (__argc != 2)
	{
		MessageBox(
			NULL,
			TEXT("Usage:\r\n")
			TEXT("   MDNSAdvertiser.exe <IP>\r\n")
			TEXT("where IP is the IP address of the interface in the LAN\r\n")
			TEXT("in which you want to multicast the information.\r\n")
			TEXT("Example: MDNSAdvertiser.exe 192.168.0.100\r\n"),
			TEXT(APP_NAME),
			MB_ICONASTERISK
		);
		goto exit0;
	}

	if (WSAStartup(
		MAKEWORD(2, 2), 
		&hWsa
	) != 0)
	{
#ifndef UNICODE
		sprintf_s(
#else
		swprintf_s(
#endif
			buffer,
			BUFSIZ,
			TEXT(MESSAGE_INIT_FAILED_BEGIN) 
			TEXT("(WSAStartup : % d)") 
			TEXT(MESSAGE_INIT_FAILED_END),
			GetLastError()
		);
		MessageBox(
			NULL,
			buffer,
			TEXT(APP_NAME),
			MB_ICONERROR
		);
		goto exit0;
	}

	if ((hSocket = socket(
		AF_INET, 
		SOCK_DGRAM, 
		IPPROTO_UDP
	)) == INVALID_SOCKET)
	{
#ifndef UNICODE
		sprintf_s(
#else
		swprintf_s(
#endif
			buffer,
			BUFSIZ,
			TEXT(MESSAGE_INIT_FAILED_BEGIN)
			TEXT("(socket : % d)")
			TEXT(MESSAGE_INIT_FAILED_END), 
			WSAGetLastError()
		);
		MessageBox(
			NULL,
			buffer,
			TEXT(APP_NAME),
			MB_ICONERROR
		);
		SetLastError(WSAGetLastError());
		goto exit1;
	}

	if (!InetPton(
		AF_INET,
		TEXT(MDNS_ADDRESS),
		&stServer.sin_addr.s_addr
	))
	{
#ifndef UNICODE
		sprintf_s(
#else
		swprintf_s(
#endif
			buffer,
			BUFSIZ,
			TEXT(MESSAGE_INIT_FAILED_BEGIN)
			TEXT("(InetPton : % d)")
			TEXT(MESSAGE_INIT_FAILED_END),
			WSAGetLastError()
		);
		MessageBox(
			NULL,
			buffer,
			TEXT(APP_NAME),
			MB_ICONERROR
		);
		SetLastError(WSAGetLastError());
		goto exit1;
	}

#ifndef UNICODE
	res = InetPton(
		AF_INET, 
		szIp, 
		&localInterface.s_addr
	);
#else
	size_t szArgv1 = strlen(szIp) + 1;
	wchar_t* w_argv1 = (wchar_t*)malloc(sizeof(wchar_t) * szArgv1);
	if (!w_argv1)
	{
#ifndef UNICODE
		sprintf_s(
#else
		swprintf_s(
#endif
			buffer,
			BUFSIZ,
			TEXT(MESSAGE_INIT_FAILED_BEGIN)
			TEXT("(malloc)")
			TEXT(MESSAGE_INIT_FAILED_END)
		);
		MessageBox(
			NULL,
			buffer,
			TEXT(APP_NAME),
			MB_ICONERROR
		);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		goto exit1;
	}
	size_t wszArgv1;
	if (res = mbstowcs_s(
		&wszArgv1,
		w_argv1,
		szArgv1,
		szIp,
		szArgv1 - 1
	))
	{
#ifndef UNICODE
		sprintf_s(
#else
		swprintf_s(
#endif
			buffer,
			BUFSIZ,
			TEXT(MESSAGE_INIT_FAILED_BEGIN)
			TEXT("(mbstowcs_s : %d)")
			TEXT(MESSAGE_INIT_FAILED_END),
			res
		);
		MessageBox(
			NULL,
			buffer,
			TEXT(APP_NAME),
			MB_ICONERROR
		);
		SetLastError(res);
		goto exit1;
	}
	res = InetPton(
		AF_INET, 
		w_argv1, 
		&localInterface.s_addr
	);
	free(w_argv1);
#endif
	if (!res)
	{
#ifndef UNICODE
		sprintf_s(
#else
		swprintf_s(
#endif
			buffer,
			BUFSIZ,
			TEXT("The application argument needs to be a valid IPv4 address")
			TEXT(MESSAGE_INIT_FAILED_END)
		);
		MessageBox(
			NULL,
			buffer,
			TEXT(APP_NAME),
			MB_ICONERROR
		);
		SetLastError(WSAGetLastError());
		goto exit1;
	}

	if (setsockopt(
		hSocket, 
		IPPROTO_IP, 
		IP_MULTICAST_IF, 
		(char*)&localInterface, 
		sizeof(localInterface)
	) < 0) {
#ifndef UNICODE
		sprintf_s(
#else
		swprintf_s(
#endif
			buffer,
			BUFSIZ,
			TEXT(MESSAGE_INIT_FAILED_BEGIN)
			TEXT("(setsockopt : % d)")
			TEXT(MESSAGE_INIT_FAILED_END),
			WSAGetLastError()
		);
		MessageBox(
			NULL,
			buffer,
			TEXT(APP_NAME),
			MB_ICONERROR
		);
		SetLastError(WSAGetLastError());
		goto exit1;
	}

	memcpy(
		payload,
		dns_r,
		sizeof(dns_r)
	);
	char* o = szIp;
	char *p = strchr(o, '.');
	*p = 0;
	payload[sizeof(dns_r) + 0] = atoi(o);
	o = p + 1;
	p = strchr(p + 1, '.');
	*p = 0;
	payload[sizeof(dns_r) + 1] = atoi(o);
	o = p + 1;
	p = strchr(p + 1, '.');
	*p = 0;
	payload[sizeof(dns_r) + 2] = atoi(o);
	o = p + 1;
	payload[sizeof(dns_r) + 3] = atoi(o);

	while (TRUE)
	{
		if (sendto(
			hSocket,
			payload,
			sizeof(payload),
			0,
			&stServer,
			sizeof(stServer)
		) < 0)
		{
#ifndef UNICODE
			sprintf_s(
#else
			swprintf_s(
#endif
				buffer,
				BUFSIZ,
				TEXT(MESSAGE_INIT_FAILED_BEGIN)
				TEXT("(sendto : % d)")
				TEXT(MESSAGE_INIT_FAILED_END),
				WSAGetLastError()
			);
			MessageBox(
				NULL,
				buffer,
				TEXT(APP_NAME),
				MB_ICONERROR
			);
			SetLastError(WSAGetLastError());
			goto exit1;
		}
		Sleep(WAIT_TIMEOUT);
	}

exit1:
	WSACleanup();
exit0:
	return GetLastError();
}