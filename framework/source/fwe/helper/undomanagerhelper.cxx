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

#include <framework/undomanagerhelper.hxx>
#include <framework/imutex.hxx>

#include <com/sun/star/document/EmptyUndoStackException.hpp>
#include <com/sun/star/document/UndoContextNotClosedException.hpp>
#include <com/sun/star/document/UndoFailedException.hpp>
#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/InvalidStateException.hpp>
#include <com/sun/star/util/NotLockedException.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/flagguard.hxx>
#include <comphelper/asyncnotification.hxx>
#include <svl/undo.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <osl/conditn.hxx>
#include <vcl/svapp.hxx>

#include <functional>
#include <mutex>
#include <stack>
#include <queue>
#include <utility>

namespace framework
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
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

    //= UndoActionWrapper

    namespace {

    class UndoActionWrapper : public SfxUndoAction
    {
    public:
        explicit            UndoActionWrapper(
                                Reference< XUndoAction > const& i_undoAction
                            );
        virtual             ~UndoActionWrapper() override;

        virtual OUString    GetComment() const override;
        virtual void        Undo() override;
        virtual void        Redo() override;
        virtual bool        CanRepeat(SfxRepeatTarget&) const override;

    private:
        const Reference< XUndoAction >  m_xUndoAction;
    };

    }

    UndoActionWrapper::UndoActionWrapper( Reference< XUndoAction > const& i_undoAction )
        : m_xUndoAction( i_undoAction )
    {
        ENSURE_OR_THROW( m_xUndoAction.is(), "illegal undo action" );
    }

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
            DBG_UNHANDLED_EXCEPTION("fwk");
        }
    }

    OUString UndoActionWrapper::GetComment() const
    {
        OUString sComment;
        try
        {
            sComment = m_xUndoAction->getTitle();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("fwk");
        }
        return sComment;
    }

    void UndoActionWrapper::Undo()
    {
        m_xUndoAction->undo();
    }

    void UndoActionWrapper::Redo()
    {
        m_xUndoAction->redo();
    }

    bool UndoActionWrapper::CanRepeat(SfxRepeatTarget&) const
    {
        return false;
    }

    //= UndoManagerRequest

    namespace {

    class UndoManagerRequest : public ::comphelper::AnyEvent
    {
    public:
        explicit UndoManagerRequest( ::std::function<void ()>  i_request )
            :m_request(std::move( i_request ))
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
                u"Concurrency error: an earlier operation on the stack failed."_ustr,
                i_context
            );
            m_finishCondition.set();
        }

    protected:
        virtual ~UndoManagerRequest() override
        {
        }

    private:
        ::std::function<void ()>    m_request;
        Any                         m_caughtException;
        ::osl::Condition            m_finishCondition;
    };

    }

    //= UndoManagerHelper_Impl

    class UndoManagerHelper_Impl : public SfxUndoListener
    {
    private:
        ::osl::Mutex                        m_aMutex;
        /// Use different mutex for listeners to prevent ABBA deadlocks
        std::mutex                          m_aListenerMutex;
        std::mutex                          m_aQueueMutex;
        bool                                m_bAPIActionRunning;
        bool                                m_bProcessingEvents;
        sal_Int32                           m_nLockCount;
        ::comphelper::OInterfaceContainerHelper4<XUndoManagerListener>   m_aUndoListeners;
        ::comphelper::OInterfaceContainerHelper4<XModifyListener>   m_aModifyListeners;
        IUndoManagerImplementation&         m_rUndoManagerImplementation;
        ::std::stack< bool >                m_aContextVisibilities;
#if OSL_DEBUG_LEVEL > 0
        bool                                m_bContextAPIFlagsEverPushed = {false};
        ::std::stack< bool >                m_aContextAPIFlags;
#endif
        ::std::queue< ::rtl::Reference< UndoManagerRequest > >
                                            m_aEventQueue;

    public:
        ::osl::Mutex&   getMutex() { return m_aMutex; }

    public:
        explicit UndoManagerHelper_Impl( IUndoManagerImplementation& i_undoManagerImpl )
            :m_bAPIActionRunning( false )
            ,m_bProcessingEvents( false )
            ,m_nLockCount( 0 )
            ,m_rUndoManagerImplementation( i_undoManagerImpl )
        {
            getUndoManager().AddUndoListener( *this );
        }

        virtual ~UndoManagerHelper_Impl()
        {
        }

        SfxUndoManager& getUndoManager() const
        {
            return m_rUndoManagerImplementation.getImplUndoManager();
        }

        Reference< XUndoManager > getXUndoManager() const
        {
            return m_rUndoManagerImplementation.getThis();
        }

        // SfxUndoListener
        virtual void actionUndone( const OUString& i_actionComment ) override;
        virtual void actionRedone( const OUString& i_actionComment ) override;
        virtual void undoActionAdded( const OUString& i_actionComment ) override;
        virtual void cleared() override;
        virtual void clearedRedo() override;
        virtual void resetAll() override;
        virtual void listActionEntered( const OUString& i_comment ) override;
        virtual void listActionLeft( const OUString& i_comment ) override;
        virtual void listActionCancelled() override;

        // public operations
        void disposing();

        void enterUndoContext( const OUString& i_title, const bool i_hidden, IMutexGuard& i_instanceLock );
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
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.addInterface( g, i_listener );
        }

        void removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.removeInterface( g, i_listener );
        }

        void addModifyListener( const Reference< XModifyListener >& i_listener )
        {
            std::unique_lock g(m_aListenerMutex);
            m_aModifyListeners.addInterface( g, i_listener );
        }

        void removeModifyListener( const Reference< XModifyListener >& i_listener )
        {
            std::unique_lock g(m_aListenerMutex);
            m_aModifyListeners.removeInterface( g, i_listener );
        }

        UndoManagerEvent
            buildEvent( OUString const& i_title ) const;

        void impl_notifyModified();
        void notify(    OUString const& i_title,
                        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& )
                    );
        void notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ) );

    private:
        /// adds a function to be called to the request processor's queue
        void impl_processRequest(::std::function<void ()> const& i_request, IMutexGuard& i_instanceLock);

        /// impl-versions of the XUndoManager API.
        void impl_enterUndoContext( const OUString& i_title, const bool i_hidden );
        void impl_leaveUndoContext();
        void impl_addUndoAction( const Reference< XUndoAction >& i_action );
        void impl_doUndoRedo( IMutexGuard& i_externalLock, const bool i_undo );
        void impl_clear();
        void impl_clearRedo();
        void impl_reset();
    };

    void UndoManagerHelper_Impl::disposing()
    {
        EventObject aEvent;
        aEvent.Source = getXUndoManager();
        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.disposeAndClear( g, aEvent );
            m_aModifyListeners.disposeAndClear( g, aEvent );
        }
        ::osl::MutexGuard aGuard( m_aMutex );

        getUndoManager().RemoveUndoListener( *this );
    }

    UndoManagerEvent UndoManagerHelper_Impl::buildEvent( OUString const& i_title ) const
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_title;
        aEvent.UndoContextDepth = getUndoManager().GetListActionDepth();
        return aEvent;
    }

    void UndoManagerHelper_Impl::impl_notifyModified()
    {
        const EventObject aEvent( getXUndoManager() );
        std::unique_lock g(m_aListenerMutex);
        m_aModifyListeners.notifyEach( g, &XModifyListener::modified, aEvent );
    }

    void UndoManagerHelper_Impl::notify( OUString const& i_title,
        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ) )
    {
        const UndoManagerEvent aEvent( buildEvent( i_title ) );

        // TODO: this notification method here is used by UndoManagerHelper_Impl, to multiplex the notifications we
        // receive from the SfxUndoManager. Those notifications are sent with a locked SolarMutex, which means
        // we're doing the multiplexing here with a locked SM, too. Which is Bad (TM).
        // Fixing this properly would require outsourcing all the notifications into an own thread - which might lead
        // to problems of its own, since clients might expect synchronous notifications.

        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.notifyEach( g, i_notificationMethod, aEvent );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ) )
    {
        const EventObject aEvent( getXUndoManager() );

        // TODO: the same comment as in the other notify, regarding SM locking applies here ...
        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.notifyEach( g, i_notificationMethod, aEvent );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::enterUndoContext( const OUString& i_title, const bool i_hidden, IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            [this, &i_title, i_hidden] () { return this->impl_enterUndoContext(i_title, i_hidden); },
            i_instanceLock
        );
    }

    void UndoManagerHelper_Impl::leaveUndoContext( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            [this] () { return this->impl_leaveUndoContext(); },
            i_instanceLock
        );
    }

    void UndoManagerHelper_Impl::addUndoAction( const Reference< XUndoAction >& i_action, IMutexGuard& i_instanceLock )
    {
        if ( !i_action.is() )
            throw IllegalArgumentException(
                u"illegal undo action object"_ustr,
                getXUndoManager(),
                1
            );

        impl_processRequest(
            [this, &i_action] () { return this->impl_addUndoAction(i_action); },
            i_instanceLock
        );
    }

    void UndoManagerHelper_Impl::clear( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            [this] () { return this->impl_clear(); },
            i_instanceLock
        );
    }

    void UndoManagerHelper_Impl::clearRedo( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            [this] () { return this->impl_clearRedo(); },
            i_instanceLock
        );
    }

    void UndoManagerHelper_Impl::reset( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            [this] () { return this->impl_reset(); },
            i_instanceLock
        );
    }

    void UndoManagerHelper_Impl::lock()
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( getMutex() );

        if ( ++m_nLockCount == 1 )
        {
            SfxUndoManager& rUndoManager = getUndoManager();
            rUndoManager.EnableUndo( false );
        }
        // <--- SYNCHRONIZED
    }

    void UndoManagerHelper_Impl::unlock()
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( getMutex() );

        if ( m_nLockCount == 0 )
            throw NotLockedException( u"Undo manager is not locked"_ustr, getXUndoManager() );

        if ( --m_nLockCount == 0 )
        {
            SfxUndoManager& rUndoManager = getUndoManager();
            rUndoManager.EnableUndo( true );
        }
        // <--- SYNCHRONIZED
    }

    void UndoManagerHelper_Impl::impl_processRequest(::std::function<void ()> const& i_request, IMutexGuard& i_instanceLock)
    {
        // create the request, and add it to our queue
        ::rtl::Reference< UndoManagerRequest > pRequest( new UndoManagerRequest( i_request ) );
        {
            std::unique_lock aQueueGuard( m_aQueueMutex );
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
                std::unique_lock aQueueGuard( m_aQueueMutex );
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
                    std::unique_lock aQueueGuard( m_aQueueMutex );
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

    void UndoManagerHelper_Impl::impl_enterUndoContext( const OUString& i_title, const bool i_hidden )
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        SfxUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore this request if the manager is locked
            return;

        if ( i_hidden && ( rUndoManager.GetUndoActionCount() == 0 ) )
            throw EmptyUndoStackException(
                u"can't enter a hidden context without a previous Undo action"_ustr,
                m_rUndoManagerImplementation.getThis()
            );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.EnterListAction( i_title, OUString(), 0, ViewShellId(-1) );
        }

        m_aContextVisibilities.push( i_hidden );

        const UndoManagerEvent aEvent( buildEvent( i_title ) );
        aGuard.clear();
        // <--- SYNCHRONIZED

        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.notifyEach( g, i_hidden ? &XUndoManagerListener::enteredHiddenContext : &XUndoManagerListener::enteredContext, aEvent );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::impl_leaveUndoContext()
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        SfxUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore this request if the manager is locked
            return;

        if ( !rUndoManager.IsInListAction() )
            throw InvalidStateException(
                u"no active undo context"_ustr,
                getXUndoManager()
            );

        size_t nContextElements = 0;

        const bool isHiddenContext = m_aContextVisibilities.top();
        m_aContextVisibilities.pop();

        const bool bHadRedoActions = ( rUndoManager.GetRedoActionCount( SfxUndoManager::TopLevel ) > 0 );
        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            if ( isHiddenContext )
                nContextElements = rUndoManager.LeaveAndMergeListAction();
            else
                nContextElements = rUndoManager.LeaveListAction();
        }
        const bool bHasRedoActions = ( rUndoManager.GetRedoActionCount( SfxUndoManager::TopLevel ) > 0 );

        // prepare notification
        void ( SAL_CALL XUndoManagerListener::*notificationMethod )( const UndoManagerEvent& ) = nullptr;

        UndoManagerEvent aContextEvent( buildEvent( OUString() ) );
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
            aContextEvent.UndoActionTitle = rUndoManager.GetUndoActionComment();
            notificationMethod = &XUndoManagerListener::leftContext;
        }

        aGuard.clear();
        // <--- SYNCHRONIZED

        {
            std::unique_lock g(m_aListenerMutex);
            if ( bHadRedoActions && !bHasRedoActions )
                m_aUndoListeners.notifyEach( g, &XUndoManagerListener::redoActionsCleared, aClearedEvent );
            m_aUndoListeners.notifyEach( g, notificationMethod, aContextEvent );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::impl_doUndoRedo( IMutexGuard& i_externalLock, const bool i_undo )
    {
        ::osl::Guard< ::framework::IMutex > aExternalGuard( i_externalLock.getGuardedMutex() );
            // note that this assumes that the mutex has been released in the thread which added the
            // Undo/Redo request, so we can successfully acquire it

        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        SfxUndoManager& rUndoManager = getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( OUString(), getXUndoManager() );

        const size_t nElements  =   i_undo
                                ?   rUndoManager.GetUndoActionCount( SfxUndoManager::TopLevel )
                                :   rUndoManager.GetRedoActionCount( SfxUndoManager::TopLevel );
        if ( nElements == 0 )
            throw EmptyUndoStackException(u"stack is empty"_ustr, getXUndoManager() );

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
            throw UndoFailedException( OUString(), getXUndoManager(), aError );
        }

        // note that in opposite to all of the other methods, we do *not* have our mutex locked when calling
        // into the SfxUndoManager implementation. This ensures that an actual XUndoAction::undo/redo is also
        // called without our mutex being locked.
        // As a consequence, we do not set m_bAPIActionRunning here. Instead, our actionUndone/actionRedone methods
        // *always* multiplex the event to our XUndoManagerListeners, not only when m_bAPIActionRunning is FALSE (This
        // again is different from all other SfxUndoListener methods).
        // So, we do not need to do this notification here ourself.
    }

    void UndoManagerHelper_Impl::impl_addUndoAction( const Reference< XUndoAction >& i_action )
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        SfxUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore the request if the manager is locked
            return;

        const UndoManagerEvent aEventAdd( buildEvent( i_action->getTitle() ) );
        const EventObject aEventClear( getXUndoManager() );

        const bool bHadRedoActions = ( rUndoManager.GetRedoActionCount() > 0 );
        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.AddUndoAction( std::make_unique<UndoActionWrapper>( i_action ) );
        }
        const bool bHasRedoActions = ( rUndoManager.GetRedoActionCount() > 0 );

        aGuard.clear();
        // <--- SYNCHRONIZED

        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.notifyEach( g, &XUndoManagerListener::undoActionAdded, aEventAdd );
            if ( bHadRedoActions && !bHasRedoActions )
                m_aUndoListeners.notifyEach( g, &XUndoManagerListener::redoActionsCleared, aEventClear );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::impl_clear()
    {
        EventObject aEvent;
        {
            SolarMutexGuard aGuard;
            ::osl::MutexGuard aGuard2( m_aMutex );

            SfxUndoManager& rUndoManager = getUndoManager();
            if ( rUndoManager.IsInListAction() )
                throw UndoContextNotClosedException( OUString(), getXUndoManager() );

            {
                ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
                rUndoManager.Clear();
            }

            aEvent = EventObject( getXUndoManager() );
        }

        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.notifyEach( g, &XUndoManagerListener::allActionsCleared, aEvent );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::impl_clearRedo()
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        SfxUndoManager& rUndoManager = getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( OUString(), getXUndoManager() );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.ClearRedo();
        }

        const EventObject aEvent( getXUndoManager() );
        aGuard.clear();
        // <--- SYNCHRONIZED

        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.notifyEach( g, &XUndoManagerListener::redoActionsCleared, aEvent );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::impl_reset()
    {
        // SYNCHRONIZED --->
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        SfxUndoManager& rUndoManager = getUndoManager();
        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.Reset();
        }

        const EventObject aEvent( getXUndoManager() );
        aGuard.clear();
        // <--- SYNCHRONIZED

        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.notifyEach( g, &XUndoManagerListener::resetAll, aEvent );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::actionUndone( const OUString& i_actionComment )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_actionComment;
        aEvent.UndoContextDepth = 0;    // Undo can happen on level 0 only
        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.notifyEach( g, &XUndoManagerListener::actionUndone, aEvent );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::actionRedone( const OUString& i_actionComment )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_actionComment;
        aEvent.UndoContextDepth = 0;    // Redo can happen on level 0 only
        {
            std::unique_lock g(m_aListenerMutex);
            m_aUndoListeners.notifyEach( g, &XUndoManagerListener::actionRedone, aEvent );
        }
        impl_notifyModified();
    }

    void UndoManagerHelper_Impl::undoActionAdded( const OUString& i_actionComment )
    {
        if ( m_bAPIActionRunning )
            return;

        notify( i_actionComment, &XUndoManagerListener::undoActionAdded );
    }

    void UndoManagerHelper_Impl::cleared()
    {
        if ( m_bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::allActionsCleared );
    }

    void UndoManagerHelper_Impl::clearedRedo()
    {
        if ( m_bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::redoActionsCleared );
    }

    void UndoManagerHelper_Impl::resetAll()
    {
        if ( m_bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::resetAll );
    }

    void UndoManagerHelper_Impl::listActionEntered( const OUString& i_comment )
    {
#if OSL_DEBUG_LEVEL > 0
        m_aContextAPIFlags.push( m_bAPIActionRunning );
        m_bContextAPIFlagsEverPushed = true;
#endif

        if ( m_bAPIActionRunning )
            return;

        notify( i_comment, &XUndoManagerListener::enteredContext );
    }

    void UndoManagerHelper_Impl::listActionLeft( const OUString& i_comment )
    {
#if OSL_DEBUG_LEVEL > 0
        // It may happen that the very first event listener is added during a
        // list action after listActionEntered() was already called, e.g. Calc
        // formula calculation event listener during the input of the very
        // first formula. Instead of checking m_aContextAPIFlags for empty,
        // still assert (on calling top()) other stack mismatches but ignore
        // this one case. See tdf#142980
        if (m_bContextAPIFlagsEverPushed)
        {
            const bool bCurrentContextIsAPIContext = m_aContextAPIFlags.top();
            m_aContextAPIFlags.pop();
            OSL_ENSURE( bCurrentContextIsAPIContext == m_bAPIActionRunning, "UndoManagerHelper_Impl::listActionLeft: API and non-API contexts interwoven!" );
        }
#endif

        if ( m_bAPIActionRunning )
            return;

        notify( i_comment, &XUndoManagerListener::leftContext );
    }

    void UndoManagerHelper_Impl::listActionCancelled()
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = m_aContextAPIFlags.top();
        m_aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == m_bAPIActionRunning, "UndoManagerHelper_Impl::listActionCancelled: API and non-API contexts interwoven!" );
