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

#ifndef DOCUMENT_UNDO_MANAGER_HXX
#define DOCUMENT_UNDO_MANAGER_HXX

#include "sfx2/sfxbasemodel.hxx"

#include <com/sun/star/document/XUndoManager.hpp>

#include <cppuhelper/implbase1.hxx>

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

//......................................................................................................................
namespace sfx2
{
//......................................................................................................................

    //==================================================================================================================
    //= DocumentUndoManager
    //==================================================================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::document::XUndoManager
                                >   DocumentUndoManager_Base;
    struct DocumentUndoManager_Impl;
    class DocumentUndoManager   :public DocumentUndoManager_Base
                                ,public SfxModelSubComponent
                                ,public ::boost::noncopyable
    {
        friend struct DocumentUndoManager_Impl;

    public:
        DocumentUndoManager( SfxBaseModel& i_document );
        virtual ~DocumentUndoManager();

        // SfxModelSubComponent overridables
        virtual void    disposing();

        // non-UNO API for our owner
        /** determines whether we have an open Undo context. No mutex locking within this method, no disposal check - this
            is the responsibility of the owner.
        */
        bool    isInContext() const;

        // XInterface
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // XUndoManager
        virtual void SAL_CALL enterUndoContext( const OUString& i_title ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL enterHiddenUndoContext(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL leaveUndoContext(  ) throw (::com::sun::star::util::InvalidStateException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addUndoAction( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoAction >& i_action ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL undo(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::document::UndoContextNotClosedException, ::com::sun::star::document::UndoFailedException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL redo(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::document::UndoContextNotClosedException, ::com::sun::star::document::UndoFailedException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isUndoPossible(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isRedoPossible(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getCurrentUndoActionTitle(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getCurrentRedoActionTitle(  ) throw (::com::sun::star::document::EmptyUndoStackException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAllUndoActionTitles(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAllRedoActionTitles(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clear(  ) throw (::com::sun::star::document::UndoContextNotClosedException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearRedo(  ) throw (::com::sun::star::document::UndoContextNotClosedException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL reset(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addUndoManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManagerListener >& i_listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeUndoManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManagerListener >& i_listener ) throw (::com::sun::star::uno::RuntimeException);

        // XLockable, base of XUndoManager
        virtual void SAL_CALL lock(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL unlock(  ) throw (::com::sun::star::util::NotLockedException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isLocked(  ) throw (::com::sun::star::uno::RuntimeException);

        // XChild, base of XUndoManager
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    private:
        ::boost::scoped_ptr< DocumentUndoManager_Impl > m_pImpl;
    };

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................

#endif // DOCUMENT_UNDO_MANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
