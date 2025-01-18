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

#include <QtVirtualDevice.hxx>

#include <QtGraphics.hxx>
#include <QtTools.hxx>

#include <QtGui/QImage>

QtVirtualDevice::QtVirtualDevice(double fScale)
    : m_fScale(fScale)
{
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
    std::erase(m_aGraphics, dynamic_cast<QtGraphics*>(pGraphics));
    delete pGraphics;
}

bool QtVirtualDevice::SetSize(tools::Long nNewDX, tools::Long nNewDY)
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

    m_pImage.reset(new QImage(nNewDX, nNewDY, Qt_DefaultFormat32));

    m_pImage->fill(Qt::transparent);
    m_pImage->setDevicePixelRatio(m_fScale);

    // update device in existing graphics
    for (auto pQtGraph : m_aGraphics)
        pQtGraph->ChangeQImage(m_pImage.get());

    return true;
}

tools::Long QtVirtualDevice::GetWidth() const { return m_pImage ? m_aFrameSize.width() : 0; }

tools::Long QtVirtualDevice::GetHeight() const { return m_pImage ? m_aFrameSize.height() : 0; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
