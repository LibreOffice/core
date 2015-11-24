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

#include "AccComponentEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccComponentEventListener::AccComponentEventListener(css::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccEventListener(pAcc, Agent)
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
throw (css::uno::RuntimeException)
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
void AccComponentEventListener::HandleValueChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateValue(m_xAccessible.get());
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_VALUECHANGE, m_xAccessible.get());
}

/**
 * handle the NAME_CHANGED event
 */
void AccComponentEventListener::HandleActionChangedEvent()
{
    pAgent->UpdateAction(m_xAccessible.get());
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_DEFACTIONCHANGE, m_xAccessible.get());
}

/**
 * handle the TEXT_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccComponentEventListener::HandleTextChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateValue(m_xAccessible.get(), newValue);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_VALUECHANGE, m_xAccessible.get());
}

/**
 * handle the CARET_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccComponentEventListener::HandleCaretChangedEvent(Any oldValue, Any newValue)
{
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_CARETCHANGE, m_xAccessible.get());
}

/**
 * set the new state and fire the MSAA event
 *
 * @param   state   new state id
 * @param   enable  true if state is set, false if state is unset
 */
void AccComponentEventListener::SetComponentState(short state, bool enable)
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
                pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
                pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
            }
            else
            {
                pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
                pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
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
            pAgent->UpdateState(m_xAccessible.get());
            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            // 8. label should have no FOCUSABLE state state, Firefox has READONLY state, we can also have.
            if (    GetRole() != AccessibleRole::LABEL
                &&  GetRole() != AccessibleRole::SCROLL_BAR)
            {
                pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
            }
        }
        else
        {
            pAgent->UpdateState(m_xAccessible.get());
            pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
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
void AccComponentEventListener::FireStatePropertyChange(short state, bool set)
{
    if( set)
        {
            // new value
            switch(state)
            {
            case AccessibleStateType::CHECKED:
            case AccessibleStateType::INDETERMINATE:
                pAgent->IncreaseState(m_xAccessible.get(), state);
                pAgent->UpdateAction(m_xAccessible.get());

                if(!pAgent->IsSpecialToolboItem(m_xAccessible.get()))
                {
                    pAgent->NotifyAccEvent(UM_EVENT_STATE_CHECKED, m_xAccessible.get());
                }
                break;
            case AccessibleStateType::PRESSED:
                pAgent->IncreaseState(m_xAccessible.get(), state);
                pAgent->NotifyAccEvent(UM_EVENT_STATE_PRESSED, m_xAccessible.get());
                break;
            case AccessibleStateType::SELECTED:
                pAgent->IncreaseState(m_xAccessible.get(), state);
                break;
            case AccessibleStateType::ARMED:
                pAgent->IncreaseState(m_xAccessible.get(), state);
                pAgent->NotifyAccEvent(UM_EVENT_STATE_ARMED, m_xAccessible.get());
                break;
            case AccessibleStateType::SHOWING:
                pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
                // UNO !SHOWING == MSAA OFFSCREEN
                pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::SHOWING );
                break;
            case AccessibleStateType::VISIBLE:
                // UNO !VISIBLE == MSAA INVISIBLE
                pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE );
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
            pAgent->DecreaseState(m_xAccessible.get(), state);
            pAgent->UpdateAction(m_xAccessible.get());

            if(!pAgent->IsSpecialToolboItem(m_xAccessible.get()))
            {
                pAgent->NotifyAccEvent(UM_EVENT_STATE_CHECKED, m_xAccessible.get());
            }
            break;
        case AccessibleStateType::PRESSED:
            pAgent->DecreaseState(m_xAccessible.get(), state);
            pAgent->NotifyAccEvent(UM_EVENT_STATE_PRESSED, m_xAccessible.get());
            break;
        case AccessibleStateType::SELECTED:
            pAgent->DecreaseState(m_xAccessible.get(), state);
            //if the state is unset, no need to send MSAA SELECTION event
            //pAgent->NotifyAccEvent(UM_EVENT_STATE_SELECTED, m_xAccessible.get());
            break;
        case AccessibleStateType::ARMED:
            {
                pAgent->DecreaseState(m_xAccessible.get(), state);
                //if the state is unset, no need to send MSAA MENU event
                //pAgent->NotifyAccEvent(UM_EVENT_STATE_ARMED, m_xAccessible.get());
            }
            break;
        case AccessibleStateType::SHOWING:
            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            // UNO !SHOWING == MSAA OFFSCREEN
            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::SHOWING);
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
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
            pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, m_xAccessible.get());
    }
    else
    {
        //if lose focus, no need to send MSAA FOCUS event
        pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
    }
}

void AccComponentEventListener::HandleSelectionChangedEventNoArgs()
{
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, m_xAccessible.get());
}

//add TEXT_SELECTION_CHANGED event
void AccComponentEventListener::HandleTextSelectionChangedEvent()
{
    pAgent->NotifyAccEvent(UM_EVENT_TEXT_SELECTION_CHANGED, m_xAccessible.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
