/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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


#include "framework/undomanagerhelper.hxx"

#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/flagguard.hxx>
#include <comphelper/asyncnotification.hxx>
#include <svl/undo.hxx>
#include <tools/diagnose_ex.h>
#include <osl/conditn.hxx>

#include <stack>
#include <queue>
#include <boost/function.hpp>

//......................................................................................................................
namespace framework
{
//......................................................................................................................

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
    using ::com::sun::star::document::XUndoManagerListener;
    using ::com::sun::star::document::UndoManagerEvent;
    using ::com::sun::star::document::EmptyUndoStackException;
    using ::com::sun::star::document::UndoContextNotClosedException;
    using ::com::sun::star::document::UndoFailedException;
    using ::com::sun::star::util::NotLockedException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::document::XUndoAction;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::document::XUndoManager;
    using ::com::sun::star::util::InvalidStateException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::util::XModifyListener;
    /** === end UNO using === **/
    using ::svl::IUndoManager;

    //==================================================================================================================
    //= UndoActionWrapper
    //==================================================================================================================
    class UndoActionWrapper : public SfxUndoAction
    {
    public:
                            UndoActionWrapper(
                                Reference< XUndoAction > const& i_undoAction
                            );
        virtual             ~UndoActionWrapper();

        virtual rtl::OUString GetComment() const;
        virtual void        Undo();
        virtual void        Redo();
        virtual sal_Bool    CanRepeat(SfxRepeatTarget&) const;

    private:
        const Reference< XUndoAction >  m_xUndoAction;
    };

    //------------------------------------------------------------------------------------------------------------------
    UndoActionWrapper::UndoActionWrapper( Reference< XUndoAction > const& i_undoAction )
        :SfxUndoAction()
        ,m_xUndoAction( i_undoAction )
    {
        ENSURE_OR_THROW( m_xUndoAction.is(), "illegal undo action" );
    }

