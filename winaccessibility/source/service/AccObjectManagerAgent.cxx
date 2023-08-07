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

#include <AccObjectManagerAgent.hxx>
#include <AccObjectWinManager.hxx>

#include  <UAccCOM.h>

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
    assert(pWinManager);
    pWinManager->UpdateAccName( pXAcc );
}

/**
   * Interface of updating MSAA name when UNO action changed event occurs.
   * @param pXAcc Uno XAccessible interface of control.
   * @return
   */
void  AccObjectManagerAgent::UpdateAction( XAccessible* pXAcc )
{
    assert(pWinManager);
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
    assert(pWinManager);
    pWinManager->SetValue( pXAcc, pAny );
}

/**
   * Interface of updating MSAA value when UNO value_changed event occurs. If we can not
   * find new value, we'll get new value from pXAcc to update com value.
   * @param pXAcc Uno XAccessible interface of control.
   * @return
   */
void  AccObjectManagerAgent::UpdateValue( XAccessible* pXAcc )
{
    assert(pWinManager);
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
    assert(pWinManager);
    pWinManager->SetAccName( pXAcc, newName );
}

/**
   * When a new UNO XAccessible object is found by listener, we create a corresponding
   * com object and insert it to our manager list.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pWnd The top window handle containing control.
   * @return If the method is correctly processed.
   */
bool AccObjectManagerAgent::InsertAccObj(
        XAccessible* pXAcc, XAccessible* pParentXAcc, HWND hWnd)
{
    assert(pWinManager);
    return pWinManager->InsertAccObj(pXAcc, pParentXAcc, hWnd);
}

/**
   * save the pair <topwindowhandle, XAccessible>
   * @param hWnd, top window handle
   * @param pXAcc XAccessible interface for top window
   * @return void
   */
void
AccObjectManagerAgent::SaveTopWindowHandle(HWND hWnd, XAccessible* pXAcc)
{
    assert(pWinManager);
    pWinManager->SaveTopWindowHandle(hWnd, pXAcc);
}


/**
   * When a UNO XAccessible object's new children are found by listener, we create
   * corresponding com objects and insert them to our manager list.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pWnd The top window handle containing control.
   * @return If the method is correctly processed.
   */
bool
AccObjectManagerAgent::InsertChildrenAccObj(XAccessible* pXAcc, HWND hWnd)
{
    assert(pWinManager);
    return pWinManager->InsertChildrenAccObj(pXAcc, hWnd);
}

/**
   * When a new UNO XAccessible object is destroyed, we delete its corresponding
   * com object and remove it from our manager list.
   * @param pXAcc Uno XAccessible interface of control.
   * @return
   */
void AccObjectManagerAgent::DeleteAccObj( XAccessible* pXAcc )
{
    assert(pWinManager);
    pWinManager->DeleteAccObj( pXAcc );
}

/**
   * When new UNO children XAccessible objects are destroyed, we delete their
   * corresponding com objects and remove them from our manager list.
   * @param pXAcc Uno XAccessible interface of control.
   * @return
   */
void AccObjectManagerAgent::DeleteChildrenAccObj( XAccessible* pXAcc )
{
    assert(pWinManager);
    pWinManager->DeleteChildrenAccObj( pXAcc );
}

/**
   * Interface of decreasing MSAA state when some UNO state is decreased.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pState The lost state of control.
   * @return
   */
void AccObjectManagerAgent::DecreaseState( XAccessible* pXAcc, sal_Int64 pState )
{
    assert(pWinManager);
    pWinManager->DecreaseState( pXAcc,  pState );
}

/**
   * Interface of increasing MSAA name when some UNO state is increased.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pState The new state of control.
   * @return
   */
void AccObjectManagerAgent::IncreaseState( XAccessible* pXAcc, sal_Int64 pState )
{
    assert(pWinManager);
    pWinManager->IncreaseState( pXAcc,  pState );
}

void  AccObjectManagerAgent::UpdateState( css::accessibility::XAccessible* pXAcc )
{
    assert(pWinManager);
    pWinManager->UpdateState(pXAcc);
}

/**
   * Interface of notify MSAA event when some UNO event occurred.
   * @param pXAcc Uno XAccessible interface of control.
   * @param eEvent event type.
   * @return If the method is correctly processed.
   */
bool AccObjectManagerAgent::NotifyAccEvent(UnoMSAAEvent eEvent, XAccessible* pXAcc)
{
    assert(pWinManager);
    return pWinManager->NotifyAccEvent(pXAcc, eEvent);
}

/**
   * Judge whether a XAccessible object is a container object.
   * @param pXAcc Uno XAccessible interface of control.
   * @return If the method is correctly processed.
   */
bool AccObjectManagerAgent::IsContainer( XAccessible* pXAcc )
{
    assert(pWinManager);
    return AccObjectWinManager::IsContainer(pXAcc);
}

/**
   * Return com object interface by querying XAccessible interface.
   * @param pXAcc Uno XAccessible interface of control.
   * @return Com interface.
   */
IMAccessible* AccObjectManagerAgent::GetIMAccByXAcc(XAccessible* pXAcc)
{
    assert(pWinManager);
    return pWinManager->GetIMAccByXAcc(pXAcc);
}

/**
   * Notify manager when a XAccessible object is destroying.
   * @param pXAcc Uno XAccessible interface of control.
   * @return.
   */
void  AccObjectManagerAgent::NotifyDestroy(XAccessible* pXAcc)
{
    assert(pWinManager);
    pWinManager->NotifyDestroy(pXAcc);
}

/**
   * Return com object interface by querying child id.
   * @param pXAcc Uno XAccessible interface of control.
   * @return Com interface.
   */
void AccObjectManagerAgent::GetIAccessibleFromResID(long childID,IMAccessible** pIMAcc)
{
    assert(pWinManager);
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
    assert(pWinManager);
    return pWinManager->GetIAccessibleFromXAccessible(pXAcc, ppXI);
}

XAccessible* AccObjectManagerAgent::GetParentXAccessible( XAccessible* pXAcc )
{
    assert(pWinManager);
    return pWinManager->GetParentXAccessible( pXAcc );
}

short AccObjectManagerAgent::GetParentRole( XAccessible* pXAcc )
{
    assert(pWinManager);
    return pWinManager->GetParentRole( pXAcc );
}

void AccObjectManagerAgent::UpdateChildState(XAccessible* pXAcc)
{
    assert(pWinManager);
    pWinManager->UpdateChildState( pXAcc );
}


bool AccObjectManagerAgent::IsSpecialToolbarItem(XAccessible* pXAcc)
{
    assert(pWinManager);
    return pWinManager->IsSpecialToolbarItem(pXAcc);
}

XAccessible* AccObjectManagerAgent::GetAccDocByAccTopWin( XAccessible* pXAcc )
{
    assert(pWinManager);
    return pWinManager->GetAccDocByAccTopWin( pXAcc );
}

bool AccObjectManagerAgent::IsTopWinAcc(XAccessible* pXAcc)
{
    assert(pWinManager);
    return pWinManager->IsTopWinAcc( pXAcc );
}

bool AccObjectManagerAgent::IsStateManageDescendant(XAccessible* pXAcc)
{
    assert(pWinManager);
    return AccObjectWinManager::IsStateManageDescendant( pXAcc );
}

/**
 *  Implementation of interface XMSAAService's method getAccObjectPtr()
 *  that returns the corresponding COM interface with the MS event.
 *  @return  Com interface.
 */
sal_Int64 AccObjectManagerAgent::Get_ToATInterface(
        sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam)
{
    return pWinManager->Get_ToATInterface(hWnd, lParam, wParam);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
