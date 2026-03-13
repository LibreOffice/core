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

#include <QtBitmap.hxx>
#include <QtSalFrame.hxx>
#include <QtSalInstance.hxx>
#include <QtSalInstance.moc>
#include <QtVirtualDevice.hxx>

QtSalInstance::QtSalInstance(const OUString& rToolkitName)
    : QtInstance(rToolkitName)
{
}

QtSalInstance::~QtSalInstance() {}

std::unique_ptr<SalVirtualDevice> QtSalInstance::CreateVirtualDevice(SalGraphics&, tools::Long nDX,
                                                                     tools::Long nDY, DeviceFormat,
                                                                     bool bAlphaMaskTransparent)
{
    std::unique_ptr<SalVirtualDevice> pVD(new QtVirtualDevice(/*scale*/ 1));
    pVD->SetSize(nDX, nDY, bAlphaMaskTransparent);
    return pVD;
}

std::unique_ptr<SalVirtualDevice> QtSalInstance::CreateVirtualDevice(SalGraphics&, tools::Long& nDX,
                                                                     tools::Long& nDY, DeviceFormat,
                                                                     const SystemGraphicsData&)
{
    std::unique_ptr<SalVirtualDevice> pVD(new QtVirtualDevice(/*scale*/ 1));
    pVD->SetSize(nDX, nDY, /*bAlphaMaskTransparent*/ false);
    return pVD;
}

QtFrame* QtSalInstance::DoCreateFrame(SalFrameStyleFlags nStyle, QtFrame* pParent)
{
    return new QtSalFrame(pParent, nStyle);
}

std::shared_ptr<SalBitmap> QtSalInstance::CreateSalBitmap() { return std::make_shared<QtBitmap>(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
