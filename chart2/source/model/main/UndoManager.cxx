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

#include "precompiled_chart2.hxx"

#include "UndoManager.hxx"
#include "ChartViewHelper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <framework/undomanagerhelper.hxx>
#include <svl/undo.hxx>
#include <unotools/undoopt.hxx>

//......................................................................................................................
namespace chart
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
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::document::XUndoManager;
    using ::com::sun::star::document::EmptyUndoStackException;
    using ::com::sun::star::document::UndoContextNotClosedException;
    using ::com::sun::star::document::UndoFailedException;
    using ::com::sun::star::util::InvalidStateException;
    using ::com::sun::star::document::XUndoAction;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::document::XUndoManagerListener;
    using ::com::sun::star::util::NotLockedException;
    using ::com::sun::star::lang::NoSupportException;
    using ::com::sun::star::util::XModifyListener;
    using ::com::sun::star::frame::XModel;
    /** === end UNO using === **/

    namespace impl
    {
        //==============================================================================================================
        //= UndoManager_Impl
        //==============================================================================================================
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
                m_aUndoManager.SetMaxUndoActionCount( (USHORT)SvtUndoOptions().GetUndoCount() );
            }

            virtual ~UndoManager_Impl()
            {
            }

            // .........................................................................................................
            // IUndoManagerImplementation
            virtual ::osl::Mutex&               getMutex();
            virtual ::svl::IUndoManager&        getImplUndoManager();
            virtual Reference< XUndoManager >   getThis();

            // .........................................................................................................
            // attribute access
            ::cppu::OWeakObject&                getParent() { return m_rParent; }
            ::framework::UndoManagerHelper&     getUndoHelper() { return m_aUndoHelper; }

            // .........................................................................................................
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

        //--------------------------------------------------------------------------------------------------------------
        ::osl::Mutex& UndoManager_Impl::getMutex()
        {
            return m_rMutex;
        }

        //--------------------------------------------------------------------------------------------------------------
        ::svl::IUndoManager& UndoManager_Impl::getImplUndoManager()
        {
            return m_aUndoManager;
        }

        //--------------------------------------------------------------------------------------------------------------
        Reference< XUndoManager > UndoManager_Impl::getThis()
        {
            return &m_rAntiImpl;
        }

        //--------------------------------------------------------------------------------------------------------------
        void UndoManager_Impl::disposing()
        {
            {
                ::osl::MutexGuard aGuard( m_rMutex );
                m_bDisposed = true;
            }
            m_aUndoHelper.disposing();
        }

        //--------------------------------------------------------------------------------------------------------------
        void UndoManager_Impl::checkDisposed_lck()
        {
            if ( m_bDisposed )
                throw DisposedException( ::rtl::OUString(), getThis() );
        }

        //==============================================================================================================
        //= UndoManagerMethodGuard
        //==============================================================================================================
        /** guard for public UNO methods of the UndoManager

            The only purpose of this guard is to check for the instance being disposed already. Everything else,
            in particular the IMutexGuard functionality required by the UndoManagerHelper class, is a dummy only,
            as all involved classes (means we ourselves, the UndoManagerHelper, the SfxUndoManager, and the Undo actions
            we create) are inherently thread-safe, thus need no external lock (in particular no SolarMutex!).
        */
        class UndoManagerMethodGuard : public ::framework::IMutexGuard
        {
        public:
            UndoManagerMethodGuard( UndoManager_Impl& i_impl )
            {
                ::osl::MutexGuard aGuard( i_impl.getMutex() );
                // throw if the instance is already disposed
                i_impl.checkDisposed_lck();
            }
            virtual ~UndoManagerMethodGuard()
            {
            }

            // IMutexGuard
            virtual ::framework::IMutex& getGuardedMutex();

            // IGuard
            virtual void clear();
            virtual void reset();
        };

        class DummyMutex : public ::framework::IMutex
        {
        public:
            virtual void acquire() { }
            virtual void release() { }
        };

        //--------------------------------------------------------------------------------------------------------------
        ::framework::IMutex& UndoManagerMethodGuard::getGuardedMutex()
        {
            static DummyMutex s_aDummyMutex;
            return s_aDummyMutex;
        }

        //--------------------------------------------------------------------------------------------------------------
        void UndoManagerMethodGuard::clear()
        {
            // nothing to do. This interface implementation is a dummy.
        }

        //--------------------------------------------------------------------------------------------------------------
        void UndoManagerMethodGuard::reset()
        {
            // nothing to do. This interface implementation is a dummy.
        }
    }

    //==================================================================================================================
    //= UndoManager
    //==================================================================================================================
    using impl::UndoManagerMethodGuard;

    //------------------------------------------------------------------------------------------------------------------
    UndoManager::UndoManager( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex )
        :m_pImpl( new impl::UndoManager_Impl( *this, i_parent, i_mutex ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    UndoManager::~UndoManager()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::acquire() throw ()
    {
        m_pImpl->getParent().acquire();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::release() throw ()
    {
        m_pImpl->getParent().release();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoManager::disposing()
    {
        m_pImpl->disposing();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::enterUndoContext( const ::rtl::OUString& i_title ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().enterUndoContext( i_title, aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::enterHiddenUndoContext(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().enterHiddenUndoContext( aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::leaveUndoContext(  ) throw (InvalidStateException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().leaveUndoContext( aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::addUndoAction( const Reference< XUndoAction >& i_action ) throw (IllegalArgumentException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().addUndoAction( i_action, aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::undo(  ) throw (EmptyUndoStackException, UndoContextNotClosedException, UndoFailedException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().undo( aGuard );

        ChartViewHelper::setViewToDirtyState( Reference< XModel >( getParent(), UNO_QUERY ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::redo(  ) throw (EmptyUndoStackException, UndoContextNotClosedException, UndoFailedException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().redo( aGuard );

        ChartViewHelper::setViewToDirtyState( Reference< XModel >( getParent(), UNO_QUERY ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL UndoManager::isUndoPossible(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().isUndoPossible();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL UndoManager::isRedoPossible(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().isRedoPossible();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UndoManager::getCurrentUndoActionTitle(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().getCurrentUndoActionTitle();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UndoManager::getCurrentRedoActionTitle(  ) throw (EmptyUndoStackException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().getCurrentRedoActionTitle();
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UndoManager::getAllUndoActionTitles(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().getAllUndoActionTitles();
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UndoManager::getAllRedoActionTitles(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().getAllRedoActionTitles();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::clear(  ) throw (UndoContextNotClosedException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().clear( aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::clearRedo(  ) throw (UndoContextNotClosedException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().clearRedo( aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::reset(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().reset( aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::addUndoManagerListener( const Reference< XUndoManagerListener >& i_listener ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().addUndoManagerListener( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::removeUndoManagerListener( const Reference< XUndoManagerListener >& i_listener ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().removeUndoManagerListener( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::lock(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().lock();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::unlock(  ) throw (NotLockedException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().unlock();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL UndoManager::isLocked(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getUndoHelper().isLocked();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL UndoManager::getParent(  ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        return *&m_pImpl->getParent();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::setParent( const Reference< XInterface >& i_parent ) throw (NoSupportException, RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        (void)i_parent;
        throw NoSupportException( ::rtl::OUString(), m_pImpl->getThis() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::addModifyListener( const Reference< XModifyListener >& i_listener ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().addModifyListener( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManager::removeModifyListener( const Reference< XModifyListener >& i_listener ) throw (RuntimeException)
    {
        UndoManagerMethodGuard aGuard( *m_pImpl );
        m_pImpl->getUndoHelper().removeModifyListener( i_listener );
    }

//......................................................................................................................
} // namespace chart
//......................................................................................................................
