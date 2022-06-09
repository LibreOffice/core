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

#include <QtBitmap.hxx>
#include <QtTools.hxx>
#include <QtGraphics.hxx>

#include <QtGui/QImage>
#include <QtCore/QVector>
#include <QtGui/QColor>

#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <tools/helpers.hxx>

QtBitmap::QtBitmap() {}

QtBitmap::QtBitmap(const QImage& rImage) { m_pImage.reset(new QImage(rImage)); }

bool QtBitmap::Create(const Size& rSize, vcl::PixelFormat ePixelFormat, const BitmapPalette& rPal)
{
    if (ePixelFormat == vcl::PixelFormat::INVALID)
        return false;

    if (ePixelFormat == vcl::PixelFormat::N1_BPP)
        assert(2 >= rPal.GetEntryCount());
    if (ePixelFormat == vcl::PixelFormat::N8_BPP)
        assert(256 >= rPal.GetEntryCount());

    m_pImage.reset(new QImage(toQSize(rSize), getBitFormat(ePixelFormat)));
    m_pImage->fill(Qt::transparent);
    m_aPalette = rPal;

    auto count = rPal.GetEntryCount();
    if (count && m_pImage)
    {
        QVector<QRgb> aColorTable(count);
        for (unsigned i = 0; i < count; ++i)
            aColorTable[i] = qRgb(rPal[i].GetRed(), rPal[i].GetGreen(), rPal[i].GetBlue());
        m_pImage->setColorTable(std::move(aColorTable));
    }
    return true;
}

bool QtBitmap::Create(const SalBitmap& rSalBmp)
{
    const QtBitmap* pBitmap = static_cast<const QtBitmap*>(&rSalBmp);
    m_pImage.reset(new QImage(*pBitmap->m_pImage));
    m_aPalette = pBitmap->m_aPalette;
    return true;
}

bool QtBitmap::Create(const SalBitmap& rSalBmp, SalGraphics* pSalGraphics)
{
    const QtBitmap* pBitmap = static_cast<const QtBitmap*>(&rSalBmp);
    QtGraphics* pGraphics = static_cast<QtGraphics*>(pSalGraphics);
    QImage* pImage = pGraphics->getQImage();
    m_pImage.reset(new QImage(pBitmap->m_pImage->convertToFormat(pImage->format())));
    return true;
}

bool QtBitmap::Create(const SalBitmap& rSalBmp, vcl::PixelFormat eNewPixelFormat)
{
    if (eNewPixelFormat == vcl::PixelFormat::INVALID)
        return false;
    const QtBitmap* pBitmap = static_cast<const QtBitmap*>(&rSalBmp);
    m_pImage.reset(new QImage(pBitmap->m_pImage->convertToFormat(getBitFormat(eNewPixelFormat))));
    return true;
}

bool QtBitmap::Create(const css::uno::Reference<css::rendering::XBitmapCanvas>& /*rBitmapCanvas*/,
                      Size& /*rSize*/, bool /*bMask*/)
{
    return false;
}

void QtBitmap::Destroy() { m_pImage.reset(); }

Size QtBitmap::GetSize() const
{
    if (m_pImage)
        return toSize(m_pImage->size());
    return Size();
}

sal_uInt16 QtBitmap::GetBitCount() const
{
    if (m_pImage)
        return getFormatBits(m_pImage->format());
    return 0;
}

BitmapBuffer* QtBitmap::AcquireBuffer(BitmapAccessMode /*nMode*/)
{
    static const BitmapPalette aEmptyPalette;

    if (!m_pImage)
        return nullptr;

    BitmapBuffer* pBuffer = new BitmapBuffer;

    pBuffer->mnWidth = m_pImage->width();
    pBuffer->mnHeight = m_pImage->height();
    pBuffer->mnBitCount = getFormatBits(m_pImage->format());
    pBuffer->mpBits = m_pImage->bits();
    pBuffer->mnScanlineSize = m_pImage->bytesPerLine();

    switch (pBuffer->mnBitCount)
    {
        case 1:
            pBuffer->mnFormat = ScanlineFormat::N1BitMsbPal | ScanlineFormat::TopDown;
            pBuffer->maPalette = m_aPalette;
            break;
        case 8:
            pBuffer->mnFormat = ScanlineFormat::N8BitPal | ScanlineFormat::TopDown;
            pBuffer->maPalette = m_aPalette;
            break;
        case 24:
            pBuffer->mnFormat = ScanlineFormat::N24BitTcRgb | ScanlineFormat::TopDown;
            pBuffer->maPalette = aEmptyPalette;
            break;
        case 32:
        {
#ifdef OSL_BIGENDIAN
            pBuffer->mnFormat = ScanlineFormat::N32BitTcArgb | ScanlineFormat::TopDown;
#else
            pBuffer->mnFormat = ScanlineFormat::N32BitTcBgra | ScanlineFormat::TopDown;
#endif
            pBuffer->maPalette = aEmptyPalette;
            break;
        }
        default:
            assert(false);
    }

    return pBuffer;
}

void QtBitmap::ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode)
{
    m_aPalette = pBuffer->maPalette;
    auto count = m_aPalette.GetEntryCount();
    if (pBuffer->mnBitCount != 4 && count)
    {
        QVector<QRgb> aColorTable(count);
        for (unsigned i = 0; i < count; ++i)
            aColorTable[i]
                = qRgb(m_aPalette[i].GetRed(), m_aPalette[i].GetGreen(), m_aPalette[i].GetBlue());
        m_pImage->setColorTable(std::move(aColorTable));
    }
    delete pBuffer;
    if (nMode == BitmapAccessMode::Write)
        InvalidateChecksum();
}

bool QtBitmap::GetSystemData(BitmapSystemData& /*rData*/) { return false; }

bool QtBitmap::ScalingSupported() const { return false; }

bool QtBitmap::Scale(const double& /*rScaleX*/, const double& /*rScaleY*/,
                     BmpScaleFlag /*nScaleFlag*/)
{
    return false;
}

bool QtBitmap::Replace(const Color& /*rSearchColor*/, const Color& /*rReplaceColor*/,
                       sal_uInt8 /*nTol*/)
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
