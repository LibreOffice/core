/*************************************************************************
 *
 *  $RCSfile: saldata.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:49 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#define _SV_SALDATA_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

// =======================================================================

rtl_TextEncoding ImplSalGetSystemEncoding()
{
    static UINT nOldAnsiCodePage = 0;
    static rtl_TextEncoding eEncoding = RTL_TEXTENCODING_MS_1252;

    UINT nAnsiCodePage = GetACP();
    if ( nAnsiCodePage != nOldAnsiCodePage )
    {
        switch ( nAnsiCodePage )
        {
            case 1252:
                eEncoding = RTL_TEXTENCODING_MS_1252;
                break;
            case 1250:
                eEncoding = RTL_TEXTENCODING_MS_1250;
                break;
            case 1251:
                eEncoding = RTL_TEXTENCODING_MS_1251;
                break;
            case 1253:
                eEncoding = RTL_TEXTENCODING_MS_1253;
                break;
            case 1254:
                eEncoding = RTL_TEXTENCODING_MS_1254;
                break;
            case 1255:
                eEncoding = RTL_TEXTENCODING_MS_1255;
                break;
            case 1256:
                eEncoding = RTL_TEXTENCODING_MS_1256;
                break;
            case 1257:
                eEncoding = RTL_TEXTENCODING_MS_1257;
                break;
            case 1258:
                eEncoding = RTL_TEXTENCODING_MS_1258;
                break;
            case 874:
                eEncoding = RTL_TEXTENCODING_MS_874;
                break;
            case 932:
                eEncoding = RTL_TEXTENCODING_MS_932;
                break;
            case 936:
                eEncoding = RTL_TEXTENCODING_MS_936;
                break;
            case 949:
                eEncoding = RTL_TEXTENCODING_MS_949;
                break;
            case 950:
                eEncoding = RTL_TEXTENCODING_MS_950;
                break;
//            case 1381:
//                eEncoding = RTL_TEXTENCODING_MS_1381;
//                break;
        }
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
    if ( aSalShlData.mbWNT )
        return SendMessageW( hWnd, nMsg, wParam, lParam );
    else
        return SendMessageA( hWnd, nMsg, wParam, lParam );
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

