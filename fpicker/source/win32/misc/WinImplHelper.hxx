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


#ifndef INCLUDED_FPICKER_SOURCE_WIN32_MISC_WINIMPLHELPER_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_MISC_WINIMPLHELPER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

bool SAL_CALL IsWindowsVistaOrNewer();

// set actions
void SAL_CALL ListboxAddItem(
    HWND hwnd, const ::com::sun::star::uno::Any& aItem, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( ::com::sun::star::lang::IllegalArgumentException );

void SAL_CALL ListboxAddItems(
    HWND hwnd, const ::com::sun::star::uno::Any& aItemList, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( ::com::sun::star::lang:: IllegalArgumentException );

void SAL_CALL ListboxDeleteItem(
    HWND hwnd, const ::com::sun::star::uno::Any& aPosition, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( ::com::sun::star::lang::IllegalArgumentException );

void SAL_CALL ListboxDeleteItems(
    HWND hwnd, const ::com::sun::star::uno::Any& aUnused, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( ::com::sun::star::lang::IllegalArgumentException );

void SAL_CALL ListboxSetSelectedItem(
    HWND hwnd, const ::com::sun::star::uno::Any& aPosition, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( ::com::sun::star::lang::IllegalArgumentException );

// get actions
::com::sun::star::uno::Any SAL_CALL ListboxGetItems( HWND hwnd );
::com::sun::star::uno::Any SAL_CALL ListboxGetSelectedItem( HWND hwnd );
::com::sun::star::uno::Any SAL_CALL ListboxGetSelectedItemIndex( HWND hwnd );

// checkbox helper functions
::com::sun::star::uno::Any SAL_CALL CheckboxGetState( HWND hwnd );

void SAL_CALL CheckboxSetState(
    HWND hwnd, const ::com::sun::star::uno::Any& aState, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rXInterface, sal_Int16 aArgPos )
    throw( ::com::sun::star::lang::IllegalArgumentException );

// calculates the length of '\0' separated and '\0\0'
// ending strings used in some Win32 functions
// e.g. Filter\0*.txt\0\0
// the returned length excludes the last '\0'
sal_uInt32 SAL_CALL _wcslenex( const sal_Unicode* pStr );


// converts a soffice label to a windows label
// the following rules for character replacements
// will be done:
// '~'  -> '&'
// '~~' -> '~'
// '&'  -> '&&'
OUString SOfficeToWindowsLabel( const OUString& aSOLabel );

// converts a windows label to a soffice label
// the following rules for character replacements
// will be done:
// '&'  -> '~'
// '&&' -> '&'
// '~'  -> '~~'
OUString WindowsToSOfficeLabel( const OUString& aWinLabel );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
