/*************************************************************************
 *
 *  $RCSfile: dialogcustomcontrols.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:04:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

CDummyCustomControl::CDummyCustomControl(HWND aControlHandle, HWND aParentHandle)
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

void SAL_CALL CDummyCustomControl::SetFont(HFONT hFont)
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
