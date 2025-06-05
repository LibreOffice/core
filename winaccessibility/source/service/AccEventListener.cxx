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

#include <AccEventListener.hxx>
#include <AccObjectWinManager.hxx>
#include <unomsaaevent.hxx>

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
                                   AccObjectWinManager& rManager)
    : m_xAccessible(pAcc)
    , m_rObjManager(rManager)
{
}

AccEventListener::~AccEventListener() {}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void AccEventListener::notifyEvent(const css::accessibility::AccessibleEventObject& aEvent)
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
        case AccessibleEventId::CHILD:
            HandleChildChangedEvent(aEvent.OldValue, aEvent.NewValue);
            break;
        case AccessibleEventId::NAME_CHANGED:
            HandleNameChangedEvent();
            break;
        case AccessibleEventId::DESCRIPTION_CHANGED:
            HandleDescriptionChangedEvent();
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
 */
void AccEventListener::HandleNameChangedEvent()
{
    if (m_rObjManager.IsTopWinAcc(m_xAccessible.get()))
    {
        XAccessible* pAccDoc = m_rObjManager.GetAccDocByAccTopWin(m_xAccessible.get());
        if (pAccDoc)
            m_rObjManager.NotifyAccEvent(pAccDoc, UnoMSAAEvent::OBJECT_NAMECHANGE);
    }

    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_NAMECHANGE);
}

/**
 * Handle the CHILD event
 * @param oldValue the child to be deleted
 * @param newValue the child to be added
 */
void AccEventListener::HandleChildChangedEvent(css::uno::Any oldValue, css::uno::Any newValue)
{
    Reference<XAccessible> xChild;
    if (newValue >>= xChild)
    {
        if (xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            m_rObjManager.InsertAccObj(pAcc, m_xAccessible.get());
            m_rObjManager.InsertChildrenAccObj(pAcc);
            m_rObjManager.NotifyAccEvent(pAcc, UnoMSAAEvent::CHILD_ADDED);
        }
    }
    else if (oldValue >>= xChild)
    {
        if (xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            m_rObjManager.NotifyAccEvent(pAcc, UnoMSAAEvent::CHILD_REMOVED);
            m_rObjManager.DeleteChildrenAccObj(pAcc);
            m_rObjManager.DeleteAccObj(pAcc);
        }
    }
}

/**
 *  handle the DESCRIPTION_CHANGED event
 */
void AccEventListener::HandleDescriptionChangedEvent()
{
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_DESCRIPTIONCHANGE);
}

/**
 *  handle the BOUNDRECT_CHANGED event
 */
void AccEventListener::HandleBoundrectChangedEvent()
{
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::BOUNDRECT_CHANGED);
}

/**
 *  handle the VISIBLE_DATA_CHANGED event
 */
void AccEventListener::HandleVisibleDataChangedEvent()
{
    m_rObjManager.UpdateValue(m_xAccessible.get());
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::VISIBLE_DATA_CHANGED);
}

/**
 *  handle the STATE_CHANGED event
 *  @param  oldValue    the old state of the source of event
 *  @param  newValue    the new state of the source of event
 */
void AccEventListener::HandleStateChangedEvent(Any oldValue, Any newValue)
{
    sal_Int64 newV, oldV;
    if (newValue >>= newV)
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
void AccEventListener::SetComponentState(sal_Int64 state, bool enable)
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
    if (enable)
    {
        m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
        m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_FOCUSED);
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
void AccEventListener::FireStatePropertyChange(sal_Int64 /*state*/, bool set)
{
    if (set)
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
    css::uno::Reference<css::accessibility::XAccessibleContext> const xContext(
        m_xAccessible->getAccessibleContext());
    if (xContext.is())
    {
        return xContext->getAccessibleRole();
    }
    return AccessibleRole::UNKNOWN;
}

/**
 *  get the role of accessible parent object which is observed
 */
short AccEventListener::GetParentRole()
{
    if (m_xAccessible.is())
    {
        return m_rObjManager.GetParentRole(m_xAccessible.get());
    }
    return -1;
}
/**
 *  remove the listener from accessible object
 */
void AccEventListener::RemoveMeFromBroadcaster()
{
    if (!m_xAccessible.is())
        return;

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
    { // may throw if it's already disposed - ignore that
    }

    m_xAccessible.clear(); // release cyclic reference
}

/**
 *  this method is invoked before broadcaster is disposed
 */
void AccEventListener::disposing(const css::lang::EventObject& /*Source*/)
{
    SolarMutexGuard g;
    // No method should be invoked anymore on broadcaster!
    m_xAccessible.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
