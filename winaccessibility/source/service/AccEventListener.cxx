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

#include <vcl/svapp.hxx>

#include <toolkit/awt/Vclxwindow.hxx>

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
using namespace cppu;

AccEventListener::AccEventListener(css::accessibility::XAccessible* pAcc,
                                   AccObjectManagerAgent* Agent)
    : m_xAccessible(pAcc)
    , pAgent(Agent)
{}

AccEventListener::~AccEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
throw (css::uno::RuntimeException)
{
    SolarMutexGuard g;

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
    if (pAgent->IsTopWinAcc(m_xAccessible.get()))
    {
        XAccessible* pAccDoc = pAgent->GetAccDocByAccTopWin(m_xAccessible.get());
        if ( pAccDoc )
        {
            pAgent->UpdateAccName(pAccDoc);
            pAgent->NotifyAccEvent(UM_EVENT_OBJECT_NAMECHANGE, pAccDoc);
        }
    }

    pAgent->UpdateAccName(m_xAccessible.get(), name);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_NAMECHANGE, m_xAccessible.get());
}

/**
 *  handle the DESCRIPTION_CHANGED event
 *  @param  desc        the new description
 */
void AccEventListener::HandleDescriptionChangedEvent(Any desc)
{
    pAgent->UpdateDescription(m_xAccessible.get(), desc);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_DESCRIPTIONCHANGE, m_xAccessible.get());
}

/**
 *  handle the BOUNDRECT_CHANGED event
 */
void AccEventListener::HandleBoundrectChangedEvent()
{
    pAgent->UpdateLocation(m_xAccessible.get());
    pAgent->NotifyAccEvent(UM_EVENT_BOUNDRECT_CHANGED, m_xAccessible.get());
}

/**
 *  handle the VISIBLE_DATA_CHANGED event
 */
void AccEventListener::HandleVisibleDataChangedEvent()
{
    pAgent->UpdateValue(m_xAccessible.get());
    pAgent->NotifyAccEvent(UM_EVENT_VISIBLE_DATA_CHANGED, m_xAccessible.get());
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
        pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
        pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, m_xAccessible.get());
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
    css::uno::Reference<css::accessibility::XAccessibleContext> const
        xContext(m_xAccessible->getAccessibleContext());
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
    if (m_xAccessible.is())
    {
        return pAgent->GetParentRole(m_xAccessible.get());
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
        if (!m_xAccessible.is())
        {
            return;
        }
        try
        {
            css::uno::Reference<XAccessibleEventBroadcaster> const xBroadcaster(
                    m_xAccessible->getAccessibleContext(), UNO_QUERY);
            if (xBroadcaster.is())
            {
                //remove the lister from accessible object
                xBroadcaster->removeAccessibleEventListener(this);
            }
        }
        catch (Exception const&)
        {   // may throw if it's already disposed - ignore that
        }
        pAgent->NotifyDestroy(m_xAccessible.get());
        m_xAccessible.clear(); // release cyclic reference
    }
    catch(...)
    {
        return;
    }

}

/**
 *  this method is invoked before listener is disposed
 */
void AccEventListener::disposing( const css::lang::EventObject& /*Source*/ )
throw (css::uno::RuntimeException)
{
    SolarMutexGuard g;

    RemoveMeFromBroadcaster();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
