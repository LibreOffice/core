/*************************************************************************
 *
 *  $RCSfile: helper.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 17:18:29 $
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

#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

#include <psprint/helper.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <tools/config.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/config.h>
#include "jvmaccess/javainfo.hxx"

using namespace rtl;

namespace psp {

enum whichOfficePath { NetPath, UserPath };

static const OUString& getOfficePath( enum whichOfficePath ePath )
{
    static OUString aNetPath;
    static OUString aUserPath;
    static OUString aEmpty;
    static bool bOnce = false;

    if( ! bOnce )
    {
        bOnce = true;
        OUString aIni;
        osl_getExecutableFile( &aIni.pData );
        aIni = aIni.copy( 0, aIni.lastIndexOf( '/' )+1 );
        aIni += OUString( RTL_CONSTASCII_USTRINGPARAM( SAL_CONFIGFILE( "bootstrap" ) ) );
        Bootstrap aBootstrap( aIni );
        aBootstrap.getFrom( OUString( RTL_CONSTASCII_USTRINGPARAM( "BaseInstallation" ) ), aNetPath );
        aBootstrap.getFrom( OUString( RTL_CONSTASCII_USTRINGPARAM( "UserInstallation" ) ), aUserPath );

        if( ! aNetPath.compareToAscii( "file://", 7 ) )
        {
            OUString aSysPath;
            if( osl_getSystemPathFromFileURL( aNetPath.pData, &aSysPath.pData ) == osl_File_E_None )
                aNetPath = aSysPath;
        }
        if( ! aUserPath.compareToAscii( "file://", 7 ) )
        {
            OUString aSysPath;
            if( osl_getSystemPathFromFileURL( aUserPath.pData, &aSysPath.pData ) == osl_File_E_None )
                aUserPath = aSysPath;
        }
    }

    switch( ePath )
    {
        case NetPath: return aNetPath;
        case UserPath: return aUserPath;
    }
    return aEmpty;
}

static OUString getEnvironmentPath( const char* pKey, sal_Unicode cPrefix )
{
    OUString aPath;

    const char* pValue = getenv( pKey );
    if( pValue && *pValue )
    {
        aPath  = OUString( cPrefix );
        aPath += OUString( pValue, strlen( pValue ), gsl_getSystemTextEncoding() );
    }
    return aPath;
}

} // namespace psp

const OUString& psp::getPrinterPath()
{
    static OUString aPath;

    if( ! aPath.getLength() )
    {
        OUString aNetPath = getOfficePath( psp::NetPath );
        OUString aUserPath = getOfficePath( psp::UserPath );
        if( aNetPath.getLength() )
        {
            aPath += aNetPath;
            aPath += OUString( RTL_CONSTASCII_USTRINGPARAM( "/share/psprint" ) );
        }
        if( aUserPath.getLength() )
        {
            if( aPath.getLength() )
                aPath += OUString( RTL_CONSTASCII_USTRINGPARAM( ":" ) );
            aPath += aUserPath;
            aPath += OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/psprint" ) );
        }
        aPath += ::psp::getEnvironmentPath( "SAL_PSPRINT", (sal_Unicode)':' );

#ifdef DEBUG
        fprintf( stderr, "initalizing printer path to \"%s\"\n", OUStringToOString( aPath, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    }
    return aPath;
}

void psp::getPrinterPathList( std::list< OUString >& rPathList, const char* pSubDir )
{
    rPathList.clear();
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    OString aPath( OUStringToOString( getPrinterPath(), aEncoding ) );
    sal_Int32 nIndex = 0;
    do
    {
        OString aDir( aPath.getToken( 0, ':', nIndex ) );
        if( ! aDir.getLength() )
            continue;

        if( pSubDir )
        {
            aDir += "/";
            aDir += pSubDir;
        }
        struct stat aStat;
        if( stat( aDir.getStr(), &aStat ) || ! S_ISDIR( aStat.st_mode ) )
            continue;

        rPathList.push_back( OStringToOUString( aDir, aEncoding ) );
    } while( nIndex != -1 );

    if( rPathList.empty() )
    {
        // last resort: next to program file (mainly for setup)
        OUString aExe;
        if( osl_getExecutableFile( &aExe.pData ) == osl_Process_E_None )
        {
            INetURLObject aDir( aExe );
            aDir.removeSegment();
            aExe = aDir.GetMainURL( INetURLObject::NO_DECODE );
            OUString aSysPath;
            if( osl_getSystemPathFromFileURL( aExe.pData, &aSysPath.pData ) == osl_File_E_None )
            {
                rPathList.push_back( aSysPath );
            }
        }
    }
}

const OUString& psp::getFontPath()
{
    static OUString aPath;

    if( ! aPath.getLength() )
    {
        OUString aNetPath = getOfficePath( psp::NetPath );
        OUString aUserPath = getOfficePath( psp::UserPath );
        if( aNetPath.getLength() )
        {
            aPath  = aNetPath;
            aPath += OUString( RTL_CONSTASCII_USTRINGPARAM("/share/fonts/truetype;") );
            aPath += aNetPath;
            aPath += OUString( RTL_CONSTASCII_USTRINGPARAM("/share/fonts/type1;") );
        }
        if( aUserPath.getLength() )
        {
            aPath += aUserPath;
            aPath += OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/fonts" ) );
        }
        aPath += ::psp::getEnvironmentPath( "SAL_FONTPATH_PRIVATE", (sal_Unicode)';' );

        // append jre/jdk fonts if possible
        OString aJREpath;
        if( aUserPath.getLength() )
        {
            OUString aJavaRc( aUserPath );
            aJavaRc += OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/config/" SAL_CONFIGFILE( "java" ) ) );
            Config aConfig( aJavaRc );
            aConfig.SetGroup( "Java" );
            aJREpath = aConfig.ReadKey( "Home" );
            if( aJREpath.compareTo( "file:", 5 ) == 0 )
            {
                OUString aURL( OStringToOUString( aJREpath, osl_getThreadTextEncoding() ) );
                OUString aSys;
                if( osl_getSystemPathFromFileURL( aURL.pData, &aSys.pData ) == osl_File_E_None )
                    aJREpath = OUStringToOString( aSys, osl_getThreadTextEncoding() );
            }
            if( aJREpath.getLength() > 0 )
            {
                OString aTestPath( aJREpath );
                aTestPath += "/jre/lib/fonts";
                if( access( aTestPath.getStr(), R_OK ) )
                {
                    aTestPath = aJREpath;
                    aTestPath += "/lib/fonts";
                    if( access( aTestPath.getStr(), R_OK ) )
                        aJREpath = OString();
                    else
                        aJREpath = aTestPath;
                }
                else
                    aJREpath = aTestPath;
            }
        }

        // if no javarc (e.g. in setup) exists or it failed try the UDK method
        if( ! aJREpath.getLength() )
        {
            OString aJavaLib;
            try
            {
                OUString aLib;
                if (osl::FileBase::getSystemPathFromFileURL(
                            jvmaccess::JavaInfo::createBestInfo(true).
                                getRuntimeLibLocation(),
                            aLib)
                        == osl::FileBase::E_None)
                    aLib.convertToString(
                        &aJavaLib, osl_getThreadTextEncoding(),
                        RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                            | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR);
            }
            catch (jvmaccess::JavaInfo::InitException &)
            {}

            if (aJavaLib.getLength() != 0)
            {
                sal_Int32 nIndex;
                while( ( nIndex = aJavaLib.lastIndexOf( '/' ) ) != -1 )
                {
                    aJavaLib = aJavaLib.copy( 0, nIndex );
                    OString aTmpPath = aJavaLib;
                    aTmpPath += "/lib/fonts";
                    if( access( aTmpPath.getStr(), R_OK ) == 0 )
                    {
                        aJREpath = aTmpPath;
                        break;
                    }
                }
            }
        }

        if( aJREpath.getLength() )
        {
            aPath += OUString( RTL_CONSTASCII_USTRINGPARAM( ";" ) );
            aPath += OStringToOUString( aJREpath, osl_getThreadTextEncoding() );
        }
#ifdef DEBUG
        fprintf( stderr, "initalizing font path to \"%s\"\n", OUStringToOString( aPath, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
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

                    delete [] pWriteBuffer;
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

            delete [] pBuffer;
        }
        else if( nType == 3 )
            bEof = true;
        else
            bSuccess = false;
    }

    return bSuccess;
}
