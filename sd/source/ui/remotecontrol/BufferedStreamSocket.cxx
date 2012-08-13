/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <BufferedStreamSocket.hxx>

#include <algorithm>

using namespace sd;
using namespace std;
using namespace osl;

BufferedStreamSocket::BufferedStreamSocket( const osl::StreamSocket &aSocket ):
    StreamSocket( aSocket ),
    aRet( 0 ),
    aRead( 0 ),
    aBuffer()
{
}

sal_Int32 BufferedStreamSocket::readLine( OString& aLine )
{
    while ( true )
    {
        // Process buffer first incase data already present.
        vector<char>::iterator aIt;
        if ( (aIt = find( aBuffer.begin(), aBuffer.end(), '\n' ))
            != aBuffer.end() )
        {
            sal_uInt64 aLocation = aIt - aBuffer.begin();

            aLine = OString( &(*aBuffer.begin()), aLocation );

            aBuffer.erase( aBuffer.begin(), aIt + 1 ); // Also delete the empty line
            aRead -= (aLocation + 1);

            return aLine.getLength();
        }

        // Then try and receive if nothing present
        aBuffer.resize( aRead + 100 );
        aRet = recv( &aBuffer[aRead], 100 );

        if ( aRet == 0 )
        {
                return aRet;
        }
        // Prevent buffer from growing massively large.
        if ( aRead > MAX_LINE_LENGTH )
        {
            aBuffer.erase( aBuffer.begin(), aBuffer.end() );
            return 0;
        }
        aRead += aRet;
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
