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

#include <sal/log.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/dibtools.hxx>

#include "emfpimage.hxx"
#include "emfpenums.hxx"

namespace emfplushelper
{
    void EMFPImage::Read(SvMemoryStream &s, sal_uInt32 dataSize, bool bUseWholeStream)
    {
        sal_uInt32 header, bitmapType;
        s.ReadUInt32(header).ReadUInt32(type);
        SAL_INFO("drawinglayer", "EMF+\t\t\tHeader: 0x" << std::hex << header);
        SAL_INFO("drawinglayer", "EMF+\t\t\tType: " << ImageDataTypeToString(type) << " (0x" << type << ")" << std::dec);

        sal_uInt64 imagesize = dataSize - 8;

        if (ImageDataTypeBitmap == type)
        {
            // bitmap
            s.ReadInt32(width).ReadInt32(height).ReadInt32(stride).ReadUInt32(pixelFormat).ReadUInt32(bitmapType);
            SAL_INFO("drawinglayer", "EMF+\t\t\tBitmap width: " << width);
            SAL_INFO("drawinglayer", "EMF+\t\t\tBitmap height: " << height);
            SAL_INFO("drawinglayer", "EMF+\t\t\tBitmap stride: " << stride);
            SAL_INFO("drawinglayer", "EMF+\t\t\tBitmap pixelFormat: " << PixelFormatToString(pixelFormat) << " (0x" << std::hex << pixelFormat << ")");

            imagesize -= 20;

            if (PixelFormatIsCanonical(pixelFormat))
            {
                SAL_INFO("drawinglayer", "EMF+\t\t\t\tCanonical pixel format - 32 bits per pixel, 24-bits for color components, 8-bit alpha channel");
            }
            else
            {
                if (PixelFormatSupportsExtendedColors(pixelFormat))
                    SAL_INFO("drawinglayer", "EMF+\t\t\t\tPixel format supports extended colors in 16-bits per channel");

                if (PixelFormatIsPremultiplied(pixelFormat))
                    SAL_INFO("drawinglayer", "EMF+\t\t\t\tPixel format uses premultiplied alpha");

                if (PixelFormatIncludesAlpha(pixelFormat))
                    SAL_INFO("drawinglayer", "EMF+\t\t\t\tPixel format uses alpha transparency compoent");

                if (PixelFormatGDISupported(pixelFormat))
                    SAL_INFO("drawinglayer", "EMF+\t\t\t\tPixel format supported in Windows GDI");
                else
                    SAL_INFO("drawinglayer", "EMF+\t\t\t\tPixel format not supported in Windows GDI");

                if (PixelFormatUsesPalette(pixelFormat))
                    SAL_INFO("drawinglayer", "EMF+\t\t\t\tPixel values are indexes into a palette");
            }

            SAL_INFO("drawinglayer", "EMF+\t\t\tBits per pixel: " << PixelFormatBitsPerPixel(pixelFormat));
            SAL_INFO("drawinglayer", "EMF+\t\t\tBitmap type: " << " bitmapType: " << BitmapDataTypeToString(bitmapType) << " (0x" << bitmapType << ")" << std::dec);

            SAL_WARN_IF(width == 0 || height == 0, "drawinglayer", "Image width or height is 0");

            if (bitmapType == BitmapDataTypeCompressed)
            {
                SAL_INFO("drawinglayer", "EMF+\t\t\tReading compressed bitmap data");
                GraphicFilter filter;
                filter.ImportGraphic(graphic, OUString(), s);
                SAL_INFO("drawinglayer", "EMF+\t\t\tgraphic width: " << graphic.GetSizePixel().Width() << ", height: " << graphic.GetSizePixel().Height());
            }
            else if (bitmapType == BitmapDataTypePixel)
            {
                SAL_INFO("drawinglayer", "EMF+\t\t\tReading pixel-based bitmap data");

                // we need to read in the palette, then go back to the original stream position
                sal_uInt64 pos = s.Tell();

                EMFPPalette palette;
                if (PixelFormatUsesPalette(pixelFormat))
                {
                    SAL_INFO("drawinglayer", "EMF+\t\t\tUses palette");
                    palette.Read(s);
                }
                else
                {
                    SAL_INFO("drawinglayer", "EMF+\t\t\tDoes not use palette");
                }

                sal_uInt64 palettesize = s.Tell() - pos;

                std::unique_ptr<DIBV5Header> v5header(new DIBV5Header);
                v5header->nWidth = width;
                v5header->nHeight = height;
                v5header->nPlanes = 1;
                v5header->nCompression = false;
                v5header->nSizeImage = ((v5header->nWidth * v5header->nBitCount + 31) & ~31) / 8 * v5header->nHeight;
                v5header->nXPelsPerMeter = 72 * 39.3701;
                v5header->nYPelsPerMeter = 72 * 39.3701;
                v5header->nColsUsed = (PixelFormatUsesPalette(pixelFormat) ? palette.entries.GetEntryCount() : 0);
                v5header->nColsImportant = 0;
                v5header->nBitCount = PixelFormatBitsPerPixel(pixelFormat);
                v5header->nCompression = COMPRESS_NONE;

                // read in the palette into a seperate image stream
                auto buffer = std::make_unique<char[]>(imagesize);
                s.ReadBytes(buffer.get(), palettesize);
                SvMemoryStream imagestream(buffer.get(), imagesize, StreamMode::STD_READWRITE);

                // we are going to have to convert the image data
                if (PixelFormatBitsPerPixel(pixelFormat) == 16)
                {
                    switch (pixelFormat)
                    {
                        case PixelFormat16bppRGB555:
                        case PixelFormat16bppGrayScale:
                        {
                            for (sal_uInt64 i=0; i < s.remainingSize(); i+=2)
                            {
                                sal_uInt16 pixel;
                                s.ReadUInt16(pixel);
                                sal_uInt8 nRed = pixel & 0x001F;
                                sal_uInt8 nGreen = (pixel & 0x03E0) >> 5;
                                sal_uInt8 nBlue = (pixel & 0x7C00) >> 10;

                                imagestream.WriteUInt8(nRed).WriteUInt8(nGreen).WriteUInt8(nBlue);
                            }

                            v5header->nColsUsed = 0;
                            v5header->nBitCount = 24;
                            break;
                        }

                        case PixelFormat16bppRGB565:
                        {
                            for (sal_uInt64 i=0; i < s.remainingSize(); i+=2)
                            {
                                sal_uInt16 pixel;
                                s.ReadUInt16(pixel);
                                sal_uInt8 nRed = pixel & 0x001F;
                                sal_uInt8 nGreen = (pixel & 0x07E0) >> 5;
                                sal_uInt8 nBlue = (pixel & 0xF800) >> 10;

                                imagestream.WriteUInt8(nRed).WriteUInt8(nGreen).WriteUInt8(nBlue);
                            }

                            v5header->nColsUsed = 0;
                            v5header->nBitCount = 24;
                            break;
                        }

                        case PixelFormat16bppARGB1555:
                        {
                            for (sal_uInt64 i=0; i < s.remainingSize(); i+=2)
                            {
                                sal_uInt16 pixel;
                                s.ReadUInt16(pixel);
                                sal_uInt8 nAlpha = pixel & 1;
                                sal_uInt8 nRed = (pixel & 0x003E) >> 1;
                                sal_uInt8 nGreen = (pixel & 0x07C0) >> 6;
                                sal_uInt8 nBlue = (pixel & 0xF800) >> 11;

                                imagestream.WriteUInt8(nAlpha).WriteUInt8(nRed).WriteUInt8(nGreen).WriteUInt8(nBlue);
                            }

                            v5header->nColsUsed = 0;
                            v5header->nBitCount = 32;
                            break;
                        }

                        default:
                            SAL_WARN("drawinglayer", "Unknown 16 bit pixel format");
                    }
                }

                Bitmap aBrushBmp(Size(width, height), v5header->nBitCount);
                AlphaMask aMask;

                if (!ReadDIBV5(aBrushBmp, aMask, imagestream, v5header.get()))
                    SAL_WARN("drawinglayer", "EMF+\t\t\tCannot read bitmap data");

                SAL_INFO("drawinglayer", "EMF+\t\t\tBitmap size: " << aBrushBmp.GetSizePixel());
                SAL_INFO("drawinglayer", "EMF+\t\t\tBitmap bit count: " << aBrushBmp.GetBitCount());
                SAL_INFO("drawinglayer", "EMF+\t\t\tBitmap colors: " << aBrushBmp.GetColorCount());
                SAL_INFO("drawinglayer", "EMF+\t\t\tBitmap byte size: " << aBrushBmp.GetSizeBytes());
            }
            else
            {
                SAL_WARN("drawinglayer", "EMF+\t\t\tInvalid bitmap data type");
            }
        }
        else if (ImageDataTypeMetafile == type)
        {
            // metafile
            sal_uInt32 mfType, mfSize;
            s.ReadUInt32(mfType).ReadUInt32(mfSize);

            if (bUseWholeStream)
                dataSize = s.remainingSize();
            else
                dataSize -= 16;

            SAL_INFO("drawinglayer", "EMF+\tmetafile type: " << mfType << " dataSize: " << mfSize << " real size calculated from record dataSize: " << dataSize);

            GraphicFilter filter;
            // workaround buggy metafiles, which have wrong mfSize set (n#705956 for example)
            SvMemoryStream mfStream(const_cast<char *>(static_cast<char const *>(s.GetData()) + s.Tell()), dataSize, StreamMode::READ);
            filter.ImportGraphic(graphic, OUString(), mfStream);

            // debug code - write the stream to debug file /tmp/emf-stream.emf
#if OSL_DEBUG_LEVEL > 1
            mfStream.Seek(0);
            static sal_Int32 emfp_debug_stream_number = 0;
            OUString emfp_debug_filename = "/tmp/emf-embedded-stream" +
                OUString::number(emfp_debug_stream_number++) + ".emf";

            SvFileStream file(emfp_debug_filename, StreamMode::WRITE | StreamMode::TRUNC);

            mfStream.WriteStream(file);
            file.Flush();
            file.Close();
#endif
        }

    }

    void EMFPPalette::Read(SvMemoryStream &s)
    {
        sal_uInt32 count=0;
        s.ReadUInt32(flags).ReadUInt32(count);
        SAL_INFO("drawinglayer", "EMF+\t\t\tPalette flags: 0x" << std::hex << flags);
        SAL_INFO("drawinglayer", "EMF+\t\t\tPalette entries: " << std::dec << count);

        entries.SetEntryCount(count);
    }

    void EMFPARGB::Read(SvMemoryStream &s)
    {
        sal_uInt8 alpha, red, green, blue;
        s.ReadUChar(blue).ReadUChar(green).ReadUChar(red).ReadUChar(alpha);

        color.SetTransparency(alpha);
        color.SetRed(red);
        color.SetGreen(green);
        color.SetBlue(blue);
        SAL_INFO("drawinglayer", "EMF+\t\t\t\tPalette entry (ARGB): 0x" << std::hex << alpha << red << green << blue);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
