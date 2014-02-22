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

#include <comphelper/accessiblecontexthelper.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
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
        IMutex*                             m_pExternalLock;    

        WeakReference< XAccessible >        m_aCreator;         

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
            :m_pExternalLock( NULL )
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
        ,m_pImpl( NULL )
    {
        assert(_pExternalLock);
        m_pImpl = new OContextHelper_Impl();
        m_pImpl->setExternalLock( _pExternalLock );
    }

    
    void OAccessibleContextHelper::forgetExternalLock()
    {
        m_pImpl->setExternalLock( NULL );
    }

    
    OAccessibleContextHelper::~OAccessibleContextHelper( )
    {
        forgetExternalLock();
            
            

        ensureDisposed();

        delete m_pImpl;
        m_pImpl = NULL;
    }

    
    IMutex* OAccessibleContextHelper::getExternalLock( )
    {
        return m_pImpl->getExternalLock();
    }

    
    void SAL_CALL OAccessibleContextHelper::disposing()
    {
        
        
        OMutexGuard aGuard( getExternalLock() );

        if ( m_pImpl->getClientId( ) )
        {
            AccessibleEventNotifier::revokeClientNotifyDisposing( m_pImpl->getClientId( ), *this );
            m_pImpl->setClientId( 0 );
        }
    }

    
    void SAL_CALL OAccessibleContextHelper::addAccessibleEventListener( const Reference< XAccessibleEventListener >& _rxListener ) throw (RuntimeException)
    {
        OMutexGuard aGuard( getExternalLock() );
            
            
            
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

    
    void SAL_CALL OAccessibleContextHelper::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& _rxListener ) throw (RuntimeException)
    {
        OMutexGuard aGuard( getExternalLock() );
            
            
            
        if ( !isAlive() )
            return;

        if ( _rxListener.is() )
        {
            sal_Int32 nListenerCount = AccessibleEventNotifier::removeEventListener( m_pImpl->getClientId( ), _rxListener );
            if ( !nListenerCount )
            {
                
                
                
                
                AccessibleEventNotifier::revokeClient( m_pImpl->getClientId( ) );
                m_pImpl->setClientId( 0 );
            }
        }
    }

    
    void SAL_CALL OAccessibleContextHelper::NotifyAccessibleEvent( const sal_Int16 _nEventId,
        const Any& _rOldValue, const Any& _rNewValue )
    {
        if ( !m_pImpl->getClientId( ) )
            
            
            return;

        
        AccessibleEventObject aEvent;
        aEvent.Source = *this;
        aEvent.EventId = _nEventId;
        aEvent.OldValue = _rOldValue;
        aEvent.NewValue = _rNewValue;

        
        AccessibleEventNotifier::addEvent( m_pImpl->getClientId( ), aEvent );
    }

    
    bool OAccessibleContextHelper::isAlive() const
    {
        return !GetBroadcastHelper().bDisposed && !GetBroadcastHelper().bInDispose;
    }

    
    void OAccessibleContextHelper::ensureAlive() const SAL_THROW( ( DisposedException ) )
    {
        if( !isAlive() )
            throw DisposedException();
    }

    
    void OAccessibleContextHelper::ensureDisposed( )
    {
        if ( !GetBroadcastHelper().bDisposed )
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

    
    sal_Int32 SAL_CALL OAccessibleContextHelper::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        
        sal_Int32 nRet = -1;

        try
        {

            Reference< XAccessibleContext > xParentContext( implGetParentContext() );

            
            if ( xParentContext.is() )
            {
                
                Reference< XAccessible > xCreator( m_pImpl->getCreator() );

                OSL_ENSURE( xCreator.is(), "OAccessibleContextHelper::getAccessibleIndexInParent: invalid creator!" );
                    
                    
                    
                    
                    

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

    
    Locale SAL_CALL OAccessibleContextHelper::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        
        Reference< XAccessible > xParent = getAccessibleParent();
        Reference< XAccessibleContext > xParentContext;
        if ( xParent.is() )
            xParentContext = xParent->getAccessibleContext();

        if ( !xParentContext.is() )
            throw IllegalAccessibleComponentStateException( OUString(), *this );

        return xParentContext->getLocale();
    }

    
    Reference< XAccessibleContext > OAccessibleContextHelper::implGetParentContext() SAL_THROW( ( RuntimeException ) )
    {
        Reference< XAccessible > xParent = getAccessibleParent();
        Reference< XAccessibleContext > xParentContext;
        if ( xParent.is() )
            xParentContext = xParent->getAccessibleContext();
        return xParentContext;
    }


}   



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
