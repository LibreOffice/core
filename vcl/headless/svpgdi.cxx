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

namespace
{
    basegfx::B2DRange getClipBox(cairo_t* cr)
    {
        double x1, y1, x2, y2;

        cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

        // support B2DRange::isEmpty()
        if(0.0 != x1 || 0.0 != y1 || 0.0 != x2 || 0.0 != y2)
        {
            return basegfx::B2DRange(x1, y1, x2, y2);
        }

        return basegfx::B2DRange();
    }

    basegfx::B2DRange getFillDamage(cairo_t* cr)
    {
        double x1, y1, x2, y2;

        // this is faster than cairo_fill_extents, at the cost of some overdraw
        cairo_path_extents(cr, &x1, &y1, &x2, &y2);

        // support B2DRange::isEmpty()
        if(0.0 != x1 || 0.0 != y1 || 0.0 != x2 || 0.0 != y2)
        {
            return basegfx::B2DRange(x1, y1, x2, y2);
        }

        return basegfx::B2DRange();
    }

    basegfx::B2DRange getClippedFillDamage(cairo_t* cr)
    {
        basegfx::B2DRange aDamageRect(getFillDamage(cr));
        aDamageRect.intersect(getClipBox(cr));
        return aDamageRect;
    }

    basegfx::B2DRange getStrokeDamage(cairo_t* cr)
    {
        double x1, y1, x2, y2;

        // less accurate, but much faster
        cairo_path_extents(cr, &x1, &y1, &x2, &y2);

        // support B2DRange::isEmpty()
        if(0.0 != x1 || 0.0 != y1 || 0.0 != x2 || 0.0 != y2)
        {
            return basegfx::B2DRange(x1, y1, x2, y2);
        }

        return basegfx::B2DRange();
    }

    basegfx::B2DRange getClippedStrokeDamage(cairo_t* cr)
    {
        basegfx::B2DRange aDamageRect(getStrokeDamage(cr));
        aDamageRect.intersect(getClipBox(cr));
        return aDamageRect;
    }
}

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
                *pDst = *pDst;
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
                    *pLDst = *pLDst;
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
                        *pDst = *pDst;
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

    cairo_t* cr = getCairoContext(false);
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

    releaseCairoContext(cr, false, extents);

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
    cairo_t* cr = getCairoContext(false);
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

    releaseCairoContext(cr, false, extents);

    return true;
}

bool SvpSalGraphics::hasFastDrawTransformedBitmap() const
{
    return false;
}

void SvpSalGraphics::clipRegion(cairo_t* cr, const vcl::Region& rClipRegion)
{
    RectangleVector aRectangles;
    if (!rClipRegion.IsEmpty())
    {
        rClipRegion.GetRegionRectangles(aRectangles);
    }
    if (!aRectangles.empty())
    {
        for (auto const& rectangle : aRectangles)
        {
            cairo_rectangle(cr, rectangle.Left(), rectangle.Top(), rectangle.GetWidth(), rectangle.GetHeight());
        }
        cairo_clip(cr);
    }
}

void SvpSalGraphics::clipRegion(cairo_t* cr)
{
    SvpSalGraphics::clipRegion(cr, m_aClipRegion);
}

bool SvpSalGraphics::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, sal_uInt8 nTransparency)
{
    const bool bHasFill(m_aFillColor != SALCOLOR_NONE);
    const bool bHasLine(m_aLineColor != SALCOLOR_NONE);

    if(!(bHasFill || bHasLine))
    {
        return true;
    }

    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    const double fTransparency = nTransparency * (1.0/100);

    // To make releaseCairoContext work, use empty extents
    basegfx::B2DRange extents;

    if (bHasFill)
    {
        cairo_rectangle(cr, nX, nY, nWidth, nHeight);

        applyColor(cr, m_aFillColor, fTransparency);

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

        applyColor(cr, m_aLineColor, fTransparency);

        // expand with possible StrokeDamage
        basegfx::B2DRange stroke_extents = getClippedStrokeDamage(cr);
        stroke_extents.transform(basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5));
        extents.expand(stroke_extents);

        cairo_stroke(cr);
    }

    releaseCairoContext(cr, false, extents);

    return true;
}

SvpSalGraphics::SvpSalGraphics()
    : m_pSurface(nullptr)
    , m_fScale(1.0)
    , m_aLineColor(Color(0x00, 0x00, 0x00))
    , m_aFillColor(Color(0xFF, 0xFF, 0XFF))
    , m_ePaintMode(PaintMode::Over)
    , m_aTextRenderImpl(*this)
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
    m_pSurface = pSurface;
    m_aFrameSize = rSize;
    dl_cairo_surface_get_device_scale(pSurface, &m_fScale, nullptr);
    ResetClipRegion();
}

void SvpSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    rDPIX = rDPIY = 96;
}

sal_uInt16 SvpSalGraphics::GetBitCount() const
{
    if (cairo_surface_get_content(m_pSurface) != CAIRO_CONTENT_COLOR_ALPHA)
        return 1;
    return 32;
}

tools::Long SvpSalGraphics::GetGraphicsWidth() const
{
    return m_pSurface ? m_aFrameSize.getX() : 0;
}

void SvpSalGraphics::ResetClipRegion()
{
    m_aClipRegion.SetNull();
}

bool SvpSalGraphics::setClipRegion( const vcl::Region& i_rClip )
{
    m_aClipRegion = i_rClip;
    return true;
}

void SvpSalGraphics::SetLineColor()
{
    m_aLineColor = SALCOLOR_NONE;
}

void SvpSalGraphics::SetLineColor( Color nColor )
{
    m_aLineColor = nColor;
}

void SvpSalGraphics::SetFillColor()
{
    m_aFillColor = SALCOLOR_NONE;
}

void SvpSalGraphics::SetFillColor( Color nColor )
{
    m_aFillColor = nColor;
}

void SvpSalGraphics::SetXORMode(bool bSet, bool )
{
    m_ePaintMode = bSet ? PaintMode::Xor : PaintMode::Over;
}

void SvpSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SalROPColor::N0:
            m_aLineColor = Color(0, 0, 0);
            break;
        case SalROPColor::N1:
            m_aLineColor = Color(0xff, 0xff, 0xff);
            break;
        case SalROPColor::Invert:
            m_aLineColor = Color(0xff, 0xff, 0xff);
            break;
    }
}

void SvpSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SalROPColor::N0:
            m_aFillColor = Color(0, 0, 0);
            break;
        case SalROPColor::N1:
            m_aFillColor = Color(0xff, 0xff, 0xff);
            break;
        case SalROPColor::Invert:
            m_aFillColor = Color(0xff, 0xff, 0xff);
            break;
    }
}

void SvpSalGraphics::drawPixel( tools::Long nX, tools::Long nY )
{
    if (m_aLineColor != SALCOLOR_NONE)
    {
        drawPixel(nX, nY, m_aLineColor);
    }
}

