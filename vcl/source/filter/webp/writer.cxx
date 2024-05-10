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

#include <vcl/graph.hxx>
#include <tools/stream.hxx>
#include <filter/WebpWriter.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <comphelper/scopeguard.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>

#include <webp/encode.h>

static int writerFunction(const uint8_t* data, size_t size, const WebPPicture* picture)
{
    SvStream* stream = static_cast<SvStream*>(picture->custom_ptr);
    return stream->WriteBytes(data, size) == size ? 1 : 0;
}

static WebPPreset presetToValue(std::u16string_view preset)
{
    if (o3tl::equalsIgnoreAsciiCase(preset, u"picture"))
        return WEBP_PRESET_PICTURE;
    if (o3tl::equalsIgnoreAsciiCase(preset, u"photo"))
        return WEBP_PRESET_PHOTO;
    if (o3tl::equalsIgnoreAsciiCase(preset, u"drawing"))
        return WEBP_PRESET_DRAWING;
    if (o3tl::equalsIgnoreAsciiCase(preset, u"icon"))
        return WEBP_PRESET_ICON;
    if (o3tl::equalsIgnoreAsciiCase(preset, u"text"))
        return WEBP_PRESET_TEXT;
    return WEBP_PRESET_DEFAULT;
}

static bool writeWebp(SvStream& rStream, const BitmapEx& bitmapEx, bool lossless,
                      std::u16string_view preset, int quality)
{
    WebPConfig config;
    if (!WebPConfigInit(&config))
    {
        SAL_WARN("vcl.filter.webp", "WebPConfigInit() failed");
        return false;
    }
    if (lossless)
    {
        if (!WebPConfigLosslessPreset(&config, 6))
        {
            SAL_WARN("vcl.filter.webp", "WebPConfigLosslessPreset() failed");
            return false;
        }
    }
    else
    {
        if (!WebPConfigPreset(&config, presetToValue(preset), quality))
        {
            SAL_WARN("vcl.filter.webp", "WebPConfigPreset() failed");
            return false;
        }
    }
    // Here various parts of 'config' can be altered if wanted.
    assert(WebPValidateConfig(&config));

    const int width = bitmapEx.GetSizePixel().Width();
    const int height = bitmapEx.GetSizePixel().Height();

    WebPPicture picture;
    if (!WebPPictureInit(&picture))
    {
        SAL_WARN("vcl.filter.webp", "WebPPictureInit() failed");
        return false;
    }
    picture.width = width;
    picture.height = height;
    picture.use_argb = lossless ? 1 : 0; // libwebp recommends argb only for lossless
    comphelper::ScopeGuard freePicture([&picture]() { WebPPictureFree(&picture); });

    // Apparently libwebp needs the entire image data at once in WebPPicture,
    // so allocate it and copy there.
    Bitmap bitmap(bitmapEx.GetBitmap());
    AlphaMask bitmapAlpha;
    if (bitmapEx.IsAlpha())
        bitmapAlpha = bitmapEx.GetAlphaMask();
    BitmapScopedReadAccess access(bitmap);
    BitmapScopedReadAccess accessAlpha(bitmapAlpha);
    bool dataDone = false;
    if (!access->IsBottomUp() && bitmapAlpha.IsEmpty())
    {
        // Try to directly copy the bitmap data.
        switch (access->GetScanlineFormat())
        {
            case ScanlineFormat::N24BitTcRgb:
                if (!WebPPictureImportRGB(&picture, access->GetBuffer(), access->GetScanlineSize()))
                {
                    SAL_WARN("vcl.filter.webp", "WebPPictureImportRGB() failed");
                    return false;
                }
                dataDone = true;
                break;
            case ScanlineFormat::N24BitTcBgr:
                if (!WebPPictureImportBGR(&picture, access->GetBuffer(), access->GetScanlineSize()))
                {
                    SAL_WARN("vcl.filter.webp", "WebPPictureImportBGR() failed");
                    return false;
                }
                dataDone = true;
                break;
            // Our argb formats are premultiplied, so can't read directly using libwebp functions.
            default:
                break;
        }
    }
    if (!dataDone)
    {
        // It would be simpler to convert the bitmap to 32bpp, but our 32bpp support is broken
        // (it's unspecified whether it's premultiplied or not, for example). So handle this manually.
        // Special handling for some common cases, generic otherwise.
        if (!WebPPictureAlloc(&picture))
        {
            SAL_WARN("vcl.filter.webp", "WebPPictureAlloc() failed");
            return false;
        }
        std::vector<uint8_t> data;
        const int bpp = 4;
        data.resize(width * height * bpp);
        if (!bitmapAlpha.IsEmpty())
        {
            for (tools::Long y = 0; y < access->Height(); ++y)
            {
                unsigned char* dst = data.data() + width * bpp * y;
                const sal_uInt8* srcB = access->GetScanline(y);
                const sal_uInt8* srcA = accessAlpha->GetScanline(y);
                for (tools::Long x = 0; x < access->Width(); ++x)
                {
                    BitmapColor color = access->GetPixelFromData(srcB, x);
                    BitmapColor transparency = accessAlpha->GetPixelFromData(srcA, x);
                    color.SetAlpha(255 - transparency.GetIndex());
                    dst[0] = color.GetRed();
                    dst[1] = color.GetGreen();
                    dst[2] = color.GetBlue();
                    dst[3] = color.GetAlpha();
                    dst += bpp;
                }
            }
        }
        else
        {
            for (tools::Long y = 0; y < access->Height(); ++y)
            {
                unsigned char* dst = data.data() + width * bpp * y;
                const sal_uInt8* src = access->GetScanline(y);
                for (tools::Long x = 0; x < access->Width(); ++x)
                {
                    Color color = access->GetPixelFromData(src, x);
                    dst[0] = color.GetRed();
                    dst[1] = color.GetGreen();
                    dst[2] = color.GetBlue();
                    dst[3] = color.GetAlpha();
                    dst += bpp;
                }
            }
        }
        // And now import from the temporary data. Use the import function rather
        // than writing the data directly to avoid the need to write the data
        // in the exact format WebPPicture wants (YUV, etc.).
        if (!WebPPictureImportRGBA(&picture, data.data(), width * bpp))
        {
            SAL_WARN("vcl.filter.webp", "WebPPictureImportRGBA() failed");
            return false;
        }
    }

    picture.writer = writerFunction;
    picture.custom_ptr = &rStream;
    return WebPEncode(&config, &picture);
}

bool ExportWebpGraphic(SvStream& rStream, const Graphic& rGraphic,
                       FilterConfigItem* pFilterConfigItem)
{
    BitmapEx bitmap = rGraphic.GetBitmapEx();
    // If lossless, neither presets nor quality matter.
    bool lossless = pFilterConfigItem->ReadBool(u"Lossless"_ustr, true);
    // Preset is WebPPreset values.
    const OUString preset = pFilterConfigItem->ReadString(u"Preset"_ustr, u""_ustr);
    int quality = pFilterConfigItem->ReadInt32(u"Quality"_ustr, 75);
    return writeWebp(rStream, bitmap, lossless, preset, quality);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
