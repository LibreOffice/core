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

#include <memory>

#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase1.hxx>
#include <dbaccess/dbaccessdllapi.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace document { class XUndoAction; }
    namespace document { class XUndoManagerListener; }
    namespace uno { class XInterface; }
}

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
    typedef ::cppu::ImplHelper1< css::document::XUndoManager > UndoManager_Base;
    class DBACCESS_DLLPUBLIC UndoManager final : public UndoManager_Base
    {
    public:
        UndoManager( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex );
        virtual ~UndoManager();

        SfxUndoManager& GetSfxUndoManager() const;

        // XInterface
        virtual void SAL_CALL acquire(  ) noexcept override;
        virtual void SAL_CALL release(  ) noexcept override;

        // XComponent equivalents
        void disposing();

        // XUndoManager
        virtual void SAL_CALL enterUndoContext( const OUString& i_title ) override;
        virtual void SAL_CALL enterHiddenUndoContext(  ) override;
        virtual void SAL_CALL leaveUndoContext(  ) override;
        virtual void SAL_CALL addUndoAction( const css::uno::Reference< css::document::XUndoAction >& i_action ) override;
        virtual void SAL_CALL undo(  ) override;
        virtual void SAL_CALL redo(  ) override;
        virtual sal_Bool SAL_CALL isUndoPossible(  ) override;
        virtual sal_Bool SAL_CALL isRedoPossible(  ) override;
        virtual OUString SAL_CALL getCurrentUndoActionTitle(  ) override;
        virtual OUString SAL_CALL getCurrentRedoActionTitle(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAllUndoActionTitles(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAllRedoActionTitles(  ) override;
        virtual void SAL_CALL clear(  ) override;
        virtual void SAL_CALL clearRedo(  ) override;
        virtual void SAL_CALL reset(  ) override;
        virtual void SAL_CALL addUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) override;
        virtual void SAL_CALL removeUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) override;

        // XLockable (base of XUndoManager)
        virtual void SAL_CALL lock(  ) override;
        virtual void SAL_CALL unlock(  ) override;
        virtual sal_Bool SAL_CALL isLocked(  ) override;

        // XChild (base of XUndoManager)
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

    private:
        std::unique_ptr< UndoManager_Impl > m_xImpl;
    };


} // namespace dbaui


#endif // INCLUDED_DBACCESS_DBAUNDOMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
