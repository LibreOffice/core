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

#include <skia/salbmp.hxx>

SkiaSalBitmap::SkiaSalBitmap() {}

SkiaSalBitmap::~SkiaSalBitmap() {}

bool SkiaSalBitmap::Create(const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal)
{
    (void)rSize;
    (void)nBitCount;
    (void)rPal;
    return false;
}

bool SkiaSalBitmap::Create(const SalBitmap& rSalBmp)
{
    (void)rSalBmp;
    return false;
}

bool SkiaSalBitmap::Create(const SalBitmap& rSalBmp, SalGraphics* pGraphics)
{
    (void)rSalBmp;
    (void)pGraphics;
    return false;
}

bool SkiaSalBitmap::Create(const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount)
{
    (void)rSalBmp;
    (void)nNewBitCount;
    return false;
}

bool SkiaSalBitmap::Create(const css::uno::Reference<css::rendering::XBitmapCanvas>& rBitmapCanvas,
                           Size& rSize, bool bMask)
{
    (void)rBitmapCanvas;
    (void)rSize;
    (void)bMask;
    return false;
}

void SkiaSalBitmap::Destroy() {}

Size SkiaSalBitmap::GetSize() const { return Size(); }

sal_uInt16 SkiaSalBitmap::GetBitCount() const { return 0; }

BitmapBuffer* SkiaSalBitmap::AcquireBuffer(BitmapAccessMode nMode)
{
    (void)nMode;
    return nullptr;
}

void SkiaSalBitmap::ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode)
{
    (void)pBuffer;
    (void)nMode;
}

bool SkiaSalBitmap::GetSystemData(BitmapSystemData& rData)
{
    (void)rData;
    return false;
}

bool SkiaSalBitmap::ScalingSupported() const { return false; }

bool SkiaSalBitmap::Scale(const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag)
{
    (void)rScaleX;
    (void)rScaleY;
    (void)nScaleFlag;
    return false;
}

bool SkiaSalBitmap::Replace(const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol)
{
    (void)rSearchColor;
    (void)rReplaceColor;
    (void)nTol;
    return false;
}

bool SkiaSalBitmap::ConvertToGreyscale() { return false; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
