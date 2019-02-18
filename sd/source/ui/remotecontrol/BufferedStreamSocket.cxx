/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "BufferedStreamSocket.hxx"

#include <osl/socket.hxx>
#include <sal/log.hxx>
#include <algorithm>

#ifdef _WIN32
  // LO vs WinAPI conflict
  #undef WB_LEFT
  #undef WB_RIGHT

  #include <winsock2.h>
#else
 #include <sys/socket.h>
 #include <unistd.h>
#endif
using namespace sd;
using namespace std;
using namespace osl;

BufferedStreamSocket::BufferedStreamSocket( const osl::StreamSocket &aSocket ):
    StreamSocket( aSocket ),
    aRet( 0 ),
    aRead( 0 ),
    aBuffer(),
    mSocket( 0 ),
    usingCSocket( false )
{
}

BufferedStreamSocket::BufferedStreamSocket( int aSocket ):
    StreamSocket(),
    aRet( 0 ),
    aRead( 0 ),
    aBuffer(),
    mSocket( aSocket ),
    usingCSocket( true )
{
}

void BufferedStreamSocket::getPeerAddr(osl::SocketAddr& rAddr)
{
    assert ( !usingCSocket );
    StreamSocket::getPeerAddr( rAddr );
}

sal_Int32 BufferedStreamSocket::write( const void* pBuffer, sal_uInt32 n )
{
    if ( !usingCSocket )
        return StreamSocket::write( pBuffer, n );
    else
        return ::send(
            mSocket,
#if defined(_WIN32)
            static_cast<char const *>(pBuffer),
#else
            pBuffer,
#endif
            static_cast<size_t>(n), 0 );
}

void BufferedStreamSocket::close()
{
    if( usingCSocket && mSocket != -1 )
    {
#ifdef _WIN32
        ::closesocket( mSocket );
#else
        ::close( mSocket );
#endif
        mSocket = -1;
    }
    else
        ::osl::StreamSocket::close();
}

sal_Int32 BufferedStreamSocket::readLine( OString& aLine )
{
    while ( true )
    {
        // Process buffer first in case data already present.
        vector<char>::iterator aIt;
        if ( (aIt = find( aBuffer.begin(), aBuffer.end(), '\n' ))
             != aBuffer.end() )
        {
            sal_uInt64 aLocation = aIt - aBuffer.begin();

            aLine = OString( &(*aBuffer.begin()), aLocation );

            aBuffer.erase( aBuffer.begin(), aIt + 1 ); // Also delete the empty line
            aRead -= (aLocation + 1);

            SAL_INFO( "sdremote.bluetooth", "recv line '" << aLine << "'" );

            return aLine.getLength() + 1;
        }

        // Then try and receive if nothing present
        aBuffer.resize( aRead + 100 );
        if ( !usingCSocket)
            aRet = StreamSocket::recv( &aBuffer[aRead], 100 );
        else
            aRet = ::recv( mSocket, &aBuffer[aRead], 100, 0 );

        SAL_INFO( "sdremote.bluetooth", "recv " << aRet << " aBuffer len " << aBuffer.size() );
        if ( aRet <= 0 )
        {
            return 0;
        }
        // Prevent buffer from growing massively large.
        if ( aRead > MAX_LINE_LENGTH )
        {
            aBuffer.clear();
            return 0;
        }
        aRead += aRet;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