    //------------------------------------------------------------------------------------------------------------------
    UndoActionWrapper::~UndoActionWrapper()
    {
        try
        {
            Reference< XComponent > xComponent( m_xUndoAction, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    rtl::OUString UndoActionWrapper::GetComment() const
    {
        rtl::OUString sComment;
        try
        {
            sComment = m_xUndoAction->getTitle();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return sComment;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoActionWrapper::Undo()
    {
        m_xUndoAction->undo();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoActionWrapper::Redo()
    {
        m_xUndoAction->redo();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool UndoActionWrapper::CanRepeat(SfxRepeatTarget&) const
    {
        return sal_False;
    }

    //==================================================================================================================
    //= UndoManagerRequest
    //==================================================================================================================
    class UndoManagerRequest : public ::comphelper::AnyEvent
    {
    public:
        UndoManagerRequest( ::boost::function0< void > const& i_request )
            :m_request( i_request )
            ,m_caughtException()
            ,m_finishCondition()
        {
            m_finishCondition.reset();
        }

        void execute()
        {
            try
            {
                m_request();
            }
            catch( const Exception& )
            {
                m_caughtException = ::cppu::getCaughtException();
            }
            m_finishCondition.set();
        }

        void wait()
        {
            m_finishCondition.wait();
            if ( m_caughtException.hasValue() )
                ::cppu::throwException( m_caughtException );
        }

        void cancel( const Reference< XInterface >& i_context )
        {
            m_caughtException <<= RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Concurrency error: an ealier operation on the stack failed." ) ),
                i_context
            );
            m_finishCondition.set();
        }

    protected:
        ~UndoManagerRequest()
        {
        }

    private:
        ::boost::function0< void >  m_request;
        Any                         m_caughtException;
        ::osl::Condition            m_finishCondition;
    };

    //------------------------------------------------------------------------------------------------------------------

    //==================================================================================================================
    //= UndoManagerHelper_Impl
    //==================================================================================================================
    class UndoManagerHelper_Impl : public SfxUndoListener
    {
    private:
        ::osl::Mutex                        m_aMutex;
        ::osl::Mutex                        m_aQueueMutex;
        bool                                m_disposed;
        bool                                m_bAPIActionRunning;
        bool                                m_bProcessingEvents;
        sal_Int32                           m_nLockCount;
        ::cppu::OInterfaceContainerHelper   m_aUndoListeners;
        ::cppu::OInterfaceContainerHelper   m_aModifyListeners;
        IUndoManagerImplementation&         m_rUndoManagerImplementation;
        ::std::stack< bool >                m_aContextVisibilities;
#if OSL_DEBUG_LEVEL > 0
        ::std::stack< bool >                m_aContextAPIFlags;
#endif
        ::std::queue< ::rtl::Reference< UndoManagerRequest > >
                                            m_aEventQueue;

    public:
        ::osl::Mutex&   getMutex() { return m_aMutex; }

    public:
        UndoManagerHelper_Impl( IUndoManagerImplementation& i_undoManagerImpl )
            :m_aMutex()
            ,m_aQueueMutex()
            ,m_disposed( false )
            ,m_bAPIActionRunning( false )
            ,m_bProcessingEvents( false )
            ,m_nLockCount( 0 )
            ,m_aUndoListeners( m_aMutex )
            ,m_aModifyListeners( m_aMutex )
            ,m_rUndoManagerImplementation( i_undoManagerImpl )
        {
            getUndoManager().AddUndoListener( *this );
        }

        virtual ~UndoManagerHelper_Impl()
        {
        }

        //..............................................................................................................
        IUndoManager& getUndoManager() const
        {
            return m_rUndoManagerImplementation.getImplUndoManager();
        }

        //..............................................................................................................
        Reference< XUndoManager > getXUndoManager() const
        {
            return m_rUndoManagerImplementation.getThis();
        }

        // SfxUndoListener
        virtual void actionUndone( const String& i_actionComment );
        virtual void actionRedone( const String& i_actionComment );
        virtual void undoActionAdded( const String& i_actionComment );
        virtual void cleared();
        virtual void clearedRedo();
        virtual void resetAll();
        virtual void listActionEntered( const String& i_comment );
        virtual void listActionLeft( const String& i_comment );
        virtual void listActionLeftAndMerged();
        virtual void listActionCancelled();
        virtual void undoManagerDying();

        // public operations
        void disposing();

        void enterUndoContext( const ::rtl::OUString& i_title, const bool i_hidden, IMutexGuard& i_instanceLock );
        void leaveUndoContext( IMutexGuard& i_instanceLock );
        void addUndoAction( const Reference< XUndoAction >& i_action, IMutexGuard& i_instanceLock );
        void undo( IMutexGuard& i_instanceLock );
        void redo( IMutexGuard& i_instanceLock );
        void clear( IMutexGuard& i_instanceLock );
        void clearRedo( IMutexGuard& i_instanceLock );
        void reset( IMutexGuard& i_instanceLock );

        void lock();
        void unlock();

        void addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
        {
            m_aUndoListeners.addInterface( i_listener );
        }

        void removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
        {
            m_aUndoListeners.removeInterface( i_listener );
        }

        void addModifyListener( const Reference< XModifyListener >& i_listener )
        {
            m_aModifyListeners.addInterface( i_listener );
        }

        void removeModifyListener( const Reference< XModifyListener >& i_listener )
        {
            m_aModifyListeners.removeInterface( i_listener );
        }

        UndoManagerEvent
            buildEvent( ::rtl::OUString const& i_title ) const;

        void impl_notifyModified();
        void notify(    ::rtl::OUString const& i_title,
                        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& )
                    );
        void notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ) )
        {
            notify( ::rtl::OUString(), i_notificationMethod );
        }

        void notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ) );

    private:
        /// adds a function to be called to the request processor's queue
        void impl_processRequest( ::boost::function0< void > const& i_request, IMutexGuard& i_instanceLock );

        /// impl-versions of the XUndoManager API.
        void impl_enterUndoContext( const ::rtl::OUString& i_title, const bool i_hidden );
        void impl_leaveUndoContext();
        void impl_addUndoAction( const Reference< XUndoAction >& i_action );
        void impl_doUndoRedo( IMutexGuard& i_externalLock, const bool i_undo );
        void impl_clear();
        void impl_clearRedo();
        void impl_reset();
    };

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::disposing()
    {
        EventObject aEvent;
        aEvent.Source = getXUndoManager();
        m_aUndoListeners.disposeAndClear( aEvent );
        m_aModifyListeners.disposeAndClear( aEvent );

        ::osl::MutexGuard aGuard( m_aMutex );

        getUndoManager().RemoveUndoListener( *this );

        m_disposed = true;
    }

    //------------------------------------------------------------------------------------------------------------------
    UndoManagerEvent UndoManagerHelper_Impl::buildEvent( ::rtl::OUString const& i_title ) const
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_title;
        aEvent.UndoContextDepth = getUndoManager().GetListActionDepth();
        return aEvent;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::impl_notifyModified()
    {
        const EventObject aEvent( getXUndoManager() );
        m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::notify( ::rtl::OUString const& i_title,
        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ) )
    {
        const UndoManagerEvent aEvent( buildEvent( i_title ) );

        // TODO: this notification method here is used by UndoManagerHelper_Impl, to multiplex the notifications we
        // receive from the IUndoManager. Those notitications are sent with a locked SolarMutex, which means
        // we're doing the multiplexing here with a locked SM, too. Which is Bad (TM).
        // Fixing this properly would require outsourcing all the notifications into an own thread - which might lead
        // to problems of its own, since clients might expect synchronous notifications.

        m_aUndoListeners.notifyEach( i_notificationMethod, aEvent );
        impl_notifyModified();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ) )
    {
        const EventObject aEvent( getXUndoManager() );

        // TODO: the same comment as in the other notify, regarding SM locking applies here ...

        m_aUndoListeners.notifyEach( i_notificationMethod, aEvent );
        impl_notifyModified();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::enterUndoContext( const ::rtl::OUString& i_title, const bool i_hidden, IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            ::boost::bind(
                &UndoManagerHelper_Impl::impl_enterUndoContext,
                this,
                ::boost::cref( i_title ),
                i_hidden
            ),
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::leaveUndoContext( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            ::boost::bind(
                &UndoManagerHelper_Impl::impl_leaveUndoContext,
                this
            ),
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::addUndoAction( const Reference< XUndoAction >& i_action, IMutexGuard& i_instanceLock )
    {
        if ( !i_action.is() )
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal undo action object" ) ),
                getXUndoManager(),
                1
            );

        impl_processRequest(
            ::boost::bind(
                &UndoManagerHelper_Impl::impl_addUndoAction,
                this,
                ::boost::ref( i_action )
            ),
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::clear( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            ::boost::bind(
                &UndoManagerHelper_Impl::impl_clear,
                this
            ),
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::clearRedo( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            ::boost::bind(
                &UndoManagerHelper_Impl::impl_clearRedo,
                this
            ),
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::reset( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            ::boost::bind(
                &UndoManagerHelper_Impl::impl_reset,
                this
            ),
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::lock()
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( getMutex() );

        if ( ++m_nLockCount == 1 )
        {
            IUndoManager& rUndoManager = getUndoManager();
            rUndoManager.EnableUndo( false );
        }
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::unlock()
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( getMutex() );

        if ( m_nLockCount == 0 )
            throw NotLockedException( "Undo manager is not locked", getXUndoManager() );

        if ( --m_nLockCount == 0 )
        {
            IUndoManager& rUndoManager = getUndoManager();
            rUndoManager.EnableUndo( true );
        }
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::impl_processRequest( ::boost::function0< void > const& i_request, IMutexGuard& i_instanceLock )
    {
        // create the request, and add it to our queue
        ::rtl::Reference< UndoManagerRequest > pRequest( new UndoManagerRequest( i_request ) );
        {
            ::osl::MutexGuard aQueueGuard( m_aQueueMutex );
            m_aEventQueue.push( pRequest );
        }

        i_instanceLock.clear();

        if ( m_bProcessingEvents )
        {
            // another thread is processing the event queue currently => it will also process the event which we just added
            pRequest->wait();
            return;
        }

        m_bProcessingEvents = true;
        do
        {
            pRequest.clear();
            {
                ::osl::MutexGuard aQueueGuard( m_aQueueMutex );
                if ( m_aEventQueue.empty() )
                {
                    // reset the flag before releasing the queue mutex, otherwise it's possible that another thread
                    // could add an event after we release the mutex, but before we reset the flag. If then this other
                    // thread checks the flag before be reset it, this thread's event would starve.
                    m_bProcessingEvents = false;
                    return;
                }
                pRequest = m_aEventQueue.front();
                m_aEventQueue.pop();
            }
            try
            {
                pRequest->execute();
                pRequest->wait();
            }
            catch( ... )
            {
                {
                    // no chance to process further requests, if the current one failed
                    // => discard them
                    ::osl::MutexGuard aQueueGuard( m_aQueueMutex );
                    while ( !m_aEventQueue.empty() )
                    {
                        pRequest = m_aEventQueue.front();
                        m_aEventQueue.pop();
                        pRequest->cancel( getXUndoManager() );
                    }
                    m_bProcessingEvents = false;
                }
                // re-throw the error
                throw;
            }
        }
        while ( true );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::impl_enterUndoContext( const ::rtl::OUString& i_title, const bool i_hidden )
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore this request if the manager is locked
            return;

        if ( i_hidden && ( rUndoManager.GetUndoActionCount( IUndoManager::CurrentLevel ) == 0 ) )
            throw EmptyUndoStackException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "can't enter a hidden context without a previous Undo action" ) ),
                m_rUndoManagerImplementation.getThis()
            );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.EnterListAction( i_title, ::rtl::OUString() );
        }

        m_aContextVisibilities.push( i_hidden );

        const UndoManagerEvent aEvent( buildEvent( i_title ) );
        aGuard.clear();
        // <--- SYNCHRONIZED

        m_aUndoListeners.notifyEach( i_hidden ? &XUndoManagerListener::enteredHiddenContext : &XUndoManagerListener::enteredContext, aEvent );
        impl_notifyModified();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::impl_leaveUndoContext()
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore this request if the manager is locked
            return;

        if ( !rUndoManager.IsInListAction() )
            throw InvalidStateException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no active undo context" ) ),
                getXUndoManager()
            );

        size_t nContextElements = 0;

        const bool isHiddenContext = m_aContextVisibilities.top();;
        m_aContextVisibilities.pop();

        const bool bHadRedoActions = ( rUndoManager.GetRedoActionCount( IUndoManager::TopLevel ) > 0 );
        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            if ( isHiddenContext )
                nContextElements = rUndoManager.LeaveAndMergeListAction();
            else
                nContextElements = rUndoManager.LeaveListAction();
        }
        const bool bHasRedoActions = ( rUndoManager.GetRedoActionCount( IUndoManager::TopLevel ) > 0 );

        // prepare notification
        void ( SAL_CALL XUndoManagerListener::*notificationMethod )( const UndoManagerEvent& ) = NULL;

        UndoManagerEvent aContextEvent( buildEvent( ::rtl::OUString() ) );
        const EventObject aClearedEvent( getXUndoManager() );
        if ( nContextElements == 0 )
        {
            notificationMethod = &XUndoManagerListener::cancelledContext;
        }
        else if ( isHiddenContext )
        {
            notificationMethod = &XUndoManagerListener::leftHiddenContext;
        }
        else
        {
            aContextEvent.UndoActionTitle = rUndoManager.GetUndoActionComment( 0, IUndoManager::CurrentLevel );
            notificationMethod = &XUndoManagerListener::leftContext;
        }

        aGuard.clear();
        // <--- SYNCHRONIZED

        if ( bHadRedoActions && !bHasRedoActions )
            m_aUndoListeners.notifyEach( &XUndoManagerListener::redoActionsCleared, aClearedEvent );
        m_aUndoListeners.notifyEach( notificationMethod, aContextEvent );
        impl_notifyModified();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::impl_doUndoRedo( IMutexGuard& i_externalLock, const bool i_undo )
    {
        ::osl::Guard< ::framework::IMutex > aExternalGuard( i_externalLock.getGuardedMutex() );
            // note that this assumes that the mutex has been released in the thread which added the
            // Undo/Redo request, so we can successfully acquire it

        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( ::rtl::OUString(), getXUndoManager() );

        const size_t nElements  =   i_undo
                                ?   rUndoManager.GetUndoActionCount( IUndoManager::TopLevel )
                                :   rUndoManager.GetRedoActionCount( IUndoManager::TopLevel );
        if ( nElements == 0 )
            throw EmptyUndoStackException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "stack is empty" )), getXUndoManager() );

