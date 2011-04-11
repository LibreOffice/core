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

#include <tchar.h>
#include "customcontrolfactory.hxx"
#include "customcontrolcontainer.hxx"
#include "dialogcustomcontrols.hxx"
#include <osl/diagnose.h>

//-----------------------------------
//
//-----------------------------------

CCustomControl* CCustomControlFactory::CreateCustomControl(HWND aControlHandle, HWND aParentHandle)
{
    OSL_PRECOND(IsWindow(aControlHandle),"Invalid control handle");
    OSL_PRECOND(IsWindow(aControlHandle),"Invalid parent handle");

    // get window class
    // if static text create static text control etc.

    TCHAR aClsName[256];
    ZeroMemory(aClsName,sizeof(aClsName));
    if (GetClassName(aControlHandle,aClsName,sizeof(aClsName)) == 0) {
        OSL_FAIL("Invalid window handle");
    }

    if (0 == _tcsicmp(aClsName,TEXT("button")))
    {
        // button means many things so we have
        // to find out what button it is
        LONG lBtnStyle = GetWindowLong(aControlHandle,GWL_STYLE);

        if (lBtnStyle & BS_CHECKBOX)
            return new CCheckboxCustomControl(aControlHandle,aParentHandle);

        if ( ((lBtnStyle & BS_PUSHBUTTON) == 0) || (lBtnStyle & BS_DEFPUSHBUTTON))
            return new CPushButtonCustomControl(aControlHandle,aParentHandle);

        return new CDummyCustomControl(aControlHandle,aParentHandle);
    }

    if (0 == _tcsicmp(aClsName,TEXT("listbox")) || 0 == _tcsicmp(aClsName,TEXT("combobox")))
        return new CComboboxCustomControl(aControlHandle,aParentHandle);

    if (0 == _tcsicmp(aClsName,TEXT("static")))
        return new CStaticCustomControl(aControlHandle,aParentHandle);

    return new CDummyCustomControl(aControlHandle,aParentHandle);
}

//-----------------------------------
//
//-----------------------------------

CCustomControl* CCustomControlFactory::CreateCustomControlContainer()
{
    return new CCustomControlContainer();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
