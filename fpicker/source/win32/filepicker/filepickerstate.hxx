/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _FILEPICKERSTATE_HXX_
#define _FILEPICKERSTATE_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

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
