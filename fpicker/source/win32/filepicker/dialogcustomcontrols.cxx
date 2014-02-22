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

#include "dialogcustomcontrols.hxx"
#include <osl/diagnose.h>


//


CDialogCustomControlBase::CDialogCustomControlBase(HWND aControlHandle, HWND aParentHandle) :
    m_CustomControlHandle(aControlHandle),
    m_ParentHandle(aParentHandle)
{
}


//


void SAL_CALL CDialogCustomControlBase::SetFont(HFONT hFont)
{
    SendMessage(
        m_CustomControlHandle,
        WM_SETFONT,
        (WPARAM)hFont,
        (LPARAM)sal_True);
}


//


void SAL_CALL CDialogCustomControlBase::AlignToBuddy(HWND aBuddyHandle)
{
    OSL_PRECOND(IsWindow(aBuddyHandle),"Invalid buddy window handle");

    RECT rcBuddy;
    GetWindowRect(aBuddyHandle,&rcBuddy);

    POINT pt = {rcBuddy.left,rcBuddy.top};
    ScreenToClient(m_ParentHandle,&pt);

    int cx_new = rcBuddy.right - rcBuddy.left;
    int cy_new = rcBuddy.bottom - rcBuddy.top;

    // keep the vertical position because
    // the Windows dialog controler does
    // this job
    RECT rcMe;
    GetWindowRect(m_CustomControlHandle,&rcMe);

    POINT ptMe = {rcMe.left,rcMe.top};
    ScreenToClient(m_ParentHandle,&ptMe);

    SetWindowPos(
        m_CustomControlHandle,
        HWND_TOP,
        pt.x,
        ptMe.y,
        cx_new,
        cy_new,
        SWP_NOACTIVATE);
}


//


CDummyCustomControl::CDummyCustomControl(HWND, HWND)
{
}


//


void SAL_CALL CDummyCustomControl::Align()
{
    // do nothing
}


//


void SAL_CALL CDummyCustomControl::SetFont(HFONT)
{
    // do nothing
}


//


CStaticCustomControl::CStaticCustomControl(HWND aControlHandle, HWND aParentHandle) :
    CDialogCustomControlBase(aControlHandle,aParentHandle)
{
}


// Align to the "File name" static
// text of the standard FileOpen dlg


void SAL_CALL CStaticCustomControl::Align()
{
    AlignToBuddy(GetDlgItem(m_ParentHandle,stc3));
}


//


CPushButtonCustomControl::CPushButtonCustomControl(HWND aControlHandle, HWND aParentHandle) :
    CDialogCustomControlBase(aControlHandle,aParentHandle)
{
}


// Align to the "OK" button of the
// standard FileOpen dlg


void SAL_CALL CPushButtonCustomControl::Align()
{
    AlignToBuddy(GetDlgItem(m_ParentHandle,IDCANCEL));
}


//


CComboboxCustomControl::CComboboxCustomControl(HWND aControlHandle, HWND aParentHandle) :
    CDialogCustomControlBase(aControlHandle,aParentHandle)
{
}


// Align to the "File name" combobox
// of the standard FileOpen dlg


void SAL_CALL CComboboxCustomControl::Align()
{
    AlignToBuddy(GetDlgItem(m_ParentHandle,cmb1));
}


//


CCheckboxCustomControl::CCheckboxCustomControl(HWND aControlHandle, HWND aParentHandle) :
    CDialogCustomControlBase(aControlHandle,aParentHandle)
{
}


// Align to the "File name" combobox
// of the standard FileOpen dlg


void SAL_CALL CCheckboxCustomControl::Align()
{
    AlignToBuddy(GetDlgItem(m_ParentHandle,cmb1));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
