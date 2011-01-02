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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <tchar.h>
#include <sal/macros.h>
#include <osl/diagnose.h>
#include "controlaccess.hxx"
#include "..\misc\WinImplHelper.hxx"

//------------------------------------------------------------
// we are using a table based algorithm to dispatch control
// actions there is one table containing one action table for
// each control class and one action table per control class
// which contains function pointer to control action functions
//------------------------------------------------------------

//------------------------------------------------------------
// namespace directives
//------------------------------------------------------------

using rtl::OUString;

namespace // private
{

    //------------------------------------------------------------
    // table setup
    //------------------------------------------------------------

    CTRL_SETVALUE_FUNCTION_T CheckboxSetValueFunctionTable[] =
    {
        CheckboxSetState
    };
    const size_t SIZE_CHECKBOX_SETVALUE_FUNCTION_TABLE =
        SAL_N_ELEMENTS( CheckboxSetValueFunctionTable );

    CTRL_GETVALUE_FUNCTION_T CheckboxGetValueFunctionTable[] =
    {
        CheckboxGetState
    };
    const size_t SIZE_CHECKBOX_GETVALUE_FUNCTION_TABLE =
        SAL_N_ELEMENTS( CheckboxGetValueFunctionTable );

    CTRL_SETVALUE_FUNCTION_T ListboxSetValueFunctionTable[] =
    {
        NULL,
        ListboxAddItem,
        ListboxAddItems,
        ListboxDeleteItem,
        ListboxDeleteItems,
        ListboxSetSelectedItem
    };
    const size_t SIZE_LISTBOX_SETVALUE_FUNCTION_TABLE =
        SAL_N_ELEMENTS( ListboxSetValueFunctionTable );

    CTRL_GETVALUE_FUNCTION_T ListboxGetValueFunctionTable[] =
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        ListboxGetItems,
        ListboxGetSelectedItem,
        ListboxGetSelectedItemIndex
    };
    const size_t SIZE_LISTBOX_GETVALUE_ACTION_TABLE =
        SAL_N_ELEMENTS( ListboxGetValueFunctionTable );

    struct _ENTRY
    {
        LPVOID lpFunctionTable;
        size_t TableSize;
    };

    // an array of function tables, one for each control class
    _ENTRY CtrlClassSetValueFunctionTable[] =
    {
        { NULL, 0 },
        { CheckboxSetValueFunctionTable, SIZE_CHECKBOX_SETVALUE_FUNCTION_TABLE },
        { ListboxSetValueFunctionTable, SIZE_LISTBOX_SETVALUE_FUNCTION_TABLE },
        { NULL, 0 }
    };

    // an array of function tables, one for each control class
    _ENTRY CtrlClassGetValueFunctionTable[] =
    {
        { NULL, 0 },
        { CheckboxGetValueFunctionTable, SIZE_CHECKBOX_GETVALUE_FUNCTION_TABLE },
        { ListboxGetValueFunctionTable, SIZE_LISTBOX_GETVALUE_ACTION_TABLE },
        { NULL, 0 }
    };

    //------------------------------------------------------------
    //
    //------------------------------------------------------------

    CTRL_SETVALUE_FUNCTION_T SAL_CALL GetCtrlSetValueFunction(
        CTRL_SETVALUE_FUNCTION_T* aCtrlSetValueFunctionTable, size_t aTableSize, sal_Int16 aCtrlAction )
    {
        if ( !aCtrlSetValueFunctionTable ||
             aCtrlAction < 0
             || sal::static_int_cast< sal_uInt16 >(aCtrlAction) >= aTableSize )
            return NULL;

        return aCtrlSetValueFunctionTable[aCtrlAction];
    }

    //------------------------------------------------------------
    //
    //------------------------------------------------------------

    CTRL_GETVALUE_FUNCTION_T SAL_CALL GetCtrlGetValueFunction(
        CTRL_GETVALUE_FUNCTION_T* aCtrlGetValueFunctionTable, size_t aTableSize, sal_Int16 aCtrlAction )
    {
        if ( !aCtrlGetValueFunctionTable ||
             aCtrlAction < 0 ||
             sal::static_int_cast< sal_uInt16 >(aCtrlAction) >= aTableSize )
            return NULL;

        return aCtrlGetValueFunctionTable[aCtrlAction];
    }

    //------------------------------------------------------------
    //
    //------------------------------------------------------------

    inline
    _ENTRY SAL_CALL GetCtrlClassSetValueFunctionTable( CTRL_CLASS aCtrlClass )
    {
        return CtrlClassSetValueFunctionTable[aCtrlClass];
    }

    //------------------------------------------------------------
    //
    //------------------------------------------------------------

    inline
    _ENTRY SAL_CALL GetCtrlClassGetValueFunctionTable( CTRL_CLASS aCtrlClass )
    {
        return CtrlClassGetValueFunctionTable[aCtrlClass];
    }

    int WindowsFileOpenCtrlIds[] =
    {
        0,
        IDOK,       //  PUSHBUTTON_OK
        IDCANCEL,   //  PUSHBUTTON_CANCEL
        cmb1,       //  LISTBOX_FILTER
        0,          //  CONTROL_FILEVIEW
        0,          //  not available in system file picker
        stc2,       //  LISTBOX_FILTER_LABEL
        stc3        //  LISTBOX_FILE_NAME_LABEL
    };
    const int SIZE_WINDOWS_FILEOPEN_CTRL_IDS = SAL_N_ELEMENTS(WindowsFileOpenCtrlIds);

}; // end namespace

