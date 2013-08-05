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


#include "vcl/strhelper.hxx"
#include "sal/alloca.h"

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

OUString GetCommandLineToken( int nToken, const OUString& rLine )
{
    sal_Int32 nLen = rLine.getLength();
    if( ! nLen )
        return OUString();

    int nActualToken = 0;
    sal_Unicode* pBuffer = (sal_Unicode*)alloca( sizeof(sal_Unicode)*( nLen + 1 ) );
    const sal_Unicode* pRun = rLine.getStr();
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

    return OUString(pBuffer);
}

OString GetCommandLineToken(int nToken, const OString& rLine)
{
    sal_Int32 nLen = rLine.getLength();
    if (!nLen)
        return rLine;

    int nActualToken = 0;
    char* pBuffer = (char*)alloca( nLen + 1 );
    const char* pRun = rLine.getStr();
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

    return OString(pBuffer);
}

int GetCommandLineTokenCount(const OUString& rLine)
{
    if (rLine.isEmpty())
        return 0;

    int nTokenCount = 0;
    const sal_Unicode *pRun = rLine.getStr();

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

OUString WhitespaceToSpace( const OUString& rLine, bool bProtect )
{
    sal_Int32 nLen = rLine.getLength();
    if( ! nLen )
        return OUString();

    sal_Unicode *pBuffer = (sal_Unicode*)alloca( sizeof(sal_Unicode)*(nLen + 1) );
    const sal_Unicode *pRun = rLine.getStr();
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
                CopyUntil( pLeap, pRun, '`', sal_True );
            else if( bProtect && *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'', sal_True );
            else if( bProtect && *pRun == '"' )
                CopyUntil( pLeap, pRun, '"', sal_True );
            else
            {
                *pLeap = *pRun;
                ++pLeap;
                ++pRun;
            }
        }
    }

    *pLeap = 0;

    // there might be a space at beginning or end
    pLeap--;
    if( *pLeap == ' ' )
        *pLeap = 0;

    return OUString(*pBuffer == ' ' ? pBuffer+1 : pBuffer);
}

OString WhitespaceToSpace(const OString& rLine, bool bProtect)
{
    sal_Int32 nLen = rLine.getLength();
    if (!nLen)
        return rLine;

    char *pBuffer = (char*)alloca( nLen + 1 );
    const char *pRun = rLine.getStr();
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
                CopyUntil( pLeap, pRun, '`', sal_True );
            else if( bProtect && *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'', sal_True );
            else if( bProtect && *pRun == '"' )
                CopyUntil( pLeap, pRun, '"', sal_True );
            else
            {
                *pLeap = *pRun;
                ++pLeap;
                ++pRun;
            }
        }
    }

    *pLeap = 0;

    // there might be a space at beginning or end
    pLeap--;
    if( *pLeap == ' ' )
        *pLeap = 0;

    return OString(*pBuffer == ' ' ? pBuffer+1 : pBuffer);
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