void SvpSalGraphics::drawPixel( tools::Long nX, tools::Long nY, Color aColor )
{
    cairo_t* cr = getCairoContext(true);
    clipRegion(cr);

    cairo_rectangle(cr, nX, nY, 1, 1);
    applyColor(cr, aColor, 0.0);
    cairo_fill(cr);

    basegfx::B2DRange extents = getClippedFillDamage(cr);
    releaseCairoContext(cr, true, extents);
}

void SvpSalGraphics::drawRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
{
    // because of the -1 hack we have to do fill and draw separately
    Color aOrigFillColor = m_aFillColor;
    Color aOrigLineColor = m_aLineColor;
    m_aFillColor = SALCOLOR_NONE;
    m_aLineColor = SALCOLOR_NONE;

    if (aOrigFillColor != SALCOLOR_NONE)
    {
        basegfx::B2DPolygon aRect = basegfx::utils::createPolygonFromRect(basegfx::B2DRectangle(nX, nY, nX+nWidth, nY+nHeight));
        m_aFillColor = aOrigFillColor;

        drawPolyPolygon(
            basegfx::B2DHomMatrix(),
            basegfx::B2DPolyPolygon(aRect),
            0.0);

        m_aFillColor = SALCOLOR_NONE;
    }

    if (aOrigLineColor != SALCOLOR_NONE)
    {
        // need same -1 hack as X11SalGraphicsImpl::drawRect
        basegfx::B2DPolygon aRect = basegfx::utils::createPolygonFromRect(basegfx::B2DRectangle( nX, nY, nX+nWidth-1, nY+nHeight-1));
        m_aLineColor = aOrigLineColor;

        drawPolyPolygon(
            basegfx::B2DHomMatrix(),
            basegfx::B2DPolyPolygon(aRect),
            0.0);

        m_aLineColor = SALCOLOR_NONE;
    }

    m_aFillColor = aOrigFillColor;
    m_aLineColor = aOrigLineColor;
}

void SvpSalGraphics::drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));
    aPoly.setClosed(false);

    drawPolyLine(
        basegfx::B2DHomMatrix(),
        aPoly,
        0.0,
        1.0,
        nullptr, // MM01
        basegfx::B2DLineJoin::Miter,
        css::drawing::LineCap_BUTT,
        basegfx::deg2rad(15.0) /*default*/,
        false);
}

void SvpSalGraphics::drawPolygon(sal_uInt32 nPoints, const Point* pPtAry)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));

    drawPolyPolygon(
        basegfx::B2DHomMatrix(),
        basegfx::B2DPolyPolygon(aPoly),
        0.0);
}

void SvpSalGraphics::drawPolyPolygon(sal_uInt32 nPoly,
                                     const sal_uInt32* pPointCounts,
                                     const Point**   pPtAry)
{
    basegfx::B2DPolyPolygon aPolyPoly;
    for(sal_uInt32 nPolygon = 0; nPolygon < nPoly; ++nPolygon)
    {
        sal_uInt32 nPoints = pPointCounts[nPolygon];
        if (nPoints)
        {
            const Point* pPoints = pPtAry[nPolygon];
            basegfx::B2DPolygon aPoly;
            aPoly.append( basegfx::B2DPoint(pPoints->getX(), pPoints->getY()), nPoints);
            for (sal_uInt32 i = 1; i < nPoints; ++i)
                aPoly.setB2DPoint(i, basegfx::B2DPoint( pPoints[i].getX(), pPoints[i].getY()));

            aPolyPoly.append(aPoly);
        }
    }

    drawPolyPolygon(
        basegfx::B2DHomMatrix(),
        aPolyPoly,
        0.0);
}

static basegfx::B2DPoint impPixelSnap(
    const basegfx::B2DPolygon& rPolygon,
    const basegfx::B2DHomMatrix& rObjectToDevice,
    basegfx::B2DHomMatrix& rObjectToDeviceInv,
    sal_uInt32 nIndex)
{
    const sal_uInt32 nCount(rPolygon.count());

    // get the data
    const basegfx::B2ITuple aPrevTuple(basegfx::fround(rObjectToDevice * rPolygon.getB2DPoint((nIndex + nCount - 1) % nCount)));
    const basegfx::B2DPoint aCurrPoint(rObjectToDevice * rPolygon.getB2DPoint(nIndex));
    const basegfx::B2ITuple aCurrTuple(basegfx::fround(aCurrPoint));
    const basegfx::B2ITuple aNextTuple(basegfx::fround(rObjectToDevice * rPolygon.getB2DPoint((nIndex + 1) % nCount)));

    // get the states
    const bool bPrevVertical(aPrevTuple.getX() == aCurrTuple.getX());
    const bool bNextVertical(aNextTuple.getX() == aCurrTuple.getX());
    const bool bPrevHorizontal(aPrevTuple.getY() == aCurrTuple.getY());
    const bool bNextHorizontal(aNextTuple.getY() == aCurrTuple.getY());
    const bool bSnapX(bPrevVertical || bNextVertical);
    const bool bSnapY(bPrevHorizontal || bNextHorizontal);

    if(bSnapX || bSnapY)
    {
        basegfx::B2DPoint aSnappedPoint(
            bSnapX ? aCurrTuple.getX() : aCurrPoint.getX(),
            bSnapY ? aCurrTuple.getY() : aCurrPoint.getY());

        if(rObjectToDeviceInv.isIdentity())
        {
            rObjectToDeviceInv = rObjectToDevice;
            rObjectToDeviceInv.invert();
        }

        aSnappedPoint *= rObjectToDeviceInv;

        return aSnappedPoint;
    }

    return rPolygon.getB2DPoint(nIndex);
}

