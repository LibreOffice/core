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

#include "QtSvpVirtualDevice.hxx"

#include <QtSvpSalFrame.hxx>
#include <QtSvpSalInstance.hxx>
#include <QtSvpSalInstance.moc>
#include <headless/svpbmp.hxx>

QtSvpSalInstance::QtSvpSalInstance(const OUString& rToolkitName)
    : QtInstance(rToolkitName)
{
}

QtSvpSalInstance::~QtSvpSalInstance() {}

std::unique_ptr<SalVirtualDevice>
QtSvpSalInstance::CreateVirtualDevice(SalGraphics& rGraphics, tools::Long nDX, tools::Long nDY,
                                      DeviceFormat /*eFormat*/, bool bAlphaMaskTransparent)
{
    SvpSalGraphics* pSvpSalGraphics = dynamic_cast<QtSvpGraphics*>(&rGraphics);
    assert(pSvpSalGraphics);
    // tdf#127529 see SvpSalInstance::CreateVirtualDevice for the rare case of a non-null pPreExistingTarget
    std::unique_ptr<SalVirtualDevice> pVD(
        new QtSvpVirtualDevice(pSvpSalGraphics->getSurface(), /*pPreExistingTarget*/ nullptr));
    pVD->SetSize(nDX, nDY, bAlphaMaskTransparent);
    return pVD;
}

std::unique_ptr<SalVirtualDevice>
QtSvpSalInstance::CreateVirtualDevice(SalGraphics& rGraphics, tools::Long& nDX, tools::Long& nDY,
                                      DeviceFormat, const SystemGraphicsData& rGd)
{
    SvpSalGraphics* pSvpSalGraphics = dynamic_cast<QtSvpGraphics*>(&rGraphics);
    assert(pSvpSalGraphics);
    // tdf#127529 see SvpSalInstance::CreateVirtualDevice for the rare case of a non-null pPreExistingTarget
    cairo_surface_t* pPreExistingTarget = static_cast<cairo_surface_t*>(rGd.pSurface);
    std::unique_ptr<SalVirtualDevice> pVD(
        new QtSvpVirtualDevice(pSvpSalGraphics->getSurface(), pPreExistingTarget));
    pVD->SetSize(nDX, nDY, /*bAlphaMaskTransparent*/ false);
    return pVD;
}

QtFrame* QtSvpSalInstance::DoCreateFrame(SalFrameStyleFlags nStyle, QtFrame* pParent)
{
    return new QtSvpSalFrame(pParent, nStyle);
}

std::shared_ptr<SalBitmap> QtSvpSalInstance::CreateSalBitmap()
{
    return std::make_shared<SvpSalBitmap>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
