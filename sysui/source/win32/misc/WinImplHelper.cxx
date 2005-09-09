/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WinImplHelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:53:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
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

#include <systools/win32/user9x.h>

//------------------------------------------------------------
// namespace directives
//------------------------------------------------------------

using rtl::OUString;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;

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
