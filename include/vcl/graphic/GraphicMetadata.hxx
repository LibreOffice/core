/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_VCL_GRAPHIC_GRAPHICMETADATA_HXX
#define INCLUDED_VCL_GRAPHIC_GRAPHICMETADATA_HXX

#include <tools/gen.hxx>
#include <vcl/mapmod.hxx>

#include <optional>

// Info class for all supported file formats
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
    SVGZ = 0x00fc,
    APNG = 0x00fd
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
    bool mbIsAnimated = false;
};
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
