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

#include <vcl/GeometryProvider.hxx>
#include <sal/log.hxx>

class SalGraphics;

/// A non-visible drawable/buffer (e.g. an X11 Pixmap).
class VCL_PLUGIN_PUBLIC SalVirtualDevice
    : public vcl::SalGeometryProvider
{
public:
    SalVirtualDevice() {}
    virtual ~SalVirtualDevice() override;

    // SalGraphics or NULL, but two Graphics for all SalVirtualDevices
    // must be returned
    virtual SalGraphics*    AcquireGraphics() = 0;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) = 0;

    // Set new size, without saving the old contents
    bool SetSize(sal_Int32 nNewDX, sal_Int32 nNewDY, sal_Int32 nScale = 100)
        { return SetSizeUsingBuffer(nNewDX, nNewDY, nullptr, nScale); }
    inline void FixSetSizeParams(sal_Int32& nDX, sal_Int32& nDY, sal_Int32& nScale) const;

    // Set new size using a buffer at the given address
    virtual bool SetSizeUsingBuffer(sal_Int32 nNewDX, sal_Int32 nNewDY, sal_uInt8*, sal_Int32 nScale = -1) = 0;
};

void SalVirtualDevice::FixSetSizeParams(sal_Int32& nDX, sal_Int32& nDY, sal_Int32& nScale) const
{
    SAL_WARN_IF(!(nDX != 0 && nDY != 0 && nScale != 0), "vcl",
                "Invalid virtual device request (" << nDX << " " << nDY << " " << nScale << ")!");
    if (nDX == 0)
        nDX = 1;
    if (nDY == 0)
        nDY = 1;
    if (nScale <= 0)
        nScale = GetDPIScalePercentage();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
