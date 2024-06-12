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

#include <sal/config.h>
#include <sal/log.hxx>

#include <comphelper/scopeguard.hxx>
#include <vcl/graph.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/animate/Animation.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <unotools/configmgr.hxx>

#include <tiffio.h>

#include <filter/TiffReader.hxx>

namespace
{
    struct Context
    {
        SvStream& rStream;
        tsize_t nStart;
        tsize_t nSize;
        bool bAllowOneShortRead;
        Context(SvStream& rInStream)
            : rStream(rInStream)
            , nStart(rInStream.Tell())
            , nSize(rInStream.remainingSize())
            , bAllowOneShortRead(false)
        {
        }
    };
}

static tsize_t tiff_read(thandle_t handle, tdata_t buf, tsize_t size)
{
    Context* pContext = static_cast<Context*>(handle);
    tsize_t nRead = pContext->rStream.ReadBytes(buf, size);
    // tdf#149417 allow one short read, which is similar to what
    // we do for jpeg since tdf#138950
    if (nRead < size && pContext->bAllowOneShortRead)
    {
        memset(static_cast<char*>(buf) + nRead, 0, size - nRead);
        pContext->bAllowOneShortRead = false;
        return size;
    }
    return nRead;
}

static tsize_t tiff_write(thandle_t, tdata_t, tsize_t)
{
    return -1;
}

static toff_t tiff_seek(thandle_t handle, toff_t offset, int whence)
{
    Context* pContext = static_cast<Context*>(handle);

    switch (whence)
    {
        case SEEK_SET:
            offset = pContext->nStart + offset;
            break;
        case SEEK_CUR:
            offset = pContext->rStream.Tell() + offset;
            break;
        case SEEK_END:
            offset = pContext->rStream.TellEnd() + offset;
            break;
        default:
            assert(false && "unknown seek type");
            break;
    }

    pContext->rStream.Seek(offset);

    return offset - pContext->nStart;
}

static int tiff_close(thandle_t)
{
    return 0;
}

static toff_t tiff_size(thandle_t handle)
{
    Context* pContext = static_cast<Context*>(handle);
    return pContext->nSize;
}

