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

#include <AccListEventListener.hxx>
#include <AccObjectWinManager.hxx>
#include <unomsaaevent.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccListEventListener::AccListEventListener(css::accessibility::XAccessible* pAcc, AccObjectWinManager& rManager)
        :AccDescendantManagerEventListener(pAcc, rManager)
{
}

AccListEventListener::~AccListEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccListEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
        HandleActiveDescendantChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
        // Since List items a transient a child events are mostly used
        // to attach/detach listeners, it is safe to ignore it here
        //TODO: investigate again
        break;
    case AccessibleEventId::VALUE_CHANGED:
        HandleValueChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    default:
        AccDescendantManagerEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 *  handle the ACTIVE_DESCENDANT_CHANGED event
 *  @param  oldValue    the child to lose active
 *  @param  newValue    the child to get active
 */
void AccListEventListener::HandleActiveDescendantChangedEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;

    if(newValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();

            // Valueset has cache the child item xacc,Update state if no insert obj
            bool bHasCache = m_rObjManager.InsertAccObj(pAcc, m_xAccessible.get());
            if (!bHasCache)
            {
                m_rObjManager.UpdateState(pAcc);
            }

            m_rObjManager.IncreaseState( pAcc, AccessibleStateType::FOCUSED);

            m_rObjManager.NotifyAccEvent(pAcc, UnoMSAAEvent::ACTIVE_DESCENDANT_CHANGED);
        }
    }
    if (oldValue >>= xChild)
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            m_rObjManager.DeleteAccObj( pAcc );
        }
    }
}

/**
 * handle the VALUE_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccListEventListener::HandleValueChangedEvent(Any, Any)
{
    //to enable value changed event
    if (GetParentRole() == AccessibleRole::COMBO_BOX)
    {
        XAccessible* pParentAcc = m_rObjManager.GetParentXAccessible(m_xAccessible.get());
        m_rObjManager.UpdateValue(pParentAcc);
        m_rObjManager.NotifyAccEvent(pParentAcc, UnoMSAAEvent::OBJECT_VALUECHANGE);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
