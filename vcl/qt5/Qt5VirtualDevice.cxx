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

#include <Qt5VirtualDevice.hxx>

#include <Qt5Graphics.hxx>
#include <Qt5Tools.hxx>

#include <QtGui/QImage>

Qt5VirtualDevice::Qt5VirtualDevice(DeviceFormat eFormat, double fScale)
    : m_eFormat(eFormat)
    , m_fScale(fScale)
{
}

SalGraphics* Qt5VirtualDevice::AcquireGraphics()
{
    assert(m_pImage);
    Qt5Graphics* pGraphics = new Qt5Graphics(m_pImage.get());
    m_aGraphics.push_back(pGraphics);
    return pGraphics;
}

void Qt5VirtualDevice::ReleaseGraphics(SalGraphics* pGraphics)
{
    m_aGraphics.erase(
        std::remove(m_aGraphics.begin(), m_aGraphics.end(), dynamic_cast<Qt5Graphics*>(pGraphics)),
        m_aGraphics.end());
    delete pGraphics;
}

bool Qt5VirtualDevice::SetSize(long nNewDX, long nNewDY)
{
    return SetSizeUsingBuffer(nNewDX, nNewDY, nullptr);
}

bool Qt5VirtualDevice::SetSizeUsingBuffer(long nNewDX, long nNewDY, sal_uInt8* pBuffer)
{
    if (nNewDX == 0)
        nNewDX = 1;
    if (nNewDY == 0)
        nNewDY = 1;

    if (m_pImage && m_aFrameSize.width() == nNewDX && m_aFrameSize.height() == nNewDY)
        return true;

    m_aFrameSize = QSize(nNewDX, nNewDY);

    nNewDX *= m_fScale;
    nNewDY *= m_fScale;

    if (m_eFormat == DeviceFormat::BITMASK)
    {
        m_pImage.reset(new QImage(nNewDX, nNewDY, QImage::Format_Mono));
    }
    else
    {
        if (pBuffer)
            m_pImage.reset(new QImage(pBuffer, nNewDX, nNewDY, Qt5_DefaultFormat32));
        else
            m_pImage.reset(new QImage(nNewDX, nNewDY, Qt5_DefaultFormat32));
    }

    m_pImage->fill(Qt::transparent);
    m_pImage->setDevicePixelRatio(m_fScale);

    // update device in existing graphics
    for (auto pQt5Graph : m_aGraphics)
        pQt5Graph->ChangeQImage(m_pImage.get());

    return true;
}

long Qt5VirtualDevice::GetWidth() const { return m_pImage ? m_aFrameSize.width() : 0; }

long Qt5VirtualDevice::GetHeight() const { return m_pImage ? m_aFrameSize.height() : 0; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