// Remove bClosePath: Checked that the already used mechanism for Win using
// Gdiplus already relies on rPolygon.isClosed(), so should be safe to replace
// this.
// For PixelSnap we need the ObjectToDevice transformation here now. This is a
// special case relative to the also executed LineDraw-Offset of (0.5, 0.5) in
// DeviceCoordinates: The LineDraw-Offset is applied *after* the snap, so we
// need the ObjectToDevice transformation *without* that offset here to do the
// same. The LineDraw-Offset will be applied by the callers using a linear
// transformation for Cairo now
// For support of PixelSnapHairline we also need the ObjectToDevice transformation
// and a method (same as in gdiimpl.cxx for Win and Gdiplus). This is needed e.g.
// for Chart-content visualization. CAUTION: It's not the same as PixelSnap (!)
// tdf#129845 add reply value to allow counting a point/byte/size measurement to
// be included
static size_t AddPolygonToPath(
    cairo_t* cr,
    const basegfx::B2DPolygon& rPolygon,
    const basegfx::B2DHomMatrix& rObjectToDevice,
    bool bPixelSnap,
    bool bPixelSnapHairline)
{
    // short circuit if there is nothing to do
    const sal_uInt32 nPointCount(rPolygon.count());
    size_t nSizeMeasure(0);

    if(0 == nPointCount)
    {
        return nSizeMeasure;
    }

    const bool bHasCurves(rPolygon.areControlPointsUsed());
    const bool bClosePath(rPolygon.isClosed());
    const bool bObjectToDeviceUsed(!rObjectToDevice.isIdentity());
    basegfx::B2DHomMatrix aObjectToDeviceInv;
    basegfx::B2DPoint aLast;

    for( sal_uInt32 nPointIdx = 0, nPrevIdx = 0;; nPrevIdx = nPointIdx++ )
    {
        int nClosedIdx = nPointIdx;
        if( nPointIdx >= nPointCount )
        {
            // prepare to close last curve segment if needed
            if( bClosePath && (nPointIdx == nPointCount) )
            {
                nClosedIdx = 0;
            }
            else
            {
                break;
            }
        }

        basegfx::B2DPoint aPoint(rPolygon.getB2DPoint(nClosedIdx));

        if(bPixelSnap)
        {
            // snap device coordinates to full pixels
            if(bObjectToDeviceUsed)
            {
                // go to DeviceCoordinates
                aPoint *= rObjectToDevice;
            }

            // snap by rounding
            aPoint.setX( basegfx::fround( aPoint.getX() ) );
            aPoint.setY( basegfx::fround( aPoint.getY() ) );

            if(bObjectToDeviceUsed)
            {
                if(aObjectToDeviceInv.isIdentity())
                {
                    aObjectToDeviceInv = rObjectToDevice;
                    aObjectToDeviceInv.invert();
                }

                // go back to ObjectCoordinates
                aPoint *= aObjectToDeviceInv;
            }
        }

        if(bPixelSnapHairline)
        {
            // snap horizontal and vertical lines (mainly used in Chart for
            // 'nicer' AAing)
            aPoint = impPixelSnap(rPolygon, rObjectToDevice, aObjectToDeviceInv, nClosedIdx);
        }

        if( !nPointIdx )
        {
            // first point => just move there
            cairo_move_to(cr, aPoint.getX(), aPoint.getY());
            aLast = aPoint;
            continue;
        }

        bool bPendingCurve(false);

        if( bHasCurves )
        {
            bPendingCurve = rPolygon.isNextControlPointUsed( nPrevIdx );
            bPendingCurve |= rPolygon.isPrevControlPointUsed( nClosedIdx );
        }

        if( !bPendingCurve )    // line segment
        {
            cairo_line_to(cr, aPoint.getX(), aPoint.getY());
            nSizeMeasure++;
        }
        else                        // cubic bezier segment
        {
            basegfx::B2DPoint aCP1 = rPolygon.getNextControlPoint( nPrevIdx );
            basegfx::B2DPoint aCP2 = rPolygon.getPrevControlPoint( nClosedIdx );

            // tdf#99165 if the control points are 'empty', create the mathematical
            // correct replacement ones to avoid problems with the graphical sub-system
            // tdf#101026 The 1st attempt to create a mathematically correct replacement control
            // vector was wrong. Best alternative is one as close as possible which means short.
            if (aCP1.equal(aLast))
            {
                aCP1 = aLast + ((aCP2 - aLast) * 0.0005);
            }

            if(aCP2.equal(aPoint))
            {
                aCP2 = aPoint + ((aCP1 - aPoint) * 0.0005);
            }

            cairo_curve_to(cr, aCP1.getX(), aCP1.getY(), aCP2.getX(), aCP2.getY(),
                               aPoint.getX(), aPoint.getY());
            // take some bigger measure for curve segments - too expensive to subdivide
            // here and that precision not needed, but four (2 points, 2 control-points)
            // would be a too low weight
            nSizeMeasure += 10;
        }

        aLast = aPoint;
    }

    if( bClosePath )
    {
        cairo_close_path(cr);
    }

    return nSizeMeasure;
}

void SvpSalGraphics::drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 )
{
    basegfx::B2DPolygon aPoly;

    // PixelOffset used: To not mix with possible PixelSnap, cannot do
    // directly on coordinates as tried before - despite being already 'snapped'
    // due to being integer. If it would be directly added here, it would be
    // 'snapped' again when !getAntiAlias(), losing the (0.5, 0.5) offset
    aPoly.append(basegfx::B2DPoint(nX1, nY1));
    aPoly.append(basegfx::B2DPoint(nX2, nY2));

    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    // PixelOffset used: Set PixelOffset as linear transformation
    cairo_matrix_t aMatrix;
    cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
    cairo_set_matrix(cr, &aMatrix);

    AddPolygonToPath(
        cr,
        aPoly,
        basegfx::B2DHomMatrix(),
        !getAntiAlias(),
        false);

    applyColor(cr, m_aLineColor);

    basegfx::B2DRange extents = getClippedStrokeDamage(cr);
    extents.transform(basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5));

    cairo_stroke(cr);

    releaseCairoContext(cr, false, extents);
}

namespace {

class SystemDependentData_CairoPath : public basegfx::SystemDependentData
{
private:
    // the path data itself
    cairo_path_t*       mpCairoPath;

    // all other values the path data  is based on and
    // need to be compared with to check for data validity
    bool                mbNoJoin;
    bool                mbAntiAlias;
    std::vector< double >                       maStroke;

public:
    SystemDependentData_CairoPath(
        basegfx::SystemDependentDataManager& rSystemDependentDataManager,
        size_t nSizeMeasure,
        cairo_t* cr,
        bool bNoJoin,
        bool bAntiAlias,
        const std::vector< double >* pStroke); // MM01
    virtual ~SystemDependentData_CairoPath() override;

    // read access
    cairo_path_t* getCairoPath() { return mpCairoPath; }
    bool getNoJoin() const { return mbNoJoin; }
    bool getAntiAlias() const { return mbAntiAlias; }
    const std::vector< double >& getStroke() const { return maStroke; }

    virtual sal_Int64 estimateUsageInBytes() const override;
};

}

SystemDependentData_CairoPath::SystemDependentData_CairoPath(
    basegfx::SystemDependentDataManager& rSystemDependentDataManager,
    size_t nSizeMeasure,
    cairo_t* cr,
    bool bNoJoin,
    bool bAntiAlias,
    const std::vector< double >* pStroke)
:   basegfx::SystemDependentData(rSystemDependentDataManager),
    mpCairoPath(nullptr),
    mbNoJoin(bNoJoin),
    mbAntiAlias(bAntiAlias)
{
    // tdf#129845 only create a copy of the path when nSizeMeasure is
    // bigger than some decent threshold
    if(nSizeMeasure > 50)
    {
        mpCairoPath = cairo_copy_path(cr);

        if(nullptr != pStroke)
        {
            maStroke = *pStroke;
        }
    }
}

SystemDependentData_CairoPath::~SystemDependentData_CairoPath()
{
    if(nullptr != mpCairoPath)
    {
        cairo_path_destroy(mpCairoPath);
        mpCairoPath = nullptr;
    }
}

sal_Int64 SystemDependentData_CairoPath::estimateUsageInBytes() const
{
    // tdf#129845 by using the default return value of zero when no path
    // was created, SystemDependentData::calculateCombinedHoldCyclesInSeconds
    // will do the right thing and not buffer this entry at all
    sal_Int64 nRetval(0);

    if(nullptr != mpCairoPath)
    {
        // per node
        // - num_data incarnations of
        // - sizeof(cairo_path_data_t) which is a union of defines and point data
        //   thus may 2 x sizeof(double)
        nRetval = mpCairoPath->num_data * sizeof(cairo_path_data_t);
    }

    return nRetval;
}

