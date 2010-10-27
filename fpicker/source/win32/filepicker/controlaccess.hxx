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


#ifndef _CONTROLACCESS_HXX_
#define _CONTROLACCESS_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

typedef void ( SAL_CALL *CTRL_SETVALUE_FUNCTION_T)( HWND, const ::com::sun::star::uno::Any&, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&, sal_Int16 );
typedef ::com::sun::star::uno::Any ( SAL_CALL *CTRL_GETVALUE_FUNCTION_T )( HWND );

// the currently supported control classes
enum CTRL_CLASS
{
    UNKNOWN = 0,
    CHECKBOX,
    LISTBOX,
    PUSHBUTTON
};

// returns the class of a control
CTRL_CLASS SAL_CALL GetCtrlClass( HWND hwndCtrl );

// returns a pointer to a setValue function based on the control class
// and the control action, if no function was found NULL will be returned
CTRL_SETVALUE_FUNCTION_T SAL_CALL GetCtrlSetValueFunction( CTRL_CLASS aCtrlClass, sal_Int16 aCtrlAction );

// returns a pointer to a getValue function based on the control class
// and the control action, if no function was found NULL will be returned
CTRL_GETVALUE_FUNCTION_T SAL_CALL GetCtrlGetValueFunction( CTRL_CLASS aCtrlClass, sal_Int16 aCtrlAction );

// translates a CommonFilePickerElementId as defined in CommonFilePickerElementIds.idl
// to a control id that is valid for the FileOpen dialog under windows as defined
// in dlgs.h in the Windows Platform SDK
int SAL_CALL CommonFilePickerCtrlIdToWinFileOpenCtrlId( sal_Int16 aControlId );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
