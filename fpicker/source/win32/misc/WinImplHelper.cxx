/*************************************************************************
 *
 *  $RCSfile: WinImplHelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:05:07 $
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

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _AUTO_BUFFER_HXX_
#include "AutoBuffer.hxx"
#endif

#ifndef _WINIMPLHELPER_HXX_
#include "WinImplHelper.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

//------------------------------------------------------------
// namespace directives
//------------------------------------------------------------

using rtl::OUString;
using rtl::OUStringBuffer;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;

//------------------------------------------------------------
//
//------------------------------------------------------------

const rtl::OUString TILDE = OUString::createFromAscii( "~" );
const sal_Unicode   TILDE_SIGN = L'~';
const rtl::OUString AMPERSAND = OUString::createFromAscii( "&" );
const sal_Unicode   AMPERSAND_SIGN = L'&';

//------------------------------------------------------------
// OS NAME          Platform                    Major   Minor
//
// Windows NT 3.51  VER_PLATFORM_WIN32_NT       3       51
// Windows NT 4.0   VER_PLATFORM_WIN32_NT       4       0
// Windows 2000     VER_PLATFORM_WIN32_NT       5       0
// Windows XP       VER_PLATFORM_WIN32_NT       5       1
// Windows 95       VER_PLATFORM_WIN32_WINDOWS  4       0
// Windows 98       VER_PLATFORM_WIN32_WINDOWS  4       10
// Windows ME       VER_PLATFORM_WIN32_WINDOWS  4       90
//------------------------------------------------------------

bool SAL_CALL IsWindowsVersion(unsigned int PlatformId, unsigned int MajorVersion, int MinorVersion = -1)
{
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    if(!GetVersionEx(&osvi))
        return false;

    bool bRet = (PlatformId == osvi.dwPlatformId) &&
                (MajorVersion == osvi.dwMajorVersion);

    if (MinorVersion > -1)
        bRet = bRet && (MinorVersion == osvi.dwMinorVersion);

    return bRet;
}

//------------------------------------------------------------
// determine if we are running under Win2000
//------------------------------------------------------------

bool SAL_CALL IsWindows2000()
{
    return IsWindowsVersion(VER_PLATFORM_WIN32_NT, 5, 0);
}

//------------------------------------------------------------
//
//------------------------------------------------------------

bool SAL_CALL IsWindowsXP()
{
    return IsWindowsVersion(VER_PLATFORM_WIN32_NT, 5, 1);
}

//------------------------------------------------------------
//
//------------------------------------------------------------

bool SAL_CALL IsWindows98()
{
    return IsWindowsVersion(VER_PLATFORM_WIN32_WINDOWS, 4, 10);
}

//------------------------------------------------------------
//
//------------------------------------------------------------

bool SAL_CALL IsWindowsME()
{
    return  IsWindowsVersion(VER_PLATFORM_WIN32_WINDOWS, 4, 90);
}

//------------------------------------------------------------
//
//------------------------------------------------------------

bool SAL_CALL IsWindows2000Platform()
{
    return IsWindowsVersion(VER_PLATFORM_WIN32_NT, 5);
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void SAL_CALL ListboxAddString( HWND hwnd, const OUString& aString )
{
    LRESULT rc = SendMessageW(
        hwnd, CB_ADDSTRING, 0, reinterpret_cast< LPARAM >(aString.getStr( )) );

    OSL_ASSERT( (CB_ERR != rc) && (CB_ERRSPACE != rc) );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

OUString SAL_CALL ListboxGetString( HWND hwnd, sal_Int32 aPosition )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    OUString aString;

    LRESULT lItem =
        SendMessageW( hwnd, CB_GETLBTEXTLEN, aPosition, 0 );

    if ( (CB_ERR != lItem) && (lItem > 0) )
    {
        // message returns the len of a combobox item
        // without trailing '\0' that's why += 1
        lItem++;

        CAutoUnicodeBuffer aBuff( lItem );

        LRESULT lRet =
            SendMessageW(
                hwnd, CB_GETLBTEXT, aPosition,
                reinterpret_cast<LPARAM>(&aBuff) );

        OSL_ASSERT( lRet != CB_ERR );

        if ( CB_ERR != lRet )
            aString = OUString( aBuff, lRet );
    }

    return aString;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void SAL_CALL ListboxAddItem( HWND hwnd, const Any& aItem, const Reference< XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( IllegalArgumentException )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    if ( !aItem.hasValue( ) ||
         aItem.getValueType( ) != getCppuType((OUString*)0) )
         throw IllegalArgumentException(
            OUString::createFromAscii( "invalid value type or any has no value" ),
            rXInterface,
            aArgPos );

    OUString cbItem;
    aItem >>= cbItem;

    ListboxAddString( hwnd, cbItem );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void SAL_CALL ListboxAddItems( HWND hwnd, const Any& aItemList, const Reference< XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( IllegalArgumentException )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    if ( !aItemList.hasValue( ) ||
         aItemList.getValueType( ) != getCppuType((Sequence<OUString>*)0) )
         throw IllegalArgumentException(
            OUString::createFromAscii( "invalid value type or any has no value" ),
            rXInterface,
            aArgPos );

    Sequence< OUString > aStringList;
    aItemList >>= aStringList;

    sal_Int32 nItemCount = aStringList.getLength( );
    for( sal_Int32 i = 0; i < nItemCount; i++ )
    {
        ListboxAddString( hwnd, aStringList[i] );
    }
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void SAL_CALL ListboxDeleteItem( HWND hwnd, const Any& aPosition, const Reference< XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( IllegalArgumentException )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    if ( !aPosition.hasValue( ) ||
         ( (aPosition.getValueType( ) != getCppuType((sal_Int32*)0)) &&
           (aPosition.getValueType( ) != getCppuType((sal_Int16*)0)) &&
           (aPosition.getValueType( ) != getCppuType((sal_Int8*)0)) ) )
         throw IllegalArgumentException(
            OUString::createFromAscii( "invalid value type or any has no value" ),
            rXInterface,
            aArgPos );

    sal_Int32 nPos;
    aPosition >>= nPos;

    LRESULT lRet = SendMessage( hwnd, CB_DELETESTRING, nPos, 0 );

    // if the return value is CB_ERR the given
    // index was not correct
    if ( CB_ERR == lRet )
        throw IllegalArgumentException(
            OUString::createFromAscii( "inavlid item position" ),
            rXInterface,
            aArgPos );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void SAL_CALL ListboxDeleteItems( HWND hwnd, const Any& /*unused*/, const Reference< XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( IllegalArgumentException )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    LRESULT lRet = 0;

    do
    {
        // the return value on success is the number
        // of remaining elements in the listbox
        lRet = SendMessageW( hwnd, CB_DELETESTRING, 0, 0 );
    }
    while ( (lRet != CB_ERR) && (lRet > 0) );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void SAL_CALL ListboxSetSelectedItem( HWND hwnd, const Any& aPosition, const Reference< XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( IllegalArgumentException )
{
    OSL_ASSERT( IsWindow( hwnd ) );

     if ( !aPosition.hasValue( ) ||
         ( (aPosition.getValueType( ) != getCppuType((sal_Int32*)0)) &&
           (aPosition.getValueType( ) != getCppuType((sal_Int16*)0)) &&
           (aPosition.getValueType( ) != getCppuType((sal_Int8*)0)) ) )
         throw IllegalArgumentException(
            OUString::createFromAscii( "invalid value type or any has no value" ),
            rXInterface,
            aArgPos );

    sal_Int32 nPos;
    aPosition >>= nPos;

    if ( nPos < -1 )
        throw IllegalArgumentException(
            OUString::createFromAscii("invalid index"),
            rXInterface,
            aArgPos );

    LRESULT lRet = SendMessageW( hwnd, CB_SETCURSEL, nPos, 0 );

    if ( (CB_ERR == lRet) && (-1 != nPos) )
        throw IllegalArgumentException(
            OUString::createFromAscii("invalid index"),
            rXInterface,
            aArgPos );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

Any SAL_CALL ListboxGetItems( HWND hwnd )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    LRESULT nItemCount = SendMessageW( hwnd, CB_GETCOUNT, 0, 0 );

    Sequence< OUString > aItemList;

    if ( CB_ERR != nItemCount )
    {
        aItemList.realloc( nItemCount );

        for ( sal_Int32 i = 0; i < nItemCount; i++ )
        {
            aItemList[i] = ListboxGetString( hwnd, i );
        }
    }

    Any aAny;
    aAny <<= aItemList;

    return aAny;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

Any SAL_CALL ListboxGetSelectedItem( HWND hwnd )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    LRESULT idxItem = SendMessageW( hwnd, CB_GETCURSEL, 0, 0 );

    Any aAny;
    aAny <<= ListboxGetString( hwnd, idxItem );

    return aAny;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

Any SAL_CALL ListboxGetSelectedItemIndex( HWND hwnd )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    LRESULT idxItem = SendMessageW( hwnd, CB_GETCURSEL, 0, 0 );

    Any aAny;
    aAny <<= static_cast< sal_Int32 >( idxItem );

    return aAny;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

Any SAL_CALL CheckboxGetState( HWND hwnd )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    LRESULT lChkState = SendMessageW( hwnd, BM_GETCHECK, 0, 0 );
    sal_Bool bChkState = (lChkState == BST_CHECKED) ? sal_True : sal_False;
    Any aAny;
    aAny.setValue( &bChkState, getCppuType((sal_Bool*)0) );
    return aAny;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void SAL_CALL CheckboxSetState(
    HWND hwnd, const ::com::sun::star::uno::Any& aState, const Reference< XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( IllegalArgumentException )
{
    OSL_ASSERT( IsWindow( hwnd ) );

    if ( !aState.hasValue( ) ||
         aState.getValueType( ) != getCppuType((sal_Bool*)0) )
         throw IllegalArgumentException(
            OUString::createFromAscii( "invalid value type or any has no value" ),
            rXInterface,
            aArgPos );

    sal_Bool bCheckState = *reinterpret_cast< const sal_Bool* >( aState.getValue( ) );
    WPARAM wParam = bCheckState ? BST_CHECKED : BST_UNCHECKED;
    SendMessageW( hwnd, BM_SETCHECK, wParam, 0 );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

sal_uInt32 SAL_CALL _wcslenex( const sal_Unicode* pStr )
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

//------------------------------------------------------------
//
//------------------------------------------------------------

void Replace( const OUString& aLabel, sal_Unicode OldChar, sal_Unicode NewChar, OUStringBuffer& aBuffer )
{
    OSL_ASSERT( aLabel.getLength( ) );
    OSL_ASSERT( aBuffer.getCapacity( ) >= (aLabel.getLength( )) );

    sal_Int32 i = 0;
    const sal_Unicode* pCurrent  = aLabel.getStr( );
    const sal_Unicode* pNext     = aLabel.getStr( ) + 1;
    const sal_Unicode* pEnd      = aLabel.getStr( ) + aLabel.getLength( );

    while( pCurrent < pEnd )
    {
        OSL_ASSERT( pNext <= pEnd );
        OSL_ASSERT( (i >= 0) && (i < aBuffer.getCapacity( )) );

        if ( OldChar == *pCurrent )
        {
            if ( OldChar == *pNext )
            {
                // two OldChars in line will
                // be replaced by one
                // e.g. ~~ -> ~
                aBuffer.insert( i, *pCurrent );

                // skip the next one
                pCurrent++;
                pNext++;
            }
            else
            {
                // one OldChar will be replace
                // by NexChar
                aBuffer.insert( i, NewChar );
            }
         }
         else if ( *pCurrent == NewChar )
         {
            // a NewChar will be replaced by
             // two NewChars
             // e.g. & -> &&
            aBuffer.insert( i++, *pCurrent );
            aBuffer.insert( i, *pCurrent );
         }
         else
         {
            aBuffer.insert( i, *pCurrent );
         }

         pCurrent++;
         pNext++;
         i++;
    }
}

//------------------------------------------------------------
// converts a soffice label to a windows label
// the following rules for character replacements
// will be done:
// '~'  -> '&'
// '~~' -> '~'
// '&'  -> '&&'
//------------------------------------------------------------

OUString SOfficeToWindowsLabel( const rtl::OUString& aSOLabel )
{
    OUString aWinLabel = aSOLabel;

    if ( (aWinLabel.indexOf( TILDE ) > -1) || (aWinLabel.indexOf( AMPERSAND ) > -1) )
    {
        sal_Int32 nStrLen = aWinLabel.getLength( );

        // in the worst case the new string is
        // doubled in length, maybe some waste
        // of memory but how long is a label
        // normaly(?)
        rtl::OUStringBuffer aBuffer( nStrLen * 2 );

        Replace( aWinLabel, TILDE_SIGN, AMPERSAND_SIGN, aBuffer );

        aWinLabel = aBuffer.makeStringAndClear( );
    }

    return aWinLabel;
}

//------------------------------------------------------------
// converts a windows label to a soffice label
// the following rules for character replacements
// will be done:
// '&'  -> '~'
// '&&' -> '&'
// '~'  -> '~~'
//------------------------------------------------------------

OUString WindowsToSOfficeLabel( const rtl::OUString& aWinLabel )
{
    OUString aSOLabel = aWinLabel;

    if ( (aSOLabel.indexOf( TILDE ) > -1) || (aSOLabel.indexOf( AMPERSAND ) > -1) )
    {
        sal_Int32 nStrLen = aSOLabel.getLength( );

        // in the worst case the new string is
        // doubled in length, maybe some waste
        // of memory but how long is a label
        // normaly(?)
        rtl::OUStringBuffer aBuffer( nStrLen * 2 );

        Replace( aSOLabel, AMPERSAND_SIGN, TILDE_SIGN, aBuffer );

        aSOLabel = aBuffer.makeStringAndClear( );
    }

    return aSOLabel;
}

