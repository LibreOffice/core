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
