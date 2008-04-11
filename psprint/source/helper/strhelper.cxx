/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: strhelper.cxx,v $
 * $Revision: 1.16 $
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
#include "precompiled_psprint.hxx"
#include <psprint/strhelper.hxx>
#include <sal/alloca.h>

namespace psp {

inline int isSpace( char cChar )
{
    return
        cChar == ' '    || cChar == '\t'    ||
        cChar == '\r'   || cChar == '\n'    ||
        cChar == 0x0c   || cChar == 0x0b;
}

inline int isSpace( sal_Unicode cChar )
{
    return
        cChar == ' '    || cChar == '\t'    ||
        cChar == '\r'   || cChar == '\n'    ||
        cChar == 0x0c   || cChar == 0x0b;
}

inline int isProtect( char cChar )
{
    return cChar == '`' || cChar == '\'' || cChar == '"';
}

inline int isProtect( sal_Unicode cChar )
{
    return cChar == '`' || cChar == '\'' || cChar == '"';
}

inline void CopyUntil( char*& pTo, const char*& pFrom, char cUntil, int bIncludeUntil = 0 )
{
    do
    {
        if( *pFrom == '\\' )
        {
            pFrom++;
            if( *pFrom )
            {
                *pTo = *pFrom;
                pTo++;
            }
        }
        else if( bIncludeUntil || ! isProtect( *pFrom ) )
        {
            *pTo = *pFrom;
            pTo++;
        }
        pFrom++;
    } while( *pFrom && *pFrom != cUntil );
    // copy the terminating character unless zero or protector
    if( ! isProtect( *pFrom ) || bIncludeUntil )
    {
        *pTo = *pFrom;
        if( *pTo )
            pTo++;
    }
    if( *pFrom )
        pFrom++;
}

inline void CopyUntil( sal_Unicode*& pTo, const sal_Unicode*& pFrom, sal_Unicode cUntil, int bIncludeUntil = 0 )
{
    do
    {
        if( *pFrom == '\\' )
        {
            pFrom++;
            if( *pFrom )
            {
                *pTo = *pFrom;
                pTo++;
            }
        }
        else if( bIncludeUntil || ! isProtect( *pFrom ) )
        {
            *pTo = *pFrom;
            pTo++;
        }
        pFrom++;
    } while( *pFrom && *pFrom != cUntil );
    // copy the terminating character unless zero or protector
    if( ! isProtect( *pFrom ) || bIncludeUntil )
    {
        *pTo = *pFrom;
        if( *pTo )
            pTo++;
    }
    if( *pFrom )
        pFrom++;
}

String GetCommandLineToken( int nToken, const String& rLine )
{
    int nLen = rLine.Len();
    if( ! nLen )
        return String();

    int nActualToken = 0;
    sal_Unicode* pBuffer = (sal_Unicode*)alloca( sizeof(sal_Unicode)*( nLen + 1 ) );
    const sal_Unicode* pRun = rLine.GetBuffer();
    sal_Unicode* pLeap = NULL;

    while( *pRun && nActualToken <= nToken )
    {
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        pLeap = pBuffer;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '`' )
                CopyUntil( pLeap, pRun, '`' );
            else if( *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'' );
            else if( *pRun == '"' )
                CopyUntil( pLeap, pRun, '"' );
            else
            {
                *pLeap = *pRun;
                pLeap++;
                pRun++;
            }
        }
        if( nActualToken != nToken )
            pBuffer[0] = 0;
        nActualToken++;
    }

    *pLeap = 0;

    String aRet( pBuffer );
    return aRet;
}

ByteString GetCommandLineToken( int nToken, const ByteString& rLine )
{
    int nLen = rLine.Len();
    if( ! nLen )
        return ByteString();

    int nActualToken = 0;
    char* pBuffer = (char*)alloca( nLen + 1 );
    const char* pRun = rLine.GetBuffer();
    char* pLeap = NULL;

    while( *pRun && nActualToken <= nToken )
    {
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        pLeap = pBuffer;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '`' )
                CopyUntil( pLeap, pRun, '`' );
            else if( *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'' );
            else if( *pRun == '"' )
                CopyUntil( pLeap, pRun, '"' );
            else
            {
                *pLeap = *pRun;
                pLeap++;
                pRun++;
            }
        }
        if( nActualToken != nToken )
            pBuffer[0] = 0;
        nActualToken++;
    }

    *pLeap = 0;

    ByteString aRet( pBuffer );
    return aRet;
}