bool SvpSalGraphics::drawPolyLine(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolygon& rPolyLine,
    double fTransparency,
    double fLineWidth,
    const std::vector< double >* pStroke, // MM01
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap,
    double fMiterMinimumAngle,
    bool bPixelSnapHairline)
{
    // short circuit if there is nothing to do
    if(0 == rPolyLine.count() || fTransparency < 0.0 || fTransparency >= 1.0)
    {
        return true;
    }

    // Wrap call to static version of ::drawPolyLine by
    // preparing/getting some local data and parameters
    // due to usage in vcl/unx/generic/gdi/salgdi.cxx.
    // This is mainly about extended handling of extents
    // and the way destruction of CairoContext is handled
    // due to current XOR stuff
    cairo_t* cr = getCairoContext(false);
    basegfx::B2DRange aExtents;
    clipRegion(cr);

    bool bRetval(
        drawPolyLine(
            cr,
            &aExtents,
            m_aLineColor,
            getAntiAlias(),
            rObjectToDevice,
            rPolyLine,
            fTransparency,
            fLineWidth,
            pStroke, // MM01
            eLineJoin,
            eLineCap,
            fMiterMinimumAngle,
            bPixelSnapHairline));

    releaseCairoContext(cr, false, aExtents);

    return bRetval;
}

bool SvpSalGraphics::drawPolyLine(
    cairo_t* cr,
    basegfx::B2DRange* pExtents,
    const Color& rLineColor,
    bool bAntiAlias,
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolygon& rPolyLine,
    double fTransparency,
    double fLineWidth,
    const std::vector< double >* pStroke, // MM01
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap,
    double fMiterMinimumAngle,
    bool bPixelSnapHairline)
{
    // short circuit if there is nothing to do
    if(0 == rPolyLine.count() || fTransparency < 0.0 || fTransparency >= 1.0)
    {
        return true;
    }

    // need to check/handle LineWidth when ObjectToDevice transformation is used
    const bool bObjectToDeviceIsIdentity(rObjectToDevice.isIdentity());

    // tdf#124848 calculate-back logical LineWidth for a hairline
    // since this implementation hands over the transformation to
    // the graphic sub-system
    if(fLineWidth == 0)
    {
        fLineWidth = 1.0;

        if(!bObjectToDeviceIsIdentity)
        {
            basegfx::B2DHomMatrix aObjectToDeviceInv(rObjectToDevice);
            aObjectToDeviceInv.invert();
            fLineWidth = (aObjectToDeviceInv * basegfx::B2DVector(fLineWidth, 0)).getLength();
        }
    }

    // PixelOffset used: Need to reflect in linear transformation
    cairo_matrix_t aMatrix;
    basegfx::B2DHomMatrix aDamageMatrix(basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5));

    if (bObjectToDeviceIsIdentity)
    {
        // Set PixelOffset as requested
        cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
    }
    else
    {
        // Prepare ObjectToDevice transformation. Take PixelOffset for Lines into
        // account: Multiply from left to act in DeviceCoordinates
        aDamageMatrix = aDamageMatrix * rObjectToDevice;
        cairo_matrix_init(
            &aMatrix,
            aDamageMatrix.get( 0, 0 ),
            aDamageMatrix.get( 1, 0 ),
            aDamageMatrix.get( 0, 1 ),
            aDamageMatrix.get( 1, 1 ),
            aDamageMatrix.get( 0, 2 ),
            aDamageMatrix.get( 1, 2 ));
    }

    // set linear transformation
    cairo_set_matrix(cr, &aMatrix);

    // setup line attributes
    cairo_line_join_t eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
    switch (eLineJoin)
    {
        case basegfx::B2DLineJoin::Bevel:
            eCairoLineJoin = CAIRO_LINE_JOIN_BEVEL;
            break;
        case basegfx::B2DLineJoin::Round:
            eCairoLineJoin = CAIRO_LINE_JOIN_ROUND;
            break;
        case basegfx::B2DLineJoin::NONE:
        case basegfx::B2DLineJoin::Miter:
            eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
            break;
    }

    // convert miter minimum angle to miter limit
    double fMiterLimit = 1.0 / sin( fMiterMinimumAngle / 2.0);

    // setup cap attribute
    cairo_line_cap_t eCairoLineCap(CAIRO_LINE_CAP_BUTT);

    switch (eLineCap)
    {
        default: // css::drawing::LineCap_BUTT:
        {
            eCairoLineCap = CAIRO_LINE_CAP_BUTT;
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            eCairoLineCap = CAIRO_LINE_CAP_ROUND;
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            eCairoLineCap = CAIRO_LINE_CAP_SQUARE;
            break;
        }
    }

    cairo_set_source_rgba(
        cr,
        rLineColor.GetRed()/255.0,
        rLineColor.GetGreen()/255.0,
        rLineColor.GetBlue()/255.0,
        1.0-fTransparency);

    cairo_set_line_join(cr, eCairoLineJoin);
    cairo_set_line_cap(cr, eCairoLineCap);
    cairo_set_line_width(cr, fLineWidth);
    cairo_set_miter_limit(cr, fMiterLimit);

    // try to access buffered data
    std::shared_ptr<SystemDependentData_CairoPath> pSystemDependentData_CairoPath(
        rPolyLine.getSystemDependentData<SystemDependentData_CairoPath>());

    // MM01 need to do line dashing as fallback stuff here now
    const double fDotDashLength(nullptr != pStroke ? std::accumulate(pStroke->begin(), pStroke->end(), 0.0) : 0.0);
    const bool bStrokeUsed(0.0 != fDotDashLength);
    assert(!bStrokeUsed || (bStrokeUsed && pStroke));

    // MM01 decide if to stroke directly
    static const bool bDoDirectCairoStroke(true);

    // MM01 activate to stroke directly
    if(bDoDirectCairoStroke && bStrokeUsed)
    {
        cairo_set_dash(cr, pStroke->data(), pStroke->size(), 0.0);
    }

    if(!bDoDirectCairoStroke && pSystemDependentData_CairoPath)
    {
        // MM01 - check on stroke change. Used against not used, or if both used,
        // equal or different?
        const bool bStrokeWasUsed(!pSystemDependentData_CairoPath->getStroke().empty());

        if(bStrokeWasUsed != bStrokeUsed
        || (bStrokeUsed && *pStroke != pSystemDependentData_CairoPath->getStroke()))
        {
            // data invalid, forget
            pSystemDependentData_CairoPath.reset();
        }
    }

    // check for basegfx::B2DLineJoin::NONE to react accordingly
    const bool bNoJoin((basegfx::B2DLineJoin::NONE == eLineJoin
        && basegfx::fTools::more(fLineWidth, 0.0)));

    if(pSystemDependentData_CairoPath)
    {
        // check data validity
        if(nullptr == pSystemDependentData_CairoPath->getCairoPath()
            || pSystemDependentData_CairoPath->getNoJoin() != bNoJoin
            || pSystemDependentData_CairoPath->getAntiAlias() != bAntiAlias
            || bPixelSnapHairline /*tdf#124700*/ )
        {
            // data invalid, forget
            pSystemDependentData_CairoPath.reset();
        }
    }

    if(pSystemDependentData_CairoPath)
    {
        // re-use data
        cairo_append_path(cr, pSystemDependentData_CairoPath->getCairoPath());
    }
    else
    {
        // create data
        size_t nSizeMeasure(0);

        // MM01 need to do line dashing as fallback stuff here now
        basegfx::B2DPolyPolygon aPolyPolygonLine;

        if(!bDoDirectCairoStroke && bStrokeUsed)
        {
            // apply LineStyle
            basegfx::utils::applyLineDashing(
                rPolyLine, // source
                *pStroke, // pattern
                &aPolyPolygonLine, // target for lines
                nullptr, // target for gaps
                fDotDashLength); // full length if available
        }
        else
        {
            // no line dashing or direct stroke, just copy
            aPolyPolygonLine.append(rPolyLine);
        }

        // MM01 checked/verified for Cairo
        for(sal_uInt32 a(0); a < aPolyPolygonLine.count(); a++)
        {
            const basegfx::B2DPolygon aPolyLine(aPolyPolygonLine.getB2DPolygon(a));

            if (!bNoJoin)
            {
                // PixelOffset now reflected in linear transformation used
                nSizeMeasure += AddPolygonToPath(
                    cr,
                    aPolyLine,
                    rObjectToDevice, // ObjectToDevice *without* LineDraw-Offset
                    !bAntiAlias,
                    bPixelSnapHairline);
            }
            else
            {
                const sal_uInt32 nPointCount(aPolyLine.count());
                const sal_uInt32 nEdgeCount(aPolyLine.isClosed() ? nPointCount : nPointCount - 1);
                basegfx::B2DPolygon aEdge;

                aEdge.append(aPolyLine.getB2DPoint(0));
                aEdge.append(basegfx::B2DPoint(0.0, 0.0));

                for (sal_uInt32 i(0); i < nEdgeCount; i++)
                {
                    const sal_uInt32 nNextIndex((i + 1) % nPointCount);
                    aEdge.setB2DPoint(1, aPolyLine.getB2DPoint(nNextIndex));
                    aEdge.setNextControlPoint(0, aPolyLine.getNextControlPoint(i));
                    aEdge.setPrevControlPoint(1, aPolyLine.getPrevControlPoint(nNextIndex));

                    // PixelOffset now reflected in linear transformation used
                    nSizeMeasure += AddPolygonToPath(
                        cr,
                        aEdge,
                        rObjectToDevice, // ObjectToDevice *without* LineDraw-Offset
                        !bAntiAlias,
                        bPixelSnapHairline);

                    // prepare next step
                    aEdge.setB2DPoint(0, aEdge.getB2DPoint(1));
                }
            }
        }

        // copy and add to buffering mechanism
        if (!bPixelSnapHairline /*tdf#124700*/)
        {
            pSystemDependentData_CairoPath = rPolyLine.addOrReplaceSystemDependentData<SystemDependentData_CairoPath>(
                ImplGetSystemDependentDataManager(),
                nSizeMeasure,
                cr,
                bNoJoin,
                bAntiAlias,
                pStroke);
        }
    }

    // extract extents
    if (pExtents)
    {
        *pExtents = getClippedStrokeDamage(cr);
        // transform also extents (ranges) of damage so they can be correctly redrawn
        pExtents->transform(aDamageMatrix);
    }

    // draw and consume
    cairo_stroke(cr);

    return true;
}

