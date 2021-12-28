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

#include <config_features.h>

#include <memory>
#include <numeric>

#include <headless/svpgdi.hxx>
#include <headless/svpbmp.hxx>
#include <headless/svpframe.hxx>
#include <headless/svpcairotextrender.hxx>
#include <saldatabasic.hxx>

#include <sal/log.hxx>
#include <tools/helpers.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/gradient.hxx>
#include <config_cairo_canvas.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/systemdependentdata.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <comphelper/lok.hxx>
#include <unx/gendata.hxx>
#include <dlfcn.h>

#if ENABLE_CAIRO_CANVAS
#   if defined CAIRO_VERSION && CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 10, 0)
#      define CAIRO_OPERATOR_DIFFERENCE (static_cast<cairo_operator_t>(23))
#   endif
#endif

bool SvpSalGraphics::blendBitmap( const SalTwoRect&, const SalBitmap& /*rBitmap*/ )
{
    return false;
}

bool SvpSalGraphics::blendAlphaBitmap( const SalTwoRect&, const SalBitmap&, const SalBitmap&, const SalBitmap& )
{
    return false;
}

namespace
{
    cairo_format_t getCairoFormat(const BitmapBuffer& rBuffer)
    {
        cairo_format_t nFormat;
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
        assert(rBuffer.mnBitCount == 32 || rBuffer.mnBitCount == 24 || rBuffer.mnBitCount == 1);
#else
        assert(rBuffer.mnBitCount == 32 || rBuffer.mnBitCount == 1);
#endif

        if (rBuffer.mnBitCount == 32)
            nFormat = CAIRO_FORMAT_ARGB32;
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
        else if (rBuffer.mnBitCount == 24)
            nFormat = CAIRO_FORMAT_RGB24_888;
#endif
        else
            nFormat = CAIRO_FORMAT_A1;
        return nFormat;
    }

    void Toggle1BitTransparency(const BitmapBuffer& rBuf)
    {
        assert(rBuf.maPalette.GetBestIndex(BitmapColor(COL_BLACK)) == 0);
        // TODO: make upper layers use standard alpha
        if (getCairoFormat(rBuf) == CAIRO_FORMAT_A1)
        {
            const int nImageSize = rBuf.mnHeight * rBuf.mnScanlineSize;
            unsigned char* pDst = rBuf.mpBits;
            for (int i = nImageSize; --i >= 0; ++pDst)
                *pDst = ~*pDst;
        }
    }

    std::unique_ptr<BitmapBuffer> FastConvert24BitRgbTo32BitCairo(const BitmapBuffer* pSrc)
    {
        if (pSrc == nullptr)
            return nullptr;

        assert(pSrc->mnFormat == SVP_24BIT_FORMAT);
        const tools::Long nWidth = pSrc->mnWidth;
        const tools::Long nHeight = pSrc->mnHeight;
        std::unique_ptr<BitmapBuffer> pDst(new BitmapBuffer);
        pDst->mnFormat = (ScanlineFormat::N32BitTcArgb | ScanlineFormat::TopDown);
        pDst->mnWidth = nWidth;
        pDst->mnHeight = nHeight;
        pDst->mnBitCount = 32;
        pDst->maColorMask = pSrc->maColorMask;
        pDst->maPalette = pSrc->maPalette;

        tools::Long nScanlineBase;
        const bool bFail = o3tl::checked_multiply<tools::Long>(pDst->mnBitCount, nWidth, nScanlineBase);
        if (bFail)
        {
            SAL_WARN("vcl.gdi", "checked multiply failed");
            pDst->mpBits = nullptr;
            return nullptr;
        }

        pDst->mnScanlineSize = AlignedWidth4Bytes(nScanlineBase);
        if (pDst->mnScanlineSize < nScanlineBase/8)
        {
            SAL_WARN("vcl.gdi", "scanline calculation wraparound");
            pDst->mpBits = nullptr;
            return nullptr;
        }

        try
        {
            pDst->mpBits = new sal_uInt8[ pDst->mnScanlineSize * nHeight ];
        }
        catch (const std::bad_alloc&)
        {
            // memory exception, clean up
            pDst->mpBits = nullptr;
            return nullptr;
        }

        for (tools::Long y = 0; y < nHeight; ++y)
        {
            sal_uInt8* pS = pSrc->mpBits + y * pSrc->mnScanlineSize;
            sal_uInt8* pD = pDst->mpBits + y * pDst->mnScanlineSize;
            for (tools::Long x = 0; x < nWidth; ++x)
            {
#if defined(ANDROID) && !HAVE_FEATURE_ANDROID_LOK
                static_assert((SVP_CAIRO_FORMAT & ~ScanlineFormat::TopDown) == ScanlineFormat::N32BitTcRgba, "Expected SVP_CAIRO_FORMAT set to N32BitTcBgra");
                static_assert((SVP_24BIT_FORMAT & ~ScanlineFormat::TopDown) == ScanlineFormat::N24BitTcRgb, "Expected SVP_24BIT_FORMAT set to N24BitTcRgb");
                pD[0] = pS[0];
                pD[1] = pS[1];
                pD[2] = pS[2];
                pD[3] = 0xff; // Alpha
#elif defined OSL_BIGENDIAN
                static_assert((SVP_CAIRO_FORMAT & ~ScanlineFormat::TopDown) == ScanlineFormat::N32BitTcArgb, "Expected SVP_CAIRO_FORMAT set to N32BitTcBgra");
                static_assert((SVP_24BIT_FORMAT & ~ScanlineFormat::TopDown) == ScanlineFormat::N24BitTcRgb, "Expected SVP_24BIT_FORMAT set to N24BitTcRgb");
                pD[0] = 0xff; // Alpha
                pD[1] = pS[0];
                pD[2] = pS[1];
                pD[3] = pS[2];
#else
                static_assert((SVP_CAIRO_FORMAT & ~ScanlineFormat::TopDown) == ScanlineFormat::N32BitTcBgra, "Expected SVP_CAIRO_FORMAT set to N32BitTcBgra");
                static_assert((SVP_24BIT_FORMAT & ~ScanlineFormat::TopDown) == ScanlineFormat::N24BitTcBgr, "Expected SVP_24BIT_FORMAT set to N24BitTcBgr");
                pD[0] = pS[0];
                pD[1] = pS[1];
                pD[2] = pS[2];
                pD[3] = 0xff; // Alpha
#endif

                pS += 3;
                pD += 4;
            }
        }

        return pDst;
    }

