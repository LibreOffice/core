/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dialogcustomcontrols.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:52:13 $
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

#ifndef _DIALOGCUSTOMCONTROLS_HXX_
#define _DIALOGCUSTOMCONTROLS_HXX_

#ifndef _CUSTOMCONTROL_HXX_
#include "customcontrol.hxx"
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//-----------------------------------
//
//-----------------------------------

class CDummyCustomControl : public CCustomControl
{
public:
    CDummyCustomControl(HWND aControlHandle, HWND aParentHandle);

    virtual void SAL_CALL Align();
    virtual void SAL_CALL SetFont(HFONT hFont);
};

//-----------------------------------
//
//-----------------------------------

class CDialogCustomControlBase : public CCustomControl
{
protected:
    CDialogCustomControlBase(HWND aControlHandle, HWND aParentHandle);

    virtual void SAL_CALL SetFont(HFONT hFont);

    // aligns the specific control class to a reference
    // buddy
    //
    void SAL_CALL AlignToBuddy(HWND aBuddyHandle);

protected:
    HWND m_CustomControlHandle;
    HWND m_ParentHandle;
};

//-----------------------------------
//
//-----------------------------------

class CStaticCustomControl : public CDialogCustomControlBase
{
public:
    CStaticCustomControl(HWND aControlHandle, HWND aParentHandle);

    virtual void SAL_CALL Align();
};

//-----------------------------------
//
//-----------------------------------

class CPushButtonCustomControl : public CDialogCustomControlBase
{
public:
    CPushButtonCustomControl(HWND aControlHandle, HWND aParentHandle);

    virtual void SAL_CALL Align();
};

//-----------------------------------
//
//-----------------------------------

class CComboboxCustomControl : public CDialogCustomControlBase
{
public:
    CComboboxCustomControl(HWND aControlHandle, HWND aParentHandle);

    virtual void SAL_CALL Align();
};

//-----------------------------------
//
//-----------------------------------

class CCheckboxCustomControl : public CDialogCustomControlBase
{
public:
    CCheckboxCustomControl(HWND aControlHandle, HWND aParentHandle);

    virtual void SAL_CALL Align();
};

#endif
