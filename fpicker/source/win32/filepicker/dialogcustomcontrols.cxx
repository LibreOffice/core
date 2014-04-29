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
    // the Windows dialog controller does
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