    // check for env var that decides for using downscale pattern
    const char* pDisableDownScale(getenv("SAL_DISABLE_CAIRO_DOWNSCALE"));
    bool bDisableDownScale(nullptr != pDisableDownScale);

    class SurfaceHelper
    {
    private:
        cairo_surface_t* pSurface;
        std::unordered_map<sal_uInt64, cairo_surface_t*> maDownscaled;

        SurfaceHelper(const SurfaceHelper&) = delete;
        SurfaceHelper& operator=(const SurfaceHelper&) = delete;

        cairo_surface_t* implCreateOrReuseDownscale(
            unsigned long nTargetWidth,
            unsigned long nTargetHeight)
        {
            const unsigned long nSourceWidth(cairo_image_surface_get_width(pSurface));
            const unsigned long nSourceHeight(cairo_image_surface_get_height(pSurface));

            // zoomed in, need to stretch at paint, no pre-scale useful
            if(nTargetWidth >= nSourceWidth || nTargetHeight >= nSourceHeight)
            {
                return pSurface;
            }

            // calculate downscale factor
            unsigned long nWFactor(1);
            unsigned long nW((nSourceWidth + 1) / 2);
            unsigned long nHFactor(1);
            unsigned long nH((nSourceHeight + 1) / 2);

            while(nW > nTargetWidth && nW > 1)
            {
                nW = (nW + 1) / 2;
                nWFactor *= 2;
            }

            while(nH > nTargetHeight && nH > 1)
            {
                nH = (nH + 1) / 2;
                nHFactor *= 2;
            }

            if(1 == nWFactor && 1 == nHFactor)
            {
                // original size *is* best binary size, use it
                return pSurface;
            }

            // go up one scale again - look for no change
            nW  = (1 == nWFactor) ? nTargetWidth : nW * 2;
            nH  = (1 == nHFactor) ? nTargetHeight : nH * 2;

            // check if we have a downscaled version of required size
            // bail out if the multiplication for the key would overflow
            if( nW >= SAL_MAX_UINT32 || nH >= SAL_MAX_UINT32 )
                return pSurface;
            const sal_uInt64 key((nW * static_cast<sal_uInt64>(SAL_MAX_UINT32)) + nH);
            auto isHit(maDownscaled.find(key));

            if(isHit != maDownscaled.end())
            {
                return isHit->second;
            }

            // create new surface in the targeted size
            cairo_surface_t* pSurfaceTarget = cairo_surface_create_similar(
                pSurface,
                cairo_surface_get_content(pSurface),
                nW,
                nH);

            // made a version to scale self first that worked well, but would've
            // been hard to support CAIRO_FORMAT_A1 including bit shifting, so
            // I decided to go with cairo itself - use CAIRO_FILTER_FAST or
            // CAIRO_FILTER_GOOD though. Please modify as needed for
            // performance/quality
            cairo_t* cr = cairo_create(pSurfaceTarget);
            const double fScaleX(static_cast<double>(nW)/static_cast<double>(nSourceWidth));
            const double fScaleY(static_cast<double>(nH)/static_cast<double>(nSourceHeight));
            cairo_scale(cr, fScaleX, fScaleY);
            cairo_set_source_surface(cr, pSurface, 0.0, 0.0);
            cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_GOOD);
            cairo_paint(cr);
            cairo_destroy(cr);

            // need to set device_scale for downscale surfaces to get
            // them handled correctly
            cairo_surface_set_device_scale(pSurfaceTarget, fScaleX, fScaleY);

            // add entry to cached entries
            maDownscaled[key] = pSurfaceTarget;

            return pSurfaceTarget;
        }

    protected:
        cairo_surface_t* implGetSurface() const { return pSurface; }
        void implSetSurface(cairo_surface_t* pNew) { pSurface = pNew; }

        bool isTrivial() const
        {
            constexpr unsigned long nMinimalSquareSizeToBuffer(64*64);
            const unsigned long nSourceWidth(cairo_image_surface_get_width(pSurface));
            const unsigned long nSourceHeight(cairo_image_surface_get_height(pSurface));

            return nSourceWidth * nSourceHeight < nMinimalSquareSizeToBuffer;
        }

