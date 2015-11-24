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

#include "AccObjectManagerAgent.hxx"
#include "AccObjectWinManager.hxx"

#include "UAccCOM.h"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

/**
   * Construction/Destruction.
   * @param
   * @return
   */
AccObjectManagerAgent::AccObjectManagerAgent()
    : pWinManager(new AccObjectWinManager(this))
{
}

AccObjectManagerAgent::~AccObjectManagerAgent()
{
}

/**
   * Interface of updating MSAA name when UNO name_changed event occurs.
   * @param pXAcc Uno XAccessible interface of control.
   * @return
   */
void  AccObjectManagerAgent::UpdateAccName( XAccessible* pXAcc )
{
    if( pWinManager )
        pWinManager->UpdateAccName( pXAcc );
}

/**
   * Interface of updating MSAA name when UNO action changed event occurs.
   * @param pXAcc Uno XAccessible interface of control.
   * @return
   */
void  AccObjectManagerAgent::UpdateAction( XAccessible* pXAcc )
{
    if( pWinManager )
        pWinManager->UpdateAction( pXAcc );
}

/**
   * Interface of updating MSAA value when UNO value_changed event occurs.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pAny New value.
   * @return
   */
void  AccObjectManagerAgent::UpdateValue( XAccessible* pXAcc, Any pAny )
{
    if( pWinManager )
        pWinManager->SetValue( pXAcc, pAny );
}

/**
   * Interface of updating MSAA value when UNO value_changed event occurs.If we can not
   * find new value,we'll get new value from pXAcc to update com value.
   * @param pXAcc Uno XAccessible interface of control.
   * @return
   */
void  AccObjectManagerAgent::UpdateValue( XAccessible* pXAcc )
{
    if( pWinManager )
        pWinManager->UpdateValue( pXAcc );
}

/**
   * Interface of updating MSAA name when UNO name_changed event occurs.
   * @param pXAcc Uno XAccessible interface of control.
   * @param newName New UNO accessible name.
   * @return
   */
void  AccObjectManagerAgent::UpdateAccName( XAccessible* pXAcc, Any newName)
{
    if( pWinManager )
        pWinManager->SetAccName( pXAcc, newName );
}


/**
   * Interface of updating MSAA location when UNO location_changed event occurs.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pXAcc Uno The top position of new location.
   * @param pXAcc Uno The left position of new location.
   * @param pXAcc Uno The width of new location.
   * @param pXAcc Uno The width of new location.
   * @return
   */
void  AccObjectManagerAgent::UpdateLocation( XAccessible* /* pXAcc */, long /*top*/, long /*left*/, long /*width*/, long /*height*/ )
{
#ifdef _IMPL_WIN
    if( pWinManager )
        pWinManager->SetLocation( pXAcc, top, left, width, height );
#endif
}

/**
   * Interface of updating MSAA name when UNO description_changed event occurs.
   * @param pXAcc Uno XAccessible interface of control.
   * @param newDesc New UNO accessible description.
   * @return
   */
void  AccObjectManagerAgent::UpdateDescription( XAccessible* pXAcc, Any newDesc )
{
    if( pWinManager )
        pWinManager->SetDescription( pXAcc, newDesc );
}

/**
   * When a new UNO XAccessible object is found by listener,we'll create a corresponding
   * com object and insert it to our manager list.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pWnd The top window handle containing control.
   * @return If the method is correctly processed.
   */
bool AccObjectManagerAgent::InsertAccObj(
        XAccessible* pXAcc, XAccessible* pParentXAcc, sal_Int64 nWnd)
{
    if( pWinManager )
        return pWinManager->InsertAccObj(pXAcc, pParentXAcc,
                static_cast<HWND>(reinterpret_cast<void*>(nWnd)));

    return false;
}

/**
   * save the pair <topwindowhandle, XAccessible>
   * @param hWnd, top window handle
   * @param pXAcc XAccessible interface for top window
   * @return void
   */
void
AccObjectManagerAgent::SaveTopWindowHandle(sal_Int64 hWnd, XAccessible* pXAcc)
{
    if( pWinManager )
        pWinManager->SaveTopWindowHandle(
                static_cast<HWND>(reinterpret_cast<void*>(hWnd)), pXAcc);
}


/**
   * When a UNO XAccessible object's new children are found by listener,we'll create
   * corresponding com objects and insert them to our manager list.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pWnd The top window handle containing control.
   * @return If the method is correctly processed.
   */
bool
AccObjectManagerAgent::InsertChildrenAccObj(XAccessible* pXAcc, sal_Int64 pWnd)
{
    if( pWinManager )
        return pWinManager->InsertChildrenAccObj( pXAcc, HWND((void*)pWnd) );

    return false;
}

/**
   * When a new UNO XAccessible object is destroied,we'll delete its corresponding
   * com object and remove it from our manager list.
   * @param pXAcc Uno XAccessible interface of control.
   * @return
   */
void AccObjectManagerAgent::DeleteAccObj( XAccessible* pXAcc )
{
    if( pWinManager )
        pWinManager->DeleteAccObj( pXAcc );
}

/**
   * When new UNO children XAccessible objects are destroyed,we'll delete their
   * corresponding com objects and remove them from our manager list.
   * @param pXAcc Uno XAccessible interface of control.
   * @return
   */
void AccObjectManagerAgent::DeleteChildrenAccObj( XAccessible* pXAcc )
{
    if( pWinManager )
        pWinManager->DeleteChildrenAccObj( pXAcc );
}

