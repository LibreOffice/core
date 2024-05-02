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

#include <strhelper.hxx>

namespace  {

bool isSpace( sal_Unicode cChar )
{
    return
        cChar == ' '    || cChar == '\t'    ||
        cChar == '\r'   || cChar == '\n'    ||
        cChar == 0x0c   || cChar == 0x0b;
}

bool isProtect( sal_Unicode cChar )
{
    return cChar == '`' || cChar == '\'' || cChar == '"';
}

void CopyUntil( char*& pTo, const char*& pFrom, char cUntil, bool bIncludeUntil = false )
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

void CopyUntil( sal_Unicode*& pTo, const sal_Unicode*& pFrom, sal_Unicode cUntil, bool bIncludeUntil = false )
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
        if( *pFrom )
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

}

namespace psp {

OUString GetCommandLineToken( int nToken, const OUString& rLine )
{
    sal_Int32 nLen = rLine.getLength();
    if( ! nLen )
        return OUString();

    int nActualToken = 0;
    sal_Unicode* pBuffer = static_cast<sal_Unicode*>(alloca( sizeof(sal_Unicode)*( nLen + 1 ) ));
    const sal_Unicode* pRun = rLine.getStr();
    sal_Unicode* pLeap = nullptr;

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

    assert(pLeap && "otherwise would early return");
    *pLeap = 0;

    return OUString(pBuffer);
}

OString GetCommandLineToken(int nToken, const OString& rLine)
{
    sal_Int32 nLen = rLine.getLength();
    if (!nLen)
        return rLine;

    int nActualToken = 0;
    char* pBuffer = static_cast<char*>(alloca( nLen + 1 ));
    const char* pRun = rLine.getStr();
    char* pLeap = nullptr;

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

    assert(pLeap && "otherwise would early return");
    *pLeap = 0;

    return pBuffer;
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

OUString WhitespaceToSpace( std::u16string_view rLine )
{
    size_t nLen = rLine.size();
    if( ! nLen )
        return OUString();

    sal_Unicode *pBuffer = static_cast<sal_Unicode*>(alloca( sizeof(sal_Unicode)*(nLen + 1) ));
    const sal_Unicode *pRun = rLine.data();
    const sal_Unicode * const pEnd = rLine.data() + rLine.size();
    sal_Unicode *pLeap = pBuffer;

    while( pRun != pEnd )
    {
        if( pRun != pEnd && isSpace( *pRun ) )
        {
            *pLeap = ' ';
            pLeap++;
            pRun++;
        }
        while( pRun != pEnd && isSpace( *pRun ) )
            pRun++;
        while( pRun != pEnd && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( pRun != pEnd )
                    pRun++;
            }
            else if( *pRun == '`' )
                CopyUntil( pLeap, pRun, '`', true );
            else if( *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'', true );
            else if( *pRun == '"' )
                CopyUntil( pLeap, pRun, '"', true );
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
    if (pLeap > pBuffer)
    {
        pLeap--;
        if( *pLeap == ' ' )
            *pLeap = 0;
    }

    return OUString(*pBuffer == ' ' ? pBuffer+1 : pBuffer);
}

OString WhitespaceToSpace(std::string_view rLine)
{
    size_t nLen = rLine.size();
    if (!nLen)
        return OString();

    char *pBuffer = static_cast<char*>(alloca( nLen + 1 ));
    const char *pRun = rLine.data();
    const char * const pEnd = rLine.data() + rLine.size();
    char *pLeap = pBuffer;

    while( pRun != pEnd )
    {
        if( pRun != pEnd && isSpace( *pRun ) )
        {
            *pLeap = ' ';
            pLeap++;
            pRun++;
        }
        while( pRun != pEnd && isSpace( *pRun ) )
            pRun++;
        while( pRun != pEnd && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( pRun != pEnd )
                    pRun++;
            }
            else if( *pRun == '`' )
                CopyUntil( pLeap, pRun, '`', true );
            else if( *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'', true );
            else if( *pRun == '"' )
                CopyUntil( pLeap, pRun, '"', true );
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
    assert(pLeap > pBuffer);
    pLeap--;
#if defined(__GNUC__) && (__GNUC__ == 12 || __GNUC__ == 13)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    if( *pLeap == ' ' )
        *pLeap = 0;
#if defined(__GNUC__) && (__GNUC__ == 12 || __GNUC__ == 13)
#pragma GCC diagnostic pop
#endif
    return *pBuffer == ' ' ? pBuffer+1 : pBuffer;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
