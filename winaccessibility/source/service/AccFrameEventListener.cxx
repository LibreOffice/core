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

#include <AccFrameEventListener.hxx>
#include <AccObjectWinManager.hxx>
#include <unomsaaevent.hxx>

#include <vcl/window.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/sysdata.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccFrameEventListener::AccFrameEventListener(css::accessibility::XAccessible* pAcc, AccObjectWinManager& rManager)
        :AccEventListener(pAcc, rManager)
{
}

AccFrameEventListener::~AccFrameEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccFrameEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::VISIBLE_DATA_CHANGED:
        HandleVisibleDataChangedEvent();
        break;
    case AccessibleEventId::BOUNDRECT_CHANGED:
        HandleBoundrectChangedEvent();
        break;
    default:
        AccEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 *  handle the CHILD event
 *  @param  oldValue    the child to be deleted
 *  @param  newValue    the child to be added
 */
void AccFrameEventListener::HandleChildChangedEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;
    if( newValue >>= xChild)
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            VCLXWindow* pvclwindow = dynamic_cast<VCLXWindow*>(m_xAccessible.get());
            assert(pvclwindow);
            const SystemEnvData* systemdata
                = pvclwindow->GetWindow()->GetSystemData();

            m_rObjManager.InsertAccObj(pAcc, m_xAccessible.get(), systemdata->hWnd);
            m_rObjManager.InsertChildrenAccObj(pAcc);
            m_rObjManager.NotifyAccEvent(pAcc, UnoMSAAEvent::CHILD_ADDED);
        }
    }
    else if (oldValue >>= xChild)
    {
        AccEventListener::HandleChildChangedEvent(oldValue, newValue);
    }
}

/**
 *  set the new state and fire the MSAA event
 *  @param state    new state id
 *  @param enable   true if state is set, false if state is unset
 */
void AccFrameEventListener::SetComponentState(sal_Int64 state, bool enable )
{
    // only the following state can be fired state event.
    switch (state)
    {
    case AccessibleStateType::ICONIFIED:
        // no msaa state
        break;
    case AccessibleStateType::VISIBLE:
        // UNO !VISIBLE == MSAA INVISIBLE
        if( enable )
            m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
        else
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
        break;
    case AccessibleStateType::ACTIVE:
        // Only frames should be active
        // no msaa state mapping
        break;
    default:
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
