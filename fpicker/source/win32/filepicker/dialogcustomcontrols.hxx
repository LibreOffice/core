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
