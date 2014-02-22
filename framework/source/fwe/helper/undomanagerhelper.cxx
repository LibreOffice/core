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


namespace framework
{


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
    using ::svl::IUndoManager;

    
    
    
    class UndoActionWrapper : public SfxUndoAction
    {
    public:
                            UndoActionWrapper(
                                Reference< XUndoAction > const& i_undoAction
                            );
        virtual             ~UndoActionWrapper();

        virtual OUString    GetComment() const;
        virtual void        Undo();
        virtual void        Redo();
        virtual bool        CanRepeat(SfxRepeatTarget&) const;

    private:
        const Reference< XUndoAction >  m_xUndoAction;
    };

    
    UndoActionWrapper::UndoActionWrapper( Reference< XUndoAction > const& i_undoAction )
        :SfxUndoAction()
        ,m_xUndoAction( i_undoAction )
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
            DBG_UNHANDLED_EXCEPTION();
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
            DBG_UNHANDLED_EXCEPTION();
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
                OUString( "Concurrency error: an ealier operation on the stack failed." ),
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

        
        IUndoManager& getUndoManager() const
        {
            return m_rUndoManagerImplementation.getImplUndoManager();
        }

        
        Reference< XUndoManager > getXUndoManager() const
        {
            return m_rUndoManagerImplementation.getThis();
        }

        
        virtual void actionUndone( const OUString& i_actionComment );
        virtual void actionRedone( const OUString& i_actionComment );
        virtual void undoActionAdded( const OUString& i_actionComment );
        virtual void cleared();
        virtual void clearedRedo();
        virtual void resetAll();
        virtual void listActionEntered( const OUString& i_comment );
        virtual void listActionLeft( const OUString& i_comment );
        virtual void listActionLeftAndMerged();
        virtual void listActionCancelled();
        virtual void undoManagerDying();

        
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
            buildEvent( OUString const& i_title ) const;