        aGuard.clear();
        // <--- SYNCHRONIZED

        try
        {
            if ( i_undo )
                rUndoManager.Undo();
            else
                rUndoManager.Redo();
        }
        catch( const RuntimeException& ) { /* allowed to leave here */ throw; }
        catch( const UndoFailedException& ) { /* allowed to leave here */ throw; }
        catch( const Exception& )
        {
            // not allowed to leave
            const Any aError( ::cppu::getCaughtException() );
            throw UndoFailedException( ::rtl::OUString(), getXUndoManager(), aError );
        }

        // note that in opposite to all of the other methods, we do *not* have our mutex locked when calling
        // into the IUndoManager implementation. This ensures that an actual XUndoAction::undo/redo is also
        // called without our mutex being locked.
        // As a consequence, we do not set m_bAPIActionRunning here. Instead, our actionUndone/actionRedone methods
        // *always* multiplex the event to our XUndoManagerListeners, not only when m_bAPIActionRunning is FALSE (This
        // again is different from all other SfxUndoListener methods).
        // So, we do not need to do this notification here ourself.
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::impl_addUndoAction( const Reference< XUndoAction >& i_action )
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore the request if the manager is locked
            return;

        const UndoManagerEvent aEventAdd( buildEvent( i_action->getTitle() ) );
        const EventObject aEventClear( getXUndoManager() );

