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
#include <framework/undomanagerhelper.hxx>

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
    //= DocumentUndoManager_Impl
    //==================================================================================================================
    struct DocumentUndoManager_Impl : public ::framework::IUndoManagerImplementation
    {
        DocumentUndoManager&                rAntiImpl;
        IUndoManager*                       pUndoManager;
        ::framework::UndoManagerHelper      aUndoHelper;

        DocumentUndoManager_Impl( DocumentUndoManager& i_antiImpl )
            :rAntiImpl( i_antiImpl )
            ,pUndoManager( impl_retrieveUndoManager( i_antiImpl.getBaseModel() ) )
                // do this *before* the construction of aUndoHelper (which actually means: put pUndoManager before
                // aUndoHelper in the member list)!
            ,aUndoHelper( *this )
        {
        }

        const SfxObjectShell* getObjectShell() const { return rAntiImpl.getBaseModel().GetObjectShell(); }
              SfxObjectShell* getObjectShell()       { return rAntiImpl.getBaseModel().GetObjectShell(); }

        // IUndoManagerImplementation
        virtual ::osl::Mutex&               getMutex();
        virtual ::svl::IUndoManager&        getImplUndoManager();
        virtual Reference< XUndoManager >   getThis();

        void disposing()
        {
            aUndoHelper.disposing();
            ENSURE_OR_RETURN_VOID( pUndoManager, "DocumentUndoManager_Impl::disposing: already disposed!" );
            pUndoManager = NULL;
        }

        void invalidateXDo_nolck();
        void enterViewStandardMode();

    private:
        static IUndoManager* impl_retrieveUndoManager( SfxBaseModel& i_baseModel )
        {
            IUndoManager* pUndoManager( NULL );
            SfxObjectShell* pObjectShell = i_baseModel.GetObjectShell();
            if ( pObjectShell != NULL )
                pUndoManager = pObjectShell->GetUndoManager();
            if ( !pUndoManager )
                throw NotInitializedException( ::rtl::OUString(), *&i_baseModel );
            return pUndoManager;
        }
    };

    //------------------------------------------------------------------------------------------------------------------
    ::osl::Mutex& DocumentUndoManager_Impl::getMutex()
    {
        return rAntiImpl.getMutex();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::svl::IUndoManager& DocumentUndoManager_Impl::getImplUndoManager()
    {
        ENSURE_OR_THROW( pUndoManager != NULL, "DocumentUndoManager_Impl::getImplUndoManager: no access to the doc's UndoManager implementation!" );

#if OSL_DEBUG_LEVEL > 0
        // in a non-product build, assert if the current UndoManager at the shell is not the same we obtained
        // (and cached) at construction time
        SfxObjectShell* pObjectShell = rAntiImpl.getBaseModel().GetObjectShell();
        OSL_ENSURE( ( pObjectShell != NULL ) && ( pUndoManager == pObjectShell->GetUndoManager() ),
            "DocumentUndoManager_Impl::getImplUndoManager: the UndoManager changed meanwhile - what about our listener?" );
#endif

        return *pUndoManager;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XUndoManager > DocumentUndoManager_Impl::getThis()
    {
        return static_cast< XUndoManager* >( &rAntiImpl );
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::invalidateXDo_nolck()
    {
        SfxModelGuard aGuard( rAntiImpl );

        const SfxObjectShell* pDocShell = getObjectShell();
        ENSURE_OR_THROW( pDocShell != NULL, "lcl_invalidateUndo: no access to the doc shell!" );
        SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( pDocShell );
        while ( pViewFrame )
        {
            pViewFrame->GetBindings().Invalidate( SID_UNDO );
            pViewFrame->GetBindings().Invalidate( SID_REDO );
            pViewFrame = SfxViewFrame::GetNext( *pViewFrame, pDocShell );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager_Impl::enterViewStandardMode()
    {
        // TODO: not sure this is a good idea: This might add another action to the Undo/Redo stack, which
        // will render the current call somewhat meaningless - finally, the caller can't be sure that really the action
        // is undone/redone which s/he intended to.
        SfxObjectShell* pDocShell = getObjectShell();
        ENSURE_OR_RETURN_VOID( pDocShell, "DocumentUndoManager_Impl::enterViewStandardMode: do doc shell!" );
        SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( pDocShell );
        while ( pViewFrame )
        {
            SfxViewShell* pViewShell = pViewFrame->GetViewShell();
            ENSURE_OR_CONTINUE( pViewShell, "DocumentUndoManager_Impl::enterViewStandardMode: no view shell in the frame!" );
            pViewShell->EnterStandardMode();
            pViewFrame = SfxViewFrame::GetNext( *pViewFrame, pDocShell );
        }
    }

    //==================================================================================================================
    //= SfxModelGuardFacade
    //==================================================================================================================
    class SfxModelGuardFacade : public ::framework::IClearableInstanceLock
    {
    public:
        SfxModelGuardFacade( SfxModelGuard& i_guard )
            :m_guard( i_guard )
        {
        }

        virtual void clear()
        {
            m_guard.clear();
        }

    private:
        SfxModelGuard&  m_guard;
    };

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
    void SAL_CALL DocumentUndoManager::enterUndoContext( const ::rtl::OUString& i_title ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        m_pImpl->aUndoHelper.enterUndoContext( i_title, SfxModelGuardFacade( aGuard ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::enterHiddenUndoContext(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        m_pImpl->aUndoHelper.enterHiddenUndoContext( SfxModelGuardFacade( aGuard ) );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::leaveUndoContext(  ) throw (InvalidStateException, RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        m_pImpl->aUndoHelper.leaveUndoContext( SfxModelGuardFacade( aGuard ) );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::addUndoAction( const Reference< XUndoAction >& i_action ) throw (RuntimeException, IllegalArgumentException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        m_pImpl->aUndoHelper.addUndoAction( i_action, SfxModelGuardFacade( aGuard ) );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::undo(  ) throw (EmptyUndoStackException, UndoContextNotClosedException, UndoFailedException, RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        m_pImpl->enterViewStandardMode();
        m_pImpl->aUndoHelper.undo( SfxModelGuardFacade( aGuard ) );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::redo(  ) throw (EmptyUndoStackException, UndoContextNotClosedException, UndoFailedException, RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        m_pImpl->enterViewStandardMode();
        m_pImpl->aUndoHelper.redo( SfxModelGuardFacade( aGuard ) );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL DocumentUndoManager::isUndoPossible(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        return m_pImpl->aUndoHelper.isUndoPossible();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL DocumentUndoManager::isRedoPossible(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        return m_pImpl->aUndoHelper.isRedoPossible();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DocumentUndoManager::getCurrentUndoActionTitle(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        return m_pImpl->aUndoHelper.getCurrentUndoActionTitle();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DocumentUndoManager::getCurrentRedoActionTitle(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        return m_pImpl->aUndoHelper.getCurrentRedoActionTitle();
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DocumentUndoManager::getAllUndoActionTitles(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        return m_pImpl->aUndoHelper.getAllUndoActionTitles();
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DocumentUndoManager::getAllRedoActionTitles(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        return m_pImpl->aUndoHelper.getAllRedoActionTitles();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::clear(  ) throw (UndoContextNotClosedException, RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        m_pImpl->aUndoHelper.clear( SfxModelGuardFacade( aGuard ) );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::clearRedo(  ) throw (UndoContextNotClosedException, RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        m_pImpl->aUndoHelper.clearRedo( SfxModelGuardFacade( aGuard ) );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::reset() throw (RuntimeException)
    {
        // SYNCHRONIZED --->
        SfxModelGuard aGuard( *this );
        m_pImpl->aUndoHelper.reset( SfxModelGuardFacade( aGuard ) );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::lock(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        m_pImpl->aUndoHelper.lock();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::unlock(  ) throw (RuntimeException, NotLockedException)
    {
        SfxModelGuard aGuard( *this );
        m_pImpl->aUndoHelper.unlock();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL DocumentUndoManager::isLocked(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        return m_pImpl->aUndoHelper.isLocked();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        return m_pImpl->aUndoHelper.addUndoManagerListener( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        return m_pImpl->aUndoHelper.removeUndoManagerListener( i_listener );
    }

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................
