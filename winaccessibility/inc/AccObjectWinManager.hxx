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

#ifndef __ACCOBJECTWINMANAGER_HXX
#define __ACCOBJECTWINMANAGER_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#include <map>
#include <windows.h>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include "ResIDGenerator.hxx"
#include "UAccCOM.h"

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

    typedef std::map<const HWND, com::sun::star::accessibility::XAccessible* >
        XHWNDToDocumentHash;

    //XAccessible to AccObject
    XIdToAccObjHash  XIdAccList;

    //HWND to All XAccessible pointer
    XHWNDToXAccHash  HwndXAcc;

    //id_Child to AccObject
    XResIdToAccObjHash XResIdAccList;

    //for file name support
    XHWNDToDocumentHash XHWNDDocList;

    com::sun::star::accessibility::XAccessible* oldFocus;

    AccObjectManagerAgent*   pAgent;
    ResIDGenerator ResIdGen;

    AccObjectWinManager(AccObjectManagerAgent* Agent=NULL);

private:
    long ImpleGenerateResID();
    AccObject* GetAccObjByXAcc( com::sun::star::accessibility::XAccessible* pXAcc);

    com::sun::star::accessibility::XAccessible* GetXAccByAccObj(AccObject* pAccObj);

    AccObject* GetTopWindowAccObj(HWND hWnd);

    com::sun::star::accessibility::XAccessible* GetAccDocByHWND(HWND hWnd);

    void       DeleteAccListener( AccObject* pAccObj );
    void       InsertAccChildNode(AccObject* pCurObj,AccObject* pParentObj,HWND pWnd);
    void       DeleteAccChildNode(AccObject* pChild);
    void       DeleteFromHwndXAcc(com::sun::star::accessibility::XAccessible* pXAcc );
    int  UpdateAccSelection(com::sun::star::accessibility::XAccessible* pXAcc);

    ::rtl::Reference<AccEventListener> CreateAccEventListener(
            com::sun::star::accessibility::XAccessible* pXAcc);
public:
    virtual ~AccObjectWinManager();
    sal_Bool InsertAccObj( com::sun::star::accessibility::XAccessible* pXAcc,com::sun::star::accessibility::XAccessible* pParentXAcc,HWND pWnd);
    sal_Bool InsertChildrenAccObj( com::sun::star::accessibility::XAccessible* pXAcc,HWND pWnd=0);
    void DeleteAccObj( com::sun::star::accessibility::XAccessible* pXAcc );
    void DeleteChildrenAccObj(com::sun::star::accessibility::XAccessible* pAccObj);

    static  AccObjectWinManager* CreateAccObjectWinManagerInstance(AccObjectManagerAgent* Agent);

    sal_Bool NotifyAccEvent( com::sun::star::accessibility::XAccessible* pXAcc,short state = 0 );

    LPARAM Get_ToATInterface(HWND hWnd, long lParam, WPARAM wParam);

    void  DecreaseState( com::sun::star::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  IncreaseState( com::sun::star::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  UpdateState( com::sun::star::accessibility::XAccessible* pXAcc );
    void  SetLocation( com::sun::star::accessibility::XAccessible* pXAcc,
                       long Top = 0,long left = 0,long width = 0,long height = 0);

    void  SetValue( com::sun::star::accessibility::XAccessible* pXAcc, com::sun::star::uno::Any pAny );
    void  UpdateValue( com::sun::star::accessibility::XAccessible* pXAcc );

    void  SetAccName( com::sun::star::accessibility::XAccessible* pXAcc, com::sun::star::uno::Any newName);
    void  UpdateAccName( com::sun::star::accessibility::XAccessible* pXAcc );

    void  SetDescription( com::sun::star::accessibility::XAccessible* pXAcc, com::sun::star::uno::Any newDesc );
    void UpdateDescription( com::sun::star::accessibility::XAccessible* pXAcc );

    void  SetRole( com::sun::star::accessibility::XAccessible* pXAcc, long Role );

    void  UpdateAccFocus( com::sun::star::accessibility::XAccessible* newFocus );
    void  UpdateAction( com::sun::star::accessibility::XAccessible* pXAcc );

    sal_Bool IsContainer( com::sun::star::accessibility::XAccessible* pAccessible );

    IMAccessible* GetIMAccByXAcc( com::sun::star::accessibility::XAccessible* pXAcc );
    IMAccessible* GetIAccessibleFromResID(long resID);

    void NotifyDestroy( com::sun::star::accessibility::XAccessible* pXAcc );
    com::sun::star::accessibility::XAccessible* GetParentXAccessible( com::sun::star::accessibility::XAccessible* pXAcc );
    short GetParentRole( com::sun::star::accessibility::XAccessible* pXAcc );

    void SaveTopWindowHandle(HWND hWnd, com::sun::star::accessibility::XAccessible* pXAcc);

    void UpdateChildState(com::sun::star::accessibility::XAccessible* pXAcc);

    bool IsSpecialToolboItem(com::sun::star::accessibility::XAccessible* pXAcc);

    short GetRole(com::sun::star::accessibility::XAccessible* pXAcc);

    com::sun::star::accessibility::XAccessible* GetAccDocByAccTopWin( com::sun::star::accessibility::XAccessible* pXAcc );
    bool IsTopWinAcc( com::sun::star::accessibility::XAccessible* pXAcc );

    bool IsStateManageDescendant(com::sun::star::accessibility::XAccessible* pAccessible);

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