#endif

        if ( m_bAPIActionRunning )
            return;

        notify( OUString(), &XUndoManagerListener::cancelledContext );
    }

    //= UndoManagerHelper

    UndoManagerHelper::UndoManagerHelper( IUndoManagerImplementation& i_undoManagerImpl )
        :m_xImpl( new UndoManagerHelper_Impl( i_undoManagerImpl ) )
    {
    }

    UndoManagerHelper::~UndoManagerHelper()
    {
    }

    void UndoManagerHelper::disposing()
    {
        m_xImpl->disposing();
    }

    void UndoManagerHelper::enterUndoContext( const OUString& i_title, IMutexGuard& i_instanceLock )
    {
        m_xImpl->enterUndoContext( i_title, false, i_instanceLock );
    }

    void UndoManagerHelper::enterHiddenUndoContext( IMutexGuard& i_instanceLock )
    {
        m_xImpl->enterUndoContext( OUString(), true, i_instanceLock );
    }

    void UndoManagerHelper::leaveUndoContext( IMutexGuard& i_instanceLock )
    {
        m_xImpl->leaveUndoContext( i_instanceLock );
    }

    void UndoManagerHelper_Impl::undo( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            [this, &i_instanceLock] () { return this->impl_doUndoRedo(i_instanceLock, true); },
            i_instanceLock
        );
    }

    void UndoManagerHelper_Impl::redo( IMutexGuard& i_instanceLock )
    {
        impl_processRequest(
            [this, &i_instanceLock] () { return this->impl_doUndoRedo(i_instanceLock, false); },
            i_instanceLock
        );
    }

    void UndoManagerHelper::addUndoAction( const Reference< XUndoAction >& i_action, IMutexGuard& i_instanceLock )
    {
        m_xImpl->addUndoAction( i_action, i_instanceLock );
    }

    void UndoManagerHelper::undo( IMutexGuard& i_instanceLock )
    {
        m_xImpl->undo( i_instanceLock );
    }

    void UndoManagerHelper::redo( IMutexGuard& i_instanceLock )
    {
        m_xImpl->redo( i_instanceLock );
    }

    bool UndoManagerHelper::isUndoPossible() const
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( m_xImpl->getMutex() );
        SfxUndoManager& rUndoManager = m_xImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return false;
        return rUndoManager.GetUndoActionCount( SfxUndoManager::TopLevel ) > 0;
        // <--- SYNCHRONIZED
    }

    bool UndoManagerHelper::isRedoPossible() const
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( m_xImpl->getMutex() );
        const SfxUndoManager& rUndoManager = m_xImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return false;
        return rUndoManager.GetRedoActionCount( SfxUndoManager::TopLevel ) > 0;
        // <--- SYNCHRONIZED
    }

    namespace
    {

        OUString lcl_getCurrentActionTitle( UndoManagerHelper_Impl& i_impl, const bool i_undo )
        {
            // SYNCHRONIZED --->
            ::osl::MutexGuard aGuard( i_impl.getMutex() );

            const SfxUndoManager& rUndoManager = i_impl.getUndoManager();
            const size_t nActionCount = i_undo
                                    ?   rUndoManager.GetUndoActionCount( SfxUndoManager::TopLevel )
                                    :   rUndoManager.GetRedoActionCount( SfxUndoManager::TopLevel );
            if ( nActionCount == 0 )
                throw EmptyUndoStackException(
                    i_undo ? u"no action on the undo stack"_ustr
                           : u"no action on the redo stack"_ustr,
                    i_impl.getXUndoManager()
                );
            return  i_undo
                ?   rUndoManager.GetUndoActionComment( 0, SfxUndoManager::TopLevel )
                :   rUndoManager.GetRedoActionComment( 0, SfxUndoManager::TopLevel );
            // <--- SYNCHRONIZED
        }

        Sequence< OUString > lcl_getAllActionTitles( UndoManagerHelper_Impl& i_impl, const bool i_undo )
        {
            // SYNCHRONIZED --->
            ::osl::MutexGuard aGuard( i_impl.getMutex() );

            const SfxUndoManager& rUndoManager = i_impl.getUndoManager();
            const size_t nCount =   i_undo
                                ?   rUndoManager.GetUndoActionCount( SfxUndoManager::TopLevel )
                                :   rUndoManager.GetRedoActionCount( SfxUndoManager::TopLevel );

            Sequence< OUString > aTitles( nCount );
            auto aTitlesRange = asNonConstRange(aTitles);
            for ( size_t i=0; i<nCount; ++i )
            {
                aTitlesRange[i] =    i_undo
                            ?   rUndoManager.GetUndoActionComment( i, SfxUndoManager::TopLevel )
                            :   rUndoManager.GetRedoActionComment( i, SfxUndoManager::TopLevel );
            }
            return aTitles;
            // <--- SYNCHRONIZED
        }
    }

    OUString UndoManagerHelper::getCurrentUndoActionTitle() const
    {
        return lcl_getCurrentActionTitle( *m_xImpl, true );
    }

    OUString UndoManagerHelper::getCurrentRedoActionTitle() const
    {
        return lcl_getCurrentActionTitle( *m_xImpl, false );
    }

    Sequence< OUString > UndoManagerHelper::getAllUndoActionTitles() const
    {
        return lcl_getAllActionTitles( *m_xImpl, true );
    }

    Sequence< OUString > UndoManagerHelper::getAllRedoActionTitles() const
    {
        return lcl_getAllActionTitles( *m_xImpl, false );
    }

    void UndoManagerHelper::clear( IMutexGuard& i_instanceLock )
    {
        m_xImpl->clear( i_instanceLock );
    }

    void UndoManagerHelper::clearRedo( IMutexGuard& i_instanceLock )
    {
        m_xImpl->clearRedo( i_instanceLock );
    }

    void UndoManagerHelper::reset( IMutexGuard& i_instanceLock )
    {
        m_xImpl->reset( i_instanceLock );
    }

    void UndoManagerHelper::lock()
    {
        m_xImpl->lock();
    }

    void UndoManagerHelper::unlock()
    {
        m_xImpl->unlock();
    }

    bool UndoManagerHelper::isLocked()
    {
        // SYNCHRONIZED --->
        ::osl::MutexGuard aGuard( m_xImpl->getMutex() );

        SfxUndoManager& rUndoManager = m_xImpl->getUndoManager();
        return !rUndoManager.IsUndoEnabled();
        // <--- SYNCHRONIZED
    }

    void UndoManagerHelper::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        if ( i_listener.is() )
            m_xImpl->addUndoManagerListener( i_listener );
    }

    void UndoManagerHelper::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        if ( i_listener.is() )
            m_xImpl->removeUndoManagerListener( i_listener );
    }

    void UndoManagerHelper::addModifyListener( const Reference< XModifyListener >& i_listener )
    {
        if ( i_listener.is() )
            m_xImpl->addModifyListener( i_listener );
    }

    void UndoManagerHelper::removeModifyListener( const Reference< XModifyListener >& i_listener )
    {
        if ( i_listener.is() )
            m_xImpl->removeModifyListener( i_listener );
    }

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
