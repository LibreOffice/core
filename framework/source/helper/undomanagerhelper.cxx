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

#include "precompiled_framework.hxx"

#include "helper/undomanagerhelper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XComponent.hpp>
/** === end UNO includes === **/

#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/flagguard.hxx>
#include <svl/undo.hxx>
#include <tools/diagnose_ex.h>

#include <stack>

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

        virtual String      GetComment() const;
        virtual void        Undo();
        virtual void        Redo();
        virtual BOOL        CanRepeat(SfxRepeatTarget&) const;

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
    String UndoActionWrapper::GetComment() const
    {
        String sComment;
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
    BOOL UndoActionWrapper::CanRepeat(SfxRepeatTarget&) const
    {
        return FALSE;
    }

    //==================================================================================================================
    //= UndoManagerHelper_Impl
    //==================================================================================================================
    class UndoManagerHelper_Impl : public SfxUndoListener
    {
    public:
        ::cppu::OInterfaceContainerHelper   aUndoListeners;
        IUndoManagerImplementation&         rUndoManagerImplementation;
        UndoManagerHelper&                  rAntiImpl;
        bool                                bAPIActionRunning;
        ::std::stack< bool >                aContextVisibilities;
#if OSL_DEBUG_LEVEL > 0
        ::std::stack< bool >                aContextAPIFlags;
#endif

        UndoManagerHelper_Impl( UndoManagerHelper& i_antiImpl, IUndoManagerImplementation& i_undoManagerImpl )
            :aUndoListeners( i_undoManagerImpl.getMutex() )
            ,rUndoManagerImplementation( i_undoManagerImpl )
            ,rAntiImpl( i_antiImpl )
            ,bAPIActionRunning( false )
        {
            getUndoManager().AddUndoListener( *this );
        }

        virtual ~UndoManagerHelper_Impl()
        {
        }

        //..............................................................................................................
        IUndoManager& getUndoManager()
        {
            return rUndoManagerImplementation.getImplUndoManager();
        }

        //..............................................................................................................
        Reference< XUndoManager > getXUndoManager()
        {
            return rUndoManagerImplementation.getThis();
        }

        //..............................................................................................................
        void disposing()
        {
            EventObject aEvent;
            aEvent.Source = getXUndoManager();
            aUndoListeners.disposeAndClear( aEvent );

            getUndoManager().RemoveUndoListener( *this );
        }

        // SfxUndoListener
        virtual void actionUndone( const String& i_actionComment );
        virtual void actionRedone( const String& i_actionComment );
        virtual void undoActionAdded( const String& i_actionComment );
        virtual void cleared();
        virtual void clearedRedo();
        virtual void listActionEntered( const String& i_comment );
        virtual void listActionLeft();
        virtual void listActionLeftAndMerged();
        virtual void listActionCancelled();
        virtual void undoManagerDying();

        // public operations
        void enterUndoContext( const ::rtl::OUString& i_title, const bool i_hidden, IClearableInstanceLock& i_instanceLock );

        void doUndoRedo(
                USHORT ( ::svl::IUndoManager::*i_checkMethod )( bool const ) const,
                BOOL ( ::svl::IUndoManager::*i_doMethod )(),
                UniString ( ::svl::IUndoManager::*i_titleRetriever )( USHORT, bool const ) const,
                void ( SAL_CALL ::com::sun::star::document::XUndoManagerListener::*i_notificationMethod )( const ::com::sun::star::document::UndoManagerEvent& ),
                IClearableInstanceLock& i_instanceLock
            );
        void notify(    ::rtl::OUString const& i_title,
                        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ),
                        IClearableInstanceLock& i_instanceLock
                    );
        void notify(    void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ),
                        IClearableInstanceLock& i_instanceLock
                    );
        void notify(    ::rtl::OUString const& i_title,
                        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& )
                    );
        void notify(
                        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& )
                    );
    };

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::notify( ::rtl::OUString const& i_title, void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ),
        IClearableInstanceLock& i_instanceLock )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_title;
        aEvent.UndoContextDepth = getUndoManager().GetListActionDepth();

        i_instanceLock.clear();
        aUndoListeners.notifyEach( i_notificationMethod, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ),
        IClearableInstanceLock& i_instanceLock )
    {
        EventObject aEvent;
        aEvent.Source = getXUndoManager();
        i_instanceLock.clear();
        aUndoListeners.notifyEach( i_notificationMethod, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::notify( ::rtl::OUString const& i_title,
        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ) )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = getXUndoManager();
        aEvent.UndoActionTitle = i_title;
        aEvent.UndoContextDepth = getUndoManager().GetListActionDepth();

        // TODO: this notification method here is used by UndoManagerHelper_Impl, to multiplex the notifications we
        // receive from the IUndoManager. Those notitications are sent with a locked SolarMutex, which means
        // we're doing the multiplexing here with a locked SM, too. Which is Bad (TM).
        // Fixing this properly would require outsourcing all the notifications into an own thread - which might lead
        // to problems of its own, since clients might expect synchronous notifications.

        aUndoListeners.notifyEach( i_notificationMethod, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ) )
    {
        EventObject aEvent;
        aEvent.Source = getXUndoManager();

        // TODO: the same comment as in the other notify, regarding SM locking applies here ...

        aUndoListeners.notifyEach( i_notificationMethod, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::enterUndoContext( const ::rtl::OUString& i_title, const bool i_hidden, IClearableInstanceLock& i_instanceLock )
    {
        // SYNCHRONIZED --->
        IUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore this request if the manager is locked
            return;

        if ( i_hidden && ( rUndoManager.GetUndoActionCount( IUndoManager::CurrentLevel ) == 0 ) )
            throw EmptyUndoStackException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "can't enter a hidden context without a previous Undo action" ) ),
                rUndoManagerImplementation.getThis()
            );

        {
            ::comphelper::FlagGuard aNotificationGuard( bAPIActionRunning );
            rUndoManager.EnterListAction( i_title, ::rtl::OUString() );
        }

        aContextVisibilities.push( i_hidden );

        notify( i_title, i_hidden ? &XUndoManagerListener::enteredHiddenContext : &XUndoManagerListener::enteredContext, i_instanceLock );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::doUndoRedo(
        USHORT ( IUndoManager::*i_checkMethod )( bool const ) const, BOOL ( IUndoManager::*i_doMethod )(),
        String ( IUndoManager::*i_titleRetriever )( USHORT, bool const ) const,
        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ),
        IClearableInstanceLock& i_instanceLock )
    {
        // SYNCHRONIZED --->
        IUndoManager& rUndoManager = getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( ::rtl::OUString(), getXUndoManager() );

        if ( (rUndoManager.*i_checkMethod)( IUndoManager::TopLevel ) == 0 )
            throw EmptyUndoStackException( ::rtl::OUString::createFromAscii( "stack is empty" ), getXUndoManager() );

        const ::rtl::OUString sUndoActionTitle = (rUndoManager.*i_titleRetriever)( 0, IUndoManager::TopLevel );
        {
            ::comphelper::FlagGuard aNotificationGuard( bAPIActionRunning );
            try
            {
                (rUndoManager.*i_doMethod)();
            }
            catch( const RuntimeException& ) { /* allowed to leave here */ throw; }
            catch( const UndoFailedException& ) { /* allowed to leave here */ throw; }
            catch( const Exception& )
            {
                // not allowed to leave
                const Any aError( ::cppu::getCaughtException() );
                throw UndoFailedException( ::rtl::OUString(), getXUndoManager(), aError );
            }
        }

        notify( sUndoActionTitle, i_notificationMethod, i_instanceLock );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::actionUndone( const String& i_actionComment )
    {
        if ( bAPIActionRunning )
            return;

        notify( i_actionComment, &XUndoManagerListener::actionUndone );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::actionRedone( const String& i_actionComment )
    {
        if ( bAPIActionRunning )
            return;

        notify( i_actionComment, &XUndoManagerListener::actionRedone );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::undoActionAdded( const String& i_actionComment )
    {
        if ( bAPIActionRunning )
            return;

        notify( i_actionComment, &XUndoManagerListener::undoActionAdded );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::cleared()
    {
        if ( bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::allActionsCleared );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::clearedRedo()
    {
        if ( bAPIActionRunning )
            return;

        notify( &XUndoManagerListener::redoActionsCleared );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::listActionEntered( const String& i_comment )
    {
#if OSL_DEBUG_LEVEL > 0
        aContextAPIFlags.push( bAPIActionRunning );
#endif

        if ( bAPIActionRunning )
            return;

        notify( i_comment, &XUndoManagerListener::enteredContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::listActionLeft()
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = aContextAPIFlags.top();
        aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == bAPIActionRunning, "UndoManagerHelper_Impl::listActionLeft: API and non-API contexts interwoven!" );
#endif

        if ( bAPIActionRunning )
            return;

        notify( getUndoManager().GetUndoActionComment( 0, IUndoManager::CurrentLevel ), &XUndoManagerListener::leftContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::listActionLeftAndMerged()
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = aContextAPIFlags.top();
        aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == bAPIActionRunning, "UndoManagerHelper_Impl::listActionLeftAndMerged: API and non-API contexts interwoven!" );
#endif

        if ( bAPIActionRunning )
            return;

        notify( ::rtl::OUString(), &XUndoManagerListener::leftHiddenContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper_Impl::listActionCancelled()
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = aContextAPIFlags.top();
        aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == bAPIActionRunning, "UndoManagerHelper_Impl::listActionCancelled: API and non-API contexts interwoven!" );
#endif

        if ( bAPIActionRunning )
            return;

        notify( ::rtl::OUString(), &XUndoManagerListener::cancelledContext );
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
        :m_pImpl( new UndoManagerHelper_Impl( *this, i_undoManagerImpl ) )
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
    void UndoManagerHelper::enterUndoContext( const ::rtl::OUString& i_title, IClearableInstanceLock& i_instanceLock )
    {
        m_pImpl->enterUndoContext( i_title, false, i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::enterHiddenUndoContext( IClearableInstanceLock& i_instanceLock )
    {
        m_pImpl->enterUndoContext( ::rtl::OUString(), true, i_instanceLock );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::leaveUndoContext( IClearableInstanceLock& i_instanceLock )
    {
        // SYNCHRONIZED --->
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore this request if the manager is locked
            return;

        if ( !rUndoManager.IsInListAction() )
            throw InvalidStateException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no active undo context" ) ),
                m_pImpl->getXUndoManager()
            );

        USHORT nContextElements = 0;
        bool isHiddenContext = false;
        {
            ::comphelper::FlagGuard aNotificationGuard( m_pImpl->bAPIActionRunning );

            isHiddenContext = m_pImpl->aContextVisibilities.top();
            m_pImpl->aContextVisibilities.pop();
            if ( isHiddenContext )
                nContextElements = rUndoManager.LeaveAndMergeListAction();
            else
                nContextElements = rUndoManager.LeaveListAction();
        }

        if ( nContextElements == 0 )
            m_pImpl->notify( ::rtl::OUString(), &XUndoManagerListener::cancelledContext, i_instanceLock );
        else if ( isHiddenContext )
            m_pImpl->notify( ::rtl::OUString(), &XUndoManagerListener::leftHiddenContext, i_instanceLock );
        else
            m_pImpl->notify( rUndoManager.GetUndoActionComment( 0, IUndoManager::CurrentLevel ), &XUndoManagerListener::leftContext, i_instanceLock );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::addUndoAction( const Reference< XUndoAction >& i_action, IClearableInstanceLock& i_instanceLock )
    {
        // SYNCHRONIZED --->
        if ( !i_action.is() )
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal undo action object" ) ),
                m_pImpl->getXUndoManager(),
                1
            );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore the request if the manager is locked
            return;

        const bool bHadRedoActions = ( rUndoManager.GetRedoActionCount( IUndoManager::CurrentLevel ) > 0 );
        {
            ::comphelper::FlagGuard aNotificationGuard( m_pImpl->bAPIActionRunning );
            rUndoManager.AddUndoAction( new UndoActionWrapper( i_action ) );
        }
        const bool bHasRedoActions = ( rUndoManager.GetRedoActionCount( IUndoManager::CurrentLevel ) > 0 );

        m_pImpl->notify( i_action->getTitle(), &XUndoManagerListener::undoActionAdded, i_instanceLock );
        // <--- SYNCHRONIZED

        if ( bHadRedoActions && !bHasRedoActions )
            m_pImpl->notify( &XUndoManagerListener::redoActionsCleared );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::undo( IClearableInstanceLock& i_instanceLock )
    {
        m_pImpl->doUndoRedo(
            &IUndoManager::GetUndoActionCount,
            &IUndoManager::Undo,
            &IUndoManager::GetUndoActionComment,
            &XUndoManagerListener::actionUndone,
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::redo( IClearableInstanceLock& i_instanceLock )
    {
        m_pImpl->doUndoRedo(
            &IUndoManager::GetRedoActionCount,
            &IUndoManager::Redo,
            &IUndoManager::GetRedoActionComment,
            &XUndoManagerListener::actionRedone,
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool UndoManagerHelper::isUndoPossible() const
    {
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return sal_False;
        return rUndoManager.GetUndoActionCount( IUndoManager::TopLevel ) > 0;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool UndoManagerHelper::isRedoPossible() const
    {
        const IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return sal_False;
        return rUndoManager.GetRedoActionCount( IUndoManager::TopLevel ) > 0;
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        //..............................................................................................................
        ::rtl::OUString lcl_getCurrentActionTitle( UndoManagerHelper_Impl& i_impl, const bool i_undo )
        {
            const IUndoManager& rUndoManager = i_impl.getUndoManager();
            const USHORT nActionCount = i_undo
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
        }

        //..............................................................................................................
        Sequence< ::rtl::OUString > lcl_getAllActionTitles( UndoManagerHelper_Impl& i_impl, const bool i_undo )
        {
            const IUndoManager& rUndoManager = i_impl.getUndoManager();
            const USHORT nCount =   i_undo
                                ?   rUndoManager.GetUndoActionCount( IUndoManager::TopLevel )
                                :   rUndoManager.GetRedoActionCount( IUndoManager::TopLevel );

            Sequence< ::rtl::OUString > aTitles( nCount );
            for ( USHORT i=0; i<nCount; ++i )
            {
                aTitles[i] =    i_undo
                            ?   rUndoManager.GetUndoActionComment( i, IUndoManager::TopLevel )
                            :   rUndoManager.GetRedoActionComment( i, IUndoManager::TopLevel );
            }
            return aTitles;
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
    void UndoManagerHelper::clear( IClearableInstanceLock& i_instanceLock )
    {
        // SYNCHRONIZED --->
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( ::rtl::OUString(), m_pImpl->getXUndoManager() );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_pImpl->bAPIActionRunning );
            rUndoManager.Clear();
        }

        m_pImpl->notify( &XUndoManagerListener::allActionsCleared, i_instanceLock );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::clearRedo( IClearableInstanceLock& i_instanceLock )
    {
        // SYNCHRONIZED --->
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( ::rtl::OUString(), m_pImpl->getXUndoManager() );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_pImpl->bAPIActionRunning );
            rUndoManager.ClearRedo();
        }

        m_pImpl->notify( &XUndoManagerListener::redoActionsCleared, i_instanceLock );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::reset( IClearableInstanceLock& i_instanceLock )
    {
        // SYNCHRONIZED --->
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        {
            ::comphelper::FlagGuard aNotificationGuard( m_pImpl->bAPIActionRunning );
            while ( rUndoManager.IsInListAction() )
                rUndoManager.LeaveListAction();
            rUndoManager.Clear();
        }

        m_pImpl->notify( &XUndoManagerListener::resetAll, i_instanceLock );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::lock()
    {
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        rUndoManager.EnableUndo( false );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::unlock()
    {
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsUndoEnabled() )
            throw NotLockedException( ::rtl::OUString::createFromAscii( "Undo manager is not locked" ), m_pImpl->getXUndoManager() );
        rUndoManager.EnableUndo( true );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool UndoManagerHelper::isLocked()
    {
        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        return !rUndoManager.IsUndoEnabled();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->aUndoListeners.addInterface( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManagerHelper::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        if ( i_listener.is() )
            m_pImpl->aUndoListeners.removeInterface( i_listener );
    }

//......................................................................................................................
} // namespace framework
//......................................................................................................................
