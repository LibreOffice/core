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

AccComponentEventListener::AccComponentEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
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
void  AccComponentEventListener::notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
throw (::com::sun::star::uno::RuntimeException)
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
    pAgent->UpdateValue(pAccessible);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_VALUECHANGE, pAccessible);
}

/**
 * handle the NAME_CHANGED event
 */
void AccComponentEventListener::HandleActionChangedEvent()
{
    pAgent->UpdateAction(pAccessible);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_DEFACTIONCHANGE, pAccessible);
}

/**
 * handle the TEXT_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccComponentEventListener::HandleTextChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateValue(pAccessible, newValue);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_VALUECHANGE, pAccessible);
}

/**
 * handle the CARET_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccComponentEventListener::HandleCaretChangedEvent(Any oldValue, Any newValue)
{
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_CARETCHANGE, pAccessible);
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
                pAgent->IncreaseState( pAccessible, AccessibleStateType::VISIBLE);
                pAgent->IncreaseState( pAccessible, AccessibleStateType::FOCUSABLE);
            }
            else
            {
                pAgent->DecreaseState( pAccessible, AccessibleStateType::VISIBLE);
                pAgent->DecreaseState( pAccessible, AccessibleStateType::FOCUSABLE);
            }
        }
        else
        {
            FireStatePropertyChange(state, enable);
        }
        break;
        break;
    case AccessibleStateType::FOCUSED:
        FireStateFocusedChange(enable);
        break;
    case AccessibleStateType::ENABLED:
        if(enable)
        {
            pAgent->UpdateState(pAccessible);
            pAgent->DecreaseState( pAccessible, AccessibleStateType::DEFUNC);
            // 8. label should have no FOCUSABLE state state, Firefox has READONLY state, we can also have.
            if (    GetRole() != AccessibleRole::LABEL
                &&  GetRole() != AccessibleRole::SCROLL_BAR)
            {
                pAgent->IncreaseState( pAccessible, AccessibleStateType::FOCUSABLE);
            }
        }
        else
        {
            pAgent->UpdateState(pAccessible);
            pAgent->IncreaseState( pAccessible, AccessibleStateType::DEFUNC);
            pAgent->DecreaseState( pAccessible, AccessibleStateType::FOCUSABLE);
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
                pAgent->IncreaseState( pAccessible, state);
                pAgent->UpdateAction( pAccessible );

                if(!pAgent->IsSpecialToolboItem(pAccessible))
                {
                    pAgent->NotifyAccEvent(UM_EVENT_STATE_CHECKED, pAccessible);
                }
                break;
            case AccessibleStateType::PRESSED:
                pAgent->IncreaseState( pAccessible, state);
                pAgent->NotifyAccEvent(UM_EVENT_STATE_PRESSED, pAccessible);
                break;
            case AccessibleStateType::SELECTED:
                pAgent->IncreaseState( pAccessible, state);
                break;
            case AccessibleStateType::ARMED:
                pAgent->IncreaseState( pAccessible, state);
                pAgent->NotifyAccEvent(UM_EVENT_STATE_ARMED, pAccessible);
                break;
            case AccessibleStateType::SHOWING:
                pAgent->DecreaseState( pAccessible, AccessibleStateType::DEFUNC);
                // UNO !SHOWING == MSAA OFFSCREEN
                pAgent->IncreaseState( pAccessible, AccessibleStateType::SHOWING );
                break;
            case AccessibleStateType::VISIBLE:
                // UNO !VISIBLE == MSAA INVISIBLE
                pAgent->IncreaseState( pAccessible, AccessibleStateType::VISIBLE );
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
            pAgent->DecreaseState( pAccessible, state );
            pAgent->UpdateAction( pAccessible );

            if(!pAgent->IsSpecialToolboItem(pAccessible))
            {
                pAgent->NotifyAccEvent(UM_EVENT_STATE_CHECKED, pAccessible);
            }
            break;
        case AccessibleStateType::PRESSED:
            pAgent->DecreaseState( pAccessible, state );
            pAgent->NotifyAccEvent(UM_EVENT_STATE_PRESSED, pAccessible);
            break;
        case AccessibleStateType::SELECTED:
            pAgent->DecreaseState( pAccessible, state );
            //if the state is unset, no need to send MSAA SELECTION event
            //pAgent->NotifyAccEvent(UM_EVENT_STATE_SELECTED, pAccessible);
            break;
        case AccessibleStateType::ARMED:
            {
                pAgent->DecreaseState( pAccessible, state);
                //if the state is unset, no need to send MSAA MENU event
                //pAgent->NotifyAccEvent(UM_EVENT_STATE_ARMED, pAccessible);
            }
            break;
        case AccessibleStateType::SHOWING:
            pAgent->DecreaseState( pAccessible, AccessibleStateType::DEFUNC);
            // UNO !SHOWING == MSAA OFFSCREEN
            pAgent->DecreaseState( pAccessible, AccessibleStateType::SHOWING );
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            pAgent->DecreaseState( pAccessible, AccessibleStateType::VISIBLE );
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
            pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, pAccessible);
    }
    else
    {
        //if lose focus, no need to send MSAA FOCUS event
        pAgent->DecreaseState( pAccessible, AccessibleStateType::FOCUSED);
    }
}

void AccComponentEventListener::HandleSelectionChangedEventNoArgs()
{
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, pAccessible);
}

//add TEXT_SELECTION_CHANGED event
void AccComponentEventListener::HandleTextSelectionChangedEvent()
{
    pAgent->NotifyAccEvent(UM_EVENT_TEXT_SELECTION_CHANGED, pAccessible);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
