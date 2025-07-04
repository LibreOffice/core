/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>
#include <vcl/dllapi.h>

#pragma once

class VCL_DLLPUBLIC DropTarget
    : public ::cppu::WeakComponentImplHelper<css::datatransfer::dnd::XDropTarget>
{
    osl::Mutex m_aMutex;
    bool m_bActive;
    sal_Int8 m_nDefaultActions;
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> m_aListeners;

protected:
    DropTarget();
    virtual ~DropTarget() override;

public:
    // convenience functions that loop over listeners
    void dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde) noexcept;
    void dragExit() noexcept;
    void dragOver(const css::datatransfer::dnd::DropTargetDragEvent& dtde) noexcept;
    virtual void drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde) noexcept;

    // XDropTarget
    virtual void SAL_CALL addDropTargetListener(
        const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>&) override final;
    virtual void SAL_CALL removeDropTargetListener(
        const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>&) override final;
    virtual sal_Bool SAL_CALL isActive() override final;
    virtual void SAL_CALL setActive(sal_Bool active) override final;
    virtual sal_Int8 SAL_CALL getDefaultActions() override final;
    virtual void SAL_CALL setDefaultActions(sal_Int8 actions) override final;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
