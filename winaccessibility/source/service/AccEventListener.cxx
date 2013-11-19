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

#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <toolkit/awt/Vclxwindow.hxx>

#ifndef _SV_SYSDATA_HXX
#if defined( WIN ) || defined( WNT ) || defined( OS2 )
typedef sal_Int32 HWND;
#endif
#endif
#include "AccEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>

#include <stdio.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;
using namespace rtl;
using namespace cppu;

AccEventListener::AccEventListener(com::sun::star::accessibility::XAccessible* pAcc,
                                   AccObjectManagerAgent* Agent)
    : pAccessible(pAcc)
    , pAgent(Agent)
    , m_isDisposed(false)
{}

AccEventListener::~AccEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccEventListener::notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
throw (::com::sun::star::uno::RuntimeException)
{

    switch (aEvent.EventId)
    {
    case AccessibleEventId::NAME_CHANGED:
        HandleNameChangedEvent(aEvent.NewValue);
        break;
    case AccessibleEventId::DESCRIPTION_CHANGED:
        HandleDescriptionChangedEvent(aEvent.NewValue);
        break;
    case AccessibleEventId::STATE_CHANGED:
        HandleStateChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    default:
        break;
    }
}

/**
 *  handle the NAME_CHANGED event
 *  @param  name        the new name with changed.
 */
void AccEventListener::HandleNameChangedEvent(Any name)
{
    if ( pAgent->IsTopWinAcc( pAccessible ) )
    {
        XAccessible* pAccDoc = pAgent->GetAccDocByAccTopWin( pAccessible );
        if ( pAccDoc )
        {
            pAgent->UpdateAccName(pAccDoc);
            pAgent->NotifyAccEvent(UM_EVENT_OBJECT_NAMECHANGE, pAccDoc);
        }
    }

    pAgent->UpdateAccName(pAccessible, name);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_NAMECHANGE, pAccessible);
}

/**
 *  handle the DESCRIPTION_CHANGED event
 *  @param  desc        the new description
 */
void AccEventListener::HandleDescriptionChangedEvent(Any desc)
{
    pAgent->UpdateDescription(pAccessible, desc);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_DESCRIPTIONCHANGE, pAccessible);
}

/**
 *  handle the BOUNDRECT_CHANGED event
 */
void AccEventListener::HandleBoundrectChangedEvent()
{
    pAgent->UpdateLocation(pAccessible);
    pAgent->NotifyAccEvent(UM_EVENT_BOUNDRECT_CHANGED, pAccessible);
}

/**
 *  handle the VISIBLE_DATA_CHANGED event
 */
void AccEventListener::HandleVisibleDataChangedEvent()
{
    pAgent->UpdateValue(pAccessible);
    pAgent->NotifyAccEvent(UM_EVENT_VISIBLE_DATA_CHANGED, pAccessible);
}

/**
 *  handle the STATE_CHANGED event
 *  @param  oldValue    the old state of the source of event
 *  @param  newValue    the new state of the source of event
 */
void AccEventListener::HandleStateChangedEvent(Any oldValue, Any newValue)
{
    short newV, oldV;
    if( newValue >>= newV)
    {
        SetComponentState(newV, true);
    }
    else if (oldValue >>= oldV)
    {
        SetComponentState(oldV, false);
    }
}

/**
 *  set the new state and fire the MSAA event
 *  @param  state       new state id
 *  @param  enable      true if state is set, false if state is unset
 */
void AccEventListener::SetComponentState(short state, bool enable )
{
    switch (state)
    {
    case AccessibleStateType::FOCUSED:
        FireStateFocusedChange(enable);
        break;
    default:
        FireStatePropertyChange(state, enable);
        break;
    }
}

/**
 *  handle the focused event
 *  @param enable   true if get focus, false if lose focus
 */
void AccEventListener::FireStateFocusedChange(bool enable)
{
    if(enable)
    {
        pAgent->IncreaseState( pAccessible, AccessibleStateType::FOCUSED);
        pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, pAccessible);
    }
    else
    {
        // no focus lost event in MSAA
    }
}


/**
 *  fire the MSAA state changed event
 *  @param state    the state id
 *  @param set      true if state is set, false if state is unset
 */
void AccEventListener::FireStatePropertyChange(short /*state*/, bool set )
{
    if( set )
    {
        //get new state
    }
    else
    {
        //lose old state
    }
}

/**
 *  get the role of accessible object which is observed
 */
short AccEventListener::GetRole()
{
    Reference<com::sun::star::accessibility::XAccessibleContext> xContext(pAccessible->getAccessibleContext(),UNO_QUERY);
    if(xContext.is())
    {
        return xContext->getAccessibleRole();
    }
    return -1;
}

/**
 *  get the role of accessible parent object which is observed
 */
short AccEventListener::GetParentRole()
{
    if(pAccessible)
    {
        return pAgent->GetParentRole(pAccessible);
    }
    return -1;
}
/**
 *  remove the listener from accessible object
 */
void AccEventListener::RemoveMeFromBroadcaster()
{
    try
    {
        osl::MutexGuard aGuard(aRemoveMutex);
        if(m_isDisposed)
            return;
        //get accessible context
        Reference< XAccessibleContext > pRContext;
        XAccessibleContext* pContext =NULL;

        if( pAccessible == NULL)
        {
            return;
        }
        pRContext = pAccessible->getAccessibleContext();
        if( !pRContext.is() )
        {
            return;
        }

        //get broadcaster from accessible component
        Reference<XAccessibleComponent> xComponent(pRContext,UNO_QUERY);
        if(!xComponent.is())
        {
            return;
        }
        Reference<XAccessibleEventBroadcaster> broadcaster(xComponent,UNO_QUERY);
        XAccessibleEventBroadcaster* pBroadcaster = broadcaster.get();
        if (pBroadcaster != NULL)
        {
            //remove the lister from accessible object
            pBroadcaster->removeAccessibleEventListener(this);
            m_isDisposed = true;
            pAgent->NotifyDestroy(pAccessible);
        }
    }
    catch(...)
    {
        return;
    }

}

/**
 *  this method is invoked before listener is disposed
 */
void AccEventListener::disposing( const ::com::sun::star::lang::EventObject& /*Source*/ )
throw (::com::sun::star::uno::RuntimeException)
{
    RemoveMeFromBroadcaster();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
