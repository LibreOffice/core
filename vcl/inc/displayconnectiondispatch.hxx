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

#pragma once

#include <sal/config.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>
#include <vcl/dllapi.h>
#include <mutex>
#include <vector>

namespace vcl {

class DisplayEventHandler : public cppu::WeakImplHelper<>
{
public:
    virtual bool handleEvent(const void* pEvent) = 0;
    virtual void shutdown() noexcept = 0;
};

class VCL_DLLPUBLIC DisplayConnectionDispatch final : public cppu::OWeakObject
{
    std::mutex                      m_aMutex;
    std::vector<rtl::Reference<DisplayEventHandler>> m_aHandlers;
    OUString                        m_ConnectionIdentifier;
public:
    DisplayConnectionDispatch();
    ~DisplayConnectionDispatch() override;

    void start();
    void terminate();

    bool dispatchEvent(const void* pEvent);

    void addEventHandler(const rtl::Reference<DisplayEventHandler>& handler);
    void removeEventHandler(const rtl::Reference<DisplayEventHandler>& handler);
    OUString getIdentifier();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
