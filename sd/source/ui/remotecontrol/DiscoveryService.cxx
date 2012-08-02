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

using namespace osl;
using namespace rtl;
using namespace sd;
using namespace std;

DiscoveryService::DiscoveryService()
    :
    Thread( "sd::DiscoveryService" ),
    mSocket()
{
}

DiscoveryService::~DiscoveryService()
{
}


void DiscoveryService::replyTo( SocketAddr& rAddr )
{
    SocketAddr aLocalAddr;
    mSocket.getLocalAddr( aLocalAddr );
    OString aAddrString = OUStringToOString( aLocalAddr.getHostname(),
                                             RTL_TEXTENCODING_UTF8 );
    OStringBuffer aBuffer( "LOREMOTE_ADVERTISE\n" );
    aBuffer.append( aAddrString ).append( "\n" );
    mSocket.sendTo( rAddr, aBuffer.getStr(), aBuffer.getLength() );
}

void DiscoveryService::execute()
{
    sal_uInt64 aRet, aRead;
    vector<char> aBuffer;
    aRead = 0;

    SocketAddr aAddr;
    while ( true )
    {
        aBuffer.resize( aRead + 100 );
        aRet = mSocket.recvFrom( &aBuffer[aRead], 100 );
        if ( aRet == 0 )
        {
            fprintf( stderr, "Socket returned 0\n" );
//             break; // I.e. transmission finished.
        }
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