bool SvpSalGraphics::drawPolyLineBezier( sal_uInt32,
                                         const Point*,
                                         const PolyFlags* )
{
    SAL_INFO("vcl.gdi", "unsupported SvpSalGraphics::drawPolyLineBezier case");
    return false;
}

bool SvpSalGraphics::drawPolygonBezier( sal_uInt32,
                                        const Point*,
                                        const PolyFlags* )
{
    SAL_INFO("vcl.gdi", "unsupported SvpSalGraphics::drawPolygonBezier case");
    return false;
}

bool SvpSalGraphics::drawPolyPolygonBezier( sal_uInt32,
                                            const sal_uInt32*,
                                            const Point* const*,
                                            const PolyFlags* const* )
{
    SAL_INFO("vcl.gdi", "unsupported SvpSalGraphics::drawPolyPolygonBezier case");
    return false;
}

namespace
{
    void add_polygon_path(cairo_t* cr, const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DHomMatrix& rObjectToDevice, bool bPixelSnap)
    {
        // try to access buffered data
        std::shared_ptr<SystemDependentData_CairoPath> pSystemDependentData_CairoPath(
            rPolyPolygon.getSystemDependentData<SystemDependentData_CairoPath>());

        if(pSystemDependentData_CairoPath)
        {
            // re-use data
            cairo_append_path(cr, pSystemDependentData_CairoPath->getCairoPath());
        }
        else
        {
            // create data
            size_t nSizeMeasure(0);

            for (const auto & rPoly : rPolyPolygon)
            {
                // PixelOffset used: Was dependent of 'm_aLineColor != SALCOLOR_NONE'
                // Adapt setupPolyPolygon-users to set a linear transformation to achieve PixelOffset
                nSizeMeasure += AddPolygonToPath(
                    cr,
                    rPoly,
                    rObjectToDevice,
                    bPixelSnap,
                    false);
            }

            // copy and add to buffering mechanism
            // for decisions how/what to buffer, see Note in WinSalGraphicsImpl::drawPolyPolygon
            pSystemDependentData_CairoPath = rPolyPolygon.addOrReplaceSystemDependentData<SystemDependentData_CairoPath>(
                ImplGetSystemDependentDataManager(),
                nSizeMeasure,
                cr,
                false,
                false,
                nullptr);
        }
    }
}

bool SvpSalGraphics::drawPolyPolygon(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolyPolygon& rPolyPolygon,
    double fTransparency)
{
    const bool bHasFill(m_aFillColor != SALCOLOR_NONE);
    const bool bHasLine(m_aLineColor != SALCOLOR_NONE);

    if(0 == rPolyPolygon.count() || !(bHasFill || bHasLine) || fTransparency < 0.0 || fTransparency >= 1.0)
    {
        return true;
    }

    cairo_t* cr = getCairoContext(true);
    clipRegion(cr);

    // Set full (Object-to-Device) transformation - if used
    if(!rObjectToDevice.isIdentity())
    {
        cairo_matrix_t aMatrix;

        cairo_matrix_init(
            &aMatrix,
            rObjectToDevice.get( 0, 0 ),
            rObjectToDevice.get( 1, 0 ),
            rObjectToDevice.get( 0, 1 ),
            rObjectToDevice.get( 1, 1 ),
            rObjectToDevice.get( 0, 2 ),
            rObjectToDevice.get( 1, 2 ));
        cairo_set_matrix(cr, &aMatrix);
    }

    // To make releaseCairoContext work, use empty extents
    basegfx::B2DRange extents;

    if (bHasFill)
    {
        add_polygon_path(cr, rPolyPolygon, rObjectToDevice, !getAntiAlias());

        applyColor(cr, m_aFillColor, fTransparency);
        // Get FillDamage (will be extended for LineDamage below)
        extents = getClippedFillDamage(cr);

        cairo_fill(cr);
    }

    if (bHasLine)
    {
        // PixelOffset used: Set PixelOffset as linear transformation
        cairo_matrix_t aMatrix;
        cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
        cairo_set_matrix(cr, &aMatrix);

        add_polygon_path(cr, rPolyPolygon, rObjectToDevice, !getAntiAlias());

        applyColor(cr, m_aLineColor, fTransparency);

        // expand with possible StrokeDamage
        basegfx::B2DRange stroke_extents = getClippedStrokeDamage(cr);
        stroke_extents.transform(basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5));
        extents.expand(stroke_extents);

        cairo_stroke(cr);
    }

    // if transformation has been applied, transform also extents (ranges)
    // of damage so they can be correctly redrawn
    extents.transform(rObjectToDevice);
    releaseCairoContext(cr, true, extents);

    return true;
}

