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

#include "Qt5Bitmap.hxx"
#include "Qt5Tools.hxx"
#include "Qt5Graphics.hxx"

#include <QtGui/QImage>
#include <QtCore/QVector>
#include <QtGui/QColor>

Qt5Bitmap::Qt5Bitmap() {}

Qt5Bitmap::Qt5Bitmap(const QImage& rImage) { m_pImage.reset(new QImage(rImage)); }

Qt5Bitmap::~Qt5Bitmap() {}

bool Qt5Bitmap::Create(const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal)
{
    assert((nBitCount == 1 || nBitCount == 4 || nBitCount == 8 || nBitCount == 16 || nBitCount == 24
            || nBitCount == 32)
           && "Unsupported BitCount!");

    if (nBitCount == 1)
        assert(2 == rPal.GetEntryCount());
    if (nBitCount == 4)
        assert(16 == rPal.GetEntryCount());
    if (nBitCount == 8)
        assert(256 == rPal.GetEntryCount());

    if (nBitCount == 4)
    {
        m_pImage.reset();
        m_aSize = rSize;
        m_nScanline = rSize.Width() / 2 + (rSize.Width() % 2) ? 0 : 1;
        m_pBuffer.reset(new sal_uInt8[m_nScanline * rSize.Height()]);
    }
    else
    {
        m_pImage.reset(new QImage(toQSize(rSize), getBitFormat(nBitCount)));
        m_pBuffer.reset();
    }
    m_aPalette = rPal;

    auto count = rPal.GetEntryCount();
    if (nBitCount != 4 && count)
    {
        QVector<QRgb> aColorTable(count);
        for (unsigned i = 0; i < count; ++i)
            aColorTable[i] = qRgb(rPal[i].GetRed(), rPal[i].GetGreen(), rPal[i].GetBlue());
        m_pImage->setColorTable(aColorTable);
    }
    return true;
}

bool Qt5Bitmap::Create(const SalBitmap& rSalBmp)
{
    const Qt5Bitmap* pBitmap = static_cast<const Qt5Bitmap*>(&rSalBmp);
    if (pBitmap->m_pImage.get())
    {
        m_pImage.reset(new QImage(*pBitmap->m_pImage.get()));
        m_pBuffer.reset();
    }
    else
    {
        m_aSize = pBitmap->m_aSize;
        m_nScanline = pBitmap->m_nScanline;
        m_pBuffer.reset(new sal_uInt8[m_nScanline * m_aSize.Height()]);
        memcpy(m_pBuffer.get(), pBitmap->m_pBuffer.get(), m_nScanline);
        m_pImage.reset();
    }
    m_aPalette = pBitmap->m_aPalette;
    return true;
}

bool Qt5Bitmap::Create(const SalBitmap& rSalBmp, SalGraphics* pSalGraphics)
{
    const Qt5Bitmap* pBitmap = static_cast<const Qt5Bitmap*>(&rSalBmp);
    Qt5Graphics* pGraphics = static_cast<Qt5Graphics*>(pSalGraphics);
    QImage* pImage = pGraphics->m_pQImage;
    m_pImage.reset(new QImage(pBitmap->m_pImage->convertToFormat(pImage->format())));
    m_pBuffer.reset();
    return true;
}

bool Qt5Bitmap::Create(const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount)
{
    assert((nNewBitCount == 1 || nNewBitCount == 4 || nNewBitCount == 8 || nNewBitCount == 16
            || nNewBitCount == 24 || nNewBitCount == 32)
           && "Unsupported BitCount!");

    const Qt5Bitmap* pBitmap = static_cast<const Qt5Bitmap*>(&rSalBmp);
    if (pBitmap->m_pBuffer.get())
        return false;

    m_pImage.reset(new QImage(pBitmap->m_pImage->convertToFormat(getBitFormat(nNewBitCount))));
    return true;
}

bool Qt5Bitmap::Create(const css::uno::Reference<css::rendering::XBitmapCanvas>& rBitmapCanvas,
                       Size& rSize, bool bMask)
{
    return false;
}

void Qt5Bitmap::Destroy()
{
    m_pImage.reset();
    m_pBuffer.reset();
}

