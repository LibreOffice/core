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

#include "AccObjectManagerAgent.hxx"
#include "AccObjectWinManager.hxx"

#include "UAccCOM2.h"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

/**
   * Construction/Destruction.
   * @param
   * @return
   */
AccObjectManagerAgent::AccObjectManagerAgent():
        pWinManager(NULL)
{
    if( pWinManager == NULL )
    {
        pWinManager = AccObjectWinManager::CreateAccObjectWinManagerInstance(this);
    }
}

AccObjectManagerAgent::~AccObjectManagerAgent()
{
    delete pWinManager;
    pWinManager = NULL;
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
unsigned char AccObjectManagerAgent::InsertAccObj( XAccessible* pXAcc,XAccessible* pParentXAcc,long pWnd)
{
    if( pWinManager )
        return (unsigned char)pWinManager->InsertAccObj( pXAcc, pParentXAcc,HWND((void*)pWnd) );

    return sal_False;
}

/**
   * save the pair <topwindowhandle, XAccessible>
   * @param hWnd, top window handle
   * @param pXAcc XAccessible interface for top window
   * @return void
   */
void AccObjectManagerAgent::SaveTopWindowHandle(long hWnd, com::sun::star::accessibility::XAccessible* pXAcc)
{
    if( pWinManager )
        pWinManager->SaveTopWindowHandle( HWND((void*)hWnd), pXAcc );
}


/**
   * When a UNO XAccessible object's new children are found by listener,we'll create
   * corresponding com objects and insert them to our manager list.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pWnd The top window handle containing control.
   * @return If the method is correctly processed.
   */
unsigned char AccObjectManagerAgent::InsertChildrenAccObj( XAccessible* pXAcc,
        long pWnd)
{
    if( pWinManager )
        return (unsigned char)pWinManager->InsertChildrenAccObj( pXAcc, HWND((void*)pWnd) );

    return sal_False;
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

void  AccObjectManagerAgent::UpdateState( com::sun::star::accessibility::XAccessible* pXAcc )
{
    if(pWinManager)
        pWinManager->UpdateState(pXAcc);
}

/**
   * Interface of notify MSAA event when some UNO event occured.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pEvent UNO event ID.
   * @return If the method is correctly processed.
   */
unsigned char AccObjectManagerAgent::NotifyAccEvent( short pEvent,XAccessible* pXAcc )
{
    if(pWinManager)
        return (unsigned char)pWinManager->NotifyAccEvent(pXAcc,pEvent);

    return sal_False;
}

/**
   * Judge whether a XAccessible object is a container object.
   * @param pXAcc Uno XAccessible interface of control.
   * @return If the method is correctly processed.
   */
unsigned short AccObjectManagerAgent::IsContainer( XAccessible* pXAcc )
{
    if(pWinManager)
        return (unsigned char)pWinManager->IsContainer(pXAcc);

    return sal_False;
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
unsigned char AccObjectManagerAgent::GetIAccessibleFromXAccessible(XAccessible* pXAcc, IAccessible** ppXI)
{
    if(pWinManager)
    {
        *ppXI = (IAccessible*)pWinManager->GetIMAccByXAcc(pXAcc);
        if(*ppXI)
            return sal_True;
    }
    return sal_False;
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

void AccObjectManagerAgent::UpdateChildState(com::sun::star::accessibility::XAccessible* pXAcc)
{
    if(pWinManager)
        pWinManager->UpdateChildState( pXAcc );
}


bool AccObjectManagerAgent::IsSpecialToolboItem(com::sun::star::accessibility::XAccessible* pXAcc)
{
    if(pWinManager)
        return pWinManager->IsSpecialToolboItem( pXAcc );

    return false;
}

short AccObjectManagerAgent::GetRole(com::sun::star::accessibility::XAccessible* pXAcc)
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
bool AccObjectManagerAgent::IsTopWinAcc( com::sun::star::accessibility::XAccessible* pXAcc )
{
    if (pWinManager)
    {
        return pWinManager->IsTopWinAcc( pXAcc );
    }
    return NULL;
}

bool AccObjectManagerAgent::IsStateManageDescendant(com::sun::star::accessibility::XAccessible* pXAcc)
{
    if(pWinManager)
        return pWinManager->IsStateManageDescendant( pXAcc );

    return sal_False;
}
