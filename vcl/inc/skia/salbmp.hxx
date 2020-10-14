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

#include <boost/shared_ptr.hpp>

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
    virtual bool InterpretAs8Bit() override;
    virtual bool ConvertToGreyscale() override;
    virtual bool Erase(const Color& color) override;
    virtual bool AlphaBlendWith(const SalBitmap& rSalBmp) override;

    const BitmapPalette& Palette() const { return mPalette; }

    // True if GetSkShader() should be preferred to GetSkImage() (or the Alpha variants).
    bool PreferSkShader() const;

    // Returns the contents as SkImage (possibly GPU-backed).
    const sk_sp<SkImage>& GetSkImage() const;
    sk_sp<SkShader> GetSkShader() const;

    // Returns the contents as alpha SkImage (possibly GPU-backed)
    const sk_sp<SkImage>& GetAlphaSkImage() const;
    sk_sp<SkShader> GetAlphaSkShader() const;

    // Key for caching/hashing.
    OString GetImageKey() const;
    OString GetAlphaImageKey() const;

    // Returns true if it is known that this bitmap can be ignored if it's to be used
    // as an alpha bitmap. An optimization, not guaranteed to return true for all such cases.
    bool IsFullyOpaqueAsAlpha() const;
    // Alpha type best suitable for the content.
    SkAlphaType alphaType() const;

#ifdef DBG_UTIL
    void dump(const char* file) const;
#endif

    // These are to be used only by unittests.
    bool unittestHasBuffer() const { return mBuffer.get(); }
    bool unittestHasImage() const { return mImage.get(); }
    bool unittestHasAlphaImage() const { return mAlphaImage.get(); }
    bool unittestHasEraseColor() const { return mEraseColorSet; }
    const sal_uInt8* unittestGetBuffer() const { return mBuffer.get(); }
    const SkImage* unittestGetImage() const { return mImage.get(); }
    const SkImage* unittestGetAlphaImage() const { return mAlphaImage.get(); }

private:
    // Reset the state to pixel data (resets cached images allocated in GetSkImage()/GetAlphaSkImage()).
    void ResetToBuffer();
    // Sets the data only as SkImage (will be converted as needed).
    void ResetToSkImage(sk_sp<SkImage> image);
    // Resets all data that does not match mSize.
    void ResetCachedDataBySize();
    // Resets all data (buffer and images).
    void ResetAllData();
    // Call to ensure mBuffer has data (will convert from mImage if necessary).
    void EnsureBitmapData();
    void EnsureBitmapData() const { return const_cast<SkiaSalBitmap*>(this)->EnsureBitmapData(); }
    // Like EnsureBitmapData(), but will also make any shared data unique.
    // Call before changing the data.
    void EnsureBitmapUniqueData();
    // Allocate mBuffer (with uninitialized contents).
    void CreateBitmapData();
    // Should be called whenever mPixelsSize or mBitCount is set/changed.
    bool ComputeScanlineSize();
    // Sets bitmap to be erased on demand.
    void EraseInternal(const Color& color);
    // Sets pixels to the erase color.
    void PerformErase();
    SkBitmap GetAsSkBitmap() const;
    bool ConserveMemory() const;
    void verify() const
#ifdef DBG_UTIL
        ;
#else
    {
    }
#endif

    template <typename charT, typename traits>
    friend inline std::basic_ostream<charT, traits>&
    operator<<(std::basic_ostream<charT, traits>& stream, const SkiaSalBitmap* bitmap)
    {
        if (bitmap == nullptr)
            return stream << "(null)";
        // p - has (non-trivial) palette
        // I/i - has SkImage (on GPU/CPU),
        // A/a - has alpha SkImage (on GPU/CPU)
        // E - has erase color
        stream << static_cast<const void*>(bitmap) << " " << bitmap->GetSize() << "x"
               << bitmap->GetBitCount();
        if (bitmap->GetBitCount() <= 8 && !bitmap->Palette().IsGreyPalette8Bit())
            stream << "p";
        stream << "/";
        if (bitmap->mImage)
            stream << (bitmap->mImage->isTextureBacked() ? "I" : "i");
        if (bitmap->mAlphaImage)
            stream << (bitmap->mAlphaImage->isTextureBacked() ? "A" : "a");
        if (bitmap->mEraseColorSet)
            stream << "E" << bitmap->mEraseColor;
        return stream;
    }

    BitmapPalette mPalette;
    int mBitCount = 0; // bpp
    Size mSize;
    // The contents of the bitmap may be stored in several different ways:
    // As mBuffer buffer, which normally stores pixels in the given format.
    // As SkImage, as cached GPU-backed data, but sometimes also a result of some operation.
    // There is no "master" storage that the other would be derived from. The usual
    // mode of operation is that mBuffer holds the data, mImage is created
    // on demand as GPU-backed cached data by calling GetSkImage(), and the cached mImage
    // is reset by ResetCachedImage(). But sometimes only mImage will be set and in that case
    // mBuffer must be filled from it on demand if necessary by EnsureBitmapData().
    boost::shared_ptr<sal_uInt8[]> mBuffer;
    int mScanlineSize; // size of one row in mBuffer (based on mPixelsSize)
    sk_sp<SkImage> mImage; // possibly GPU-backed
    sk_sp<SkImage> mAlphaImage; // cached contents as alpha image, possibly GPU-backed
    // Actual scaling triggered by scale() is done on-demand. This is the size of the pixel
    // data in mBuffer, if it differs from mSize, then there is a scaling operation pending.
    Size mPixelsSize;
    SkFilterQuality mScaleQuality = kHigh_SkFilterQuality; // quality for on-demand scaling
    // Erase() is delayed, just sets these two instead of filling the buffer.
    bool mEraseColorSet = false;
    Color mEraseColor;
#ifdef DBG_UTIL
    int mWriteAccessCount = 0; // number of write AcquireAccess() that have not been released
#endif
};

#endif // INCLUDED_VCL_INC_SKIA_SALBMP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
