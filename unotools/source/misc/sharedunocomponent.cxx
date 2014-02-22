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

#include <unotools/sharedunocomponent.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <cppuhelper/implbase1.hxx>
#include <tools/debug.hxx>


namespace utl
{


    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::util::XCloseListener;
    using ::com::sun::star::util::CloseVetoException;

    
    
    
    
    DisposableComponent::DisposableComponent( const Reference< XInterface >& _rxComponent )
        :m_xComponent( _rxComponent, UNO_QUERY )
    {
        DBG_ASSERT( m_xComponent.is() || !_rxComponent.is(), "DisposableComponent::DisposableComponent: should be an XComponent!" );
    }

    
    DisposableComponent::~DisposableComponent()
    {
        if ( m_xComponent.is() )
        {
            try
            {
                m_xComponent->dispose();
            }
            catch( const Exception& )
            {
                OSL_FAIL( "DisposableComponent::~DisposableComponent: caught an exception!" );
            }
            m_xComponent.clear();
        }
    }

    
    
    
    DBG_NAME( CloseableComponentImpl )
    typedef ::cppu::WeakImplHelper1 <   XCloseListener
                                    >   CloseableComponentImpl_Base;
    class CloseableComponentImpl : public CloseableComponentImpl_Base
    {
    private:
        Reference< XCloseable > m_xCloseable;

    public:
        CloseableComponentImpl( const Reference< XInterface >& _rxComponent );

        /** closes the component

            @nofail
        */
        void    nf_closeComponent();

    protected:
        virtual ~CloseableComponentImpl();

        
        virtual void SAL_CALL queryClosing( const EventObject& Source, ::sal_Bool GetsOwnership ) throw (CloseVetoException, RuntimeException);
        virtual void SAL_CALL notifyClosing( const EventObject& Source ) throw (RuntimeException);

        
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** starts or stops being a CloseListener at the component

            Only to be called upon construction of the instance, or when the component
            is to be closed.

        @nofail
        */
        void    impl_nf_switchListening( bool _bListen );


    private:
        CloseableComponentImpl();                                           
        CloseableComponentImpl( const CloseableComponentImpl& );            
        CloseableComponentImpl& operator=( const CloseableComponentImpl& ); 
    };

    
    CloseableComponentImpl::CloseableComponentImpl( const Reference< XInterface >& _rxComponent )
        :m_xCloseable( _rxComponent, UNO_QUERY )
    {
        DBG_CTOR( CloseableComponentImpl, NULL );
        DBG_ASSERT( m_xCloseable.is() || !_rxComponent.is(), "CloseableComponentImpl::CloseableComponentImpl: component is not an XCloseable!" );
        impl_nf_switchListening( true );
    }
    
    CloseableComponentImpl::~CloseableComponentImpl()
    {
        nf_closeComponent();
        DBG_DTOR( CloseableComponentImpl, NULL );
    }

    
    void CloseableComponentImpl::nf_closeComponent()
    {
        if ( !m_xCloseable.is() )
            
            return;

        
        impl_nf_switchListening( false );

        
        try
        {
            m_xCloseable->close( sal_True );
        }
        catch( const CloseVetoException& ) { /* fine */ }
        catch( const Exception& )
        {
            OSL_FAIL( "CloseableComponentImpl::nf_closeComponent: caught an unexpected exception!" );
        }

        
        m_xCloseable.clear();
    }

    
    void CloseableComponentImpl::impl_nf_switchListening( bool _bListen )
    {
        if ( !m_xCloseable.is() )
            return;

        try
        {
            if ( _bListen )
                m_xCloseable->addCloseListener( this );
            else
                m_xCloseable->removeCloseListener( this );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "CloseableComponentImpl::impl_nf_switchListening: caught an exception!" );
        }
    }

    
    void SAL_CALL CloseableComponentImpl::queryClosing( const EventObject&
    #ifdef DBG_UTIL
    Source
    #endif
    , ::sal_Bool /*GetsOwnership*/ ) throw (CloseVetoException, RuntimeException)
    {
        
        
        DBG_ASSERT( Source.Source == m_xCloseable, "CloseableComponentImpl::queryClosing: where did this come from?" );
        throw CloseVetoException();
    }

    
    void SAL_CALL CloseableComponentImpl::notifyClosing( const EventObject&
    #ifdef DBG_UTIL
    Source
    #endif
    ) throw (RuntimeException)
    {
        DBG_ASSERT( Source.Source == m_xCloseable, "CloseableComponentImpl::notifyClosing: where did this come from?" );

        
        
        
        OSL_FAIL( "CloseableComponentImpl::notifyClosing: unreachable!" );
    }

    
    void SAL_CALL CloseableComponentImpl::disposing( const EventObject&
    #ifdef DBG_UTIL
    Source
    #endif
    ) throw (RuntimeException)
    {
        DBG_ASSERT( Source.Source == m_xCloseable, "CloseableComponentImpl::disposing: where did this come from?" );
        OSL_FAIL( "CloseableComponentImpl::disposing: unreachable!" );
            
    }

    
    
    
    DBG_NAME( CloseableComponent )
    
    CloseableComponent::CloseableComponent( const Reference< XInterface >& _rxComponent )
        :m_pImpl( new CloseableComponentImpl( _rxComponent ) )
    {
        DBG_CTOR( CloseableComponent, NULL );
    }

    
    CloseableComponent::~CloseableComponent()
    {
        
        m_pImpl->nf_closeComponent();
        DBG_DTOR( CloseableComponent, NULL );
    }


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
