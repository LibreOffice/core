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

#include <comphelper/compbase.hxx>

namespace comphelper
{
WeakComponentImplHelperBase::~WeakComponentImplHelperBase() {}

// css::lang::XComponent
void SAL_CALL WeakComponentImplHelperBase::dispose()
{
    std::unique_lock aGuard(m_aMutex);
    if (m_bDisposed)
        return;
    m_bDisposed = true;
    disposing();
    css::lang::EventObject aEvt(static_cast<OWeakObject*>(this));
    maEventListeners.disposeAndClear(aGuard, aEvt);
}

void WeakComponentImplHelperBase::disposing() {}

void SAL_CALL WeakComponentImplHelperBase::addEventListener(
    css::uno::Reference<css::lang::XEventListener> const& rxListener)
{
    std::unique_lock aGuard(m_aMutex);
    if (m_bDisposed)
        return;
    maEventListeners.addInterface(rxListener);
}

void SAL_CALL WeakComponentImplHelperBase::removeEventListener(
    css::uno::Reference<css::lang::XEventListener> const& rxListener)
{
    std::unique_lock aGuard(m_aMutex);
    maEventListeners.removeInterface(rxListener);
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
