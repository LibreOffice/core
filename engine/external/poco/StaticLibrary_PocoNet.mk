# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,PocoNet))

$(eval $(call gb_StaticLibrary_use_unpacked,PocoNet,poco))

# keep the default std::vector ABI - the non-dbgutil online server links this
$(eval $(call gb_StaticLibrary_add_defs,PocoNet,\
	-U_GLIBCXX_DEBUG \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,PocoNet))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,PocoNet,cpp))

$(eval $(call gb_StaticLibrary_set_include,PocoNet,\
	-I$(gb_UnpackedTarball_workdir)/poco/include \
	$(if $(filter WNT,$(OS)),-I$(gb_UnpackedTarball_workdir)/poco/dependencies/wepoll/src) \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,PocoNet,\
	-DPOCO_STATIC \
	-DPOCO_NO_AUTOMATIC_LIBS \
	-UIOS \
))

# On Windows POCO emulates epoll with the bundled wepoll (SocketImpl/PollSet
# include "wepoll.h"); compile it in.  Other platforms use real epoll/poll.
ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_generated_cobjects,PocoNet,\
	UnpackedTarball/poco/dependencies/wepoll/src/wepoll \
))
endif

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoNet,\
	UnpackedTarball/poco/Net/src/AbstractHTTPRequestHandler \
	UnpackedTarball/poco/Net/src/DatagramSocket \
	UnpackedTarball/poco/Net/src/DatagramSocketImpl \
	UnpackedTarball/poco/Net/src/DialogSocket \
	UnpackedTarball/poco/Net/src/DNS \
	UnpackedTarball/poco/Net/src/EscapeHTMLStream \
	UnpackedTarball/poco/Net/src/FilePartSource \
	UnpackedTarball/poco/Net/src/FTPClientSession \
	UnpackedTarball/poco/Net/src/FTPStreamFactory \
	UnpackedTarball/poco/Net/src/HostEntry \
	UnpackedTarball/poco/Net/src/HTMLForm \
	UnpackedTarball/poco/Net/src/HTTPAuthenticationParams \
	UnpackedTarball/poco/Net/src/HTTPBasicCredentials \
	UnpackedTarball/poco/Net/src/HTTPBufferAllocator \
	UnpackedTarball/poco/Net/src/HTTPChunkedStream \
	UnpackedTarball/poco/Net/src/HTTPClientSession \
	UnpackedTarball/poco/Net/src/HTTPCookie \
	UnpackedTarball/poco/Net/src/HTTPCredentials \
	UnpackedTarball/poco/Net/src/HTTPDigestCredentials \
	UnpackedTarball/poco/Net/src/HTTPFixedLengthStream \
	UnpackedTarball/poco/Net/src/HTTPHeaderStream \
	UnpackedTarball/poco/Net/src/HTTPIOStream \
	UnpackedTarball/poco/Net/src/HTTPMessage \
	UnpackedTarball/poco/Net/src/HTTPNTLMCredentials \
	UnpackedTarball/poco/Net/src/HTTPReactorServer \
	UnpackedTarball/poco/Net/src/HTTPReactorServerSession \
	UnpackedTarball/poco/Net/src/HTTPRequest \
	UnpackedTarball/poco/Net/src/HTTPRequestHandler \
	UnpackedTarball/poco/Net/src/HTTPRequestHandlerFactory \
	UnpackedTarball/poco/Net/src/HTTPResponse \
	UnpackedTarball/poco/Net/src/HTTPServer \
	UnpackedTarball/poco/Net/src/HTTPServerConnection \
	UnpackedTarball/poco/Net/src/HTTPServerConnectionFactory \
	UnpackedTarball/poco/Net/src/HTTPServerParams \
	UnpackedTarball/poco/Net/src/HTTPServerRequest \
	UnpackedTarball/poco/Net/src/HTTPServerRequestImpl \
	UnpackedTarball/poco/Net/src/HTTPServerResponse \
	UnpackedTarball/poco/Net/src/HTTPServerResponseImpl \
	UnpackedTarball/poco/Net/src/HTTPServerSession \
	UnpackedTarball/poco/Net/src/HTTPSession \
	UnpackedTarball/poco/Net/src/HTTPSessionFactory \
	UnpackedTarball/poco/Net/src/HTTPSessionInstantiator \
	UnpackedTarball/poco/Net/src/HTTPStream \
	UnpackedTarball/poco/Net/src/HTTPStreamFactory \
	UnpackedTarball/poco/Net/src/ICMPClient \
	UnpackedTarball/poco/Net/src/ICMPEventArgs \
	UnpackedTarball/poco/Net/src/ICMPPacket \
	UnpackedTarball/poco/Net/src/ICMPPacketImpl \
	UnpackedTarball/poco/Net/src/ICMPSocket \
	UnpackedTarball/poco/Net/src/ICMPSocketImpl \
	UnpackedTarball/poco/Net/src/ICMPv4PacketImpl \
	UnpackedTarball/poco/Net/src/IPAddress \
	UnpackedTarball/poco/Net/src/IPAddressImpl \
	UnpackedTarball/poco/Net/src/MailMessage \
	UnpackedTarball/poco/Net/src/MailRecipient \
	UnpackedTarball/poco/Net/src/MailStream \
	UnpackedTarball/poco/Net/src/MediaType \
	UnpackedTarball/poco/Net/src/MessageHeader \
	UnpackedTarball/poco/Net/src/MulticastSocket \
	UnpackedTarball/poco/Net/src/MultipartReader \
	UnpackedTarball/poco/Net/src/MultipartWriter \
	UnpackedTarball/poco/Net/src/NameValueCollection \
	UnpackedTarball/poco/Net/src/Net \
	UnpackedTarball/poco/Net/src/NetException \
	UnpackedTarball/poco/Net/src/NetworkInterface \
	UnpackedTarball/poco/Net/src/NTLMCredentials \
	UnpackedTarball/poco/Net/src/NTPClient \
	UnpackedTarball/poco/Net/src/NTPEventArgs \
	UnpackedTarball/poco/Net/src/NTPPacket \
	UnpackedTarball/poco/Net/src/NullPartHandler \
	UnpackedTarball/poco/Net/src/OAuth10Credentials \
	UnpackedTarball/poco/Net/src/OAuth20Credentials \
	UnpackedTarball/poco/Net/src/PartHandler \
	UnpackedTarball/poco/Net/src/PartSource \
	UnpackedTarball/poco/Net/src/PartStore \
	UnpackedTarball/poco/Net/src/PollSet \
	UnpackedTarball/poco/Net/src/POP3ClientSession \
	UnpackedTarball/poco/Net/src/QuotedPrintableDecoder \
	UnpackedTarball/poco/Net/src/QuotedPrintableEncoder \
	UnpackedTarball/poco/Net/src/RawSocket \
	UnpackedTarball/poco/Net/src/RawSocketImpl \
	UnpackedTarball/poco/Net/src/RemoteSyslogChannel \
	UnpackedTarball/poco/Net/src/RemoteSyslogListener \
	UnpackedTarball/poco/Net/src/ServerSocket \
	UnpackedTarball/poco/Net/src/ServerSocketImpl \
	UnpackedTarball/poco/Net/src/SMTPChannel \
	UnpackedTarball/poco/Net/src/SMTPClientSession \
	UnpackedTarball/poco/Net/src/Socket \
	UnpackedTarball/poco/Net/src/SocketAddress \
	UnpackedTarball/poco/Net/src/SocketAddressImpl \
	UnpackedTarball/poco/Net/src/SocketImpl \
	UnpackedTarball/poco/Net/src/SocketNotification \
	UnpackedTarball/poco/Net/src/SocketNotifier \
	UnpackedTarball/poco/Net/src/SocketProactor \
	UnpackedTarball/poco/Net/src/SocketReactor \
	UnpackedTarball/poco/Net/src/SocketStream \
	UnpackedTarball/poco/Net/src/SSPINTLMCredentials \
	UnpackedTarball/poco/Net/src/StreamSocket \
	UnpackedTarball/poco/Net/src/StreamSocketImpl \
	UnpackedTarball/poco/Net/src/StringPartSource \
	UnpackedTarball/poco/Net/src/TCPReactorAcceptor \
	UnpackedTarball/poco/Net/src/TCPReactorServer \
	UnpackedTarball/poco/Net/src/TCPReactorServerConnection \
	UnpackedTarball/poco/Net/src/TCPServer \
	UnpackedTarball/poco/Net/src/TCPServerConnection \
	UnpackedTarball/poco/Net/src/TCPServerConnectionFactory \
	UnpackedTarball/poco/Net/src/TCPServerDispatcher \
	UnpackedTarball/poco/Net/src/TCPServerParams \
	UnpackedTarball/poco/Net/src/UDPClient \
	UnpackedTarball/poco/Net/src/UDPServerParams \
	UnpackedTarball/poco/Net/src/WebSocket \
	UnpackedTarball/poco/Net/src/WebSocketImpl \
))

# vim: set noet sw=4 ts=4:
