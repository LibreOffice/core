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

#include <SkImage.h>

class VCL_PLUGIN_PUBLIC SkiaSalBitmap final : public SalBitmap
{
public:
    SkiaSalBitmap();
    SkiaSalBitmap(const sk_sp<SkImage>& image);
    virtual ~SkiaSalBitmap() override;

    // SalBitmap methods
    virtual bool Create(const Size& rSize, sal_uInt16 nBitCount,
                        const BitmapPalette& rPal) override;
    virtual bool Create(const SalBitmap& rSalBmp) override;
    virtual bool Create(const SalBitmap& rSalBmp, SalGraphics* pGraphics) override;
    virtual bool Create(const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount) override;
    virtual bool Create(const css::uno::Reference<css::rendering::XBitmapCanvas>& rBitmapCanvas,
                        Size& rSize, bool bMask = false) override;

    virtual void Destroy() final override;

    virtual Size GetSize() const override;
    virtual sal_uInt16 GetBitCount() const override;

    virtual BitmapBuffer* AcquireBuffer(BitmapAccessMode nMode) override;
    virtual void ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode) override;

    virtual bool GetSystemData(BitmapSystemData& rData) override;

    virtual bool ScalingSupported() const override;
    virtual bool Scale(const double& rScaleX, const double& rScaleY,
                       BmpScaleFlag nScaleFlag) override;
    virtual bool Replace(const Color& rSearchColor, const Color& rReplaceColor,
                         sal_uInt8 nTol) override;
    virtual bool ConvertToGreyscale() override;

    // Returns the contents as SkImage (possibly GPU-backed).
    const sk_sp<SkImage>& GetSkImage() const;

    // Returns the contents as alpha SkImage (possibly GPU-backed)
    const sk_sp<SkImage>& GetAlphaSkImage() const;

#ifdef DBG_UTIL
    void dump(const char* file) const;
#endif

private:
    void ResetCachedBitmap();
    SkBitmap GetAsSkBitmap() const;
#ifdef DBG_UTIL
    void verify() const;
#else
    void verify() const {};
#endif

    template <typename charT, typename traits>
    friend inline std::basic_ostream<charT, traits>&
    operator<<(std::basic_ostream<charT, traits>& stream, const SkiaSalBitmap* bitmap)
    {
        // B - has SkBitmap, D - has data buffer, I/i - has SkImage (on GPU/CPU),
        // A/a - has alpha SkImage (on GPU/CPU)
        return stream << static_cast<const void*>(bitmap) << " " << bitmap->GetSize() << "/"
                      << bitmap->mBitCount << (!bitmap->mBitmap.drawsNothing() ? "B" : "")
                      << (bitmap->mBuffer.get() ? "D" : "")
                      << (bitmap->mImage ? (bitmap->mImage->isTextureBacked() ? "I" : "i") : "")
                      << (bitmap->mAlphaImage ? (bitmap->mAlphaImage->isTextureBacked() ? "A" : "a")
                                              : "");
    }

    // Bitmap pixels, if format is supported by Skia. If not, mBuffer is used.
    SkBitmap mBitmap;
    BitmapPalette mPalette;
    int mBitCount; // bpp
    Size mSize;
    std::unique_ptr<sal_uInt8[]> mBuffer;
    int mScanlineSize; // size of one row in mBuffer
    sk_sp<SkImage> mImage; // cached contents, possibly GPU-backed
    sk_sp<SkImage> mAlphaImage; // cached contents as alpha image, possibly GPU-backed
#ifdef DBG_UTIL
    int mWriteAccessCount; // number of write AcquireAccess() that have not been released
#endif
};

#endif // INCLUDED_VCL_INC_SKIA_SALBMP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
