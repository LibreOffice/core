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

#include <comphelper/accessiblecontexthelper.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <comphelper/accessibleeventnotifier.hxx>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;

    IMutex::~IMutex() {}

    /** implementation class for OAccessibleContextHelper. No own thread safety!
    */
    class OContextHelper_Impl
    {
    private:
        IMutex*                             m_pExternalLock;    // the optional additional external lock

        WeakReference< XAccessible >        m_aCreator;         // the XAccessible which created our XAccessibleContext

        AccessibleEventNotifier::TClientId  m_nClientId;

    public:
        inline  Reference< XAccessible >    getCreator( ) const                 { return m_aCreator; }
        inline  void                        setCreator( const Reference< XAccessible >& _rAcc );

        inline  IMutex*                     getExternalLock( )                  { return m_pExternalLock; }
        inline  void                        setExternalLock( IMutex* _pLock )   { m_pExternalLock = _pLock; }

        inline  AccessibleEventNotifier::TClientId
                                            getClientId() const                 { return m_nClientId; }
        inline  void                        setClientId( const AccessibleEventNotifier::TClientId _nId )
                                                                                { m_nClientId = _nId; }

    public:
        OContextHelper_Impl()
            :m_pExternalLock( nullptr )
            ,m_nClientId( 0 )
        {
        }
    };


    inline  void OContextHelper_Impl::setCreator( const Reference< XAccessible >& _rAcc )
    {
        m_aCreator = _rAcc;
    }

    OAccessibleContextHelper::OAccessibleContextHelper( IMutex* _pExternalLock )
        :OAccessibleContextHelper_Base( GetMutex() )
        ,m_pImpl(new OContextHelper_Impl)
    {
        assert(_pExternalLock);
        m_pImpl->setExternalLock( _pExternalLock );
    }


    OAccessibleContextHelper::~OAccessibleContextHelper( )
    {
        /* forgets the reference to the external lock, if present.

           <p>This means any further locking will not be guard the external lock anymore, never.</p>

           <p>To be used in derived classes which do not supply the external lock themself, but instead get
           them passed from own derivees (or clients).</p>
        */
        m_pImpl->setExternalLock( nullptr );

            // this ensures that the lock, which may be already destroyed as part of the derivee,
            // is not used anymore

        ensureDisposed();
    }


    IMutex* OAccessibleContextHelper::getExternalLock( )
    {
        return m_pImpl->getExternalLock();
    }


    void SAL_CALL OAccessibleContextHelper::disposing()
    {
        // rhbz#1001768: de facto this class is locked by SolarMutex;
        // do not lock m_Mutex because it may cause deadlock
        OMutexGuard aGuard( getExternalLock() );

        if ( m_pImpl->getClientId( ) )
        {
            AccessibleEventNotifier::revokeClientNotifyDisposing( m_pImpl->getClientId( ), *this );
            m_pImpl->setClientId( 0 );
        }
    }


    void SAL_CALL OAccessibleContextHelper::addAccessibleEventListener( const Reference< XAccessibleEventListener >& _rxListener )
    {
        OMutexGuard aGuard( getExternalLock() );
            // don't use the OContextEntryGuard - it will throw an exception if we're not alive
            // anymore, while the most recent specification for XComponent states that we should
            // silently ignore the call in such a situation
        if ( !isAlive() )
        {
            if ( _rxListener.is() )
                _rxListener->disposing( EventObject( *this ) );
            return;
        }

        if ( _rxListener.is() )
        {
            if ( !m_pImpl->getClientId( ) )
                m_pImpl->setClientId( AccessibleEventNotifier::registerClient( ) );

            AccessibleEventNotifier::addEventListener( m_pImpl->getClientId( ), _rxListener );
        }
    }


    void SAL_CALL OAccessibleContextHelper::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& _rxListener )
    {
        OMutexGuard aGuard( getExternalLock() );
            // don't use the OContextEntryGuard - it will throw an exception if we're not alive
            // anymore, while the most recent specification for XComponent states that we should
            // silently ignore the call in such a situation
        if ( !isAlive() )
            return;

        if ( _rxListener.is() && m_pImpl->getClientId() )
        {
            sal_Int32 nListenerCount = AccessibleEventNotifier::removeEventListener( m_pImpl->getClientId( ), _rxListener );
            if ( !nListenerCount )
            {
                // no listeners anymore
                // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
                // and at least to us not firing any events anymore, in case somebody calls
                // NotifyAccessibleEvent, again
                AccessibleEventNotifier::revokeClient( m_pImpl->getClientId( ) );
                m_pImpl->setClientId( 0 );
            }
        }
    }


    void OAccessibleContextHelper::NotifyAccessibleEvent( const sal_Int16 _nEventId,
        const Any& _rOldValue, const Any& _rNewValue )
    {
        if ( !m_pImpl->getClientId( ) )
            // if we don't have a client id for the notifier, then we don't have listeners, then
            // we don't need to notify anything
            return;

        // build an event object
        AccessibleEventObject aEvent;
        aEvent.Source = *this;
        aEvent.EventId = _nEventId;
        aEvent.OldValue = _rOldValue;
        aEvent.NewValue = _rNewValue;

        // let the notifier handle this event
        AccessibleEventNotifier::addEvent( m_pImpl->getClientId( ), aEvent );
    }


    bool OAccessibleContextHelper::isAlive() const
    {
        return !rBHelper.bDisposed && !rBHelper.bInDispose;
    }


    void OAccessibleContextHelper::ensureAlive() const
    {
        if( !isAlive() )
            throw DisposedException();
    }


    void OAccessibleContextHelper::ensureDisposed( )
    {
        if ( !rBHelper.bDisposed )
        {
            OSL_ENSURE( 0 == m_refCount, "OAccessibleContextHelper::ensureDisposed: this method _has_ to be called from without your dtor only!" );
            acquire();
            dispose();
        }
    }


    void OAccessibleContextHelper::lateInit( const Reference< XAccessible >& _rxAccessible )
    {
        m_pImpl->setCreator( _rxAccessible );
    }


    Reference< XAccessible > OAccessibleContextHelper::getAccessibleCreator( ) const
    {
        return m_pImpl->getCreator();
    }


    sal_Int32 SAL_CALL OAccessibleContextHelper::getAccessibleIndexInParent(  )
    {
        OExternalLockGuard aGuard( this );

        // -1 for child not found/no parent (according to specification)
        sal_Int32 nRet = -1;

        try
        {

            Reference< XAccessibleContext > xParentContext( implGetParentContext() );

            //  iterate over parent's children and search for this object
            if ( xParentContext.is() )
            {
                // our own XAccessible for comparing with the children of our parent
                Reference< XAccessible > xCreator( m_pImpl->getCreator() );

                OSL_ENSURE( xCreator.is(), "OAccessibleContextHelper::getAccessibleIndexInParent: invalid creator!" );
                    // two ideas why this could be NULL:
                    // * nobody called our late ctor (init), so we never had a creator at all -> bad
                    // * the creator is already dead. In this case, we should have been disposed, and
                    //   never survived the above OContextEntryGuard.
                    // in all other situations the creator should be non-NULL

                if ( xCreator.is() )
                {
                    sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
                    for ( sal_Int32 nChild = 0; ( nChild < nChildCount ) && ( -1 == nRet ); ++nChild )
                    {
                        Reference< XAccessible > xChild( xParentContext->getAccessibleChild( nChild ) );
                        if ( xChild.get() == xCreator.get() )
                            nRet = nChild;
                    }
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAccessibleContextHelper::getAccessibleIndexInParent: caught an exception!" );
        }

        return nRet;
    }


    Locale SAL_CALL OAccessibleContextHelper::getLocale(  )
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


    Reference< XAccessibleContext > OAccessibleContextHelper::implGetParentContext()
    {
        Reference< XAccessible > xParent = getAccessibleParent();
        Reference< XAccessibleContext > xParentContext;
        if ( xParent.is() )
            xParentContext = xParent->getAccessibleContext();
        return xParentContext;
    }


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
