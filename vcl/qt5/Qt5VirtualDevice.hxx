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

#include <salvd.hxx>
#include <basegfx/vector/b2ivector.hxx>

#include <memory>
#include <list>

class Qt5Graphics;
class QImage;
enum class DeviceFormat;

class Qt5VirtualDevice : public SalVirtualDevice
{
    std::list<Qt5Graphics*> m_aGraphics;
    std::unique_ptr<QImage> m_pImage;
    DeviceFormat m_eFormat;
    basegfx::B2IVector m_aFrameSize;
    double m_fScale;

public:
    Qt5VirtualDevice(DeviceFormat eFormat, double fScale);
    virtual ~Qt5VirtualDevice() override;

    // SalVirtualDevice
    virtual SalGraphics* AcquireGraphics() override;
    virtual void ReleaseGraphics(SalGraphics* pGraphics) override;

    virtual bool SetSize(long nNewDX, long nNewDY) override;
    virtual bool SetSizeUsingBuffer(long nNewDX, long nNewDY, sal_uInt8* pBuffer) override;

    // SalGeometryProvider
    virtual long GetWidth() const override;
    virtual long GetHeight() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