//------------------------------------------------------------
//
//------------------------------------------------------------

CTRL_SETVALUE_FUNCTION_T SAL_CALL GetCtrlSetValueFunction( CTRL_CLASS aCtrlClass, sal_Int16 aCtrlAction )
{
    _ENTRY aEntry =
        GetCtrlClassSetValueFunctionTable( aCtrlClass );

    return GetCtrlSetValueFunction(
        reinterpret_cast< CTRL_SETVALUE_FUNCTION_T* >( aEntry.lpFunctionTable ),
        aEntry.TableSize,
        aCtrlAction );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

CTRL_GETVALUE_FUNCTION_T SAL_CALL GetCtrlGetValueFunction( CTRL_CLASS aCtrlClass, sal_Int16 aCtrlAction )
{
    _ENTRY aEntry =
        GetCtrlClassGetValueFunctionTable( aCtrlClass );

    return GetCtrlGetValueFunction(
        reinterpret_cast< CTRL_GETVALUE_FUNCTION_T* >( aEntry.lpFunctionTable ),
        aEntry.TableSize,
        aCtrlAction );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

CTRL_CLASS SAL_CALL GetCtrlClass( HWND hwndCtrl )
{
    CTRL_CLASS aCtrlClass = UNKNOWN;
    const size_t nClassNameSize = 256;
    TCHAR aClassName[nClassNameSize];

    int nRet = GetClassName(hwndCtrl,aClassName,nClassNameSize);
    if (nRet)
    {
        if (0 == _tcsicmp(aClassName,TEXT("button")))
        {
            // button means many things so we have
            // to find out what button it is
            LONG lBtnStyle = GetWindowLong(hwndCtrl,GWL_STYLE);
            if (lBtnStyle & BS_CHECKBOX)
                aCtrlClass = CHECKBOX;
            else if (((lBtnStyle & BS_PUSHBUTTON) == 0) || (lBtnStyle & BS_DEFPUSHBUTTON))
                aCtrlClass = PUSHBUTTON;
        }
        else if (0 == _tcsicmp(aClassName,TEXT("listbox")) ||
                  0 == _tcsicmp(aClassName,TEXT("combobox")))
            aCtrlClass = LISTBOX;
    }

    return aCtrlClass;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

int SAL_CALL CommonFilePickerCtrlIdToWinFileOpenCtrlId( sal_Int16 aControlId )
{
    if ( aControlId < 0 || aControlId > SIZE_WINDOWS_FILEOPEN_CTRL_IDS )
        return aControlId;

    return WindowsFileOpenCtrlIds[aControlId];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
