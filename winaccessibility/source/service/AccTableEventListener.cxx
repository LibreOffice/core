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
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>

#include <vcl/svapp.hxx>

#include "AccTableEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccTableEventListener::AccTableEventListener(css::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccDescendantManagerEventListener(pAcc, Agent)
{}
AccTableEventListener::~AccTableEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccTableEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
throw (css::uno::RuntimeException)
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
        HandleActiveDescendantChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;

    case AccessibleEventId::TABLE_CAPTION_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_CAPTION_CHANGED, m_xAccessible.get());
            break;
        }
    case AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_COLUMN_DESCRIPTION_CHANGED, m_xAccessible.get());
            break;
        }
    case AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_COLUMN_HEADER_CHANGED, m_xAccessible.get());
            break;
        }
    case AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_ROW_HEADER_CHANGED, m_xAccessible.get());
            break;
        }
    case AccessibleEventId::TABLE_MODEL_CHANGED:
        {

            HandleTableModelChangeEvent(aEvent.NewValue);
            break;
        }
    case AccessibleEventId::TABLE_SUMMARY_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_SUMMARY_CHANGED, m_xAccessible.get());
            break;
        }
    case AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_ROW_DESCRIPTION_CHANGED, m_xAccessible.get());
            break;
        }
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
void AccTableEventListener::HandleActiveDescendantChangedEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;
    if(newValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->InsertAccObj(pAcc, m_xAccessible.get());
            pAgent->NotifyAccEvent(UM_EVENT_ACTIVE_DESCENDANT_CHANGED, pAcc);
        }
    }
    else if (oldValue >>= xChild)
    {
        //delete an existing child
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->DeleteAccObj( pAcc );
        }
    }

}
void AccTableEventListener::HandleTableModelChangeEvent(Any newValue)
{
    AccessibleTableModelChange aModelChange;
    if (newValue >>= aModelChange)
    {
        if (m_xAccessible.is())
        {
            //delete all oldValue's existing children
            pAgent->DeleteChildrenAccObj(m_xAccessible.get());
            //add all oldValue's existing children
            pAgent->InsertChildrenAccObj(m_xAccessible.get());
        }
        pAgent->NotifyAccEvent(UM_EVENT_TABLE_MODEL_CHANGED, m_xAccessible.get());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
