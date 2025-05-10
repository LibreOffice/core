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

#include <algorithm>

#include <vcl/filter/PngImageReader.hxx>
#include <graphic/GraphicFormatDetector.hxx>
#include <graphic/DetectorTools.hxx>
#include <tools/solar.h>
#include <tools/zcodec.hxx>
#include <tools/fract.hxx>
#include <filter/WebpReader.hxx>
#include "igif/gifread.hxx"
#include <vcl/TypeSerializer.hxx>
#include <vcl/outdev.hxx>
#include <utility>

constexpr sal_uInt32 SVG_CHECK_SIZE = 8192;
constexpr sal_uInt32 WMF_EMF_CHECK_SIZE = 44;

namespace
{
class SeekGuard
{
public:
    SeekGuard(SvStream& rStream, sal_uInt64 nStartPosition)
        : mrStream(rStream)
        , mnStartPosition(nStartPosition)
    {
    }
    ~SeekGuard() { mrStream.Seek(mnStartPosition); }

private:
    SvStream& mrStream;
    sal_uInt64 mnStartPosition;
};
}

namespace vcl
{
bool peekGraphicFormat(SvStream& rStream, OUString& rFormatExtension, bool bTest)
{
    vcl::GraphicFormatDetector aDetector(rStream, rFormatExtension);
    if (!aDetector.detect())
        return false;

    // The following variable is used when bTest == true. It remains false
    // if the format (rFormatExtension) has not yet been set.
    bool bSomethingTested = false;

    // Now the different formats are checked. The order *does* matter. e.g. a MET file
    // could also go through the BMP test, however, a BMP file can hardly go through the MET test.
    // So MET should be tested prior to BMP. However, theoretically a BMP file could conceivably
    // go through the MET test. These problems are of course not only in MET and BMP.
    // Therefore, in the case of a format check (bTest == true)  we only test *exactly* this
    // format. Everything else could have fatal consequences, for example if the user says it is
    // a BMP file (and it is a BMP) file, and the file would go through the MET test ...

    if (!bTest || rFormatExtension.startsWith("MET"))
    {
        bSomethingTested = true;
        if (aDetector.checkMET())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("BMP"))
    {
        bSomethingTested = true;
        if (aDetector.checkBMP())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("WMF") || rFormatExtension.startsWith("WMZ")
        || rFormatExtension.startsWith("EMF") || rFormatExtension.startsWith("EMZ"))
    {
        bSomethingTested = true;
        if (aDetector.checkWMF() || aDetector.checkEMF())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("PCX"))
    {
        bSomethingTested = true;
        if (aDetector.checkPCX())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("TIF"))
    {
        bSomethingTested = true;
        if (aDetector.checkTIF())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("GIF"))
    {
        bSomethingTested = true;
        if (aDetector.checkGIF())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("APNG"))
    {
        bSomethingTested = true;
        if (aDetector.checkAPNG())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("PNG"))
    {
        bSomethingTested = true;
        if (aDetector.checkPNG())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("JPG"))
    {
        bSomethingTested = true;
        if (aDetector.checkJPG())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("SVM"))
    {
        bSomethingTested = true;
        if (aDetector.checkSVM())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("PCD"))
    {
        bSomethingTested = true;
        if (aDetector.checkPCD())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("PSD"))
    {
        bSomethingTested = true;
        if (aDetector.checkPSD())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("EPS"))
    {
        bSomethingTested = true;
        if (aDetector.checkEPS())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("DXF"))
    {
        if (aDetector.checkDXF())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("PCT"))
    {
        bSomethingTested = true;
        if (aDetector.checkPCT())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("PBM") || rFormatExtension.startsWith("PGM")
        || rFormatExtension.startsWith("PPM"))
    {
        bSomethingTested = true;
        if (aDetector.checkPBM() || aDetector.checkPGM() || aDetector.checkPPM())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("RAS"))
    {
        bSomethingTested = true;
        if (aDetector.checkRAS())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest)
    {
        bSomethingTested = true;
        if (aDetector.checkXPM())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }
    else if (rFormatExtension.startsWith("XPM"))
    {
        return true;
    }

    if (!bTest)
    {
        if (aDetector.checkXBM())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }
    else if (rFormatExtension.startsWith("XBM"))
    {
        return true;
    }

    if (!bTest)
    {
        if (aDetector.checkSVG())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }
    else if (rFormatExtension.startsWith("SVG"))
    {
        return true;
    }

    if (!bTest || rFormatExtension.startsWith("TGA"))
    {
        bSomethingTested = true;
        if (aDetector.checkTGA())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("MOV"))
    {
        if (aDetector.checkMOV())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("PDF"))
    {
        if (aDetector.checkPDF())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("WEBP"))
    {
        bSomethingTested = true;
        if (aDetector.checkWEBP())
        {
            rFormatExtension = getImportFormatShortName(aDetector.getMetadata().mnFormat);
            return true;
        }
    }

    return bTest && !bSomethingTested;
}

namespace
{
bool isPCT(SvStream& rStream, sal_uLong nStreamPos, sal_uLong nStreamLen)
{
    sal_uInt8 sBuf[3];
    // store number format
    SvStreamEndian oldNumberFormat = rStream.GetEndian();
    sal_uInt32 nOffset; // in MS documents the pict format is used without the first 512 bytes
    for (nOffset = 0; (nOffset <= 512) && ((nStreamPos + nOffset + 14) <= nStreamLen);
         nOffset += 512)
    {
        short y1, x1, y2, x2;
        bool bdBoxOk = true;

        rStream.Seek(nStreamPos + nOffset);
        // size of the pict in version 1 pict ( 2bytes) : ignored
        rStream.SeekRel(2);
        // bounding box (bytes 2 -> 9)
        rStream.SetEndian(SvStreamEndian::BIG);
        rStream.ReadInt16(y1).ReadInt16(x1).ReadInt16(y2).ReadInt16(x2);
        rStream.SetEndian(oldNumberFormat); // reset format

        // read version op
        rStream.ReadBytes(sBuf, 3);

        if (!rStream.good())
            break;

        if (x1 > x2 || y1 > y2 || // bad bdbox
            (x1 == x2 && y1 == y2) || // 1 pixel picture
            x2 - x1 > 2048 || y2 - y1 > 2048) // picture abnormally big
            bdBoxOk = false;

        // see http://developer.apple.com/legacy/mac/library/documentation/mac/pdf/Imaging_With_QuickDraw/Appendix_A.pdf
        // normal version 2 - page A23 and A24
        if (sBuf[0] == 0x00 && sBuf[1] == 0x11 && sBuf[2] == 0x02)
            return true;
        // normal version 1 - page A25
        else if (sBuf[0] == 0x11 && sBuf[1] == 0x01 && bdBoxOk)
            return true;
    }
    return false;
}

} // end anonymous namespace

GraphicFormatDetector::GraphicFormatDetector(SvStream& rStream, OUString aFormatExtension,
                                             bool bExtendedInfo)
    : mrStream(rStream)
    , maExtension(std::move(aFormatExtension))
    , mnFirstLong(0)
    , mnSecondLong(0)
    , mnStreamPosition(0)
    , mnStreamLength(0)
    , mbExtendedInfo(bExtendedInfo)
    , mbWasCompressed(false)
    , maMetadata()
{
}

bool GraphicFormatDetector::detect()
{
    maFirstBytes.clear();
    maFirstBytes.resize(256, 0);

    mnFirstLong = 0;
    mnSecondLong = 0;

    mnStreamPosition = mrStream.Tell();
    mnStreamLength = mrStream.remainingSize();
    SeekGuard aGuard(mrStream, mnStreamPosition);

    if (!mnStreamLength)
    {
        SvLockBytes* pLockBytes = mrStream.GetLockBytes();
        if (pLockBytes)
            pLockBytes->SetSynchronMode();
        mnStreamLength = mrStream.remainingSize();
    }

    if (mnStreamLength == 0)
    {
        return false; // this prevents at least a STL assertion
    }
    else if (mnStreamLength >= maFirstBytes.size())
    {
        // load first 256 bytes into a buffer
        sal_uInt64 nRead = mrStream.ReadBytes(maFirstBytes.data(), maFirstBytes.size());
        if (nRead < maFirstBytes.size())
            mnStreamLength = nRead;
    }
    else
    {
        mnStreamLength = mrStream.ReadBytes(maFirstBytes.data(), mnStreamLength);
    }

    if (mrStream.GetError())
        return false;

    for (int i = 0; i < 4; ++i)
    {
        mnFirstLong = (mnFirstLong << 8) | sal_uInt32(maFirstBytes[i]);
        mnSecondLong = (mnSecondLong << 8) | sal_uInt32(maFirstBytes[i + 4]);
    }
    return true;
}

bool GraphicFormatDetector::checkMET()
{
    if (maFirstBytes[2] != 0xd3)
        return false;
    SeekGuard aGuard(mrStream, mnStreamPosition);
    mrStream.SetEndian(SvStreamEndian::BIG);
    mrStream.Seek(mnStreamPosition);
    sal_uInt16 nFieldSize;
    sal_uInt8 nMagic;

    mrStream.ReadUInt16(nFieldSize).ReadUChar(nMagic);
    for (int i = 0; i < 3; i++)
    {
        if (nFieldSize < 6)
            return false;
        if (mnStreamLength < mrStream.Tell() + nFieldSize)
            return false;
        mrStream.SeekRel(nFieldSize - 3);
        mrStream.ReadUInt16(nFieldSize).ReadUChar(nMagic);
        if (nMagic != 0xd3)
            return false;
    }
    mrStream.SetEndian(SvStreamEndian::LITTLE);

    if (mrStream.GetError())
        return false;

    maMetadata.mnFormat = GraphicFileFormat::MET;
    return true;
}

bool GraphicFormatDetector::checkBMP()
{
    SeekGuard aGuard(mrStream, mnStreamPosition);
    bool bRet = false;
    sal_uInt8 nOffset;

    // We're possibly also able to read an OS/2 bitmap array
    // ('BA'), therefore we must adjust the offset to discover the
    // first bitmap in the array
    if (maFirstBytes[0] == 0x42 && maFirstBytes[1] == 0x41)
        nOffset = 14;
    else
        nOffset = 0;

    // Now we initially test on 'BM'
    if (maFirstBytes[0 + nOffset] == 0x42 && maFirstBytes[1 + nOffset] == 0x4d)
    {
        // OS/2 can set the Reserved flags to a value other than 0
        // (which they really should not do...);
        // In this case we test the size of the BmpInfoHeaders
        if ((maFirstBytes[6 + nOffset] == 0x00 && maFirstBytes[7 + nOffset] == 0x00
             && maFirstBytes[8 + nOffset] == 0x00 && maFirstBytes[9 + nOffset] == 0x00)
            || maFirstBytes[14 + nOffset] == 0x28 || maFirstBytes[14 + nOffset] == 0x0c)
        {
            maMetadata.mnFormat = GraphicFileFormat::BMP;
            bRet = true;
            if (mbExtendedInfo)
            {
                sal_uInt32 nTemp32;
                sal_uInt16 nTemp16;
                sal_uInt32 nCompression;

                mrStream.SetEndian(SvStreamEndian::LITTLE);
                mrStream.Seek(mnStreamPosition + nOffset + 2);

                // up to first info
                mrStream.SeekRel(0x10);

                // Pixel width
                mrStream.ReadUInt32(nTemp32);
                maMetadata.maPixSize.setWidth(nTemp32);

                // Pixel height
                mrStream.ReadUInt32(nTemp32);
                maMetadata.maPixSize.setHeight(nTemp32);

                // Planes
                mrStream.ReadUInt16(nTemp16);
                maMetadata.mnPlanes = nTemp16;

                // BitCount
                mrStream.ReadUInt16(nTemp16);
                maMetadata.mnBitsPerPixel = nTemp16;

                // Compression
                mrStream.ReadUInt32(nTemp32);
                nCompression = nTemp32;

                // logical width
                mrStream.SeekRel(4);
                mrStream.ReadUInt32(nTemp32);
                sal_uInt32 nXPelsPerMeter = 0;
                if (nTemp32)
                {
                    maMetadata.maLogSize.setWidth((maMetadata.maPixSize.Width() * 100000)
                                                  / nTemp32);
                    nXPelsPerMeter = nTemp32;
                }

                // logical height
                mrStream.ReadUInt32(nTemp32);
                sal_uInt32 nYPelsPerMeter = 0;
                if (nTemp32)
                {
                    maMetadata.maLogSize.setHeight((maMetadata.maPixSize.Height() * 100000)
                                                   / nTemp32);
                    nYPelsPerMeter = nTemp32;
                }

                // further validation, check for rational values
                if ((maMetadata.mnBitsPerPixel > 24) || (nCompression > 3))
                {
                    maMetadata.mnFormat = GraphicFileFormat::NOT;
                    bRet = false;
                }

                if (bRet && nXPelsPerMeter && nYPelsPerMeter)
                {
                    maMetadata.maPreferredMapMode
                        = MapMode(MapUnit::MapMM, Point(), Fraction(1000, nXPelsPerMeter),
                                  Fraction(1000, nYPelsPerMeter));

                    maMetadata.maPreferredLogSize
                        = Size(maMetadata.maPixSize.getWidth(), maMetadata.maPixSize.getHeight());
                }
            }
        }
    }
    return bRet;
}

bool GraphicFormatDetector::checkWMF()
{
    sal_uInt64 nCheckSize = std::min<sal_uInt64>(mnStreamLength, 256);
    sal_uInt8 sExtendedOrDecompressedFirstBytes[WMF_EMF_CHECK_SIZE];
    sal_uInt64 nDecompressedSize = nCheckSize;
    // check if it is gzipped -> wmz
    checkAndUncompressBuffer(sExtendedOrDecompressedFirstBytes, WMF_EMF_CHECK_SIZE,
                             nDecompressedSize);
    if (mnFirstLong == 0xd7cdc69a || mnFirstLong == 0x01000900)
    {
        if (mbWasCompressed)
            maMetadata.mnFormat = GraphicFileFormat::WMZ;
        else
            maMetadata.mnFormat = GraphicFileFormat::WMF;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkEMF()
{
    sal_uInt64 nCheckSize = std::min<sal_uInt64>(mnStreamLength, 256);
    sal_uInt8 sExtendedOrDecompressedFirstBytes[WMF_EMF_CHECK_SIZE];
    sal_uInt64 nDecompressedSize = nCheckSize;
    // check if it is gzipped -> emz
    sal_uInt8* pCheckArray = checkAndUncompressBuffer(sExtendedOrDecompressedFirstBytes,
                                                      WMF_EMF_CHECK_SIZE, nDecompressedSize);
    if (mnFirstLong == 0x01000000 && pCheckArray[40] == 0x20 && pCheckArray[41] == 0x45
        && pCheckArray[42] == 0x4d && pCheckArray[43] == 0x46)
    {
        if (mbWasCompressed)
            maMetadata.mnFormat = GraphicFileFormat::EMZ;
        else
            maMetadata.mnFormat = GraphicFileFormat::EMF;
        if (mbExtendedInfo)
        {
            sal_Int32 nBoundLeft = 0, nBoundTop = 0, nBoundRight = 0, nBoundBottom = 0;
            sal_Int32 nFrameLeft = 0, nFrameTop = 0, nFrameRight = 0, nFrameBottom = 0;
            nBoundLeft = pCheckArray[8] | (pCheckArray[9] << 8) | (pCheckArray[10] << 16)
                         | (pCheckArray[11] << 24);
            nBoundTop = pCheckArray[12] | (pCheckArray[13] << 8) | (pCheckArray[14] << 16)
                        | (pCheckArray[15] << 24);
            nBoundRight = pCheckArray[16] | (pCheckArray[17] << 8) | (pCheckArray[18] << 16)
                          | (pCheckArray[19] << 24);
            nBoundBottom = pCheckArray[20] | (pCheckArray[21] << 8) | (pCheckArray[22] << 16)
                           | (pCheckArray[23] << 24);
            nFrameLeft = pCheckArray[24] | (pCheckArray[25] << 8) | (pCheckArray[26] << 16)
                         | (pCheckArray[27] << 24);
            nFrameTop = pCheckArray[28] | (pCheckArray[29] << 8) | (pCheckArray[30] << 16)
                        | (pCheckArray[31] << 24);
            nFrameRight = pCheckArray[32] | (pCheckArray[33] << 8) | (pCheckArray[34] << 16)
                          | (pCheckArray[35] << 24);
            nFrameBottom = pCheckArray[36] | (pCheckArray[37] << 8) | (pCheckArray[38] << 16)
                           | (pCheckArray[39] << 24);
            // size in pixels
            maMetadata.maPixSize.setWidth(nBoundRight - nBoundLeft + 1);
            maMetadata.maPixSize.setHeight(nBoundBottom - nBoundTop + 1);
            // size in 0.01mm units
            maMetadata.maLogSize.setWidth(nFrameRight - nFrameLeft + 1);
            maMetadata.maLogSize.setHeight(nFrameBottom - nFrameTop + 1);
        }
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkPCX()
{
    // ! Because 0x0a can be interpreted as LF too ...
    // we can't be sure that this special sign represent a PCX file only.
    // Every Ascii file is possible here :-(
    // We must detect the whole header.
    bool bRet = false;
    sal_uInt8 cByte = 0;
    SeekGuard aGuard(mrStream, mrStream.Tell());
    mrStream.SetEndian(SvStreamEndian::LITTLE);
    mrStream.ReadUChar(cByte);
    if (cByte == 0x0a)
    {
        maMetadata.mnFormat = GraphicFileFormat::PCX;
        mrStream.SeekRel(1);
        // compression
        mrStream.ReadUChar(cByte);
        bRet = (cByte == 0 || cByte == 1);
        if (bRet)
        {
            // Bits/Pixel
            mrStream.ReadUChar(cByte);
            maMetadata.mnBitsPerPixel = cByte;

            // image dimensions
            sal_uInt16 nXmin(0), nXmax(0), nYmin(0), nYmax(0);
            mrStream.ReadUInt16(nXmin);
            mrStream.ReadUInt16(nYmin);
            mrStream.ReadUInt16(nXmax);
            mrStream.ReadUInt16(nYmax);

            maMetadata.maPixSize.setWidth(nXmax - nXmin + 1);
            maMetadata.maPixSize.setHeight(nYmax - nYmin + 1);

            // resolution
            sal_uInt16 nDPIx(0), nDPIy(0);
            mrStream.ReadUInt16(nDPIx);
            mrStream.ReadUInt16(nDPIy);

            // set logical size
            MapMode aMap(MapUnit::MapInch, Point(), Fraction(1, nDPIx), Fraction(1, nDPIy));
            maMetadata.maLogSize = OutputDevice::LogicToLogic(maMetadata.maPixSize, aMap,
                                                              MapMode(MapUnit::Map100thMM));

            // number of color planes
            cByte = 5; // Illegal value in case of EOF.
            mrStream.SeekRel(49);
            mrStream.ReadUChar(cByte);
            maMetadata.mnPlanes = cByte;

            bRet = (maMetadata.mnPlanes <= 4);
        }
    }
    return bRet;
}

bool GraphicFormatDetector::checkTIF()
{
    SeekGuard aGuard(mrStream, mrStream.Tell());
    mrStream.Seek(mnStreamPosition);
    bool bRet = false;
    sal_uInt8 cByte1 = 0;
    sal_uInt8 cByte2 = 1;

    mrStream.ReadUChar(cByte1);
    mrStream.ReadUChar(cByte2);
    if (cByte1 == cByte2)
    {
        bool bDetectOk = false;

        if (cByte1 == 0x49)
        {
            mrStream.SetEndian(SvStreamEndian::LITTLE);
            bDetectOk = true;
        }
        else if (cByte1 == 0x4d)
        {
            mrStream.SetEndian(SvStreamEndian::BIG);
            bDetectOk = true;
        }

        if (bDetectOk)
        {
            sal_uInt16 nTemp16 = 0;
            sal_uInt32 nTemp32 = 0;

            mrStream.ReadUInt16(nTemp16);
            if (nTemp16 == 0x2a)
            {
                maMetadata.mnFormat = GraphicFileFormat::TIF;
                bRet = true;

                if (mbExtendedInfo)
                {
                    sal_uInt32 nIfdOffset = 0;

                    // Offset of the first IFD
                    mrStream.ReadUInt32(nIfdOffset);
                    mrStream.SeekRel(nIfdOffset - 8); // read 6 bytes until here

                    sal_uInt16 nNumberOfTags = 0;
                    mrStream.ReadUInt16(nNumberOfTags);

                    bool bOk = true;
                    sal_Int32 nCount = 0;

                    // read tags till we find Tag256(Width)
                    mrStream.ReadUInt16(nTemp16);
                    while (nTemp16 != 256 && bOk)
                    {
                        mrStream.SeekRel(10);
                        mrStream.ReadUInt16(nTemp16);
                        nCount++;
                        if (nCount > nNumberOfTags)
                            bOk = false;
                    }

                    if (bOk)
                    {
                        // width
                        mrStream.ReadUInt16(nTemp16);
                        mrStream.SeekRel(4);
                        if (nTemp16 == 3)
                        {
                            mrStream.ReadUInt16(nTemp16);
                            maMetadata.maPixSize.setWidth(nTemp16);
                            mrStream.SeekRel(2);
                        }
                        else
                        {
                            mrStream.ReadUInt32(nTemp32);
                            maMetadata.maPixSize.setWidth(nTemp32);
                        }

                        // height
                        mrStream.SeekRel(2);
                        mrStream.ReadUInt16(nTemp16);
                        mrStream.SeekRel(4);
                        if (nTemp16 == 3)
                        {
                            mrStream.ReadUInt16(nTemp16);
                            maMetadata.maPixSize.setHeight(nTemp16);
                            mrStream.SeekRel(2);
                        }
                        else
                        {
                            mrStream.ReadUInt32(nTemp32);
                            maMetadata.maPixSize.setHeight(nTemp32);
                        }

                        // Bits/Pixel
                        mrStream.ReadUInt16(nTemp16);
                        if (nTemp16 == 258)
                        {
                            mrStream.SeekRel(6);
                            mrStream.ReadUInt16(nTemp16);
                            maMetadata.mnBitsPerPixel = nTemp16;
                            mrStream.SeekRel(2);
                        }
                        else
                            mrStream.SeekRel(-2);

                        // compression
                        mrStream.ReadUInt16(nTemp16);
                        if (nTemp16 == 259)
                        {
                            mrStream.SeekRel(6);
                            mrStream.ReadUInt16(nTemp16); // compression
                            mrStream.SeekRel(2);
                        }
                        else
                            mrStream.SeekRel(-2);
                    }
                }
            }
        }
    }
    return bRet;
}

bool GraphicFormatDetector::checkGIF()
{
    SeekGuard aGuard(mrStream, mnStreamPosition);

    if (mnFirstLong == 0x47494638 && (maFirstBytes[4] == 0x37 || maFirstBytes[4] == 0x39)
        && maFirstBytes[5] == 0x61)
    {
        maMetadata.mnFormat = GraphicFileFormat::GIF;
        if (mbExtendedInfo)
        {
            sal_uInt16 nWidth = maFirstBytes[6] | (maFirstBytes[7] << 8);
            sal_uInt16 nHeight = maFirstBytes[8] | (maFirstBytes[9] << 8);
            maMetadata.maPixSize = Size(nWidth, nHeight);
            maMetadata.mnBitsPerPixel = ((maFirstBytes[10] & 112) >> 4) + 1;

            Size aLogicSize;
            bool bAnimated = IsGIFAnimated(mrStream, aLogicSize);
            if (aLogicSize.getWidth() && aLogicSize.getHeight())
            {
                maMetadata.maLogSize = aLogicSize;
            }
            maMetadata.mbIsAnimated = bAnimated;
        }
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkPNG()
{
    SeekGuard aGuard(mrStream, mnStreamPosition);
    uint64_t nSignature = (static_cast<uint64_t>(mnFirstLong) << 32) | mnSecondLong;
    if (nSignature == PNG_SIGNATURE)
    {
        maMetadata.mnFormat = GraphicFileFormat::PNG;
        if (mbExtendedInfo)
        {
            sal_uInt32 nTemp32;
            mrStream.Seek(mnStreamPosition + PNG_SIGNATURE_SIZE);
            do
            {
                sal_uInt8 cByte = 0;

                // IHDR-Chunk
                mrStream.SeekRel(8);

                // width
                mrStream.ReadUInt32(nTemp32);
                if (!mrStream.good())
                    break;
                maMetadata.maPixSize.setWidth(nTemp32);

                // height
                mrStream.ReadUInt32(nTemp32);
                if (!mrStream.good())
                    break;
                maMetadata.maPixSize.setHeight(nTemp32);

                // Bits/Pixel
                mrStream.ReadUChar(cByte);
                if (!mrStream.good())
                    break;
                maMetadata.mnBitsPerPixel = cByte;

                // Colour type - check whether it supports alpha values
                sal_uInt8 cColType = 0;
                mrStream.ReadUChar(cColType);
                if (!mrStream.good())
                    break;
                maMetadata.mbIsAlpha = maMetadata.mbIsTransparent
                    = (cColType == 4 || cColType == 6);

                // Planes always 1;
                // compression always
                maMetadata.mnPlanes = 1;

                sal_uInt32 nLen32 = 0;
                nTemp32 = 0;

                mrStream.SeekRel(7);

                // read up to the start of the image
                mrStream.ReadUInt32(nLen32);
                mrStream.ReadUInt32(nTemp32);
                while (mrStream.good() && nTemp32 != PNG_IDAT_SIGNATURE)
                {
                    if (nTemp32 == PNG_PHYS_SIGNATURE) // physical pixel dimensions
                    {
                        sal_uLong nXRes;
                        sal_uLong nYRes;

                        // horizontal resolution
                        nTemp32 = 0;
                        mrStream.ReadUInt32(nTemp32);
                        nXRes = nTemp32;

                        // vertical resolution
                        nTemp32 = 0;
                        mrStream.ReadUInt32(nTemp32);
                        nYRes = nTemp32;

                        // unit
                        cByte = 0;
                        mrStream.ReadUChar(cByte);

                        if (cByte)
                        {
                            if (nXRes)
                                maMetadata.maLogSize.setWidth(
                                    (maMetadata.maPixSize.Width() * 100000) / nXRes);

                            if (nYRes)
                                maMetadata.maLogSize.setHeight(
                                    (maMetadata.maPixSize.Height() * 100000) / nYRes);
                        }

                        nLen32 -= 9;
                    }
                    else if (nTemp32 == PNG_TRNS_SIGNATURE) // transparency
                    {
                        maMetadata.mbIsTransparent = true;
                        maMetadata.mbIsAlpha = (cColType != 0 && cColType != 2);
                    }

                    // skip forward to next chunk
                    mrStream.SeekRel(4 + nLen32);
                    mrStream.ReadUInt32(nLen32);
                    mrStream.ReadUInt32(nTemp32);
                }
            } while (false);
        }
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkAPNG()
{
    mrStream.Seek(mnStreamPosition);
    if (PngImageReader::isAPng(mrStream))
    {
        maMetadata.mnFormat = GraphicFileFormat::APNG;
        maMetadata.mbIsAnimated = true;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkJPG()
{
    if ((mnFirstLong == 0xffd8ffe0 && maFirstBytes[6] == 0x4a && maFirstBytes[7] == 0x46
         && maFirstBytes[8] == 0x49 && maFirstBytes[9] == 0x46)
        || (mnFirstLong == 0xffd8fffe) || (0xffd8ff00 == (mnFirstLong & 0xffffff00)))
    {
        maMetadata.mnFormat = GraphicFileFormat::JPG;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkSVM()
{
    sal_uInt32 n32 = 0;
    bool bRet = false;
    SeekGuard aGuard(mrStream, mrStream.Tell());
    mrStream.SetEndian(SvStreamEndian::LITTLE);
    mrStream.ReadUInt32(n32);
    if (n32 == 0x44475653)
    {
        sal_uInt8 cByte = 0;
        mrStream.ReadUChar(cByte);
        if (cByte == 0x49)
        {
            maMetadata.mnFormat = GraphicFileFormat::SVM;
            bRet = true;

            if (mbExtendedInfo)
            {
                sal_uInt32 nTemp32;
                sal_uInt16 nTemp16;

                mrStream.SeekRel(0x04);

                // width
                nTemp32 = 0;
                mrStream.ReadUInt32(nTemp32);
                maMetadata.maLogSize.setWidth(nTemp32);

                // height
                nTemp32 = 0;
                mrStream.ReadUInt32(nTemp32);
                maMetadata.maLogSize.setHeight(nTemp32);

                // read MapUnit and determine PrefSize
                nTemp16 = 0;
                mrStream.ReadUInt16(nTemp16);
                maMetadata.maLogSize = OutputDevice::LogicToLogic(
                    maMetadata.maLogSize, MapMode(static_cast<MapUnit>(nTemp16)),
                    MapMode(MapUnit::Map100thMM));
            }
        }
    }
    else
    {
        mrStream.SeekRel(-4);
        n32 = 0;
        mrStream.ReadUInt32(n32);

        if (n32 == 0x4D4C4356)
        {
            sal_uInt16 nTmp16 = 0;

            mrStream.ReadUInt16(nTmp16);

            if (nTmp16 == 0x4654)
            {
                maMetadata.mnFormat = GraphicFileFormat::SVM;
                bRet = true;

                if (mbExtendedInfo)
                {
                    MapMode aMapMode;
                    mrStream.SeekRel(0x06);
                    TypeSerializer aSerializer(mrStream);
                    aSerializer.readMapMode(aMapMode);
                    aSerializer.readSize(maMetadata.maLogSize);
                    maMetadata.maLogSize = OutputDevice::LogicToLogic(
                        maMetadata.maLogSize, aMapMode, MapMode(MapUnit::Map100thMM));
                }
            }
        }
    }
    return bRet;
}

bool GraphicFormatDetector::checkPCD()
{
    if (mnStreamLength < 2055)
        return false;
    char sBuffer[8];
    SeekGuard aGuard(mrStream, mnStreamPosition);
    mrStream.Seek(mnStreamPosition + 2048);
    sBuffer[mrStream.ReadBytes(sBuffer, 7)] = 0;

    if (strncmp(sBuffer, "PCD_IPI", 7) == 0)
    {
        maMetadata.mnFormat = GraphicFileFormat::PCD;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkPSD()
{
    SeekGuard aGuard(mrStream, mnStreamPosition);
    bool bRet = false;
    if ((mnFirstLong == 0x38425053) && ((mnSecondLong >> 16) == 1))
    {
        maMetadata.mnFormat = GraphicFileFormat::PSD;
        bRet = true;
        if (mbExtendedInfo)
        {
            sal_uInt16 nChannels = 0;
            sal_uInt32 nRows = 0;
            sal_uInt32 nColumns = 0;
            sal_uInt16 nDepth = 0;
            sal_uInt16 nMode = 0;
            mrStream.Seek(mnStreamPosition + 6);
            mrStream.SeekRel(6); // Pad
            mrStream.ReadUInt16(nChannels)
                .ReadUInt32(nRows)
                .ReadUInt32(nColumns)
                .ReadUInt16(nDepth)
                .ReadUInt16(nMode);
            if ((nDepth == 1) || (nDepth == 8) || (nDepth == 16))
            {
                maMetadata.mnBitsPerPixel = (nDepth == 16) ? 8 : nDepth;
                switch (nChannels)
                {
                    case 4:
                    case 3:
                        maMetadata.mnBitsPerPixel = 24;
                        [[fallthrough]];
                    case 2:
                    case 1:
                        maMetadata.maPixSize.setWidth(nColumns);
                        maMetadata.maPixSize.setHeight(nRows);
                        break;
                    default:
                        bRet = false;
                }
            }
            else
                bRet = false;
        }
    }
    return bRet;
}

bool GraphicFormatDetector::checkEPS()
{
    const char* pFirstBytesAsCharArray = reinterpret_cast<char*>(maFirstBytes.data());

    if (mnFirstLong == 0xC5D0D3C6)
    {
        maMetadata.mnFormat = GraphicFileFormat::EPS;
        return true;
    }
    else if (checkArrayForMatchingStrings(pFirstBytesAsCharArray, 30,
                                          { "%!PS-Adobe"_ostr, " EPS"_ostr }))
    {
        maMetadata.mnFormat = GraphicFileFormat::EPS;
        return true;
    }

    return false;
}

bool GraphicFormatDetector::checkDXF()
{
    if (strncmp(reinterpret_cast<char*>(maFirstBytes.data()), "AutoCAD Binary DXF", 18) == 0)
    {
        maMetadata.mnFormat = GraphicFileFormat::DXF;
        return true;
    }

    // ASCII DXF File Format
    int i = 0;
    while (i < 256 && maFirstBytes[i] <= 32)
    {
        ++i;
    }

    if (i < 256 && maFirstBytes[i] == '0')
    {
        ++i;

        // only now do we have sufficient data to make a judgement
        // based on a '0' + 'SECTION' == DXF argument

        while (i < 256 && maFirstBytes[i] <= 32)
        {
            ++i;
        }

        if (i + 7 < 256
            && (strncmp(reinterpret_cast<char*>(maFirstBytes.data() + i), "SECTION", 7) == 0))
        {
            maMetadata.mnFormat = GraphicFileFormat::DXF;
            return true;
        }
    }
    return false;
}

bool GraphicFormatDetector::checkPCT()
{
    SeekGuard aGuard(mrStream, mnStreamPosition);
    if (isPCT(mrStream, mnStreamPosition, mnStreamLength))
    {
        maMetadata.mnFormat = GraphicFileFormat::PCT;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkPBM()
{
    SeekGuard aGuard(mrStream, mrStream.Tell());
    sal_uInt8 nFirst = 0, nSecond = 0, nThird = 0;
    mrStream.ReadUChar(nFirst).ReadUChar(nSecond).ReadUChar(nThird);
    if (nFirst == 'P' && ((nSecond == '1') || (nSecond == '4')) && isspace(nThird))
    {
        maMetadata.mnFormat = GraphicFileFormat::PBM;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkPGM()
{
    sal_uInt8 nFirst = 0, nSecond = 0, nThird = 0;
    SeekGuard aGuard(mrStream, mrStream.Tell());
    mrStream.ReadUChar(nFirst).ReadUChar(nSecond).ReadUChar(nThird);
    if (nFirst == 'P' && ((nSecond == '2') || (nSecond == '5')) && isspace(nThird))
    {
        maMetadata.mnFormat = GraphicFileFormat::PGM;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkPPM()
{
    sal_uInt8 nFirst = 0, nSecond = 0, nThird = 0;
    SeekGuard aGuard(mrStream, mrStream.Tell());
    mrStream.ReadUChar(nFirst).ReadUChar(nSecond).ReadUChar(nThird);
    if (nFirst == 'P' && ((nSecond == '3') || (nSecond == '6')) && isspace(nThird))
    {
        maMetadata.mnFormat = GraphicFileFormat::PPM;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkRAS()
{
    if (mnFirstLong == 0x59a66a95)
    {
        maMetadata.mnFormat = GraphicFileFormat::RAS;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkXPM()
{
    const char* pFirstBytesAsCharArray = reinterpret_cast<char*>(maFirstBytes.data());
    if (matchArrayWithString(pFirstBytesAsCharArray, 256, "/* XPM */"_ostr))
    {
        maMetadata.mnFormat = GraphicFileFormat::XPM;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkXBM()
{
    sal_uInt64 nSize = std::min<sal_uInt64>(mnStreamLength, 2048);
    std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nSize]);

    SeekGuard aGuard(mrStream, mnStreamPosition);
    mrStream.Seek(mnStreamPosition);
    nSize = mrStream.ReadBytes(pBuffer.get(), nSize);

    const char* pBufferAsCharArray = reinterpret_cast<char*>(pBuffer.get());

    if (checkArrayForMatchingStrings(pBufferAsCharArray, nSize, { "#define"_ostr, "_width"_ostr }))
    {
        maMetadata.mnFormat = GraphicFileFormat::XBM;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkSVG()
{
    SeekGuard aGuard(mrStream, mnStreamPosition);
    sal_uInt64 nCheckSize = std::min<sal_uInt64>(mnStreamLength, 256);
    sal_uInt8 sExtendedOrDecompressedFirstBytes[SVG_CHECK_SIZE];
    sal_uInt64 nDecompressedSize = nCheckSize;
    // check if it is gzipped -> svgz
    sal_uInt8* pCheckArray = checkAndUncompressBuffer(sExtendedOrDecompressedFirstBytes,
                                                      SVG_CHECK_SIZE, nDecompressedSize);
    nCheckSize = std::min<sal_uInt64>(nDecompressedSize, 256);
    bool bIsSvg(false);
    bool bIsGZip = mbWasCompressed;
    const char* pCheckArrayAsCharArray = reinterpret_cast<char*>(pCheckArray);
    // check for XML
    // #119176# SVG files which have no xml header at all have shown up this is optional
    // check for "xml" then "version" then "DOCTYPE" and "svg" tags
    if (checkArrayForMatchingStrings(pCheckArrayAsCharArray, nCheckSize,
                                     { "<?xml"_ostr, "version"_ostr, "DOCTYPE"_ostr, "svg"_ostr }))
    {
        bIsSvg = true;
    }

    // check for svg element in 1st 256 bytes
    // search for '<svg'
    if (!bIsSvg
        && checkArrayForMatchingStrings(pCheckArrayAsCharArray, nCheckSize, { "<svg"_ostr }))
    {
        bIsSvg = true;
    }

    // extended search for svg element
    if (!bIsSvg)
    {
        // it's a xml, look for '<svg' in full file. Should not happen too
        // often since the tests above will handle most cases, but can happen
        // with Svg files containing big comment headers or Svg as the host
        // language

        pCheckArrayAsCharArray = reinterpret_cast<char*>(sExtendedOrDecompressedFirstBytes);

        if (bIsGZip)
        {
            nCheckSize = std::min<sal_uInt64>(nDecompressedSize, SVG_CHECK_SIZE);
        }
        else
        {
            nCheckSize = std::min<sal_uInt64>(mnStreamLength, SVG_CHECK_SIZE);
            mrStream.Seek(mnStreamPosition);
            nCheckSize = mrStream.ReadBytes(sExtendedOrDecompressedFirstBytes, nCheckSize);
        }

        // search for '<svg'
        if (checkArrayForMatchingStrings(pCheckArrayAsCharArray, nCheckSize, { "<svg"_ostr }))
        {
            bIsSvg = true;
        }
    }

    if (bIsSvg)
    {
        if (mbWasCompressed)
            maMetadata.mnFormat = GraphicFileFormat::SVGZ;
        else
            maMetadata.mnFormat = GraphicFileFormat::SVG;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkTGA()
{
    SeekGuard aGuard(mrStream, mnStreamPosition);
    // Check TGA ver.2 footer bytes
    if (mnStreamLength > 18)
    {
        char sFooterBytes[18];

        mrStream.Seek(STREAM_SEEK_TO_END);
        mrStream.SeekRel(-18);
        if (mrStream.ReadBytes(sFooterBytes, 18) == 18
            && memcmp(sFooterBytes, "TRUEVISION-XFILE.", SAL_N_ELEMENTS(sFooterBytes)) == 0)
        {
            maMetadata.mnFormat = GraphicFileFormat::TGA;
            return true;
        }
    }

    // Fallback to file extension check
    if (maExtension.startsWith("TGA"))
    {
        maMetadata.mnFormat = GraphicFileFormat::TGA;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkMOV()
{
    if ((maFirstBytes[4] == 'f' && maFirstBytes[5] == 't' && maFirstBytes[6] == 'y'
         && maFirstBytes[7] == 'p' && maFirstBytes[8] == 'q' && maFirstBytes[9] == 't')
        || (maFirstBytes[4] == 'm' && maFirstBytes[5] == 'o' && maFirstBytes[6] == 'o'
            && maFirstBytes[7] == 'v' && maFirstBytes[11] == 'l' && maFirstBytes[12] == 'm'))
    {
        maMetadata.mnFormat = GraphicFileFormat::MOV;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkPDF()
{
    if (maFirstBytes[0] == '%' && maFirstBytes[1] == 'P' && maFirstBytes[2] == 'D'
        && maFirstBytes[3] == 'F' && maFirstBytes[4] == '-')
    {
        maMetadata.mnFormat = GraphicFileFormat::PDF;
        return true;
    }
    return false;
}

bool GraphicFormatDetector::checkWEBP()
{
    if (maFirstBytes[0] == 'R' && maFirstBytes[1] == 'I' && maFirstBytes[2] == 'F'
        && maFirstBytes[3] == 'F' && maFirstBytes[8] == 'W' && maFirstBytes[9] == 'E'
        && maFirstBytes[10] == 'B' && maFirstBytes[11] == 'P')
    {
        maMetadata.mnFormat = GraphicFileFormat::WEBP;
        if (mbExtendedInfo)
        {
            mrStream.Seek(mnStreamPosition);
            ReadWebpInfo(mrStream, maMetadata.maPixSize, maMetadata.mnBitsPerPixel,
                         maMetadata.mbIsAlpha);
            maMetadata.mbIsTransparent = maMetadata.mbIsAlpha;
        }
        return true;
    }
    return false;
}

const GraphicMetadata& GraphicFormatDetector::getMetadata() { return maMetadata; }

sal_uInt8* GraphicFormatDetector::checkAndUncompressBuffer(sal_uInt8* aUncompressedBuffer,
                                                           sal_uInt32 nSize, sal_uInt64& nRetSize)
{
    SeekGuard aGuard(mrStream, mnStreamPosition);
    if (ZCodec::IsZCompressed(mrStream))
    {
        ZCodec aCodec;
        mrStream.Seek(mnStreamPosition);
        aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, /*gzLib*/ true);
        auto nDecompressedOut = aCodec.Read(mrStream, aUncompressedBuffer, nSize);
        // ZCodec::Decompress returns -1 on failure
        nRetSize = nDecompressedOut < 0 ? 0 : nDecompressedOut;
        aCodec.EndCompression();
        // Recalculate first/second long
        for (int i = 0; i < 4; ++i)
        {
            mnFirstLong = (mnFirstLong << 8) | sal_uInt32(aUncompressedBuffer[i]);
            mnSecondLong = (mnSecondLong << 8) | sal_uInt32(aUncompressedBuffer[i + 4]);
        }
        mbWasCompressed = true;
        return aUncompressedBuffer;
    }
    mbWasCompressed = false;
    return maFirstBytes.data();
}

} // vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
