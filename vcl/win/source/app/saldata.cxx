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

#include <svsys.h>
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

OUString ImplSalGetUniString(const sal_Char* pStr, sal_Int32 const nLen)
{
    return OUString( pStr, (-1 == nLen) ? strlen(pStr) : nLen,
                      ImplSalGetSystemEncoding(),
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
        // change to LowerCase if the char is between 'A' and 'Z'
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

BOOL ImplPostMessage( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    return PostMessageW( hWnd, nMsg, wParam, lParam );
}

// -----------------------------------------------------------------------

BOOL ImplSendMessage( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    return SendMessageW( hWnd, nMsg, wParam, lParam );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