bool SvpSalGraphics::drawGradient(const tools::PolyPolygon& rPolyPolygon, const Gradient& rGradient)
{
    if (rGradient.GetStyle() != GradientStyle::Linear
        && rGradient.GetStyle() != GradientStyle::Radial)
        return false; // unsupported
    if (rGradient.GetSteps() != 0)
        return false; // We can't tell cairo how many colors to use in the gradient.

    cairo_t* cr = getCairoContext(true);
    clipRegion(cr);

    tools::Rectangle aInputRect(rPolyPolygon.GetBoundRect());
    if( rPolyPolygon.IsRect())
    {
        // Rect->Polygon conversion loses the right and bottom edge, fix that.
        aInputRect.AdjustRight( 1 );
        aInputRect.AdjustBottom( 1 );
        basegfx::B2DHomMatrix rObjectToDevice;
        AddPolygonToPath(cr, tools::Polygon(aInputRect).getB2DPolygon(), rObjectToDevice, !getAntiAlias(), false);
    }
    else
    {
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPolygon.getB2DPolyPolygon());
        for (auto const & rPolygon : std::as_const(aB2DPolyPolygon))
        {
            basegfx::B2DHomMatrix rObjectToDevice;
            AddPolygonToPath(cr, rPolygon, rObjectToDevice, !getAntiAlias(), false);
        }
    }

    Gradient aGradient(rGradient);

    tools::Rectangle aBoundRect;
    Point aCenter;

    aGradient.SetAngle(aGradient.GetAngle() + 2700_deg10);
    aGradient.GetBoundRect(aInputRect, aBoundRect, aCenter);
    Color aStartColor = aGradient.GetStartColor();
    Color aEndColor = aGradient.GetEndColor();

    cairo_pattern_t* pattern;
    if (rGradient.GetStyle() == GradientStyle::Linear)
    {
        tools::Polygon aPoly(aBoundRect);
        aPoly.Rotate(aCenter, aGradient.GetAngle() % 3600_deg10);
        pattern = cairo_pattern_create_linear(aPoly[0].X(), aPoly[0].Y(), aPoly[1].X(), aPoly[1].Y());
    }
    else
    {
        double radius = std::max(aBoundRect.GetWidth() / 2.0, aBoundRect.GetHeight() / 2.0);
        // Move the center a bit to the top-left (the default VCL algorithm is a bit off-center that way,
        // cairo is the opposite way).
        pattern = cairo_pattern_create_radial(aCenter.X() - 0.5, aCenter.Y() - 0.5, 0,
            aCenter.X() - 0.5, aCenter.Y() - 0.5, radius);
        std::swap( aStartColor, aEndColor );
    }

    cairo_pattern_add_color_stop_rgba(pattern, aGradient.GetBorder() / 100.0,
        aStartColor.GetRed() * aGradient.GetStartIntensity()   / 25500.0,
        aStartColor.GetGreen() * aGradient.GetStartIntensity() / 25500.0,
        aStartColor.GetBlue() * aGradient.GetStartIntensity()  / 25500.0,
        1.0);

    cairo_pattern_add_color_stop_rgba(pattern, 1.0,
        aEndColor.GetRed() * aGradient.GetEndIntensity()   / 25500.0,
        aEndColor.GetGreen() * aGradient.GetEndIntensity() / 25500.0,
        aEndColor.GetBlue() * aGradient.GetEndIntensity()  / 25500.0,
        1.0);

    cairo_set_source(cr, pattern);
    cairo_pattern_destroy(pattern);

    basegfx::B2DRange extents = getClippedFillDamage(cr);
    cairo_fill_preserve(cr);

    releaseCairoContext(cr, true, extents);

    return true;
}

bool SvpSalGraphics::implDrawGradient(basegfx::B2DPolyPolygon const & rPolyPolygon, SalGradient const & rGradient)
{
    cairo_t* cr = getCairoContext(true);
    clipRegion(cr);

    basegfx::B2DHomMatrix rObjectToDevice;

    for (auto const & rPolygon : rPolyPolygon)
        AddPolygonToPath(cr, rPolygon, rObjectToDevice, !getAntiAlias(), false);

    cairo_pattern_t* pattern = cairo_pattern_create_linear(rGradient.maPoint1.getX(), rGradient.maPoint1.getY(), rGradient.maPoint2.getX(), rGradient.maPoint2.getY());

    for (SalGradientStop const & rStop : rGradient.maStops)
    {
        double r = rStop.maColor.GetRed() / 255.0;
        double g = rStop.maColor.GetGreen() / 255.0;
        double b = rStop.maColor.GetBlue() / 255.0;
        double a = rStop.maColor.GetAlpha() / 255.0;
        double offset = rStop.mfOffset;

        cairo_pattern_add_color_stop_rgba(pattern, offset, r, g, b, a);
    }
    cairo_set_source(cr, pattern);
    cairo_pattern_destroy(pattern);

    basegfx::B2DRange extents = getClippedFillDamage(cr);
    cairo_fill_preserve(cr);

    releaseCairoContext(cr, true, extents);

    return true;
}

