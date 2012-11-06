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

#include <osl/diagnose.h>
#include "AutoBuffer.hxx"
#include "WinImplHelper.hxx"
#include <com/sun/star/uno/Sequence.hxx>

#include <windows.h>

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
             "invalid value type or any has no value",
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
             "invalid value type or any has no value",
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
             "invalid value type or any has no value",
             rXInterface,
             aArgPos );

    sal_Int32 nPos;
    aPosition >>= nPos;

    LRESULT lRet = SendMessage( hwnd, CB_DELETESTRING, nPos, 0 );

    // if the return value is CB_ERR the given
    // index was not correct
    if ( CB_ERR == lRet )
        throw IllegalArgumentException(
            "inavlid item position",
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
             "invalid value type or any has no value",
             rXInterface,
             aArgPos );

    sal_Int32 nPos;
    aPosition >>= nPos;

    if ( nPos < -1 )
        throw IllegalArgumentException(
            "invalid index",
            rXInterface,
            aArgPos );

    LRESULT lRet = SendMessageW( hwnd, CB_SETCURSEL, nPos, 0 );

    if ( (CB_ERR == lRet) && (-1 != nPos) )
        throw IllegalArgumentException(
            "invalid index",
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
             "invalid value type or any has no value",
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