    public:
        explicit SurfaceHelper()
        :   pSurface(nullptr)
        {
        }
        ~SurfaceHelper()
        {
            cairo_surface_destroy(pSurface);
            for(auto& candidate : maDownscaled)
            {
                cairo_surface_destroy(candidate.second);
            }
        }
        cairo_surface_t* getSurface(
            unsigned long nTargetWidth = 0,
            unsigned long nTargetHeight = 0) const
        {
            if (bDisableDownScale || 0 == nTargetWidth || 0 == nTargetHeight || !pSurface || isTrivial())
            {
                // caller asks for original or disabled or trivial (smaller then a minimal square size)
                // also excludes zero cases for width/height after this point if need to prescale
                return pSurface;
            }

            return const_cast<SurfaceHelper*>(this)->implCreateOrReuseDownscale(
                nTargetWidth,
                nTargetHeight);
        }
    };

    class BitmapHelper : public SurfaceHelper
    {
    private:
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
        const bool m_bForceARGB32;
#endif
        SvpSalBitmap aTmpBmp;

    public:
        explicit BitmapHelper(
            const SalBitmap& rSourceBitmap,
            const bool bForceARGB32 = false)
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
            : m_bForceARGB32(bForceARGB32)
#endif
        {
            const SvpSalBitmap& rSrcBmp = static_cast<const SvpSalBitmap&>(rSourceBitmap);
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
            if ((rSrcBmp.GetBitCount() != 32 && rSrcBmp.GetBitCount() != 24) || bForceARGB32)
#else
            (void)bForceARGB32;
            if (rSrcBmp.GetBitCount() != 32)
#endif
            {
                //big stupid copy here
                const BitmapBuffer* pSrc = rSrcBmp.GetBuffer();
                const SalTwoRect aTwoRect = { 0, 0, pSrc->mnWidth, pSrc->mnHeight,
                                              0, 0, pSrc->mnWidth, pSrc->mnHeight };
                std::unique_ptr<BitmapBuffer> pTmp = (pSrc->mnFormat == SVP_24BIT_FORMAT
                                   ? FastConvert24BitRgbTo32BitCairo(pSrc)
                                   : StretchAndConvert(*pSrc, aTwoRect, SVP_CAIRO_FORMAT));
                aTmpBmp.Create(std::move(pTmp));

                assert(aTmpBmp.GetBitCount() == 32);
                implSetSurface(SvpSalGraphics::createCairoSurface(aTmpBmp.GetBuffer()));
            }
            else
            {
                implSetSurface(SvpSalGraphics::createCairoSurface(rSrcBmp.GetBuffer()));
            }
        }
        void mark_dirty()
        {
            cairo_surface_mark_dirty(implGetSurface());
        }
        unsigned char* getBits(sal_Int32 &rStride)
        {
            cairo_surface_flush(implGetSurface());

            unsigned char *mask_data = cairo_image_surface_get_data(implGetSurface());

            const cairo_format_t nFormat = cairo_image_surface_get_format(implGetSurface());
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
            if (!m_bForceARGB32)
                assert(nFormat == CAIRO_FORMAT_RGB24_888 && "Expected RGB24_888 image");
            else
#endif
            {
                assert(nFormat == CAIRO_FORMAT_ARGB32 && "need to implement CAIRO_FORMAT_A1 after all here");
            }

            rStride = cairo_format_stride_for_width(nFormat, cairo_image_surface_get_width(implGetSurface()));

            return mask_data;
        }
    };

    sal_Int64 estimateUsageInBytesForSurfaceHelper(const SurfaceHelper* pHelper)
    {
        sal_Int64 nRetval(0);

        if(nullptr != pHelper)
        {
            cairo_surface_t* pSurface(pHelper->getSurface());

            if(pSurface)
            {
                const tools::Long nStride(cairo_image_surface_get_stride(pSurface));
                const tools::Long nHeight(cairo_image_surface_get_height(pSurface));

                nRetval = nStride * nHeight;

                // if we do downscale, size will grow by 1/4 + 1/16 + 1/32 + ...,
                // rough estimation just multiplies by 1.25, should be good enough
                // for estimation of buffer survival time
                if(!bDisableDownScale)
                {
                    nRetval = (nRetval * 5) / 4;
                }
            }
        }

        return nRetval;
    }

    class SystemDependentData_BitmapHelper : public basegfx::SystemDependentData
    {
    private:
        std::shared_ptr<BitmapHelper>       maBitmapHelper;

    public:
        SystemDependentData_BitmapHelper(
            basegfx::SystemDependentDataManager& rSystemDependentDataManager,
            const std::shared_ptr<BitmapHelper>& rBitmapHelper)
        :   basegfx::SystemDependentData(rSystemDependentDataManager),
            maBitmapHelper(rBitmapHelper)
        {
        }

        const std::shared_ptr<BitmapHelper>& getBitmapHelper() const { return maBitmapHelper; };
        virtual sal_Int64 estimateUsageInBytes() const override;
    };

    sal_Int64 SystemDependentData_BitmapHelper::estimateUsageInBytes() const
    {
        return estimateUsageInBytesForSurfaceHelper(maBitmapHelper.get());
    }

    class MaskHelper : public SurfaceHelper
    {
    private:
        std::unique_ptr<unsigned char[]> pAlphaBits;

    public:
        explicit MaskHelper(const SalBitmap& rAlphaBitmap)
        {
            const SvpSalBitmap& rMask = static_cast<const SvpSalBitmap&>(rAlphaBitmap);
            const BitmapBuffer* pMaskBuf = rMask.GetBuffer();

            if (rAlphaBitmap.GetBitCount() == 8)
            {
                // the alpha values need to be inverted for Cairo
                // so big stupid copy and invert here
                const int nImageSize = pMaskBuf->mnHeight * pMaskBuf->mnScanlineSize;
                pAlphaBits.reset( new unsigned char[nImageSize] );
                memcpy(pAlphaBits.get(), pMaskBuf->mpBits, nImageSize);

                // TODO: make upper layers use standard alpha
                sal_uInt32* pLDst = reinterpret_cast<sal_uInt32*>(pAlphaBits.get());
                for( int i = nImageSize/sizeof(sal_uInt32); --i >= 0; ++pLDst )
                    *pLDst = ~*pLDst;
                assert(reinterpret_cast<unsigned char*>(pLDst) == pAlphaBits.get()+nImageSize);

                implSetSurface(
                    cairo_image_surface_create_for_data(
                        pAlphaBits.get(),
                        CAIRO_FORMAT_A8,
                        pMaskBuf->mnWidth,
                        pMaskBuf->mnHeight,
                        pMaskBuf->mnScanlineSize));
            }
            else
            {
                // the alpha values need to be inverted for Cairo
                // so big stupid copy and invert here
                const int nImageSize = pMaskBuf->mnHeight * pMaskBuf->mnScanlineSize;
                pAlphaBits.reset( new unsigned char[nImageSize] );
                memcpy(pAlphaBits.get(), pMaskBuf->mpBits, nImageSize);

                const sal_Int32 nBlackIndex = pMaskBuf->maPalette.GetBestIndex(BitmapColor(COL_BLACK));
                if (nBlackIndex == 0)
                {
                    // TODO: make upper layers use standard alpha
                    unsigned char* pDst = pAlphaBits.get();
                    for (int i = nImageSize; --i >= 0; ++pDst)
                        *pDst = ~*pDst;
                }

                implSetSurface(
                    cairo_image_surface_create_for_data(
                        pAlphaBits.get(),
                        CAIRO_FORMAT_A1,
                        pMaskBuf->mnWidth,
                        pMaskBuf->mnHeight,
                        pMaskBuf->mnScanlineSize));
            }
        }
    };

    class SystemDependentData_MaskHelper : public basegfx::SystemDependentData
    {
    private:
        std::shared_ptr<MaskHelper>       maMaskHelper;

    public:
        SystemDependentData_MaskHelper(
            basegfx::SystemDependentDataManager& rSystemDependentDataManager,
            const std::shared_ptr<MaskHelper>& rMaskHelper)
        :   basegfx::SystemDependentData(rSystemDependentDataManager),
            maMaskHelper(rMaskHelper)
        {
        }

        const std::shared_ptr<MaskHelper>& getMaskHelper() const { return maMaskHelper; };
        virtual sal_Int64 estimateUsageInBytes() const override;
    };

    sal_Int64 SystemDependentData_MaskHelper::estimateUsageInBytes() const
    {
        return estimateUsageInBytesForSurfaceHelper(maMaskHelper.get());
    }

    // MM02 decide to use buffers or not
    const char* pDisableMM02Goodies(getenv("SAL_DISABLE_MM02_GOODIES"));
    bool bUseBuffer(nullptr == pDisableMM02Goodies);
    const tools::Long nMinimalSquareSizeToBuffer(64*64);

    void tryToUseSourceBuffer(
        const SalBitmap& rSourceBitmap,
        std::shared_ptr<BitmapHelper>& rSurface)
    {
        // MM02 try to access buffered BitmapHelper
        std::shared_ptr<SystemDependentData_BitmapHelper> pSystemDependentData_BitmapHelper;
        const bool bBufferSource(bUseBuffer
            && rSourceBitmap.GetSize().Width() * rSourceBitmap.GetSize().Height() > nMinimalSquareSizeToBuffer);

        if(bBufferSource)
        {
            const SvpSalBitmap& rSrcBmp(static_cast<const SvpSalBitmap&>(rSourceBitmap));
            pSystemDependentData_BitmapHelper = rSrcBmp.getSystemDependentData<SystemDependentData_BitmapHelper>();

            if(pSystemDependentData_BitmapHelper)
            {
                // reuse buffered data
                rSurface = pSystemDependentData_BitmapHelper->getBitmapHelper();
            }
        }

        if(rSurface)
            return;

        // create data on-demand
        rSurface = std::make_shared<BitmapHelper>(rSourceBitmap);

        if(bBufferSource)
        {
            // add to buffering mechanism to potentially reuse next time
            const SvpSalBitmap& rSrcBmp(static_cast<const SvpSalBitmap&>(rSourceBitmap));
            rSrcBmp.addOrReplaceSystemDependentData<SystemDependentData_BitmapHelper>(
                ImplGetSystemDependentDataManager(),
                rSurface);
        }
    }

    void tryToUseMaskBuffer(
        const SalBitmap& rMaskBitmap,
        std::shared_ptr<MaskHelper>& rMask)
    {
        // MM02 try to access buffered MaskHelper
        std::shared_ptr<SystemDependentData_MaskHelper> pSystemDependentData_MaskHelper;
        const bool bBufferMask(bUseBuffer
            && rMaskBitmap.GetSize().Width() * rMaskBitmap.GetSize().Height() > nMinimalSquareSizeToBuffer);

        if(bBufferMask)
        {
            const SvpSalBitmap& rSrcBmp(static_cast<const SvpSalBitmap&>(rMaskBitmap));
            pSystemDependentData_MaskHelper = rSrcBmp.getSystemDependentData<SystemDependentData_MaskHelper>();

            if(pSystemDependentData_MaskHelper)
            {
                // reuse buffered data
                rMask = pSystemDependentData_MaskHelper->getMaskHelper();
            }
        }

        if(rMask)
            return;

        // create data on-demand
        rMask = std::make_shared<MaskHelper>(rMaskBitmap);

        if(bBufferMask)
        {
            // add to buffering mechanism to potentially reuse next time
            const SvpSalBitmap& rSrcBmp(static_cast<const SvpSalBitmap&>(rMaskBitmap));
            rSrcBmp.addOrReplaceSystemDependentData<SystemDependentData_MaskHelper>(
                ImplGetSystemDependentDataManager(),
                rMask);
        }
    }
}