Size Qt5Bitmap::GetSize() const
{
    if (m_pBuffer.get())
        return m_aSize;
    else if (m_pImage.get())
        return toSize(m_pImage->size());
    return Size();
}

sal_uInt16 Qt5Bitmap::GetBitCount() const
{
    if (m_pBuffer.get())
        return 4;
    else if (m_pImage.get())
        return getFormatBits(m_pImage->format());
    return 0;
}

BitmapBuffer* Qt5Bitmap::AcquireBuffer(BitmapAccessMode nMode)
{
    static const BitmapPalette aEmptyPalette;

    if (!(m_pImage.get() || m_pBuffer.get()))
        return nullptr;

    BitmapBuffer* pBuffer = new BitmapBuffer;

    if (m_pBuffer.get())
    {
        pBuffer->mnWidth = m_aSize.Width();
        pBuffer->mnHeight = m_aSize.Height();
        pBuffer->mnBitCount = 4;
        pBuffer->mpBits = m_pBuffer.get();
        pBuffer->mnScanlineSize = m_nScanline;
    }
    else
    {
        pBuffer->mnWidth = m_pImage->width();
        pBuffer->mnHeight = m_pImage->height();
        pBuffer->mnBitCount = getFormatBits(m_pImage->format());
        pBuffer->mpBits = m_pImage->bits();
        pBuffer->mnScanlineSize = m_pImage->bytesPerLine();
    }

    switch (pBuffer->mnBitCount)
    {
        case 1:
            pBuffer->mnFormat = ScanlineFormat::N1BitLsbPal | ScanlineFormat::TopDown;
            pBuffer->maPalette = m_aPalette;
            break;
        case 4:
            pBuffer->mnFormat = ScanlineFormat::N4BitMsnPal | ScanlineFormat::TopDown;
            pBuffer->maPalette = m_aPalette;
            break;
        case 8:
            pBuffer->mnFormat = ScanlineFormat::N8BitPal | ScanlineFormat::TopDown;
            pBuffer->maPalette = m_aPalette;
            break;
        case 16:
        {
#ifdef OSL_BIGENDIAN
            pBuffer->mnFormat = ScanlineFormat::N16BitTcMsbMask | ScanlineFormat::TopDown;
#else
            pBuffer->mnFormat = ScanlineFormat::N16BitTcLsbMask | ScanlineFormat::TopDown;
#endif
            ColorMaskElement aRedMask(0xf800); // 5
            aRedMask.CalcMaskShift();
            ColorMaskElement aGreenMask(0x07e0); // 6
            aGreenMask.CalcMaskShift();
            ColorMaskElement aBlueMask(0x001f); // 5
            aBlueMask.CalcMaskShift();
            pBuffer->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
            pBuffer->maPalette = aEmptyPalette;
            break;
        }
        case 24:
            pBuffer->mnFormat = ScanlineFormat::N24BitTcRgb | ScanlineFormat::TopDown;
            pBuffer->maPalette = aEmptyPalette;
            break;
        case 32:
        {
            pBuffer->mnFormat = ScanlineFormat::N32BitTcArgb | ScanlineFormat::TopDown;
            pBuffer->maPalette = aEmptyPalette;
            break;
        }
    }

    return pBuffer;
}

void Qt5Bitmap::ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode)
{
    m_aPalette = pBuffer->maPalette;
    auto count = m_aPalette.GetEntryCount();
    if (pBuffer->mnBitCount != 4 && count)
    {
        QVector<QRgb> aColorTable(count);
        for (unsigned i = 0; i < count; ++i)
            aColorTable[i]
                = qRgb(m_aPalette[i].GetRed(), m_aPalette[i].GetGreen(), m_aPalette[i].GetBlue());
        m_pImage->setColorTable(aColorTable);
    }
    delete pBuffer;
}

bool Qt5Bitmap::GetSystemData(BitmapSystemData& rData) { return false; }

bool Qt5Bitmap::ScalingSupported() const { return false; }

bool Qt5Bitmap::Scale(const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag)
{
    return false;
}

bool Qt5Bitmap::Replace(const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol)
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
