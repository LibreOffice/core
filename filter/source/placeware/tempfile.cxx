/*************************************************************************
 *
 *  $RCSfile: tempfile.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2002-10-25 14:03:33 $
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
 *  Contributor(s): Christian Lippka (christian.lippka@sun.com)
 *
 *
 ************************************************************************/


#include <osl/file.h>

#ifdef UNX

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

#ifdef NDEBUG
#    define NO_DEBUG_CRT
#endif

#ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x0400
#   define _CTYPE_DISABLE_MACROS /* wg. dynamischer C-Runtime MH */
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <malloc.h>

#define elementsof(arr) (sizeof(arr)/sizeof(arr[0]))

oslFileError SAL_CALL my_getTempDirURL( rtl_uString** pustrTempDir )
{
    WCHAR   szBuffer[MAX_PATH];
    LPWSTR  lpBuffer = szBuffer;
    DWORD   nBufferLength = elementsof(szBuffer) - 1;

    DWORD           nLength;
    oslFileError    error;

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
        rtl_uString *ustrTempPath = NULL;

        if ( '\\' == lpBuffer[nLength-1] )
            lpBuffer[nLength-1] = 0;

        rtl_uString_newFromStr( &ustrTempPath, lpBuffer );

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
:maURL( rTempFileURL ), osl::File( rTempFileURL )
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
    oslFileError nRC = my_getTempDirURL( &aTempDirURL.pData );

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
