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

#include "AccDescendantManagerEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccDescendantManagerEventListener::AccDescendantManagerEventListener(css::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
    :   AccComponentEventListener(pAcc, Agent)
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
throw (css::uno::RuntimeException)
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::SELECTION_CHANGED:
        HandleSelectionChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::CHILD:
        HandleChildChangedEvent(aEvent.OldValue, aEvent.NewValue);
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
 *  handle the CHILD event
 *  @param  oldValue    the child to be deleted
 *  @param  newValue    the child to be added
 */
void AccDescendantManagerEventListener::HandleChildChangedEvent(Any oldValue, Any newValue)
{

    Reference< XAccessible > xChild;
    if( newValue >>= xChild)
    {
        //create a new child
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->InsertAccObj(pAcc, m_xAccessible.get());
            pAgent->InsertChildrenAccObj(pAcc);

            pAgent->NotifyAccEvent(UM_EVENT_CHILD_ADDED, pAcc);

        }
        else
        {}
    }

    if (oldValue >>= xChild)
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();

            pAgent->NotifyAccEvent(UM_EVENT_CHILD_REMOVED, pAcc);
            pAgent->DeleteChildrenAccObj( pAcc );
            pAgent->DeleteAccObj( pAcc );
        }
        else
        {}
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
            //if the Role is the SC cell ,don't add the selected state.
            if (pAgent->GetRole(pAcc) != AccessibleRole::TABLE_CELL)
            {
                pAgent->IncreaseState( pAcc, AccessibleStateType::SELECTED);
            }

            pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, pAcc);
            bSend=true;
        }
    }
    if(oldValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->DecreaseState( pAcc, AccessibleStateType::SELECTED);
        }
    }
    if (!bSend)
    {
        pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, m_xAccessible.get());
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

            pAgent->InsertAccObj(pAcc, m_xAccessible.get());
            pAgent->InsertChildrenAccObj(pAcc);
        }
    }
    if (oldValue >>= xChild)
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->DeleteChildrenAccObj( pAcc );
            pAgent->DeleteAccObj( pAcc );
        }
    }
}

bool AccDescendantManagerEventListener::NotifyChildEvent(short nWinEvent,const Any &Value)
{
    Reference< XAccessible > xChild;
    if(Value >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->NotifyAccEvent(nWinEvent, pAcc);

            if (pAgent && pAgent->IsStateManageDescendant(m_xAccessible.get())
                    && (nWinEvent == UM_EVENT_SELECTION_CHANGED_REMOVE))
            {
                pAgent->DeleteAccObj( pAcc );
            }
            return true;
        }
    }
    return false;
}
void AccDescendantManagerEventListener::HandleSelectionChangedAddEvent(const Any& /*oldValue*/, const Any &newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_ADD,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_ADD, m_xAccessible.get());
}

void AccDescendantManagerEventListener::HandleSelectionChangedRemoveEvent(const Any& /*oldValue*/, const Any &newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_REMOVE,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_REMOVE, m_xAccessible.get());
}

void AccDescendantManagerEventListener::HandleSelectionChangedWithinEvent(const Any& /*oldValue*/, const Any &newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_WITHIN,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_WITHIN, m_xAccessible.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