void SvpSalGraphics::applyColor(cairo_t *cr, Color aColor, double fTransparency)
{
    if (cairo_surface_get_content(m_pSurface) == CAIRO_CONTENT_COLOR_ALPHA)
    {
        cairo_set_source_rgba(cr, aColor.GetRed()/255.0,
                                  aColor.GetGreen()/255.0,
                                  aColor.GetBlue()/255.0,
                                  1.0 - fTransparency);
    }
    else
    {
        double fSet = aColor == COL_BLACK ? 1.0 : 0.0;
        cairo_set_source_rgba(cr, 1, 1, 1, fSet);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    }
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
    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    basegfx::B2DRange extents = renderWithOperator(cr, rTR, source, eOp);

    releaseCairoContext(cr, false, extents);
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

    cairo_surface_t* source = pSrc->m_pSurface;

    cairo_surface_t *pCopy = nullptr;
    if (pSrc == this)
    {
        //self copy is a problem, so dup source in that case
        pCopy = cairo_surface_create_similar(source,
                                            cairo_surface_get_content(m_pSurface),
                                            aTR.mnSrcWidth * m_fScale,
                                            aTR.mnSrcHeight * m_fScale);
        dl_cairo_surface_set_device_scale(pCopy, m_fScale, m_fScale);
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

    cairo_t* cr = getCairoContext(false);
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

    releaseCairoContext(cr, false, extents);
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
    renderSource(cr, aTR, m_pSurface);

    cairo_destroy(cr);
    cairo_surface_destroy(target);

    Toggle1BitTransparency(*pBitmap->GetBuffer());

    return pBitmap;
}

Color SvpSalGraphics::getPixel( tools::Long nX, tools::Long nY )
{
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
    cairo_surface_t *target = cairo_surface_create_similar_image(m_pSurface, CAIRO_FORMAT_ARGB32, 1, 1);
#else
    cairo_surface_t *target = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
#endif

    cairo_t* cr = cairo_create(target);

    cairo_rectangle(cr, 0, 0, 1, 1);
    cairo_set_source_surface(cr, m_pSurface, -nX, -nY);
    cairo_paint(cr);
    cairo_destroy(cr);

    cairo_surface_flush(target);
    vcl::bitmap::lookup_table const & unpremultiply_table = vcl::bitmap::get_unpremultiply_table();
    unsigned char *data = cairo_image_surface_get_data(target);
    sal_uInt8 a = data[SVP_CAIRO_ALPHA];
    sal_uInt8 b = unpremultiply_table[a][data[SVP_CAIRO_BLUE]];
    sal_uInt8 g = unpremultiply_table[a][data[SVP_CAIRO_GREEN]];
    sal_uInt8 r = unpremultiply_table[a][data[SVP_CAIRO_RED]];
    Color aColor(ColorAlpha, a, r, g, b);
    cairo_surface_destroy(target);

    return aColor;
}

namespace
{
    cairo_pattern_t * create_stipple()
    {
        static unsigned char data[16] = { 0xFF, 0xFF, 0x00, 0x00,
                                          0xFF, 0xFF, 0x00, 0x00,
                                          0x00, 0x00, 0xFF, 0xFF,
                                          0x00, 0x00, 0xFF, 0xFF };
        cairo_surface_t* surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_A8, 4, 4, 4);
        cairo_pattern_t* pattern = cairo_pattern_create_for_surface(surface);
        cairo_surface_destroy(surface);
        cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
        cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);
        return pattern;
    }
}

void SvpSalGraphics::invert(const basegfx::B2DPolygon &rPoly, SalInvert nFlags)
{
    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    // To make releaseCairoContext work, use empty extents
    basegfx::B2DRange extents;

    AddPolygonToPath(
        cr,
        rPoly,
        basegfx::B2DHomMatrix(),
        !getAntiAlias(),
        false);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

    if (cairo_version() >= CAIRO_VERSION_ENCODE(1, 10, 0))
    {
        cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
    }
    else
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::invert, archaic cairo");
    }

    if (nFlags & SalInvert::TrackFrame)
    {
        cairo_set_line_width(cr, 2.0);
        const double dashLengths[2] = { 4.0, 4.0 };
        cairo_set_dash(cr, dashLengths, 2, 0);

        extents = getClippedStrokeDamage(cr);
        //see tdf#106577 under wayland, some pixel droppings seen, maybe we're
        //out by one somewhere, or cairo_stroke_extents is confused by
        //dashes/line width
        if(!extents.isEmpty())
        {
            extents.grow(1);
        }

        cairo_stroke(cr);
    }
    else
    {
        extents = getClippedFillDamage(cr);

        cairo_clip(cr);

        if (nFlags & SalInvert::N50)
        {
            cairo_pattern_t *pattern = create_stipple();
            cairo_surface_t* surface = cairo_surface_create_similar(m_pSurface,
                                                                    cairo_surface_get_content(m_pSurface),
                                                                    extents.getWidth() * m_fScale,
                                                                    extents.getHeight() * m_fScale);

            dl_cairo_surface_set_device_scale(surface, m_fScale, m_fScale);
            cairo_t* stipple_cr = cairo_create(surface);
            cairo_set_source_rgb(stipple_cr, 1.0, 1.0, 1.0);
            cairo_mask(stipple_cr, pattern);
            cairo_pattern_destroy(pattern);
            cairo_destroy(stipple_cr);
            cairo_mask_surface(cr, surface, extents.getMinX(), extents.getMinY());
            cairo_surface_destroy(surface);
        }
        else
        {
            cairo_paint(cr);
        }
    }

    releaseCairoContext(cr, false, extents);
}

void SvpSalGraphics::invert( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, SalInvert nFlags )
{
    basegfx::B2DPolygon aRect = basegfx::utils::createPolygonFromRect(basegfx::B2DRectangle(nX, nY, nX+nWidth, nY+nHeight));

    invert(aRect, nFlags);
}

void SvpSalGraphics::invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));
    aPoly.setClosed(true);

    invert(aPoly, nFlags);
}

