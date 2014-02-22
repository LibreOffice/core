/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include <vcl/svapp.hxx>

#include "AccMenuEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccMenuEventListener::AccMenuEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccComponentEventListener(pAcc, Agent)
{}
AccMenuEventListener::~AccMenuEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccMenuEventListener::notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::CHILD:
        HandleChildChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED:
        
        
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
void AccMenuEventListener::HandleChildChangedEvent(Any oldValue, Any newValue)
{

    Reference< XAccessible > xChild;
    if( newValue >>= xChild)
    {
        
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
    else if (oldValue >>= xChild)
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
void AccMenuEventListener::HandleSelectionChangedEventNoArgs()
{
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, m_xAccessible.get());
}

/**
 *  handle the Menu_popup event
 */
void AccMenuEventListener::FireStatePropertyChange(short state, bool set)
{
    if( set )
    {
        
        switch(state)
        {
            
        case AccessibleStateType::SELECTED:
            pAgent->IncreaseState(m_xAccessible.get(), state);
            pAgent->UpdateChildState(m_xAccessible.get());
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
            
        case AccessibleStateType::SELECTED:
            pAgent->DecreaseState(m_xAccessible.get(), state);

            break;
        default:
            AccComponentEventListener::FireStatePropertyChange(state, set);
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