bool SvpSalGraphics::drawAlphaBitmap( const SalTwoRect& rTR, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap )
{
    if (rAlphaBitmap.GetBitCount() != 8 && rAlphaBitmap.GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap alpha depth case: " << rAlphaBitmap.GetBitCount());
        return false;
    }

    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<BitmapHelper> aSurface;
    tryToUseSourceBuffer(rSourceBitmap, aSurface);
    cairo_surface_t* source = aSurface->getSurface(
        rTR.mnDestWidth,
        rTR.mnDestHeight);

    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return false;
    }

    // MM02 try to access buffered MaskHelper
    std::shared_ptr<MaskHelper> aMask;
    tryToUseMaskBuffer(rAlphaBitmap, aMask);
    cairo_surface_t *mask = aMask->getSurface(
        rTR.mnDestWidth,
        rTR.mnDestHeight);

    if (!mask)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return false;
    }

    cairo_t* cr = m_aCairoCommon.getCairoContext(false, getAntiAlias());
    clipRegion(cr);

    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_pattern_t* maskpattern = cairo_pattern_create_for_surface(mask);
    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    double fXScale = static_cast<double>(rTR.mnDestWidth)/rTR.mnSrcWidth;
    double fYScale = static_cast<double>(rTR.mnDestHeight)/rTR.mnSrcHeight;
    cairo_scale(cr, fXScale, fYScale);
    cairo_set_source_surface(cr, source, -rTR.mnSrcX, -rTR.mnSrcY);

    //tdf#114117 when stretching a single pixel width/height source to fit an area
    //set extend and filter to stretch it with simplest expected interpolation
    if ((fXScale != 1.0 && rTR.mnSrcWidth == 1) || (fYScale != 1.0 && rTR.mnSrcHeight == 1))
    {
        cairo_pattern_t* sourcepattern = cairo_get_source(cr);
        cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_REPEAT);
        cairo_pattern_set_filter(sourcepattern, CAIRO_FILTER_NEAREST);
        cairo_pattern_set_extend(maskpattern, CAIRO_EXTEND_REPEAT);
        cairo_pattern_set_filter(maskpattern, CAIRO_FILTER_NEAREST);
    }

    //this block is just "cairo_mask_surface", but we have to make it explicit
    //because of the cairo_pattern_set_filter etc we may want applied
    cairo_matrix_t matrix;
    cairo_matrix_init_translate(&matrix, rTR.mnSrcX, rTR.mnSrcY);
    cairo_pattern_set_matrix(maskpattern, &matrix);
    cairo_mask(cr, maskpattern);

    cairo_pattern_destroy(maskpattern);

    m_aCairoCommon.releaseCairoContext(cr, false, extents);

    return true;
}

