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

#include <memory>
#include <vcl/graph.hxx>
#include <tools/stream.hxx>
#include <filter/WebpReader.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <salinst.hxx>
#include <sal/log.hxx>
#include <comphelper/configuration.hxx>
#include <svdata.hxx>
#include <comphelper/scopeguard.hxx>

#include <webp/decode.h>

static bool readWebpInfo(SvStream& stream, std::vector<uint8_t>& data,
                         WebPBitstreamFeatures& features)
{
    for (;;)
    {
        // Read 4096 (more) bytes.
        size_t lastSize = data.size();
        data.resize(data.size() + 4096);
        sal_Size nBytesRead = stream.ReadBytes(data.data() + lastSize, 4096);
        if (nBytesRead <= 0)
            return false;
        data.resize(lastSize + nBytesRead);
        int status = WebPGetFeatures(data.data(), data.size(), &features);
        if (status == VP8_STATUS_OK)
            break;
        if (status == VP8_STATUS_NOT_ENOUGH_DATA)
            continue; // Try again with 4096 more bytes read.
        return false;
    }
    return true;
}

static bool readWebp(SvStream& stream, Graphic& graphic)
{
    WebPDecoderConfig config;
    if (!WebPInitDecoderConfig(&config))
    {
        SAL_WARN("vcl.filter.webp", "WebPInitDecoderConfig() failed");
        return false;
    }
    comphelper::ScopeGuard freeBuffer([&config]() { WebPFreeDecBuffer(&config.output); });
    std::vector<uint8_t> data;
    if (!readWebpInfo(stream, data, config.input))
        return false;
    // Here various parts of 'config' can be altered if wanted.
    const int& width = config.input.width;
    const int& height = config.input.height;
    const int& has_alpha = config.input.has_alpha;

    if (width > SAL_MAX_INT32 / 8 || height > SAL_MAX_INT32 / 8)
        return false; // avoid overflows later

    const bool bFuzzing = comphelper::IsFuzzing();
    const bool bSupportsBitmap32 = bFuzzing || ImplGetSVData()->mpDefInst->supportsBitmap32();

    Bitmap bitmap;
    AlphaMask bitmapAlpha;
    if (bSupportsBitmap32 && has_alpha)
    {
        bitmap = Bitmap(Size(width, height), vcl::PixelFormat::N32_BPP);
    }
    else
    {
        bitmap = Bitmap(Size(width, height), vcl::PixelFormat::N24_BPP);
        if (has_alpha)
            bitmapAlpha = AlphaMask(Size(width, height));
    }

    BitmapScopedWriteAccess access(bitmap);
    if (!access)
        return false;
    // If data cannot be read directly into the bitmap, read data first to this buffer and then convert.
    std::vector<uint8_t> tmpRgbaData;
    enum class PixelMode
    {
        DirectRead, // read data directly to the bitmap
        Split, // read to tmp buffer and split to rgb and alpha
        SetPixel // read to tmp buffer and use setPixel()
    };
    PixelMode pixelMode = PixelMode::SetPixel;

    config.output.width = width;
    config.output.height = height;
    config.output.is_external_memory = 1;
    if (bSupportsBitmap32 && has_alpha)
    {
        switch (access->GetScanlineFormat())
        {
            // Our bitmap32 code expects premultiplied.
            case ScanlineFormat::N32BitTcRgba:
                config.output.colorspace = MODE_rgbA;
                pixelMode = PixelMode::DirectRead;
                break;
            case ScanlineFormat::N32BitTcBgra:
                config.output.colorspace = MODE_bgrA;
                pixelMode = PixelMode::DirectRead;
                break;
            case ScanlineFormat::N32BitTcArgb:
                config.output.colorspace = MODE_Argb;
                pixelMode = PixelMode::DirectRead;
                break;
            default:
                config.output.colorspace = MODE_RGBA;
                pixelMode = PixelMode::SetPixel;
                break;
        }
    }
    else
    {
        if (has_alpha)
        {
            switch (access->GetScanlineFormat())
            {
                case ScanlineFormat::N24BitTcRgb:
                    config.output.colorspace = MODE_RGBA;
                    pixelMode = PixelMode::Split;
                    break;
                case ScanlineFormat::N24BitTcBgr:
                    config.output.colorspace = MODE_BGRA;
                    pixelMode = PixelMode::Split;
                    break;
                default:
                    config.output.colorspace = MODE_RGBA;
                    pixelMode = PixelMode::SetPixel;
                    break;
            }
        }
        else
        {
            switch (access->GetScanlineFormat())
            {
                case ScanlineFormat::N24BitTcRgb:
                    config.output.colorspace = MODE_RGB;
                    pixelMode = PixelMode::DirectRead;
                    break;
                case ScanlineFormat::N24BitTcBgr:
                    config.output.colorspace = MODE_BGR;
                    pixelMode = PixelMode::DirectRead;
                    break;
                default:
                    config.output.colorspace = MODE_RGBA;
                    pixelMode = PixelMode::SetPixel;
                    break;
            }
        }
    }
    if (pixelMode == PixelMode::DirectRead)
    {
        config.output.u.RGBA.rgba = access->GetBuffer();
        config.output.u.RGBA.stride = access->GetScanlineSize();
        config.output.u.RGBA.size = access->GetScanlineSize() * access->Height();
    }
    else
    {
        tmpRgbaData.resize(width * height * 4);
        config.output.u.RGBA.rgba = tmpRgbaData.data();
        config.output.u.RGBA.stride = width * 4;
        config.output.u.RGBA.size = tmpRgbaData.size();
    }

    std::unique_ptr<WebPIDecoder, decltype(&WebPIDelete)> decoder(WebPIDecode(nullptr, 0, &config),
                                                                  WebPIDelete);

    bool success = true;
    for (;;)
    {
        // During first iteration, use data read while reading the header.
        int status = WebPIAppend(decoder.get(), data.data(), data.size());
        if (status == VP8_STATUS_OK)
            break;
        if (status != VP8_STATUS_SUSPENDED)
        {
            // An error, still try to return at least a partially read bitmap,
            // even if returning an error flag.
            success = false;
            break;
        }
        // If more data is needed, reading 4096 bytes more and repeat.
        data.resize(4096);
        sal_Size nBytesRead = stream.ReadBytes(data.data(), 4096);
        if (nBytesRead <= 0)
        {
            // Truncated file, again try to return at least something.
            success = false;
            break;
        }
        data.resize(nBytesRead);
    }

    switch (pixelMode)
    {
        case PixelMode::DirectRead:
        {
            // Adjust for IsBottomUp() if necessary.
            if (access->IsBottomUp())
            {
                std::vector<char> tmp;
                const sal_uInt32 lineSize = access->GetScanlineSize();
                tmp.resize(lineSize);
                for (tools::Long y = 0; y < access->Height() / 2; ++y)
                {
                    tools::Long otherY = access->Height() - 1 - y;
                    memcpy(tmp.data(), access->GetScanline(y), lineSize);
                    memcpy(access->GetScanline(y), access->GetScanline(otherY), lineSize);
                    memcpy(access->GetScanline(otherY), tmp.data(), lineSize);
                }
            }
            break;
        }
        case PixelMode::Split:
        {
            // Split to normal and alpha bitmaps.
            BitmapScopedWriteAccess accessAlpha(bitmapAlpha);
            for (tools::Long y = 0; y < access->Height(); ++y)
            {
                const unsigned char* src = tmpRgbaData.data() + width * 4 * y;
                unsigned char* dstB = access->GetScanline(y);
                unsigned char* dstA = accessAlpha->GetScanline(y);
                for (tools::Long x = 0; x < access->Width(); ++x)
                {
                    memcpy(dstB, src, 3);
                    *dstA = *(src + 3);
                    src += 4;
                    dstB += 3;
                    dstA += 1;
                }
            }
            break;
        }
        case PixelMode::SetPixel:
        {
            for (tools::Long y = 0; y < access->Height(); ++y)
            {
                const unsigned char* src = tmpRgbaData.data() + width * 4 * y;
                for (tools::Long x = 0; x < access->Width(); ++x)
                {
                    sal_uInt8 r = src[0];
                    sal_uInt8 g = src[1];
                    sal_uInt8 b = src[2];
                    sal_uInt8 a = src[3];
                    access->SetPixel(y, x, Color(ColorAlpha, a, r, g, b));
                    src += 4;
                }
            }
            if (!bitmapAlpha.IsEmpty())
            {
                BitmapScopedWriteAccess accessAlpha(bitmapAlpha);
                for (tools::Long y = 0; y < accessAlpha->Height(); ++y)
                {
                    const unsigned char* src = tmpRgbaData.data() + width * 4 * y;
                    for (tools::Long x = 0; x < accessAlpha->Width(); ++x)
                    {
                        sal_uInt8 a = src[3];
                        accessAlpha->SetPixelIndex(y, x, a);
                        src += 4;
                    }
                }
            }
            break;
        }
    }

    access.reset(); // Flush BitmapScopedWriteAccess.
    if (bSupportsBitmap32 && has_alpha)
        graphic = BitmapEx(bitmap);
    else
    {
        if (has_alpha)
            graphic = BitmapEx(bitmap, bitmapAlpha);
        else
            graphic = BitmapEx(bitmap);
    }
    return success;
}

bool ImportWebpGraphic(SvStream& rStream, Graphic& rGraphic)
{
    bool bRetValue = readWebp(rStream, rGraphic);
    if (!bRetValue)
        rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
    return bRetValue;
}

bool ReadWebpInfo(SvStream& stream, Size& pixelSize, sal_uInt16& bitsPerPixel, bool& hasAlpha)
{
    std::vector<uint8_t> data;
    WebPBitstreamFeatures features;
    if (!readWebpInfo(stream, data, features))
        return false;
    pixelSize = Size(features.width, features.height);
    bitsPerPixel = features.has_alpha ? 32 : 24;
    hasAlpha = features.has_alpha;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