/**
   * Interface of decreasing MSAA state when some UNO state is decreased.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pState The lost state of control.
   * @return
   */
void AccObjectManagerAgent::DecreaseState( XAccessible* pXAcc,unsigned short pState )
{
    if(pWinManager)
    {
        pWinManager->DecreaseState( pXAcc,  pState );
    }
}

/**
   * Interface of increasing MSAA name when some UNO state is increased.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pState The new state of control.
   * @return
   */
void AccObjectManagerAgent::IncreaseState( XAccessible* pXAcc,unsigned short pState )
{
    if(pWinManager)
    {
        pWinManager->IncreaseState( pXAcc,  pState );
    }
}

void  AccObjectManagerAgent::UpdateState( css::accessibility::XAccessible* pXAcc )
{
    if(pWinManager)
        pWinManager->UpdateState(pXAcc);
}

/**
   * Interface of notify MSAA event when some UNO event occurred.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pEvent UNO event ID.
   * @return If the method is correctly processed.
   */
bool AccObjectManagerAgent::NotifyAccEvent(short pEvent, XAccessible* pXAcc)
{
    if(pWinManager)
        return pWinManager->NotifyAccEvent(pXAcc,pEvent);

    return false;
}

/**
   * Judge whether a XAccessible object is a container object.
   * @param pXAcc Uno XAccessible interface of control.
   * @return If the method is correctly processed.
   */
bool AccObjectManagerAgent::IsContainer( XAccessible* pXAcc )
{
    if(pWinManager)
        return pWinManager->IsContainer(pXAcc);

    return false;
}

/**
   * Return com object interface by querying XAccessible interface.
   * @param pXAcc Uno XAccessible interface of control.
   * @return Com interface.
   */
IMAccessible* AccObjectManagerAgent::GetIMAccByXAcc(XAccessible* pXAcc)
{
    if(pWinManager)
        return pWinManager->GetIMAccByXAcc(pXAcc);

    return NULL;
}

/**
   * Notify manger when a XAccessible object is destroying.
   * @param pXAcc Uno XAccessible interface of control.
   * @return.
   */
void  AccObjectManagerAgent::NotifyDestroy(XAccessible* pXAcc)
{
    if(pWinManager)
        pWinManager->NotifyDestroy(pXAcc);
}

/**
   * Return com object interface by querying child id.
   * @param pXAcc Uno XAccessible interface of control.
   * @return Com interface.
   */
void AccObjectManagerAgent::GetIAccessibleFromResID(long childID,IMAccessible** pIMAcc)
{
    if(pWinManager)
        *pIMAcc = pWinManager->GetIAccessibleFromResID(childID);
}

/**
   * Return object interface by querying interface.
   * @param pXAcc Uno XAccessible interface of control.
   * @return Com interface.
   */
bool AccObjectManagerAgent::GetIAccessibleFromXAccessible(
        XAccessible* pXAcc, IAccessible** ppXI)
{
    if(pWinManager)
    {
        *ppXI = (IAccessible*)pWinManager->GetIMAccByXAcc(pXAcc);
        if(*ppXI)
            return true;
    }
    return false;
}

XAccessible* AccObjectManagerAgent::GetParentXAccessible( XAccessible* pXAcc )
{
    if(pWinManager)
        return pWinManager->GetParentXAccessible( pXAcc );

    return NULL;
}

short AccObjectManagerAgent::GetParentRole( XAccessible* pXAcc )
{
    if(pWinManager)
        return pWinManager->GetParentRole( pXAcc );

    return -1;
}

void AccObjectManagerAgent::UpdateDescription( XAccessible* pXAcc )
{
    if(pWinManager)
        pWinManager->UpdateDescription( pXAcc );
}

void AccObjectManagerAgent::UpdateChildState(XAccessible* pXAcc)
{
    if(pWinManager)
        pWinManager->UpdateChildState( pXAcc );
}


bool AccObjectManagerAgent::IsSpecialToolboItem(XAccessible* pXAcc)
{
    if(pWinManager)
        return pWinManager->IsSpecialToolboItem( pXAcc );

    return false;
}

short AccObjectManagerAgent::GetRole(XAccessible* pXAcc)
{
    if(pWinManager)
        return pWinManager->GetRole( pXAcc );

    return -1;
}

XAccessible* AccObjectManagerAgent::GetAccDocByAccTopWin( XAccessible* pXAcc )
{
    if (pWinManager)
    {
        return pWinManager->GetAccDocByAccTopWin( pXAcc );
    }
    return NULL;
}
bool AccObjectManagerAgent::IsTopWinAcc(XAccessible* pXAcc)
{
    if (pWinManager)
    {
        return pWinManager->IsTopWinAcc( pXAcc );
    }
    return NULL;
}

bool AccObjectManagerAgent::IsStateManageDescendant(XAccessible* pXAcc)
{
    if(pWinManager)
        return pWinManager->IsStateManageDescendant( pXAcc );

    return false;
}

/**
 *  Implementation of interface XMSAAService's method getAccObjectPtr()
 *  that returns the corresponding COM interface with the MS event.
 *  @return  Com interface.
 */
sal_Int64 AccObjectManagerAgent::Get_ToATInterface(
        sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam)
{
    return static_cast<sal_Int64>(pWinManager->Get_ToATInterface(
            static_cast<HWND>(reinterpret_cast<void*>(hWnd)), lParam, wParam));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