bool ImportTiffGraphicImport(SvStream& rTIFF, Graphic& rGraphic)
{
    auto origErrorHandler = TIFFSetErrorHandler(nullptr);
    auto origWarningHandler = TIFFSetWarningHandler(nullptr);
    comphelper::ScopeGuard restoreDefaultHandlers([&]() {
        TIFFSetErrorHandler(origErrorHandler);
        TIFFSetWarningHandler(origWarningHandler);
    });

    Context aContext(rTIFF);
    TIFF* tif = TIFFClientOpen("libtiff-svstream", "r", &aContext,
                               tiff_read, tiff_write,
                               tiff_seek, tiff_close,
                               tiff_size, nullptr, nullptr);

    if (!tif)
        return false;

    const auto nOrigPos = rTIFF.Tell();

    Animation aAnimation;

    const bool bFuzzing = utl::ConfigManager::IsFuzzing();
    uint64_t nTotalPixelsRequired = 0;

    do
    {
        uint32_t w, h;

        if (TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w) != 1)
        {
            SAL_WARN("filter.tiff", "missing width");
            break;
        }

        if (TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h) != 1)
        {
            SAL_WARN("filter.tiff", "missing height");
            break;
        }

        if (w > SAL_MAX_INT32 / 32 || h > SAL_MAX_INT32 / 32)
        {
            SAL_WARN("filter.tiff", "image too large");
            break;
        }

        uint32_t nPixelsRequired;
        constexpr size_t nMaxPixelsAllowed = SAL_MAX_INT32/4;
        // two buffers currently required, so limit further
        bool bOk = !o3tl::checked_multiply(w, h, nPixelsRequired) && nPixelsRequired <= nMaxPixelsAllowed / 2;
        SAL_WARN_IF(!bOk, "filter.tiff", "skipping oversized tiff image " << w << " x " << h);

        if (!TIFFIsTiled(tif))
        {
            size_t nStripSize = TIFFStripSize(tif);
            if (nStripSize > SAL_MAX_INT32)
            {
                SAL_WARN("filter.tiff", "skipping oversized tiff strip size " << nStripSize);
                bOk = false;
            }
        }

        uint16_t PhotometricInterpretation(0);
        uint16_t Compression(COMPRESSION_NONE);
        if (bOk)
        {
            TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &PhotometricInterpretation);
            TIFFGetField(tif, TIFFTAG_COMPRESSION, &Compression);
        }

        if (bOk && bFuzzing)
        {
            const uint64_t MAX_PIXEL_SIZE = 120000000;
            const uint64_t MAX_TILE_SIZE = 100000000;
            nTotalPixelsRequired += nPixelsRequired;
            if (TIFFTileSize64(tif) > MAX_TILE_SIZE || nTotalPixelsRequired > MAX_PIXEL_SIZE)
            {
                SAL_WARN("filter.tiff", "skipping large tiffs");
                break;
            }

            if (TIFFIsTiled(tif))
            {
                uint32_t tw, th;
                TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tw);
                TIFFGetField(tif, TIFFTAG_TILELENGTH, &th);

                if (tw > w || th > h)
                {
                    bOk = th < 1000 * tw && tw < 1000 * th;
                    SAL_WARN_IF(!bOk, "filter.tiff", "skipping slow bizarre ratio tile of " << tw << " x " << th << " for image of " << w << " x " << h);
                }

                if (PhotometricInterpretation == PHOTOMETRIC_LOGL)
                {
                    uint32_t nLogLBufferRequired;
                    bOk &= !o3tl::checked_multiply(tw, th, nLogLBufferRequired) && nLogLBufferRequired < MAX_PIXEL_SIZE;
                    SAL_WARN_IF(!bOk, "filter.tiff", "skipping oversized tiff tile " << tw << " x " << th);
                }

                if (Compression == COMPRESSION_CCITTFAX4)
                {
                    uint32_t DspRuns;
                    bOk &= !o3tl::checked_multiply(tw, static_cast<uint32_t>(4), DspRuns) && DspRuns < MAX_PIXEL_SIZE;
                    SAL_WARN_IF(!bOk, "filter.tiff", "skipping oversized tiff tile width: " << tw);
                }
            }
        }

        if (!bOk)
            break;

        std::vector<uint32_t> raster(nPixelsRequired);

        const bool bNewCodec = Compression >= COMPRESSION_ZSTD; // >= 50000 at time of writing
        // For tdf#149417 we generally allow one short read for fidelity with the old
        // parser that this replaced. But don't allow that for:
        // a) new compression variations that the old parser didn't handle
        // b) complicated pixel layout variations that the old parser didn't handle
        // so we don't take libtiff into uncharted territory.
        aContext.bAllowOneShortRead = !bNewCodec && PhotometricInterpretation != PHOTOMETRIC_YCBCR;

        if (TIFFReadRGBAImageOriented(tif, w, h, raster.data(), ORIENTATION_TOPLEFT, 1))
        {
            Bitmap bitmap(Size(w, h), vcl::PixelFormat::N24_BPP);
            BitmapScopedWriteAccess access(bitmap);
            if (!access)
            {
                SAL_WARN("filter.tiff", "cannot create image " << w << " x " << h);
                break;
            }

            AlphaMask bitmapAlpha(Size(w, h));
            BitmapScopedWriteAccess accessAlpha(bitmapAlpha);
            if (!accessAlpha)
            {
                SAL_WARN("filter.tiff", "cannot create alpha " << w << " x " << h);
                break;
            }

            /*
                ORIENTATION_TOPLEFT = 1
                ORIENTATION_TOPRIGHT = 2
                ORIENTATION_BOTRIGHT = 3
                ORIENTATION_BOTLEFT = 4
                ORIENTATION_LEFTTOP = 5
                ORIENTATION_RIGHTTOP = 6
                ORIENTATION_RIGHTBOT = 7
                ORIENTATION_LEFTBOT = 8
             */
            uint16_t nOrientation;
            if (TIFFGetField(tif, TIFFTAG_ORIENTATION, &nOrientation) != 1)
                nOrientation = 0;

            for (uint32_t y = 0; y < h; ++y)
            {
                const uint32_t* src = raster.data() + w * y;
                for (uint32_t x = 0; x < w; ++x)
                {
                    sal_uInt8 r = TIFFGetR(*src);
                    sal_uInt8 g = TIFFGetG(*src);
                    sal_uInt8 b = TIFFGetB(*src);
                    sal_uInt8 a = TIFFGetA(*src);

                    uint32_t dest;
                    switch (nOrientation)
                    {
                        case ORIENTATION_LEFTBOT:
                            dest = w - 1 - x;
                            break;
                        default:
                            dest = x;
                            break;
                    }

                    access->SetPixel(y, dest, Color(r, g, b));
                    accessAlpha->SetPixelIndex(y, dest, a);
                    ++src;
                }
            }

            raster.clear();

            access.reset();
            accessAlpha.reset();

            BitmapEx aBitmapEx(bitmap, bitmapAlpha);

            if (!bFuzzing)
            {
                switch (nOrientation)
                {
                    case ORIENTATION_LEFTBOT:
                        aBitmapEx.Rotate(2700_deg10, COL_BLACK);
                        break;
                    default:
                        break;
                }
            }

            MapMode aMapMode;
            uint16_t ResolutionUnit = RESUNIT_NONE;
            if (TIFFGetField(tif, TIFFTAG_RESOLUTIONUNIT, &ResolutionUnit) == 1 && ResolutionUnit != RESUNIT_NONE)
            {
                float xres = 0, yres = 0;

                if (TIFFGetField(tif, TIFFTAG_XRESOLUTION, &xres) == 1 &&
                    TIFFGetField(tif, TIFFTAG_YRESOLUTION, &yres) == 1 &&
                    xres != 0 && yres != 0)
                {
                    if (ResolutionUnit == RESUNIT_INCH)
                        aMapMode =  MapMode(MapUnit::MapInch, Point(0,0), Fraction(1/xres), Fraction(1/yres));
                    else if (ResolutionUnit == RESUNIT_CENTIMETER)
                        aMapMode =  MapMode(MapUnit::MapCM, Point(0,0), Fraction(1/xres), Fraction(1/yres));
                }
            }
            aBitmapEx.SetPrefMapMode(aMapMode);
            aBitmapEx.SetPrefSize(Size(w, h));

            AnimationFrame aAnimationFrame(aBitmapEx, Point(0, 0), aBitmapEx.GetSizePixel(),
                                             ANIMATION_TIMEOUT_ON_CLICK, Disposal::Back);
            aAnimation.Insert(aAnimationFrame);
        }
        else
            break;
    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);

    const auto nImages = aAnimation.Count();
    if (nImages)
    {
        if (nImages == 1)
            rGraphic = aAnimation.GetBitmapEx();
        else
            rGraphic = aAnimation;

        // seek to end of TIFF if succeeded
        rTIFF.Seek(STREAM_SEEK_TO_END);

        return true;
    }

    rTIFF.Seek(nOrigPos);
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