bool SvpSalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap,
    double fAlpha)
{
    if (pAlphaBitmap && pAlphaBitmap->GetBitCount() != 8 && pAlphaBitmap->GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap alpha depth case: " << pAlphaBitmap->GetBitCount());
        return false;
    }

    if( fAlpha != 1.0 )
        return false;

    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<BitmapHelper> aSurface;
    tryToUseSourceBuffer(rSourceBitmap, aSurface);
    const tools::Long nDestWidth(basegfx::fround(basegfx::B2DVector(rX - rNull).getLength()));
    const tools::Long nDestHeight(basegfx::fround(basegfx::B2DVector(rY - rNull).getLength()));
    cairo_surface_t* source(
        aSurface->getSurface(
            nDestWidth,
            nDestHeight));

    if(!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap case");
        return false;
    }

    // MM02 try to access buffered MaskHelper
    std::shared_ptr<MaskHelper> aMask;
    if(nullptr != pAlphaBitmap)
    {
        tryToUseMaskBuffer(*pAlphaBitmap, aMask);
    }

    // access cairo_surface_t from MaskHelper
    cairo_surface_t* mask(nullptr);
    if(aMask)
    {
        mask = aMask->getSurface(
            nDestWidth,
            nDestHeight);
    }

    if(nullptr != pAlphaBitmap && nullptr == mask)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap case");
        return false;
    }

    const Size aSize = rSourceBitmap.GetSize();
    cairo_t* cr = m_aCairoCommon.getCairoContext(false, getAntiAlias());
    clipRegion(cr);

    // setup the image transformation
    // using the rNull,rX,rY points as destinations for the (0,0),(0,Width),(Height,0) source points
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix,
                      aXRel.getX()/aSize.Width(), aXRel.getY()/aSize.Width(),
                      aYRel.getX()/aSize.Height(), aYRel.getY()/aSize.Height(),
                      rNull.getX(), rNull.getY());

    cairo_transform(cr, &matrix);

    cairo_rectangle(cr, 0, 0, aSize.Width(), aSize.Height());
    basegfx::B2DRange extents = getClippedFillDamage(cr);
    cairo_clip(cr);

    cairo_set_source_surface(cr, source, 0, 0);
    if (mask)
        cairo_mask_surface(cr, mask, 0, 0);
    else
        cairo_paint(cr);

    m_aCairoCommon.releaseCairoContext(cr, false, extents);

    return true;
}

