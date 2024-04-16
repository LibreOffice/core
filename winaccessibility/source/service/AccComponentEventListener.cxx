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

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include <vcl/svapp.hxx>

#include <AccComponentEventListener.hxx>
#include <AccObjectWinManager.hxx>
#include <unomsaaevent.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccComponentEventListener::AccComponentEventListener(css::accessibility::XAccessible* pAcc, AccObjectWinManager& rManager)
        :AccEventListener(pAcc, rManager)
{
}

AccComponentEventListener::~AccComponentEventListener()
{
}

/**
 * Uno's event notifier when event is captured
 *
 * @param   AccessibleEventObject   the event object which contains information about event
 */
void  AccComponentEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::VALUE_CHANGED:
        HandleValueChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::ACTION_CHANGED:
        HandleActionChangedEvent();
        break;
    case AccessibleEventId::TEXT_CHANGED:
        HandleTextChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::CARET_CHANGED:
        HandleCaretChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::VISIBLE_DATA_CHANGED:
        HandleVisibleDataChangedEvent();
        break;
    case AccessibleEventId::BOUNDRECT_CHANGED:
        HandleBoundrectChangedEvent();
        break;
    case AccessibleEventId::SELECTION_CHANGED:
        HandleSelectionChangedEventNoArgs();
        break;
        //to add TEXT_SELECTION_CHANGED event
    case AccessibleEventId::TEXT_SELECTION_CHANGED:
        HandleTextSelectionChangedEvent();
        break;
        //End
    default:
        AccEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 * handle the VALUE_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccComponentEventListener::HandleValueChangedEvent(Any, Any)
{
    m_rObjManager.UpdateValue(m_xAccessible.get());
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_VALUECHANGE);
}

/**
 * handle the NAME_CHANGED event
 */
void AccComponentEventListener::HandleActionChangedEvent()
{
    m_rObjManager.UpdateAction(m_xAccessible.get());
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_DEFACTIONCHANGE);
}

/**
 * handle the TEXT_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccComponentEventListener::HandleTextChangedEvent(Any, Any newValue)
{
    m_rObjManager.SetValue(m_xAccessible.get(), newValue);
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_VALUECHANGE);
}

/**
 * handle the CARET_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccComponentEventListener::HandleCaretChangedEvent(Any, Any)
{
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_CARETCHANGE);
}

/**
 * set the new state and fire the MSAA event
 *
 * @param   state   new state id
 * @param   enable  true if state is set, false if state is unset
 */
void AccComponentEventListener::SetComponentState(sal_Int64 state, bool enable)
{
    // only the following state can be fired state event.
    switch (state)
    {
    case AccessibleStateType::CHECKED:
    case AccessibleStateType::PRESSED:
    case AccessibleStateType::SELECTED:
    case AccessibleStateType::ARMED:
    case AccessibleStateType::INDETERMINATE:
    case AccessibleStateType::SHOWING:
        FireStatePropertyChange(state, enable);
        break;
    case AccessibleStateType::VISIBLE:
        if (GetRole() == AccessibleRole::MENU_ITEM)
        {
            if(enable)
            {
                m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
                m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
            }
            else
            {
                m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
                m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
            }
        }
        else
        {
            FireStatePropertyChange(state, enable);
        }
        break;
    case AccessibleStateType::FOCUSED:
        FireStateFocusedChange(enable);
        break;
    case AccessibleStateType::ENABLED:
        if(enable)
        {
            m_rObjManager.UpdateState(m_xAccessible.get());
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            // 8. label should have no FOCUSABLE state, Firefox has READONLY state, we can also have.
            if (    GetRole() != AccessibleRole::LABEL
                &&  GetRole() != AccessibleRole::STATIC
                &&  GetRole() != AccessibleRole::NOTIFICATION
                &&  GetRole() != AccessibleRole::SCROLL_BAR)
            {
                m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
            }
        }
        else
        {
            m_rObjManager.UpdateState(m_xAccessible.get());
            m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
        }
        break;
    case AccessibleStateType::ACTIVE:
        // Only frames should be active
        // no msaa state mapping
        break;
    default:
        break;
    }
}

