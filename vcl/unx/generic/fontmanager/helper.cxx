/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_folders.h>

#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <tools/urlobj.hxx>
#include <vcl/helper.hxx>
#include <vcl/ppdparser.hxx>
#include <memory>

using ::rtl::Bootstrap;

namespace psp {

OUString const & getOfficePath( whichOfficePath ePath )
{
    static OUString aInstallationRootPath;
    static OUString aUserPath;
    static OUString aConfigPath;
    static OUString aEmpty;
    static bool bOnce = false;

    if( ! bOnce )
    {
        bOnce = true;
        OUString aIni;
        Bootstrap::get( "BRAND_BASE_DIR", aInstallationRootPath );
        aIni = aInstallationRootPath + "/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap" );
        Bootstrap aBootstrap( aIni );
        aBootstrap.getFrom( "CustomDataUrl", aConfigPath );
        aBootstrap.getFrom( "UserInstallation", aUserPath );
        OUString aUPath = aUserPath;

        if( aConfigPath.startsWith( "file://" ) )
        {
            OUString aSysPath;
            if( osl_getSystemPathFromFileURL( aConfigPath.pData, &aSysPath.pData ) == osl_File_E_None )
                aConfigPath = aSysPath;
        }
        if( aInstallationRootPath.startsWith( "file://" ) )
        {
            OUString aSysPath;
            if( osl_getSystemPathFromFileURL( aInstallationRootPath.pData, &aSysPath.pData ) == osl_File_E_None )
                aInstallationRootPath = aSysPath;
        }
        if( aUserPath.startsWith( "file://" ) )
        {
            OUString aSysPath;
            if( osl_getSystemPathFromFileURL( aUserPath.pData, &aSysPath.pData ) == osl_File_E_None )
                aUserPath = aSysPath;
        }
        // ensure user path exists
        aUPath += "/user/psprint";
        #if OSL_DEBUG_LEVEL > 1
        oslFileError eErr =
        #endif
        osl_createDirectoryPath( aUPath.pData, nullptr, nullptr );
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "try to create \"%s\" = %d\n", OUStringToOString( aUPath, RTL_TEXTENCODING_UTF8 ).getStr(), eErr );
        #endif
    }

    switch( ePath )
    {
        case whichOfficePath::ConfigPath: return aConfigPath;
        case whichOfficePath::InstallationRootPath: return aInstallationRootPath;
        case whichOfficePath::UserPath: return aUserPath;
    }
    return aEmpty;
}

static OString getEnvironmentPath( const char* pKey )
{
    OString aPath;

    const char* pValue = getenv( pKey );
    if( pValue && *pValue )
    {
        aPath = OString( pValue );
    }
    return aPath;
}

} // namespace psp

