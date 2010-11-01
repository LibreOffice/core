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

#ifndef _DIALOGCUSTOMCONTROLS_HXX_
#define _DIALOGCUSTOMCONTROLS_HXX_

#include "customcontrol.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
