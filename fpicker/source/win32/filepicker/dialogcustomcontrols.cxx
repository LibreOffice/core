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

#ifndef _DIALOGCUSTOMCONTROLS_CXX_
#include "dialogcustomcontrols.hxx"
#endif
#include <osl/diagnose.h>

//-----------------------------------
//
//-----------------------------------

CDialogCustomControlBase::CDialogCustomControlBase(HWND aControlHandle, HWND aParentHandle) :
    m_CustomControlHandle(aControlHandle),
    m_ParentHandle(aParentHandle)
{
}

//-----------------------------------
//
//-----------------------------------

void SAL_CALL CDialogCustomControlBase::SetFont(HFONT hFont)
{
    SendMessage(
        m_CustomControlHandle,
        WM_SETFONT,
        (WPARAM)hFont,
        (LPARAM)sal_True);
}

//-----------------------------------
//
//-----------------------------------

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

//-----------------------------------
//
//-----------------------------------

CDummyCustomControl::CDummyCustomControl(HWND, HWND)
{
}

//-----------------------------------
//
//-----------------------------------

void SAL_CALL CDummyCustomControl::Align()
{
    // do nothing
}

//-----------------------------------
//
//-----------------------------------

void SAL_CALL CDummyCustomControl::SetFont(HFONT)
{
    // do nothing
}

//-----------------------------------
//
//-----------------------------------

CStaticCustomControl::CStaticCustomControl(HWND aControlHandle, HWND aParentHandle) :
    CDialogCustomControlBase(aControlHandle,aParentHandle)
{
}

//-----------------------------------
// Align to the "File name" static
// text of the standard FileOpen dlg
//-----------------------------------

void SAL_CALL CStaticCustomControl::Align()
{
    AlignToBuddy(GetDlgItem(m_ParentHandle,stc3));
}

//-----------------------------------
//
//-----------------------------------

CPushButtonCustomControl::CPushButtonCustomControl(HWND aControlHandle, HWND aParentHandle) :
    CDialogCustomControlBase(aControlHandle,aParentHandle)
{
}

//-----------------------------------
// Align to the "OK" button of the
// standard FileOpen dlg
//-----------------------------------

void SAL_CALL CPushButtonCustomControl::Align()
{
    AlignToBuddy(GetDlgItem(m_ParentHandle,IDCANCEL));
}

//-----------------------------------
//
//-----------------------------------

CComboboxCustomControl::CComboboxCustomControl(HWND aControlHandle, HWND aParentHandle) :
    CDialogCustomControlBase(aControlHandle,aParentHandle)
{
}

//-----------------------------------
// Align to the "File name" combobox
// of the standard FileOpen dlg
//-----------------------------------

void SAL_CALL CComboboxCustomControl::Align()
{
    AlignToBuddy(GetDlgItem(m_ParentHandle,cmb1));
}

//-----------------------------------
//
//-----------------------------------

CCheckboxCustomControl::CCheckboxCustomControl(HWND aControlHandle, HWND aParentHandle) :
    CDialogCustomControlBase(aControlHandle,aParentHandle)
{
}

//-----------------------------------
// Align to the "File name" combobox
// of the standard FileOpen dlg
//-----------------------------------

void SAL_CALL CCheckboxCustomControl::Align()
{
    AlignToBuddy(GetDlgItem(m_ParentHandle,cmb1));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
