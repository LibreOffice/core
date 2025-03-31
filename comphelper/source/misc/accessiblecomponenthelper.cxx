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

#include <comphelper/accessiblecomponenthelper.hxx>
#include <comphelper/accessiblecontexthelper.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <comphelper/solarmutex.hxx>


namespace comphelper
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;

    OAccessibleComponentHelper::OAccessibleComponentHelper()
        :WeakComponentImplHelper( GetMutex() )
        ,m_nClientId( 0 )
    {
    }


    OAccessibleComponentHelper::~OAccessibleComponentHelper()
    {
            // this ensures that the lock, which may be already destroyed as part of the derivee,
            // is not used anymore

        ensureDisposed();
    }


    void SAL_CALL OAccessibleComponentHelper::disposing()
    {
        // rhbz#1001768: de facto this class is locked by SolarMutex;
        // do not lock m_Mutex because it may cause deadlock
        osl::Guard<SolarMutex> aGuard(SolarMutex::get());

        if ( m_nClientId )
        {
            AccessibleEventNotifier::revokeClientNotifyDisposing( m_nClientId, *this );
            m_nClientId=0;
        }
    }


    void SAL_CALL OAccessibleComponentHelper::addAccessibleEventListener(const Reference< XAccessibleEventListener >& _rxListener)
    {
        osl::Guard<SolarMutex> aGuard(SolarMutex::get());
            // don't use the OContextEntryGuard - it will throw an exception if we're not alive
            // anymore, while the most recent specification for XComponent states that we should
            // silently ignore the call in such a situation

        if (!_rxListener.is())
            return;

        if ( !isAlive() )
        {
            _rxListener->disposing( EventObject( *this ) );
            return;
        }

        if (!m_nClientId)
            m_nClientId = AccessibleEventNotifier::registerClient();

        AccessibleEventNotifier::addEventListener(m_nClientId, _rxListener);
    }


    void SAL_CALL OAccessibleComponentHelper::removeAccessibleEventListener(const Reference< XAccessibleEventListener >& _rxListener)
    {
        osl::Guard<SolarMutex> aGuard(SolarMutex::get());
            // don't use the OContextEntryGuard - it will throw an exception if we're not alive
            // anymore, while the most recent specification for XComponent states that we should
            // silently ignore the call in such a situation
        if ( !isAlive() )
            return;

        if ( !(_rxListener.is() && m_nClientId) )
            return;

        sal_Int32 nListenerCount = AccessibleEventNotifier::removeEventListener( m_nClientId, _rxListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            AccessibleEventNotifier::revokeClient( m_nClientId );
            m_nClientId = 0;
        }
    }


    void OAccessibleComponentHelper::NotifyAccessibleEvent( const sal_Int16 _nEventId,
        const Any& _rOldValue, const Any& _rNewValue, sal_Int32 nIndexHint )
    {
        if ( !m_nClientId )
            // if we don't have a client id for the notifier, then we don't have listeners, then
            // we don't need to notify anything
            return;

        // build an event object
        AccessibleEventObject aEvent(*this, _nEventId, _rNewValue, _rOldValue, nIndexHint);

        // let the notifier handle this event
        AccessibleEventNotifier::addEvent( m_nClientId, aEvent );
    }

    bool OAccessibleComponentHelper::hasAccessibleListeners() const
    {
        return m_nClientId != 0;
    }

    bool OAccessibleComponentHelper::isAlive() const
    {
        return !rBHelper.bDisposed && !rBHelper.bInDispose;
    }


    void OAccessibleComponentHelper::ensureAlive() const
    {
        if( !isAlive() )
            throw DisposedException();
    }


    void OAccessibleComponentHelper::ensureDisposed( )
    {
        if ( !rBHelper.bDisposed )
        {
            OSL_ENSURE( 0 == m_refCount, "OAccessibleComponentHelper::ensureDisposed: this method _has_ to be called from without your dtor only!" );
            acquire();
            dispose();
        }
    }

    OUString SAL_CALL OAccessibleComponentHelper::getAccessibleId()
    {
        return OUString();
    }


    sal_Int64 SAL_CALL OAccessibleComponentHelper::getAccessibleIndexInParent()
    {
        OExternalLockGuard aGuard( this );

        try
        {
            Reference< XAccessibleContext > xParentContext( implGetParentContext() );
            if (!xParentContext.is())
                return -1;

            //  iterate over parent's children and search for this object
            sal_Int64 nChildCount = xParentContext->getAccessibleChildCount();
            for (sal_Int64 nChild = 0; nChild < nChildCount; ++nChild)
            {
                Reference< XAccessible > xChild( xParentContext->getAccessibleChild( nChild ) );
                if (xChild.is() && xChild->getAccessibleContext().get() == this)
                    return nChild;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL("OAccessibleComponentHelper::getAccessibleIndexInParent: caught an exception!");
        }

        return -1;
    }


    Locale SAL_CALL OAccessibleComponentHelper::getLocale()
    {
        // simply ask the parent
        Reference< XAccessible > xParent = getAccessibleParent();
        Reference< XAccessibleContext > xParentContext;
        if ( xParent.is() )
            xParentContext = xParent->getAccessibleContext();

        if ( !xParentContext.is() )
            throw IllegalAccessibleComponentStateException( OUString(), *this );

        return xParentContext->getLocale();
    }


    Reference< XAccessibleContext > OAccessibleComponentHelper::implGetParentContext()
    {
        Reference< XAccessible > xParent = getAccessibleParent();
        Reference< XAccessibleContext > xParentContext;
        if ( xParent.is() )
            xParentContext = xParent->getAccessibleContext();
        return xParentContext;
    }


    sal_Bool SAL_CALL OAccessibleComponentHelper::containsPoint(const awt::Point& _rPoint)
    {
        OExternalLockGuard aGuard( this );
        awt::Rectangle aBounds( implGetBounds() );
        return  ( _rPoint.X >= 0 )
            &&  ( _rPoint.Y >= 0 )
            &&  ( _rPoint.X < aBounds.Width )
            &&  ( _rPoint.Y < aBounds.Height );
    }


    awt::Point SAL_CALL OAccessibleComponentHelper::getLocation()
    {
        OExternalLockGuard aGuard( this );
        awt::Rectangle aBounds( implGetBounds() );
        return awt::Point( aBounds.X, aBounds.Y );
    }


    awt::Point SAL_CALL OAccessibleComponentHelper::getLocationOnScreen()
    {
        OExternalLockGuard aGuard( this );

        awt::Point aScreenLoc( 0, 0 );

        Reference< XAccessibleComponent > xParentComponent( implGetParentContext(), UNO_QUERY );
        OSL_ENSURE(xParentComponent.is(), "OAccessibleComponentHelper::getLocationOnScreen: no parent component!");
        if ( xParentComponent.is() )
        {
            awt::Point aParentScreenLoc( xParentComponent->getLocationOnScreen() );
            awt::Point aOwnRelativeLoc( getLocation() );
            aScreenLoc.X = aParentScreenLoc.X + aOwnRelativeLoc.X;
            aScreenLoc.Y = aParentScreenLoc.Y + aOwnRelativeLoc.Y;
        }

        return aScreenLoc;
    }


    awt::Size SAL_CALL OAccessibleComponentHelper::getSize()
    {
        OExternalLockGuard aGuard( this );
        awt::Rectangle aBounds( implGetBounds() );
        return awt::Size( aBounds.Width, aBounds.Height );
    }


    awt::Rectangle SAL_CALL OAccessibleComponentHelper::getBounds()
    {
        OExternalLockGuard aGuard( this );
        return implGetBounds();
    }

    OUString SAL_CALL OAccessibleComponentHelper::getTitledBorderText()
    {
        OExternalLockGuard aGuard(this);
        return OUString();
    }

    OUString SAL_CALL OAccessibleComponentHelper::getToolTipText()
    {
        OExternalLockGuard aGuard(this);
        return OUString();
    }

}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