int GetCommandLineTokenCount( const String& rLine )
{
    if( ! rLine.Len() )
        return 0;

    int nTokenCount = 0;
    const sal_Unicode *pRun = rLine.GetBuffer();


    while( *pRun )
    {
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        if( ! *pRun )
            break;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '`' )
            {
                do pRun++; while( *pRun && *pRun != '`' );
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '\'' )
            {
                do pRun++; while( *pRun && *pRun != '\'' );
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '"' )
            {
                do pRun++; while( *pRun && *pRun != '"' );
                if( *pRun )
                    pRun++;
            }
            else
                pRun++;
        }
        nTokenCount++;
    }

    return nTokenCount;
}

int GetCommandLineTokenCount( const ByteString& rLine )
{
    if( ! rLine.Len() )
        return 0;

    int nTokenCount = 0;
    const char *pRun = rLine.GetBuffer();


    while( *pRun )
    {
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        if( ! *pRun )
            break;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '`' )
            {
                do pRun++; while( *pRun && *pRun != '`' );
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '\'' )
            {
                do pRun++; while( *pRun && *pRun != '\'' );
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '"' )
            {
                do pRun++; while( *pRun && *pRun != '"' );
                if( *pRun )
                    pRun++;
            }
            else
                pRun++;
        }
        nTokenCount++;
    }

    return nTokenCount;
}

String WhitespaceToSpace( const String& rLine, BOOL bProtect )
{
    int nLen = rLine.Len();
    if( ! nLen )
        return String();

    sal_Unicode *pBuffer = (sal_Unicode*)alloca( sizeof(sal_Unicode)*(nLen + 1) );
    const sal_Unicode *pRun = rLine.GetBuffer();
    sal_Unicode *pLeap = pBuffer;

    while( *pRun )
    {
        if( *pRun && isSpace( *pRun ) )
        {
            *pLeap = ' ';
            pLeap++;
            pRun++;
        }
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( *pRun )
                    pRun++;
            }
            else if( bProtect && *pRun == '`' )
                CopyUntil( pLeap, pRun, '`', TRUE );
            else if( bProtect && *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'', TRUE );
            else if( bProtect && *pRun == '"' )
                CopyUntil( pLeap, pRun, '"', TRUE );
            else
            {
                *pLeap = *pRun;
                *pLeap++;
                *pRun++;
            }
        }
    }

    *pLeap = 0;

    // there might be a space at beginning or end
    pLeap--;
    if( *pLeap == ' ' )
        *pLeap = 0;

    String aRet( *pBuffer == ' ' ? pBuffer+1 : pBuffer );
    return aRet;
}

ByteString WhitespaceToSpace( const ByteString& rLine, BOOL bProtect )
{
    int nLen = rLine.Len();
    if( ! nLen )
        return ByteString();

    char *pBuffer = (char*)alloca( nLen + 1 );
    const char *pRun = rLine.GetBuffer();
    char *pLeap = pBuffer;

    while( *pRun )
    {
        if( *pRun && isSpace( *pRun ) )
        {
            *pLeap = ' ';
            pLeap++;
            pRun++;
        }
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( *pRun )
                    pRun++;
            }
            else if( bProtect && *pRun == '`' )
                CopyUntil( pLeap, pRun, '`', TRUE );
            else if( bProtect && *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'', TRUE );
            else if( bProtect && *pRun == '"' )
                CopyUntil( pLeap, pRun, '"', TRUE );
            else
            {
                *pLeap = *pRun;
                *pLeap++;
                *pRun++;
            }
        }
    }

    *pLeap = 0;

    // there might be a space at beginning or end
    pLeap--;
    if( *pLeap == ' ' )
        *pLeap = 0;

    ByteString aRet( *pBuffer == ' ' ? pBuffer+1 : pBuffer );
    return aRet;
}

} // namespace
