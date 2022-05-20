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

#include <vcl/graph.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/animate/Animation.hxx>
#include <bitmap/BitmapWriteAccess.hxx>
#include <tools/stream.hxx>

#include <tiffio.h>

#include <filter/TiffReader.hxx>

namespace
{
    struct Context
    {
        SvStream& rStream;
        tsize_t nSize;

        tileContigRoutine pOrigContig;
        tileSeparateRoutine pOrigSeparate;
        BitmapWriteAccess* pWriteAccess;
        BitmapWriteAccess* pAlphaAccess;
        std::vector<uint32_t> aBuffer;

        Context(SvStream& rInStream, tsize_t nInSize)
            : rStream(rInStream)
            , nSize(nInSize)
            , pOrigContig(nullptr)
            , pOrigSeparate(nullptr)
            , pWriteAccess(nullptr)
            , pAlphaAccess(nullptr)
        {
        }

        void SetPixels(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
        {
            const uint32_t* pSrc = aBuffer.data();

            for (uint32_t nRow = 0; nRow < h; ++nRow)
            {
                for (uint32_t nCol = 0; nCol < w; ++nCol)
                {
                    pWriteAccess->SetPixel(y + nRow, x + nCol,
                        Color(TIFFGetR(*pSrc), TIFFGetG(*pSrc), TIFFGetB(*pSrc)));
                    pAlphaAccess->SetPixelIndex(y, x, 255 - TIFFGetA(*pSrc));
                    ++pSrc;
                }
            }
        }
    };
}

static tsize_t tiff_read(thandle_t handle, tdata_t buf, tsize_t size)
{
    Context* pContext = static_cast<Context*>(handle);
    return pContext->rStream.ReadBytes(buf, size);
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

static void putContigPixel(TIFFRGBAImage* img, uint32_t* /*raster*/,
                           uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                           int32_t fromskew, int32_t toskew,
                           unsigned char* cp)
{
    Context* pContext = static_cast<Context*>(TIFFClientdata(img->tif));

    pContext->aBuffer.resize(w * h);
    (pContext->pOrigContig)(img, pContext->aBuffer.data(), 0, 0, w, h,
                            fromskew, toskew, cp);

    pContext->SetPixels(x, y, w, h);
}

static void putSeparatePixel(TIFFRGBAImage* img, uint32_t* /*raster*/,
                             uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                             int32_t fromskew, int32_t toskew,
                             unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
{
    Context* pContext = static_cast<Context*>(TIFFClientdata(img->tif));

    pContext->aBuffer.resize(w * h);
    (pContext->pOrigSeparate)(img, pContext->aBuffer.data(), 0, 0, w, h,
                              fromskew, toskew, r, g, b, a);

    pContext->SetPixels(x, y, w, h);
}

bool ImportTiffGraphicImport(SvStream& rTIFF, Graphic& rGraphic)
{
    Context aContext(rTIFF, rTIFF.remainingSize());
    TIFF* tif = TIFFClientOpen("libtiff-svstream", "r", &aContext,
                               tiff_read, tiff_write,
                               tiff_seek, tiff_close,
                               tiff_size, nullptr, nullptr);

    if (!tif)
        return false;

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

        if (w > SAL_MAX_INT32 / 8 || h > SAL_MAX_INT32 / 8)
        {
            SAL_WARN("filter.tiff", "image too large");
            break;
        }

        Bitmap bitmap(Size(w, h), vcl::PixelFormat::N24_BPP);
        AlphaMask bitmapAlpha(Size(w, h));

        BitmapScopedWriteAccess access(bitmap);
        AlphaScopedWriteAccess accessAlpha(bitmapAlpha);

        if (!access || !accessAlpha)
        {
            SAL_WARN("filter.tiff", "could not create bitmaps");
            break;
        }

        aContext.pWriteAccess = access.get();
        aContext.pAlphaAccess = accessAlpha.get();

        char emsg[1024] = "";
        TIFFRGBAImage img;
        bool bOk = false;
        // Expanded out TIFFReadRGBAImageOriented to create this block then
        // inserted custom "put" methods to write (via a limited size buffer)
        // into the final Bitmap incrementally
        if (TIFFRGBAImageOK(tif, emsg) && TIFFRGBAImageBegin(&img, tif, 1, emsg))
        {
            img.req_orientation = ORIENTATION_TOPLEFT;
            assert(!TIFFIsTiled(img.tif));
            if (!TIFFIsTiled(img.tif))
            {
                aContext.pOrigContig = img.put.contig;
                img.put.contig = putContigPixel;
            }
            else
            {
                aContext.pOrigSeparate = img.put.separate;
                img.put.separate = putSeparatePixel;
            }

            bOk = TIFFRGBAImageGet(&img, nullptr, w, img.height);
            TIFFRGBAImageEnd(&img);
        }

        access.reset();
        accessAlpha.reset();

        if (bOk)
        {
            BitmapEx aBitmapEx(bitmap, bitmapAlpha);
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
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
