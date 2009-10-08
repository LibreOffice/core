/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: documenteventnotifier.cxx,v $
*
* $Revision: 1.1.2.4 $
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
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "documenteventnotifier.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/frame/DoubleInitializationException.hpp>
/** === end UNO includes === **/

#include <comphelper/asyncnotification.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::frame::DoubleInitializationException;
    using ::com::sun::star::document::XDocumentEventListener;
    using ::com::sun::star::document::DocumentEvent;
    using ::com::sun::star::frame::XController2;
    /** === end UNO using === **/
    using namespace ::com::sun::star;

    //==================================================================
    //= DocumentEventHolder
    //==================================================================
    typedef ::comphelper::EventHolder< DocumentEvent >  DocumentEventHolder;

    //====================================================================
    //= DocumentEventNotifier_Impl
    //====================================================================
    class DocumentEventNotifier_Impl : public ::comphelper::IEventProcessor
    {
        oslInterlockedCount                                     m_refCount;
        ::cppu::OWeakObject&                                    m_rDocument;
        ::osl::Mutex&                                           m_rMutex;
        bool                                                    m_bInitialized;
        bool                                                    m_bDisposed;
        ::rtl::Reference< ::comphelper::AsyncEventNotifier >    m_pEventBroadcaster;
        ::cppu::OInterfaceContainerHelper                       m_aLegacyEventListeners;
        ::cppu::OInterfaceContainerHelper                       m_aDocumentEventListeners;

    public:
        DocumentEventNotifier_Impl( ::cppu::OWeakObject& _rBroadcasterDocument, ::osl::Mutex& _rMutex )
            :m_refCount( 0 )
            ,m_rDocument( _rBroadcasterDocument )
            ,m_rMutex( _rMutex )
            ,m_bInitialized( false )
            ,m_bDisposed( false )
            ,m_aLegacyEventListeners( _rMutex )
            ,m_aDocumentEventListeners( _rMutex )
        {
        }

        // IReference
        virtual void SAL_CALL acquire();
        virtual void SAL_CALL release();

        void addLegacyEventListener( const Reference< document::XEventListener >& _Listener )
        {
            m_aLegacyEventListeners.addInterface( _Listener );
        }

        void removeLegacyEventListener( const Reference< document::XEventListener >& _Listener )
        {
            m_aLegacyEventListeners.removeInterface( _Listener );
        }

        void addDocumentEventListener( const Reference< XDocumentEventListener >& _Listener )
        {
            m_aDocumentEventListeners.addInterface( _Listener );
        }

        void removeDocumentEventListener( const Reference< XDocumentEventListener >& _Listener )
        {
            m_aDocumentEventListeners.removeInterface( _Listener );
        }

        void disposing();

        void onDocumentInitialized();

        void    notifyDocumentEvent( const ::rtl::OUString& _EventName, const Reference< XController2 >& _ViewController,
                    const Any& _Supplement )
        {
            impl_notifyEvent_nothrow( DocumentEvent(
                m_rDocument, _EventName, _ViewController, _Supplement ) );
        }

        void    notifyDocumentEventAsync( const ::rtl::OUString& _EventName, const Reference< XController2 >& _ViewController,
                    const Any& _Supplement )
        {
            impl_notifyEventAsync_nothrow( DocumentEvent(
                m_rDocument, _EventName, _ViewController, _Supplement ) );
        }

    protected:
        virtual ~DocumentEventNotifier_Impl()
        {
        }

        // IEventProcessor
        virtual void processEvent( const ::comphelper::AnyEvent& _rEvent );

    private:
        void impl_notifyEvent_nothrow( const DocumentEvent& _rEvent );
        void impl_notifyEventAsync_nothrow( const DocumentEvent& _rEvent );
    };

    //--------------------------------------------------------------------
    void SAL_CALL DocumentEventNotifier_Impl::acquire()
    {
        osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DocumentEventNotifier_Impl::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
            delete this;
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier_Impl::disposing()
    {
        // cancel any pending asynchronous events
        ::osl::MutexGuard aGuard( m_rMutex );
        if ( m_pEventBroadcaster.is() )
        {
            m_pEventBroadcaster->removeEventsForProcessor( this );
            m_pEventBroadcaster->terminate();
            m_pEventBroadcaster = NULL;
        }

        lang::EventObject aEvent( m_rDocument );
        m_aLegacyEventListeners.disposeAndClear( aEvent );
        m_aDocumentEventListeners.disposeAndClear( aEvent );

        m_bDisposed = true;
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier_Impl::onDocumentInitialized()
    {
        if ( m_bInitialized )
            throw DoubleInitializationException();

        m_bInitialized = true;
        if ( m_pEventBroadcaster.is() )
            // there are already pending asynchronous events
            m_pEventBroadcaster->create();
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier_Impl::impl_notifyEvent_nothrow( const DocumentEvent& _rEvent )
    {
        OSL_PRECOND( m_bInitialized,
            "DocumentEventNotifier_Impl::impl_notifyEvent_nothrow: only to be called when the document is already initialized!" );
        try
        {
            document::EventObject aLegacyEvent( _rEvent.Source, _rEvent.EventName );
            m_aLegacyEventListeners.notifyEach( &document::XEventListener::notifyEvent, aLegacyEvent );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        try
        {
            m_aDocumentEventListeners.notifyEach( &XDocumentEventListener::documentEventOccured, _rEvent );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier_Impl::impl_notifyEventAsync_nothrow( const DocumentEvent& _rEvent )
    {
        if ( !m_pEventBroadcaster.is() )
        {
            m_pEventBroadcaster.set( new ::comphelper::AsyncEventNotifier );
            if ( m_bInitialized )
                // start processing the events if and only if we (our document, respectively) are
                // already initialized
                m_pEventBroadcaster->create();
        }
        m_pEventBroadcaster->addEvent( new DocumentEventHolder( _rEvent ), this );
    }

    // -----------------------------------------------------------------------------
    void DocumentEventNotifier_Impl::processEvent( const ::comphelper::AnyEvent& _rEvent )
    {
        // beware, this is called from the notification thread
        {
            ::osl::MutexGuard aGuard( m_rMutex );
            if  ( m_bDisposed )
                return;
        }
        const DocumentEventHolder& rEventHolder = dynamic_cast< const DocumentEventHolder& >( _rEvent );
        impl_notifyEvent_nothrow( rEventHolder.getEventObject() );
    }

    //====================================================================
    //= DocumentEventNotifier
    //====================================================================
    //--------------------------------------------------------------------
    DocumentEventNotifier::DocumentEventNotifier( ::cppu::OWeakObject& _rBroadcasterDocument, ::osl::Mutex& _rMutex )
        :m_pImpl( new DocumentEventNotifier_Impl( _rBroadcasterDocument, _rMutex ) )
    {
    }

    //--------------------------------------------------------------------
    DocumentEventNotifier::~DocumentEventNotifier()
    {
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier::disposing()
    {
        m_pImpl->disposing();
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier::onDocumentInitialized()
    {
        m_pImpl->onDocumentInitialized();
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier::addLegacyEventListener( const Reference< document::XEventListener >& _Listener )
    {
        m_pImpl->addLegacyEventListener( _Listener );
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier::removeLegacyEventListener( const Reference< document::XEventListener >& _Listener )
    {
        m_pImpl->removeLegacyEventListener( _Listener );
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier::addDocumentEventListener( const Reference< XDocumentEventListener >& _Listener )
    {
        m_pImpl->addDocumentEventListener( _Listener );
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier::removeDocumentEventListener( const Reference< XDocumentEventListener >& _Listener )
    {
        m_pImpl->removeDocumentEventListener( _Listener );
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier::notifyDocumentEvent( const ::rtl::OUString& _EventName,
        const Reference< XController2 >& _ViewController, const Any& _Supplement )
    {
        m_pImpl->notifyDocumentEvent( _EventName, _ViewController, _Supplement );
    }

    //--------------------------------------------------------------------
    void DocumentEventNotifier::notifyDocumentEventAsync( const ::rtl::OUString& _EventName,
        const Reference< XController2 >& _ViewController, const Any& _Supplement )
    {
        m_pImpl->notifyDocumentEventAsync( _EventName, _ViewController, _Supplement );
    }

//........................................................................
} // namespace dbaccess
//........................................................................
