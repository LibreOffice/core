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

#include <vcl/svapp.hxx>

#include <AccMenuEventListener.hxx>
#include <AccObjectWinManager.hxx>
#include <unomsaaevent.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccMenuEventListener::AccMenuEventListener(css::accessibility::XAccessible* pAcc, AccObjectWinManager& rManager)
        :AccComponentEventListener(pAcc, rManager)
{}
AccMenuEventListener::~AccMenuEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccMenuEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::SELECTION_CHANGED:
        //don't need to process anything,just same as word behavior
        //handleSelectionChangedEvent();
        break;
    default:
        AccComponentEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 *  handle the SELECTION_CHANGED event
 */
void AccMenuEventListener::HandleSelectionChangedEventNoArgs()
{
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED);
}

/**
 *  handle the Menu_popup event
 */
void AccMenuEventListener::FireStatePropertyChange(sal_Int64 state, bool set)
{
    if( set )
    {
        // new value
        switch(state)
        {
            //for sub menu is popup, there is a menu selected event.
        case AccessibleStateType::SELECTED:
            m_rObjManager.IncreaseState(m_xAccessible.get(), state);
            m_rObjManager.UpdateChildState(m_xAccessible.get());
            break;
        default:
            AccComponentEventListener::FireStatePropertyChange(state, set);
            break;
        }
    }
    else
    {
        switch(state)
        {
            //for sub menu is popup, there is a menu selected event.
        case AccessibleStateType::SELECTED:
            m_rObjManager.DecreaseState(m_xAccessible.get(), state);

            break;
        default:
            AccComponentEventListener::FireStatePropertyChange(state, set);
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
