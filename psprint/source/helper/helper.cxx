/*************************************************************************
 *
 *  $RCSfile: helper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2001-09-14 12:02:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <psprint/helper.hxx>
#include <tools/string.hxx>
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>

const ::rtl::OUString& psp::getPrinterPath()
{
    static ::rtl::OUString aPath;

    if( ! aPath.getLength() )
    {
        ::com::sun::star::uno::Any aPathEntry;
        ::rtl::OUString aEntry;

        aPathEntry = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::OFFICEINSTALL );
        aPathEntry >>= aPath;
        aPath += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/share/psprint:" ) );
        aPathEntry = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::INSTALLPATH );
        aPathEntry >>= aEntry;
        aPath += aEntry;
        aPath += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/psprint" ) );

        const char* pEnv = getenv( "SAL_PSPRINT" );
        if( pEnv && *pEnv )
        {
            aPath += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":" ) );
            aEntry = ::rtl::OUString( pEnv ? pEnv : "", pEnv ? strlen( pEnv ) : 0, gsl_getSystemTextEncoding() );
            aPath += aEntry;
        }
#ifdef DEBUG
        fprintf( stderr, "initalizing printer path to \"%s\"\n", ::rtl::OUStringToOString( aPath, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    }
    return aPath;
}

bool psp::convertPfbToPfa( ::osl::File& rInFile, ::osl::File& rOutFile )
{
    static unsigned char hexDigits[] =
        {
            '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
        };

    bool bSuccess = true;
    bool bEof = false;
    unsigned char buffer[256];
    sal_uInt64 nRead;
    sal_uInt64 nOrgPos = 0;
    rInFile.getPos( nOrgPos );

    while( bSuccess && ! bEof )
    {
        // read leading bytes
        bEof = ! rInFile.read( buffer, 6, nRead ) && nRead == 6 ? false : true;
        int nType = buffer[ 1 ];
        int nBytesToRead = buffer[2] | buffer[3] << 8 | buffer[4] << 16 | buffer[5] << 24;
        if( buffer[0] != 0x80 ) // test for pfb m_agic number
        {
            // this migt be a pfa font already
            sal_uInt64 nWrite = 0;
            if( ! rInFile.read( buffer+6, 9, nRead ) && nRead == 9 &&
                ( ! strncmp( (char*)buffer, "%!FontType1-", 12 ) ||
                  ! strncmp( (char*)buffer, "%!PS-AdobeFont-", 15 ) ) )
            {
                if( rOutFile.write( buffer, 15, nWrite ) || nWrite != 15 )
                    bSuccess = false;
                while( bSuccess &&
                       ! rInFile.read( buffer, sizeof( buffer ), nRead ) &&
                       nRead != 0 )
                {
                    if( rOutFile.write( buffer, nRead, nWrite ) ||
                        nWrite != nRead )
                        bSuccess = false;
                }
                bEof = true;
            }
            else
                bSuccess = false;
        }
        else if( nType == 1 || nType == 2 )
        {
            unsigned char* pBuffer = new unsigned char[ nBytesToRead+1 ];

            if( ! rInFile.read( pBuffer, nBytesToRead, nRead ) && nRead == nBytesToRead )
            {
                if( nType == 1 )
                {
                    // ascii data, convert dos lineends( \r\n ) and
                    // m_ac lineends( \r ) to \n
                    unsigned char * pWriteBuffer = new unsigned char[ nBytesToRead ];
                    int nBytesToWrite = 0;
                    for( int i = 0; i < nBytesToRead; i++ )
                    {
                        if( pBuffer[i] != '\r' )
                            pWriteBuffer[ nBytesToWrite++ ] = pBuffer[i];
                        else if( pBuffer[ i+1 ] == '\n' )
                        {
                            i++;
                            pWriteBuffer[ nBytesToWrite++ ] = '\n';
                        }
                        else
                            pWriteBuffer[ nBytesToWrite++ ] = '\n';
                    }
                    if( rOutFile.write( pWriteBuffer, nBytesToWrite, nRead ) || nRead != nBytesToWrite )
                        bSuccess = false;

                    delete pWriteBuffer;
                }
                else
                {
                    // binary data
                    int nBuffer = 0;
                    for( int i = 0; i < nBytesToRead && bSuccess; i++ )
                    {
                        buffer[ nBuffer++ ] = hexDigits[ pBuffer[ i ] >> 4 ];
                        buffer[ nBuffer++ ] = hexDigits[ pBuffer[ i ] & 15 ];
                        if( nBuffer >= 80 )
                        {
                            buffer[ nBuffer++ ] = '\n';
                            if( rOutFile.write( buffer, nBuffer, nRead ) || nRead != nBuffer )
                                bSuccess = false;
                            nBuffer = 0;
                        }
                    }
                    if( nBuffer > 0 && bSuccess )
                    {
                        buffer[ nBuffer++ ] = '\n';
                        if( rOutFile.write( buffer, nBuffer, nRead ) || nRead != nBuffer )
                            bSuccess = false;
                    }
                }
            }
            else
                bSuccess = false;

            delete pBuffer;
        }
        else if( nType == 3 )
            bEof = true;
        else
            bSuccess = false;
    }

    return bSuccess;
}
