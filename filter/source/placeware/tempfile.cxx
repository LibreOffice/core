/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"


#include <osl/file.h>

#if defined( UNX) || defined(OS2)

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
        rtl_uString	*ustrTempPath = NULL;

        rtl_string2UString( &ustrTempPath, pValue, strlen( pValue ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        error = osl_getFileURLFromSystemPath( ustrTempPath, pustrTempDir );
        rtl_uString_release( ustrTempPath );
        
        return error;
    }
    else
        return osl_File_E_NOENT;
}
#else

#ifdef NDEBUG
#    define NO_DEBUG_CRT
#endif

#ifndef _WIN32_WINNT 
#	define _WIN32_WINNT 0x0400
#	define _CTYPE_DISABLE_MACROS /* wg. dynamischer C-Runtime MH */
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

#define elementsof(arr) (sizeof(arr)/sizeof(arr[0]))

oslFileError SAL_CALL my_getTempDirURL( rtl_uString** pustrTempDir )
{
    WCHAR	szBuffer[MAX_PATH];
    LPWSTR	lpBuffer = szBuffer;
    DWORD	nBufferLength = elementsof(szBuffer) - 1;

    DWORD			nLength;
    oslFileError	error;

    do
    {
        nLength = GetTempPathW( elementsof(szBuffer), lpBuffer );
        if ( nLength > nBufferLength )
        {
            nLength++;
            lpBuffer = (LPWSTR)alloca( sizeof(WCHAR) * nLength );
            nBufferLength = nLength - 1;
        }
    } while ( nLength > nBufferLength );

    if ( nLength )
    {
        rtl_uString	*ustrTempPath = NULL;

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

using namespace rtl;

TempFile::TempFile( const OUString& rTempFileURL )
:osl::File( rTempFileURL ), maURL( rTempFileURL )
{
}

TempFile::~TempFile()
{
    close();

    if( maURL.getLength() )
        osl::File::remove( maURL );
}

OUString TempFile::createTempFileURL()
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
            aTmp += OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ));
        aTmp += OUString::valueOf( (sal_Int32) (unsigned) u, nRadix );
        aTmp += OUString::createFromAscii( ".tmp" );

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

OUString TempFile::getFileURL()
{
    return maURL;
}
