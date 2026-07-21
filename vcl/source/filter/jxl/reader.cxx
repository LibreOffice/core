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
#include <filter/JxlReader.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <salinst.hxx>
#include <sal/log.hxx>

#include <jxl/codestream_header.h>
#include <jxl/decode.h>
#include <jxl/decode_cxx.h>
#include <jxl/resizable_parallel_runner.h>
#include <jxl/resizable_parallel_runner_cxx.h>
#include <jxl/types.h>

static bool readJxl(SvStream& stream, Graphic& graphic)
{
    std::vector<uint8_t> data;
    std::vector<uint8_t> pixels;

    size_t xsize = 0;
    size_t ysize = 0;

    // Multi-threaded parallel runner.
    JxlResizableParallelRunnerPtr runner = JxlResizableParallelRunnerMake(nullptr);

    JxlDecoderPtr dec = JxlDecoderMake(nullptr);
    if (JXL_DEC_SUCCESS
        != JxlDecoderSubscribeEvents(dec.get(), JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING
                                                    | JXL_DEC_FULL_IMAGE))
    {
        SAL_WARN("vcl.filter.jxl", "JxlDecoderSubscribeEvents failed");
        return false;
    }

    if (JXL_DEC_SUCCESS
        != JxlDecoderSetParallelRunner(dec.get(), JxlResizableParallelRunner, runner.get()))
    {
        SAL_WARN("vcl.filter.jxl", "JxlDecoderSetParallelRunner failed");
        return false;
    }

    JxlBasicInfo info;
    JxlPixelFormat format = { 4, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0 };

    size_t chunksize = 4096;

    // Read first chunksize bytes.
    data.resize(chunksize);
    sal_Size nBytesRead = stream.ReadBytes(data.data(), chunksize);
    if (nBytesRead <= 0)
        return false; // no bytes to read
    data.resize(nBytesRead);

    if (JXL_DEC_SUCCESS != JxlDecoderSetInput(dec.get(), data.data(), nBytesRead))
    {
        SAL_WARN("vcl.filter.jxl", "JxlDecoderSetInput failed");
        return false;
    }

    // Try using partially read image if there is an error.
    bool success = true;
    for (;;)
    {
        JxlDecoderStatus status = JxlDecoderProcessInput(dec.get());

        if (status == JXL_DEC_ERROR)
        {
            SAL_WARN("vcl.filter.jxl", "Decoder error");
            success = false;
            break;
        }
        else if (status == JXL_DEC_NEED_MORE_INPUT || status == JXL_DEC_SUCCESS
                 || status == JXL_DEC_FULL_IMAGE)
        {
            JxlDecoderReleaseInput(dec.get());
            JxlDecoderFlushImage(dec.get());

            // Read 4096 (more) bytes.
            data.resize(chunksize);
            nBytesRead = stream.ReadBytes(data.data(), chunksize);
            if (nBytesRead <= 0)
            {
                if (status == JXL_DEC_NEED_MORE_INPUT)
                {
                    SAL_WARN("vcl.filter.jxl", "Incomplete/truncated file");
                    success = false;
                    break;
                }
                else
                {
                    // Finished decoding complete file.
                    break;
                }
            }
            data.resize(nBytesRead);

            if (JXL_DEC_SUCCESS != JxlDecoderSetInput(dec.get(), data.data(), nBytesRead))
            {
                SAL_WARN("vcl.filter.jxl", "JxlDecoderSetInput failed");
                success = false;
                break;
            }
        }
        else if (status == JXL_DEC_BASIC_INFO)
        {
            if (JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(dec.get(), &info))
            {
                SAL_WARN("vcl.filter.jxl", "JxlDecoderGetBasicInfo failed");
                success = false;
                break;
            }
            xsize = info.xsize;
            ysize = info.ysize;
            JxlResizableParallelRunnerSetThreads(
                runner.get(), JxlResizableParallelRunnerSuggestThreads(info.xsize, info.ysize));
        }
        else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER)
        {
            size_t buffer_size;
            if (JXL_DEC_SUCCESS != JxlDecoderImageOutBufferSize(dec.get(), &format, &buffer_size))
            {
                SAL_WARN("vcl.filter.jxl", "JxlDecoderImageOutBufferSize failed");
                success = false;
                break;
            }
            if (buffer_size != xsize * ysize * 4)
            {
                SAL_WARN("vcl.filter.jxl", "Invalid out buffer size, expected "
                                               << xsize * ysize * 4 << " but got " << buffer_size);
                success = false;
                break;
            }
            pixels.resize(xsize * ysize * 4);
            if (JXL_DEC_SUCCESS
                != JxlDecoderSetImageOutBuffer(dec.get(), &format, pixels.data(), pixels.size()))
            {
                SAL_WARN("vcl.filter.jxl", "JxlDecoderSetImageOutBuffer failed");
                success = false;
                break;
            }
        }
        else if (status != JXL_DEC_COLOR_ENCODING) // not using color profile
        {
            SAL_WARN("vcl.filter.jxl", "Unknown JxlDecoderStatus");
            success = false;
            break;
        }
    }

    // convert pixels into a bitmap

    Bitmap bitmap(Size(xsize, ysize), vcl::PixelFormat::N32_BPP);
    BitmapScopedWriteAccess access(bitmap);
    if (!access)
        return false;

    for (tools::Long y = 0, nHeight = access->Height(); y < nHeight; ++y)
    {
        const unsigned char* src = pixels.data() + xsize * 4 * y;
        for (tools::Long x = 0, nWidth = access->Width(); x < nWidth; ++x)
        {
            sal_uInt8 r = src[0];
            sal_uInt8 g = src[1];
            sal_uInt8 b = src[2];
            sal_uInt8 a = src[3];
            access->SetPixel(y, x, Color(ColorAlpha, a, r, g, b));
            src += 4;
        }
    }

    access.reset(); // Flush BitmapScopedWriteAccess
    graphic = bitmap;

    return success;
}

bool ImportJxlGraphic(SvStream& rStream, Graphic& rGraphic)
{
    bool bRetValue = readJxl(rStream, rGraphic);
    if (!bRetValue)
        rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
    return bRetValue;
}
