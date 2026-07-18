/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <cppuhelper/implbase2.hxx>

#include <memory>

namespace chart
{
class ChartModel;

    namespace impl
    {
        class UndoManager_Impl;
        typedef ::cppu::ImplHelper2 <   css::document::XUndoManager
                                    ,   css::util::XModifyBroadcaster
                                    >   UndoManager_Base;
    }

    class UndoManager : public impl::UndoManager_Base
    {
    public:
        UndoManager( ::chart::ChartModel& i_parent, ::osl::Mutex& i_mutex );
        virtual ~UndoManager();

        // XInterface
        virtual void acquire(  ) noexcept override;
        virtual void release(  ) noexcept override;

        // XComponent equivalents
        void disposing();

        // XUndoManager
        virtual void enterUndoContext( const OUString& i_title ) override;
        virtual void enterHiddenUndoContext(  ) override;
        virtual void leaveUndoContext(  ) override;
        virtual void addUndoAction( const css::uno::Reference< css::document::XUndoAction >& i_action ) override;
        virtual void undo(  ) override;
        virtual void redo(  ) override;
        virtual bool isUndoPossible(  ) override;
        virtual bool isRedoPossible(  ) override;
        virtual OUString getCurrentUndoActionTitle(  ) override;
        virtual OUString getCurrentRedoActionTitle(  ) override;
        virtual cpo::uno::Sequence< OUString > getAllUndoActionTitles(  ) override;
        virtual cpo::uno::Sequence< OUString > getAllRedoActionTitles(  ) override;
        virtual void clear(  ) override;
        virtual void clearRedo(  ) override;
        virtual void reset(  ) override;
        virtual void addUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) override;
        virtual void removeUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) override;

        // XLockable (base of XUndoManager)
        virtual void lock(  ) override;
        virtual void unlock(  ) override;
        virtual bool isLocked(  ) override;

        // XChild (base of XUndoManager)
        virtual css::uno::Reference< css::uno::XInterface > getParent(  ) override;
        virtual void setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

        // XModifyBroadcaster
        virtual void addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
        virtual void removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    private:
        std::unique_ptr< impl::UndoManager_Impl >   m_pImpl;
    };

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
