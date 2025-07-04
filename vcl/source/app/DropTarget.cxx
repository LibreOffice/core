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

#include <DropTarget.hxx>

using namespace com::sun::star;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;

DropTarget::DropTarget()
    : ::cppu::WeakComponentImplHelper<XDropTarget>(m_aMutex)
    , m_bActive(false)
    , m_nDefaultActions(0)
{
}

DropTarget::~DropTarget() {}

void DropTarget::addDropTargetListener(const uno::Reference<XDropTargetListener>& xListener)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    m_aListeners.push_back(xListener);
}

void DropTarget::removeDropTargetListener(const uno::Reference<XDropTargetListener>& xListener)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    std::erase(m_aListeners, xListener);
}

sal_Bool DropTarget::isActive() { return m_bActive; }

void DropTarget::setActive(sal_Bool active)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    m_bActive = active;
}

sal_Int8 DropTarget::getDefaultActions() { return m_nDefaultActions; }

void DropTarget::setDefaultActions(sal_Int8 actions)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    m_nDefaultActions = actions;
}

void DropTarget::drop(const DropTargetDropEvent& dtde) noexcept
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<uno::Reference<XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->drop(dtde);
    }
}

void DropTarget::dragEnter(const DropTargetDragEnterEvent& dtde) noexcept
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<uno::Reference<XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragEnter(dtde);
    }
}

void DropTarget::dragExit() noexcept
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<uno::Reference<XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    DropTargetEvent aEvent;
    aEvent.Source = getXWeak();
    for (auto const& listener : aListeners)
    {
        listener->dragExit(aEvent);
    }
}

void DropTarget::dragOver(const DropTargetDragEvent& dtde) noexcept
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<uno::Reference<XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragOver(dtde);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
