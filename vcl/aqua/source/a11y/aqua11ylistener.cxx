/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "aqua/salinst.h"
#include "aqua/aqua11ylistener.hxx"
#include "aqua/aqua11yfactory.h"
#include "aqua/aqua11yfocustracker.hxx"
#include "aqua/aqua11ywrapper.h"

#include "aqua11ytextwrapper.h"

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

//------------------------------------------------------------------------------

AquaA11yEventListener::AquaA11yEventListener(id wrapperObject, sal_Int16 role) : m_wrapperObject(wrapperObject), m_role(role)
{
    [ m_wrapperObject retain ];
}

//------------------------------------------------------------------------------

AquaA11yEventListener::~AquaA11yEventListener()
{
    [ m_wrapperObject release ];
}

//------------------------------------------------------------------------------

void SAL_CALL
AquaA11yEventListener::disposing( const EventObject& ) throw( RuntimeException )
{
    [ AquaA11yFactory removeFromWrapperRepositoryFor: [ (AquaA11yWrapper *) m_wrapperObject accessibleContext ] ];
}

//------------------------------------------------------------------------------

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
