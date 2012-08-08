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


#ifndef _CONTROLACCESS_HXX_
#define _CONTROLACCESS_HXX_

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
