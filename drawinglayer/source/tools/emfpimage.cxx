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
#include <vcl/graphicfilter.hxx>
#include <sal/log.hxx>
#include "emfpimage.hxx"

namespace emfplushelper
{
    void EMFPImage::Read(SvMemoryStream &s, sal_uInt32 dataSize, bool bUseWholeStream)
    {
        sal_uInt32 header(0), bitmapType(0);
        s.ReadUInt32(header).ReadUInt32(type);
        SAL_INFO("drawinglayer.emf", "EMF+\timage\nEMF+\theader: 0x" << std::hex << header << " type: " << type << std::dec);

        if (ImageDataTypeBitmap == type)
        {
            // bitmap
            s.ReadInt32(width).ReadInt32(height).ReadInt32(stride).ReadUInt32(pixelFormat).ReadUInt32(bitmapType);
            SAL_INFO("drawinglayer.emf", "EMF+\tbitmap width: " << width << " height: " << height << " stride: " << stride << " pixelFormat: 0x" << std::hex << pixelFormat << " bitmapType: 0x" << bitmapType << std::dec);

            if ((bitmapType != 0) || (width == 0))
            {
                // non native formats
                GraphicFilter filter;
                filter.ImportGraphic(graphic, u"", s);
                SAL_INFO("drawinglayer.emf", "EMF+\tbitmap width: " << graphic.GetSizePixel().Width() << " height: " << graphic.GetSizePixel().Height());
            }
        }
        else if (ImageDataTypeMetafile == type)
        {
            // metafile
            sal_uInt32 mfType(0), mfSize(0);
            s.ReadUInt32(mfType).ReadUInt32(mfSize);

            if (bUseWholeStream)
                dataSize = s.remainingSize();
            else
            {
                if (dataSize < 16)
                    return;
                dataSize -= 16;
            }

            SAL_INFO("drawinglayer.emf", "EMF+\tmetafile type: " << mfType << " dataSize: " << mfSize << " real size calculated from record dataSize: " << dataSize);

            GraphicFilter filter;
            // workaround buggy metafiles, which have wrong mfSize set (n#705956 for example)
            SvMemoryStream mfStream(const_cast<char *>(static_cast<char const *>(s.GetData()) + s.Tell()), dataSize, StreamMode::READ);
            filter.ImportGraphic(graphic, u"", mfStream);

            mfStream.Seek(0);
            // 1 = Wmf, 2 = WmfPlaceable, 3 = Emf, 4 = EmfPlusOnly, 5 = EmfPlusDual
            if (mfType == 3 || mfType == 4 || mfType == 5)
            {
                sal_uInt32 dwRecordType, dwRecordSize, dSignature, nVersion;
                sal_Int32 rclFrameLeft, rclFrameTop, rclFrameRight, rclFrameBottom;

                mfStream.ReadUInt32(dwRecordType);
                mfStream.ReadUInt32(dwRecordSize);
                mfStream.SeekRel(16);

                mfStream.ReadInt32(rclFrameLeft);
                mfStream.ReadInt32(rclFrameTop);
                mfStream.ReadInt32(rclFrameRight);
                mfStream.ReadInt32(rclFrameBottom);

                mfStream.ReadUInt32(dSignature);
                mfStream.ReadUInt32(nVersion);

                if (dwRecordType == 1 && dSignature == 0x464D4520)
                {
                    mfStream.SeekRel(24);

                    sal_Int32 szlDeviceWidth, szlDeviceHeight;
                    sal_Int32 szlMillimetersWidth, szlMillimetersHeight;

                    mfStream.ReadInt32(szlDeviceWidth);
                    mfStream.ReadInt32(szlDeviceHeight);
                    mfStream.ReadInt32(szlMillimetersWidth);
                    mfStream.ReadInt32(szlMillimetersHeight);

                    const double fPxPerMmX
                        = (szlMillimetersWidth > 0)
                            ? static_cast<double>(szlDeviceWidth) / szlMillimetersWidth
                            : 3.7795;
                    const double fPxPerMmY
                        = (szlMillimetersHeight > 0)
                            ? static_cast<double>(szlDeviceHeight) / szlMillimetersHeight
                            : 3.7795;

                    x = std::round(fPxPerMmX * rclFrameLeft / 100.0);
                    y = std::round(fPxPerMmY * rclFrameTop / 100.0);
                    width = std::round(fPxPerMmX * (rclFrameRight - rclFrameLeft) / 100.0);
                    height = std::round(fPxPerMmY * (rclFrameBottom - rclFrameTop) / 100.0);

                    SAL_INFO("drawinglayer.emf", "EMF+\tNested EMF Header detected.");
                    SAL_INFO("drawinglayer.emf",
                             "EMF+\tDevicePx: " << szlDeviceWidth << "x" << szlDeviceHeight
                                                << ", DeviceMm: " << szlMillimetersWidth << "x"
                                                << szlMillimetersHeight);
                    SAL_INFO("drawinglayer.emf", "EMF+\tfPxPerMmX: " << fPxPerMmX << "x" << fPxPerMmY);
                    SAL_INFO("drawinglayer.emf", "EMF+\tFrame [0.01mm]: ["
                                                 << rclFrameLeft << ", " << rclFrameTop << ", "
                                                 << rclFrameRight << ", " << rclFrameBottom << "]");
                    SAL_INFO("drawinglayer.emf",
                             "EMF+\tVersion: 0x" << std::hex << nVersion << std::dec);
                    SAL_INFO("drawinglayer.emf", "EMF+\tCalculated Image Canvas (Pixels): Pos("
                                                 << x << ", " << y << "), Size(" << width << "x"
                                                 << height << ")");
                }
            }


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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
