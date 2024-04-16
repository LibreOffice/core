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

#include <map>
#include <mutex>

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <prewin.h>
#include <windows.h>
#include <postwin.h>
#include <rtl/ref.hxx>
#include "ResIDGenerator.hxx"
#include  <UAccCOM.h>

namespace com::sun::star::accessibility {
class XAccessible;
}
class ResIDGenerator;
class AccEventListener;
class AccObject;
enum class UnoMSAAEvent;

/*******************************************************************
AccObjectWinManager complete the functions:
1. Insert, delete,query,update Acc objects
2. Create, delete,fire AccEventLister, the lifecycle of AccEventListener is as same as
   Acc Object
 3.Return COM interface for AT,by the call back in salframe
 4.Pass accessible information to Acc objects
 *******************************************************************/
class AccObjectWinManager
{
private:
    typedef std::map<com::sun::star::accessibility::XAccessible*, AccObject> XIdToAccObjHash;
    typedef std::map<HWND, com::sun::star::accessibility::XAccessible*> XHWNDToXAccHash;
    typedef std::map<const long, AccObject*> XResIdToAccObjHash;

    typedef std::map<const HWND, css::accessibility::XAccessible* >
        XHWNDToDocumentHash;

    // guard any access to XIdAccList and HwndXAcc
    std::recursive_mutex m_Mutex;

    //XAccessible to AccObject
    XIdToAccObjHash  XIdAccList;

    //HWND to All XAccessible pointer
    XHWNDToXAccHash  HwndXAcc;

    //id_Child to AccObject
    XResIdToAccObjHash XResIdAccList;

    //for file name support
    XHWNDToDocumentHash XHWNDDocList;

    css::accessibility::XAccessible* oldFocus;

    ResIDGenerator ResIdGen;

private:
    long ImpleGenerateResID();
    AccObject* GetAccObjByXAcc( css::accessibility::XAccessible* pXAcc);

    IMAccessible* GetTopWindowIMAccessible(HWND hWnd);

    css::accessibility::XAccessible* GetAccDocByHWND(HWND hWnd);

    static rtl::Reference<AccEventListener> DeleteAccListener(AccObject* pAccObj);
    static void InsertAccChildNode(AccObject* pCurObj,AccObject* pParentObj,HWND pWnd);
    static void DeleteAccChildNode(AccObject* pChild);
    void       DeleteFromHwndXAcc(css::accessibility::XAccessible const * pXAcc );

    ::rtl::Reference<AccEventListener> CreateAccEventListener(
            css::accessibility::XAccessible* pXAcc);
public:
    AccObjectWinManager();
    virtual ~AccObjectWinManager();
    virtual bool InsertAccObj(css::accessibility::XAccessible* pXAcc,
                              css::accessibility::XAccessible* pParentXAcc,
                              HWND pWnd = nullptr);
    bool InsertChildrenAccObj( css::accessibility::XAccessible* pXAcc,HWND pWnd=nullptr);
    void DeleteAccObj( css::accessibility::XAccessible* pXAcc );
    void DeleteChildrenAccObj(css::accessibility::XAccessible* pAccObj);

    bool NotifyAccEvent(css::accessibility::XAccessible* pXAcc, UnoMSAAEvent eEvent);

    sal_Int64 Get_ToATInterface(sal_Int64 nHWnd, long lParam, WPARAM wParam);

    void  DecreaseState(css::accessibility::XAccessible* pXAcc, sal_Int64 nState);
    void  IncreaseState(css::accessibility::XAccessible* pXAcc, sal_Int64 nState);
    void  UpdateState( css::accessibility::XAccessible* pXAcc );

    void  SetValue( css::accessibility::XAccessible* pXAcc, css::uno::Any pAny );
    void  UpdateValue( css::accessibility::XAccessible* pXAcc );

    void  SetAccName( css::accessibility::XAccessible* pXAcc, css::uno::Any newName);
    void  UpdateAccName( css::accessibility::XAccessible* pXAcc );

    void  UpdateAccFocus( css::accessibility::XAccessible* newFocus );
    void  UpdateAction( css::accessibility::XAccessible* pXAcc );

    static bool IsContainer( css::accessibility::XAccessible* pAccessible );

    virtual IMAccessible* GetIAccessibleFromXAccessible(css::accessibility::XAccessible* pXAcc);
    virtual IMAccessible* GetIAccessibleFromResID(long resID);

    void NotifyDestroy( css::accessibility::XAccessible* pXAcc );
    css::accessibility::XAccessible* GetParentXAccessible( css::accessibility::XAccessible* pXAcc );
    short GetParentRole( css::accessibility::XAccessible* pXAcc );

    void SaveTopWindowHandle(HWND hWnd, css::accessibility::XAccessible* pXAcc);

    void UpdateChildState(css::accessibility::XAccessible* pXAcc);

    static short GetRole(css::accessibility::XAccessible* pXAcc);

    css::accessibility::XAccessible* GetAccDocByAccTopWin( css::accessibility::XAccessible* pXAcc );
    bool IsTopWinAcc( css::accessibility::XAccessible* pXAcc );

    static bool IsStateManageDescendant(css::accessibility::XAccessible* pAccessible);

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
