/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

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

String WhitespaceToSpace( const String& rLine, sal_Bool bProtect )
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

    String aRet( *pBuffer == ' ' ? pBuffer+1 : pBuffer );
    return aRet;
}

ByteString WhitespaceToSpace( const ByteString& rLine, sal_Bool bProtect )
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

    ByteString aRet( *pBuffer == ' ' ? pBuffer+1 : pBuffer );
    return aRet;
}

} // namespace
