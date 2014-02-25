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
#include "customcontrolfactory.hxx"
#include "customcontrolcontainer.hxx"
#include "dialogcustomcontrols.hxx"
#include <osl/diagnose.h>





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





CCustomControl* CCustomControlFactory::CreateCustomControlContainer()
{
    return new CCustomControlContainer();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
