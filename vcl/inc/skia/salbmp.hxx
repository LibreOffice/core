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

#ifndef INCLUDED_VCL_INC_SKIA_SALBMP_H
#define INCLUDED_VCL_INC_SKIA_SALBMP_H

#include <salbmp.hxx>

#include <SkBitmap.h>

class VCL_PLUGIN_PUBLIC SkiaSalBitmap : public SalBitmap
{
public:
    SkiaSalBitmap();
    virtual ~SkiaSalBitmap() override;

public:
    // SalBitmap methods
    bool Create(const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal) override;
    bool Create(const SalBitmap& rSalBmp) override;
    bool Create(const SalBitmap& rSalBmp, SalGraphics* pGraphics) override;
    bool Create(const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount) override;
    bool Create(const css::uno::Reference<css::rendering::XBitmapCanvas>& rBitmapCanvas,
                Size& rSize, bool bMask = false) override;

    void Destroy() final override;

    Size GetSize() const override;
    sal_uInt16 GetBitCount() const override;

    BitmapBuffer* AcquireBuffer(BitmapAccessMode nMode) override;
    void ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode) override;

    bool GetSystemData(BitmapSystemData& rData) override;

    bool ScalingSupported() const override;
    bool Scale(const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag) override;
    bool Replace(const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol) override;
    bool ConvertToGreyscale() override;

private:
    //    SkBitmap mBitmap;
};

#endif // INCLUDED_VCL_INC_OPENGL_SALBMP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
