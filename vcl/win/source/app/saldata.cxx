/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: saldata.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_vcl.hxx"
#include <tools/svwin.h>
#include "rtl/tencinfo.h"
#include <saldata.hxx>
#include <vcl/svapp.hxx>


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

ByteString ImplSalGetWinAnsiString( const UniString& rStr, BOOL bFileName )
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
    if ( aSalShlData.mbWNT )
        return SetWindowLongW( hWnd, nIndex, dwNewLong );
    else
        return SetWindowLongA( hWnd, nIndex, dwNewLong );
}

// -----------------------------------------------------------------------

LONG ImplGetWindowLong( HWND hWnd, int nIndex )
{
    if ( aSalShlData.mbWNT )
        return GetWindowLongW( hWnd, nIndex );
    else
        return GetWindowLongA( hWnd, nIndex );
}

// -----------------------------------------------------------------------

WIN_BOOL ImplPostMessage( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    if ( aSalShlData.mbWNT )
        return PostMessageW( hWnd, nMsg, wParam, lParam );
    else
        return PostMessageA( hWnd, nMsg, wParam, lParam );
}

// -----------------------------------------------------------------------

WIN_BOOL ImplSendMessage( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    WIN_BOOL bRet;
    if ( aSalShlData.mbWNT )
        bRet = SendMessageW( hWnd, nMsg, wParam, lParam );
    else
        bRet = SendMessageA( hWnd, nMsg, wParam, lParam );

    return bRet;
}

// -----------------------------------------------------------------------

WIN_BOOL ImplGetMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax )
{
    if ( aSalShlData.mbWNT )
        return GetMessageW( lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax );
    else
        return GetMessageA( lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax );
}

// -----------------------------------------------------------------------

WIN_BOOL ImplPeekMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg )
{
    if ( aSalShlData.mbWNT )
        return PeekMessageW( lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg );
    else
        return PeekMessageA( lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg );
}

// -----------------------------------------------------------------------

LONG ImplDispatchMessage( CONST MSG *lpMsg )
{
    if ( aSalShlData.mbWNT )
        return DispatchMessageW( lpMsg );
    else
        return DispatchMessageA( lpMsg );
}