        const bool bHadRedoActions = ( rUndoManager.GetRedoActionCount( IUndoManager::CurrentLevel ) > 0 );
        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.AddUndoAction( new UndoActionWrapper( i_action ) );
        }
        const bool bHasRedoActions = ( rUndoManager.GetRedoActionCount( IUndoManager::CurrentLevel ) > 0 );

        aGuard.clear();
        // <--- SYNCHRONIZED

        m_aUndoListeners.notifyEach( &XUndoManagerListener::undoActionAdded, aEventAdd );
        if ( bHadRedoActions && !bHasRedoActions )
            m_aUndoListeners.notifyEach( &XUndoManagerListener::redoActionsCleared, aEventClear );
        impl_notifyModified();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::impl_clear()
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( ::rtl::OUString(), getXUndoManager() );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.Clear();
        }

        const EventObject aEvent( getXUndoManager() );
        aGuard.clear();
        // <--- SYNCHRONIZED

        m_aUndoListeners.notifyEach( &XUndoManagerListener::allActionsCleared, aEvent );
        impl_notifyModified();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::impl_clearRedo()
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( ::rtl::OUString(), getXUndoManager() );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.ClearRedo();
        }

        const EventObject aEvent( getXUndoManager() );
        aGuard.clear();
        // <--- SYNCHRONIZED

        m_aUndoListeners.notifyEach( &XUndoManagerListener::redoActionsCleared, aEvent );
        impl_notifyModified();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::impl_reset()
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.Reset();
        }

        const EventObject aEvent( getXUndoManager() );
        aGuard.clear();
        // <--- SYNCHRONIZED

        m_aUndoListeners.notifyEach( &XUndoManagerListener::resetAll, aEvent );
        impl_notifyModified();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::actionUndone( const String& i_actionComment )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_actionComment;
        aEvent.UndoContextDepth = 0;    // Undo can happen on level 0 only
        m_aUndoListeners.notifyEach( &XUndoManagerListener::actionUndone, aEvent );
        impl_notifyModified();
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::actionRedone( const String& i_actionComment )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_actionComment;
        aEvent.UndoContextDepth = 0;    // Redo can happen on level 0 only
        m_aUndoListeners.notifyEach( &XUndoManagerListener::actionRedone, aEvent );
        impl_notifyModified();
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::undoActionAdded( const String& i_actionComment )
    {
        if ( m_bAPIActionRunning )
            return;

        notify( i_actionComment, &XUndoManagerListener::undoActionAdded );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::cleared()
    {
        if ( m_bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::allActionsCleared );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::clearedRedo()
    {
        if ( m_bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::redoActionsCleared );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::resetAll()
    {
        if ( m_bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::resetAll );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::listActionEntered( const String& i_comment )
    {
#if OSL_DEBUG_LEVEL > 0
        m_aContextAPIFlags.push( m_bAPIActionRunning );
#endif

        if ( m_bAPIActionRunning )
            return;

        notify( i_comment, &XUndoManagerListener::enteredContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::listActionLeft( const String& i_comment )
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = m_aContextAPIFlags.top();
        m_aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == m_bAPIActionRunning, "UndoManagerHelper_Impl::listActionLeft: API and non-API contexts interwoven!" );
#endif

        if ( m_bAPIActionRunning )
            return;

        notify( i_comment, &XUndoManagerListener::leftContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::listActionLeftAndMerged()
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = m_aContextAPIFlags.top();
        m_aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == m_bAPIActionRunning, "UndoManagerHelper_Impl::listActionLeftAndMerged: API and non-API contexts interwoven!" );
#endif

        if ( m_bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::leftHiddenContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::listActionCancelled()
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = m_aContextAPIFlags.top();
        m_aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == m_bAPIActionRunning, "UndoManagerHelper_Impl::listActionCancelled: API and non-API contexts interwoven!" );
#endif

        if ( m_bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::cancelledContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::undoManagerDying()
    {
        // TODO: do we need to care? Or is this the responsibility of our owner?
    }

    //==================================================================================================================
    //= UndoManagerHelper
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    UndoManagerHelper::UndoManagerHelper( IUndoManagerImplementation& i_undoManagerImpl )
        :m_pImpl( new UndoManagerHelper_Impl( i_undoManagerImpl ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    UndoManagerHelper::~UndoManagerHelper()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::disposing()
    {
        m_pImpl->disposing();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::enterUndoContext( const ::rtl::OUString& i_title, IMutexGuard& i_instanceLock )
    {
        m_pImpl->enterUndoContext( i_title, false, i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::enterHiddenUndoContext( IMutexGuard& i_instanceLock )
    {
        m_pImpl->enterUndoContext( ::rtl::OUString(), true, i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::leaveUndoContext( IMutexGuard& i_instanceLock )
    {
        m_pImpl->leaveUndoContext( i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::undo( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            ::boost::bind(
                &UndoManagerHelper_Impl::impl_doUndoRedo,
                this,
                ::boost::ref( i_instanceLock ),
                true
            ),
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::redo( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            ::boost::bind(
                &UndoManagerHelper_Impl::impl_doUndoRedo,
                this,
                ::boost::ref( i_instanceLock ),
                false
            ),
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::addUndoAction( const Reference< XUndoAction >& i_action, IMutexGuard& i_instanceLock )
    {
        m_pImpl->addUndoAction( i_action, i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::undo( IMutexGuard& i_instanceLock )
    {
        m_pImpl->undo( i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::redo( IMutexGuard& i_instanceLock )
    {
        m_pImpl->redo( i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool UndoManagerHelper::isUndoPossible() const
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( m_pImpl->getMutex() );
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return sal_False;
        return rUndoManager.GetUndoActionCount( IUndoManager::TopLevel ) > 0;
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool UndoManagerHelper::isRedoPossible() const
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( m_pImpl->getMutex() );
        const IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return sal_False;
        return rUndoManager.GetRedoActionCount( IUndoManager::TopLevel ) > 0;
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        //..............................................................................................................
        ::rtl::OUString lcl_getCurrentActionTitle( UndoManagerHelper_Impl& i_impl, const bool i_undo )
        {
            // SYNCHRONIZED --->
            ::osl::MutexGuard aGuard( i_impl.getMutex() );

            const IUndoManager& rUndoManager = i_impl.getUndoManager();
            const size_t nActionCount = i_undo
                                    ?   rUndoManager.GetUndoActionCount( IUndoManager::TopLevel )
                                    :   rUndoManager.GetRedoActionCount( IUndoManager::TopLevel );
            if ( nActionCount == 0 )
                throw EmptyUndoStackException(
                    i_undo ? ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no action on the undo stack" ) )
                           : ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no action on the redo stack" ) ),
                    i_impl.getXUndoManager()
                );
            return  i_undo
                ?   rUndoManager.GetUndoActionComment( 0, IUndoManager::TopLevel )
                :   rUndoManager.GetRedoActionComment( 0, IUndoManager::TopLevel );
            // <--- SYNCHRONIZED
        }

        //..............................................................................................................
        Sequence< ::rtl::OUString > lcl_getAllActionTitles( UndoManagerHelper_Impl& i_impl, const bool i_undo )
        {
            // SYNCHRONIZED --->
            ::osl::MutexGuard aGuard( i_impl.getMutex() );

            const IUndoManager& rUndoManager = i_impl.getUndoManager();
            const size_t nCount =   i_undo
                                ?   rUndoManager.GetUndoActionCount( IUndoManager::TopLevel )
                                :   rUndoManager.GetRedoActionCount( IUndoManager::TopLevel );

            Sequence< ::rtl::OUString > aTitles( nCount );
            for ( size_t i=0; i<nCount; ++i )
            {
                aTitles[i] =    i_undo
                            ?   rUndoManager.GetUndoActionComment( i, IUndoManager::TopLevel )
                            :   rUndoManager.GetRedoActionComment( i, IUndoManager::TopLevel );
            }
            return aTitles;
            // <--- SYNCHRONIZED
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString UndoManagerHelper::getCurrentUndoActionTitle() const
    {
        return lcl_getCurrentActionTitle( *m_pImpl, true );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString UndoManagerHelper::getCurrentRedoActionTitle() const
    {
        return lcl_getCurrentActionTitle( *m_pImpl, false );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > UndoManagerHelper::getAllUndoActionTitles() const
    {
        return lcl_getAllActionTitles( *m_pImpl, true );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > UndoManagerHelper::getAllRedoActionTitles() const
    {
        return lcl_getAllActionTitles( *m_pImpl, false );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::clear( IMutexGuard& i_instanceLock )
    {
        m_pImpl->clear( i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::clearRedo( IMutexGuard& i_instanceLock )
    {
        m_pImpl->clearRedo( i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::reset( IMutexGuard& i_instanceLock )
    {
        m_pImpl->reset( i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::lock()
    {
        m_pImpl->lock();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::unlock()
    {
        m_pImpl->unlock();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool UndoManagerHelper::isLocked()
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( m_pImpl->getMutex() );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        return !rUndoManager.IsUndoEnabled();
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->addUndoManagerListener( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->removeUndoManagerListener( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::addModifyListener( const Reference< XModifyListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->addModifyListener( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::removeModifyListener( const Reference< XModifyListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->removeModifyListener( i_listener );
    }

//......................................................................................................................
} // namespace framework
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
