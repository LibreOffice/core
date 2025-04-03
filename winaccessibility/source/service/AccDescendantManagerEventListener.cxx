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

#include <AccDescendantManagerEventListener.hxx>
#include <AccObjectWinManager.hxx>
#include <unomsaaevent.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccDescendantManagerEventListener::AccDescendantManagerEventListener(css::accessibility::XAccessible* pAcc, AccObjectWinManager& rManager)
    :   AccComponentEventListener(pAcc, rManager)
{
}

AccDescendantManagerEventListener::~AccDescendantManagerEventListener()
{
}

/**
 * Uno's event notifier when event is captured
 * @param AccessibleEventObject     the event object which contains information about event
 */
void  AccDescendantManagerEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::SELECTION_CHANGED:
        HandleSelectionChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS:
        HandleChildChangedNoFocusEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED_ADD:
        HandleSelectionChangedAddEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED_REMOVE:
        HandleSelectionChangedRemoveEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED_WITHIN:
        HandleSelectionChangedWithinEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    default:
        AccComponentEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 *  handle the SELECTION_CHANGED event
 */
void AccDescendantManagerEventListener::HandleSelectionChangedEvent(Any oldValue, Any newValue)
{
    bool bSend =false;
    Reference< XAccessible > xChild;
    if(newValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            Reference<css::accessibility::XAccessibleContext> xContext = pAcc->getAccessibleContext();
            //if the Role is the SC cell ,don't add the selected state.
            if (xContext.is() && xContext->getAccessibleRole() != AccessibleRole::TABLE_CELL)
            {
                m_rObjManager.IncreaseState( pAcc, AccessibleStateType::SELECTED);
            }

            m_rObjManager.NotifyAccEvent(pAcc, UnoMSAAEvent::SELECTION_CHANGED);
            bSend=true;
        }
    }
    if(oldValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            m_rObjManager.DecreaseState( pAcc, AccessibleStateType::SELECTED);
        }
    }
    if (!bSend)
    {
        m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED);
    }
}


void AccDescendantManagerEventListener::HandleChildChangedNoFocusEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;
    if(newValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();

            m_rObjManager.InsertAccObj(pAcc, m_xAccessible.get());
            m_rObjManager.InsertChildrenAccObj(pAcc);
        }
    }
    if (oldValue >>= xChild)
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            m_rObjManager.DeleteChildrenAccObj( pAcc );
            m_rObjManager.DeleteAccObj( pAcc );
        }
    }
}

bool AccDescendantManagerEventListener::NotifyChildEvent(UnoMSAAEvent eWinEvent, const Any& Value)
{
    Reference< XAccessible > xChild;
    if(Value >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            m_rObjManager.NotifyAccEvent(pAcc, eWinEvent);

            if (AccObjectWinManager::IsStateManageDescendant(m_xAccessible.get()))
            {
                if (eWinEvent == UnoMSAAEvent::SELECTION_CHANGED_REMOVE)
                {
                    // The object has just been sent in a SELECTION_CHANGED_REMOVE event
                    // and accessibility tools may query for the object and call methods on
                    // it as a response to this.
                    // Therefore, don't delete the object yet, but remember it for deletion
                    // once the next event of a different type occurs.
                    m_aUnselectedChildrenForDeletion.push_back(pAcc);
                }
                else
                {
                    // handle any pending deletions for objects previously removed from selection
                    for (XAccessible* pAcc2 : m_aUnselectedChildrenForDeletion)
                        m_rObjManager.DeleteAccObj(pAcc2);
                    m_aUnselectedChildrenForDeletion.clear();
                }
            }
            return true;
        }
    }
    return false;
}
void AccDescendantManagerEventListener::HandleSelectionChangedAddEvent(const Any& /*oldValue*/, const Any &newValue)
{
    if (NotifyChildEvent(UnoMSAAEvent::SELECTION_CHANGED_ADD, newValue))
    {
        return ;
    }
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED_ADD);
}

void AccDescendantManagerEventListener::HandleSelectionChangedRemoveEvent(const Any& /*oldValue*/, const Any &newValue)
{
    if (NotifyChildEvent(UnoMSAAEvent::SELECTION_CHANGED_REMOVE, newValue))
    {
        return ;
    }
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED_REMOVE);
}

void AccDescendantManagerEventListener::HandleSelectionChangedWithinEvent(const Any& /*oldValue*/, const Any &newValue)
{
    if (NotifyChildEvent(UnoMSAAEvent::SELECTION_CHANGED_WITHIN, newValue))
    {
        return ;
    }
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED_WITHIN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