bool SvpSalGraphics::hasFastDrawTransformedBitmap() const
{
    return false;
}

bool SvpSalGraphics::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, sal_uInt8 nTransparency)
{
    const bool bHasFill(m_aCairoCommon.m_aFillColor != SALCOLOR_NONE);
    const bool bHasLine(m_aCairoCommon.m_aLineColor != SALCOLOR_NONE);

    if(!(bHasFill || bHasLine))
    {
        return true;
    }

    cairo_t* cr = m_aCairoCommon.getCairoContext(false, getAntiAlias());
    clipRegion(cr);

    const double fTransparency = nTransparency * (1.0/100);

    // To make releaseCairoContext work, use empty extents
    basegfx::B2DRange extents;

    if (bHasFill)
    {
        cairo_rectangle(cr, nX, nY, nWidth, nHeight);

        m_aCairoCommon.applyColor(cr, m_aCairoCommon.m_aFillColor, fTransparency);

        // set FillDamage
        extents = getClippedFillDamage(cr);

        cairo_fill(cr);
    }

    if (bHasLine)
    {
        // PixelOffset used: Set PixelOffset as linear transformation
        // Note: Was missing here - probably not by purpose (?)
        cairo_matrix_t aMatrix;
        cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
        cairo_set_matrix(cr, &aMatrix);

        cairo_rectangle(cr, nX, nY, nWidth, nHeight);

        m_aCairoCommon.applyColor(cr, m_aCairoCommon.m_aLineColor, fTransparency);

        // expand with possible StrokeDamage
        basegfx::B2DRange stroke_extents = getClippedStrokeDamage(cr);
        stroke_extents.transform(basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5));
        extents.expand(stroke_extents);

        cairo_stroke(cr);
    }

    m_aCairoCommon.releaseCairoContext(cr, false, extents);

    return true;
}

SvpSalGraphics::SvpSalGraphics()
    : m_aTextRenderImpl(*this)
    , m_pBackend(new SvpGraphicsBackend(m_aCairoCommon))
{
    bool bLOKActive = comphelper::LibreOfficeKit::isActive();
    initWidgetDrawBackends(bLOKActive);
}

SvpSalGraphics::~SvpSalGraphics()
{
    ReleaseFonts();
}

void SvpSalGraphics::setSurface(cairo_surface_t* pSurface, const basegfx::B2IVector& rSize)
{
    m_aCairoCommon.m_pSurface = pSurface;
    m_aCairoCommon.m_aFrameSize = rSize;
    dl_cairo_surface_get_device_scale(pSurface, &m_aCairoCommon.m_fScale, nullptr);
    ResetClipRegion();
}

void SvpSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    rDPIX = rDPIY = 96;
}

void SvpSalGraphics::copyArea( tools::Long nDestX,
                               tools::Long nDestY,
                               tools::Long nSrcX,
                               tools::Long nSrcY,
                               tools::Long nSrcWidth,
                               tools::Long nSrcHeight,
                               bool /*bWindowInvalidate*/ )
{
    SalTwoRect aTR(nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);
    copyBits(aTR, this);
}

static basegfx::B2DRange renderWithOperator(cairo_t* cr, const SalTwoRect& rTR,
                                          cairo_surface_t* source, cairo_operator_t eOperator = CAIRO_OPERATOR_SOURCE)
{
    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    double fXScale = 1.0f;
    double fYScale = 1.0f;
    if (rTR.mnSrcWidth != 0 && rTR.mnSrcHeight != 0) {
        fXScale = static_cast<double>(rTR.mnDestWidth)/rTR.mnSrcWidth;
        fYScale = static_cast<double>(rTR.mnDestHeight)/rTR.mnSrcHeight;
        cairo_scale(cr, fXScale, fYScale);
    }

    cairo_save(cr);
    cairo_set_source_surface(cr, source, -rTR.mnSrcX, -rTR.mnSrcY);
    if ((fXScale != 1.0 && rTR.mnSrcWidth == 1) || (fYScale != 1.0 && rTR.mnSrcHeight == 1))
    {
        cairo_pattern_t* sourcepattern = cairo_get_source(cr);
        cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_REPEAT);
        cairo_pattern_set_filter(sourcepattern, CAIRO_FILTER_NEAREST);
    }
    cairo_set_operator(cr, eOperator);
    cairo_paint(cr);
    cairo_restore(cr);

    return extents;
}

