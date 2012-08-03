/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdlib.h>
#include <algorithm>
#include <vector>

#include <comphelper/processfactory.hxx>
#include <rtl/strbuf.hxx>

#include "DiscoveryService.hxx"

#ifdef WIN32
  #include <winsock.h>
  typedef int socklen_t;
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif

using namespace osl;
using namespace rtl;
using namespace sd;
using namespace std;

DiscoveryService::DiscoveryService()
    :
    Thread( "sd::DiscoveryService" )
//     mSocket()
{
    mSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    sockaddr_in aAddr;
    aAddr.sin_family = AF_INET;
    aAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    aAddr.sin_port = htons( PORT_DISCOVERY );

    bind( mSocket, (sockaddr*) &aAddr, sizeof(sockaddr_in) );

    struct ip_mreq multicastRequest;

    multicastRequest.imr_multiaddr.s_addr = inet_addr( "239.0.0.1" );
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

    setsockopt( mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
        &multicastRequest, sizeof(multicastRequest));
}

DiscoveryService::~DiscoveryService()
{
}


void DiscoveryService::replyTo( sockaddr_in& rAddr )
{
    OStringBuffer aBuffer("LOREMOTE_ADVERTISE\n");
    aBuffer.append( OUStringToOString( osl::SocketAddr::getLocalHostname(),
            RTL_TEXTENCODING_UTF8 ) ).append( "\n\n" );
    sendto( mSocket, aBuffer.getStr(), aBuffer.getLength(), 0,
            (sockaddr*) &rAddr, sizeof(rAddr) );
}

void DiscoveryService::execute()
{
    fprintf( stderr, "Discovery service is listening\n" );;
    sal_uInt64 aRet, aRead;
    vector<char> aBuffer;
    aRead = 0;

    while ( true )
    {
        aBuffer.resize( aRead + 100 );

        sockaddr_in aAddr;
        socklen_t aLen = sizeof( aAddr );
        fprintf( stderr, "DiscoveryService waiting for packet\n" );
//         aRet = mSocket.recvFrom( &aBuffer[aRead], 100 );
        recvfrom( mSocket, &aBuffer[aRead], 100, 0, (sockaddr*) &aAddr, &aLen );
        fprintf( stderr, "DiscoveryService received a packet.\n" );
//         if ( aRet == 0 )
//         {
//             fprintf( stderr, "Socket returned 0\n" );
// //             break; // I.e. transmission finished.
//         }
        aRead += aRet;
        vector<char>::iterator aIt;
        while ( (aIt = find( aBuffer.begin(), aBuffer.end(), '\n' ))
            != aBuffer.end() )
        {
            sal_uInt64 aLocation = aIt - aBuffer.begin();
            OString aString( &(*aBuffer.begin()), aLocation );
            if ( aString.compareTo( "LOREMOTE_SEARCH" ) == 0 ) {
                replyTo( aAddr );
            }
            aBuffer.erase( aBuffer.begin(), aIt + 1 ); // Also delete the newline
            aRead -= (aLocation + 1);
        }
    }
}

DiscoveryService *sd::DiscoveryService::spService = NULL;

void DiscoveryService::setup()
{
  if (spService)
    return;

  spService = new DiscoveryService();
  spService->launch();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */