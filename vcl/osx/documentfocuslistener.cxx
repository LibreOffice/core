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

#include "documentfocuslistener.hxx"

#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <osl/diagnose.h>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

DocumentFocusListener::DocumentFocusListener(AquaA11yFocusTracker& rTracker) :
    m_aFocusTracker(rTracker)
{
}

void SAL_CALL
DocumentFocusListener::disposing( const EventObject& aEvent )
{
    // Unref the object here, but do not remove as listener since the object
    // might no longer be in a state that safely allows this.
    if( aEvent.Source.is() )
        m_aRefList.erase(aEvent.Source);
}

void SAL_CALL
DocumentFocusListener::notifyEvent( const AccessibleEventObject& aEvent )
{
    try {
        switch( aEvent.EventId )
        {
            case AccessibleEventId::STATE_CHANGED:
            {
                sal_Int16 nState = AccessibleStateType::INVALID;
                aEvent.NewValue >>= nState;

                if( AccessibleStateType::FOCUSED == nState )
                    m_aFocusTracker.setFocusedObject( getAccessible(aEvent) );
            }
            break;

            case AccessibleEventId::CHILD:
            {
                Reference< XAccessible > xChild;
                if( (aEvent.OldValue >>= xChild) && xChild.is() )
                    detachRecursive(xChild);

                if( (aEvent.NewValue >>= xChild) && xChild.is() )
                    attachRecursive(xChild);
            }
            break;

            case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
            {
                Reference< XAccessible > xAccessible( getAccessible(aEvent) );
                detachRecursive(xAccessible);
                attachRecursive(xAccessible);
                SAL_INFO("vcl", "Invalidate all children called" );
            }
            break;

            default:
                break;
        }
    }
    catch (const IndexOutOfBoundsException&)
    {
        SAL_WARN("vcl", "Focused object has invalid index in parent");
    }
}

Reference< XAccessible > DocumentFocusListener::getAccessible(const EventObject& aEvent )
{
    Reference< XAccessible > xAccessible(aEvent.Source, UNO_QUERY);

    if( xAccessible.is() )
        return xAccessible;

    Reference< XAccessibleContext > xContext(aEvent.Source, UNO_QUERY);

    if( xContext.is() )
    {
        Reference< XAccessible > xParent( xContext->getAccessibleParent() );
        if( xParent.is() )
        {
            Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );
            if( xParentContext.is() )
            {
                return xParentContext->getAccessibleChild( xContext->getAccessibleIndexInParent() );
            }
        }
    }

    return Reference< XAccessible >();
}

void DocumentFocusListener::attachRecursive(const Reference< XAccessible >& xAccessible)
{
    Reference< XAccessibleContext > xContext = xAccessible->getAccessibleContext();

    if( xContext.is() )
        attachRecursive(xAccessible, xContext);
}

void DocumentFocusListener::attachRecursive(
    const Reference< XAccessible >& xAccessible,
    const Reference< XAccessibleContext >& xContext
)
{
    if( xContext.is() )
    {
        Reference< XAccessibleStateSet > xStateSet = xContext->getAccessibleStateSet();

        if( xStateSet.is() )
            attachRecursive(xAccessible, xContext, xStateSet);
    }
}

void DocumentFocusListener::attachRecursive(
    const Reference< XAccessible >& xAccessible,
    const Reference< XAccessibleContext >& xContext,
    const Reference< XAccessibleStateSet >& xStateSet
)
{
    if( xStateSet->contains(AccessibleStateType::FOCUSED ) )
        m_aFocusTracker.setFocusedObject( xAccessible );

    Reference< XAccessibleEventBroadcaster > xBroadcaster =
        Reference< XAccessibleEventBroadcaster >(xContext, UNO_QUERY);

    // If not already done, add the broadcaster to the list and attach as listener.
    if( xBroadcaster.is() && m_aRefList.insert(xBroadcaster).second )
    {
        xBroadcaster->addAccessibleEventListener(static_cast< XAccessibleEventListener *>(this));

        if( ! xStateSet->contains(AccessibleStateType::MANAGES_DESCENDANTS ) )
        {
            sal_Int32 n, nmax = xContext->getAccessibleChildCount();
            for( n = 0; n < nmax; n++ )
            {
                Reference< XAccessible > xChild( xContext->getAccessibleChild( n ) );

                if( xChild.is() )
                    attachRecursive(xChild);
            }
        }
    }
}

void DocumentFocusListener::detachRecursive(const Reference< XAccessible >& xAccessible)
{
    Reference< XAccessibleContext > xContext = xAccessible->getAccessibleContext();

    if( xContext.is() )
        detachRecursive(xAccessible, xContext);
}

void DocumentFocusListener::detachRecursive(
    const Reference< XAccessible >& xAccessible,
    const Reference< XAccessibleContext >& xContext
)
{
    Reference< XAccessibleStateSet > xStateSet = xContext->getAccessibleStateSet();

    if( xStateSet.is() )
        detachRecursive(xAccessible, xContext, xStateSet);
}

void DocumentFocusListener::detachRecursive(
    const Reference< XAccessible >&,
    const Reference< XAccessibleContext >& xContext,
    const Reference< XAccessibleStateSet >& xStateSet
)
{
    Reference< XAccessibleEventBroadcaster > xBroadcaster =
        Reference< XAccessibleEventBroadcaster >(xContext, UNO_QUERY);

    if( xBroadcaster.is() && 0 < m_aRefList.erase(xBroadcaster) )
    {
        xBroadcaster->removeAccessibleEventListener(static_cast< XAccessibleEventListener *>(this));

        if( ! xStateSet->contains(AccessibleStateType::MANAGES_DESCENDANTS ) )
        {
            sal_Int32 n, nmax = xContext->getAccessibleChildCount();
            for( n = 0; n < nmax; n++ )
            {
                Reference< XAccessible > xChild( xContext->getAccessibleChild( n ) );

                if( xChild.is() )
                    detachRecursive(xChild);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
