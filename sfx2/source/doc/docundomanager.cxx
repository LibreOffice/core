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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "docundomanager.hxx"
#include "sfx2/sfxbasemodel.hxx"
#include "sfx2/objsh.hxx"
#include "sfx2/viewfrm.hxx"
#include "sfx2/viewsh.hxx"
#include "sfx2/bindings.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XComponent.hpp>
/** === end UNO includes === **/

#include <comphelper/anytostring.hxx>
#include <comphelper/flagguard.hxx>
#include <svl/undo.hxx>
#include <tools/diagnose_ex.h>

#include <stack>

//......................................................................................................................
namespace sfx2
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
    using ::com::sun::star::util::InvalidStateException;
    using ::com::sun::star::document::EmptyUndoStackException;
    using ::com::sun::star::util::NotLockedException;
    using ::com::sun::star::document::UndoContextNotClosedException;
    using ::com::sun::star::document::XUndoAction;
    using ::com::sun::star::document::XUndoManagerSupplier;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::NotInitializedException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::document::UndoManagerEvent;
    using ::com::sun::star::document::XUndoManagerListener;
    using ::com::sun::star::document::UndoFailedException;
    using ::com::sun::star::document::XUndoManager;
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
    //= helper
    //==================================================================================================================
    namespace
    {
#ifdef DBG_UTIL
        struct UndoManagerDiagnosticsGuard
        {
            UndoManagerDiagnosticsGuard( const ::svl::IUndoManager& i_undoManager )
                :m_pUndoManager( dynamic_cast< SfxUndoManager* >( const_cast< ::svl::IUndoManager* >( &i_undoManager ) ) )
            {
                if ( m_pUndoManager )
                    m_pUndoManager->DbgEnableCompatibilityAssertions( false );
            }

            ~UndoManagerDiagnosticsGuard()
            {
                if ( m_pUndoManager )
                    m_pUndoManager->DbgEnableCompatibilityAssertions( true );
            }

        private:
            SfxUndoManager* m_pUndoManager;
        };
#else
        struct UndoManagerDiagnosticsGuard
        {
            UndoManagerDiagnosticsGuard( const ::svl::IUndoManager& )
            {
            }
        };
#endif
    }

    //==================================================================================================================
    //= DocumentUndoManager_Impl
    //==================================================================================================================
    struct DocumentUndoManager_Impl : public SfxUndoListener
    {
        ::cppu::OInterfaceContainerHelper   aUndoListeners;
        IUndoManager*                pUndoManager;
        DocumentUndoManager&                rAntiImpl;
        bool                                bAPIActionRunning;
        ::std::stack< bool >                aContextVisibilities;
        ::std::stack< ::rtl::OUString >     aContextTitles;
#if OSL_DEBUG_LEVEL > 0
        ::std::stack< bool >                aContextAPIFlags;
#endif

        DocumentUndoManager_Impl( DocumentUndoManager& i_antiImpl )
            :aUndoListeners( i_antiImpl.getMutex() )
            ,pUndoManager( NULL )
            ,rAntiImpl( i_antiImpl )
            ,bAPIActionRunning( false )
        {
            SfxObjectShell* pObjectShell = i_antiImpl.getBaseModel().GetObjectShell();
            if ( pObjectShell != NULL )
                pUndoManager = pObjectShell->GetUndoManager();
            if ( !pUndoManager )
                throw NotInitializedException( ::rtl::OUString(), *&i_antiImpl.getBaseModel() );
            // TODO: we probably should add ourself as listener to the SfxObjectShell, in case somebody sets a new
            // UndoManager
            // (well, adding a listener for this is not possible currently, but I also think that setting a new
            // UndoManager does not happen in real life)
            pUndoManager->AddUndoListener( *this );
        }

        const SfxObjectShell* getObjectShell() const { return rAntiImpl.getBaseModel().GetObjectShell(); }
              SfxObjectShell* getObjectShell()       { return rAntiImpl.getBaseModel().GetObjectShell(); }

        //..............................................................................................................
        IUndoManager& getUndoManager()
        {
            ENSURE_OR_THROW( pUndoManager != NULL, "DocumentUndoManager_Impl::getUndoManager: no access to the doc's UndoManager implementation!" );

#if OSL_DEBUG_LEVEL > 0
            // in a non-product build, assert if the current UndoManager at the shell is not the same we obtained
            // (and cached) at construction time
            SfxObjectShell* pObjectShell = rAntiImpl.getBaseModel().GetObjectShell();
            OSL_ENSURE( ( pObjectShell != NULL ) && ( pUndoManager == pObjectShell->GetUndoManager() ),
                "DocumentUndoManager_Impl::getUndoManager: the UndoManager changed meanwhile - what about our listener?" );
#endif

            return *pUndoManager;
        }

        void disposing()
        {
            ENSURE_OR_RETURN_VOID( pUndoManager, "DocumentUndoManager_Impl::disposing: already disposed!" );
            pUndoManager->RemoveUndoListener( *this );
            pUndoManager = NULL;
        }

        const ::rtl::OUString& getTopContextTitle() const { return aContextTitles.top(); }

        // SfxUndoListener
        virtual void actionUndone( SfxUndoAction& i_action );
        virtual void actionRedone( SfxUndoAction& i_action );
        virtual void undoActionAdded( SfxUndoAction& i_action );
        virtual void cleared();
        virtual void clearedRedo();
        virtual void listActionEntered( const String& i_comment );
        virtual void listActionLeft();
        virtual void listActionLeftAndMerged();
        virtual void listActionCancelled();
        virtual void undoManagerDying();

        // public operations
        void enterUndoContext( const ::rtl::OUString& i_title, const bool i_hidden );
    };

     //==================================================================================================================
    namespace
    {
        //..............................................................................................................
        void lcl_invalidateXDo( const DocumentUndoManager_Impl& i_impl )
        {
            const SfxObjectShell* pDocShell = i_impl.getObjectShell();
            ENSURE_OR_THROW( pDocShell != NULL, "lcl_invalidateUndo: no access to the doc shell!" );
            SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( pDocShell );
            while ( pViewFrame )
            {
                pViewFrame->GetBindings().Invalidate( SID_UNDO );
                pViewFrame->GetBindings().Invalidate( SID_REDO );
                pViewFrame = SfxViewFrame::GetNext( *pViewFrame, pDocShell );
            }
        }

    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::enterUndoContext( const ::rtl::OUString& i_title, const bool i_hidden )
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( rAntiImpl );

        IUndoManager& rUndoManager = getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore this request if the manager is locked
            return;

        if ( i_hidden && ( rUndoManager.GetUndoActionCount( IUndoManager::CurrentLevel ) == 0 ) )
            throw EmptyUndoStackException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "can't enter a hidden context without a previous Undo action" ) ),
                static_cast< XUndoManager* >( &rAntiImpl )
            );

        {
            ::comphelper::FlagGuard aNotificationGuard( bAPIActionRunning );
            rUndoManager.EnterListAction( i_title, ::rtl::OUString() );
        }

        aContextVisibilities.push( i_hidden );

        rAntiImpl.impl_notify( i_title, i_hidden ? &XUndoManagerListener::enteredHiddenContext : &XUndoManagerListener::enteredContext, aGuard );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::actionUndone( SfxUndoAction& i_action )
    {
        if ( bAPIActionRunning )
            return;

        rAntiImpl.impl_notify( i_action.GetComment(), &XUndoManagerListener::actionUndone );
    }

     //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::actionRedone( SfxUndoAction& i_action )
    {
        if ( bAPIActionRunning )
            return;

        rAntiImpl.impl_notify( i_action.GetComment(), &XUndoManagerListener::actionRedone );
    }

     //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::undoActionAdded( SfxUndoAction& i_action )
    {
        if ( bAPIActionRunning )
            return;

        rAntiImpl.impl_notify( i_action.GetComment(), &XUndoManagerListener::undoActionAdded );
    }

     //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::cleared()
    {
        if ( bAPIActionRunning )
            return;

        rAntiImpl.impl_notify( &XUndoManagerListener::allActionsCleared );
    }

     //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::clearedRedo()
    {
        if ( bAPIActionRunning )
            return;

        rAntiImpl.impl_notify( &XUndoManagerListener::redoActionsCleared );
    }

     //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::listActionEntered( const String& i_comment )
    {
#if OSL_DEBUG_LEVEL > 0
        aContextAPIFlags.push( bAPIActionRunning );
#endif

        aContextTitles.push( i_comment );

        if ( bAPIActionRunning )
            return;

        rAntiImpl.impl_notify( i_comment, &XUndoManagerListener::enteredContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::listActionLeft()
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = aContextAPIFlags.top();
        aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == bAPIActionRunning, "DocumentUndoManager_Impl::listActionLeft: API and non-API contexts interwoven!" );
#endif

        if ( bAPIActionRunning )
            return;

        rAntiImpl.impl_notify( pUndoManager->GetUndoActionComment( 0, IUndoManager::CurrentLevel ), &XUndoManagerListener::leftContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::listActionLeftAndMerged()
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = aContextAPIFlags.top();
        aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == bAPIActionRunning, "DocumentUndoManager_Impl::listActionLeftAndMerged: API and non-API contexts interwoven!" );
#endif

        aContextTitles.pop();

        if ( bAPIActionRunning )
            return;

        rAntiImpl.impl_notify( ::rtl::OUString(), &XUndoManagerListener::leftHiddenContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::listActionCancelled()
    {
#if OSL_DEBUG_LEVEL > 0
        const bool bCurrentContextIsAPIContext = aContextAPIFlags.top();
        aContextAPIFlags.pop();
        OSL_ENSURE( bCurrentContextIsAPIContext == bAPIActionRunning, "DocumentUndoManager_Impl::listActionCancelled: API and non-API contexts interwoven!" );
#endif

        if ( bAPIActionRunning )
            return;

        rAntiImpl.impl_notify( ::rtl::OUString(), &XUndoManagerListener::cancelledContext );
    }

     //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::undoManagerDying()
    {
        pUndoManager = NULL;
    }

    //==================================================================================================================
    //= DocumentUndoManager
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DocumentUndoManager::DocumentUndoManager( SfxBaseModel& i_document )
        :SfxModelSubComponent( i_document )
        ,m_pImpl( new DocumentUndoManager_Impl( *this ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    DocumentUndoManager::~DocumentUndoManager()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager::disposing()
    {
        EventObject aEvent;
        aEvent.Source = static_cast< XUndoManager* >( this );
        m_pImpl->aUndoListeners.disposeAndClear( aEvent );

        m_pImpl->disposing();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::acquire(  ) throw ()
    {
        SfxModelSubComponent::acquire();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::release(  ) throw ()
    {
        SfxModelSubComponent::release();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager::impl_notify( ::rtl::OUString const& i_title, void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ),
        SfxModelGuard& i_instanceLock )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = static_cast< XUndoManager* >( this );
        aEvent.UndoActionTitle = i_title;
        aEvent.UndoContextDepth = m_pImpl->getUndoManager().GetListActionDepth();

        i_instanceLock.clear();
        m_pImpl->aUndoListeners.notifyEach( i_notificationMethod, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager::impl_notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ),
        SfxModelGuard& i_instanceLock )
    {
        EventObject aEvent;
        aEvent.Source = static_cast< XUndoManager* >( this );
        i_instanceLock.clear();
        m_pImpl->aUndoListeners.notifyEach( i_notificationMethod, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager::impl_notify( ::rtl::OUString const& i_title, void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ) )
    {
        UndoManagerEvent aEvent;
        aEvent.Source = static_cast< XUndoManager* >( this );
        aEvent.UndoActionTitle = i_title;
        aEvent.UndoContextDepth = m_pImpl->getUndoManager().GetListActionDepth();

        // TODO: this notification method here is used by DocumentUndoManager_Impl, to multiplex the notifications we
        // receive from the IUndoManager. Those notitications are sent with a locked SolarMutex, which means
        // we're doing the multiplexing here with a locked SM, too. Which is Bad (TM).
        // Fixing this properly would require outsourcing all the notifications into an own thread - which might lead
        // to problems of its own, since clients might expect synchronous notifications.

        m_pImpl->aUndoListeners.notifyEach( i_notificationMethod, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager::impl_notify( void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const EventObject& ) )
    {
        EventObject aEvent;
        aEvent.Source = static_cast< XUndoManager* >( this );

        // TODO: the same comment as in the other impl_notify, regarding SM locking applies here ...

        m_pImpl->aUndoListeners.notifyEach( i_notificationMethod, aEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::enterUndoContext( const ::rtl::OUString& i_title ) throw (RuntimeException)
    {
        m_pImpl->enterUndoContext( i_title, false );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::enterHiddenUndoContext(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        m_pImpl->enterUndoContext( ::rtl::OUString(), true );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::leaveUndoContext(  ) throw (InvalidStateException, RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( !rUndoManager.IsUndoEnabled() )
            // ignore this request if the manager is locked
            return;

        if ( !rUndoManager.IsInListAction() )
            throw InvalidStateException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no active undo context" ) ),
                static_cast< XUndoManager* >( this )
            );

        USHORT nContextElements = 0;
        bool isHiddenContext = false;
        const ::rtl::OUString sContextTitle = m_pImpl->getTopContextTitle();
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
            impl_notify( ::rtl::OUString(), &XUndoManagerListener::cancelledContext, aGuard );
        else if ( isHiddenContext )
            impl_notify( ::rtl::OUString(), &XUndoManagerListener::leftHiddenContext, aGuard );
        else
            impl_notify( rUndoManager.GetUndoActionComment( 0, IUndoManager::CurrentLevel ), &XUndoManagerListener::leftContext, aGuard );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::addUndoAction( const Reference< XUndoAction >& i_action ) throw (RuntimeException, IllegalArgumentException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        if ( !i_action.is() )
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal undo action object" ) ),
                static_cast< XUndoManager* >( this ),
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

        lcl_invalidateXDo( *m_pImpl );
        impl_notify( i_action->getTitle(), &XUndoManagerListener::undoActionAdded, aGuard );
        // <--- SYNCHRONIZED

        if ( bHadRedoActions && !bHasRedoActions )
            impl_notify( &XUndoManagerListener::redoActionsCleared );
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager::impl_do_nolck(
        USHORT ( IUndoManager::*i_checkMethod )( bool const ) const, BOOL ( IUndoManager::*i_doMethod )(),
        String ( IUndoManager::*i_titleRetriever )( USHORT, bool const ) const,
        void ( SAL_CALL XUndoManagerListener::*i_notificationMethod )( const UndoManagerEvent& ) )
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( ::rtl::OUString(), static_cast< XUndoManager* >( this ) );

        // let all views enter the standard mode
        // TODO: not sure this is a good idea: This might add another action to the Undo/Redo stack, which
        // will render the call somewhat meaningless - finally, the caller can't be sure that really the action
        // is undone/redone which s/he intended to.
        SfxObjectShell* pDocShell = m_pImpl->getObjectShell();
        OSL_ENSURE( pDocShell, "DocumentUndoManager::impl_do_nolck: do doc shell!" );
        if ( pDocShell )
        {
            SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( pDocShell );
            while ( pViewFrame )
            {
                SfxViewShell* pViewShell = pViewFrame->GetViewShell();
                ENSURE_OR_CONTINUE( pViewShell, "DocumentUndoManager::impl_do_nolck: no view shell in the frame!" );
                pViewShell->EnterStandardMode();
                pViewFrame = SfxViewFrame::GetNext( *pViewFrame, pDocShell );
            }
        }

        if ( (rUndoManager.*i_checkMethod)( IUndoManager::TopLevel ) == 0 )
            throw EmptyUndoStackException( ::rtl::OUString::createFromAscii( "stack is empty" ), static_cast< XUndoManager* >( this ) );

        const ::rtl::OUString sUndoActionTitle = (rUndoManager.*i_titleRetriever)( 0, IUndoManager::TopLevel );
        {
            ::comphelper::FlagGuard aNotificationGuard( m_pImpl->bAPIActionRunning );
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
                throw UndoFailedException( ::rtl::OUString(), static_cast< XUndoManager* >( this ), aError );
            }
        }

        impl_notify( sUndoActionTitle, i_notificationMethod, aGuard );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::undo(  ) throw (EmptyUndoStackException, UndoContextNotClosedException, UndoFailedException, RuntimeException)
    {
        impl_do_nolck(
            &IUndoManager::GetUndoActionCount,
            &IUndoManager::Undo,
            &IUndoManager::GetUndoActionComment,
            &XUndoManagerListener::actionUndone
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::redo(  ) throw (EmptyUndoStackException, UndoContextNotClosedException, UndoFailedException, RuntimeException)
    {
        impl_do_nolck(
            &IUndoManager::GetRedoActionCount,
            &IUndoManager::Redo,
            &IUndoManager::GetRedoActionComment,
            &XUndoManagerListener::actionRedone
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL DocumentUndoManager::isUndoPossible(  ) throw (RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return sal_False;
        return rUndoManager.GetUndoActionCount( IUndoManager::TopLevel ) > 0;
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL DocumentUndoManager::isRedoPossible(  ) throw (RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            return sal_False;
        return rUndoManager.GetRedoActionCount( IUndoManager::TopLevel ) > 0;
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        //..............................................................................................................
        ::rtl::OUString lcl_getCurrentActionTitle( DocumentUndoManager_Impl& i_impl, const bool i_undo )
        {
            // SYNCHRONIZED --->
            SfxModelGuard aGuard( i_impl.rAntiImpl );

            const IUndoManager& rUndoManager = i_impl.getUndoManager();
            const USHORT nActionCount = i_undo
                                    ?   rUndoManager.GetUndoActionCount( IUndoManager::TopLevel )
                                    :   rUndoManager.GetRedoActionCount( IUndoManager::TopLevel );
            if ( nActionCount == 0 )
                throw EmptyUndoStackException(
                    i_undo ? ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no action on the undo stack" ) )
                           : ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no action on the redo stack" ) ),
                    static_cast< XUndoManager* >( &i_impl.rAntiImpl )
                );
            return  i_undo
                ?   rUndoManager.GetUndoActionComment( 0, IUndoManager::TopLevel )
                :   rUndoManager.GetRedoActionComment( 0, IUndoManager::TopLevel );
            // <--- SYNCHRONIZED
        }

        //..............................................................................................................
        Sequence< ::rtl::OUString > lcl_getAllActionTitles( DocumentUndoManager_Impl& i_impl, const bool i_undo )
        {
            // SYNCHRONIZED --->
            SfxModelGuard aGuard( i_impl.rAntiImpl );

            const IUndoManager& rUndoManager = i_impl.getUndoManager();
            const sal_Int32 nCount =    i_undo
                                    ?   rUndoManager.GetUndoActionCount( IUndoManager::TopLevel )
                                    :   rUndoManager.GetRedoActionCount( IUndoManager::TopLevel );

            Sequence< ::rtl::OUString > aTitles( nCount );
            for ( sal_Int32 i=0; i<nCount; ++i )
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
    ::rtl::OUString SAL_CALL DocumentUndoManager::getCurrentUndoActionTitle(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        return lcl_getCurrentActionTitle( *m_pImpl, true );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DocumentUndoManager::getCurrentRedoActionTitle(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        return lcl_getCurrentActionTitle( *m_pImpl, false );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DocumentUndoManager::getAllUndoActionTitles(  ) throw (RuntimeException)
    {
        return lcl_getAllActionTitles( *m_pImpl, true );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DocumentUndoManager::getAllRedoActionTitles(  ) throw (RuntimeException)
    {
        return lcl_getAllActionTitles( *m_pImpl, false );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::clear(  ) throw (UndoContextNotClosedException, RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( ::rtl::OUString(), static_cast< XUndoManager* >( this ) );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_pImpl->bAPIActionRunning );
            rUndoManager.Clear();
        }
        impl_notify( &XUndoManagerListener::allActionsCleared, aGuard );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::clearRedo(  ) throw (UndoContextNotClosedException, RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsInListAction() )
            throw UndoContextNotClosedException( ::rtl::OUString(), static_cast< XUndoManager* >( this ) );

        {
            ::comphelper::FlagGuard aNotificationGuard( m_pImpl->bAPIActionRunning );
            rUndoManager.ClearRedo();
        }
        impl_notify( &XUndoManagerListener::redoActionsCleared, aGuard );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::reset() throw (RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        {
            ::comphelper::FlagGuard aNotificationGuard( m_pImpl->bAPIActionRunning );
            while ( rUndoManager.IsInListAction() )
                rUndoManager.LeaveListAction();
            rUndoManager.Clear();
        }

        impl_notify( &XUndoManagerListener::resetAll, aGuard );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::lock(  ) throw (RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        rUndoManager.EnableUndo( false );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::unlock(  ) throw (RuntimeException, NotLockedException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        if ( rUndoManager.IsUndoEnabled() )
            throw NotLockedException( ::rtl::OUString::createFromAscii( "Undo manager is not locked" ), static_cast< XUndoManager* >( this ) );
        rUndoManager.EnableUndo( true );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL DocumentUndoManager::isLocked(  ) throw (RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );

        IUndoManager& rUndoManager = m_pImpl->getUndoManager();
        return !rUndoManager.IsUndoEnabled();
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener ) throw (RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        if ( i_listener.is() )
            m_pImpl->aUndoListeners.addInterface( i_listener );
        // <--- SYNCHRONIZED
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener ) throw (RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        if ( i_listener.is() )
            m_pImpl->aUndoListeners.removeInterface( i_listener );
        // <--- SYNCHRONIZED
    }

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................
