/*************************************************************************
 *
 *  $RCSfile: WinImplHelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-04-26 08:56:59 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_FILEPICKERELEMENTID_HPP_
#include <com/sun/star/ui/FilePickerElementID.hpp>
#endif

#ifndef _AUTO_BUFFER_HXX_
#include "AutoBuffer.hxx"
#endif

#ifndef _WINIMPLHELPER_HXX_
#include "WinImplHelper.hxx"
#endif

#include <systools/win32/user9x.h>

//------------------------------------------------------------
// namespace directives
//------------------------------------------------------------

using rtl::OUString;

//------------------------------------------------------------
// determine if we are running under Win2000
//------------------------------------------------------------

sal_Bool SAL_CALL IsWin2000( )
{
    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx;
    sal_Bool bRet = sal_False;

    osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );
    bOsVersionInfoEx = GetVersionEx( ( OSVERSIONINFO* )&osvi );
    if( !bOsVersionInfoEx )
    {
        // if OSVERSIONINFOEX doesn't work
        osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
        if( !GetVersionEx( ( OSVERSIONINFO* )&osvi ) )
            return sal_False;
    }

    if( ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId ) && ( osvi.dwMajorVersion >= 5 ) )
        bRet = sal_True;

    return bRet;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

OUString SAL_CALL getCurrentComboboxItem( HWND hwndCbo )
{
    LRESULT currentIndex = SendMessageW( hwndCbo, CB_GETCURSEL, 0, 0 );

    OUString aItem;

    if ( CB_ERR != currentIndex )
    {
        LRESULT lenItem =
            SendMessageW( hwndCbo, CB_GETLBTEXTLEN, ( WPARAM ) currentIndex, 0 );

        if ( (CB_ERR != lenItem) && (lenItem > 0) )
        {
            // message returns the len of a combobox item
            // without trailing '\0' that's why + 1
            lenItem++;

            CAutoUnicodeBuffer aBuff( lenItem );

            LRESULT lRet =
                SendMessageW(
                    hwndCbo,
                    CB_GETLBTEXT,
                    ( WPARAM ) currentIndex,
                    ( LPARAM ) &aBuff );
            OSL_ASSERT( lRet == ( lenItem - 1 ) );

            aItem = OUString( aBuff, lenItem );
        }
    }

    return aItem;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

sal_Bool SAL_CALL addComboboxItem( HWND hwndCbo, const rtl::OUString& aItem )
{
    LRESULT rc = SendMessageW(
        hwndCbo, CB_ADDSTRING, 0, reinterpret_cast< LPARAM >(aItem.getStr( )) );
    return ( (CB_ERR != rc) && (CB_ERRSPACE != rc) );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

sal_Bool SAL_CALL getCheckboxState( HWND hwndCbx )
{
    LRESULT cbxState = SendMessageW( hwndCbx, BM_GETCHECK, 0, 0 );
    return ( cbxState == BST_CHECKED ) ? sal_True : sal_False;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void SAL_CALL setCheckboxState( HWND hwndCbx, sal_Bool bCheckState )
{
    WPARAM wParam = bCheckState ? BST_CHECKED : BST_UNCHECKED;
    SendMessageW( hwndCbx, BM_SETCHECK, wParam, 0 );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

sal_uInt32 _wcslenex( const sal_Unicode* pStr )
{
    if ( !pStr )
        return 0;

    const sal_Unicode* pTemp = pStr;
    sal_uInt32 strLen = 0;
    while( *pTemp || *(pTemp + 1) )
    {
        pTemp++;
        strLen++;
    }

    return strLen;
}