        void impl_notifyModified();
        void notify(    OUString const& i_title,
                        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& )
                    );
        void notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ) )
        {
            notify( OUString(), i_notificationMethod );
        }

        void notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ) );

    private:
        
        void impl_processRequest( ::boost::function0< void > const& i_request, IMutexGuard& i_instanceLock );

        
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
        m_aUndoListeners.disposeAndClear( aEvent );
        m_aModifyListeners.disposeAndClear( aEvent );

        ::osl::MutexGuard aGuard( m_aMutex );

        getUndoManager().RemoveUndoListener( *this );

        m_disposed = true;
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
        m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvent );
    }

    
    void UndoManagerHelper_Impl::notify( OUString const& i_title,
        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ) )
    {
        const UndoManagerEvent aEvent( buildEvent( i_title ) );

        
        
        
        
        

        m_aUndoListeners.notifyEach( i_notificationMethod, aEvent );
        impl_notifyModified();
    }

    
    void UndoManagerHelper_Impl::notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ) )
    {
        const EventObject aEvent( getXUndoManager() );

        

        m_aUndoListeners.notifyEach( i_notificationMethod, aEvent );
        impl_notifyModified();
    }

    
    void UndoManagerHelper_Impl::enterUndoContext( const OUString& i_title, const bool i_hidden, IMutexGuard& i_instanceLock )
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

    
    void UndoManagerHelper_Impl::addUndoAction( const Reference< XUndoAction >& i_action, IMutexGuard& i_instanceLock )
    {
        if ( !i_action.is() )
            throw IllegalArgumentException(
                OUString( "illegal undo action object" ),
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

    
    void UndoManagerHelper_Impl::lock()
    {
        
        ::osl::MutexGuard aGuard( getMutex() );

        if ( ++m_nLockCount == 1 )
        {
            IUndoManager& rUndoManager = getUndoManager();
            rUndoManager.EnableUndo( false );
        }
        
    }

    
    void UndoManagerHelper_Impl::unlock()
    {
        
        ::osl::MutexGuard aGuard( getMutex() );

        if ( m_nLockCount == 0 )
            throw NotLockedException( "Undo manager is not locked", getXUndoManager() );

        if ( --m_nLockCount == 0 )
        {
            IUndoManager& rUndoManager = getUndoManager();
            rUndoManager.EnableUndo( true );
        }
        
    }

    
    void UndoManagerHelper_Impl::impl_processRequest( ::boost::function0< void > const& i_request, IMutexGuard& i_instanceLock )
    {
        
        ::rtl::Reference< UndoManagerRequest > pRequest( new UndoManagerRequest( i_request ) );
        {
            ::osl::MutexGuard aQueueGuard( m_aQueueMutex );
            m_aEventQueue.push( pRequest );
        }

        i_instanceLock.clear();

        if ( m_bProcessingEvents )
        {
            
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
                    
                    
                    ::osl::MutexGuard aQueueGuard( m_aQueueMutex );
                    while ( !m_aEventQueue.empty() )
                    {
                        pRequest = m_aEventQueue.front();
                        m_aEventQueue.pop();
                        pRequest->cancel( getXUndoManager() );
                    }
                    m_bProcessingEvents = false;
                }
                
                throw;
            }
        }
        while ( true );
    }

    
    void UndoManagerHelper_Impl::impl_enterUndoContext( const OUString& i_title, const bool i_hidden )
    {
        
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            
            return;

        if ( i_hidden && ( rUndoManager.GetUndoActionCount( IUndoManager::CurrentLevel ) == 0 ) )
            throw EmptyUndoStackException(
                OUString( "can't enter a hidden context without a previous Undo action" ),
                m_rUndoManagerImplementation.getThis()
            );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.EnterListAction( i_title, OUString() );
        }

        m_aContextVisibilities.push( i_hidden );

        const UndoManagerEvent aEvent( buildEvent( i_title ) );
        aGuard.clear();
        

        m_aUndoListeners.notifyEach( i_hidden ? &XUndoManagerListener::enteredHiddenContext : &XUndoManagerListener::enteredContext, aEvent );
        impl_notifyModified();
    }

    
    void UndoManagerHelper_Impl::impl_leaveUndoContext()
    {
        
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            
            return;

        if ( !rUndoManager.IsInListAction() )
            throw InvalidStateException(
                OUString( "no active undo context" ),
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

        
        void ( SAL_CALL XUndoManagerListener::*notificationMethod )( const UndoManagerEvent& ) = NULL;

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
            aContextEvent.UndoActionTitle = rUndoManager.GetUndoActionComment( 0, IUndoManager::CurrentLevel );
            notificationMethod = &XUndoManagerListener::leftContext;
        }

        aGuard.clear();
        

        if ( bHadRedoActions && !bHasRedoActions )
            m_aUndoListeners.notifyEach( &XUndoManagerListener::redoActionsCleared, aClearedEvent );
        m_aUndoListeners.notifyEach( notificationMethod, aContextEvent );
        impl_notifyModified();
    }

    
    void UndoManagerHelper_Impl::impl_doUndoRedo( IMutexGuard& i_externalLock, const bool i_undo )
    {
        ::osl::Guard< ::framework::IMutex > aExternalGuard( i_externalLock.getGuardedMutex() );
            
            

        
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( OUString(), getXUndoManager() );

        const size_t nElements  =   i_undo
                                ?   rUndoManager.GetUndoActionCount( IUndoManager::TopLevel )
                                :   rUndoManager.GetRedoActionCount( IUndoManager::TopLevel );
        if ( nElements == 0 )
            throw EmptyUndoStackException("stack is empty", getXUndoManager() );

        aGuard.clear();
        

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
            
            const Any aError( ::cppu::getCaughtException() );
            throw UndoFailedException( OUString(), getXUndoManager(), aError );
        }

        
        
        
        
        
        
        
    }

    
    void UndoManagerHelper_Impl::impl_addUndoAction( const Reference< XUndoAction >& i_action )
    {
        
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            
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
        

        m_aUndoListeners.notifyEach( &XUndoManagerListener::undoActionAdded, aEventAdd );
        if ( bHadRedoActions && !bHasRedoActions )
            m_aUndoListeners.notifyEach( &XUndoManagerListener::redoActionsCleared, aEventClear );
        impl_notifyModified();
    }

    
    void UndoManagerHelper_Impl::impl_clear()
    {
        
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( OUString(), getXUndoManager() );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.Clear();
        }

        const EventObject aEvent( getXUndoManager() );
        aGuard.clear();
        

        m_aUndoListeners.notifyEach( &XUndoManagerListener::allActionsCleared, aEvent );
        impl_notifyModified();
    }

    
    void UndoManagerHelper_Impl::impl_clearRedo()
    {
        
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( OUString(), getXUndoManager() );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.ClearRedo();
        }

        const EventObject aEvent( getXUndoManager() );
        aGuard.clear();
        

        m_aUndoListeners.notifyEach( &XUndoManagerListener::redoActionsCleared, aEvent );
        impl_notifyModified();
    }

    
    void UndoManagerHelper_Impl::impl_reset()
    {
        
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        IUndoManager& rUndoManager = getUndoManager();
        {
            ::comphelper::FlagGuard aNotificationGuard( m_bAPIActionRunning );
            rUndoManager.Reset();
        }

        const EventObject aEvent( getXUndoManager() );
        aGuard.clear();
        

        m_aUndoListeners.notifyEach( &XUndoManagerListener::resetAll, aEvent );
        impl_notifyModified();
    }

    
    void UndoManagerHelper_Impl::actionUndone( const OUString& i_actionComment )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_actionComment;
        aEvent.UndoContextDepth = 0;    
        m_aUndoListeners.notifyEach( &XUndoManagerListener::actionUndone, aEvent );
        impl_notifyModified();
    }

     
    void UndoManagerHelper_Impl::actionRedone( const OUString& i_actionComment )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_actionComment;
        aEvent.UndoContextDepth = 0;    
        m_aUndoListeners.notifyEach( &XUndoManagerListener::actionRedone, aEvent );
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
#endif

        if ( m_bAPIActionRunning )
            return;

        notify( i_comment, &XUndoManagerListener::enteredContext );
    }

     
    void UndoManagerHelper_Impl::listActionLeft( const OUString& i_comment )
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

     
    void UndoManagerHelper_Impl::undoManagerDying()
    {
        
    }

    
    
    
    
    UndoManagerHelper::UndoManagerHelper( IUndoManagerImplementation& i_undoManagerImpl )
        :m_pImpl( new UndoManagerHelper_Impl( i_undoManagerImpl ) )
    {
    }

    
    UndoManagerHelper::~UndoManagerHelper()
    {
    }

    
    void UndoManagerHelper::disposing()
    {
        m_pImpl->disposing();
    }

    
    void UndoManagerHelper::enterUndoContext( const OUString& i_title, IMutexGuard& i_instanceLock )
    {
        m_pImpl->enterUndoContext( i_title, false, i_instanceLock );
    }

    
    void UndoManagerHelper::enterHiddenUndoContext( IMutexGuard& i_instanceLock )
    {
        m_pImpl->enterUndoContext( OUString(), true, i_instanceLock );
    }

    
    void UndoManagerHelper::leaveUndoContext( IMutexGuard& i_instanceLock )
    {
        m_pImpl->leaveUndoContext( i_instanceLock );
    }

    
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

    
    void UndoManagerHelper::addUndoAction( const Reference< XUndoAction >& i_action, IMutexGuard& i_instanceLock )
    {
        m_pImpl->addUndoAction( i_action, i_instanceLock );
    }

    
    void UndoManagerHelper::undo( IMutexGuard& i_instanceLock )
    {
        m_pImpl->undo( i_instanceLock );
    }

    
    void UndoManagerHelper::redo( IMutexGuard& i_instanceLock )
    {
        m_pImpl->redo( i_instanceLock );
    }

    
    ::sal_Bool UndoManagerHelper::isUndoPossible() const
    {
        
        ::osl::MutexGuard aGuard( m_pImpl->getMutex() );
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return sal_False;
        return rUndoManager.GetUndoActionCount( IUndoManager::TopLevel ) > 0;
        
    }

    
    ::sal_Bool UndoManagerHelper::isRedoPossible() const
    {
        
        ::osl::MutexGuard aGuard( m_pImpl->getMutex() );
        const IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return sal_False;
        return rUndoManager.GetRedoActionCount( IUndoManager::TopLevel ) > 0;
        
    }

    
    namespace
    {
        
        OUString lcl_getCurrentActionTitle( UndoManagerHelper_Impl& i_impl, const bool i_undo )
        {
            
            ::osl::MutexGuard aGuard( i_impl.getMutex() );

            const IUndoManager& rUndoManager = i_impl.getUndoManager();
            const size_t nActionCount = i_undo
                                    ?   rUndoManager.GetUndoActionCount( IUndoManager::TopLevel )
                                    :   rUndoManager.GetRedoActionCount( IUndoManager::TopLevel );
            if ( nActionCount == 0 )
                throw EmptyUndoStackException(
                    i_undo ? OUString( "no action on the undo stack" )
                           : OUString( "no action on the redo stack" ),
                    i_impl.getXUndoManager()
                );
            return  i_undo
                ?   rUndoManager.GetUndoActionComment( 0, IUndoManager::TopLevel )
                :   rUndoManager.GetRedoActionComment( 0, IUndoManager::TopLevel );
            
        }

        
        Sequence< OUString > lcl_getAllActionTitles( UndoManagerHelper_Impl& i_impl, const bool i_undo )
        {
            
            ::osl::MutexGuard aGuard( i_impl.getMutex() );

            const IUndoManager& rUndoManager = i_impl.getUndoManager();
            const size_t nCount =   i_undo
                                ?   rUndoManager.GetUndoActionCount( IUndoManager::TopLevel )
                                :   rUndoManager.GetRedoActionCount( IUndoManager::TopLevel );

            Sequence< OUString > aTitles( nCount );
            for ( size_t i=0; i<nCount; ++i )
            {
                aTitles[i] =    i_undo
                            ?   rUndoManager.GetUndoActionComment( i, IUndoManager::TopLevel )
                            :   rUndoManager.GetRedoActionComment( i, IUndoManager::TopLevel );
            }
            return aTitles;
            
        }
    }

    
    OUString UndoManagerHelper::getCurrentUndoActionTitle() const
    {
        return lcl_getCurrentActionTitle( *m_pImpl, true );
    }

    
    OUString UndoManagerHelper::getCurrentRedoActionTitle() const
    {
        return lcl_getCurrentActionTitle( *m_pImpl, false );
    }

    
    Sequence< OUString > UndoManagerHelper::getAllUndoActionTitles() const
    {
        return lcl_getAllActionTitles( *m_pImpl, true );
    }

    
    Sequence< OUString > UndoManagerHelper::getAllRedoActionTitles() const
    {
        return lcl_getAllActionTitles( *m_pImpl, false );
    }

    
    void UndoManagerHelper::clear( IMutexGuard& i_instanceLock )
    {
        m_pImpl->clear( i_instanceLock );
    }

    
    void UndoManagerHelper::clearRedo( IMutexGuard& i_instanceLock )
    {
        m_pImpl->clearRedo( i_instanceLock );
    }

    
    void UndoManagerHelper::reset( IMutexGuard& i_instanceLock )
    {
        m_pImpl->reset( i_instanceLock );
    }

    
    void UndoManagerHelper::lock()
    {
        m_pImpl->lock();
    }

    
    void UndoManagerHelper::unlock()
    {
        m_pImpl->unlock();
    }

    
    ::sal_Bool UndoManagerHelper::isLocked()
    {
        
        ::osl::MutexGuard aGuard( m_pImpl->getMutex() );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        return !rUndoManager.IsUndoEnabled();
        
    }

    
    void UndoManagerHelper::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->addUndoManagerListener( i_listener );
    }

    
    void UndoManagerHelper::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->removeUndoManagerListener( i_listener );
    }

    
    void UndoManagerHelper::addModifyListener( const Reference< XModifyListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->addModifyListener( i_listener );
    }

    
    void UndoManagerHelper::removeModifyListener( const Reference< XModifyListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->removeModifyListener( i_listener );
    }


} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