static basegfx::B2DRange renderSource(cairo_t* cr, const SalTwoRect& rTR,
                                          cairo_surface_t* source)
{
    return renderWithOperator(cr, rTR, source, CAIRO_OPERATOR_SOURCE);
}

void SvpSalGraphics::copyWithOperator( const SalTwoRect& rTR, cairo_surface_t* source,
                                 cairo_operator_t eOp )
{
    cairo_t* cr = m_aCairoCommon.getCairoContext(false, getAntiAlias());
    clipRegion(cr);

    basegfx::B2DRange extents = renderWithOperator(cr, rTR, source, eOp);

    m_aCairoCommon.releaseCairoContext(cr, false, extents);
}

void SvpSalGraphics::copySource( const SalTwoRect& rTR, cairo_surface_t* source )
{
   copyWithOperator(rTR, source, CAIRO_OPERATOR_SOURCE);
}

void SvpSalGraphics::copyBits( const SalTwoRect& rTR,
                               SalGraphics*      pSrcGraphics )
{
    SalTwoRect aTR(rTR);

    SvpSalGraphics* pSrc = pSrcGraphics ?
        static_cast<SvpSalGraphics*>(pSrcGraphics) : this;

    cairo_surface_t* source = pSrc->m_aCairoCommon.m_pSurface;

    cairo_surface_t *pCopy = nullptr;
    if (pSrc == this)
    {
        //self copy is a problem, so dup source in that case
        pCopy = cairo_surface_create_similar(source,
                                            cairo_surface_get_content(m_aCairoCommon.m_pSurface),
                                            aTR.mnSrcWidth * m_aCairoCommon.m_fScale,
                                            aTR.mnSrcHeight * m_aCairoCommon.m_fScale);
        dl_cairo_surface_set_device_scale(pCopy, m_aCairoCommon.m_fScale, m_aCairoCommon.m_fScale);
        cairo_t* cr = cairo_create(pCopy);
        cairo_set_source_surface(cr, source, -aTR.mnSrcX, -aTR.mnSrcY);
        cairo_rectangle(cr, 0, 0, aTR.mnSrcWidth, aTR.mnSrcHeight);
        cairo_fill(cr);
        cairo_destroy(cr);

        source = pCopy;

        aTR.mnSrcX = 0;
        aTR.mnSrcY = 0;
    }

    copySource(aTR, source);

    if (pCopy)
        cairo_surface_destroy(pCopy);
}

void SvpSalGraphics::drawBitmap(const SalTwoRect& rTR, const SalBitmap& rSourceBitmap)
{
    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<BitmapHelper> aSurface;
    tryToUseSourceBuffer(rSourceBitmap, aSurface);
    cairo_surface_t* source = aSurface->getSurface(
        rTR.mnDestWidth,
        rTR.mnDestHeight);

    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return;
    }

#if 0 // LO code is not yet bitmap32-ready.
      // if m_bSupportsBitmap32 becomes true for Svp revisit this
    copyWithOperator(rTR, source, CAIRO_OPERATOR_OVER);
#else
    copyWithOperator(rTR, source, CAIRO_OPERATOR_SOURCE);
#endif
}

void SvpSalGraphics::drawBitmap(const SalTwoRect& rTR, const BitmapBuffer* pBuffer, cairo_operator_t eOp)
{
    cairo_surface_t* source = createCairoSurface( pBuffer );
    copyWithOperator(rTR, source, eOp);
    cairo_surface_destroy(source);
}

void SvpSalGraphics::drawBitmap( const SalTwoRect& rTR,
                                 const SalBitmap& rSourceBitmap,
                                 const SalBitmap& rTransparentBitmap )
{
    drawAlphaBitmap(rTR, rSourceBitmap, rTransparentBitmap);
}

