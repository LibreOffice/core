/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dialogcustomcontrols.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:51:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

#ifndef _DIALOGCUSTOMCONTROLS_CXX_
#include "dialogcustomcontrols.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
        (LPARAM)TRUE);
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
