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

#ifndef INCLUDED_DBACCESS_DBAUNDOMANAGER_HXX
#define INCLUDED_DBACCESS_DBAUNDOMANAGER_HXX

#include <exception>
#include <memory>

#include <com/sun/star/document/EmptyUndoStackException.hpp>
#include <com/sun/star/document/UndoContextNotClosedException.hpp>
#include <com/sun/star/document/UndoFailedException.hpp>
#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/InvalidStateException.hpp>
#include <com/sun/star/util/NotLockedException.hpp>
#include <cppuhelper/implbase.hxx>
#include <dbaccess/dbaccessdllapi.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace document { class XUndoAction; }
    namespace document { class XUndoManagerListener; }
    namespace uno { class XInterface; }
} } }

namespace cppu {
    class OWeakObject;
}

namespace osl {
    class Mutex;
}

class SfxUndoManager;


namespace dbaui
{


    //= UndoManager

    struct UndoManager_Impl;
    typedef ::cppu::ImplHelper < css::document::XUndoManager > UndoManager_Base;
    class DBACCESS_DLLPUBLIC UndoManager : public UndoManager_Base
    {
    public:
        UndoManager( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex );
        virtual ~UndoManager();

        SfxUndoManager& GetSfxUndoManager() const;

        // XInterface
        virtual void SAL_CALL acquire(  ) throw () override;
        virtual void SAL_CALL release(  ) throw () override;

        // XComponent equivalents
        void disposing();

        // XUndoManager
        virtual void SAL_CALL enterUndoContext( const OUString& i_title ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL enterHiddenUndoContext(  ) throw (css::document::EmptyUndoStackException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL leaveUndoContext(  ) throw (css::util::InvalidStateException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addUndoAction( const css::uno::Reference< css::document::XUndoAction >& i_action ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL undo(  ) throw (css::document::EmptyUndoStackException, css::document::UndoContextNotClosedException, css::document::UndoFailedException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL redo(  ) throw (css::document::EmptyUndoStackException, css::document::UndoContextNotClosedException, css::document::UndoFailedException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isUndoPossible(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isRedoPossible(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getCurrentUndoActionTitle(  ) throw (css::document::EmptyUndoStackException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getCurrentRedoActionTitle(  ) throw (css::document::EmptyUndoStackException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAllUndoActionTitles(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAllRedoActionTitles(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL clear(  ) throw (css::document::UndoContextNotClosedException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL clearRedo(  ) throw (css::document::UndoContextNotClosedException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL reset(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) throw (css::uno::RuntimeException, std::exception) override;

        // XLockable (base of XUndoManager)
        virtual void SAL_CALL lock(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL unlock(  ) throw (css::util::NotLockedException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isLocked(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XChild (base of XUndoManager)
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    private:
        std::unique_ptr< UndoManager_Impl > m_xImpl;
    };


} // namespace dbaui


#endif // INCLUDED_DBACCESS_DBAUNDOMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
