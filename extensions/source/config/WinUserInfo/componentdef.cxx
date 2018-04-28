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

#include "WinUserInfoBe.hxx"
#include <cppuhelper/implementationentry.hxx>

using namespace extensions::config::WinUserInfo;

static css::uno::Reference<css::uno::XInterface>
createWinUserInfoBe(const css::uno::Reference<css::uno::XComponentContext>& aContext)
{
    return *new WinUserInfoBe(aContext);
}

static const cppu::ImplementationEntry kImplementations_entries[]
    = { { createWinUserInfoBe, WinUserInfoBe::getWinUserInfoBeName,
          WinUserInfoBe::getWinUserInfoBeServiceNames, cppu::createSingleComponentFactory, nullptr,
          0 },
        { nullptr, nullptr, nullptr, nullptr, nullptr, 0 } };

extern "C" SAL_DLLPUBLIC_EXPORT void*
WinUserInfoBe_component_getFactory(const sal_Char* aImplementationName, void* aServiceManager,
                                   void* aRegistryKey)
{
    return cppu::component_getFactoryHelper(aImplementationName, aServiceManager, aRegistryKey,
                                            kImplementations_entries);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
