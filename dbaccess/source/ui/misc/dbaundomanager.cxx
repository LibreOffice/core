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

#include <dbaccess/dbaundomanager.hxx>

#include <com/sun/star/lang/DisposedException.hpp>

#include <svl/undo.hxx>
#include <vcl/svapp.hxx>
#include <framework/undomanagerhelper.hxx>
#include <framework/imutex.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::document::XUndoManager;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::document::UndoContextNotClosedException;
    using ::com::sun::star::document::UndoFailedException;
    using ::com::sun::star::document::EmptyUndoStackException;
    using ::com::sun::star::util::InvalidStateException;
    using ::com::sun::star::document::XUndoAction;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::document::XUndoManagerListener;
    using ::com::sun::star::util::NotLockedException;
    using ::com::sun::star::lang::NoSupportException;

    // UndoManager_Impl
    struct UndoManager_Impl : public ::framework::IUndoManagerImplementation
    {
        UndoManager_Impl( UndoManager& i_antiImpl, ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex )
            :rAntiImpl( i_antiImpl )
            ,rParent( i_parent )
            ,rMutex( i_mutex )
            ,bDisposed( false )
            ,aUndoManager()
            ,aUndoHelper( *this )
        {
        }

        virtual ~UndoManager_Impl()
        {
        }

        UndoManager&                    rAntiImpl;
        ::cppu::OWeakObject&            rParent;
        ::osl::Mutex&                   rMutex;
        bool                            bDisposed;
        SfxUndoManager                  aUndoManager;
        ::framework::UndoManagerHelper  aUndoHelper;

        // IUndoManagerImplementation
        virtual SfxUndoManager&             getImplUndoManager() override;
        virtual Reference< XUndoManager >   getThis() override;
    };

    SfxUndoManager& UndoManager_Impl::getImplUndoManager()
    {
        return aUndoManager;
    }

    Reference< XUndoManager > UndoManager_Impl::getThis()
    {
        return static_cast< XUndoManager* >( &rAntiImpl );
    }

    // OslMutexFacade
    class OslMutexFacade : public ::framework::IMutex
    {
    public:
        explicit OslMutexFacade( ::osl::Mutex& i_mutex )
            :m_rMutex( i_mutex )
        {
        }

        virtual ~OslMutexFacade() {}

        virtual void acquire() override;
        virtual void release() override;

    private:
        ::osl::Mutex&   m_rMutex;
    };

    void OslMutexFacade::acquire()
    {
        m_rMutex.acquire();
    }

    void OslMutexFacade::release()
    {
        m_rMutex.release();
    }

    // UndoManagerMethodGuard
    /** guard for public UNO methods of the UndoManager
    */
    class UndoManagerMethodGuard : public ::framework::IMutexGuard
    {
    public:
        explicit UndoManagerMethodGuard( UndoManager_Impl& i_impl )
            :m_aGuard( i_impl.rMutex )
            ,m_aMutexFacade( i_impl.rMutex )
        {
            // throw if the instance is already disposed
            if ( i_impl.bDisposed )
                throw DisposedException( OUString(), i_impl.getThis() );
        }
        virtual ~UndoManagerMethodGuard()
        {
        }

        // IMutexGuard
        virtual void clear() override;
        virtual ::framework::IMutex& getGuardedMutex() override;

    private:
        ::osl::ResettableMutexGuard m_aGuard;
        OslMutexFacade              m_aMutexFacade;
    };

    ::framework::IMutex& UndoManagerMethodGuard::getGuardedMutex()
    {
        return m_aMutexFacade;
    }

    void UndoManagerMethodGuard::clear()
    {
        m_aGuard.clear();
    }

    // UndoManager
    UndoManager::UndoManager( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex )
        :m_xImpl( new UndoManager_Impl( *this, i_parent, i_mutex ) )
    {
    }

    UndoManager::~UndoManager()
    {
    }

    SfxUndoManager& UndoManager::GetSfxUndoManager() const
    {
        return m_xImpl->aUndoManager;
    }

    void SAL_CALL UndoManager::acquire(  ) throw ()
    {
        m_xImpl->rParent.acquire();
    }

    void SAL_CALL UndoManager::release(  ) throw ()
    {
        m_xImpl->rParent.release();
    }

    void UndoManager::disposing()
    {
        {
            ::osl::MutexGuard aGuard( m_xImpl->rMutex );
            m_xImpl->bDisposed = true;
        }
        m_xImpl->aUndoHelper.disposing();
    }

    void SAL_CALL UndoManager::enterUndoContext( const OUString& i_title )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.enterUndoContext( i_title, aGuard );
    }

    void SAL_CALL UndoManager::enterHiddenUndoContext(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.enterHiddenUndoContext( aGuard );
    }

    void SAL_CALL UndoManager::leaveUndoContext(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.leaveUndoContext( aGuard );
    }

    void SAL_CALL UndoManager::addUndoAction( const Reference< XUndoAction >& i_action )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.addUndoAction( i_action, aGuard );
    }

    void SAL_CALL UndoManager::undo(  )
    {
        SolarMutexGuard aSolarGuard;
            // (all our UndoActions work directly on VCL code, usually, so ...)
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.undo( aGuard );
    }

    void SAL_CALL UndoManager::redo(  )
    {
        SolarMutexGuard aSolarGuard;
            // (all our UndoActions work directly on VCL code, usually, so ...)
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.redo( aGuard );
    }

    sal_Bool SAL_CALL UndoManager::isUndoPossible(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        return m_xImpl->aUndoHelper.isUndoPossible();
    }

    sal_Bool SAL_CALL UndoManager::isRedoPossible(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        return m_xImpl->aUndoHelper.isRedoPossible();
    }

    OUString SAL_CALL UndoManager::getCurrentUndoActionTitle(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        return m_xImpl->aUndoHelper.getCurrentUndoActionTitle();
    }

    OUString SAL_CALL UndoManager::getCurrentRedoActionTitle(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        return m_xImpl->aUndoHelper.getCurrentRedoActionTitle();
    }

    Sequence< OUString > SAL_CALL UndoManager::getAllUndoActionTitles(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        return m_xImpl->aUndoHelper.getAllUndoActionTitles();
    }

    Sequence< OUString > SAL_CALL UndoManager::getAllRedoActionTitles(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        return m_xImpl->aUndoHelper.getAllRedoActionTitles();
    }

    void SAL_CALL UndoManager::clear(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.clear( aGuard );
    }

    void SAL_CALL UndoManager::clearRedo(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.clearRedo( aGuard );
    }

    void SAL_CALL UndoManager::reset(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.reset( aGuard );
    }

    void SAL_CALL UndoManager::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.addUndoManagerListener( i_listener );
    }

    void SAL_CALL UndoManager::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.removeUndoManagerListener( i_listener );
    }

    void SAL_CALL UndoManager::lock(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.lock();
    }

    void SAL_CALL UndoManager::unlock(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        m_xImpl->aUndoHelper.unlock();
    }

    sal_Bool SAL_CALL UndoManager::isLocked(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        return m_xImpl->aUndoHelper.isLocked();
    }

    Reference< XInterface > SAL_CALL UndoManager::getParent(  )
    {
        UndoManagerMethodGuard aGuard( *m_xImpl );
        return m_xImpl->rParent;
    }

    void SAL_CALL UndoManager::setParent( const Reference< XInterface >& )
    {
        throw NoSupportException( OUString(), m_xImpl->getThis() );
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
