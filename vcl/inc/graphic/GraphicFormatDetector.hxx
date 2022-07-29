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

#ifndef INCLUDED_VCL_INC_GRAPHICFORMATDETECTOR_HXX
#define INCLUDED_VCL_INC_GRAPHICFORMATDETECTOR_HXX

#include <tools/stream.hxx>
#include <vector>
#include <vcl/mapmod.hxx>
#include <tools/gen.hxx>
#include <optional>
//  Info class for all supported file formats
enum class GraphicFileFormat
{
    NOT = 0x0000,
    BMP = 0x0001,
    GIF = 0x0002,
    JPG = 0x0003,
    PCD = 0x0004,
    PCX = 0x0005,
    PNG = 0x0006,
    TIF = 0x0007,
    XBM = 0x0008,
    XPM = 0x0009,
    PBM = 0x000a,
    PGM = 0x000b,
    PPM = 0x000c,
    RAS = 0x000d,
    TGA = 0x000e,
    PSD = 0x000f,
    EPS = 0x0010,
    WEBP = 0x0011,
    MOV = 0x00e0,
    PDF = 0x00e1,
    DXF = 0x00f1,
    MET = 0x00f2,
    PCT = 0x00f3,
    // retired SGF = 0x00f4,
    SVM = 0x00f5,
    WMF = 0x00f6,
    // retired SGV = 0x00f7,
    EMF = 0x00f8,
    SVG = 0x00f9,
    WMZ = 0x00fa,
    EMZ = 0x00fb,
    SVGZ = 0x00fc
};
struct GraphicMetadata
{
    Size maPixSize{};
    Size maLogSize{};
    std::optional<Size> maPreferredLogSize = std::nullopt;
    std::optional<MapMode> maPreferredMapMode = std::nullopt;
    sal_uInt16 mnBitsPerPixel = 0;
    sal_uInt16 mnPlanes = 0;
    GraphicFileFormat mnFormat = GraphicFileFormat::NOT;
    sal_uInt8 mnNumberOfImageComponents = 0;
    bool mbIsTransparent = false;
    bool mbIsAlpha = false;
};
namespace vcl
{
OUString getImportFormatShortName(GraphicFileFormat nFormat)
{
    const char* pKeyName = nullptr;

    switch (nFormat)
    {
        case GraphicFileFormat::BMP:
            pKeyName = "BMP";
            break;
        case GraphicFileFormat::GIF:
            pKeyName = "GIF";
            break;
        case GraphicFileFormat::JPG:
            pKeyName = "JPG";
            break;
        case GraphicFileFormat::PCD:
            pKeyName = "PCD";
            break;
        case GraphicFileFormat::PCX:
            pKeyName = "PCX";
            break;
        case GraphicFileFormat::PNG:
            pKeyName = "PNG";
            break;
        case GraphicFileFormat::XBM:
            pKeyName = "XBM";
            break;
        case GraphicFileFormat::XPM:
            pKeyName = "XPM";
            break;
        case GraphicFileFormat::PBM:
            pKeyName = "PBM";
            break;
        case GraphicFileFormat::PGM:
            pKeyName = "PGM";
            break;
        case GraphicFileFormat::PPM:
            pKeyName = "PPM";
            break;
        case GraphicFileFormat::RAS:
            pKeyName = "RAS";
            break;
        case GraphicFileFormat::TGA:
            pKeyName = "TGA";
            break;
        case GraphicFileFormat::PSD:
            pKeyName = "PSD";
            break;
        case GraphicFileFormat::EPS:
            pKeyName = "EPS";
            break;
        case GraphicFileFormat::TIF:
            pKeyName = "TIF";
            break;
        case GraphicFileFormat::DXF:
            pKeyName = "DXF";
            break;
        case GraphicFileFormat::MET:
            pKeyName = "MET";
            break;
        case GraphicFileFormat::PCT:
            pKeyName = "PCT";
            break;
        case GraphicFileFormat::SVM:
            pKeyName = "SVM";
            break;
        case GraphicFileFormat::WMF:
            pKeyName = "WMF";
            break;
        case GraphicFileFormat::EMF:
            pKeyName = "EMF";
            break;
        case GraphicFileFormat::SVG:
            pKeyName = "SVG";
            break;
        case GraphicFileFormat::WMZ:
            pKeyName = "WMZ";
            break;
        case GraphicFileFormat::EMZ:
            pKeyName = "EMZ";
            break;
        case GraphicFileFormat::SVGZ:
            pKeyName = "SVGZ";
            break;
        case GraphicFileFormat::WEBP:
            pKeyName = "WEBP";
            break;
        case GraphicFileFormat::MOV:
            pKeyName = "MOV";
            break;
        case GraphicFileFormat::PDF:
            pKeyName = "PDF";
            break;
        default:
            assert(false);
    }

    return OUString::createFromAscii(pKeyName);
}
/***
 * This function is has two modes:
 * - determine the file format when bTest = false
 *   returns true, success
 *   out rFormatExtension - on success: file format string
 * - verify file format when bTest = true
 *   returns false, if file type can't be verified
 *           true, if the format is verified or the format is not known
 */
VCL_DLLPUBLIC bool peekGraphicFormat(SvStream& rStream, OUString& rFormatExtension, bool bTest);

class VCL_DLLPUBLIC GraphicFormatDetector
{
public:
    SvStream& mrStream;
    OUString maExtension;

    std::vector<sal_uInt8> maFirstBytes;
    sal_uInt32 mnFirstLong;
    sal_uInt32 mnSecondLong;

    sal_uInt64 mnStreamPosition;
    sal_uInt64 mnStreamLength;

    GraphicFormatDetector(SvStream& rStream, OUString aFormatExtension, bool bExtendedInfo = false);

    bool detect();

    bool checkMET();
    bool checkBMP();
    bool checkWMForEMF();
    bool checkPCX();
    bool checkTIF();
    bool checkGIF();
    bool checkPNG();
    bool checkJPG();
    bool checkSVM();
    bool checkPCD();
    bool checkPSD();
    bool checkEPS();
    bool checkDXF();
    bool checkPCT();
    bool checkPBMorPGMorPPM();
    bool checkRAS();
    bool checkXPM();
    bool checkXBM();
    bool checkSVG();
    bool checkTGA();
    bool checkMOV();
    bool checkPDF();
    bool checkWEBP();
    const GraphicMetadata& getMetadata();

private:
    /**
     * @brief Checks whether mrStream needs to be uncompressed and returns a pointer to the
     * to aUncompressedBuffer or a pointer to maFirstBytes if it doesn't need to be uncompressed
     *
     * @param aUncompressedBuffer the buffer to hold the uncompressed data
     * @param nSize the amount of bytes to uncompress
     * @param nRetSize the amount of bytes actually uncompressed
     * @return sal_uInt8* a pointer to maFirstBytes or aUncompressed buffer
     */
    sal_uInt8* checkAndUncompressBuffer(sal_uInt8* aUncompressedBuffer, sal_uInt32 nSize,
                                        sal_uInt64& nDecompressedSize);
    bool mbExtendedInfo;
    GraphicMetadata maMetadata;
    bool mbWasCompressed;
};
}

#endif // INCLUDED_VCL_INC_GRAPHICFORMATDETECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