void SvpSalGraphics::drawMask( const SalTwoRect& rTR,
                               const SalBitmap& rSalBitmap,
                               Color nMaskColor )
{
    /** creates an image from the given rectangle, replacing all black pixels
     *  with nMaskColor and make all other full transparent */
    // MM02 here decided *against* using buffered BitmapHelper
    // because the data gets somehow 'unmuliplied'. This may also be
    // done just once, but I am not sure if this is safe to do.
    // So for now dispense re-using data here.
    BitmapHelper aSurface(rSalBitmap, true); // The mask is argb32
    if (!aSurface.getSurface())
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawMask case");
        return;
    }
    sal_Int32 nStride;
    unsigned char *mask_data = aSurface.getBits(nStride);
    vcl::bitmap::lookup_table const & unpremultiply_table = vcl::bitmap::get_unpremultiply_table();
    for (tools::Long y = rTR.mnSrcY ; y < rTR.mnSrcY + rTR.mnSrcHeight; ++y)
    {
        unsigned char *row = mask_data + (nStride*y);
        unsigned char *data = row + (rTR.mnSrcX * 4);
        for (tools::Long x = rTR.mnSrcX; x < rTR.mnSrcX + rTR.mnSrcWidth; ++x)
        {
            sal_uInt8 a = data[SVP_CAIRO_ALPHA];
            sal_uInt8 b = unpremultiply_table[a][data[SVP_CAIRO_BLUE]];
            sal_uInt8 g = unpremultiply_table[a][data[SVP_CAIRO_GREEN]];
            sal_uInt8 r = unpremultiply_table[a][data[SVP_CAIRO_RED]];
            if (r == 0 && g == 0 && b == 0)
            {
                data[0] = nMaskColor.GetBlue();
                data[1] = nMaskColor.GetGreen();
                data[2] = nMaskColor.GetRed();
                data[3] = 0xff;
            }
            else
            {
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
                data[3] = 0;
            }
            data+=4;
        }
    }
    aSurface.mark_dirty();

    cairo_t* cr = m_aCairoCommon.getCairoContext(false, getAntiAlias());
    clipRegion(cr);

    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    double fXScale = static_cast<double>(rTR.mnDestWidth)/rTR.mnSrcWidth;
    double fYScale = static_cast<double>(rTR.mnDestHeight)/rTR.mnSrcHeight;
    cairo_scale(cr, fXScale, fYScale);
    cairo_set_source_surface(cr, aSurface.getSurface(), -rTR.mnSrcX, -rTR.mnSrcY);
    if ((fXScale != 1.0 && rTR.mnSrcWidth == 1) || (fYScale != 1.0 && rTR.mnSrcHeight == 1))
    {
        cairo_pattern_t* sourcepattern = cairo_get_source(cr);
        cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_REPEAT);
        cairo_pattern_set_filter(sourcepattern, CAIRO_FILTER_NEAREST);
    }
    cairo_paint(cr);

    m_aCairoCommon.releaseCairoContext(cr, false, extents);
}

std::shared_ptr<SalBitmap> SvpSalGraphics::getBitmap( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
{
    std::shared_ptr<SvpSalBitmap> pBitmap = std::make_shared<SvpSalBitmap>();
    BitmapPalette aPal;
    vcl::PixelFormat ePixelFormat = vcl::PixelFormat::INVALID;
    if (GetBitCount() == 1)
    {
        ePixelFormat = vcl::PixelFormat::N1_BPP;
        aPal.SetEntryCount(2);
        aPal[0] = COL_BLACK;
        aPal[1] = COL_WHITE;
    }
    else
    {
        ePixelFormat = vcl::PixelFormat::N32_BPP;
    }

    if (!pBitmap->Create(Size(nWidth, nHeight), ePixelFormat, aPal))
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::getBitmap, cannot create bitmap");
        return nullptr;
    }

    cairo_surface_t* target = SvpSalGraphics::createCairoSurface(pBitmap->GetBuffer());
    if (!target)
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::getBitmap, cannot create cairo surface");
        return nullptr;
    }
    cairo_t* cr = cairo_create(target);

    SalTwoRect aTR(nX, nY, nWidth, nHeight, 0, 0, nWidth, nHeight);
    renderSource(cr, aTR, m_aCairoCommon.m_pSurface);

    cairo_destroy(cr);
    cairo_surface_destroy(target);

    Toggle1BitTransparency(*pBitmap->GetBuffer());

    return pBitmap;
}

namespace
{
    bool isCairoCompatible(const BitmapBuffer* pBuffer)
    {
        if (!pBuffer)
            return false;

        // We use Cairo that supports 24-bit RGB.
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
        if (pBuffer->mnBitCount != 32 && pBuffer->mnBitCount != 24 && pBuffer->mnBitCount != 1)
#else
        if (pBuffer->mnBitCount != 32 && pBuffer->mnBitCount != 1)
#endif
            return false;

        cairo_format_t nFormat = getCairoFormat(*pBuffer);
        return (cairo_format_stride_for_width(nFormat, pBuffer->mnWidth) == pBuffer->mnScanlineSize);
    }
}

cairo_surface_t* SvpSalGraphics::createCairoSurface(const BitmapBuffer *pBuffer)
{
    if (!isCairoCompatible(pBuffer))
        return nullptr;

    cairo_format_t nFormat = getCairoFormat(*pBuffer);
    cairo_surface_t *target =
        cairo_image_surface_create_for_data(pBuffer->mpBits,
                                        nFormat,
                                        pBuffer->mnWidth, pBuffer->mnHeight,
                                        pBuffer->mnScanlineSize);
    if (cairo_surface_status(target) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(target);
        return nullptr;
    }
    return target;
}

#if ENABLE_CAIRO_CANVAS
bool SvpSalGraphics::SupportsCairo() const
{
    return false;
}

cairo::SurfaceSharedPtr SvpSalGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& /*rSurface*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr SvpSalGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int /*x*/, int /*y*/, int /*width*/, int /*height*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr SvpSalGraphics::CreateBitmapSurface(const OutputDevice& /*rRefDevice*/, const BitmapSystemData& /*rData*/, const Size& /*rSize*/) const
{
    return cairo::SurfaceSharedPtr();
}

css::uno::Any SvpSalGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& /*rSurface*/, const basegfx::B2ISize& /*rSize*/) const
{
    return css::uno::Any();
}

#endif // ENABLE_CAIRO_CANVAS

SystemGraphicsData SvpSalGraphics::GetGraphicsData() const
{
    return SystemGraphicsData();
}

bool SvpSalGraphics::supportsOperation(OutDevSupportType eType) const
{
    switch (eType)
    {
        case OutDevSupportType::TransparentRect:
        case OutDevSupportType::B2DDraw:
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