/**
 * fire the MSAA state changed event
 *
 * @param   state   the state id
 * @param   set     true if state is set, false if state is unset
 */
void AccComponentEventListener::FireStatePropertyChange(sal_Int64 state, bool set)
{
    if (!m_xAccessible.is())
        return;

    css::uno::Reference<css::accessibility::XAccessibleContext> xAccContext = m_xAccessible->getAccessibleContext();
    if (!xAccContext.is())
        return;

    const sal_Int16 nRole = xAccContext->getAccessibleRole();
    // for these button roles, MSAA state STATE_SYSTEM_PRESSED is used instead of
    // STATE_SYSTEM_CHECKED (s. AccObject::GetMSAAStateFromUNO)
    const bool bPressedInsteadOfChecked
        = (nRole == AccessibleRole::PUSH_BUTTON) || (nRole == AccessibleRole::TOGGLE_BUTTON);

    if( set)
    {
        // new value
        switch(state)
        {
        case AccessibleStateType::CHECKED:
        case AccessibleStateType::INDETERMINATE:
            m_rObjManager.IncreaseState(m_xAccessible.get(), state);
            m_rObjManager.UpdateAction(m_xAccessible.get());
            if (bPressedInsteadOfChecked)
                m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_PRESSED);
            else
                m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_CHECKED);
            break;
        case AccessibleStateType::PRESSED:
            m_rObjManager.IncreaseState(m_xAccessible.get(), state);
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_PRESSED);
            break;
        case AccessibleStateType::SELECTED:
            m_rObjManager.IncreaseState(m_xAccessible.get(), state);
            break;
        case AccessibleStateType::ARMED:
            m_rObjManager.IncreaseState(m_xAccessible.get(), state);
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_ARMED);
            break;
        case AccessibleStateType::SHOWING:
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            // UNO !SHOWING == MSAA OFFSCREEN
            m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::SHOWING);
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_SHOWING);
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
            break;
        default:
            break;
        }
    }
    else
    {
        // old value
        switch(state)
        {
        case AccessibleStateType::CHECKED:
        case AccessibleStateType::INDETERMINATE:
            m_rObjManager.DecreaseState(m_xAccessible.get(), state);
            m_rObjManager.UpdateAction(m_xAccessible.get());
            if (bPressedInsteadOfChecked)
                m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_PRESSED);
            else
                m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_CHECKED);
            break;
        case AccessibleStateType::PRESSED:
            m_rObjManager.DecreaseState(m_xAccessible.get(), state);
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_PRESSED);
            break;
        case AccessibleStateType::SELECTED:
            m_rObjManager.DecreaseState(m_xAccessible.get(), state);
            //if the state is unset, no need to send MSAA SELECTION event
            //m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_SELECTED);
            break;
        case AccessibleStateType::ARMED:
            {
                m_rObjManager.DecreaseState(m_xAccessible.get(), state);
                //if the state is unset, no need to send MSAA MENU event
                //m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_ARMED);
            }
            break;
        case AccessibleStateType::SHOWING:
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            // UNO !SHOWING == MSAA OFFSCREEN
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::SHOWING);
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
            break;

        default:
            break;
        }
    }
}

/**
 * handle the focused event
 *
 * @param   enable  true if get focus, false if lose focus
 */
void AccComponentEventListener::FireStateFocusedChange(bool enable)
{
    if(enable)
    {
        if (GetParentRole() != AccessibleRole::COMBO_BOX)
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_FOCUSED);
    }
    else
    {
        //if lose focus, no need to send MSAA FOCUS event
        m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
    }
}

void AccComponentEventListener::HandleSelectionChangedEventNoArgs()
{
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED);
}

//add TEXT_SELECTION_CHANGED event
void AccComponentEventListener::HandleTextSelectionChangedEvent()
{
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::TEXT_SELECTION_CHANGED);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
