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

#include <unx/salinst.h>

class SalYieldMutex;
class SalFrame;

class KDESalInstance : public X11SalInstance
{
protected:
    virtual SalX11Display* CreateDisplay() const override;

public:
    explicit KDESalInstance(std::unique_ptr<SalYieldMutex> pMutex);
    virtual SalFrame* CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;

    virtual bool hasNativeFileSelection() const override { return true; }

    virtual css::uno::Reference< css::ui::dialogs::XFilePicker2 >
        createFilePicker( const css::uno::Reference<
                              css::uno::XComponentContext >& ) override;

    virtual bool IsMainThread() const override;

    std::unique_ptr<SalVirtualDevice> CreateVirtualDevice(SalGraphics* pGraphics,
        long &nDX, long &nDY, DeviceFormat eFormat, const SystemGraphicsData *pData) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
