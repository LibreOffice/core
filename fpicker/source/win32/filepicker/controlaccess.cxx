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

#include <tchar.h>
#include <sal/macros.h>
#include <osl/diagnose.h>
#include "controlaccess.hxx"
#include "../misc/WinImplHelper.hxx"

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
    const int SIZE_WINDOWS_FILEOPEN_CTRL_IDS =
        sizeof(WindowsFileOpenCtrlIds)/sizeof(WindowsFileOpenCtrlIds[0]);

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
