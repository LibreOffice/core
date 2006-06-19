/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlaccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:14:21 $
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


#ifndef _CONTROLACCESS_HXX_
#define _CONTROLACCESS_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include <windows.h>

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

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
