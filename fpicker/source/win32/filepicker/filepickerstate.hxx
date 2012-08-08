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

#ifndef _FILEPICKERSTATE_HXX_
#define _FILEPICKERSTATE_HXX_

#include <sal/types.h>
#include "controlcommand.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
//---------------------------------------------
//
//---------------------------------------------

class CControlCommand;
class CFileOpenDialog;

//---------------------------------------------
// declaration
//---------------------------------------------

class CFilePickerState
{
public:
    virtual ~CFilePickerState( );

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const ::com::sun::star::uno::Any& aValue ) = 0;

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction ) = 0;

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable ) = 0;

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel ) = 0;

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 aControlId ) = 0;

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles( CFileOpenDialog* aFileOpenDialog ) = 0;

    virtual rtl::OUString SAL_CALL getDisplayDirectory( CFileOpenDialog* aFileOpenDialog ) = 0;
};

//---------------------------------------------
// this class is not thread-safe
//---------------------------------------------

class CNonExecuteFilePickerState : public CFilePickerState
{
public:
    CNonExecuteFilePickerState( );

    virtual ~CNonExecuteFilePickerState( );

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const ::com::sun::star::uno::Any& aValue );

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction );

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable );

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel );

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 aControlId );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles( CFileOpenDialog* aFileOpenDialog );

    virtual rtl::OUString SAL_CALL getDisplayDirectory( CFileOpenDialog* aFileOpenDialog );

    void SAL_CALL reset( );

    CControlCommand* SAL_CALL getControlCommand( ) const;

protected:
    void SAL_CALL addControlCommand( CControlCommand* aControlCommand );

private:
    CControlCommand* m_FirstControlCommand;
};

//---------------------------------------------
// this class is not thread-safe
//---------------------------------------------

class CExecuteFilePickerState : public CFilePickerState
{
public:
    CExecuteFilePickerState( HWND hwndDlg = NULL );

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const ::com::sun::star::uno::Any& aValue );

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction );

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable );

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel );

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 aControlId );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles( CFileOpenDialog* aFileOpenDialog );

    virtual rtl::OUString SAL_CALL getDisplayDirectory( CFileOpenDialog* aFileOpenDialog );

    void SAL_CALL initFilePickerControls( CControlCommand* firstControlCommand );

    void SAL_CALL cacheControlState( HWND hwndControl, CFilePickerState* aFilePickerState );

    void SAL_CALL setHwnd( HWND hwndDlg );

private:

    inline sal_Bool SAL_CALL IsListboxControl( HWND hwndControl ) const;

    inline sal_Int16 SAL_CALL ListboxIdToListboxLabelId( sal_Int16 aListboxId ) const;

    inline HWND SAL_CALL GetListboxLabelItem( sal_Int16 aControlId ) const;

    // returns a hwnd for a control if successful
    // if bIncludeStdCtrls is false, the standard file dialog
    // controls like OK button, etc. will not be considered
    // the function return 0 on failure
    HWND SAL_CALL GetHwndDlgItem( sal_Int16 aControlId, sal_Bool bIncludeStdCtrls = sal_True ) const;

    HWND m_hwndDlg;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
