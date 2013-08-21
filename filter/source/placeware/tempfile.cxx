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

#include <osl/file.h>
#include <sal/macros.h>

#if defined( UNX)

#include <stdio.h>
#include <string.h>
#include <osl/thread.h>

oslFileError SAL_CALL my_getTempDirURL( rtl_uString** pustrTempDir )
{
    const char *pValue = getenv( "TEMP" );

    if ( !pValue )
    {
        pValue = getenv( "TMP" );
#if defined(SOLARIS) || defined (LINUX)
        if ( !pValue )
            pValue = P_tmpdir;
#endif
    }

    if ( pValue )
    {
        oslFileError error;
        rtl_uString *ustrTempPath = NULL;

        rtl_string2UString( &ustrTempPath, pValue, strlen( pValue ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        error = osl_getFileURLFromSystemPath( ustrTempPath, pustrTempDir );
        rtl_uString_release( ustrTempPath );

        return error;
    }
    else
        return osl_File_E_NOENT;
}
#else

#ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x0400
#   define _CTYPE_DISABLE_MACROS /* wg. dynamischer C-Runtime MH */
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>

#if defined _MSC_VER
#pragma warning(pop)
#endif

oslFileError SAL_CALL my_getTempDirURL( rtl_uString** pustrTempDir )
{
    WCHAR   szBuffer[MAX_PATH];
    LPWSTR  lpBuffer = szBuffer;
    DWORD   nBufferLength = SAL_N_ELEMENTS(szBuffer) - 1;

    DWORD           nLength;
    oslFileError    error;

    do
    {
        nLength = GetTempPathW( SAL_N_ELEMENTS(szBuffer), lpBuffer );
        if ( nLength > nBufferLength )
        {
            nLength++;
            lpBuffer = (LPWSTR)alloca( sizeof(WCHAR) * nLength );
            nBufferLength = nLength - 1;
        }
    } while ( nLength > nBufferLength );

    if ( nLength )
    {
        rtl_uString *ustrTempPath = NULL;

        if ( '\\' == lpBuffer[nLength-1] )
            lpBuffer[nLength-1] = 0;

        rtl_uString_newFromStr( &ustrTempPath, reinterpret_cast<const sal_Unicode*>(lpBuffer) );

        error = osl_getFileURLFromSystemPath( ustrTempPath, pustrTempDir );

        rtl_uString_release( ustrTempPath );
    }
    else
        error = GetLastError() == ERROR_SUCCESS ? osl_File_E_None : osl_File_E_INVAL;

    return error;
}
#endif

#include "tempfile.hxx"

PlaceWareTempFile::PlaceWareTempFile( const OUString& rTempFileURL )
:osl::File( rTempFileURL ), maURL( rTempFileURL )
{
}

PlaceWareTempFile::~PlaceWareTempFile()
{
    close();

    if( !maURL.isEmpty() )
        osl::File::remove( maURL );
}

OUString PlaceWareTempFile::createTempFileURL()
{
    OUString aTempFileURL;

    const sal_uInt32 nRadix = 26;

    OUString aTempDirURL;
    /* oslFileError nRC = */ my_getTempDirURL( &aTempDirURL.pData );

    static sal_uInt32 u = osl_getGlobalTimer();
    for ( sal_uInt32 nOld = u; ++u != nOld; )
    {
        u %= (nRadix*nRadix*nRadix);
        OUString aTmp( aTempDirURL );
        if( aTmp.getStr()[ aTmp.getLength() - 1 ] != sal_Unicode( '/' ) )
            aTmp += "/";
        aTmp += OUString::number(  (unsigned) u, nRadix );
        aTmp += ".tmp";

        osl::File aFile( aTmp );
        osl::FileBase::RC err = aFile.open(osl_File_OpenFlag_Create);
        if (  err == FileBase::E_None )
        {
            aTempFileURL = aTmp;
            aFile.close();
            break;
        }
        else if ( err != FileBase::E_EXIST )
        {
             // if f.e. name contains invalid chars stop trying to create files
             break;
        }
    }

    return aTempFileURL;
}

OUString PlaceWareTempFile::getFileURL()
{
    return maURL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
