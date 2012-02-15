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

#include "tools/svwin.h"
#include "rtl/tencinfo.h"
#include "vcl/svapp.hxx"

#include "win/saldata.hxx"

// =======================================================================

rtl_TextEncoding ImplSalGetSystemEncoding()
{
    static UINT nOldAnsiCodePage = 0;
    static rtl_TextEncoding eEncoding = RTL_TEXTENCODING_MS_1252;

    UINT nAnsiCodePage = GetACP();
    if ( nAnsiCodePage != nOldAnsiCodePage )
    {
        rtl_TextEncoding nEnc
            = rtl_getTextEncodingFromWindowsCodePage(nAnsiCodePage);
        if (nEnc != RTL_TEXTENCODING_DONTKNOW)
            eEncoding = nEnc;
    }

    return eEncoding;
}

// -----------------------------------------------------------------------

ByteString ImplSalGetWinAnsiString( const UniString& rStr, sal_Bool bFileName )
{
    rtl_TextEncoding eEncoding = ImplSalGetSystemEncoding();
    if ( bFileName )
    {
        return ByteString( rStr, eEncoding,
                           RTL_UNICODETOTEXT_FLAGS_UNDEFINED_UNDERLINE |
                           RTL_UNICODETOTEXT_FLAGS_INVALID_UNDERLINE |
                           RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE |
                           RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR |
                           RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 );
    }
    else
    {
        return ByteString( rStr, eEncoding,
                           RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |
                           RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT |
                           RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE |
                           RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR |
                           RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 );
    }
}

// -----------------------------------------------------------------------

UniString ImplSalGetUniString( const sal_Char* pStr, xub_StrLen nLen )
{
    return UniString( pStr, nLen, ImplSalGetSystemEncoding(),
                      RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
                      RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                      RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT );
}

// =======================================================================

int ImplSalWICompareAscii( const wchar_t* pStr1, const char* pStr2 )
{
    int         nRet;
    wchar_t     c1;
    char       c2;
    do
    {
        // Ist das Zeichen zwischen 'A' und 'Z' dann umwandeln
        c1 = *pStr1;
        c2 = *pStr2;
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = ((sal_Int32)c1)-((sal_Int32)((unsigned char)c2));
        if ( nRet != 0 )
            break;

        pStr1++;
        pStr2++;
    }
    while ( c2 );

    return nRet;
}

// =======================================================================

LONG ImplSetWindowLong( HWND hWnd, int nIndex, DWORD dwNewLong )
{
    return SetWindowLongW( hWnd, nIndex, dwNewLong );
}

// -----------------------------------------------------------------------

LONG ImplGetWindowLong( HWND hWnd, int nIndex )
{
    return GetWindowLongW( hWnd, nIndex );
}

// -----------------------------------------------------------------------

BOOL ImplPostMessage( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    return PostMessageW( hWnd, nMsg, wParam, lParam );
}

// -----------------------------------------------------------------------

BOOL ImplSendMessage( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL bRet = SendMessageW( hWnd, nMsg, wParam, lParam );
    return bRet;
}

// -----------------------------------------------------------------------

BOOL ImplGetMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax )
{
    return GetMessageW( lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax );
}

// -----------------------------------------------------------------------

BOOL ImplPeekMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg )
{
    return PeekMessageW( lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg );
}

// -----------------------------------------------------------------------

LONG ImplDispatchMessage( CONST MSG *lpMsg )
{
    return DispatchMessageW( lpMsg );
}

