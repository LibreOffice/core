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
        Context(SvStream& rInStream, tsize_t nInSize)
            : rStream(rInStream)
            , nSize(nInSize)
        {
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

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

        size_t npixels = w * h;
        uint32_t* raster = static_cast<uint32_t*>(_TIFFmalloc(npixels * sizeof (uint32_t)));
        if (raster)
        {
            if (TIFFReadRGBAImageOriented(tif, w, h, raster, ORIENTATION_TOPLEFT, 1))
            {
                Bitmap bitmap(Size(w, h), vcl::PixelFormat::N24_BPP);
                AlphaMask bitmapAlpha(Size(w, h));

                BitmapScopedWriteAccess access(bitmap);
                AlphaScopedWriteAccess accessAlpha(bitmapAlpha);

                for (tools::Long y = 0; y < access->Height(); ++y)
                {
                    const uint32_t* src = raster + w * y;
                    for (tools::Long x = 0; x < access->Width(); ++x)
                    {
                        sal_uInt8 r = TIFFGetR(*src);
                        sal_uInt8 g = TIFFGetG(*src);
                        sal_uInt8 b = TIFFGetB(*src);
                        sal_uInt8 a = TIFFGetA(*src);
                        access->SetPixel(y, x, Color(r, g, b));
                        accessAlpha->SetPixelIndex(y, x, 255 - a);
                        ++src;
                    }
                }

                access.reset();
                accessAlpha.reset();

                BitmapEx aBitmapEx(bitmap, bitmapAlpha);
                AnimationBitmap aAnimationBitmap(aBitmapEx, Point(0, 0), aBitmapEx.GetSizePixel(),
                                                 ANIMATION_TIMEOUT_ON_CLICK, Disposal::Back);
                aAnimation.Insert(aAnimationBitmap);
            }
            _TIFFfree(raster);
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
