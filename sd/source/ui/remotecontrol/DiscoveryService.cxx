/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <iostream>

#include <rtl/strbuf.hxx>
#include <osl/socket.hxx>
#include <config_features.h>
#include <sal/log.hxx>

#include "DiscoveryService.hxx"
#include "ZeroconfService.hxx"

#ifdef _WIN32
  // LO vs WinAPI conflict
  #undef WB_LEFT
  #undef WB_RIGHT

  #include <winsock2.h>
  #include <ws2tcpip.h>

  #include "WINNetworkService.hxx"
  typedef int socklen_t;
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif

#ifdef MACOSX
  #include <osl/conditn.hxx>
  #include <premac.h>
  #import <CoreFoundation/CoreFoundation.h>
  #include <postmac.h>
  #import "OSXNetworkService.hxx"
#endif

#if HAVE_FEATURE_AVAHI
  #include "AvahiNetworkService.hxx"
#endif

using namespace osl;
using namespace sd;

DiscoveryService::DiscoveryService()
    : mSocket(-1)
    , zService(nullptr)
{
}

DiscoveryService::~DiscoveryService()
{
    if (mSocket != -1)
    {
#ifdef _WIN32
        closesocket( mSocket );
#else
        close( mSocket );
#endif
    }

    if (zService)
         zService->clear();
}

void DiscoveryService::setupSockets()
{

#ifdef MACOSX
    // Bonjour for OSX
    zService = new OSXNetworkService();
    zService->setup();
#endif

#if HAVE_FEATURE_AVAHI
    // Avahi for Linux
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);

    zService = new AvahiNetworkService(hostname);
    zService->setup();
#endif

#ifdef _WIN32
    zService = new WINNetworkService();
    zService->setup();
#endif

    // Old implementation for backward compatibility matter
    mSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (mSocket == -1)
    {
        SAL_WARN("sd", "DiscoveryService: socket failed: " << errno);
        return; // would be better to throw, but unsure if caller handles that
    }

    sockaddr_in aAddr;
    memset(&aAddr, 0, sizeof(aAddr));
    aAddr.sin_family = AF_INET;
    aAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    aAddr.sin_port = htons( PORT_DISCOVERY );

    int rc = bind( mSocket, reinterpret_cast<sockaddr*>(&aAddr), sizeof(sockaddr_in) );

    if (rc)
    {
        SAL_WARN("sd", "DiscoveryService: bind failed: " << errno);
        return; // would be better to throw, but unsure if caller handles that
    }

    struct ip_mreq multicastRequest;

// the Win32 SDK 8.1 deprecates inet_addr()
#if defined(_WIN32)
    IN_ADDR addr;
    INT ret = InetPtonW(AF_INET, L"239.0.0.1", & addr);
    if (1 == ret)
    {
        multicastRequest.imr_multiaddr.s_addr = addr.S_un.S_addr;
    }
#else
    multicastRequest.imr_multiaddr.s_addr = inet_addr( "239.0.0.1" );
#endif
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

    rc = setsockopt( mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
    #ifdef _WIN32
        reinterpret_cast<const char*>(&multicastRequest),
    #else
        &multicastRequest,
    #endif
        sizeof(multicastRequest));

    if (rc)
    {
        SAL_WARN("sd", "DiscoveryService: setsockopt failed: " << errno);
        return; // would be better to throw, but unsure if caller handles that
    }
}

void SAL_CALL DiscoveryService::run()
{
    osl::Thread::setName("DiscoveryService");

    setupSockets();

    // Kept for backward compatibility
    char aBuffer[BUFFER_SIZE];
    while ( true )
    {
        memset( aBuffer, 0, sizeof(char) * BUFFER_SIZE );
        sockaddr_in aAddr;
        socklen_t aLen = sizeof( aAddr );
        if(recvfrom( mSocket, aBuffer, BUFFER_SIZE, 0, reinterpret_cast<sockaddr*>(&aAddr), &aLen ) > 0)
        {
            OString aString( aBuffer, strlen( "LOREMOTE_SEARCH" ) );
            if ( aString == "LOREMOTE_SEARCH" )
            {
                OStringBuffer aStringBuffer("LOREMOTE_ADVERTISE\n");
                aStringBuffer.append( OUStringToOString(
                                              osl::SocketAddr::getLocalHostname(), RTL_TEXTENCODING_UTF8 ) )
                    .append( "\n\n" );
                if ( sendto( mSocket, aStringBuffer.getStr(),
                             aStringBuffer.getLength(), 0, reinterpret_cast<sockaddr*>(&aAddr),
                             sizeof(aAddr) ) <= 0 )
                {
                    // Write error or closed socket -- we are done.
                    return;
                }
            }
        }
        else
        {
            // Read error or closed socket -- we are done.
            return;
        }
    }
}

DiscoveryService *sd::DiscoveryService::spService = nullptr;

void DiscoveryService::setup()
{
  if (spService)
    return;

  spService = new DiscoveryService();
  spService->create();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
