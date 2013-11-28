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

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

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
    switch (aEvent.EventId)
    {
    case AccessibleEventId::VALUE_CHANGED:
        handleValueChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::ACTION_CHANGED:
        handleActionChangedEvent();
        break;
    case AccessibleEventId::TEXT_CHANGED:
        handleTextChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::CARET_CHANGED:
        handleCaretChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::VISIBLE_DATA_CHANGED:
        handleVisibleDataChangedEvent();
        break;
    case AccessibleEventId::BOUNDRECT_CHANGED:
        handleBoundrectChangedEvent();
        break;
    case AccessibleEventId::SELECTION_CHANGED:
        handleSelectionChangedEvent();
        break;
        //to add TEXT_SELECTION_CHANGED event
    case AccessibleEventId::TEXT_SELECTION_CHANGED:
        handleTextSelectionChangedEvent();
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
void AccComponentEventListener::handleValueChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateValue(pAccessible);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_VALUECHANGE, pAccessible);
}

/**
 * handle the NAME_CHANGED event
 */
void AccComponentEventListener::handleActionChangedEvent()
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
void AccComponentEventListener::handleTextChangedEvent(Any oldValue, Any newValue)
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
void AccComponentEventListener::handleCaretChangedEvent(Any oldValue, Any newValue)
{
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_CARETCHANGE, pAccessible);
}

/**
 *  handle the VISIBLE_DATA_CHANGED event
 */
void AccComponentEventListener::handleVisibleDataChangedEvent()
{
    AccEventListener::handleVisibleDataChangedEvent();
}

/**
 *  handle the BOUNDRECT_CHANGED event
 */
void AccComponentEventListener::handleBoundrectChangedEvent()
{
    AccEventListener::handleBoundrectChangedEvent();
}

/**
 * set the new state and fire the MSAA event
 *
 * @param   state   new state id
 * @param   enable  true if state is set, false if state is unset
 */
void AccComponentEventListener::setComponentState(short state, bool enable )
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
        fireStatePropertyChange(state, enable);
        break;
    case AccessibleStateType::VISIBLE:
        if (getRole() == AccessibleRole::MENU_ITEM)
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
            fireStatePropertyChange(state, enable);
        }
        break;
        break;
    case AccessibleStateType::FOCUSED:
        fireStateFocusdChange(enable);
        break;
    case AccessibleStateType::ENABLED:
        if(enable)
        {
            pAgent->UpdateState(pAccessible);
            pAgent->DecreaseState( pAccessible, AccessibleStateType::DEFUNC);
            // 8. label should have no FOCUSABLE state state, Firefox has READONLY state, we can also have.
            if( getRole() != AccessibleRole::LABEL
                && getRole() != AccessibleRole::SCROLL_BAR)                pAgent->IncreaseState( pAccessible, AccessibleStateType::FOCUSABLE);
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
void AccComponentEventListener::fireStatePropertyChange(short state, bool set)
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
void AccComponentEventListener::fireStateFocusdChange(bool enable)
{
    if(enable)
    {
        if(getParentRole() != AccessibleRole::COMBO_BOX )
            pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, pAccessible);
    }
    else
    {
        //if lose focus, no need to send MSAA FOCUS event
        pAgent->DecreaseState( pAccessible, AccessibleStateType::FOCUSED);
    }
}

void AccComponentEventListener::handleSelectionChangedEvent()
{
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, pAccessible);
}

//add TEXT_SELECTION_CHANGED event
void AccComponentEventListener::handleTextSelectionChangedEvent()
{
    pAgent->NotifyAccEvent(UM_EVENT_TEXT_SELECTION_CHANGED, pAccessible);
}
