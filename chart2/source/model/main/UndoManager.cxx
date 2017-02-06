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

#include "UndoManager.hxx"
#include "ChartViewHelper.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>

#include <framework/undomanagerhelper.hxx>
#include <officecfg/Office/Common.hxx>
#include <svl/undo.hxx>

namespace chart
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::document::XUndoManager;
    using ::com::sun::star::document::XUndoAction;
    using ::com::sun::star::document::XUndoManagerListener;
    using ::com::sun::star::lang::NoSupportException;
    using ::com::sun::star::util::XModifyListener;
    using ::com::sun::star::frame::XModel;

    namespace impl
    {
        class UndoManager_Impl : public ::framework::IUndoManagerImplementation
        {
        public:
            UndoManager_Impl( UndoManager& i_antiImpl, ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex )
                :m_rAntiImpl( i_antiImpl )
                ,m_rParent( i_parent )
                ,m_rMutex( i_mutex )
                ,m_bDisposed( false )
                ,m_aUndoManager()
                ,m_aUndoHelper( *this )
            {
                m_aUndoManager.SetMaxUndoActionCount(
                    officecfg::Office::Common::Undo::Steps::get());
            }

            virtual ~UndoManager_Impl()
            {
            }

            ::osl::Mutex&                       getMutex();
            // IUndoManagerImplementation
            virtual ::svl::IUndoManager&        getImplUndoManager() override;
            virtual Reference< XUndoManager >   getThis() override;

            // attribute access
            ::cppu::OWeakObject&                getParent() { return m_rParent; }
            ::framework::UndoManagerHelper&     getUndoHelper() { return m_aUndoHelper; }

            // public interface

            /// is called when the owner of the UndoManager is being disposed
            void    disposing();

            /// checks whether we're already disposed, throws a DisposedException if so
            void    checkDisposed_lck();

        private:
            UndoManager&                        m_rAntiImpl;
            ::cppu::OWeakObject&                m_rParent;
            ::osl::Mutex&                       m_rMutex;
            bool                                m_bDisposed;

            SfxUndoManager                      m_aUndoManager;
            ::framework::UndoManagerHelper      m_aUndoHelper;
        };

        ::osl::Mutex& UndoManager_Impl::getMutex()
        {
            return m_rMutex;
        }

        ::svl::IUndoManager& UndoManager_Impl::getImplUndoManager()
        {
            return m_aUndoManager;
        }

        Reference< XUndoManager > UndoManager_Impl::getThis()
        {
            return &m_rAntiImpl;
        }

        void UndoManager_Impl::disposing()
        {
            {
                ::osl::MutexGuard aGuard( m_rMutex );
                m_bDisposed = true;
            }
            m_aUndoHelper.disposing();
        }

        void UndoManager_Impl::checkDisposed_lck()
        {
            if ( m_bDisposed )
                throw DisposedException( OUString(), getThis() );
        }

        /** guard for public UNO methods of the UndoManager

            The only purpose of this guard is to check for the instance being disposed already. Everything else,
            in particular the IMutexGuard functionality required by the UndoManagerHelper class, is a dummy only,
            as all involved classes (means we ourselves, the UndoManagerHelper, the SfxUndoManager, and the Undo actions
            we create) are inherently thread-safe, thus need no external lock (in particular no SolarMutex!).
        */
        class UndoManagerMethodGuard : public ::framework::IMutexGuard
        {
        public:
            explicit UndoManagerMethodGuard( UndoManager_Impl& i_impl )
            {
                ::osl::MutexGuard aGuard( i_impl.getMutex() );
                // throw if the instance is already disposed
                i_impl.checkDisposed_lck();
            }
            virtual ~UndoManagerMethodGuard()
            {
            }

            // IMutexGuard
            virtual void clear() override;
            virtual ::framework::IMutex& getGuardedMutex() override;
        };

        class DummyMutex : public ::framework::IMutex
        {
        public:
            virtual ~DummyMutex() {}
            virtual void acquire() override { }
            virtual void release() override { }
        };

        ::framework::IMutex& UndoManagerMethodGuard::getGuardedMutex()
        {
            static DummyMutex s_aDummyMutex;
            return s_aDummyMutex;
        }

        void UndoManagerMethodGuard::clear()
        {
            // nothing to do. This interface implementation is a dummy.
        }
    }

    using impl::UndoManagerMethodGuard;

    UndoManager::UndoManager( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex )
        :m_pImpl( new impl::UndoManager_Impl( *this, i_parent, i_mutex ) )
    {
    }

    UndoManager::~UndoManager()
    {
    }

    void SAL_CALL UndoManager::acquire() throw ()
    {
        m_pImpl->getParent().acquire();
    }

    void SAL_CALL UndoManager::release() throw ()
    {
        m_pImpl->getParent().release();
    }

    void UndoManager::disposing()
    {
        m_pImpl->disposing();
    }

    void SAL_CALL UndoManager::enterUndoContext( const OUString& i_title )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().enterUndoContext( i_title, aGuard );
    }

    void SAL_CALL UndoManager::enterHiddenUndoContext(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().enterHiddenUndoContext( aGuard );
    }

    void SAL_CALL UndoManager::leaveUndoContext(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().leaveUndoContext( aGuard );
    }

    void SAL_CALL UndoManager::addUndoAction( const Reference< XUndoAction >& i_action )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().addUndoAction( i_action, aGuard );
    }

    void SAL_CALL UndoManager::undo(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().undo( aGuard );

        ChartViewHelper::setViewToDirtyState( Reference< XModel >( getParent(), UNO_QUERY ) );
    }

    void SAL_CALL UndoManager::redo(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().redo( aGuard );

        ChartViewHelper::setViewToDirtyState( Reference< XModel >( getParent(), UNO_QUERY ) );
    }

    sal_Bool SAL_CALL UndoManager::isUndoPossible(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().isUndoPossible();
    }

    sal_Bool SAL_CALL UndoManager::isRedoPossible(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().isRedoPossible();
    }

    OUString SAL_CALL UndoManager::getCurrentUndoActionTitle(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().getCurrentUndoActionTitle();
    }

    OUString SAL_CALL UndoManager::getCurrentRedoActionTitle(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().getCurrentRedoActionTitle();
    }

    Sequence< OUString > SAL_CALL UndoManager::getAllUndoActionTitles(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().getAllUndoActionTitles();
    }

    Sequence< OUString > SAL_CALL UndoManager::getAllRedoActionTitles(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().getAllRedoActionTitles();
    }

    void SAL_CALL UndoManager::clear(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().clear( aGuard );
    }

    void SAL_CALL UndoManager::clearRedo(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().clearRedo( aGuard );
    }

    void SAL_CALL UndoManager::reset(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().reset( aGuard );
    }

    void SAL_CALL UndoManager::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().addUndoManagerListener( i_listener );
    }

    void SAL_CALL UndoManager::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().removeUndoManagerListener( i_listener );
    }

    void SAL_CALL UndoManager::lock(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().lock();
    }

    void SAL_CALL UndoManager::unlock(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().unlock();
    }

    sal_Bool SAL_CALL UndoManager::isLocked(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().isLocked();
    }

    Reference< XInterface > SAL_CALL UndoManager::getParent(  )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return *&m_pImpl->getParent();
    }

    void SAL_CALL UndoManager::setParent( const Reference< XInterface >& i_parent )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        (void)i_parent;
        throw NoSupportException( OUString(), m_pImpl->getThis() );
    }

    void SAL_CALL UndoManager::addModifyListener( const Reference< XModifyListener >& i_listener )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().addModifyListener( i_listener );
    }

    void SAL_CALL UndoManager::removeModifyListener( const Reference< XModifyListener >& i_listener )
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().removeModifyListener( i_listener );
    }

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
