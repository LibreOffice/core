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
#include <bitmap/BitmapWriteAccess.hxx>
#include <tools/stream.hxx>
#include <unotools/configmgr.hxx>

#include <tiffio.h>

#include <filter/TiffReader.hxx>

namespace
{
    struct Context
    {
        SvStream& rStream;
        tsize_t nSize;
        int nShortReads;
        Context(SvStream& rInStream, tsize_t nInSize)
            : rStream(rInStream)
            , nSize(nInSize)
            , nShortReads(0)
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
    if (nRead < size && !pContext->nShortReads)
    {
        memset(static_cast<char*>(buf) + nRead, 0, size - nRead);
        ++pContext->nShortReads;
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
            pContext->rStream.Seek(offset);
            break;
        case SEEK_CUR:
            pContext->rStream.SeekRel(offset);
            break;
        case SEEK_END:
            pContext->rStream.Seek(STREAM_SEEK_TO_END);
            pContext->rStream.SeekRel(offset);
            break;
        default:
            assert(false && "unknown seek type");
            break;
    }

    return pContext->rStream.Tell();
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

    Context aContext(rTIFF, rTIFF.remainingSize());
    TIFF* tif = TIFFClientOpen("libtiff-svstream", "r", &aContext,
                               tiff_read, tiff_write,
                               tiff_seek, tiff_close,
                               tiff_size, nullptr, nullptr);

    if (!tif)
        return false;

    const auto nOrigPos = rTIFF.Tell();

    Animation aAnimation;

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

        if (utl::ConfigManager::IsFuzzing())
        {
            const uint64_t MAX_SIZE = 500000000;
            if (TIFFTileSize64(tif) > MAX_SIZE)
            {
                SAL_WARN("filter.tiff", "skipping large tiffs");
                break;
            }
        }

        uint32_t nPixelsRequired;
        constexpr size_t nMaxPixelsAllowed = SAL_MAX_INT32/4;
        // two buffers currently required, so limit further
        bool bOk = !o3tl::checked_multiply(w, h, nPixelsRequired) && nPixelsRequired <= nMaxPixelsAllowed / 2;
        if (!bOk)
        {
            SAL_WARN("filter.tiff", "skipping oversized tiff image " << w << " x " << h);
            break;
        }

        std::vector<uint32_t> raster(nPixelsRequired);
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
            AlphaScopedWriteAccess accessAlpha(bitmapAlpha);
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
                    accessAlpha->SetPixelIndex(y, dest, 255 - a);
                    ++src;
                }
            }

            raster.clear();

            access.reset();
            accessAlpha.reset();

            BitmapEx aBitmapEx(bitmap, bitmapAlpha);

            switch (nOrientation)
            {
                case ORIENTATION_LEFTBOT:
                    aBitmapEx.Rotate(2700_deg10, COL_BLACK);
                    break;
                default:
                    break;
            }

            AnimationBitmap aAnimationBitmap(aBitmapEx, Point(0, 0), aBitmapEx.GetSizePixel(),
                                             ANIMATION_TIMEOUT_ON_CLICK, Disposal::Back);
            aAnimation.Insert(aAnimationBitmap);
        }
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
