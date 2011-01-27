/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "documentfocuslistener.hxx"

#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>

#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;


//------------------------------------------------------------------------------

DocumentFocusListener::DocumentFocusListener(AquaA11yFocusTracker& rTracker) :
    m_aFocusTracker(rTracker)
{
}

//------------------------------------------------------------------------------

void SAL_CALL
DocumentFocusListener::disposing( const EventObject& aEvent )
    throw (RuntimeException)
{
    // Unref the object here, but do not remove as listener since the object
    // might no longer be in a state that safely allows this.
    if( aEvent.Source.is() )
        m_aRefList.erase(aEvent.Source);
}

//------------------------------------------------------------------------------

void SAL_CALL
DocumentFocusListener::notifyEvent( const AccessibleEventObject& aEvent )
    throw( RuntimeException )
{
    switch( aEvent.EventId )
    {
        case AccessibleEventId::STATE_CHANGED:
            try
            {
                sal_Int16 nState = AccessibleStateType::INVALID;
                aEvent.NewValue >>= nState;

                if( AccessibleStateType::FOCUSED == nState )
                    m_aFocusTracker.setFocusedObject( getAccessible(aEvent) );
            }
            catch(IndexOutOfBoundsException e)
            {
                OSL_TRACE("Focused object has invalid index in parent");
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
        }

            OSL_TRACE( "Invalidate all children called\n" );
            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------------

Reference< XAccessible > DocumentFocusListener::getAccessible(const EventObject& aEvent )
    throw (IndexOutOfBoundsException, RuntimeException)
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

//------------------------------------------------------------------------------

void DocumentFocusListener::attachRecursive(const Reference< XAccessible >& xAccessible)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XAccessibleContext > xContext = xAccessible->getAccessibleContext();

    if( xContext.is() )
        attachRecursive(xAccessible, xContext);
}

//------------------------------------------------------------------------------

void DocumentFocusListener::attachRecursive(
    const Reference< XAccessible >& xAccessible,
    const Reference< XAccessibleContext >& xContext
)  throw (IndexOutOfBoundsException, RuntimeException)
{
    if( xContext.is() )
    {
        Reference< XAccessibleStateSet > xStateSet = xContext->getAccessibleStateSet();

        if( xStateSet.is() )
            attachRecursive(xAccessible, xContext, xStateSet);
    }
}

//------------------------------------------------------------------------------

void DocumentFocusListener::attachRecursive(
    const Reference< XAccessible >& xAccessible,
    const Reference< XAccessibleContext >& xContext,
    const Reference< XAccessibleStateSet >& xStateSet
) throw (IndexOutOfBoundsException,RuntimeException)
{
    if( xStateSet->contains(AccessibleStateType::FOCUSED ) )
        m_aFocusTracker.setFocusedObject( xAccessible );

    Reference< XAccessibleEventBroadcaster > xBroadcaster =
        Reference< XAccessibleEventBroadcaster >(xContext, UNO_QUERY);

    // If not already done, add the broadcaster to the list and attach as listener.
    if( xBroadcaster.is() && m_aRefList.insert(xBroadcaster).second )
    {
        xBroadcaster->addEventListener(static_cast< XAccessibleEventListener *>(this));

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

//------------------------------------------------------------------------------

void DocumentFocusListener::detachRecursive(const Reference< XAccessible >& xAccessible)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XAccessibleContext > xContext = xAccessible->getAccessibleContext();

    if( xContext.is() )
        detachRecursive(xAccessible, xContext);
}

//------------------------------------------------------------------------------

void DocumentFocusListener::detachRecursive(
    const Reference< XAccessible >& xAccessible,
    const Reference< XAccessibleContext >& xContext
)  throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XAccessibleStateSet > xStateSet = xContext->getAccessibleStateSet();

    if( xStateSet.is() )
        detachRecursive(xAccessible, xContext, xStateSet);
}

//------------------------------------------------------------------------------

void DocumentFocusListener::detachRecursive(
    const Reference< XAccessible >&,
    const Reference< XAccessibleContext >& xContext,
    const Reference< XAccessibleStateSet >& xStateSet
) throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XAccessibleEventBroadcaster > xBroadcaster =
        Reference< XAccessibleEventBroadcaster >(xContext, UNO_QUERY);

    if( xBroadcaster.is() && 0 < m_aRefList.erase(xBroadcaster) )
    {
        xBroadcaster->removeEventListener(static_cast< XAccessibleEventListener *>(this));

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
