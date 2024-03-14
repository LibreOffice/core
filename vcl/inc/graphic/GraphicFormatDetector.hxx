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

#pragma once

#include <tools/stream.hxx>
#include <vector>
#include <vcl/graphic/GraphicMetadata.hxx>

namespace vcl
{
static inline OUString getImportFormatShortName(GraphicFileFormat nFormat)
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
        case GraphicFileFormat::APNG:
            pKeyName = "APNG";
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
    bool checkWMF();
    bool checkEMF();
    bool checkPCX();
    bool checkTIF();
    bool checkGIF();
    bool checkPNG();
    bool checkAPNG();
    bool checkJPG();
    SAL_DLLPRIVATE bool checkSVM();
    SAL_DLLPRIVATE bool checkPCD();
    bool checkPSD();
    bool checkEPS();
    SAL_DLLPRIVATE bool checkDXF();
    SAL_DLLPRIVATE bool checkPCT();
    SAL_DLLPRIVATE bool checkPBM();
    SAL_DLLPRIVATE bool checkPGM();
    SAL_DLLPRIVATE bool checkPPM();
    SAL_DLLPRIVATE bool checkRAS();
    bool checkXPM();
    bool checkXBM();
    bool checkSVG();
    bool checkTGA();
    SAL_DLLPRIVATE bool checkMOV();
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
    SAL_DLLPRIVATE sal_uInt8* checkAndUncompressBuffer(sal_uInt8* aUncompressedBuffer,
                                                       sal_uInt32 nSize,
                                                       sal_uInt64& nDecompressedSize);
    bool mbExtendedInfo;
    bool mbWasCompressed;
    GraphicMetadata maMetadata;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
