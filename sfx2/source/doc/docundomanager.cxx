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


#include "docundomanager.hxx"
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <comphelper/anytostring.hxx>
#include <comphelper/flagguard.hxx>
#include <svl/undo.hxx>
#include <tools/diagnose_ex.h>
#include <framework/undomanagerhelper.hxx>

#include <stack>


namespace sfx2
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::document::XUndoAction;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::NotInitializedException;
    using ::com::sun::star::document::XUndoManagerListener;
    using ::com::sun::star::document::XUndoManager;
    using ::com::sun::star::lang::NoSupportException;
    using ::com::sun::star::frame::XModel;

    using ::svl::IUndoManager;


    //= DocumentUndoManager_Impl

    struct DocumentUndoManager_Impl : public ::framework::IUndoManagerImplementation
    {
        DocumentUndoManager&                rAntiImpl;
        IUndoManager*                       pUndoManager;
        ::framework::UndoManagerHelper      aUndoHelper;

        explicit DocumentUndoManager_Impl( DocumentUndoManager& i_antiImpl )
            :rAntiImpl( i_antiImpl )
            ,pUndoManager( impl_retrieveUndoManager( i_antiImpl.getBaseModel() ) )
                // do this *before* the construction of aUndoHelper (which actually means: put pUndoManager before
                // aUndoHelper in the member list)!
            ,aUndoHelper( *this )
        {
        }

        virtual ~DocumentUndoManager_Impl()
        {
        };

        // IUndoManagerImplementation
        virtual ::svl::IUndoManager&        getImplUndoManager() override;
        virtual Reference< XUndoManager >   getThis() override;

        void disposing()
        {
            aUndoHelper.disposing();
            ENSURE_OR_RETURN_VOID( pUndoManager, "DocumentUndoManager_Impl::disposing: already disposed!" );
            pUndoManager = nullptr;
        }

        void invalidateXDo_nolck();

    private:
        static IUndoManager* impl_retrieveUndoManager( SfxBaseModel& i_baseModel )
        {
            IUndoManager* pUndoManager( nullptr );
            SfxObjectShell* pObjectShell = i_baseModel.GetObjectShell();
            if ( pObjectShell != nullptr )
                pUndoManager = pObjectShell->GetUndoManager();
            if ( !pUndoManager )
                throw NotInitializedException( OUString(), *&i_baseModel );
            return pUndoManager;
        }
    };


    ::svl::IUndoManager& DocumentUndoManager_Impl::getImplUndoManager()
    {
        ENSURE_OR_THROW( pUndoManager != nullptr, "DocumentUndoManager_Impl::getImplUndoManager: no access to the doc's UndoManager implementation!" );

#if OSL_DEBUG_LEVEL > 0
        // in a non-product build, assert if the current UndoManager at the shell is not the same we obtained
        // (and cached) at construction time
        SfxObjectShell* pObjectShell = rAntiImpl.getBaseModel().GetObjectShell();
        OSL_ENSURE( ( pObjectShell != nullptr ) && ( pUndoManager == pObjectShell->GetUndoManager() ),
            "DocumentUndoManager_Impl::getImplUndoManager: the UndoManager changed meanwhile - what about our listener?" );
#endif

        return *pUndoManager;
    }


    Reference< XUndoManager > DocumentUndoManager_Impl::getThis()
    {
        return static_cast< XUndoManager* >( &rAntiImpl );
    }


    void DocumentUndoManager_Impl::invalidateXDo_nolck()
    {
        SfxModelGuard aGuard( rAntiImpl );

        const SfxObjectShell* pDocShell = rAntiImpl.getBaseModel().GetObjectShell();
        ENSURE_OR_THROW( pDocShell != nullptr, "lcl_invalidateUndo: no access to the doc shell!" );
        SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( pDocShell );
        while ( pViewFrame )
        {
            pViewFrame->GetBindings().Invalidate( SID_UNDO );
            pViewFrame->GetBindings().Invalidate( SID_REDO );
            pViewFrame = SfxViewFrame::GetNext( *pViewFrame, pDocShell );
        }
    }


    //= SolarMutexFacade

    /** a facade for the SolarMutex, implementing ::framework::IMutex
    */
    class SolarMutexFacade : public ::framework::IMutex
    {
    public:
        SolarMutexFacade()
        {
        }

        virtual ~SolarMutexFacade() {}

        virtual void acquire() override
        {
            Application::GetSolarMutex().acquire();
        }

        virtual void release() override
        {
            Application::GetSolarMutex().release();
        }
    };


    //= UndoManagerGuard

    class UndoManagerGuard  :public ::framework::IMutexGuard
    {
    public:
        explicit UndoManagerGuard( DocumentUndoManager& i_undoManager )
            :m_guard( i_undoManager )
            ,m_solarMutexFacade()
        {
        }

        virtual ~UndoManagerGuard()
        {
        }

        UndoManagerGuard(const UndoManagerGuard&) = delete;
        UndoManagerGuard& operator=(const UndoManagerGuard&) = delete;

        virtual void clear() override
        {
            m_guard.clear();
        }

        virtual ::framework::IMutex& getGuardedMutex() override
        {
            // note that this means that we *know* that SfxModelGuard also locks the SolarMutex (nothing more, nothing less).
            // If this ever changes, we need to adjust this code here, too.
            return m_solarMutexFacade;
        }

    private:
        SfxModelGuard       m_guard;
        SolarMutexFacade    m_solarMutexFacade;
    };


    //= DocumentUndoManager


    DocumentUndoManager::DocumentUndoManager( SfxBaseModel& i_document )
        :SfxModelSubComponent( i_document )
        ,m_pImpl( new DocumentUndoManager_Impl( *this ) )
    {
    }

    DocumentUndoManager::~DocumentUndoManager()
    {
    }

    void DocumentUndoManager::disposing()
    {
        m_pImpl->disposing();
    }


    bool DocumentUndoManager::isInContext() const
    {
        // No mutex locking within this method, no disposal check - this is the responsibility of the owner.
        return m_pImpl->getImplUndoManager().IsInListAction();
    }


    void SAL_CALL DocumentUndoManager::acquire() throw()
    {
        OWeakObject::acquire();
        SfxModelSubComponent::acquireModel();
    }


    void SAL_CALL DocumentUndoManager::release() throw()
    {
        SfxModelSubComponent::releaseModel();
        OWeakObject::release();
    }


    void SAL_CALL DocumentUndoManager::enterUndoContext( const OUString& i_title )
    {
        // SYNCHRONIZED --->
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.enterUndoContext( i_title, aGuard );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }


    void SAL_CALL DocumentUndoManager::enterHiddenUndoContext(  )
    {
        // SYNCHRONIZED --->
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.enterHiddenUndoContext( aGuard );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }


    void SAL_CALL DocumentUndoManager::leaveUndoContext(  )
    {
        // SYNCHRONIZED --->
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.leaveUndoContext( aGuard );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }


    void SAL_CALL DocumentUndoManager::addUndoAction( const Reference< XUndoAction >& i_action )
    {
        // SYNCHRONIZED --->
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.addUndoAction( i_action, aGuard );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }


    void SAL_CALL DocumentUndoManager::undo(  )
    {
        // SYNCHRONIZED --->
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.undo( aGuard );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }


    void SAL_CALL DocumentUndoManager::redo(  )
    {
        // SYNCHRONIZED --->
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.redo( aGuard );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }


    sal_Bool SAL_CALL DocumentUndoManager::isUndoPossible(  )
    {
        UndoManagerGuard aGuard( *this );
        return m_pImpl->aUndoHelper.isUndoPossible();
    }


    sal_Bool SAL_CALL DocumentUndoManager::isRedoPossible(  )
    {
        UndoManagerGuard aGuard( *this );
        return m_pImpl->aUndoHelper.isRedoPossible();
    }


    OUString SAL_CALL DocumentUndoManager::getCurrentUndoActionTitle(  )
    {
        UndoManagerGuard aGuard( *this );
        return m_pImpl->aUndoHelper.getCurrentUndoActionTitle();
    }


    OUString SAL_CALL DocumentUndoManager::getCurrentRedoActionTitle(  )
    {
        UndoManagerGuard aGuard( *this );
        return m_pImpl->aUndoHelper.getCurrentRedoActionTitle();
    }


    Sequence< OUString > SAL_CALL DocumentUndoManager::getAllUndoActionTitles(  )
    {
        UndoManagerGuard aGuard( *this );
        return m_pImpl->aUndoHelper.getAllUndoActionTitles();
    }


    Sequence< OUString > SAL_CALL DocumentUndoManager::getAllRedoActionTitles(  )
    {
        UndoManagerGuard aGuard( *this );
        return m_pImpl->aUndoHelper.getAllRedoActionTitles();
    }


    void SAL_CALL DocumentUndoManager::clear(  )
    {
        // SYNCHRONIZED --->
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.clear( aGuard );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }


    void SAL_CALL DocumentUndoManager::clearRedo(  )
    {
        // SYNCHRONIZED --->
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.clearRedo( aGuard );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }


    void SAL_CALL DocumentUndoManager::reset()
    {
        // SYNCHRONIZED --->
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.reset( aGuard );
        // <--- SYNCHRONIZED
        m_pImpl->invalidateXDo_nolck();
    }


    void SAL_CALL DocumentUndoManager::lock(  )
    {
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.lock();
    }


    void SAL_CALL DocumentUndoManager::unlock(  )
    {
        UndoManagerGuard aGuard( *this );
        m_pImpl->aUndoHelper.unlock();
    }


    sal_Bool SAL_CALL DocumentUndoManager::isLocked(  )
    {
        UndoManagerGuard aGuard( *this );
        return m_pImpl->aUndoHelper.isLocked();
    }


    void SAL_CALL DocumentUndoManager::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        UndoManagerGuard aGuard( *this );
        return m_pImpl->aUndoHelper.addUndoManagerListener( i_listener );
    }


    void SAL_CALL DocumentUndoManager::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        UndoManagerGuard aGuard( *this );
        return m_pImpl->aUndoHelper.removeUndoManagerListener( i_listener );
    }


    Reference< XInterface > SAL_CALL DocumentUndoManager::getParent(  )
    {
        UndoManagerGuard aGuard( *this );
        return static_cast< XModel* >( &getBaseModel() );
    }


    void SAL_CALL DocumentUndoManager::setParent( const Reference< XInterface >& i_parent )
    {
        (void)i_parent;
        throw NoSupportException( OUString(), m_pImpl->getThis() );
    }


} // namespace sfx2


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
