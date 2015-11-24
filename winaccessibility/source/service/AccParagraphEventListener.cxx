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

#include "AccParagraphEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccParagraphEventListener::AccParagraphEventListener(css::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccContainerEventListener(pAcc, Agent)
{}
AccParagraphEventListener::~AccParagraphEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccParagraphEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
throw (css::uno::RuntimeException)
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::CARET_CHANGED:
        HandleCaretChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::VISIBLE_DATA_CHANGED:
        HandleVisibleDataChangedEvent();
        break;
    case AccessibleEventId::BOUNDRECT_CHANGED:
        HandleBoundrectChangedEvent();
        break;
        //Added for paragraph selected state.
    case AccessibleEventId::STATE_CHANGED:
        {
            short State;
            if( (aEvent.NewValue >>= State) && (State == AccessibleStateType::SELECTED) )
            {
                pAgent->IncreaseState(m_xAccessible.get(), State);
                break;
            }
            else if( (aEvent.OldValue >>= State) && (State == AccessibleStateType::SELECTED) )
            {
                pAgent->DecreaseState(m_xAccessible.get(), State);
                break;
            }

            AccContainerEventListener::notifyEvent(aEvent);
            break;
        }

    case AccessibleEventId::TEXT_SELECTION_CHANGED:
        HandleTextSelectionChangedEvent();
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
void AccParagraphEventListener::HandleCaretChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateLocation(m_xAccessible.get());
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_CARETCHANGE, m_xAccessible.get());
}

/**
 *  set the new state and fire the MSAA event
 *  @param state    new state id
 *  @param enable   true if state is set, false if state is unset
 */
void AccParagraphEventListener::SetComponentState(short state, bool enable )
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
        AccContainerEventListener::SetComponentState(state, enable);
        break;
    }
}

void AccParagraphEventListener::HandleTextSelectionChangedEvent()
{
    pAgent->NotifyAccEvent(UM_EVENT_TEXT_SELECTION_CHANGED, m_xAccessible.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
