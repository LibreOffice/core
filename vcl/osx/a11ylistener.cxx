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

#include "osx/salinst.h"
#include "osx/a11ylistener.hxx"
#include "osx/a11yfactory.h"
#include "osx/a11yfocustracker.hxx"
#include "osx/a11ywrapper.h"

#include "a11ytextwrapper.h"

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

NSString * getTableNotification( const AccessibleEventObject& aEvent )
{
    AccessibleTableModelChange aChange;
    NSString * notification = nil;

    if( (aEvent.NewValue >>= aChange) &&
        ( AccessibleTableModelChangeType::INSERT == aChange.Type || AccessibleTableModelChangeType::DELETE == aChange.Type ) &&
        aChange.FirstRow != aChange.LastRow )
    {
        notification = NSAccessibilityRowCountChangedNotification;
    }

    return notification;
}



AquaA11yEventListener::AquaA11yEventListener(id wrapperObject, sal_Int16 role) : m_wrapperObject(wrapperObject), m_role(role)
{
    [ m_wrapperObject retain ];
}



AquaA11yEventListener::~AquaA11yEventListener()
{
    [ m_wrapperObject release ];
}



void SAL_CALL
AquaA11yEventListener::disposing( const EventObject& ) throw( RuntimeException )
{
    [ AquaA11yFactory removeFromWrapperRepositoryFor: [ (AquaA11yWrapper *) m_wrapperObject accessibleContext ] ];
}



void SAL_CALL
AquaA11yEventListener::notifyEvent( const AccessibleEventObject& aEvent ) throw( RuntimeException )
{
    NSString * notification = nil;
    id element = m_wrapperObject;
    Rectangle bounds;

    // TODO: NSAccessibilityValueChanged, NSAccessibilitySelectedRowsChangedNotification
    switch( aEvent.EventId )
    {
        case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
            if( m_role != AccessibleRole::LIST ) {
                Reference< XAccessible > xAccessible;
                if( aEvent.NewValue >>= xAccessible )
                    AquaA11yFocusTracker::get().setFocusedObject( xAccessible );
            }
            break;

        case AccessibleEventId::NAME_CHANGED:
            notification = NSAccessibilityTitleChangedNotification;
            break;

        case AccessibleEventId::CHILD:
            // only needed for tooltips (says Apple)
            if ( m_role == AccessibleRole::TOOL_TIP ) {
                if(aEvent.NewValue.hasValue()) {
                    notification = NSAccessibilityCreatedNotification;
                } else if(aEvent.OldValue.hasValue()) {
                    notification = NSAccessibilityUIElementDestroyedNotification;
                }
            }
            break;

        case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
            // TODO: depricate or remember all children
            break;

        case AccessibleEventId::BOUNDRECT_CHANGED:
            bounds = [ element accessibleComponent ] -> getBounds();
            if ( m_oldBounds.X != 0 && ( bounds.X != m_oldBounds.X || bounds.Y != m_oldBounds.Y ) ) {
                NSAccessibilityPostNotification(element, NSAccessibilityMovedNotification); // post directly since both cases can happen simultaneously
            }
            if ( m_oldBounds.X != 0 && ( bounds.Width != m_oldBounds.Width || bounds.Height != m_oldBounds.Height ) ) {
                NSAccessibilityPostNotification(element, NSAccessibilityResizedNotification); // post directly since both cases can happen simultaneously
            }
            m_oldBounds = bounds;
            break;

        case AccessibleEventId::SELECTION_CHANGED:
            notification = NSAccessibilitySelectedChildrenChangedNotification;
            break;

        case AccessibleEventId::TEXT_SELECTION_CHANGED:
            notification = NSAccessibilitySelectedTextChangedNotification;
            break;

        case AccessibleEventId::TABLE_MODEL_CHANGED:
            notification = getTableNotification(aEvent);
            break;

        case AccessibleEventId::CARET_CHANGED:
            notification = NSAccessibilitySelectedTextChangedNotification;
            break;

        case AccessibleEventId::TEXT_CHANGED:
            notification = NSAccessibilityValueChangedNotification;
            break;

        default:
            break;
    }

    if( nil != notification )
        NSAccessibilityPostNotification(element, notification);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
