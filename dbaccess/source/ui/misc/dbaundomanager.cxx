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

namespace dbaui
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
        virtual ::svl::IUndoManager&        getImplUndoManager();
        virtual Reference< XUndoManager >   getThis();
    };

    ::svl::IUndoManager& UndoManager_Impl::getImplUndoManager()
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
        OslMutexFacade( ::osl::Mutex& i_mutex )
            :m_rMutex( i_mutex )
        {
        }

        virtual ~OslMutexFacade() {}

        virtual void acquire();
        virtual void release();

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
        UndoManagerMethodGuard( UndoManager_Impl& i_impl )
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
        virtual ::framework::IMutex& getGuardedMutex();

        // IGuard
        virtual void clear();
        virtual void reset();

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

    void UndoManagerMethodGuard::reset()
    {
        m_aGuard.reset();
    }

    // UndoManager
    UndoManager::UndoManager( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex )
        :m_pImpl( new UndoManager_Impl( *this, i_parent, i_mutex ) )
    {
    }

    UndoManager::~UndoManager()
    {
    }

    SfxUndoManager& UndoManager::GetSfxUndoManager() const
    {
        return m_pImpl->aUndoManager;
    }

    void SAL_CALL UndoManager::acquire(  ) throw ()
    {
        m_pImpl->rParent.acquire();
    }

    void SAL_CALL UndoManager::release(  ) throw ()
    {
        m_pImpl->rParent.release();
    }

    void UndoManager::disposing()
    {
        {
            ::osl::MutexGuard aGuard( m_pImpl->rMutex );
            m_pImpl->bDisposed = true;
        }
        m_pImpl->aUndoHelper.disposing();
    }

    void SAL_CALL UndoManager::enterUndoContext( const OUString& i_title ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.enterUndoContext( i_title, aGuard );
    }

    void SAL_CALL UndoManager::enterHiddenUndoContext(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.enterHiddenUndoContext( aGuard );
    }

    void SAL_CALL UndoManager::leaveUndoContext(  ) throw (InvalidStateException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.leaveUndoContext( aGuard );
    }

    void SAL_CALL UndoManager::addUndoAction( const Reference< XUndoAction >& i_action ) throw (IllegalArgumentException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.addUndoAction( i_action, aGuard );
    }

    void SAL_CALL UndoManager::undo(  ) throw (EmptyUndoStackException, UndoContextNotClosedException, UndoFailedException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
            // (all our UndoActions work directly on VCL code, usually, so ...)
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.undo( aGuard );
    }

    void SAL_CALL UndoManager::redo(  ) throw (EmptyUndoStackException, UndoContextNotClosedException, UndoFailedException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
            // (all our UndoActions work directly on VCL code, usually, so ...)
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.redo( aGuard );
    }

    ::sal_Bool SAL_CALL UndoManager::isUndoPossible(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->aUndoHelper.isUndoPossible();
    }

    ::sal_Bool SAL_CALL UndoManager::isRedoPossible(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->aUndoHelper.isRedoPossible();
    }

    OUString SAL_CALL UndoManager::getCurrentUndoActionTitle(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->aUndoHelper.getCurrentUndoActionTitle();
    }

    OUString SAL_CALL UndoManager::getCurrentRedoActionTitle(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->aUndoHelper.getCurrentRedoActionTitle();
    }

    Sequence< OUString > SAL_CALL UndoManager::getAllUndoActionTitles(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->aUndoHelper.getAllUndoActionTitles();
    }

    Sequence< OUString > SAL_CALL UndoManager::getAllRedoActionTitles(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->aUndoHelper.getAllRedoActionTitles();
    }

    void SAL_CALL UndoManager::clear(  ) throw (UndoContextNotClosedException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.clear( aGuard );
    }

    void SAL_CALL UndoManager::clearRedo(  ) throw (UndoContextNotClosedException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.clearRedo( aGuard );
    }

    void SAL_CALL UndoManager::reset(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.reset( aGuard );
    }

    void SAL_CALL UndoManager::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.addUndoManagerListener( i_listener );
    }

    void SAL_CALL UndoManager::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.removeUndoManagerListener( i_listener );
    }

    void SAL_CALL UndoManager::lock(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.lock();
    }

    void SAL_CALL UndoManager::unlock(  ) throw (NotLockedException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->aUndoHelper.unlock();
    }

    ::sal_Bool SAL_CALL UndoManager::isLocked(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->aUndoHelper.isLocked();
    }

    Reference< XInterface > SAL_CALL UndoManager::getParent(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return *&m_pImpl->rParent;
    }

    void SAL_CALL UndoManager::setParent( const Reference< XInterface >& i_parent ) throw (NoSupportException, RuntimeException)
    {
        (void)i_parent;
        throw NoSupportException( OUString(), m_pImpl->getThis() );
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
