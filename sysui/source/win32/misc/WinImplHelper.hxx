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




#ifndef _WINIMPLHELPER_HXX_
#define _WINIMPLHELPER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring>
#endif

#include <windows.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

sal_Bool SAL_CALL IsWin2000( );

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

#endif