void psp::getPrinterPathList( std::list< OUString >& rPathList, const char* pSubDir )
{
    rPathList.clear();
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    OUStringBuffer aPathBuffer( 256 );

    // append net path
    aPathBuffer.append( getOfficePath( whichOfficePath::InstallationRootPath ) );
    if( !aPathBuffer.isEmpty() )
    {
        aPathBuffer.append( "/" LIBO_SHARE_FOLDER "/psprint" );
        if( pSubDir )
        {
            aPathBuffer.append( '/' );
            aPathBuffer.appendAscii( pSubDir );
        }
        rPathList.push_back( aPathBuffer.makeStringAndClear() );
    }
    // append user path
    aPathBuffer.append( getOfficePath( whichOfficePath::UserPath ) );
    if( !aPathBuffer.isEmpty() )
    {
        aPathBuffer.append( "/user/psprint" );
        if( pSubDir )
        {
            aPathBuffer.append( '/' );
            aPathBuffer.appendAscii( pSubDir );
        }
        rPathList.push_back( aPathBuffer.makeStringAndClear() );
    }

    OString aPath( getEnvironmentPath("SAL_PSPRINT") );
    sal_Int32 nIndex = 0;
    do
    {
        OString aDir( aPath.getToken( 0, ':', nIndex ) );
        if( aDir.isEmpty() )
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

    #ifdef SYSTEM_PPD_DIR
    if( pSubDir && rtl_str_compare( pSubDir, PRINTER_PPDDIR ) == 0 )
    {
        rPathList.push_back( OStringToOUString( OString( SYSTEM_PPD_DIR ), RTL_TEXTENCODING_UTF8 ) );
    }
    #endif

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

OUString const & psp::getFontPath()
{
    static OUString aPath;

    if (aPath.isEmpty())
    {
        OUStringBuffer aPathBuffer( 512 );

        OUString aConfigPath( getOfficePath( whichOfficePath::ConfigPath ) );
        OUString aInstallationRootPath( getOfficePath( whichOfficePath::InstallationRootPath ) );
        OUString aUserPath( getOfficePath( whichOfficePath::UserPath ) );
        if( !aConfigPath.isEmpty() )
        {
            // #i53530# Path from CustomDataUrl will completely
            // replace net and user paths if the path exists
            aPathBuffer.append(aConfigPath);
            aPathBuffer.append("/" LIBO_SHARE_FOLDER "/fonts");
            // check existence of config path
            struct stat aStat;
            if( 0 != stat( OUStringToOString( aPathBuffer.makeStringAndClear(), osl_getThreadTextEncoding() ).getStr(), &aStat )
                || ! S_ISDIR( aStat.st_mode ) )
                aConfigPath.clear();
            else
            {
                aPathBuffer.append(aConfigPath);
                aPathBuffer.append("/" LIBO_SHARE_FOLDER "/fonts");
            }
        }
        if( aConfigPath.isEmpty() )
        {
            if( !aInstallationRootPath.isEmpty() )
            {
                aPathBuffer.append( aInstallationRootPath );
                aPathBuffer.append( "/" LIBO_SHARE_FOLDER "/fonts/truetype;");
                aPathBuffer.append( aInstallationRootPath );
                aPathBuffer.append( "/" LIBO_SHARE_FOLDER "/fonts/type1;" );
            }
            if( !aUserPath.isEmpty() )
            {
                aPathBuffer.append( aUserPath );
                aPathBuffer.append( "/user/fonts" );
            }
        }

        aPath = aPathBuffer.makeStringAndClear();
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "initializing font path to \"%s\"\n", OUStringToOString( aPath, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    }
    return aPath;
}

bool psp::convertPfbToPfa( ::osl::File& rInFile, ::osl::File& rOutFile )
{
    static const unsigned char hexDigits[] =
        {
            '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
        };

    bool bSuccess = true;
    bool bEof = false;
    unsigned char buffer[256];
    sal_uInt64 nSize(0);
    rInFile.getSize(nSize);

    while( bSuccess && ! bEof )
    {
        sal_uInt64 nRead;
        // read leading bytes
        bEof = ((0 != rInFile.read( buffer, 6, nRead)) || (nRead != 6));
        if( bEof )
            break;
        unsigned int nType = buffer[ 1 ];
        unsigned int nBytesToRead = buffer[2] | buffer[3] << 8 | buffer[4] << 16 | buffer[5] << 24;
        if( buffer[0] != 0x80 ) // test for pfb magic number
        {
            // this might be a pfa font already
            if( ! rInFile.read( buffer+6, 9, nRead ) && nRead == 9 &&
                ( ! std::strncmp( reinterpret_cast<char*>(buffer), "%!FontType1-", 12 ) ||
                  ! std::strncmp( reinterpret_cast<char*>(buffer), "%!PS-AdobeFont-", 15 ) ) )
            {
                sal_uInt64 nWrite = 0;
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
            sal_uInt64 nOrgPos(0);
            rInFile.getPos(nOrgPos);
            nBytesToRead = std::min<sal_uInt64>(nBytesToRead, nSize - nOrgPos);

            std::unique_ptr<unsigned char[]> pBuffer(new unsigned char[nBytesToRead+1]);
            pBuffer[nBytesToRead] = 0;

            if( ! rInFile.read( pBuffer.get(), nBytesToRead, nRead ) && nRead == nBytesToRead )
            {
                if( nType == 1 )
                {
                    // ascii data, convert dos lineends( \r\n ) and
                    // m_ac lineends( \r ) to \n
                    std::unique_ptr<unsigned char[]> pWriteBuffer(new unsigned char[ nBytesToRead ]);
                    unsigned int nBytesToWrite = 0;
                    for( unsigned int i = 0; i < nBytesToRead; i++ )
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
                    if( rOutFile.write( pWriteBuffer.get(), nBytesToWrite, nRead ) || nRead != nBytesToWrite )
                        bSuccess = false;
                }
                else
                {
                    // binary data
                    unsigned int nBuffer = 0;
                    for( unsigned int i = 0; i < nBytesToRead && bSuccess; i++ )
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
        }
        else if( nType == 3 )
            bEof = true;
        else
            bSuccess = false;
    }

    return bSuccess;
}

void psp::normPath( OString& rPath )
{
    char buf[PATH_MAX];

    // double slashes and slash at end are probably
    // removed by realpath anyway, but since this runs
    // on many different platforms let's play it safe
    OString aPath = rPath.replaceAll("//", "/");

    if( aPath.endsWith("/") )
        aPath = aPath.copy(0, aPath.getLength()-1);

    if( ( aPath.indexOf("./") != -1 ||
          aPath.indexOf( '~' ) != -1 )
        && realpath( aPath.getStr(), buf ) )
    {
        rPath = buf;
    }
    else
    {
        rPath = aPath;
    }
}

void psp::splitPath( OString& rPath, OString& rDir, OString& rBase )
{
    normPath( rPath );
    sal_Int32 nIndex = rPath.lastIndexOf( '/' );
    if( nIndex > 0 )
        rDir = rPath.copy( 0, nIndex );
    else if( nIndex == 0 ) // root dir
        rDir = rPath.copy( 0, 1 );
    if( rPath.getLength() > nIndex+1 )
        rBase = rPath.copy( nIndex+1 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
