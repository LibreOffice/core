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

#pragma once

#include <vector>
#include <map>
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <oleacc.h>

#include <rtl/ref.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>

#include "accHelper.hxx"
#include  <UAccCOM.h>

class AccEventListener;
class AccObjectWinManager;
class AccObject;

typedef std::vector<AccObject *> IAccChildList;


class AccObject
{
private:

    short               m_accRole;
    long                m_resID;
    HWND                m_pParantID;
    IMAccessible* const m_pIMAcc; // AccObjectManager::GetTopWindowIMAccessible relies on this being const
    AccObject*          m_pParentObj;
    IAccChildList       m_childrenList;
    ::rtl::Reference<AccEventListener>  m_pListener;

    css::uno::Reference < css::accessibility::XAccessible > m_xAccRef;
    css::uno::Reference < css::accessibility::XAccessibleAction > m_xAccActionRef;
    css::uno::Reference < css::accessibility::XAccessibleContext > m_xAccContextRef;

    void ImplInitializeCreateObj();//create COM object

    void UpdateActionDesc();
    void UpdateRole();

    DWORD GetMSAAStateFromUNO(sal_Int64 xState);//translate state from UNO to MSAA value
    css::accessibility::XAccessibleSelection* GetXAccessibleSelection();
    void GetExpandedState(sal_Bool* isExpandable, sal_Bool* isExpanded);
    static OUString GetMAccessibleValueFromAny(css::uno::Any pAny);

public:

    AccObject(css::accessibility::XAccessible* pXAcc = nullptr, AccObjectWinManager* pManager = nullptr,
              AccEventListener* accListener=nullptr);
    virtual ~AccObject();

    bool UpdateAccessibleInfoFromUnoToMSAA(  ); //implement accessible information mapping
    void UpdateDefaultAction();

    IMAccessible*  GetIMAccessible();   //return COM interface in acc object
    css::uno::Reference<css::accessibility::XAccessible> const& GetXAccessible();

    void SetResID(long id);//ResID means ChildID in MSAA
    long GetResID();


    void SetParentHWND(HWND hWnd);//need to set top window handle when send event to AT
    HWND GetParentHWND();

    ::rtl::Reference<AccEventListener> SetListener(::rtl::Reference<AccEventListener> const& pListener);
    AccEventListener* getListener();

    void SetParentObj(AccObject* pParentAccObj);
    AccObject* GetParentObj();

    void InsertChild( AccObject* pChild,short pos = LAST_CHILD);
    void DeleteChild( AccObject* pChild );
    AccObject* NextChild();

    void NotifyDestroy();

    void  DecreaseState( sal_Int64 xState );//call COM interface DecreaseState method
    void  IncreaseState( sal_Int64 xState );//call COM interface IncreaseState method

    void  SetName( css::uno::Any newName);
    void  SetValue( css::uno::Any pAny );

    short GetRole() const;

    void  UpdateState();
    void  UpdateName();
    void  UpdateValue();
    void  UpdateAction();
    void  UpdateValidWindow();

    void  setFocus();
    void  unsetFocus();

    void  setLocalizedResourceString();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
