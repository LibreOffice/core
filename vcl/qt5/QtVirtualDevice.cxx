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

#include <QtVirtualDevice.hxx>

#include <QtGraphics.hxx>
#include <QtTools.hxx>

#include <QtGui/QImage>

QtVirtualDevice::QtVirtualDevice(sal_Int32 nNewDX, sal_Int32 nNewDY, sal_Int32 nScale)
{
    SetSizeUsingBuffer(nNewDX, nNewDY, nullptr, nScale);
}

SalGraphics* QtVirtualDevice::AcquireGraphics()
{
    assert(m_pImage);
    QtGraphics* pGraphics = new QtGraphics(m_pImage.get());
    m_aGraphics.push_back(pGraphics);
    return pGraphics;
}

void QtVirtualDevice::ReleaseGraphics(SalGraphics* pGraphics)
{
    m_aGraphics.erase(
        std::remove(m_aGraphics.begin(), m_aGraphics.end(), dynamic_cast<QtGraphics*>(pGraphics)),
        m_aGraphics.end());
    delete pGraphics;
}

bool QtVirtualDevice::SetSizeUsingBuffer(sal_Int32 nNewDX, sal_Int32 nNewDY, sal_uInt8* pBuffer,
                                         sal_Int32 nScale)
{
    FixSetSizeParams(nNewDX, nNewDY, nScale);
    if (!pBuffer && m_pImage && m_pImage->width() == nNewDX && m_pImage->height() == nNewDY
        && nScale == GetDPIScalePercentage())
        return true;

    float fScale = nScale / 100.0;
    nNewDX *= fScale;
    nNewDY *= fScale;

    if (pBuffer)
        m_pImage.reset(new QImage(pBuffer, nNewDX, nNewDY, Qt_DefaultFormat32));
    else
        m_pImage.reset(new QImage(nNewDX, nNewDY, Qt_DefaultFormat32));

    m_pImage->fill(Qt::transparent);
    m_pImage->setDevicePixelRatio(fScale);

    // update device in existing graphics
    for (auto pQtGraph : m_aGraphics)
        pQtGraph->setQImage(m_pImage.get());

    return true;
}

sal_Int32 QtVirtualDevice::GetSgpMetric(vcl::SGPmetric eMetric) const
{
    assert(m_pImage);
    if (!m_pImage)
        return -1;
    return ::GetSgpMetricFromQImage(eMetric, *m_pImage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
