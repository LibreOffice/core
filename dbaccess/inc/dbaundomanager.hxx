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

#ifndef DBACCESS_DBAUNDOMANAGER_HXX
#define DBACCESS_DBAUNDOMANAGER_HXX

#include "dbaccessdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/document/XUndoManager.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <boost/scoped_ptr.hpp>

class SfxUndoManager;

//......................................................................................................................
namespace dbaui
{
//......................................................................................................................

    //==================================================================================================================
    //= UndoManager
    //==================================================================================================================
    struct UndoManager_Impl;
    typedef ::cppu::ImplHelper1< ::com::sun::star::document::XUndoManager > UndoManager_Base;
    class DBACCESS_DLLPUBLIC UndoManager : public UndoManager_Base
    {
    public:
        UndoManager( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex );
        virtual ~UndoManager();

        SfxUndoManager& GetSfxUndoManager() const;

        // XInterface
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // XComponent equivalents
        void disposing();

        // XUndoManager
        virtual void SAL_CALL enterUndoContext( const ::rtl::OUString& i_title ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL enterHiddenUndoContext(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL leaveUndoContext(  ) throw (::com::sun::star::util::InvalidStateException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addUndoAction( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoAction >& i_action ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL undo(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::document::UndoContextNotClosedException, ::com::sun::star::document::UndoFailedException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL redo(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::document::UndoContextNotClosedException, ::com::sun::star::document::UndoFailedException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isUndoPossible(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isRedoPossible(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getCurrentUndoActionTitle(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getCurrentRedoActionTitle(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAllUndoActionTitles(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAllRedoActionTitles(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clear(  ) throw (::com::sun::star::document::UndoContextNotClosedException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearRedo(  ) throw (::com::sun::star::document::UndoContextNotClosedException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL reset(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addUndoManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManagerListener >& i_listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeUndoManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManagerListener >& i_listener ) throw (::com::sun::star::uno::RuntimeException);

        // XLockable (base of XUndoManager)
        virtual void SAL_CALL lock(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL unlock(  ) throw (::com::sun::star::util::NotLockedException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isLocked(  ) throw (::com::sun::star::uno::RuntimeException);

        // XChild (base of XUndoManager)
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    private:
        ::boost::scoped_ptr< UndoManager_Impl > m_pImpl;
    };

//......................................................................................................................
} // namespace dbaui
//......................................................................................................................

#endif // DBACCESS_DBAUNDOMANAGER_HXX
