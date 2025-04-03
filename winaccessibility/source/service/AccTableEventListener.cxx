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
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>

#include <vcl/svapp.hxx>

#include <AccTableEventListener.hxx>
#include <AccObjectWinManager.hxx>
#include <unomsaaevent.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccTableEventListener::AccTableEventListener(css::accessibility::XAccessible* pAcc, AccObjectWinManager& rManager)
        :AccDescendantManagerEventListener(pAcc, rManager)
{}
AccTableEventListener::~AccTableEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccTableEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
        HandleActiveDescendantChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;

    case AccessibleEventId::TABLE_CAPTION_CHANGED:
        {
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::TABLE_CAPTION_CHANGED);
            break;
        }
    case AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
        {
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::TABLE_COLUMN_DESCRIPTION_CHANGED);
            break;
        }
    case AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
        {
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::TABLE_COLUMN_HEADER_CHANGED);
            break;
        }
    case AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
        {
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::TABLE_ROW_HEADER_CHANGED);
            break;
        }
    case AccessibleEventId::TABLE_MODEL_CHANGED:
        {
            HandleTableModelChangeEvent(aEvent.NewValue);
            break;
        }
    case AccessibleEventId::TABLE_SUMMARY_CHANGED:
        {
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::TABLE_SUMMARY_CHANGED);
            break;
        }
    case AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
        {
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::TABLE_ROW_DESCRIPTION_CHANGED);
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
            m_rObjManager.InsertAccObj(pAcc, m_xAccessible.get());
            m_rObjManager.NotifyAccEvent(pAcc, UnoMSAAEvent::ACTIVE_DESCENDANT_CHANGED);
        }
    }
    else if (oldValue >>= xChild)
    {
        //delete an existing child
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            m_rObjManager.DeleteAccObj( pAcc );
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
            m_rObjManager.DeleteChildrenAccObj(m_xAccessible.get());
            //add all oldValue's existing children
            m_rObjManager.InsertChildrenAccObj(m_xAccessible.get());
        }
        m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::TABLE_MODEL_CHANGED);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