bool SvpSalGraphics::drawEPS( tools::Long, tools::Long, tools::Long, tools::Long, void*, sal_uInt32 )
{
    return false;
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

cairo_t* SvpSalGraphics::createTmpCompatibleCairoContext() const
{
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
    cairo_surface_t *target = cairo_surface_create_similar_image(m_pSurface,
#else
    cairo_surface_t *target = cairo_image_surface_create(
#endif
            CAIRO_FORMAT_ARGB32,
            m_aFrameSize.getX() * m_fScale,
            m_aFrameSize.getY() * m_fScale);

    dl_cairo_surface_set_device_scale(target, m_fScale, m_fScale);

    return cairo_create(target);
}

cairo_t* SvpSalGraphics::getCairoContext(bool bXorModeAllowed) const
{
    cairo_t* cr;
    if (m_ePaintMode == PaintMode::Xor && bXorModeAllowed)
        cr = createTmpCompatibleCairoContext();
    else
        cr = cairo_create(m_pSurface);
    cairo_set_line_width(cr, 1);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_antialias(cr, getAntiAlias() ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    // ensure no linear transformation and no PathInfo in local cairo_path_t
    cairo_identity_matrix(cr);
    cairo_new_path(cr);

    return cr;
}

cairo_user_data_key_t* SvpSalGraphics::getDamageKey()
{
    static cairo_user_data_key_t aDamageKey;
    return &aDamageKey;
}

void SvpSalGraphics::releaseCairoContext(cairo_t* cr, bool bXorModeAllowed, const basegfx::B2DRange& rExtents) const
{
    const bool bXoring = (m_ePaintMode == PaintMode::Xor && bXorModeAllowed);

    if (rExtents.isEmpty())
    {
        //nothing changed, return early
        if (bXoring)
        {
            cairo_surface_t* surface = cairo_get_target(cr);
            cairo_surface_destroy(surface);
        }
        cairo_destroy(cr);
        return;
    }

    basegfx::B2IRange aIntExtents(basegfx::unotools::b2ISurroundingRangeFromB2DRange(rExtents));
    sal_Int32 nExtentsLeft(aIntExtents.getMinX()), nExtentsTop(aIntExtents.getMinY());
    sal_Int32 nExtentsRight(aIntExtents.getMaxX()), nExtentsBottom(aIntExtents.getMaxY());
    sal_Int32 nWidth = m_aFrameSize.getX();
    sal_Int32 nHeight = m_aFrameSize.getY();
    nExtentsLeft = std::max<sal_Int32>(nExtentsLeft, 0);
    nExtentsTop = std::max<sal_Int32>(nExtentsTop, 0);
    nExtentsRight = std::min<sal_Int32>(nExtentsRight, nWidth);
    nExtentsBottom = std::min<sal_Int32>(nExtentsBottom, nHeight);

    cairo_surface_t* surface = cairo_get_target(cr);
    cairo_surface_flush(surface);

    //For the most part we avoid the use of XOR these days, but there
    //are some edge cases where legacy stuff still supports it, so
    //emulate it (slowly) here.
    if (bXoring)
    {
        cairo_surface_t* target_surface = m_pSurface;
        if (cairo_surface_get_type(target_surface) != CAIRO_SURFACE_TYPE_IMAGE)
        {
            //in the unlikely case we can't use m_pSurface directly, copy contents
            //to another temp image surface
            cairo_t* copycr = createTmpCompatibleCairoContext();
            cairo_rectangle(copycr, nExtentsLeft, nExtentsTop,
                                    nExtentsRight - nExtentsLeft,
                                    nExtentsBottom - nExtentsTop);
            cairo_set_source_surface(copycr, m_pSurface, 0, 0);
            cairo_paint(copycr);
            target_surface = cairo_get_target(copycr);
            cairo_destroy(copycr);
        }

        cairo_surface_flush(target_surface);
        unsigned char *target_surface_data = cairo_image_surface_get_data(target_surface);
        unsigned char *xor_surface_data = cairo_image_surface_get_data(surface);

        cairo_format_t nFormat = cairo_image_surface_get_format(target_surface);
        assert(nFormat == CAIRO_FORMAT_ARGB32 && "need to implement CAIRO_FORMAT_A1 after all here");
        sal_Int32 nStride = cairo_format_stride_for_width(nFormat, nWidth * m_fScale);
        sal_Int32 nUnscaledExtentsLeft = nExtentsLeft * m_fScale;
        sal_Int32 nUnscaledExtentsRight = nExtentsRight * m_fScale;
        sal_Int32 nUnscaledExtentsTop = nExtentsTop * m_fScale;
        sal_Int32 nUnscaledExtentsBottom = nExtentsBottom * m_fScale;

        // Handle headless size forced to (1,1) by SvpSalFrame::GetSurfaceFrameSize().
        int target_surface_width = cairo_image_surface_get_width(target_surface);
        if (nUnscaledExtentsLeft > target_surface_width)
            nUnscaledExtentsLeft = target_surface_width;
        if (nUnscaledExtentsRight > target_surface_width)
            nUnscaledExtentsRight = target_surface_width;
        int target_surface_height = cairo_image_surface_get_height(target_surface);
        if (nUnscaledExtentsTop > target_surface_height)
            nUnscaledExtentsTop = target_surface_height;
        if (nUnscaledExtentsBottom > target_surface_height)
            nUnscaledExtentsBottom = target_surface_height;

        vcl::bitmap::lookup_table const & unpremultiply_table
            = vcl::bitmap::get_unpremultiply_table();
        vcl::bitmap::lookup_table const & premultiply_table = vcl::bitmap::get_premultiply_table();
        for (sal_Int32 y = nUnscaledExtentsTop; y < nUnscaledExtentsBottom; ++y)
        {
            unsigned char *true_row = target_surface_data + (nStride*y);
            unsigned char *xor_row = xor_surface_data + (nStride*y);
            unsigned char *true_data = true_row + (nUnscaledExtentsLeft * 4);
            unsigned char *xor_data = xor_row + (nUnscaledExtentsLeft * 4);
            for (sal_Int32 x = nUnscaledExtentsLeft; x < nUnscaledExtentsRight; ++x)
            {
                sal_uInt8 a = true_data[SVP_CAIRO_ALPHA];
                sal_uInt8 xor_a = xor_data[SVP_CAIRO_ALPHA];
                sal_uInt8 b = unpremultiply_table[a][true_data[SVP_CAIRO_BLUE]] ^
                              unpremultiply_table[xor_a][xor_data[SVP_CAIRO_BLUE]];
                sal_uInt8 g = unpremultiply_table[a][true_data[SVP_CAIRO_GREEN]] ^
                              unpremultiply_table[xor_a][xor_data[SVP_CAIRO_GREEN]];
                sal_uInt8 r = unpremultiply_table[a][true_data[SVP_CAIRO_RED]] ^
                              unpremultiply_table[xor_a][xor_data[SVP_CAIRO_RED]];
                true_data[SVP_CAIRO_BLUE] = premultiply_table[a][b];
                true_data[SVP_CAIRO_GREEN] = premultiply_table[a][g];
                true_data[SVP_CAIRO_RED] = premultiply_table[a][r];
                true_data+=4;
                xor_data+=4;
            }
        }
        cairo_surface_mark_dirty(target_surface);

        if (target_surface != m_pSurface)
        {
            cairo_t* copycr = cairo_create(m_pSurface);
            //unlikely case we couldn't use m_pSurface directly, copy contents
            //back from image surface
            cairo_rectangle(copycr, nExtentsLeft, nExtentsTop,
                                    nExtentsRight - nExtentsLeft,
                                    nExtentsBottom - nExtentsTop);
            cairo_set_source_surface(copycr, target_surface, 0, 0);
            cairo_paint(copycr);
            cairo_destroy(copycr);
            cairo_surface_destroy(target_surface);
        }

        cairo_surface_destroy(surface);
    }

    cairo_destroy(cr); // unref

    DamageHandler* pDamage = static_cast<DamageHandler*>(cairo_surface_get_user_data(m_pSurface, getDamageKey()));

    if (pDamage)
    {
        pDamage->damaged(pDamage->handle, nExtentsLeft, nExtentsTop,
                                          nExtentsRight - nExtentsLeft,
                                          nExtentsBottom - nExtentsTop);
    }
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

void dl_cairo_surface_set_device_scale(cairo_surface_t *surface, double x_scale, double y_scale)
{
#ifdef ANDROID
    cairo_surface_set_device_scale(surface, x_scale, y_scale);
#else
    static auto func = reinterpret_cast<void(*)(cairo_surface_t*, double, double)>(
        dlsym(nullptr, "cairo_surface_set_device_scale"));
    if (func)
        func(surface, x_scale, y_scale);
#endif
}

void dl_cairo_surface_get_device_scale(cairo_surface_t *surface, double* x_scale, double* y_scale)
{
#ifdef ANDROID
    cairo_surface_get_device_scale(surface, x_scale, y_scale);
#else
    static auto func = reinterpret_cast<void(*)(cairo_surface_t*, double*, double*)>(
        dlsym(nullptr, "cairo_surface_get_device_scale"));
    if (func)
        func(surface, x_scale, y_scale);
    else
    {
        if (x_scale)
            *x_scale = 1.0;
        if (y_scale)
            *y_scale = 1.0;
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
