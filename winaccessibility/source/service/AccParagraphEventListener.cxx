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

#include "AccParagraphEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccParagraphEventListener::AccParagraphEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccContainerEventListener(pAcc, Agent)
{}
AccParagraphEventListener::~AccParagraphEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccParagraphEventListener::notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
throw (::com::sun::star::uno::RuntimeException)
{
    switch (aEvent.EventId)
    {
    case AccessibleEventId::CARET_CHANGED:
        handleCaretChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::VISIBLE_DATA_CHANGED:
        handleVisibleDataChangedEvent();
        break;
    case AccessibleEventId::BOUNDRECT_CHANGED:
        handleBoundrectChangedEvent();
        break;
        //Added for paragraph selected state.
    case AccessibleEventId::STATE_CHANGED:
        {
            short State;
            if( (aEvent.NewValue >>= State) && (State == AccessibleStateType::SELECTED) )
            {
                pAgent->IncreaseState( pAccessible, State);
                break;
            }
            else if( (aEvent.OldValue >>= State) && (State == AccessibleStateType::SELECTED) )
            {
                pAgent->DecreaseState( pAccessible, State);
                break;
            }

            AccContainerEventListener::notifyEvent(aEvent);
            break;
        }

    case AccessibleEventId::TEXT_SELECTION_CHANGED:
        handleTextSelectionChangedEvent();
        break;

    default:
        AccContainerEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 *  handle the CARET_CHANGED event
 *  @param oldValue     in UNO, this parameter is always NULL
 *  @param newValue     in UNO, this parameter is always NULL
 */
void AccParagraphEventListener::handleCaretChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateLocation(pAccessible);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_CARETCHANGE, pAccessible);
}

/**
 *  handle the VISIBLE_DATA_CHANGED event
 */
void AccParagraphEventListener::handleVisibleDataChangedEvent()
{
    AccEventListener::handleVisibleDataChangedEvent();
}

/**
 *  handle the BOUNDRECT_CHANGED event
 */
void AccParagraphEventListener::handleBoundrectChangedEvent()
{
    AccEventListener::handleBoundrectChangedEvent();
}

/**
 *  set the new state and fire the MSAA event
 *  @param state    new state id
 *  @param enable   true if state is set, false if state is unset
 */
void AccParagraphEventListener::setComponentState(short state, bool enable )
{
    // only the following state can be fired state event.
    switch (state)
    {
    case AccessibleStateType::EDITABLE:
        // no msaa state
        break;
    case AccessibleStateType::MULTI_LINE:
        // no msaa state mapping
        break;
    case AccessibleStateType::SINGLE_LINE:
        // no msaa state mapping
        break;
    default:
        AccContainerEventListener::setComponentState(state, enable);
        break;
    }
}

void AccParagraphEventListener::handleTextSelectionChangedEvent()
{
    pAgent->NotifyAccEvent(UM_EVENT_TEXT_SELECTION_CHANGED, pAccessible);
}

