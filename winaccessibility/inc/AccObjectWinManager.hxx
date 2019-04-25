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

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <map>
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <rtl/ref.hxx>
#include "ResIDGenerator.hxx"

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include  <UAccCOM.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

class ResIDGenerator;
class AccObjectManagerAgent;
class AccEventListener;
class AccObject;

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
    friend class AccObjectManagerAgent;

private:
    typedef std::map<void*, AccObject> XIdToAccObjHash;
    typedef std::map<HWND, void*> XHWNDToXAccHash;
    typedef std::map<const long, AccObject*> XResIdToAccObjHash;

    typedef std::map<const HWND, css::accessibility::XAccessible* >
        XHWNDToDocumentHash;

    //XAccessible to AccObject
    XIdToAccObjHash  XIdAccList;

    //HWND to All XAccessible pointer
    XHWNDToXAccHash  HwndXAcc;

    //id_Child to AccObject
    XResIdToAccObjHash XResIdAccList;

    //for file name support
    XHWNDToDocumentHash XHWNDDocList;

    css::accessibility::XAccessible* oldFocus;

    AccObjectManagerAgent*   pAgent;
    ResIDGenerator ResIdGen;

    AccObjectWinManager(AccObjectManagerAgent* Agent=nullptr);

private:
    long ImpleGenerateResID();
    AccObject* GetAccObjByXAcc( css::accessibility::XAccessible* pXAcc);

    AccObject* GetTopWindowAccObj(HWND hWnd);

    css::accessibility::XAccessible* GetAccDocByHWND(HWND hWnd);

    static void DeleteAccListener( AccObject* pAccObj );
    static void InsertAccChildNode(AccObject* pCurObj,AccObject* pParentObj,HWND pWnd);
    static void DeleteAccChildNode(AccObject* pChild);
    void       DeleteFromHwndXAcc(css::accessibility::XAccessible const * pXAcc );
    int  UpdateAccSelection(css::accessibility::XAccessible* pXAcc);

    ::rtl::Reference<AccEventListener> CreateAccEventListener(
            css::accessibility::XAccessible* pXAcc);
public:
    virtual ~AccObjectWinManager();
    bool InsertAccObj( css::accessibility::XAccessible* pXAcc,css::accessibility::XAccessible* pParentXAcc,HWND pWnd);
    bool InsertChildrenAccObj( css::accessibility::XAccessible* pXAcc,HWND pWnd=nullptr);
    void DeleteAccObj( css::accessibility::XAccessible* pXAcc );
    void DeleteChildrenAccObj(css::accessibility::XAccessible* pAccObj);

    bool NotifyAccEvent( css::accessibility::XAccessible* pXAcc,short state = 0 );

    LRESULT Get_ToATInterface(HWND hWnd, long lParam, WPARAM wParam);

    void  DecreaseState( css::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  IncreaseState( css::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  UpdateState( css::accessibility::XAccessible* pXAcc );
    void  SetLocation( css::accessibility::XAccessible* pXAcc,
                       long Top = 0,long left = 0,long width = 0,long height = 0);

    void  SetValue( css::accessibility::XAccessible* pXAcc, css::uno::Any pAny );
    void  UpdateValue( css::accessibility::XAccessible* pXAcc );

    void  SetAccName( css::accessibility::XAccessible* pXAcc, css::uno::Any newName);
    void  UpdateAccName( css::accessibility::XAccessible* pXAcc );

    void  SetDescription( css::accessibility::XAccessible* pXAcc, css::uno::Any newDesc );
    void UpdateDescription( css::accessibility::XAccessible* pXAcc );

    void  SetRole( css::accessibility::XAccessible* pXAcc, long Role );

    void  UpdateAccFocus( css::accessibility::XAccessible* newFocus );
    void  UpdateAction( css::accessibility::XAccessible* pXAcc );

    static bool IsContainer( css::accessibility::XAccessible* pAccessible );

    IMAccessible* GetIMAccByXAcc( css::accessibility::XAccessible* pXAcc );
    IMAccessible* GetIAccessibleFromResID(long resID);

    void NotifyDestroy( css::accessibility::XAccessible* pXAcc );
    css::accessibility::XAccessible* GetParentXAccessible( css::accessibility::XAccessible* pXAcc );
    short GetParentRole( css::accessibility::XAccessible* pXAcc );

    void SaveTopWindowHandle(HWND hWnd, css::accessibility::XAccessible* pXAcc);

    void UpdateChildState(css::accessibility::XAccessible* pXAcc);

    bool IsSpecialToolboItem(css::accessibility::XAccessible* pXAcc);

    static short GetRole(css::accessibility::XAccessible* pXAcc);

    css::accessibility::XAccessible* GetAccDocByAccTopWin( css::accessibility::XAccessible* pXAcc );
    bool IsTopWinAcc( css::accessibility::XAccessible* pXAcc );

    static bool IsStateManageDescendant(css::accessibility::XAccessible* pAccessible);

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
