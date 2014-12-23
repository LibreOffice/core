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

#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/outdev.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphictools.hxx>
#include <unotools/fontdefs.hxx>
#include <vcl/emfact.hxx>

#include <unordered_map>

namespace
{
    const char *
    emf_action_name(sal_uInt16 nEMFAction)
    {
#ifndef SAL_LOG_INFO
        (void) nEMFAction;
        return "";
#else
        switch( nEMFAction )
        {
        // Control action types
        case EMR_HEADER_ACTION:                 return "HEADER";
        case EMR_EOF_ACTION:                    return "EOF";

        // Comment action types
        case EMR_COMMENT_ACTION:                return "COMMENT";

        // Bitmap action types
        case EMR_BITBLT_ACTION:                 return "BITBLT";
        case EMR_STRETCHBLT_ACTION:             return "STRETCHBLT";
        case EMR_MASKBLT_ACTION:                return "MASKBLT";
        case EMR_SETDIBITSTODEVICE_ACTION:      return "SETDIBITSTODEVICE";
        case EMR_STRETCHDIBITS_ACTION:          return "STRETCHDIBITS";
        case EMR_ALPHABLEND_ACTION:             return "ALPHABLEND";
        case EMR_TRANSPARENTBLT_ACTION:         return "TRANSPARENTBLT";

        // Layout action type
        case EMR_SETLAYOUT_ACTION:              return "SETLAYOUT";

        // State action types
        case EMR_SAVEDC_ACTION:                 return "SAVEDC";
        case EMR_RESTOREDC_ACTION:              return "RESTOREDC";
        case EMR_SETICMMODE_ACTION:             return "SETICMMODE";
        case EMR_COLORMATCHTOTARGETW_ACTION:    return "COLORMATCHTOTARGETW";
        case EMR_MOVETOEX_ACTION:               return "MOVETOEX";
        case EMR_PIXELFORMAT_ACTION:            return "PIXELFORMAT";
        case EMR_SCALEVIEWPORTEXTEX_ACTION:     return "SCALEVIEWPORTEXTEX";
        case EMR_SCALEWINDOWEXTEX_ACTION:       return "SCALEWINDOWEXTEX";
        case EMR_SETARCDIRECTION_ACTION:        return "SETARCDIRECTION";
        case EMR_SETBKCOLOR_ACTION:             return "SETBKCOLOR";
        case EMR_SETBKMODE_ACTION:              return "SETBKMODE";
        case EMR_SETBRUSHORGEX_ACTION:          return "SETBRUSHORGEX";
        case EMR_SETCOLORADJUSTMENT_ACTION:     return "SETCOLORADJUSTMENT";
        case EMR_SETICMPROFILEA_ACTION:         return "SETICMPROFILEA";
        case EMR_SETICMPROFILEW_ACTION:         return "SETICMPROFILEW";
        case EMR_SETLINKEDUFIS_ACTION:          return "SETLINKEDUFIS";
        case EMR_SETMAPMODE_ACTION:             return "SETMAPMODE";
        case EMR_SETMAPPERFLAGS_ACTION:         return "SETMAPPERFLAGS";
        case EMR_SETMITERLIMIT_ACTION:          return "SETMITERLIMIT";
        case EMR_SETPOLYFILLMODE_ACTION:        return "SETPOLYFILLMODE";
        case EMR_SETROP2_ACTION:                return "SETROP2";
        case EMR_SETSTRETCHBLTMODE_ACTION:      return "SETSTRETCHBLTMODE";
        case EMR_SETTEXTALIGN_ACTION:           return "SETTEXTALIGN";
        case EMR_SETTEXTCOLOR_ACTION:           return "SETTEXTCOLOR";
        case EMR_SETTEXTJUSTIFICATION_ACTION:   return "SETTEXTJUSTIFICATION";
        case EMR_SETVIEWPORTEXTEX_ACTION:       return "SETVIEWPORTEXTEX";
        case EMR_SETVIEWPORTORGEX_ACTION:       return "SETVIEWPORTORGEX";
        case EMR_SETWINDOWEXTEX_ACTION:         return "SETWINDOWEXTEX";
        case EMR_SETWINDOWORGEX_ACTION:         return "SETWINDOWORGEX";

        // Clipping action types
        case EMR_OFFSETCLIPRGN_ACTION:          return "OFFSETCLIPRGN";
        case EMR_SETMETARGN_ACTION:             return "SETMETARGN";
        case EMR_EXCLUDECLIPRECT_ACTION:        return "EXCLUDECLIPRECT";
        case EMR_INTERSECTCLIPRECT_ACTION:      return "INTERSECTCLIPRECT";
        case EMR_SELECTCLIPPATH_ACTION:         return "SELECTCLIPPATH";
        case EMR_EXTSELECTCLIPRGN_ACTION:       return "EXTSELECTCLIPRGN";

        // Transform action types
        case EMR_SETWORLDTRANSFORM_ACTION:      return "SETWORLDTRANSFORM";
        case EMR_MODIFYWORLDTRANSFORM_ACTION:   return "MODIFYWORLDTRANSFORM";

        // Object creation action types
        case EMR_CREATEBRUSHINDIRECT_ACTION:    return "CREATEBRUSHINDIRECT";
        case EMR_CREATECOLORSPACE_ACTION:       return "CREATECOLORSPACE";
        case EMR_CREATECOLORSPACEW_ACTION:      return "CREATECOLORSPACEW";
        case EMR_EXTCREATEFONTINDIRECTW_ACTION: return "EXTCREATEFONTINDIRECTW";
        case EMR_CREATEDIBPATTERNBRUSHPT_ACTION: return "CREATEDIBPATTERNBRUSHPT";
        case EMR_CREATEPEN_ACTION:              return "CREATEPEN";
        case EMR_CREATEPALETTE_ACTION:          return "CREATEPALETTE";
        case EMR_EXTCREATEPEN_ACTION:           return "EXTCREATEPEN";

        // Object manipulation action types
        case EMR_COLORCORRECTPALETTE_ACTION:    return "COLORCORRECTPALETTE";
        case EMR_DELETECOLORSPACE_ACTION:       return "DELETECOLORSPACE";
        case EMR_DELETEOBJECT_ACTION:           return "DELETEOBJECT";
        case EMR_RESIZEPALETTE_ACTION:          return "RESIZEPALETTE";
        case EMR_SELECTOBJECT_ACTION:           return "SELECTOBJECT";
        case EMR_SELECTPALETTE_ACTION:          return "SELECTPALETTE";
        case EMR_SETCOLORSPACE_ACTION:          return "SETCOLORSPACE";
        case EMR_SETPALETTEENTRIES_ACTION:      return "SETPALETTEENTRIES";

        // Drawing action types
        case EMR_ANGLEARC_ACTION:               return "ANGLEARC";
        case EMR_ARC_ACTION:                    return "ARC";
        case EMR_ARCTO_ACTION:                  return "ARCTO";
        case EMR_CHORD_ACTION:                  return "CHORD";
        case EMR_ELLIPSE_ACTION:                return "ELLIPSE";
        case EMR_EXTFLOODFILL_ACTION:           return "FLOODFILL";
        case EMR_EXTTEXTOUTA_ACTION:            return "EXTTEXTOUTA";
        case EMR_EXTTEXTOUTW_ACTION:            return "EXTTEXTOUTW";
        case EMR_FILLPATH_ACTION:               return "FILLPATH";
        case EMR_FILLRGN_ACTION:                return "FILLRGN";
        case EMR_FRAMERGN_ACTION:               return "FRAMERGN";
        case EMR_GRADIENTFILL_ACTION:           return "GRADIENTFILL";
        case EMR_LINETO_ACTION:                 return "LINETO";
        case EMR_PAINTRGN_ACTION:               return "PAINTRGN";
        case EMR_PIE_ACTION:                    return "PIE";
        case EMR_POLYBEZIER_ACTION:             return "POLYBEZIER";
        case EMR_POLYBEZIERTO_ACTION:           return "POLYBEZIERTO";
        case EMR_POLYBEZIER16_ACTION:           return "POLYBEZIER16";
        case EMR_POLYBEZIERTO16_ACTION:         return "POLYBEZIERTO16";
        case EMR_POLYDRAW_ACTION:               return "POLYDRAW";
        case EMR_POLYDRAW16_ACTION:             return "POLYDRAW16";
        case EMR_POLYGON_ACTION:                return "POLYGON";
        case EMR_POLYGON16_ACTION:              return "POLYGON16";
        case EMR_POLYLINE_ACTION:               return "POLYLINE";
        case EMR_POLYLINE16_ACTION:             return "POLYLINE16";
        case EMR_POLYLINETO_ACTION:             return "POLYLINETO";
        case EMR_POLYLINETO16_ACTION:           return "POLYLINETO16";
        case EMR_POLYPOLYGON_ACTION:            return "POLYPOLYGON";
        case EMR_POLYPOLYGON16_ACTION:          return "POLYPOLYGON16";
        case EMR_POLYPOLYLINE_ACTION:           return "POLYPOLYLINE";
        case EMR_POLYPOLYLINE16_ACTION:         return "POLYPOLYLINE16";
        case EMR_POLYTEXTOUTA_ACTION:           return "POLYTEXTOUTA";
        case EMR_POLYTEXTOUTW_ACTION:           return "POLYTEXTOUTW";
        case EMR_RECTANGLE_ACTION:              return "RECTANGLE";
        case EMR_SETPIXELV_ACTION:              return "SETPIXELV";
        case EMR_SMALLTEXTOUT_ACTION:           return "SMALLTEXTOUT";
        case EMR_STROKEANDFILLPATH_ACTION:      return "STROKEANDFILLPATH";
        case EMR_STROKEPATH_ACTION:             return "STROKEPATH";

        // Escape action types
        case EMR_DRAWESCAPE_ACTION:             return "DRAWESCAPE";
        case EMR_EXTESCAPE_ACTION:              return "EXTESCAPE";
        case EMR_NAMEDESCAPE_ACTION:            return "NAMEDESCAPE";

        // OpenGL action types
        case EMR_GLSRECORD_ACTION:              return "GLSRECORD";
        case EMR_GLSBOUNDEDRECORD_ACTION:       return "GLSBOUNDEDRECORD";

        // Path action types
        case EMR_BEGINPATH_ACTION:              return "BEGINPATH";
        case EMR_ENDPATH_ACTION:                return "ENDPATH";
        case EMR_CLOSEFIGURE_ACTION:            return "CLOSEFIGURE";
        case EMR_FLATTENPATH_ACTION:            return "FLATTENPATH";
        case EMR_WIDENPATH_ACTION:              return "WIDENPATH";
        case EMR_ABORTPATH_ACTION:              return "ABORTPATH";

        default:
            // Yes, return a pointer to a static buffer. This is a very
            // local debugging output function, so no big deal.
            static char buffer[6];
            sprintf(buffer, "%u", nEMFAction);
            return buffer;
        }
#endif
    }
}

void PointL::Read( SvStream& rIStm )
{
    rIStm.ReadInt32(mnX);
    rIStm.ReadInt32(mnY);

    SAL_INFO("vcl.emf", "\t\tPoint: (" << mnX << ", " << mnY << ")");
}

void PointS::Read( SvStream& rIStm )
{
    rIStm.ReadInt16(mnX);
    rIStm.ReadInt16(mnY);

    SAL_INFO("vcl.emf", "\t\tPoint: (" << mnX << ", " << mnY << ")");
}


void RectL::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32(mnLeft);
    rIStm.ReadUInt32(mnTop);
    rIStm.ReadUInt32(mnRight);
    rIStm.ReadUInt32(mnBottom);

    SAL_INFO("vcl.emf", "\t\tRectL structure: (" << mnLeft << ", " << mnTop << ", " << mnRight << ", " << mnBottom << ")");
}

void SizeL::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32(mnX);
    rIStm.ReadUInt32(mnY);

    SAL_INFO("vcl.emf", "\t\tSize: (" << mnX << ", " << mnY << ")");
}

void LogPaletteEntry::Read( SvStream& rIStm )
{
    sal_uChar nReserved;
    rIStm.ReadUChar(nReserved);
    rIStm.ReadUChar(mnBlue);
    rIStm.ReadUChar(mnGreen);
    rIStm.ReadUChar(mnRed);

    SAL_INFO("vcl.emf", "\t\t\tRGB value: (" << std::hex << (sal_uInt8) mnRed << ", " << (sal_uInt8) mnGreen << ", " << (sal_uInt8) mnBlue << ")" << std::dec);
}

void LogPalette::Read( SvStream& rIStm )
{
    sal_uInt16 nVersion=0;
    rIStm.ReadUInt16(nVersion);

    if (nVersion != 0x0300)
        SAL_WARN("vcl.emf", "Version field invalid value (0x" << std::hex << nVersion << std::dec << ")");

    SAL_INFO("vcl.emf", "\t\tVersion: 0x" << std::hex << nVersion << std::dec);

    sal_uInt16 nNumEntries=0;
    rIStm.ReadUInt16(nNumEntries);
    SAL_INFO("vcl.emf", "\t\tNumber of entries: " << nNumEntries);

    for (sal_uInt16 i=0; i < nNumEntries; i++)
    {
        SAL_INFO("vcl.emf", "\t\tEntry " << (i+1) << ":");
        LogPaletteEntry aPaletteEntry;
        aPaletteEntry.Read(rIStm);
        maPaletteEntries.push_back(aPaletteEntry);
    }
}

void ROP4::Read( SvStream& rIStm )
{
    rIStm.SeekRel(2L);
    rIStm.ReadUChar(mnBackgroundROP3);
    SAL_INFO("vcl.emf", "Background ROP3: 0x" << std::hex << sal_uInt8(mnBackgroundROP3) << std::dec);

    rIStm.ReadUChar(mnForegroundROP3);
    SAL_INFO("vcl.emf", "Foreground ROP3: 0x" << std::hex << sal_uInt8(mnForegroundROP3) << std::dec);
}

void BLENDFUNCTION::Read( SvStream& rIStm )
{
    rIStm.ReadUChar(mnBlendOperation);
    if (mnBlendOperation != 0x00)
        SAL_WARN("vcl.emf", "Blend operation is not 0x00, it is 0x" << std::hex << sal_uInt8(mnBlendOperation) << std::dec);

    SAL_INFO("vcl.emf", "\t\tBlend operation: 0x" << std::hex << mnBlendOperation << std::dec);

    rIStm.ReadUChar(mnBlendFlags);
    if (mnBlendFlags != 0x00)
        SAL_WARN("vcl.emf", "Blend flags is not 0x00, it is 0x" << std::hex << sal_uInt8(mnBlendFlags) << std::dec);

    SAL_INFO("vcl.emf", "\t\tBlend flags: 0x" << std::hex << sal_uInt8(mnBlendFlags) << std::dec);

    rIStm.ReadUChar(mnSrcConstantAlpha);
    SAL_INFO("vcl.emf", "\t\tSource transparency: 0x" << std::hex << sal_uInt8(mnSrcConstantAlpha) << std::dec);

    rIStm.ReadUChar(mnAlphaFormat);

    if (mnAlphaFormat == 0x00)
        SAL_INFO("vcl.emf", "\t\tAlpha format: 0x00 (use ScrConstanceAlpha value to blend src and dest bitmaps)");
    else if (mnAlphaFormat == 0x01)
        SAL_INFO("vcl.emf", "\t\tAlpha format: AC_SRC_ALPHA (src bitmap is 32-bits-per-pixel and has alpha value)");
    else
        SAL_WARN("vcl.emf", "Alpha format value is 0x" << std::hex << sal_uInt8(mnAlphaFormat) << std::hex);
}

static void decodePenStyle(sal_uInt32 nPenStyle)
{
    if (nPenStyle == 0x00000000)
    {
        SAL_INFO("vcl.emf", "\t\t\tPS_COSMETIC");
        SAL_INFO("vcl.emf", "\t\t\tPS_ENDCAP_ROUND");
        SAL_INFO("vcl.emf", "\t\t\tPS_JOIN_ROUND");
        SAL_INFO("vcl.emf", "\t\t\tPS_SOLID");
    }
    else
    {
        if (nPenStyle & PS_DASH_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_DASH");
        if (nPenStyle & PS_DOT_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_DOT");
        if (nPenStyle & PS_DASHDOT_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_DASHDOT");
        if (nPenStyle & PS_DASHDOTDOT_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_DASHDOTDOT");
        if (nPenStyle & PS_NULL_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_NULL");
        if (nPenStyle & PS_INSIDEFRAME_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_INSIDEFRAME");
        if (nPenStyle & PS_USERSTYLE_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_USERSTYLE");
        if (nPenStyle & PS_ALTERNATE_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_ALTERNATE");
        if (nPenStyle & PS_ENDCAP_SQUARE_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_ENDCAP_SQUARE");
        if (nPenStyle & PS_ENDCAP_FLAT_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_ENDCAP_FLAT");
        if (nPenStyle & PS_JOIN_BEVEL_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_JOIN_BEVEL");
        if (nPenStyle & PS_JOIN_MITER_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_JOIN_MITER");
        if (nPenStyle & PS_GEOMETRIC_EMF)
            SAL_INFO("vcl.emf", "\t\t\tPS_GEOMETRIC");
    }
}

void LogPen::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32(mnPenStyle);
    SAL_INFO("vcl.emf", "\t\tPen style:");

    decodePenStyle(mnPenStyle);

    SAL_INFO("vcl.emf", "\t\tWidth:");
    PointL aWidth;
    aWidth.Read(rIStm);
    mnWidth = aWidth.mnX;
    SAL_INFO("vcl.emf", "\t\tWidth is x value: " << mnWidth);

    SAL_INFO("vcl.emf", "\t\tColor:");
    maColorRef.Read(rIStm);
}

void LogPenEx::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32( mnPenStyle );
    SAL_INFO("vcl.emf", "\t\tPen style:");
    decodePenStyle( mnPenStyle );

    rIStm.ReadUInt32(mnWidth);
    SAL_INFO("vcl.emf", "\t\tWidth:" << mnWidth);

    SAL_INFO("vcl.emf", "\t\tBrush:");
    maBrush.Read(rIStm);

    sal_uInt32 nNumEntries=0;
    rIStm.ReadUInt32(nNumEntries);
    SAL_INFO("vcl.emf", "\t\tNumber of style entries: " << nNumEntries);

    if (nNumEntries > 0)
        SAL_INFO("vcl.emf", "\t\tDash width entries:");

    for (sal_uInt32 i=0; i < nNumEntries; i++)
    {
        sal_uInt32 nDashWidth=0;
        rIStm.ReadUInt32(nDashWidth);
        SAL_INFO("vcl.emf", "\t\t\tDash width " << i << ":" << nDashWidth);
        maStyleEntries.push_back(nDashWidth);

        i++;

        if (i < nDashWidth)
        {
            sal_uInt32 nDashGapWidth=0;
            rIStm.ReadUInt32(nDashGapWidth);
            SAL_INFO("vcl.emf", "\t\t\tDash width " << i << " (gap):" << nDashGapWidth);
            maStyleEntries.push_back(nDashGapWidth);
        }
    }
}

void CIEXYZ::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32(mnCiexyzX);
    rIStm.ReadUInt32(mnCiexyzY);
    rIStm.ReadUInt32(mnCiexyzZ);

    SAL_INFO("vcl.emf", "\t\tCIE XYZ: (" << mnCiexyzX << ", " << mnCiexyzY << ", " << mnCiexyzZ << ")");
}

void CIEXYZTriple::Read( SvStream& rIStm )
{
    maCiexyzRed.Read(rIStm);
    maCiexyzGreen.Read(rIStm);
    maCiexyzBlue.Read(rIStm);
}

static float Decode88FixedFormat(sal_uInt32 nNumber)
{
    sal_uInt32 nDecimalPart=0;
    nDecimalPart = (nNumber >> 16);

    sal_uInt32 nFracPart=0;
    nFracPart = ((nNumber >> 8) & 0xFF);

    float fDecodedNum = nDecimalPart + (nFracPart/1000);
    return fDecodedNum;
}

void LogColorSpaceBase::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32(mnSignature);

    if (mnSignature != 0x50534F43)
        SAL_WARN("vcl.emf", "LogColorSpace signature is invalid (0x" << std::hex << mnSignature << std::dec << ")");

    SAL_INFO("vcl.emf", "\tSignature: 0x" << std::hex << mnSignature << std::dec);

    rIStm.ReadUInt32(mnVersion);

    if (mnVersion != 0x00004000)
        SAL_WARN("vcl.emf", "LogColorSpace version is wrong (0x" << std::hex << mnVersion << std::dec << ")");

    SAL_INFO("vcl.emf", "\tVersion: 0x" << std::hex << mnVersion << std::dec);

    rIStm.ReadUInt32(mnSize);
    SAL_INFO("vcl.emf", "\tSize: " << mnSize);

    if (mnSize-48 == 0)
    {
        sal_uInt32 nColorSpaceType=0;
        rIStm.ReadUInt32(nColorSpaceType);

        switch (nColorSpaceType)
        {
            case 0x00000000:
            {
                meColorSpaceType=LCS_CALIBRATED_RGB;
                SAL_INFO("vcl.emf", "\tColorSpaceType: LCS_CALIBRATED_RGB");
            }
            break;

            case 0x73524742:
            {
                meColorSpaceType=LCS_sRGB;
                SAL_INFO("vcl.emf", "\tColorSpaceType: LCS_sRGB");
            }
            break;

            case 0x57696E20:
            {
                meColorSpaceType=LCS_WINDOWS_COLOR_SPACE;
                SAL_INFO("vcl.emf", "\tColorSpaceType: LCS_WINDOWS_COLOR_SPACE");
            }
            break;

            default:
                SAL_WARN("vcl.emf", "ColorSpaceType value is invalid (0x" << std::hex << nColorSpaceType << std::dec << ")");
        }

        sal_uInt32 nGammutMapping=0;
        rIStm.ReadUInt32(nGammutMapping);

        switch (nGammutMapping)
        {
            case 0x00000008:
            {
                meIntent=LCS_GM_ABS_COLORIMETRIC;
                SAL_INFO("vcl.emf", "\tGammut mapping: LCS_GM_ABS_COLORIMETRIC");
            }
            break;

            case 0x00000001:
            {
                meIntent=LCS_GM_BUSINESS;
                SAL_INFO("vcl.emf", "\tGammut mapping: LCS_GM_BUSINESS");
            }
            break;

            case 0x00000002:
            {
                meIntent=LCS_GM_GRAPHICS;
                SAL_INFO("vcl.emf", "\tGammut mapping: LCS_GM_GRAPHICS");
            }
            break;

            case 0x00000004:
            {
                meIntent=LCS_GM_IMAGES;
                SAL_INFO("vcl.emf", "\tGammut mapping: LCS_GM_IMAGES");
            }
            break;

            default:
                SAL_WARN("vcl.emf", "Gammut mapping value is invalid (0x" << std::hex << nGammutMapping << std::dec << ")");
        }

        maEndpoints.Read(rIStm);

        rIStm.ReadUInt32(mnGammaRed);
        rIStm.ReadUInt32(mnGammaGreen);
        rIStm.ReadUInt32(mnGammaBlue);

        SAL_INFO("vcl.emf", "\tGamma: (" << Decode88FixedFormat(mnGammaRed) << ", " << Decode88FixedFormat(mnGammaGreen) <<  ", " << Decode88FixedFormat(mnGammaBlue) << ")");
    }
}

void LogColorSpace::Read( SvStream& rIStm )
{
    LogColorSpaceBase::Read(rIStm);

    sal_uInt64 nStartPos = rIStm.Tell();

    if (mnSize-48 == 0)
    {
        rIStm.Seek(nStartPos+48);

        sal_uInt16 nRemainingSize = mnSize-48;
        char sFileName[nRemainingSize];

        rIStm.Read(sFileName, nRemainingSize);

        sal_uInt16 nChar=0;
        for (nChar=0; nChar < nRemainingSize; nChar++)
        {
            if (sFileName[nChar] == '\0')
                break;
        }

        if (nChar == nRemainingSize-1)
            sFileName[nRemainingSize] = '\0';

        maFileName = OUString::createFromAscii(sFileName);
        SAL_INFO("vcl.emf", "\tFilename: " << maFileName);
    }
}

void LogColorSpaceW::Read( SvStream& rIStm )
{
    LogColorSpaceBase::Read(rIStm);

    sal_uInt64 nStartPos = rIStm.Tell();

    if (mnSize-48 == 0)
    {
        rIStm.Seek(nStartPos+48);

        sal_uInt16 nRemainingSize = mnSize-48;
        sal_uInt16 nFilenameChars = nRemainingSize / 2;

        // The next bit reads UTF16-LE so we need to ensure that we set to little-endian format
        rIStm.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        maFileName = read_uInt16s_ToOUString( rIStm, nFilenameChars );
        SAL_INFO("vcl.emf", "\tFilename: " << maFileName);
    }
}

void PitchAndFamily::Read( SvStream& rIStm )
{
    sal_uInt8 nPitchAndFamily=0;
    rIStm.ReadUChar(nPitchAndFamily);

    sal_uInt8 nFamily = (nPitchAndFamily & 0xF);    // first 4 bits are family (see [MS-WMF] 2.2.2.14)
    sal_uInt8 nPitch  = (nPitchAndFamily >> 6);    // last 2 bits are pitch (see [MS-WMF] 2.2.2.14)

    switch (nFamily)
    {
        case 0x00:
        {
            meFamily=FF_DONTCARE_EMF;
            SAL_INFO("vcl.emf", "\t\tFamily: don't care");
        }
        break;

        case 0x01:
        {
            meFamily=FF_ROMAN_EMF;
            SAL_INFO("vcl.emf", "\t\tFamily: Roman");
        }
        break;

        case 0x02:
        {
            meFamily=FF_SWISS_EMF;
            SAL_INFO("vcl.emf", "\t\tFamily: Swiss");
        }
        break;

        case 0x03:
        {
            meFamily=FF_MODERN_EMF;
            SAL_INFO("vcl.emf", "\t\tFamily: Modern");
        }
        break;

        case 0x04:
        {
            meFamily=FF_SCRIPT_EMF;
            SAL_INFO("vcl.emf", "\t\tFamily: Script");
        }
        break;

        case 0x05:
        {
            meFamily=FF_DECORATIVE_EMF;
            SAL_INFO("vcl.emf", "\t\tFamily: Decorative");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Font family flag is invalid - value is 0x" << std::hex << (sal_uInt16) nFamily << std::dec);

    }

    switch (nPitch)
    {
        case 0:
        {
            mePitch = DEFAULT_PITCH_EMF;
            SAL_INFO("vcl.emf", "\t\tPitch: Default");
        }
        break;

        case 1:
        {
            mePitch = FIXED_PITCH_EMF;
            SAL_INFO("vcl.emf", "\t\tPitch: Fixed");
        }
        break;

        case 2:
        {
            mePitch = VARIABLE_PITCH_EMF;
            SAL_INFO("vcl.emf", "\t\tPitch: Variable");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Pitch flag is invalied - value is " << (sal_uInt8) nPitch);
    }
}

LogFont::LogFont() {}
LogFontEx::LogFontEx() {}
LogFontExDv::LogFontExDv() {}

void LogFont::Read( SvStream& rIStm )
{
    rIStm.ReadInt32(mnHeight);

    // see [MS-EMF] section 2.2.13
    if (mnHeight < 0)
        SAL_INFO("vcl.emf", "\t\tHeight: " << mnHeight << " - convert to device units and match to cell height of available fonts");
    else if (mnHeight == 0)
        SAL_INFO("vcl.emf", "\t\tHeight: use default height");
    else
        SAL_INFO("vcl.emf", "\t\tHeight: " << mnHeight << " - convert to device units and match absolute value to character height of available fonts");

    rIStm.ReadInt32(mnWidth);
    SAL_INFO("vcl.emf", "\t\tAverage width: " << mnWidth);

    rIStm.ReadInt32(mnEscapement);
    SAL_INFO("vcl.emf", "\t\tAngle between escapement vector and device x-axis: " << mnEscapement * 10 << " degrees");

    rIStm.ReadInt32(mnOrientation);
    SAL_INFO("vcl.emf", "\t\tAngle between character baseline and device x-axis: " << mnOrientation * 10 << " degrees");

    rIStm.ReadInt32(mnWeight);
    if (mnWeight == 0)
        SAL_INFO("vcl.emf", "\t\tWeight: default");
    else if (mnWeight > 1000)
        SAL_WARN("vcl.emf", "Weight of " << mnWeight << " is out of bounds!");
    else if (mnWeight < 0)
        SAL_WARN("vcl.emf", "Negative weight of " << mnWeight << "!");
    else
        SAL_INFO("vcl.emf", "\t\tWeight: " << mnWeight);

    sal_uInt8 nItalic=0;
    rIStm.ReadUChar(nItalic);
    switch (nItalic)
    {
        case 0x00:
        {
            mbItalic = false;
            SAL_INFO("vcl.emf", "\t\tItalic: false");
        }
        break;

        case 0x01:
        {
            mbItalic = true;
            SAL_INFO("vcl.emf", "\t\tItalic: true");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Italic value is invalid (0x" << std::hex << nItalic << std::dec << ")");
    }

    sal_uInt8 nUnderline=0;
    rIStm.ReadUChar(nUnderline);
    switch (nUnderline)
    {
        case 0x00:
        {
            mbItalic = false;
            SAL_INFO("vcl.emf", "\t\tUnderline: false");
        }
        break;

        case 0x01:
        {
            mbItalic = true;
            SAL_INFO("vcl.emf", "\t\tUnderline: true");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Underline value is invalid (0x" << std::hex << nUnderline << std::dec << ")");
    }

    sal_uInt8 nStrikeout=0;
    rIStm.ReadUChar(nStrikeout);
    switch (nStrikeout)
    {
        case 0x00:
        {
            mbStrikeout = false;
            SAL_INFO("vcl.emf", "\t\tStrikeout: false");
        }
        break;

        case 0x01:
        {
            mbStrikeout = true;
            SAL_INFO("vcl.emf", "\t\tStrikeout: true");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Strikeout value is invalid (0x" << std::hex << (sal_uInt8) nStrikeout << std::dec << ")");
    }


    sal_uInt8 nCharSet=0;
    rIStm.ReadUChar(nCharSet);
    switch (nCharSet)
    {
        case 0x00000000:
        {
            meCharSet = ANSI_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: ANSI");
        }
        break;

        case 0x00000001:
        {
            meCharSet = DEFAULT_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Default");
        }
        break;

        case 0x00000002:
        {
            meCharSet = SYMBOL_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Symbol");
        }
        break;

        case 0x0000004D:
        {
            meCharSet = MAC_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Mac");
        }
        break;

        case 0x00000080:
        {
            meCharSet = SHIFTJIS_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: SHIFTJIS");
        }
        break;

        case 0x00000081:
        {
            meCharSet = HANGUL_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Hangul");
        }
        break;

        case 0x00000082:
        {
            meCharSet = JOHAB_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Johab");
        }
        break;

        case 0x00000086:
        {
            meCharSet = GB2312_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: GB2312");
        }
        break;

        case 0x00000088:
        {
            meCharSet = CHINESEBIG5_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: CHINESEBIG5");
        }
        break;

        case 0x000000A1:
        {
            meCharSet = GREEK_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Greek");
        }
        break;

        case 0x000000A2:
        {
            meCharSet = TURKISH_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Turkish");
        }
        break;

        case 0x000000A3:
        {
            meCharSet = VIETNAMESE_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Vietnamese");
        }
        break;

        case 0x000000B1:
        {
            meCharSet = HEBREW_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Hebrew");
        }
        break;

        case 0x000000B2:
        {
            meCharSet = ARABIC_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Arabic");
        }
        break;

        case 0x000000BA:
        {
            meCharSet = BALTIC_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Baltic");
        }
        break;

        case 0x000000CC:
        {
            meCharSet = RUSSIAN_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Russian");
        }
        break;

        case 0x000000DE:
        {
            meCharSet = THAI_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Thai");
        }
        break;

        case 0x000000EE:
        {
            meCharSet = EASTEUROPE_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: Eastern Europe");
        }
        break;

        case 0x000000FF:
        {
            meCharSet = OEM_CHARSET_EMF;
            SAL_INFO("vcl.emf", "\t\tCharacter set: OEM");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Invalid character set (0x" << std::hex << nCharSet << std::dec);
    }

    sal_uInt8 nOutPrecision=0;
    rIStm.ReadUChar(nOutPrecision);

    switch (nOutPrecision)
    {
        case 0x00000000:
        {
            meOutPrecision = OUT_DEFAULT_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_DEFAULT_PRECIS");
        }
        break;

        case 0x00000001:
        {
            meOutPrecision = OUT_STRING_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_STRING_PRECIS");
        }
        break;

        case 0x00000003:
        {
            meOutPrecision = OUT_STROKE_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_STROKE_PRECIS");
        }
        break;

        case 0x00000004:
        {
            meOutPrecision = OUT_TT_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_TT_PRECIS");
        }
        break;

        case 0x00000005:
        {
            meOutPrecision = OUT_DEVICE_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_DEV_PRECIS");
        }
        break;

        case 0x00000006:
        {
            meOutPrecision = OUT_RASTER_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_RASTER_PRECIS");
        }
        break;

        case 0x00000007:
        {
            meOutPrecision = OUT_TT_ONLY_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_TT_ONLY_PRECIS");
        }
        break;

        case 0x00000008:
        {
            meOutPrecision = OUT_OUTLINE_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_OUTLINE_PRECIS");
        }
        break;

        case 0x00000009:
        {
            meOutPrecision = OUT_SCREEN_OUTLINE_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_SCREEN_OUTLINE_PRECIS");
        }
        break;

        case 0x0000000A:
        {
            meOutPrecision = OUT_PS_ONLY_PRECIS;
            SAL_INFO("vcl.emf", "\t\tOutput precision: OUT_PS_ONLY_PRECIS");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Output precision is invalid value (0x" << std::hex << nOutPrecision << std::dec);
    }

    rIStm.ReadUChar(mnClipPrecision);

    SAL_INFO("vcl.emf", "\t\tClipping precision:");

    if (mnClipPrecision == CLIP_DEFAULT_PRECIS)
    {
        SAL_INFO("vcl.emf", "\t\t\tCLIP_DEFAULT_PRECIS");
    }
    else
    {
        if (mnClipPrecision & CLIP_CHARACTER_PRECIS)
            SAL_INFO("vcl.emf", "\t\t\tCLIP_CHARACTER_PRECIS");

        if (mnClipPrecision & CLIP_STROKE_PRECIS)
            SAL_INFO("vcl.emf", "\t\t\tCLIP_STROKE_PRECIS");

        if (mnClipPrecision & CLIP_LH_ANGLES)
            SAL_INFO("vcl.emf", "\t\t\tCLIP_LH_ANGLES");

        if (mnClipPrecision & CLIP_TT_ALWAYS)
            SAL_INFO("vcl.emf", "\t\t\tCLIP_TT_ALWAYS");

        if (mnClipPrecision & CLIP_DFA_DISABLE)
            SAL_INFO("vcl.emf", "\t\t\tCLIP_DFA_DISABLE");

        if (mnClipPrecision & CLIP_EMBEDDED)
            SAL_INFO("vcl.emf", "\t\t\tCLIP_EMBEDDED");
    }

    sal_uInt8 nFontQuality=0;
    rIStm.ReadUChar(nFontQuality);

    switch (nFontQuality)
    {
        case 0x00:
        {
            meQuality = DEFAULT_QUALITY;
            SAL_INFO("vcl.emf", "\t\tFont quality: default");
        }
        break;

        case 0x01:
        {
            meQuality = DRAFT_QUALITY;
            SAL_INFO("vcl.emf", "\t\tFont quality: draft");
        }
        break;

        case 0x02:
        {
            meQuality = PROOF_QUALITY;
            SAL_INFO("vcl.emf", "\t\tFont quality: proof");
        }
        break;

        case 0x03:
        {
            meQuality = NONANTIALIASED_QUALITY;
            SAL_INFO("vcl.emf", "\t\tFont quality: disable anti-aliasing");
        }

        case 0x04:
        {
            meQuality = ANTIALIASED_QUALITY;
            SAL_INFO("vcl.emf", "\t\tFont quality: enable anti-aliasing");
        }

        case 0x05:
        {
            meQuality = CLEARTYPE_QUALITY;
            SAL_INFO("vcl.emf", "\t\tFont quality: ClearType");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Invalid font quality value (0x" << std::hex << nFontQuality << std::dec << ")");
    }

    maPitchAndFamily.Read(rIStm);

    rIStm.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    maFacename = read_uInt16s_ToOUString( rIStm, 32 );
    SAL_INFO("vcl.emf", "\t\tFace name: " << maFacename);
}

void LogFontEx::Read( SvStream& rIStm )
{
    LogFont::Read(rIStm);

    // The next bit reads UTF16-LE so we need to ensure that we set to little-endian format
    rIStm.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    maFullName = read_uInt16s_ToOUString( rIStm, 64 );
    maStyle = read_uInt16s_ToOUString( rIStm, 32 );
    maScript = read_uInt16s_ToOUString( rIStm, 32 );

    SAL_INFO("vcl.emf", "\t\tFull name: " << maFullName);
    SAL_INFO("vcl.emf", "\t\tStyle: " << maStyle);
    SAL_INFO("vcl.emf", "\t\tScript: " << maScript);
}

void DesignVector::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32(mnSignature);
    if (mnSignature != 0x08007664)
        SAL_WARN("vcl.emf", "Signature should be 0x08007664, but it is 0x" << std::hex << mnSignature << std::dec);

    SAL_INFO("vcl.emf", "\t\tSignature: 0x" << std::hex << mnSignature << std::dec);

    rIStm.ReadUInt32(mnNumAxes);
    if (mnNumAxes > 16)
        SAL_WARN("vcl.emf", "Number of axes is greater than 16 (" << mnNumAxes << ")");

    SAL_INFO("vcl.emf", "\t\tNumber of axes: " << mnNumAxes);

    SAL_INFO("vcl.emf", "\t\tValues:");

    for (sal_uInt32 i=0; (i < mnNumAxes) && (i < 16); i++)
    {
        rIStm.ReadUInt32(maValues[i]);
        SAL_INFO("vcl.emf", "\t\t\tValue[" << i << "]: " << maValues[i]);
    }
}

void LogFontExDv::Read( SvStream& rIStm )
{
    LogFontEx::Read(rIStm);
    SAL_INFO("vcl.emf", "\t\tDesignVector structure:");
    maDesignVector.Read(rIStm);
}

void LogBrushEx::Read( SvStream& rIStm )
{
    sal_uInt32 nBrushStyle=0;
    rIStm.ReadUInt32(nBrushStyle);

    switch (nBrushStyle)
    {
        case 0x0000:
        {
            meBrushStyle=BS_SOLID_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_SOLID");
        }
        break;

        case 0x0001:
        {
            meBrushStyle=BS_NULL_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_NULL");
        }
        break;

        case 0x0002:
        {
            meBrushStyle=BS_HATCHED_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_HATCHED");
        }
        break;

        case 0x0003:
        {
            meBrushStyle=BS_PATTERN_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_PATTERN");
        }
        break;

        case 0x0004:
        {
            meBrushStyle=BS_INDEXED_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_INDEXED");
        }
        break;

        case 0x0005:
        {
            meBrushStyle=BS_DIBPATTERN_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_DIBPATTERN");
        }
        break;

        case 0x0006:
        {
            meBrushStyle=BS_DIBPATTERNPT_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_DIBPATTERNPT");
        }
        break;

        case 0x0007:
        {
            meBrushStyle=BS_PATTERN8X8_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_PATTERN8X8");
        }
        break;

        case 0x0008:
        {
            meBrushStyle=BS_DIBPATTERN8X8_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_DIBPATTERN8X8");
        }
        break;

        case 0x0009:
        {
            meBrushStyle=BS_MONOPATTERN_EMF;
            SAL_INFO("vcl.emf", "\t\tBrush style: BS_MONOPATTERN");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Invalid brush style (0x" << std::hex << nBrushStyle << std::dec << ")");
    }

    SAL_INFO("vcl.emf", "\t\tColor:");
    maColor.Read(rIStm);

    sal_uInt32 nHatchStyle=0;
    rIStm.ReadUInt32(nHatchStyle);

    switch (nHatchStyle)
    {
        case 0x0006:
        {
            meHatchStyle=HS_SOLIDCLR_EMF;
            SAL_INFO("vcl.emf", "\t\tHatch style: HS_SOLIDCLR");
        }
        break;

        case 0x0007:
        {
            meHatchStyle=HS_DITHEREDCLR_EMF;
            SAL_INFO("vcl.emf", "\t\tHatch style: HS_DITHEREDCLR");
        }
        break;

        case 0x0008:
        {
            meHatchStyle=HS_SOLIDTEXTCLR_EMF;
            SAL_INFO("vcl.emf", "\t\tHatch style: HS_SOLIDTEXTCLR");
        }
        break;

        case 0x0009:
        {
            meHatchStyle=HS_DITHEREDTEXTCLR_EMF;
            SAL_INFO("vcl.emf", "\t\tHatch style: HS_DITHEREDTEXTCLR");
        }
        break;

        case 0x000A:
        {
            meHatchStyle=HS_SOLIDBKCLR_EMF;
            SAL_INFO("vcl.emf", "\t\tHatch style: HS_SOLIDBKCLR");
        }
        break;

        case 0x000B:
        {
            meHatchStyle=HS_DITHEREDBKCLR_EMF;
            SAL_INFO("vcl.emf", "\t\tHatch style: HS_DITHEREDBKCLR");
        }
        break;

        default:
        {
            switch (meBrushStyle)
            {

                   SAL_WARN("vcl.emf", "Hatch style value 0x" << std::hex << nHatchStyle << std::dec << " is invalid");
                   break;

                default:
                    SAL_INFO("vcl.emf", "\t\tHatch style not used");
            }
        }
    }
}

void ColorRef::Read( SvStream& rIStm )
{
    rIStm.ReadUChar(mnRed);
    rIStm.ReadUChar(mnGreen);
    rIStm.ReadUChar(mnBlue);

    SAL_INFO("vcl.emf", "\t\tRed:   0x" << std::hex << (sal_uInt16) mnRed << std::dec);
    SAL_INFO("vcl.emf", "\t\tGreen: 0x" << std::hex << (sal_uInt16) mnRed << std::dec);
    SAL_INFO("vcl.emf", "\t\tBlue:  0x" << std::hex << (sal_uInt16) mnRed << std::dec);

    sal_uInt8 nReserved=0;
    rIStm.ReadUChar(nReserved);
}

void ColorAdjustment::Read( SvStream& rIStm )
{
    rIStm.ReadUInt16(mnSize);
    rIStm.ReadUInt16(mnValues);
    rIStm.ReadUInt16(mnIlluminantIndex);

    if (mnSize != 0x0018)
        SAL_WARN("vcl.emf", "Invalid ColorAdjustment size");

    SAL_INFO("vcl.emf", "\t\tSize: " << mnSize);
    SAL_INFO("vcl.emf", "\t\tValues: " << mnValues);
    SAL_INFO("vcl.emf", "\t\tIlluminant index: " << mnIlluminantIndex);

    rIStm.ReadUInt16(mnRedGamma);
    rIStm.ReadUInt16(mnGreenGamma);
    rIStm.ReadUInt16(mnBlueGamma);

    if (mnRedGamma < 2500 || mnRedGamma > 65000)
        SAL_WARN("vcl.emf", "Red gamma correction value is out of range");
    else if (mnGreenGamma < 2500 || mnGreenGamma > 65000)
        SAL_WARN("vcl.emf", "Green gamma correction value is out of range");
    else if (mnBlueGamma < 2500 || mnBlueGamma > 65000)
        SAL_WARN("vcl.emf", "Blue gamma correction value is out of range");

    if (mnRedGamma == 10000)
        SAL_INFO("vcl.emf", "\t\tRed gamma disabled" << mnRedGamma);
    else
        SAL_INFO("vcl.emf", "\t\tRed gamma: " << mnRedGamma);

    if (mnGreenGamma == 10000)
        SAL_INFO("vcl.emf", "\t\tGreen gamma disabled" << mnGreenGamma);
    else
        SAL_INFO("vcl.emf", "\t\tGreen gamma: " << mnGreenGamma);

    if (mnBlueGamma == 10000)
        SAL_INFO("vcl.emf", "\t\tBlue gamma disabled" << mnBlueGamma);
    else
        SAL_INFO("vcl.emf", "\t\tBlue gamma: " << mnBlueGamma);

    rIStm.ReadUInt16(mnReferenceBlack);
    rIStm.ReadUInt16(mnReferenceWhite);

    if (mnReferenceBlack > 4000)
        SAL_WARN("vcl.emf", "Black reference for source colors out of range");

    SAL_INFO("vcl.emf", "\t\tBlack reference color: " << mnReferenceBlack);

    if (mnReferenceWhite < 6000 || mnReferenceWhite > 10000)
        SAL_WARN("vcl.emf", "White reference for source colors out of range");

    SAL_INFO("vcl.emf", "\t\tWhite reference color: " << mnReferenceWhite);

    rIStm.ReadInt16(mnContrast);
    rIStm.ReadInt16(mnBrightness);
    rIStm.ReadInt16(mnColorfulness);
    rIStm.ReadInt16(mnRedGreenTint);

    if (mnContrast < -100 || mnContrast > 100)
        SAL_WARN("vcl.emf", "Contract out of range");

    SAL_INFO("vcl.emf", "\t\tContrast: " << mnContrast);

    if (mnBrightness < -100 || mnBrightness > 100)
        SAL_WARN("vcl.emf", "Brightness out of range");

    SAL_INFO("vcl.emf", "\t\tBrightness: " << mnBrightness);

    if (mnColorfulness < -100 || mnColorfulness > 100)
        SAL_WARN("vcl.emf", "Colorfulness out of range");

    SAL_INFO("vcl.emf", "\t\tColorfulness: " << mnColorfulness);

    if (mnRedGreenTint < -100 || mnRedGreenTint > 100)
        SAL_WARN("vcl.emf", "Red/green tint out of range");

    SAL_INFO("vcl.emf", "\t\tRed/green tint: " << mnRedGreenTint);
}

static void decode_EmfTextOutOptions(sal_uInt32 nOptions)
{
    if (nOptions & ETO_OPAQUE_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_OPAQUE");

    if (nOptions & ETO_CLIPPED_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_CLIPPED");

    if (nOptions & ETO_GLYPH_INDEX_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_GLYPH_INDEX");

    if (nOptions & ETO_RTLREADING_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_RTLREADING");

    if (nOptions & ETO_NO_RECT_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_NO_RECT");

    if (nOptions & ETO_SMALL_CHARS_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_SMALL_CHARS");

    if (nOptions & ETO_NUMERICSLOCAL_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_NUMERICSLOCAL");

    if (nOptions & ETO_NUMERICSLATIN_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_NUMERICSLATIN");

    if (nOptions & ETO_IGNORELANGUAGE_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_IGNORELANGUAGE");

    if (nOptions & ETO_PDY_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_PDY");

    if (nOptions & ETO_REVERSE_INDEX_MAP_EMF)
        SAL_INFO("vcl.emf", "\t\t\tETO_REVERSE_INDEX_MAP");

    if (nOptions & 0x0000C069)
        SAL_WARN("vcl.emf", "Unused bits set in options - 0x" << std::hex << (nOptions & 0x0000C069) << std::dec);
}

void EmrText::Read( SvStream& rIStm, sal_uInt64 nStartRecordOffset, bool bUnicode)
{
    SAL_INFO("vcl.emf", "\t\tReference:");
    maReference.Read(rIStm);

    sal_uInt32 nChars=0;
    rIStm.ReadUInt32(nChars);

    if (nChars == 0)
        SAL_WARN("vcl.emf", "Empty string!");

    sal_uInt32 nOffsetString=0;
    rIStm.ReadUInt32(nOffsetString);
    SAL_INFO("vcl.emf", "\t\tOffset position to text: " << nOffsetString);

    rIStm.ReadUInt32(mnOptions);

    SAL_INFO("vcl.emf", "\t\tOptions: 0x" << std::hex << mnOptions << std::dec);
    decode_EmfTextOutOptions(mnOptions);

    SAL_INFO("vcl.emf", "\tRectangle:");
    maRectangle.Read(rIStm);

    sal_uInt32 nOffsetDx=0;
    // Note: must be 32-bit aligned according to [MS-EMF], funnily enough
    // if you read the record correctly then it will be anyway... just an aside
    rIStm.ReadUInt32(nOffsetDx);
    SAL_INFO("vcl.emf", "\t\tOffset position for character interspacing: " << nOffsetDx);

    rIStm.Seek(nStartRecordOffset + nOffsetString - 4);

    if (!bUnicode)
    {
        // TODO we are reading an arbitrary number of characters.
        // We really need some sanity checks to ensure that we aren't
        // reading malformed data - malicious or otherwise
        char sOutputString[nChars+1];
        rIStm.Read(sOutputString, nChars);
        sOutputString[nChars+1] = '\0';
        maStringBuffer = OUString::createFromAscii(sOutputString);
        SAL_INFO("vcl.emf", "\t\tOutput string (ASCII):");
    }
    else
    {
        rIStm.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        maStringBuffer = read_uInt16s_ToOUString(rIStm, nChars);
        SAL_INFO("vcl.emf", "\t\tOutput string (Unicode):");
    }

    SAL_INFO("vcl.emf", "\t\t" << maStringBuffer);

    rIStm.Seek(nStartRecordOffset + nOffsetDx - 4);

    sal_uInt64 nDXValues=nChars;

    if (mnOptions & ETO_PDY_EMF)
        nDXValues = nChars * 2;

    SAL_INFO("vcl.emf", "\t\tCharacter cell spacing:");

    for (sal_uInt32 i=0; i < nDXValues; i++)
    {
        sal_uInt32 nValue=0;
        rIStm.ReadUInt32(nValue);
        SAL_INFO("vcl.emf", "\t\t\tValue " << i << ": " << nValue);
        maOutputDx.push_back(nValue);
    }
}


void RegionDataHeader::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32(mnSize);

    SAL_INFO("vcl.emf", "\t\tSize: " << mnSize);

    rIStm.ReadUInt32(mnType);

    SAL_INFO("vcl.emf", "\t\tType: 0x" << std::hex << mnType << std::dec);

    if (mnType != 0x00000001)
        SAL_WARN("vcl.emf", "RegionDataHeader type should be RDH_RECTANGLES (0x00000001), but it is 0x" << std::hex << mnType << std::dec);

    rIStm.ReadUInt32(mnCountRects);

    SAL_INFO("vcl.emf", "\t\tNumber of rectangles: " << mnCountRects);

    rIStm.ReadUInt32(mnRgnSize);

    SAL_INFO("vcl.emf", "\t\tRegion size (bytes)" << mnRgnSize);

    rIStm.ReadUInt32(maBounds.mnLeft);
    rIStm.ReadUInt32(maBounds.mnTop);
    rIStm.ReadUInt32(maBounds.mnRight);
    rIStm.ReadUInt32(maBounds.mnBottom);

    SAL_INFO("vcl.emf", "\t\tBounding rect (logical units): " << maBounds.mnLeft  << ", " << maBounds.mnTop << ", "
                                                    << maBounds.mnTop   << ", " << maBounds.mnBottom);
}

void RegionData::Read( SvStream& rIStm )
{
    mpRegionDataHeader = new RegionDataHeader;
    mpRegionDataHeader->Read(rIStm);

    SAL_INFO("vcl.emf", "\t\tRegion data:");

    for (sal_uInt32 i=0; i < mpRegionDataHeader->mnCountRects; i++)
    {
        RectL aRect;
        rIStm.ReadUInt32(aRect.mnLeft);
        rIStm.ReadUInt32(aRect.mnTop);
        rIStm.ReadUInt32(aRect.mnRight);
        rIStm.ReadUInt32(aRect.mnBottom);

        SAL_INFO("vcl.emf", "\t\t\tRegion rect " << i << ": " << aRect.mnLeft  << ", " << aRect.mnTop << ", "
                                                    << aRect.mnRight << ", " << aRect.mnBottom);

        maData.push_back(aRect);
    }
}

void PixelFormatDescriptor::Read( SvStream& rIStm )
{
    rIStm.ReadUInt16(mnSize);

    SAL_INFO("vcl.emf", "\t\tSize: " << mnSize);

    rIStm.ReadUInt16(mnVersion);

    if (mnVersion != 0x0001)
        SAL_WARN("vcl.emf", "PixelFormatDescriptor version is not 0x0001, and is invalid version 0x" << std::hex << mnVersion << std::dec);

    SAL_INFO("vcl.emf", "\t\tVersion: " << mnVersion);

    rIStm.ReadUInt32(mnFlags);

    SAL_INFO("vcl.emf", "\t\tFlags: ");

    // PFD_DOUBLEBUFFER and PFD_SUPPORT_GDI - see dwFlags table in section [MS-EMF] 2.2.22
    if ((mnFlags & PFD_SUPPORT_GDI) && (mnFlags & PFD_DOUBLEBUFFER))
        SAL_WARN("vcl.emf", "PixelFormatDescriptor flag conflict: PFD_DOUBLEBUFFER and PFD_SUPPORT_GDI cannot be set together");

    if (mnFlags & PFD_NEED_PALETTE)
        SAL_INFO("vcl.emf", "\t\t\tPFD_NEED_PALLETTE");
    if (mnFlags & PFD_GENERIC_FORMAT)
        SAL_INFO("vcl.emf", "\t\t\tPFD_GENERIC_FORMAT");
    if (mnFlags & PFD_SUPPORT_OPENGL)
        SAL_INFO("vcl.emf", "\t\t\tPFD_SUPPORT_OPENGL");
    if (mnFlags & PFD_SUPPORT_GDI)
        SAL_INFO("vcl.emf", "\t\t\tPFD_SUPPORT_GDI");
    if (mnFlags & PFD_DRAW_TO_BITMAP)
        SAL_INFO("vcl.emf", "\t\t\tPFD_DRAW_TO_BITMAP");
    if (mnFlags & PFD_DRAW_TO_WINDOW)
        SAL_INFO("vcl.emf", "\t\t\tPFD_DRAW_TO_BITMAP");
    if (mnFlags & PFD_STEREO)
        SAL_INFO("vcl.emf", "\t\t\tPFD_STEREO");
    if (mnFlags & PFD_DOUBLEBUFFER)
        SAL_INFO("vcl.emf", "\t\t\tPFD_DOUBLEBUFFER");
    if (mnFlags & PFD_SUPPORT_COMPOSITION)
        SAL_INFO("vcl.emf", "\t\t\tPFD_SUPPORT_COMPOSITION");
    if (mnFlags & PFD_DIRECT3D_ACCELERATED)
        SAL_INFO("vcl.emf", "\t\t\tPFD_DIRECT3D_ACCELERATED");
    if (mnFlags & PFD_SUPPORT_DIRECTDRAW)
        SAL_INFO("vcl.emf", "\t\t\tPFD_SUPPORT_DIRECTDRAW");
    if (mnFlags & PFD_GENERIC_ACCELERATED)
        SAL_INFO("vcl.emf", "\t\t\tPFD_GENERIC_ACCELERATED");
    if (mnFlags & PFD_SWAP_LAYER_BUFFERS)
        SAL_INFO("vcl.emf", "\t\t\tPFD_SWAP_LAYER_BUFFERS");
    if (mnFlags & PFD_SWAP_COPY)
        SAL_INFO("vcl.emf", "\t\t\tPFD_SWAP_COPY");
    if (mnFlags & PFD_SWAP_EXCHANGE)
        SAL_INFO("vcl.emf", "\t\t\tPFD_SWAP_EXCHANGE");
    if (mnFlags & PFD_NEED_SYSTEM_PALETTE)
        SAL_INFO("vcl.emf", "\t\t\tPFD_NEED_SYSTEM_PALETTE");
    if (mnFlags & PFD_STEREO_DONTCARE)
        SAL_INFO("vcl.emf", "\t\t\tPFD_STEREO_DONTCARE");
    if (mnFlags & PFD_DOUBLEBUFFER_DONTCARE)
        SAL_INFO("vcl.emf", "\t\t\tPFD_DOUBLEBUFFER_DONTCARE");
    if (mnFlags & PFD_DEPTH_DONTCARE)
        SAL_INFO("vcl.emf", "\t\t\tPFD_DEPTH_DONTCARE");

    rIStm.ReadUChar(mnPixelType);

    if (mnPixelType == PFD_TYPE_RGBA)
        SAL_INFO("vcl.emf", "\t\tPixel type: PFD_TYPE_RGBA (0x" << std::hex << mnPixelType << std::dec << ")");
    else if (mnPixelType == PFD_TYPE_COLORINDEX)
        SAL_INFO("vcl.emf", "\t\tPixel type: PFD_TYPE_COLORINDEX (0x" << std::hex << mnPixelType << std::dec << ")");
    else
        SAL_WARN("vcl.emf", "Pixel type is not a valid value: it is 0x" << std::hex << mnPixelType << std::dec << ")");

    rIStm.ReadUChar(mnColorBits);
    SAL_INFO("vcl.emf", "\t\tColor bits: " << mnColorBits);

    rIStm.ReadUChar(mnRedBits);
    SAL_INFO("vcl.emf", "\t\tRed bits: " << mnRedBits);

    rIStm.ReadUChar(mnRedShift);
    SAL_INFO("vcl.emf", "\t\tRed shift: " << mnRedShift);

    rIStm.ReadUChar(mnGreenBits);
    SAL_INFO("vcl.emf", "\t\tGreen bits: " << mnGreenBits);

    rIStm.ReadUChar(mnGreenShift);
    SAL_INFO("vcl.emf", "\t\tGreen shift: " << mnGreenShift);

    rIStm.ReadUChar(mnBlueBits);
    SAL_INFO("vcl.emf", "\t\tBlue bits: " << mnBlueBits);

    rIStm.ReadUChar(mnRedShift);
    SAL_INFO("vcl.emf", "\t\tBlue shift: " << mnBlueShift);

    rIStm.ReadUChar(mnAlphaBits);
    SAL_INFO("vcl.emf", "\t\tAlpha bits: " << mnAlphaBits);

    rIStm.ReadUChar(mnAlphaShift);
    SAL_INFO("vcl.emf", "\t\tAlpha shift: " << mnAlphaShift);

    rIStm.ReadUChar(mnAccumBits);
    SAL_INFO("vcl.emf", "\t\tAccumulation buffer bitplanes: " << mnAccumBits);

    rIStm.ReadUChar(mnAccumRedBits);
    SAL_INFO("vcl.emf", "\t\tAccumulation buffer red bitplanes: " << mnAccumRedBits);

    rIStm.ReadUChar(mnAccumGreenBits);
    SAL_INFO("vcl.emf", "\t\tAccumulation buffer green bitplanes: " << mnAccumGreenBits);

    rIStm.ReadUChar(mnAccumBlueBits);
    SAL_INFO("vcl.emf", "\t\tAccumulation buffer blue bitplanes: " << mnAccumBlueBits);

    rIStm.ReadUChar(mnDepthBits);
    SAL_INFO("vcl.emf", "\t\tDepth (z-axis) buffer bits: " << mnDepthBits);

    // according to Microsoft [MS-EMF], auxiliary buffers aren't supported...
    rIStm.ReadUChar(mnAuxBuffers);
    SAL_INFO("vcl.emf", "\t\tAuxiliary buffers: " << mnAuxBuffers);

    rIStm.ReadUChar(mnLayerType);
    SAL_INFO("vcl.emf", "\t\tLayer type: 0x" << std::hex << mnLayerType << std::dec);

    sal_uInt8 nReserved=0;
    rIStm.ReadUChar(nReserved);    // a completely dodgy name for this field, but literally that's what the EMF spec calls it
    mnOverlayPlanes = (nReserved >> 4);             // bits 0 to 3
    mnUnderlayPlanes = ((nReserved & 0xF0) >> 4);    // bits 4 to 7

    SAL_INFO("vcl.emf", "\t\tOverlay planes: " << mnOverlayPlanes);
    SAL_INFO("vcl.emf", "\t\tUnderlay planes: " << mnUnderlayPlanes);

    rIStm.ReadUInt32(mnLayerMask);
    SAL_INFO("vcl.emf", "\t\tLayer mask: 0x" << std::hex << mnLayerMask << std::dec);

    rIStm.ReadUInt32(mnVisibleMask);
    if (mnPixelType == PFD_TYPE_RGBA)
        SAL_INFO("vcl.emf", "\t\tVisible mask (RGBA): 0x" << std::hex << mnPixelType << std::dec);
    else if (mnPixelType == PFD_TYPE_COLORINDEX)
        SAL_INFO("vcl.emf", "\t\tVisible mask (indexed color): 0x" << std::hex << mnPixelType << std::dec);

    rIStm.ReadUInt32(mnDamageMask);
    SAL_INFO("vcl.emf", "\t\tDamage mask: 0x" << std::hex << mnDamageMask << std::dec);
}

void XFormEMF::Read( SvStream &rIStm )
{
    rIStm.ReadFloat( mfM11 );
    rIStm.ReadFloat( mfM12 );
    rIStm.ReadFloat( mfM21 );
    rIStm.ReadFloat( mfM22 );
    rIStm.ReadFloat( mfDx );
    rIStm.ReadFloat( mfDy );

    SAL_INFO("vcl.emf", "\t\tM11: " << mfM11);
    SAL_INFO("vcl.emf", "\t\tM12: " << mfM12);
    SAL_INFO("vcl.emf", "\t\tM21: " << mfM21);
    SAL_INFO("vcl.emf", "\t\tM22: " << mfM22);
    SAL_INFO("vcl.emf", "\t\tDx:  " << mfDx);
    SAL_INFO("vcl.emf", "\t\tDy:  " << mfDy);
}

void TriVertex::Read( SvStream& rIStm )
{
    rIStm.ReadInt32(mnX);
    rIStm.ReadInt32(mnY);

    SAL_INFO("vcl.emf", "\t\tPosition: x=" << mnX << ", y=" << mnY);

    rIStm.ReadUInt16(mnRed);
    rIStm.ReadUInt16(mnGreen);
    rIStm.ReadUInt16(mnBlue);
    rIStm.ReadUInt16(mnAlpha);

    SAL_INFO("vcl.emf", "\t\tColor: RGBA(" << mnRed << ", " << mnGreen << ", " << mnBlue << ", " << mnAlpha << ")");
}

EmfAction::EmfAction()
{
    // nothing
}

EmfAction::EmfAction( sal_uInt32 nType )
    : mnType(nType)
{
    // nothing
}

EmfAction::~EmfAction()
{
    // nothing
}

class VCL_DLLPUBLIC EmfHeaderAction : public EmfAction
{
    // Header object
    RectL       maBounds;
    RectL       maFrame;
    sal_uInt32  mnRecordSignature;
    sal_uInt32  mnVersion;
    sal_uInt32  mnBytes;
    sal_uInt32  mnRecords;
    sal_uInt16  mnHandles;
    SizeL       maDevice;
    SizeL       maDeviceMM;
    sal_uInt32  mnDescriptionSize;
    sal_uInt32  mnOffDescription;
    sal_uInt32  mnPalEntries;

    // HeaderExtension1 object
    sal_uInt32  mnPixelFormatSize;
    sal_uInt32  mnOffPixelFormat;
    bool        mbOpenGL;

    // HeaderExtension2 object
    sal_uInt32  mnMicrometersX;
    sal_uInt32  mnMicrometersY;

    HeaderType  meHdrType;

protected:
    virtual     ~EmfHeaderAction() {}

public:
    explicit    EmfHeaderAction()
                : EmfAction( EMR_HEADER_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize, nHeaderSize;

        rIStm.ReadUInt32( nSize );

        nHeaderSize = 88;

        if (nSize >= 88)
        {
            // EmfMetaFileHeader object
            nHeaderSize = nSize;
            readEMFMetaFileHeader(rIStm);

            // check per flowchart in [MS-EMF] section 2.3.4.2
            if ((mnOffDescription >= 88) && (mnOffDescription * 2 <= nSize))
            {
                nHeaderSize = mnOffDescription;
                meHdrType = NORMAL;
            }
            else if (mnOffDescription != 0)
            {
                SAL_WARN("vcl.emf", "Invalid description values in EMF header!");
            }
        }

        if (nHeaderSize >= 100)
        {
            // EmfMetaFileHeaderExtension1 object
            readHeaderExtension1(rIStm);

            if (mnOffPixelFormat == 0)
            {
                meHdrType = EXT1;
            }
            else if ((mnOffPixelFormat >= 100) && (mnOffPixelFormat + mnPixelFormatSize <= nSize))
            {
                meHdrType = EXT1;

                if (mnOffPixelFormat < nHeaderSize)
                    nHeaderSize = mnOffPixelFormat;
            }
        }

        if (nHeaderSize >= 108)
        {
            // EmfMetaFileHeaderExtension2 object
            readHeaderExtension2(rIStm);
            meHdrType = EXT2;
        }
    }

private:
    /** Reads the Header object - see [MS-EMF], section 2.2.9
     */
    void readEMFMetaFileHeader( SvStream& rIStm )
    {
        // get the bounds of the metafile
        rIStm.ReadUInt32( maBounds.mnLeft );
        rIStm.ReadUInt32( maBounds.mnTop );
        rIStm.ReadUInt32( maBounds.mnRight );
        rIStm.ReadUInt32( maBounds.mnBottom );

        SAL_INFO("vcl.emf", "\tBounding box: " << maBounds.mnLeft  << ", " << maBounds.mnTop << ", "
                                     << maBounds.mnRight << ", " << maBounds.mnBottom);

        // get the dimensions in .01 mm units of the bounding
        // rectange
        rIStm.ReadUInt32( maFrame.mnLeft );
        rIStm.ReadUInt32( maFrame.mnTop );
        rIStm.ReadUInt32( maFrame.mnRight );
        rIStm.ReadUInt32( maFrame.mnBottom );

        SAL_INFO("vcl.emf", "\tBounding box (mm): " << maFrame.mnLeft  << ", " << maFrame.mnTop << ", "
                                          << maFrame.mnRight << ", " << maFrame.mnBottom);

        rIStm.ReadUInt32( mnRecordSignature );

        SAL_INFO("vcl.emf", "\tSignature: 0x" << std::hex << mnRecordSignature << std::dec);

        // An EMF signature must be ASCII "FME" (0x464D4520)
        // see [MS-EMF] section 2.1.14
        if (mnRecordSignature != 0x464D4520)
            SAL_WARN("vcl.emf", "EMF signature invalid! Should be 0x464D4520, but is 0x" << std::hex << mnRecordSignature << std::dec << ".");

        rIStm.ReadUInt32( mnVersion );

        SAL_INFO("vcl.emf", "\tVersion: " << mnVersion);

        // This should be 0x00010000, see [MS-EMF], section 2.2.9
        if (mnVersion != 0x00010000)
            SAL_WARN("vcl.emf", "EMF version should be 0x00010000, but it is 0x" << std::hex << mnVersion << std::dec << ".");

        rIStm.ReadUInt32( mnBytes );

        SAL_INFO("vcl.emf", "\tSize (bytes): " << mnBytes);

        rIStm.ReadUInt32( mnRecords );
        rIStm.ReadUInt16( mnHandles );

        SAL_INFO("vcl.emf", "\tRecords: " << mnRecords);
        SAL_INFO("vcl.emf", "\tObjects: " << mnHandles);

        sal_uInt16 nReserved;
        rIStm.ReadUInt16( nReserved );

        SAL_INFO("vcl.emf", "\tReserved: 0x" << std::hex << nReserved << std::dec);

        // this must be 0x0000 - can't be used
        // see [MS-EMF] section 2.1.14
        if (nReserved)
            SAL_WARN("vcl.emf", "Reserved field in EMF header is not 0x0000! It is 0x" << std::hex << nReserved << std::dec << ".");

        rIStm.ReadUInt32( mnDescriptionSize );
        rIStm.ReadUInt32( mnOffDescription );

        SAL_INFO("vcl.emf", "\tDescription size: " << mnDescriptionSize);
        SAL_INFO("vcl.emf", "\tDescription offset: 0x" << std::hex << mnOffDescription << std::dec);

        rIStm.ReadUInt32( mnPalEntries );

        SAL_INFO("vcl.emf", "\tPAL entries: " << mnPalEntries);

        // size of reference device, in pixels
        rIStm.ReadUInt32( maDevice.mnX );
        rIStm.ReadUInt32( maDevice.mnY );

        SAL_INFO("vcl.emf", "\tReference device size (pixels): (" << maDevice.mnX << ", " << maDevice.mnY << ")");

        // size of reference device, in mm
        rIStm.ReadUInt32( maDeviceMM.mnX );
        rIStm.ReadUInt32( maDeviceMM.mnY );

        SAL_INFO("vcl.emf", "\tReference device size (mm): (" << maDeviceMM.mnX << ", " << maDeviceMM.mnY << ")");
    }

    /** Reads the HeaderExtension1 object, see [EMF] section 2.2.10
     */
    void readHeaderExtension1( SvStream& rIStm )
    {
        SAL_INFO("vcl.emf", "");
        SAL_INFO("vcl.emf", "HEADER EXTENSION 1");
        SAL_INFO("vcl.emf", "------------------");

        rIStm.ReadUInt32( mnPixelFormatSize );
        rIStm.ReadUInt32( mnOffPixelFormat );

        sal_uInt32 nOpenGL;
        rIStm.ReadUInt32( nOpenGL );

        SAL_INFO("vcl.emf", "\tPixel format size: " << mnPixelFormatSize);
        SAL_INFO("vcl.emf", "\tPixel format offset: 0x" << std::hex << mnOffPixelFormat << std::dec);
        SAL_INFO("vcl.emf", "\tOpenGL flag: " << nOpenGL);

        switch (nOpenGL)
        {
            case 0x00000000: mbOpenGL = false; break;
            case 0x00000001: mbOpenGL = true; break;
            default: SAL_WARN("vcl.emf", "OpenGL flag in EMF header extension 1 is neither 0x1 or 0x0!");
        }
    }

    /** Reads the HeaderExtension2 object, see [EMF] section 2.2.11
     */
    void readHeaderExtension2( SvStream& rIStm )
    {
        SAL_INFO("vcl.emf", "");
        SAL_INFO("vcl.emf", "HEADER EXTENSION 2");
        SAL_INFO("vcl.emf", "------------------");

        rIStm.ReadUInt32( mnMicrometersX );
        rIStm.ReadUInt32( mnMicrometersY );

        SAL_INFO("vcl.emf", "\tMicrometers X=" << mnMicrometersX << " Y=" << mnMicrometersY);
    }

};

class VCL_DLLPUBLIC EmfEOFAction : public EmfAction
{
protected:
    virtual     ~EmfEOFAction() {}

public:
                EmfEOFAction()
                : EmfAction( EMR_EOF_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        sal_uInt64 nOriginalPos=0;

        nOriginalPos = rIStm.Tell();
        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.Seek(nOriginalPos + nSize - 4);
    }
};

class VCL_DLLPUBLIC EmfCommentAction : public EmfAction
{
protected:
    virtual     ~EmfCommentAction() {}

public:
                EmfCommentAction()
                : EmfAction( EMR_COMMENT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        sal_uInt64 nOriginalPos=0;

        nOriginalPos = rIStm.Tell();
        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.Seek(nOriginalPos + nSize - 4);
    }
};

// State - save and restore device context

class VCL_DLLPUBLIC EmfSaveDCAction : public EmfAction
{
protected:
    virtual     ~EmfSaveDCAction() {}

public:
                EmfSaveDCAction()
                : EmfAction( EMR_SAVEDC_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        sal_uInt64 nOriginalPos=0;

        nOriginalPos = rIStm.Tell();
        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.Seek(nOriginalPos + nSize - 4);
    }
};


class VCL_DLLPUBLIC EmfRestoreDCAction : public EmfAction
{
    sal_Int32  mnSavedDC;

protected:
    virtual     ~EmfRestoreDCAction() {}

public:
                EmfRestoreDCAction()
                : EmfAction( EMR_RESTOREDC_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadInt32(mnSavedDC);

        SAL_INFO("vcl.emf", "\tSaved DC: " << mnSavedDC);
    }
};

// State - color actions

class VCL_DLLPUBLIC EmfSetBKColorAction : public EmfAction
{
    ColorRef mnColor;

protected:
    virtual     ~EmfSetBKColorAction() {}

public:
                EmfSetBKColorAction()
                : EmfAction( EMR_SETBKCOLOR_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tColor: ");

        mnColor.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfSetBKModeAction : public EmfAction
{
    BackgroundMode meBackgroundMode;

protected:
    virtual     ~EmfSetBKModeAction() {}

public:
                EmfSetBKModeAction()
                : EmfAction( EMR_SETBKMODE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        sal_uInt32 nBackgroundMode=0;
        rIStm.ReadUInt32(nBackgroundMode);

        if (nBackgroundMode == 0x0001)
        {
            SAL_INFO("vcl.emf", "\tBackground mode: transparent");
            meBackgroundMode = TRANSPARENT;
        }
        else if (nBackgroundMode == 0x0002)
        {
            SAL_INFO("vcl.emf", "\tBackground mode: opaque");
            meBackgroundMode = OPAQUE;
        }
        else
        {
            SAL_WARN("vcl.emf", "Background mode is invalid value, it is 0x" << std::hex << nBackgroundMode << std::dec);
        }
    }
};


class VCL_DLLPUBLIC EmfSetColorAdjustmentAction : public EmfAction
{
    ColorAdjustment aColorAdjustment;

protected:
    virtual     ~EmfSetColorAdjustmentAction() {}

public:
                EmfSetColorAdjustmentAction()
                : EmfAction( EMR_SETCOLORADJUSTMENT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tColor adjustment:");

    };

};


class VCL_DLLPUBLIC EmfSetICMModeAction : public EmfAction
{
    ICMMode meICMMode;

protected:
    virtual     ~EmfSetICMModeAction() {}

public:
                EmfSetICMModeAction()
                : EmfAction( EMR_SETICMMODE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        sal_uInt32 nICMMode=0;
        rIStm.ReadUInt32(nICMMode);

        switch (nICMMode)
        {
            case 0x01:
            {
                meICMMode=ICM_OFF;
                SAL_INFO("vcl.emf", "\tICM mode: ICM_OFF");
            }
            break;

            case 0x02:
            {
                meICMMode=ICM_ON;
                SAL_INFO("vcl.emf", "\tICM mode: ICM_ON");
            }
            break;

            case 0x03:
            {
                meICMMode=ICM_QUERY;
                SAL_INFO("vcl.emf", "\tICM mode: ICM_QUERY");
            }
            break;

            case 0x04:
            {
                meICMMode=ICM_DONE_OUTSIDEDC;
                SAL_INFO("vcl.emf", "\tICM mode: ICM_DONE_OUTSIDEDC");
            }
            break;

            default:
                SAL_WARN("vcl.emf", "ICM mode is invalid value (0x" << std::hex << nICMMode << std::dec << ")");
        }
    }
};

class VCL_DLLPUBLIC EmfSetICMProfileWAction : public EmfAction
{
    sal_uInt32  mnFlags;
    sal_uInt32  mnNameSize;
    sal_uInt32  mnDataSize;
    OUString    maProfileName;

protected:
    virtual     ~EmfSetICMProfileWAction() {}

public:
                EmfSetICMProfileWAction()
                : EmfAction( EMR_SETICMPROFILEW_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnFlags);
        SAL_INFO("vcl.emf", "\tColor profile flags: 0x" << std::hex << mnFlags << std::dec);

        rIStm.ReadUInt32(mnNameSize);
        rIStm.ReadUInt32(mnDataSize);

        // The next bit reads UTF16-LE so we need to ensure that we set to little-endian format
        rIStm.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        maProfileName = read_uInt16s_ToOUString( rIStm, (mnDataSize+mnNameSize)*2 );
        SAL_INFO("vcl.emf", "\tProfile name: " << maProfileName);
    }
};

class VCL_DLLPUBLIC EmfSetICMProfileAAction : public EmfAction
{
    sal_uInt32  mnFlags;
    sal_uInt32  mnNameSize;
    sal_uInt32  mnDataSize;
    OUString    maProfileName;

protected:
    virtual     ~EmfSetICMProfileAAction() {}

public:
                EmfSetICMProfileAAction()
                : EmfAction( EMR_SETICMPROFILEA_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnFlags);
        SAL_INFO("vcl.emf", "\tColor profile flags: 0x" << std::hex << mnFlags << std::dec);

        rIStm.ReadUInt32(mnNameSize);
        rIStm.ReadUInt32(mnDataSize);

        char sProfileName[mnNameSize];
        rIStm.Read(sProfileName, mnNameSize);
        sProfileName[mnNameSize] = '\0';

        maProfileName = OUString::createFromAscii(sProfileName);
        SAL_INFO("vcl.emf", "\tProfile name: " << maProfileName);
    }
};


class VCL_DLLPUBLIC EmfColorMatchToTargetWAction : public EmfAction
{
protected:
    virtual     ~EmfColorMatchToTargetWAction() {}

public:
                EmfColorMatchToTargetWAction()
                : EmfAction( EMR_COLORMATCHTOTARGETW_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        sal_uInt64 nOriginalPos=0;

        nOriginalPos = rIStm.Tell();
        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.Seek(nOriginalPos + nSize - 4);

        SAL_WARN("vcl.emf", "ColorMatchToTargetW: color matching not yet implemented");
    }
};


// State - miter limits

class VCL_DLLPUBLIC EmfSetMiterLimitAction : public EmfAction
{
    sal_uInt32  mnMiterLimit;

protected:
    virtual     ~EmfSetMiterLimitAction() {}

public:
                EmfSetMiterLimitAction()
                : EmfAction( EMR_SETMITERLIMIT_ACTION) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnMiterLimit);
        SAL_INFO("vcl.emf", "\tMiter limit: " << mnMiterLimit);
    }
};


// State - polygon fill mode

class VCL_DLLPUBLIC EmfSetPolyFillModeAction: public EmfAction
{
    PolygonFillMode  mePolygonFillMode;

protected:
    virtual     ~EmfSetPolyFillModeAction() {}

public:
                EmfSetPolyFillModeAction()
                : EmfAction( EMR_SETPOLYFILLMODE_ACTION) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        sal_uInt32 nPolygonFillMode;
        rIStm.ReadUInt32(nPolygonFillMode);

        switch (nPolygonFillMode)
        {
            case 0x0001:
            {
                mePolygonFillMode=ALTERNATE;
                SAL_INFO("vcl.emf", "\tPolygon fill mode: ALTERNATE");
            }
            break;

            case 0x0002:
            {
                mePolygonFillMode=WINDING;
                SAL_INFO("vcl.emf", "\tPolygon fill mode: WINDING");
            }
            break;

            default:
                SAL_WARN("vcl.emf", "Polygon fill mode has invalid value of 0x" << std::hex << nPolygonFillMode << std::dec);
        }
    }
};

// State - text actions

class VCL_DLLPUBLIC EmfSetTextAlignAction: public EmfAction
{
    sal_uInt32 mnTextAlignmentMode;

protected:
    virtual     ~EmfSetTextAlignAction() {}

public:
                EmfSetTextAlignAction()
                : EmfAction( EMR_SETTEXTALIGN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnTextAlignmentMode);

        SAL_INFO("vcl.emf", "\tText alignment mode: 0x" << mnTextAlignmentMode);
    }
};


class VCL_DLLPUBLIC EmfSetTextJustificationAction: public EmfAction
{
    sal_Int32 mnBreakExtra;
    sal_Int32 mnBreakCount;

protected:
    virtual     ~EmfSetTextJustificationAction() {}

public:
                EmfSetTextJustificationAction()
                : EmfAction( EMR_SETTEXTCOLOR_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadInt32(mnBreakExtra);
        rIStm.ReadInt32(mnBreakCount);

        SAL_INFO("vcl.emf", "\tExtra space to add (logical units): " << mnBreakExtra);
        SAL_INFO("vcl.emf", "\tNumber of break characters: " << mnBreakCount);
    }
};

class VCL_DLLPUBLIC EmfSetTextColorAction: public EmfAction
{
    ColorRef maColor;

protected:
    virtual     ~EmfSetTextColorAction() {}

public:
                EmfSetTextColorAction()
                : EmfAction( EMR_SETTEXTCOLOR_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tText color:");
        maColor.Read(rIStm);
    }
};


// State - font actions

class VCL_DLLPUBLIC EmfForceUFIMappingAction : public EmfAction
{
    UniversalFontId aUFI;

protected:
    virtual     ~EmfForceUFIMappingAction() {}

public:
                EmfForceUFIMappingAction()
                : EmfAction( EMR_FORCEUFIMAPPING_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tUFI (UniversalFontID):");
        aUFI.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfSetLinkedUFIsAction : public EmfAction
{
    std::unordered_map<sal_uInt32, UniversalFontId> maUFIs;

protected:
    virtual     ~EmfSetLinkedUFIsAction() {}

public:
                EmfSetLinkedUFIsAction()
                : EmfAction( EMR_SETLINKEDUFIS_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        sal_uInt32 nNumLinkedUFIs=0;
        rIStm.ReadUInt32(nNumLinkedUFIs);

        UniversalFontId aUFI;

        for (sal_uInt32 i=0; i<nNumLinkedUFIs; i++)
        {
            SAL_INFO("vcl.emf", "\tUFI (UniversalFontID):");

            aUFI.Read(rIStm);
            maUFIs[aUFI.mnIndex] = aUFI;
        }
    }
};

class VCL_DLLPUBLIC EmfSetMapperFlagsAction : public EmfAction
{
    sal_uInt32  mnFlags;

protected:
    virtual     ~EmfSetMapperFlagsAction() {}

public:
                EmfSetMapperFlagsAction()
                : EmfAction( EMR_SETMAPPERFLAGS_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnFlags);
        SAL_INFO("vcl.emf", "\tFlags: " << std::hex << mnFlags << std::dec);
    }
};


class VCL_DLLPUBLIC EmfSetROP2Action : public EmfAction
{
    BinaryRasterOperations meROP2Mode;

protected:
    virtual     ~EmfSetROP2Action() {}

public:
                EmfSetROP2Action()
                : EmfAction( EMR_SETROP2_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        sal_uInt32 nROP2Mode=0;
        rIStm.ReadUInt32(nROP2Mode);

        switch (nROP2Mode)
        {
            case 0x0001:
            {
                meROP2Mode=R2_BLACK_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_BLACK");
            }
            break;

            case 0x0002:
            {
                meROP2Mode=R2_NOTMERGEPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_NOTMERGEPEN");
            }
            break;


            case 0x0003:
            {
                meROP2Mode=R2_MASKNOTPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_MASKNOTPEN");
            }
            break;

            case 0x0004:
            {
                meROP2Mode=R2_NOTCOPYPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_NOTCOPYPEN");
            }
            break;

            case 0x0005:
            {
                meROP2Mode=R2_MASKPENNOT_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_MASKPENNOT");
            }
            break;

            case 0x0006:
            {
                meROP2Mode=R2_NOT_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_NOT");
            }
            break;

            case 0x0007:
            {
                meROP2Mode=R2_XORPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_XORPEN");
            }
            break;

            case 0x0008:
            {
                meROP2Mode=R2_NOTMASKPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_NOTMASKPEN");
            }
            break;

            case 0x0009:
            {
                meROP2Mode=R2_MASKPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_MASKPEN");
            }
            break;

            case 0x000A:
            {
                meROP2Mode=R2_NOTXORPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_NOTXORPEN");
            }
            break;

            case 0x000B:
            {
                meROP2Mode=R2_NOP_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_ROP");
            }
            break;

            case 0x000C:
            {
                meROP2Mode=R2_MERGENOTPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_MERGENOTPEN");
            }
            break;

            case 0x000D:
            {
                meROP2Mode=R2_COPYPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_COPYPEN");
            }
            break;

            case 0x000E:
            {
                meROP2Mode=R2_MERGEPENNOT_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_MERGENOTPEN");
            }
            break;

            case 0x000F:
            {
                meROP2Mode=R2_MERGEPEN_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_MERGEPEN");
            }
            break;

            case 0x0010:
            {
                meROP2Mode=R2_WHITE_EMF;
                SAL_INFO("vcl.emf", "\tROP2 mode: R2_WHITE");
            }
            break;

            default:
                SAL_WARN("vcl.emf", "Invalid ROP2 operation value (0x" << std::hex << nROP2Mode << std::dec);
        }
    }
};

// State map mode

class VCL_DLLPUBLIC EmfSetMapModeAction: public EmfAction
{
    sal_uInt32  mnMapMode;

protected:
    virtual     ~EmfSetMapModeAction() {}

public:
                EmfSetMapModeAction()
                : EmfAction( EMR_SETMAPMODE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnMapMode);

        switch (mnMapMode)
        {
            case MM_TEXT:           SAL_INFO("vcl.emf", "\tMapMode: MM_TEXT"); break;
            case MM_LOMETRIC:       SAL_INFO("vcl.emf", "\tMapMode: MM_LOMETRIC"); break;
            case MM_HIMETRIC:       SAL_INFO("vcl.emf", "\tMapMode: MM_HIMETRIC"); break;
            case MM_LOENGLISH:      SAL_INFO("vcl.emf", "\tMapMode: MM_LOENGLISH"); break;
            case MM_HIENGLISH:      SAL_INFO("vcl.emf", "\tMapMOde: MM_HIENGLISH"); break;
            case MM_TWIPS:          SAL_INFO("vcl.emf", "\tMapMode: MM_TWIPS"); break;
            case MM_ISOTROPIC:      SAL_INFO("vcl.emf", "\tMapMode: MM_ISOTROPIC"); break;
            case MM_ANISOTROPIC:    SAL_INFO("vcl.emf", "\tMapMode: MM_ANISOTROPIC"); break;
            default:
                SAL_WARN("vcl.emf", "MapMode - invalid value 0x" << std::hex << mnMapMode << std::dec);
        }
    }
};

// State - stretch BLT mode

class VCL_DLLPUBLIC EmfStretchBLTModeAction: public EmfAction
{
    StretchMode  meStretchMode;

protected:
    virtual     ~EmfStretchBLTModeAction() {}

public:
                EmfStretchBLTModeAction()
                : EmfAction( EMR_SETSTRETCHBLTMODE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        sal_uInt32 nStretchMode = 0;
        rIStm.ReadUInt32(nStretchMode);

        switch (nStretchMode)
        {
            case 0x0001:
            {
                meStretchMode = STRETCH_ANDSCANS_EMF;
                SAL_INFO("vcl.emf", "\tStretch mode = STRETCH_ANDSCANS");
            }
            break;

            case 0x0002:
            {
                meStretchMode = STRETCH_ORSCANS_EMF;
                SAL_INFO("vcl.emf", "\tStretch mode = STRETCH_ORSCANS");
            }
            break;

            case 0x0003:
            {
                meStretchMode = STRETCH_DELETESCANS_EMF;
                SAL_INFO("vcl.emf", "\tStretch mode = STRETCH_DELETESCANS");
            }
            break;

            case 0x0004:
            {
                meStretchMode = STRETCH_HALFTONE_EMF;
                SAL_INFO("vcl.emf", "\tStretch mode = STRETCH_HALFTONE");
            }

            default:
                SAL_WARN("vcl.emf", "Stretch mode - invalid value 0x" << std::hex << nStretchMode << std::dec);
        }
    }
};


// State - coordinate actions

class VCL_DLLPUBLIC EmfMoveToExAction : public EmfAction
{
    PointL      maOffset;

protected:
    virtual     ~EmfMoveToExAction() {}

public:
                EmfMoveToExAction()
                : EmfAction( EMR_MOVETOEX_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);


        maOffset.Read(rIStm);
    }
};

// State - stretch BLT mode

class VCL_DLLPUBLIC EmfSetBrushOrgExAction : public EmfAction
{
    PointL      maOrigin;

protected:
    virtual     ~EmfSetBrushOrgExAction() {}

public:
                EmfSetBrushOrgExAction()
                : EmfAction( EMR_SETBRUSHORGEX_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);


        maOrigin.Read(rIStm);
    }
};



// State - viewport and window actions

class VCL_DLLPUBLIC EmfSetViewportExtExAction : public EmfAction
{
    SizeL       maExtent;

protected:
    virtual     ~EmfSetViewportExtExAction() {}

public:
                EmfSetViewportExtExAction()
                : EmfAction( EMR_SETVIEWPORTEXTEX_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tExtent:");
        maExtent.Read(rIStm);
    }

};


class VCL_DLLPUBLIC EmfSetViewportOrgExAction : public EmfAction
{
    PointL      maOrigin;

protected:
    virtual     ~EmfSetViewportOrgExAction() {}

public:
                EmfSetViewportOrgExAction()
                : EmfAction( EMR_SETVIEWPORTORGEX_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tOrigin:");
        maOrigin.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfSetWindowExtExAction : public EmfAction
{
    SizeL       maExtent;

protected:
    virtual     ~EmfSetWindowExtExAction() {}

public:
                EmfSetWindowExtExAction()
                : EmfAction( EMR_SETWINDOWEXTEX_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tExtent:");
        maExtent.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfSetWindowOrgExAction : public EmfAction
{
    PointL      maOrigin;

protected:
    virtual     ~EmfSetWindowOrgExAction() {}

public:
                EmfSetWindowOrgExAction()
                : EmfAction( EMR_SETWINDOWORGEX_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tOrigin:");
        maOrigin.Read(rIStm);
    };

};


class VCL_DLLPUBLIC EmfScaleViewportExtExAction : public EmfAction
{
    sal_Int32  mnXNum;
    sal_Int32  mnXDenom;
    sal_Int32  mnYNum;
    sal_Int32  mnYDenom;

protected:
    virtual     ~EmfScaleViewportExtExAction() {}

public:
                EmfScaleViewportExtExAction()
                : EmfAction( EMR_SCALEVIEWPORTEXTEX_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadInt32(mnXNum);
        rIStm.ReadInt32(mnXDenom);
        rIStm.ReadInt32(mnYNum);
        rIStm.ReadInt32(mnYDenom);

        SAL_INFO("vcl.emf", "\tXNum: " << mnXNum);
        SAL_INFO("vcl.emf", "\tXDenom: " << mnXDenom);
        SAL_INFO("vcl.emf", "\tYNum: " << mnYNum);
        SAL_INFO("vcl.emf", "\tYDenom: " << mnYDenom);
    }
};


class VCL_DLLPUBLIC EmfScaleWindowExtExAction : public EmfAction
{
    sal_Int32  mnXNum;
    sal_Int32  mnXDenom;
    sal_Int32  mnYNum;
    sal_Int32  mnYDenom;

protected:
    virtual     ~EmfScaleWindowExtExAction() {}

public:
                EmfScaleWindowExtExAction()
                : EmfAction( EMR_SCALEWINDOWEXTEX_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadInt32(mnXNum);
        rIStm.ReadInt32(mnXDenom);
        rIStm.ReadInt32(mnYNum);
        rIStm.ReadInt32(mnYDenom);

        SAL_INFO("vcl.emf", "\tXNum: " << mnXNum);
        SAL_INFO("vcl.emf", "\tXDenom: " << mnXDenom);
        SAL_INFO("vcl.emf", "\tYNum: " << mnYNum);
        SAL_INFO("vcl.emf", "\tYDenom: " << mnYDenom);
    }
};


// State - region actions

class VCL_DLLPUBLIC EmfInvertRegionAction : public EmfAction
{
    RectL       maBounds;
    RegionData  maRegionData;

protected:
    virtual     ~EmfInvertRegionAction() {}

public:
                EmfInvertRegionAction()
                : EmfAction( EMR_INVERTRGN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "Bounds:");
        maBounds.Read(rIStm);

        SAL_INFO("vcl.emf", "Region data:");
        maRegionData.Read(rIStm);
    };

};

// State - pixel format action

class VCL_DLLPUBLIC EmfPixelFormatAction : public EmfAction
{
    PixelFormatDescriptor aPfd;

protected:
    virtual     ~EmfPixelFormatAction() {}

public:
                EmfPixelFormatAction()
                : EmfAction( EMR_PIXELFORMAT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tPixel format:");
        aPfd.Read(rIStm);
    }
};

// State - ARC direction

class VCL_DLLPUBLIC EmfSetArcDirectionAction : public EmfAction
{
    sal_uInt32 eArcDirection;

protected:
    virtual     ~EmfSetArcDirectionAction() {}

public:
                EmfSetArcDirectionAction()
                : EmfAction( EMR_SETLAYOUT_ACTION ) {}

    enum ArcDirection { AD_COUNTERCLOCKWISE = 0x0001, AD_CLOCKWISE = 0x0002 };

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0, nArcDirection=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(nArcDirection);
        eArcDirection = nArcDirection;

        if (eArcDirection == AD_COUNTERCLOCKWISE)
            SAL_INFO("vcl.emf", "\tArc direction: counterclockwise");
        else
            SAL_INFO("vcl.emf", "\tArc direction: clockwise");

    }
};

// State - layout records

class VCL_DLLPUBLIC EmfSetLayoutAction : public EmfAction
{
    LayoutMode meLayoutMode;

protected:
    virtual     ~EmfSetLayoutAction() {}

public:
                EmfSetLayoutAction()
                : EmfAction( EMR_SETLAYOUT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        sal_uInt32 nLayoutMode=0;
        rIStm.ReadUInt32(nLayoutMode);

        switch (nLayoutMode)
        {
            case 0x00000000:
            {
                SAL_INFO("vcl.emf", "Layout mode: LAYOUT_LTR");
                meLayoutMode = LAYOUT_LTR;
            }
            break;

            case 0x00000001:
            {
                SAL_INFO("vcl.emf", "Layout mode: LAYOUT_RTL");
                meLayoutMode = LAYOUT_RTL;
            }
            break;

            case 0x00000008:
            {
                SAL_INFO("vcl.emf", "Layout mode: LAYOUT_BITMAPORIENTATIONPRESERVED");
                meLayoutMode = LAYOUT_BITMAPORIENTATIONPRESERVED;
            }
            break;

            default:
                SAL_WARN("vcl.emf", "Layout mode value is invalid (0x" << std::hex << nLayoutMode << std::dec << ")");
        }
    }
};



// State - transform records

class VCL_DLLPUBLIC EmfSetWorldTransformAction: public EmfAction
{
    XFormEMF    maXform;

protected:
    virtual     ~EmfSetWorldTransformAction() {}

public:
                EmfSetWorldTransformAction()
                : EmfAction( EMR_SETWORLDTRANSFORM_ACTION) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tXform: ");
        maXform.Read(rIStm);

    }
};


class VCL_DLLPUBLIC EmfModifyWorldTransformAction: public EmfAction
{
    XFormEMF       maXform;
    ModifyWorldTransformMode meModifyWorldTransformMode;

protected:
    virtual     ~EmfModifyWorldTransformAction() {}

public:
                EmfModifyWorldTransformAction()
                : EmfAction( EMR_MODIFYWORLDTRANSFORM_ACTION) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tXform: ");
        maXform.Read(rIStm);

        sal_uInt32 nModifyWorldTransformMode=0;
        rIStm.ReadUInt32(nModifyWorldTransformMode);
        switch (nModifyWorldTransformMode)
        {
            case 0x01:
            {
                meModifyWorldTransformMode = MWT_IDENTITY_EMF;
                SAL_INFO("vcl.emf", "\tModify world transform mode: MWT_IDENTITY");
            }
            break;

            case 0x02:
            {
                meModifyWorldTransformMode = MWT_LEFTMULTIPLY_EMF;
                SAL_INFO("vcl.emf", "\tModify world transform mode: MWT_LEFTMULTIPLY");
            }
            break;

            case 0x03:
            {
                meModifyWorldTransformMode = MWT_RIGHTMULTIPLY_EMF;
                SAL_INFO("vcl.emf", "\tModify world transform mode: MWT_RIGHTMULTIPLY");
            }
            break;

            case 0x04:
            {
                meModifyWorldTransformMode = MWT_SET_EMF;
                SAL_INFO("vcl.emf", "\tModify world transform mode: MWT_SET");
            }
            break;

            default:
                SAL_WARN("vcl.emf", "Invalid value for Modify World Transform Mode - value is 0x" <<
                         std::hex << nModifyWorldTransformMode << std::dec);
        }
    }
};

// Object creation

class VCL_DLLPUBLIC EmfCreateBrushIndirectAction : public EmfAction
{
    sal_uInt32 mnBrushIndex;
    LogBrushEx maLogBrush;

protected:
    virtual     ~EmfCreateBrushIndirectAction() {}

public:
                EmfCreateBrushIndirectAction()
                : EmfAction( EMR_CREATEBRUSHINDIRECT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnBrushIndex);
        SAL_INFO("vcl.emf", "\tBrush index: " << mnBrushIndex);

        SAL_INFO("vcl.emf", "\tLogical brush:");
        maLogBrush.Read(rIStm);
    }
};

template < class LogColorSpaceType >
class VCL_DLLPUBLIC EmfCreateColorSpaceBaseAction : public EmfAction
{
    sal_uInt32 mnColorSpaceIndex;
    LogColorSpaceType maLogColorSpace;

protected:
    virtual     ~EmfCreateColorSpaceBaseAction() {}

public:
                EmfCreateColorSpaceBaseAction( sal_uInt16 nType )
                : EmfAction( nType ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnColorSpaceIndex);
        SAL_INFO("vcl.emf", "\tColorspace index: " << mnColorSpaceIndex);

        SAL_INFO("vcl.emf", "\tLogical colorspace:");
        maLogColorSpace.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfCreateColorSpaceAction : public EmfCreateColorSpaceBaseAction< LogColorSpace >
{
public:
                EmfCreateColorSpaceAction()
                : EmfCreateColorSpaceBaseAction< LogColorSpace >( EMR_CREATECOLORSPACE_ACTION ) {}

};


class VCL_DLLPUBLIC EmfCreateColorSpaceWAction : public EmfCreateColorSpaceBaseAction< LogColorSpaceW >
{
public:
                EmfCreateColorSpaceWAction()
                : EmfCreateColorSpaceBaseAction< LogColorSpaceW >( EMR_CREATECOLORSPACE_ACTION ) {}

};

class VCL_DLLPUBLIC EmfExtCreateFontIndirectWAction : public EmfAction
{
    sal_uInt32  mnFontIndex;
    LogFont*    mpLogFont;

protected:
    virtual     ~EmfExtCreateFontIndirectWAction() {}

public:
                EmfExtCreateFontIndirectWAction()
                : EmfAction( EMR_EXTCREATEFONTINDIRECTW_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnFontIndex);

        SAL_INFO("vcl.emf", "\tFont index: " << mnFontIndex);

        if (nSize - 12 > 92)
        {
            if (nSize - 12 > 348)
                mpLogFont = new LogFontExDv();
            else
                mpLogFont = new LogFontEx();
        }
        else if (nSize == 92)
            mpLogFont = new LogFont();
        else
            SAL_WARN("vcl.emf", "Not a valid LogFont!");

        mpLogFont->Read(rIStm);
    }
};


static DIBColors decodeDIBColors( sal_uInt32 nDIBColors )
{
    DIBColors eDIBColors;

    switch (nDIBColors)
    {
        case 0x00:
        {
            SAL_INFO("vcl.emf", "\tDIB colors: DIB_RGB_COLORS");
            eDIBColors = DIB_RGB_COLORS;
        }
        break;

        case 0x01:
        {
            SAL_INFO("vcl.emf", "\tDIB colors: DIB_PAL_COLORS");
            eDIBColors = DIB_PAL_COLORS;
        }
        break;

        case 0x03:
        {
            SAL_INFO("vcl.emf", "\tDIB colors: DIB_PAL_INDICES");
            eDIBColors = DIB_PAL_INDICES;
        }
        break;

        default:
            SAL_WARN("vcl.emf", "DIB colors value is invalid (0x" << std::hex << nDIBColors << std::dec << ")");
    }

    return eDIBColors;
}

class VCL_DLLPUBLIC EmfCreateDIBPatternBrushGenericAction : public EmfAction
{
    sal_uInt32  mnBrushIndex;
    DIBColors meDIBColors;
    Bitmap maBitmap;

protected:
    virtual     ~EmfCreateDIBPatternBrushGenericAction() {}

public:
                EmfCreateDIBPatternBrushGenericAction( sal_uInt16 nType )
                : EmfAction( nType ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnBrushIndex);
        SAL_INFO("vcl.emf", "\tDIB brush index: " << mnBrushIndex);

        sal_uInt32 nDIBColors=0;
        rIStm.ReadUInt32(nDIBColors);

        meDIBColors = decodeDIBColors(nDIBColors);

        sal_uInt32 nOffsetDIBHeader=0, nOffsetDIB=0;
        rIStm.ReadUInt32(nOffsetDIBHeader).ReadUInt32(nOffsetDIB);

        SAL_INFO("vcl.emf", "\tDIB read");
        ReadDIB( maBitmap, rIStm, false);
    }
};


class VCL_DLLPUBLIC EmfCreateMonoBrushAction : public EmfCreateDIBPatternBrushGenericAction
{
protected:
    virtual     ~EmfCreateMonoBrushAction() {}

public:
                EmfCreateMonoBrushAction()
                : EmfCreateDIBPatternBrushGenericAction( EMR_CREATEMONOBRUSH_ACTION ) {}
};


class VCL_DLLPUBLIC EmfCreateDIBPatternBrushAction : public EmfCreateDIBPatternBrushGenericAction
{
protected:
    virtual     ~EmfCreateDIBPatternBrushAction() {}

public:
                EmfCreateDIBPatternBrushAction()
                : EmfCreateDIBPatternBrushGenericAction( EMR_CREATEDIBPATTERNBRUSHPT_ACTION ) {}
};


class VCL_DLLPUBLIC EmfCreatePaletteAction : public EmfAction
{
    sal_uInt32  mnPaletteIndex;
    LogPalette  maLogPalette;

protected:
    virtual     ~EmfCreatePaletteAction() {}

public:
                EmfCreatePaletteAction()
                : EmfAction( EMR_CREATEPALETTE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnPaletteIndex);
        SAL_INFO("vcl.emf", "\tPalette index: " << mnPaletteIndex);

        SAL_INFO("vcl.emf", "\tLog palette:");
        maLogPalette.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfCreatePenAction : public EmfAction
{
    sal_uInt32  mnPenIndex;
    LogPen  maLogPen;

protected:
    virtual     ~EmfCreatePenAction() {}

public:
                EmfCreatePenAction()
                : EmfAction( EMR_CREATEPEN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnPenIndex);
        SAL_INFO("vcl.emf", "\tPen index: " << mnPenIndex);

        SAL_INFO("vcl.emf", "\tLog pen:");
        maLogPen.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfExtCreatePenAction : public EmfAction
{
    sal_uInt32  mnPenIndex;
    LogPenEx    maLogPenEx;
    Bitmap      maDIB;

protected:
    virtual     ~EmfExtCreatePenAction() {}

public:
                EmfExtCreatePenAction()
                : EmfAction( EMR_EXTCREATEPEN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt64 nStartRecordOffset = ( rIStm.Tell() - 4);

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnPenIndex);
        SAL_INFO("vcl.emf", "\tPen index: " << mnPenIndex);

        sal_uInt32 nDIBHeaderOffset=0;
        rIStm.ReadUInt32(nDIBHeaderOffset);
        SAL_INFO("vcl.emf", "\tDIB header offset: " << nDIBHeaderOffset);

        sal_uInt32 nDIBHeaderSize=0;
        rIStm.ReadUInt32(nDIBHeaderSize);
        SAL_INFO("vcl.emf", "\tDIB header size: " << nDIBHeaderSize);

        sal_uInt32 nDIBOffset=0;
        rIStm.ReadUInt32(nDIBOffset);
        SAL_INFO("vcl.emf", "\tDIB offset: " << nDIBOffset);

        sal_uInt32 nDIBSize=0;
        rIStm.ReadUInt32(nDIBSize);
        SAL_INFO("vcl.emf", "\tDIB size: " << nDIBSize);

        SAL_INFO("vcl.emf", "\tLog pen:");
        maLogPenEx.Read(rIStm);

        rIStm.Seek(nStartRecordOffset + nDIBHeaderOffset);
        if (nDIBHeaderOffset != nSize)
        {
            SAL_INFO("vcl.emf", "\tReading DIB");
            ReadDIB( maDIB, rIStm, false );
        }
        else
        {
            rIStm.Seek( nStartRecordOffset + nSize );
        }
    }
};



// Object manipulation


class VCL_DLLPUBLIC EmfSelectObjectAction : public EmfAction
{
    sal_uInt32  mnObjectIndex;

protected:
    virtual     ~EmfSelectObjectAction() {}

public:
                EmfSelectObjectAction()
                : EmfAction( EMR_SELECTOBJECT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnObjectIndex);

        if (mnObjectIndex == 0)
            SAL_WARN("vcl.emf", "Invalid object index! Cannot be 0 as this is reserved");

        SAL_INFO("vcl.emf", "\tObject index: " << mnObjectIndex);
    }
};


class VCL_DLLPUBLIC EmfDeleteObjectAction : public EmfAction
{
    sal_uInt32  mnObjectIndex;

protected:
    virtual     ~EmfDeleteObjectAction() {}

public:
                EmfDeleteObjectAction()
                : EmfAction( EMR_DELETEOBJECT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnObjectIndex);

        if (mnObjectIndex == 0)
            SAL_WARN("vcl.emf", "Invalid object index! Cannot be 0 as this is reserved");

        SAL_INFO("vcl.emf", "\tObject index: " << mnObjectIndex);
    }
};


class VCL_DLLPUBLIC EmfColorCorrectPaletteAction : public EmfAction
{
    sal_uInt32 mnPaletteIndex;
    sal_uInt32 mnFirstPaletteEntry;
    sal_uInt32 mnNumEntries;

protected:
    virtual     ~EmfColorCorrectPaletteAction() {}

public:
                EmfColorCorrectPaletteAction()
                : EmfAction( EMR_COLORCORRECTPALETTE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnPaletteIndex);
        SAL_INFO("vcl.emf", "\tPalette index: " << mnPaletteIndex);

        rIStm.ReadUInt32(mnFirstPaletteEntry);
        SAL_INFO("vcl.emf", "\tFirst palette entry: " << mnFirstPaletteEntry);

        rIStm.ReadUInt32(mnNumEntries);
        SAL_INFO("vcl.emf", "\tNumber of entries: " << mnNumEntries);

        rIStm.SeekRel( 4L );
    }
};


class VCL_DLLPUBLIC EmfResizePaletteAction : public EmfAction
{
    sal_uInt32 mnPaletteIndex;
    sal_uInt32 mnNumEntries;

protected:
    virtual     ~EmfResizePaletteAction() {}

public:
                EmfResizePaletteAction()
                : EmfAction( EMR_RESIZEPALETTE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnPaletteIndex);
        SAL_INFO("vcl.emf", "\tPalette index: " << mnPaletteIndex);

        rIStm.ReadUInt32(mnNumEntries);

        if (mnNumEntries > 0x00000400)
            SAL_WARN("vcl.emf", "More than 1024 entries to be resized! Invalid value (" << mnNumEntries << ")");

        SAL_INFO("vcl.emf", "\tNumber of entries: " << mnNumEntries);
    }
};


class VCL_DLLPUBLIC EmfSelectPaletteAction : public EmfAction
{
    sal_uInt32  mnPaletteIndex;

protected:
    virtual     ~EmfSelectPaletteAction() {}

public:
                EmfSelectPaletteAction()
                : EmfAction( EMR_SELECTPALETTE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnPaletteIndex);

        if (mnPaletteIndex == 0)
            SAL_WARN("vcl.emf", "Invalid object index! Cannot be 0 as this is reserved");

        SAL_INFO("vcl.emf", "\tPalette index: " << mnPaletteIndex);
    }
};


class VCL_DLLPUBLIC EmfSetColorSpaceAction : public EmfAction
{
    sal_uInt32  mnColorSpaceIndex;

protected:
    virtual     ~EmfSetColorSpaceAction() {}

public:
                EmfSetColorSpaceAction()
                : EmfAction( EMR_SETCOLORSPACE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnColorSpaceIndex);
        SAL_INFO("vcl.emf", "\tColorspace index: " << mnColorSpaceIndex);
    }
};


class VCL_DLLPUBLIC EmfDeleteColorSpaceAction : public EmfAction
{
    sal_uInt32  mnColorSpaceIndex;

protected:
    virtual     ~EmfDeleteColorSpaceAction() {}

public:
                EmfDeleteColorSpaceAction()
                : EmfAction( EMR_DELETECOLORSPACE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnColorSpaceIndex);
        SAL_INFO("vcl.emf", "\tColorspace index: " << mnColorSpaceIndex);
    }
};

class VCL_DLLPUBLIC EmfSetPaletteEntriesAction : public EmfAction
{
    sal_uInt32 mnPaletteIndex;
    sal_uInt32 mnFirstPaletteEntry;
    sal_uInt32 mnNumEntries;
    std::vector< LogPaletteEntry > maPalEntries;

protected:
    virtual     ~EmfSetPaletteEntriesAction() {}

public:
                EmfSetPaletteEntriesAction()
                : EmfAction( EMR_SETPALETTEENTRIES_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(mnPaletteIndex);
        SAL_INFO("vcl.emf", "\tPalette index: " << mnPaletteIndex);

        rIStm.ReadUInt32(mnFirstPaletteEntry);
        SAL_INFO("vcl.emf", "\tFirst palette entry: " << mnFirstPaletteEntry);

        rIStm.ReadUInt32(mnNumEntries);
        SAL_INFO("vcl.emf", "\tNumber of entries: " << mnNumEntries);

        for (sal_uInt32 i=0; i < mnNumEntries; i++)
        {
            LogPaletteEntry aPalEntry;
            SAL_INFO("vcl.emf", "\tPalette entry " << i << ":");
            aPalEntry.Read(rIStm);

            maPalEntries.push_back( aPalEntry );
        }
    }
};



// Clipping


class VCL_DLLPUBLIC EmfExcludeClipRectAction : public EmfAction
{
    RectL       maClip;

protected:
    virtual     ~EmfExcludeClipRectAction() {}

public:
                EmfExcludeClipRectAction()
                : EmfAction( EMR_EXCLUDECLIPRECT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.ReadUInt32(maClip.mnLeft);
        rIStm.ReadUInt32(maClip.mnTop);
        rIStm.ReadUInt32(maClip.mnRight);
        rIStm.ReadUInt32(maClip.mnBottom);

        SAL_INFO("vcl.emf", "\tClipping rectangle: " << maClip.mnLeft << ", " << maClip.mnTop << ", " << maClip.mnRight << ", " << maClip.mnBottom);
    }
};


class VCL_DLLPUBLIC EmfExtSelectClipRegionAction : public EmfAction
{
    RegionMode  meRegionMode;
    RegionData  maRgnData;

protected:
    virtual     ~EmfExtSelectClipRegionAction() {}

public:
                EmfExtSelectClipRegionAction()
                : EmfAction( EMR_EXTSELECTCLIPRGN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        sal_uInt32 nRgnDataSize=0;
        rIStm.ReadUInt32(nRgnDataSize);

        sal_uInt32 nRegionMode=0;
        rIStm.ReadUInt32(nRegionMode);

        switch (nRegionMode)
        {
            case 0x01:
            {
                meRegionMode = RGN_AND_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_AND");
            }
            break;

            case 0x02:
            {
                meRegionMode = RGN_OR_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_OR");
            }
            break;

            case 0x03:
            {
                meRegionMode = RGN_XOR_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_XOR");
            }
            break;

            case 0x04:
            {
                meRegionMode = RGN_DIFF_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_DIFF");
            }
            break;

            case 0x05:
            {
                meRegionMode = RGN_COPY_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_COPY");
            }
            break;

            default:
                SAL_WARN("vcl.emf", "Invalid value for region mode - value is 0x" << std::hex << nRegionMode << std::dec);
        }

        if (meRegionMode != RGN_COPY_EMF)
        {
            SAL_INFO("vcl.emf", "\tRegion data:");
            maRgnData.Read(rIStm);
        }
    }
};


class VCL_DLLPUBLIC EmfIntersectClipRectAction : public EmfAction
{
    RectL maClip;

protected:
    virtual     ~EmfIntersectClipRectAction() {}

public:
                EmfIntersectClipRectAction()
                : EmfAction( EMR_INTERSECTCLIPRECT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tClip rectangle:");
        maClip.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfOffsetClipRegionAction : public EmfAction
{
    PointL maOffset;

protected:
    virtual     ~EmfOffsetClipRegionAction() {}

public:
                EmfOffsetClipRegionAction()
                : EmfAction( EMR_OFFSETCLIPRGN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tOffset:");
        maOffset.Read(rIStm);
    }
};



class VCL_DLLPUBLIC EmfSelectClipPathAction : public EmfAction
{
    RegionMode  meRegionMode;

protected:
    virtual     ~EmfSelectClipPathAction() {}

public:
                EmfSelectClipPathAction()
                : EmfAction( EMR_SELECTCLIPPATH_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;

        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        sal_uInt32 nRegionMode=0;
        rIStm.ReadUInt32(nRegionMode);

        switch (nRegionMode)
        {
            case 0x01:
            {
                meRegionMode = RGN_AND_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_AND");
            }
            break;

            case 0x02:
            {
                meRegionMode = RGN_OR_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_OR");
            }
            break;

            case 0x03:
            {
                meRegionMode = RGN_XOR_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_XOR");
            }
            break;

            case 0x04:
            {
                meRegionMode = RGN_DIFF_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_DIFF");
            }
            break;

            case 0x05:
            {
                meRegionMode = RGN_COPY_EMF;
                SAL_INFO("vcl.emf", "\tRegion mode: RGN_COPY");
            }
            break;

            default:
                SAL_WARN("vcl.emf", "Invalid value for region mode - value is 0x" << std::hex << nRegionMode << std::dec);
        }

    }
};


class VCL_DLLPUBLIC EmfSetMetaRegionAction : public EmfAction
{
protected:
    virtual     ~EmfSetMetaRegionAction() {}

public:
                EmfSetMetaRegionAction()
                : EmfAction( EMR_INVERTRGN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);
    }
};

// Drawing

class VCL_DLLPUBLIC EmfAngleArcAction : public EmfAction
{
    PointL      maCenter;
    sal_uInt32  mnRadius;
    float       mfStartAngle;
    float       mfSweepAngle;

protected:
    virtual     ~EmfAngleArcAction() {}

public:
                EmfAngleArcAction()
                : EmfAction( EMR_ANGLEARC_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tCenter:");
        maCenter.Read(rIStm);

        rIStm.ReadUInt32(mnRadius);
        SAL_INFO("vcl.emf", "\tRadius: " << mnRadius);

        rIStm.ReadFloat(mfStartAngle);
        SAL_INFO("vcl.emf", "\tStart angle: " << mfStartAngle);

        rIStm.ReadFloat(mfSweepAngle);
        SAL_INFO("vcl.emf", "\tSweep angle: " << mfSweepAngle);
    }
};


class VCL_DLLPUBLIC EmfArcAction : public EmfAction
{
    RectL   maBox;
    PointL  maStart;
    PointL  maEnd;

protected:
    virtual     ~EmfArcAction() {}

public:
                EmfArcAction()
                : EmfAction( EMR_ARC_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "Bounding box:");
        maBox.Read(rIStm);

        SAL_INFO("vcl.emf", "Start point:");
        maStart.Read(rIStm);

        SAL_INFO("vcl.emf", "End point:");
        maEnd.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfArcToAction : public EmfAction
{
    RectL   maBox;
    PointL  maStart;
    PointL  maEnd;

protected:
    virtual     ~EmfArcToAction() {}

public:
                EmfArcToAction()
                : EmfAction( EMR_ARCTO_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "Bounding box:");
        maBox.Read(rIStm);

        SAL_INFO("vcl.emf", "Start point:");
        maStart.Read(rIStm);

        SAL_INFO("vcl.emf", "End point:");
        maEnd.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfChordAction : public EmfAction
{
    RectL   maBox;
    PointL  maStart;
    PointL  maEnd;

protected:
    virtual     ~EmfChordAction() {}

public:
                EmfChordAction()
                : EmfAction( EMR_CHORD_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding box:");
        maBox.Read(rIStm);

        SAL_INFO("vcl.emf", "\tStart point:");
        maStart.Read(rIStm);

        SAL_INFO("vcl.emf", "\tEnd point:");
        maEnd.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfEllipseAction : public EmfAction
{
    RectL   maBox;

protected:
    virtual     ~EmfEllipseAction() {}

public:
                EmfEllipseAction()
                : EmfAction( EMR_ELLIPSE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding box:");
        maBox.Read(rIStm);
    }
};

class VCL_DLLPUBLIC EmfExtFloodFillAction : public EmfAction
{
    PointL   maStart;
    ColorRef maColor;
    FloodFill meFloodFillMode;

protected:
    virtual     ~EmfExtFloodFillAction() {}

public:
                EmfExtFloodFillAction()
                : EmfAction( EMR_EXTFLOODFILL_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tStart point:");
        maStart.Read(rIStm);

        SAL_INFO("vcl.emf", "\tColor:");
        maColor.Read(rIStm);

        sal_uInt32 nFloodFillMode=0;
        rIStm.ReadUInt32(nFloodFillMode);

        switch (nFloodFillMode)
        {
            case 0x00000000:
            {
                SAL_INFO("vcl.emf", "\tFlood fill mode: fill border");
                meFloodFillMode = FLOODFILLBORDER;
            }
            break;

            case 0x00000001:
            {
                SAL_INFO("vcl.emf", "\tFlood fill mode: fill surface");
                meFloodFillMode = FLOODFILLSURFACE;
            }
            break;

            default:
                SAL_WARN("vcl.emf", "Flood fill mode value is invalid (0x" << std::hex << nFloodFillMode << std::dec << ")");
        }
    }
};

static GraphicsMode readGraphicsMode( SvStream& rIStm )
{
    GraphicsMode eGraphicsMode;
    sal_uInt32 nGraphicsMode=0;
    rIStm.ReadUInt32(nGraphicsMode);

    switch (nGraphicsMode)
    {
        case 0x00000001:
        {
            eGraphicsMode = GM_COMPATIBLE_EMF;
            SAL_INFO("vcl.emf", "\tGraphics mode: GM_COMPATIBLE");
        }
        break;

        case 0x00000002:
        {
            eGraphicsMode = GM_ADVANCED_EMF;
            SAL_INFO("vcl.emf", "\tGraphics mode: GM_ADVANCED");
        }
        break;

        default:
            SAL_WARN("vcl.emf", "Graphics mode value is invalid (0x" << std::hex << ")");
    }

    return eGraphicsMode;
}

class VCL_DLLPUBLIC EmfExtTextOutAction: public EmfAction
{
    friend class EmfExtTextOutAAction;
    friend class EmfExtTextOutWAction;

    RectL           maBounds; /// ignore this, see [MS-EMF] section 2.3.5.7
    GraphicsMode    meGraphicsMode;
    float           mfXScale;
    float           mfYScale;
    EmrText         maEmrText;

    sal_uInt64      mnOriginal;

protected:
    virtual     ~EmfExtTextOutAction() {}

public:
                EmfExtTextOutAction( sal_uInt32 nAction )
                : EmfAction( nAction ) {}

    virtual void Read( SvStream& rIStm )
    {
        mnOriginal = rIStm.Tell();

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds rectangle (must be ignored):");
        maBounds.Read(rIStm);

        meGraphicsMode = readGraphicsMode(rIStm);

        rIStm.ReadFloat(mfXScale);
        SAL_INFO("vcl.emf", "\tX scale: " << mfXScale);

        rIStm.ReadFloat(mfYScale);
        SAL_INFO("vcl.emf", "\tY scale: " << mfXScale);

    }
};

class VCL_DLLPUBLIC EmfExtTextOutAAction: public EmfExtTextOutAction
{
public:
                EmfExtTextOutAAction()
                : EmfExtTextOutAction( EMR_EXTTEXTOUTA_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        EmfExtTextOutAction::Read(rIStm);

        SAL_INFO("vcl.emf", "\tEmrText structure:");
        bool bUnicode = false;
        maEmrText.Read(rIStm, mnOriginal, bUnicode);
    }
};

class VCL_DLLPUBLIC EmfExtTextOutWAction: public EmfExtTextOutAction
{
public:
                EmfExtTextOutWAction()
                : EmfExtTextOutAction( EMR_EXTTEXTOUTW_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        EmfExtTextOutAction::Read(rIStm);

        SAL_INFO("vcl.emf", "\tText:");
        bool bUnicode = true;
        maEmrText.Read(rIStm, mnOriginal, bUnicode);
    }
};


class VCL_DLLPUBLIC EmfFillPathAction : public EmfAction
{
    RectL       maBounds;

protected:
    virtual     ~EmfFillPathAction() {}

public:
                EmfFillPathAction()
                : EmfAction( EMR_FILLPATH_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds:");
        maBounds.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfFillRegionAction : public EmfAction
{
    RectL       maBounds;
    sal_uInt32  mnBrushIndex;
    RegionData  maRegionData;

protected:
    virtual     ~EmfFillRegionAction() {}

public:
                EmfFillRegionAction()
                : EmfAction( EMR_FILLRGN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds:");
        maBounds.Read(rIStm);

        sal_uInt32 nRgnDataSize=0;
        rIStm.ReadUInt32(nRgnDataSize);

        rIStm.ReadUInt32(mnBrushIndex);
        SAL_INFO("vcl.emf", "\tBrush index: " << mnBrushIndex);

        SAL_INFO("vcl.emf", "\tRegion data:");
        maRegionData.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfFrameRegionAction : public EmfAction
{
    RectL       maBounds;
    sal_uInt32  mnBrushIndex;
    sal_Int32   mnWidth;
    sal_Int32   mnHeight;
    RegionData  maRegionData;

protected:
    virtual     ~EmfFrameRegionAction() {}

public:
                EmfFrameRegionAction()
                : EmfAction( EMR_FRAMERGN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds:");
        maBounds.Read(rIStm);

        sal_uInt32 nRgnDataSize=0;
        rIStm.ReadUInt32(nRgnDataSize);

        rIStm.ReadUInt32(mnBrushIndex);
        SAL_INFO("vcl.emf", "\tBrush index: " << mnBrushIndex);

        rIStm.ReadInt32(mnWidth);
        SAL_INFO("vcl.emf", "\tWidth: " << mnWidth);

        rIStm.ReadInt32(mnHeight);
        SAL_INFO("vcl.emf", "\tHeight: " << mnHeight);

        SAL_INFO("vcl.emf", "\tRegion data:");
        maRegionData.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfGradientFillAction: public EmfAction
{
    RectL           maBounds;
    GradientFill    meGradientFillMode;
    std::vector< TriVertex > maTriVertexes;
    VertexObjects   maVertexObjects;

protected:
    virtual     ~EmfGradientFillAction() {}

public:
                EmfGradientFillAction()
                : EmfAction( EMR_GRADIENTFILL_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds:");
        maBounds.Read(rIStm);

        sal_uInt32 nVer=0;
        rIStm.ReadUInt32(nVer);

        sal_uInt32 nTri=0;
        rIStm.ReadUInt32(nTri);

        sal_uInt32 nMode=0;
        rIStm.ReadUInt32(nMode);

        switch (nMode)
        {
            case 0x00000000:
            {
                SAL_INFO("vcl.emf", "\tGradient fill mode: GRADIENT_FILL_RECT_H");
                meGradientFillMode = GRADIENT_FILL_RECT_H;
            }
            break;

            case 0x00000001:
            {
                SAL_INFO("vcl.emf", "\tGradient fill mode: GRADIENT_FILL_RECT_V");
                meGradientFillMode = GRADIENT_FILL_RECT_V;
            }
            break;

            case 0x00000003:
            {
                SAL_INFO("vcl.emf", "\tGradient fill mode: GRADIENT_FILL_TRIANGLE");
                meGradientFillMode = GRADIENT_FILL_TRIANGLE;
            }
            break;

            default:
                SAL_WARN("vcl.emf", "Gradient fill mode value is invalid (0x" << std::hex << nMode << std::dec);
        }

        for (sal_uInt32 i=0; i < nVer; i++)
        {
            TriVertex aTriVertex;
            aTriVertex.Read(rIStm);
            maTriVertexes.push_back(aTriVertex);
        }

        maVertexObjects.Read( rIStm, nTri, meGradientFillMode == GRADIENT_FILL_TRIANGLE ? true : false );
    }
};

void GradientRectangle::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32(mnUpper);
    rIStm.ReadUInt32(mnLower);

    SAL_INFO("vcl.emf", "\t\tGradient rectangle: " << mnUpper << ", " << mnLower);
}

void GradientTriangle::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32(mnVertex1);
    rIStm.ReadUInt32(mnVertex2);
    rIStm.ReadUInt32(mnVertex3);

    SAL_INFO("vcl.emf", "\t\tGradient triangle: " << mnVertex1 << ", " << mnVertex2 << ", " << mnVertex3);
}

void VertexObjects::Read( SvStream& rIStm, sal_uInt32 nNumObjects, bool bGradRectsUsed)
{
    if (bGradRectsUsed)
    {
        for (sal_uInt32 i=0; i < nNumObjects; i++)
        {
            GradientRectangle aGradRect;
            aGradRect.Read(rIStm);
            maVertexObjects.push_back(aGradRect);
        }

        sal_uInt64 nPos=0;
        nPos = rIStm.Tell();

        sal_uInt64 nPadding=nNumObjects*4;
        rIStm.Seek(nPos+nPadding);
    }
    else
    {
        for (sal_uInt32 i=0; i < nNumObjects; i++)
        {
            GradientTriangle aGradTriangle;
            aGradTriangle.Read(rIStm);
            maVertexObjects.push_back(aGradTriangle);
        }
    }
}


class VCL_DLLPUBLIC EmfLineToAction : public EmfAction
{
    PointL maPoint;

protected:
    virtual     ~EmfLineToAction() {}

public:
                EmfLineToAction()
                : EmfAction( EMR_LINETO_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tPoint:");
        maPoint.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfPaintRegionAction : public EmfAction
{
    RectL       maBounds;
    RegionData  maRegionData;

protected:
    virtual     ~EmfPaintRegionAction() {}

public:
                EmfPaintRegionAction()
                : EmfAction( EMR_PAINTRGN_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds:");
        maBounds.Read(rIStm);

        sal_uInt32 nRgnDataSize=0;
        rIStm.ReadUInt32(nRgnDataSize);

        SAL_INFO("vcl.emf", "\tRegion data:");
        maRegionData.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfPieAction : public EmfAction
{
    RectL   maBox;
    PointL  maStart;
    PointL  maEnd;

protected:
    virtual     ~EmfPieAction() {}

public:
                EmfPieAction()
                : EmfAction( EMR_PIE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding box:");
        maBox.Read(rIStm);

        SAL_INFO("vcl.emf", "\tStart point (endpoint of first radial):");
        maStart.Read(rIStm);

        SAL_INFO("vcl.emf", "\tEnd point (endpoint of second radial):");
        maEnd.Read(rIStm);
    }
};

template <class PointType>
class EmfPolyBezierGenericAction :
    public EmfAction,
    public EmfClosePath
{
    friend class EmfPolyBezierAction;
    friend class EmfPolyBezier16Action;

    RectL maBounds;
    std::vector< PointType > maPoints;
    std::vector< PointType > maIgnoredPoints;
    sal_uInt32 mnCurrentPosition;

protected:
    virtual     ~EmfPolyBezierGenericAction() {}

public:
                EmfPolyBezierGenericAction( sal_uInt16 nType )
                : EmfAction( nType ) {}

    virtual void ClosePath()
    {
        if ((maPoints[0].mnX == maPoints[maPoints.size()].mnX) &&
            (maPoints[0].mnY == maPoints[maPoints.size()].mnY))
        {
            SAL_INFO("vcl.emf", "\tClosing Bezier");
            return;
        }

        if (maIgnoredPoints.size() == 2)
        {
            SAL_INFO("vcl.emf", "\tClosing bezier:");
            SAL_INFO("vcl.emf", "\t\tPoint: (" << maPoints[0].mnX << ", " << maPoints[0].mnY << ")");

            maPoints.push_back(maIgnoredPoints[0]);
            maPoints.push_back(maIgnoredPoints[1]);
        }

        maPoints.push_back(maPoints[0]);

        EmfClosePath::ClosePath();
    }

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds:");
        maBounds.Read(rIStm);

        sal_uInt32 nCount=0;
        rIStm.ReadUInt32(nCount);

        sal_uInt32 nPointCounter=0;

        SAL_INFO("vcl.emf", "\tFirst point:");
        PointType aFirstPoint;
        aFirstPoint.Read(rIStm);
        maPoints.push_back(aFirstPoint);

        nPointCounter++;

        while (nPointCounter < nCount)
        {
            if ((nCount - nPointCounter) == 1)
            {
                SAL_WARN("vcl.warn", "There are too many points in this bezier! Ignoring one point");

                SAL_INFO("vcl.emf", "\tIgnored point: ");
                PointType aIgnoredPoint;
                aIgnoredPoint.Read(rIStm);
                maIgnoredPoints.push_back(aIgnoredPoint);

                nPointCounter++;
            }
            else if ((nCount - nPointCounter) == 2)
            {
                PointType aPoint1;
                SAL_INFO("vcl.emf", "\tPossible control point 1: ");
                aPoint1.Read(rIStm);
                maIgnoredPoints.push_back(aPoint1);

                SAL_INFO("vcl.emf", "\tPossible control point 2: ");
                PointType aPoint2;
                aPoint2.Read(rIStm);
                maIgnoredPoints.push_back(aPoint2);

                nPointCounter += 2;
                continue;
            }

            if (nPointCounter != 1)
            {
                SAL_INFO("vcl.emf", "\tStart point: ");
                SAL_INFO("vcl.emf", "\t\tPoint: (" << aFirstPoint.mnX << ", " << aFirstPoint.mnY << ")");
            }

            SAL_INFO("vcl.emf", "\tControl point 1:");
            PointType aControlPoint1;
            aControlPoint1.Read(rIStm);
            maPoints.push_back(aControlPoint1);
            nPointCounter++;

            SAL_INFO("vcl.emf", "\tControl point 2:");
            PointType aControlPoint2;
            aControlPoint2.Read(rIStm);
            maPoints.push_back(aControlPoint2);
            nPointCounter++;

            SAL_INFO("vcl.emf", "\tEnd point:");
            PointType aEndPoint;
            aEndPoint.Read(rIStm);
            maPoints.push_back(aEndPoint);
            nPointCounter++;

            aFirstPoint = aEndPoint;
        }
    }
};


class VCL_DLLPUBLIC EmfPolyBezierAction : public EmfPolyBezierGenericAction< PointL >
{
protected:
    virtual     ~EmfPolyBezierAction() {}

public:
                EmfPolyBezierAction()
                : EmfPolyBezierGenericAction< PointL > ( EMR_POLYBEZIER_ACTION ) {}

};


class VCL_DLLPUBLIC EmfPolyBezier16Action : public EmfPolyBezierGenericAction< PointS >
{
protected:
    virtual     ~EmfPolyBezier16Action() {}

public:
                EmfPolyBezier16Action()
                : EmfPolyBezierGenericAction< PointS > ( EMR_POLYBEZIER16_ACTION ) {}

};

template <class PointType>
class EmfPolyDrawGenericAction :
    public EmfAction,
    public EmfClosePath
{
    friend class EmfPolyDrawAction;
    friend class EmfPolyDraw16Action;

    RectL maBounds;
    std::vector< PointType > maPoints;
    std::vector< PointEMF >  maPointTypes;

protected:
    virtual     ~EmfPolyDrawGenericAction() {}

public:
                EmfPolyDrawGenericAction( sal_uInt16 nType )
                : EmfAction( nType ) {}

    virtual void ClosePath()
    {
        if ( ( (maPoints[0].mnX == maPoints[maPoints.size()].mnX) && (maPoints[0].mnY == maPoints[maPoints.size()].mnY) ) ||
             ( maPointTypes[maPointTypes.size()] == PT_CLOSEFIGURE ) )
        {
            SAL_INFO("vcl.emf", "\tBezier closed");
            return;
        }

        PointEMF aPointType = PT_CLOSEFIGURE;
        maPointTypes.push_back(aPointType);
        SAL_INFO("vcl.emf", "\tPoint type: PT_CLOSEFIGURE");
        maPoints.push_back(maPoints[0]);

        EmfClosePath::ClosePath();
    }

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds: ");
        maBounds.Read(rIStm);

        sal_uInt32 nNumPoints=0;
        rIStm.ReadUInt32(nNumPoints);
        SAL_INFO("vcl.emf", "\tNumber of points: " << nNumPoints);

        for (sal_uInt32 i=0; i < nNumPoints; i++)
        {
            PointType aPoint;
            SAL_INFO("vcl.emf", "\tPoint " << i+1 << ":");
            aPoint.Read(rIStm);
            maPoints.push_back(aPoint);
        }

        for (sal_uInt32 i=0; i < nNumPoints; i++)
        {
            PointEMF aPointType;
            sal_uInt8 nPointType=0;

            rIStm.ReadUChar(nPointType);

            switch (nPointType)
            {
                case 0x01:
                {
                    aPointType = PT_CLOSEFIGURE;
                    maPointTypes.push_back(aPointType);
                    SAL_INFO("vcl.emf", "\tPoint " << i+1 << " type: PT_CLOSEFIGURE");
                }
                break;

                case 0x02:
                {
                    aPointType = PT_LINETO;
                    maPointTypes.push_back(aPointType);
                    SAL_INFO("vcl.emf", "\tPoint " << i+1 << " type: PT_LINETO");
                }
                break;

                case 0x03:
                {
                    aPointType = PT_BEZIERTO;
                    maPointTypes.push_back(aPointType);
                    SAL_INFO("vcl.emf", "\tPoint " << i+1 << " type: PT_BEZIERTO");
                }
                break;

                case 0x04:
                {
                    aPointType = PT_MOVETO;
                    maPointTypes.push_back(aPointType);
                    SAL_INFO("vcl.emf", "\tPoint " << i+1 << " type: PT_MOVETO");
                }
                break;

                default:
                    SAL_WARN("vcl.emf", "Point type invalid value (0x" << std::hex << (sal_uInt8) nPointType << ")" << std::dec);
            }
        }
    }
};


class VCL_DLLPUBLIC EmfPolyDrawAction : public EmfPolyDrawGenericAction< PointL >
{
protected:
    virtual     ~EmfPolyDrawAction() {}

public:
                EmfPolyDrawAction()
                : EmfPolyDrawGenericAction< PointL > ( EMR_POLYDRAW_ACTION ) {}

};


class VCL_DLLPUBLIC EmfPolyDraw16Action : public EmfPolyDrawGenericAction< PointS >
{
protected:
    virtual     ~EmfPolyDraw16Action() {}

public:
                EmfPolyDraw16Action()
                : EmfPolyDrawGenericAction< PointS > ( EMR_POLYDRAW16_ACTION ) {}

};

template < class PointType >
class VCL_DLLPUBLIC EmfPolyAction :
    public EmfAction,
    public EmfClosePath
{
    friend class EmfPolygonAction;
    friend class EmfPolyLineAction;
    friend class EmfPolygon16Action;
    friend class EmfPolyLine16Action;

    RectL maBounds;
    std::vector< PointType > maPoints;
    sal_uChar* mpSkippedPoints;

protected:
    virtual     ~EmfPolyAction() {}

public:
                EmfPolyAction( sal_uInt16 nType )
                : EmfAction( nType ) {}

    virtual void ClosePath()
    {
        if ((maPoints[0].mnX == maPoints[maPoints.size()].mnX) &&
            (maPoints[0].mnY == maPoints[maPoints.size()].mnY))
        {
            SAL_INFO("vcl.emf", "\tBezier closed");
            return;
        }

        SAL_INFO("vcl.emf", "\tClosing bezier:");
        SAL_INFO("vcl.emf", "\t\tPoint: (" << maPoints[0].mnX << ", " << maPoints[0].mnY << ")");
        maPoints.push_back(maPoints[0]);

        EmfClosePath::ClosePath();
    }

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds: ");
        maBounds.Read(rIStm);

        sal_uInt32 nNumPoints=0;
        rIStm.ReadUInt32(nNumPoints);
        SAL_INFO("vcl.emf", "\tNumber of points: " << nNumPoints);

        // See [MS-EMF] section 2.3.5.24, et al. - the maximum number of points
        // a Poly-type record can have is 16K
        for (sal_uInt32 i=0; i < nNumPoints && i <= (16 * 1024); i++)
        {
            PointType aPoint;
            SAL_INFO("vcl.emf", "\tPoint " << i+1 << ":");
            aPoint.Read(rIStm);
            maPoints.push_back(aPoint);
        }

        // We don't want to actually modify the record though, so we'll stash the
        // extra points in a temporary pointer. If the record gets modified, then
        // we'll discard them. Otherwise, we'll write them back at a later date.

        if (nNumPoints > (16 * 1024))
        {
            rIStm.Read(mpSkippedPoints, (nNumPoints - (16 * 1024)) * 64);
            SAL_INFO("vcl.emf", "\tSkipping " << nNumPoints - (16 * 1024) << " points");
        }
    }
};



class VCL_DLLPUBLIC EmfPolygonAction : public EmfPolyAction< PointL >
{
protected:
    virtual     ~EmfPolygonAction() {}

public:
                EmfPolygonAction()
                : EmfPolyAction< PointL >( EMR_POLYGON_ACTION ) {}
};


class VCL_DLLPUBLIC EmfPolylineAction : public EmfPolyAction< PointL >
{
protected:
    virtual     ~EmfPolylineAction() {}

public:
                EmfPolylineAction()
                : EmfPolyAction< PointL >( EMR_POLYLINE_ACTION ) {}
};


class VCL_DLLPUBLIC EmfPolygon16Action : public EmfPolyAction< PointS >
{
protected:
    virtual     ~EmfPolygon16Action() {}

public:
                EmfPolygon16Action()
                : EmfPolyAction< PointS >( EMR_POLYGON16_ACTION ) {}
};


class VCL_DLLPUBLIC EmfPolyline16Action : public EmfPolyAction< PointS >
{
protected:
    virtual     ~EmfPolyline16Action() {}

public:
                EmfPolyline16Action()
                : EmfPolyAction< PointS >( EMR_POLYLINE16_ACTION ) {}
};

template < class PointType >
class VCL_DLLPUBLIC EmfPolyPolyAction : public EmfAction
{
    friend class EmfPolyPolygonAction;
    friend class EmfPolyPolylineAction;
    friend class EmfPolyPolygon16Action;
    friend class EmfPolyPolyline16Action;

    RectL maBounds;

    std::vector< sal_uInt32 > maPolygonPointCounts;
    std::vector< EmfPolygon< PointType > > maPolygons;
    std::vector< sal_uInt32 > maOverrunPolygonCounts;
    std::vector< EmfPolygon< PointType > > maOverrunPolygons;

protected:
    virtual     ~EmfPolyPolyAction() {}

public:
                EmfPolyPolyAction( sal_uInt16 nType )
                : EmfAction(nType) {}

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds: ");
        maBounds.Read(rIStm);

        sal_uInt32 nNumPolygons=0;
        rIStm.ReadUInt32(nNumPolygons);
        SAL_INFO("vcl.emf", "\tNumber of polygons: " << nNumPolygons);

        sal_uInt32 nNumPoints=0;
        rIStm.ReadUInt32(nNumPoints);
        SAL_INFO("vcl.emf", "\tNumber of points: " << nNumPoints);

        for (sal_uInt32 i=0; i < nNumPolygons; i++)
        {
            sal_uInt32 nPointCount=0;
            rIStm.ReadUInt32(nPointCount);

            // the total number of points cannot be greater that 16K
            // if we go over that number, then just read the last polygon
            // in the polypolygon as the number of points up to the point
            // at 16K
            // We do need to know the polygon position before we truncate so
            // we can read the overrun data because we don't want to modify
            // this data unless we write to the record, in which case we will
            // drop it.
            nNumPoints += nPointCount;
            bool bOverrun=false;

            if (!bOverrun && (nNumPoints > 16 * 1024))
            {
                maOverrunPolygonCounts.push_back(nPointCount);

                nPointCount = nNumPoints - (16 * 1024);
                maPolygonPointCounts.push_back(nPointCount);

                bOverrun=true;
            }
            else if (bOverrun)
            {
                maOverrunPolygonCounts.push_back(nPointCount);
            }
            else if (!bOverrun)
            {
                maPolygonPointCounts.push_back(nPointCount);
            }
        }

        sal_uInt64 nLastPolygonPos=0;

        for (sal_uInt32 i=0; i < maPolygonPointCounts.size(); i++)
        {
            nLastPolygonPos = rIStm.Tell();
            SAL_INFO("vcl.emf", "\tPolygon[ " << i << "]:");

            for (sal_uInt32 j=0; j < maPolygonPointCounts[i]; j++)
            {
                PointType aPoint;
                SAL_INFO("vcl.emf", "\t\tPoint " << j+1 << ":");
                aPoint.Read(rIStm);
                maPolygons[i].maPoints.push_back(aPoint);
            }
        }

        // We don't want to actually modify the record though, so we'll stash the
        // extra points in a temporary vector. If the record gets modified, then
        // we'll discard them. Otherwise, we'll write them back at a later date.

        if (!maOverrunPolygonCounts.empty())
        {
            rIStm.Seek(nLastPolygonPos);

            for (sal_uInt32 i=0; i < maOverrunPolygonCounts.size(); i++)
            {
                SAL_INFO("vcl.emf", "\tOverrun polygon[ " << i << "]:");
                for (sal_uInt32 j=0; j < maOverrunPolygonCounts[i]; j++)
                {
                    PointType aPoint;
                    SAL_INFO("vcl.emf", "\t\tPoint " << j+1 << ":");
                    aPoint.Read(rIStm);
                    maOverrunPolygons[i].maPoints.push_back(aPoint);
                }
            }
        }
    }
};


class VCL_DLLPUBLIC EmfPolyPolygonAction : public EmfPolyPolyAction< PointL >
{
protected:
    virtual     ~EmfPolyPolygonAction() {}

public:
                EmfPolyPolygonAction()
                : EmfPolyPolyAction< PointL >( EMR_POLYPOLYGON_ACTION ) {}
};


class VCL_DLLPUBLIC EmfPolyPolylineAction : public EmfPolyPolyAction< PointL >
{
protected:
    virtual     ~EmfPolyPolylineAction() {}

public:
                EmfPolyPolylineAction()
                : EmfPolyPolyAction< PointL >( EMR_POLYPOLYLINE_ACTION ) {}
};


class VCL_DLLPUBLIC EmfPolyPolygon16Action : public EmfPolyPolyAction< PointS >
{
protected:
    virtual     ~EmfPolyPolygon16Action() {}

public:
                EmfPolyPolygon16Action()
                : EmfPolyPolyAction< PointS >( EMR_POLYPOLYGON16_ACTION ) {}
};


class VCL_DLLPUBLIC EmfPolyPolyline16Action : public EmfPolyPolyAction< PointS >
{
protected:
    virtual     ~EmfPolyPolyline16Action() {}

public:
                EmfPolyPolyline16Action()
                : EmfPolyPolyAction< PointS >( EMR_POLYPOLYLINE16_ACTION ) {}
};


class VCL_DLLPUBLIC EmfPolyTextOutAction : public EmfAction
{
    friend class EmfPolyTextOutAAction;
    friend class EmfPolyTextOutWAction;

    RectL maBound;
    GraphicsMode meGraphicsMode;
    float mfXScale;
    float mfYScale;
    std::vector< EmrText > maStrings;

protected:
    virtual     ~EmfPolyTextOutAction() {}

public:
                EmfPolyTextOutAction( sal_uInt16 nType )
                : EmfAction( nType ) {}

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding box:");
        maBound.Read(rIStm);

        sal_uInt32 nGraphicsMode=0;
        rIStm.ReadUInt32(nGraphicsMode);

        meGraphicsMode = readGraphicsMode(rIStm);

        rIStm.ReadFloat(mfXScale);
        SAL_INFO("vcl.emf", "\tX scale: " << mfXScale);
        rIStm.ReadFloat(mfYScale);
        SAL_INFO("vcl.emf", "\tY scale: " << mfYScale);
    }
};


class VCL_DLLPUBLIC EmfPolyTextOutAAction : public EmfPolyTextOutAction
{
protected:
    virtual     ~EmfPolyTextOutAAction() {}

public:
                EmfPolyTextOutAAction()
                : EmfPolyTextOutAction( EMR_POLYTEXTOUTA_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        EmfPolyTextOutAction::Read(rIStm);

        sal_uInt32 nStrings=0;
        rIStm.ReadUInt32(nStrings);
        SAL_INFO("vcl.emf", "\tNumber of text strings: " << nStrings);

        for (sal_uInt32 i=0; i < nStrings; i++)
        {
            EmrText aText;
            aText.Read(rIStm, rIStm.Tell(), false);
            maStrings.push_back(aText);
        }
    }
};


class VCL_DLLPUBLIC EmfPolyTextOutWAction : public EmfPolyTextOutAction
{
protected:
    virtual     ~EmfPolyTextOutWAction() {}

public:
                EmfPolyTextOutWAction()
                : EmfPolyTextOutAction( EMR_POLYTEXTOUTW_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        EmfPolyTextOutAction::Read(rIStm);

        sal_uInt32 nStrings=0;
        rIStm.ReadUInt32(nStrings);
        SAL_INFO("vcl.emf", "\tNumber of text strings: " << nStrings);

        for (sal_uInt32 i=0; i < nStrings; i++)
        {
            EmrText aText;
            aText.Read(rIStm, rIStm.Tell(), false);
            maStrings.push_back(aText);
        }
    }
};


class VCL_DLLPUBLIC EmfRectangleAction : public EmfAction
{
    RectL maBox;

protected:
    virtual     ~EmfRectangleAction() {}

public:
                EmfRectangleAction()
                : EmfAction( EMR_RECTANGLE_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBox:");
        maBox.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfRoundRectAction : public EmfAction
{
    RectL maBox;
    SizeL maCorner;

protected:
    virtual     ~EmfRoundRectAction() {}

public:
                EmfRoundRectAction()
                : EmfAction( EMR_ROUNDRECT_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBox:");
        maBox.Read(rIStm);

        SAL_INFO("vcl.emf", "\tCorner:");
        maCorner.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfSetPixelVAction : public EmfAction
{
    PointL maPixel;
    ColorRef maColor;

protected:
    virtual     ~EmfSetPixelVAction() {}

public:
                EmfSetPixelVAction()
                : EmfAction( EMR_SETPIXELV_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tPixel:");
        maPixel.Read(rIStm);

        SAL_INFO("vcl.emf", "\tColor:");
        maColor.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfSmallTextOutAction : public EmfAction
{
    PointL maStringPos;
    sal_uInt32 mnOptions;
    GraphicsMode meGraphicsMode;
    float mfXScale;
    float mfYScale;
    RectL maBounds;
    OUString maTextString;

protected:
    virtual     ~EmfSmallTextOutAction() {}

public:
                EmfSmallTextOutAction()
                : EmfAction( EMR_SMALLTEXTOUT_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tText position:");
        maStringPos.Read(rIStm);

        sal_uInt32 nStringLen=0;
        rIStm.ReadUInt32(nStringLen);
        SAL_INFO("vcl.emf", "String length (not NULL terminated): " << nStringLen);

        rIStm.ReadUInt32(mnOptions);
        SAL_INFO("vcl.emf", "\tText options:");
        decode_EmfTextOutOptions(mnOptions);

        SAL_INFO("vcl.emf", "\tGraphics mode:");
        meGraphicsMode = readGraphicsMode(rIStm);

        rIStm.ReadFloat(mfXScale);
        SAL_INFO("vcl.emf", "\tX scale: " << mfXScale);

        rIStm.ReadFloat(mfYScale);
        SAL_INFO("vcl.emf", "\tY scale: " << mfYScale);

        if (!(mnOptions & ETO_NO_RECT_EMF))
        {
            SAL_INFO("vcl.emf", "Bounds:");
            maBounds.Read(rIStm);
        }

        if (mnOptions & ETO_SMALL_CHARS_EMF)  // one-byte strings (actually UTF16-LE but just the first byte)
        {
            char sOutputString[nStringLen+1];
            rIStm.Read(sOutputString, nStringLen);
            sOutputString[nStringLen+1] = '\0';
            maTextString = OUString::createFromAscii(sOutputString);
            SAL_INFO("vcl.emf", "\tText string (\"small chars\"): " << maTextString);
        }
        else // UTF16-LE strings
        {
            // The next bit reads UTF16-LE so we need to ensure that we set to little-endian format
            rIStm.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
            maTextString = read_uInt16s_ToOUString( rIStm, nStringLen );
            SAL_INFO("vcl.emf", "\tText string (unicode): " << maTextString);
        }

    }
};


class VCL_DLLPUBLIC EmfStrokePathAction : public EmfAction
{
    RectL maBounds;

protected:
    virtual     ~EmfStrokePathAction() {}

public:
                EmfStrokePathAction()
                : EmfAction( EMR_STROKEPATH_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds:");
        maBounds.Read(rIStm);
    }
};


class VCL_DLLPUBLIC EmfStrokeAndFillPathAction : public EmfAction
{
    RectL maBounds;

protected:
    virtual     ~EmfStrokeAndFillPathAction() {}

public:
                EmfStrokeAndFillPathAction()
                : EmfAction( EMR_STROKEANDFILLPATH_ACTION ) {}

    virtual void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds:");
        maBounds.Read(rIStm);
    }
};

// Escape records

class VCL_DLLPUBLIC EmfEscapeAction : public EmfAction
{
protected:
    virtual     ~EmfEscapeAction() {}

public:
                EmfEscapeAction( sal_uInt16 nType )
                : EmfAction( nType ) { SAL_WARN("vcl.emf", "EMF escape record being read - NOT IMPLEMENTED"); }

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        sal_uInt64 nOriginalPos=0;

        nOriginalPos = rIStm.Tell();
        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.Seek(nOriginalPos + nSize - 4);
    }
};


class VCL_DLLPUBLIC EmfDrawEscapeAction : public EmfEscapeAction
{
public:
                EmfDrawEscapeAction()
                : EmfEscapeAction( EMR_DRAWESCAPE_ACTION ) {}
};


class VCL_DLLPUBLIC EmfExtEscapeAction : public EmfEscapeAction
{
public:
                EmfExtEscapeAction()
                : EmfEscapeAction( EMR_EXTESCAPE_ACTION ) {}
};


class VCL_DLLPUBLIC EmfNamedEscapeAction : public EmfEscapeAction
{
public:
                EmfNamedEscapeAction()
                : EmfEscapeAction( EMR_NAMEDESCAPE_ACTION ) {}
};

// OpenGL


class VCL_DLLPUBLIC EmfOpenGLAction : public EmfAction
{
protected:
    virtual     ~EmfOpenGLAction() {}

public:
                EmfOpenGLAction( sal_uInt16 nType )
                : EmfAction( nType ) { SAL_WARN("vcl.emf", "EMF OpenGL record being read - NOT IMPLEMENTED"); }

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        sal_uInt64 nOriginalPos=0;

        nOriginalPos = rIStm.Tell();
        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.Seek(nOriginalPos + nSize - 4);
    }

};


class VCL_DLLPUBLIC EmfGLSBoundedRecordAction : public EmfOpenGLAction
{
public:
                EmfGLSBoundedRecordAction()
                : EmfOpenGLAction( EMR_GLSBOUNDEDRECORD_ACTION ) {}
};


class VCL_DLLPUBLIC EmfGLSRecordAction : public EmfOpenGLAction
{
public:
                EmfGLSRecordAction()
                : EmfOpenGLAction( EMR_GLSRECORD_ACTION ) {}
};


// Path bracket


class VCL_DLLPUBLIC EmfPathBracketAction : public EmfAction
{
protected:
    virtual     ~EmfPathBracketAction() {}

public:
                EmfPathBracketAction( sal_uInt16 nType )
                : EmfAction( nType ) { }

    void Read( SvStream& rIStm )
    {
        sal_uInt32 nSize=0;
        sal_uInt64 nOriginalPos=0;

        nOriginalPos = rIStm.Tell();
        rIStm.ReadUInt32(nSize);

        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        rIStm.Seek(nOriginalPos + nSize - 4);
    }
};


class VCL_DLLPUBLIC EmfAbortPathAction : public EmfPathBracketAction
{
public:
                EmfAbortPathAction()
                : EmfPathBracketAction( EMR_ABORTPATH_ACTION ) {}
};


class VCL_DLLPUBLIC EmfBeginPathAction : public EmfPathBracketAction
{
public:
                EmfBeginPathAction()
                : EmfPathBracketAction( EMR_BEGINPATH_ACTION ) {}
};


class VCL_DLLPUBLIC EmfEndPathAction : public EmfPathBracketAction
{
public:
                EmfEndPathAction()
                : EmfPathBracketAction( EMR_ENDPATH_ACTION ) {}
};


class VCL_DLLPUBLIC EmfCloseFigureAction : public EmfPathBracketAction
{
public:
                EmfCloseFigureAction()
                : EmfPathBracketAction( EMR_CLOSEFIGURE_ACTION ) {}
};


class VCL_DLLPUBLIC EmfFlattenPathAction : public EmfPathBracketAction
{
public:
                EmfFlattenPathAction()
                : EmfPathBracketAction( EMR_FLATTENPATH_ACTION ) {}
};


class VCL_DLLPUBLIC EmfWidenPathAction : public EmfPathBracketAction
{
public:
                EmfWidenPathAction()
                : EmfPathBracketAction( EMR_WIDENPATH_ACTION ) {}
};


// Bitmap
/*
static sal_uInt32 decodeTernaryRasterOperation( sal_uInt32 nOp )
{
    // implementing a much better way of decoding TernaryRasterOperation values
    // thanks to the WINE project for the info - see http://wiki.winehq.org/TernaryRasterOps

    sal_uInt32 nLowWord = (nOp & 0x00FF);

    sal_uInt16 nOrder = (nLowWord >> 4);
    bool bEven = ((nLowWord & 0x0010) >> 4) ? true : false;
    sal_uInt16 nFirstBoolOp  = (nLowWord & 0x00C0) >> 0x5;
    sal_uInt16 nSecondBoolOp = (nLowWord & 0x0300) >> 0x8;
    sal_uInt32 nThirdBoolOp  = (nLowWord & 0x0C00) >> 0xa;
    sal_uInt32 nFourthBoolOp = (nLowWord & 0x3000) >> 0xc;
    sal_uInt32 nFifthBoolOp  = (nLowWord & 0xC000) >> 0xe;

    // TODO - implement this later
};
*/



class VCL_DLLPUBLIC EmfAlphaBlendAction : public EmfAction
{
    RectL       maBounds;
    PointL      maDest;
    sal_uInt32  mnWidth;
    sal_uInt32  mnHeight;
    BLENDFUNCTION maBlendFunction;
    PointL      maSrc;
    XFormEMF    maXformSrc;
    ColorRef    maBkColorSrc;
    DIBColors   meUsageSrc;
    Bitmap      maDIB;

protected:
    virtual     ~EmfAlphaBlendAction() {}

public:
                EmfAlphaBlendAction()
                : EmfAction( EMR_ALPHABLEND_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt64 nStartPos=(rIStm.Tell() - 4);

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding rectangle:");
        maBounds.Read(rIStm);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of destination rectangle:");
        maDest.Read(rIStm);

        rIStm.ReadUInt32(mnWidth);
        SAL_INFO("vcl.emf", "\tWidth: " << mnWidth);

        rIStm.ReadUInt32(mnHeight);
        SAL_INFO("vcl.emf", "\tHeight: " << mnHeight);

        SAL_INFO("vcl.emf", "\tBlend function:");
        maBlendFunction.Read(rIStm);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of source rectangle");
        maSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tXForm world-space to page-space transform:");
        maXformSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tBackground color of source bitmap");
        maBkColorSrc.Read(rIStm);

        sal_uInt32 nUsageSrc=0;
        rIStm.ReadUInt32(nUsageSrc);
        meUsageSrc = decodeDIBColors(nUsageSrc);

        sal_uInt32 nBitmapHeaderOffset=0, nBitmapHeaderSize=0, nBitmapOffset=0, nBitmapSize=0;
        rIStm.ReadUInt32(nBitmapHeaderOffset).ReadUInt32(nBitmapHeaderSize).ReadUInt32(nBitmapOffset).ReadUInt32(nBitmapSize);
        SAL_INFO("vcl.emf", "\tBitmap offsets:");
        SAL_INFO("vcl.emf", "\t\tHeader offset: " << nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\t\tHeader size:   " << nBitmapHeaderSize);
        SAL_INFO("vcl.emf", "\t\tBitmap offset: " << nBitmapOffset);
        SAL_INFO("vcl.emf", "\t\tBitmap size:   " << nBitmapSize);

        if (nBitmapHeaderOffset)
        {
            rIStm.Seek(nStartPos + nBitmapHeaderOffset);
            SAL_INFO("vcl.emf", "\tReading DIB");
            ReadDIB( maDIB, rIStm, false );
        }
        else
        {
            SAL_INFO("vcl.emf", "\tNo DIB to read");
            rIStm.Seek(nStartPos + nSize);
        }
    }
};


class VCL_DLLPUBLIC EmfMaskBltAction : public EmfAction
{
    RectL       maBounds;
    PointL      maDest;
    sal_uInt32  mnWidth;
    sal_uInt32  mnHeight;
    ROP4        maROP4;
    PointL      maSrc;
    XFormEMF    maXformSrc;
    ColorRef    maBkColorSrc;
    DIBColors   meUsageSrc;
    PointL      maMaskPoint;
    DIBColors   meUsageMask;
    Bitmap      maDIB;
    Bitmap      maDIBMask;

protected:
    virtual     ~EmfMaskBltAction() {}

public:
                EmfMaskBltAction()
                : EmfAction( EMR_MASKBLT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt64 nStartPos=(rIStm.Tell() - 4);

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding rectangle:");
        maBounds.Read(rIStm);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of destination rectangle:");
        maDest.Read(rIStm);

        rIStm.ReadUInt32(mnWidth);
        SAL_INFO("vcl.emf", "\tWidth: " << mnWidth);

        rIStm.ReadUInt32(mnHeight);
        SAL_INFO("vcl.emf", "\tHeight: " << mnHeight);

        SAL_INFO("vcl.emf", "\tROP4:");
        maROP4.Read(rIStm);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of source rectangle");
        maSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tXForm world-space to page-space transform:");
        maXformSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tBackground color of source bitmap");
        maBkColorSrc.Read(rIStm);

        sal_uInt32 nUsageSrc=0;
        rIStm.ReadUInt32(nUsageSrc);
        meUsageSrc = decodeDIBColors(nUsageSrc);

        sal_uInt32 nBitmapHeaderOffset=0, nBitmapHeaderSize=0, nBitmapOffset=0, nBitmapSize=0;
        rIStm.ReadUInt32(nBitmapHeaderOffset).ReadUInt32(nBitmapHeaderSize).ReadUInt32(nBitmapOffset).ReadUInt32(nBitmapSize);
        SAL_INFO("vcl.emf", "\tBitmap offsets:");
        SAL_INFO("vcl.emf", "\t\tHeader offset: " << nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\t\tHeader size:   " << nBitmapHeaderSize);
        SAL_INFO("vcl.emf", "\t\tBitmap offset: " << nBitmapOffset);
        SAL_INFO("vcl.emf", "\t\tBitmap size:   " << nBitmapSize);

        SAL_INFO("vcl.emf", "\tLogical upper left point of mask bitmap:");
        maMaskPoint.Read(rIStm);

        sal_uInt32 nMaskHeaderOffset=0, nMaskHeaderSize=0, nMaskOffset=0, nMaskSize=0;
        rIStm.ReadUInt32(nMaskHeaderOffset).ReadUInt32(nMaskHeaderSize).ReadUInt32(nMaskOffset).ReadUInt32(nMaskSize);

        rIStm.Seek(nStartPos + nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\tReading DIB");
        ReadDIB( maDIB, rIStm, false );

        rIStm.Seek(nStartPos + nMaskHeaderOffset);
        SAL_INFO("vcl.emf", "\tReading mask");
        ReadDIB( maDIBMask, rIStm, false );

        rIStm.Seek(nStartPos + nSize);
    }
};

class VCL_DLLPUBLIC EmfBitBltAction : public EmfAction
{
    RectL       maBounds;
    PointL      maDest;
    sal_uInt32  mnWidth;
    sal_uInt32  mnHeight;
    sal_uInt32  mnBitBltRasterOperation;
    PointL      maSrc;
    XFormEMF    maXformSrc;
    ColorRef    maBkColorSrc;
    DIBColors   meUsageSrc;
    Bitmap      maDIB;

protected:
    virtual     ~EmfBitBltAction() {}

public:
                EmfBitBltAction()
                : EmfAction( EMR_BITBLT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt64 nStartPos=(rIStm.Tell() - 4);

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding rectangle:");
        maBounds.Read(rIStm);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of destination rectangle:");
        maDest.Read(rIStm);

        rIStm.ReadUInt32(mnWidth);
        SAL_INFO("vcl.emf", "\tWidth: " << mnWidth);

        rIStm.ReadUInt32(mnHeight);
        SAL_INFO("vcl.emf", "\tHeight: " << mnHeight);

        rIStm.ReadUInt32(mnBitBltRasterOperation);
        SAL_INFO("vcl.emf", "\tRaster operation: 0x" << std::hex << mnBitBltRasterOperation << std::dec);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of source rectangle");
        maSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tXForm world-space to page-space transform:");
        maXformSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tBackground color of source bitmap");
        maBkColorSrc.Read(rIStm);

        sal_uInt32 nUsageSrc=0;
        rIStm.ReadUInt32(nUsageSrc);
        meUsageSrc = decodeDIBColors(nUsageSrc);

        sal_uInt32 nBitmapHeaderOffset=0, nBitmapHeaderSize=0, nBitmapOffset=0, nBitmapSize=0;
        rIStm.ReadUInt32(nBitmapHeaderOffset).ReadUInt32(nBitmapHeaderSize).ReadUInt32(nBitmapOffset).ReadUInt32(nBitmapSize);
        SAL_INFO("vcl.emf", "\tBitmap offsets:");
        SAL_INFO("vcl.emf", "\t\tHeader offset: " << nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\t\tHeader size:   " << nBitmapHeaderSize);
        SAL_INFO("vcl.emf", "\t\tBitmap offset: " << nBitmapOffset);
        SAL_INFO("vcl.emf", "\t\tBitmap size:   " << nBitmapSize);

        if (nBitmapHeaderOffset)
        {
            rIStm.Seek(nStartPos + nBitmapHeaderOffset);
            SAL_INFO("vcl.emf", "\tReading DIB");
            ReadDIB( maDIB, rIStm, false );
            rIStm.Seek(nStartPos + nSize);
        }
        else
        {
            SAL_INFO("vcl.emf", "\tNo DIB to read");
            rIStm.Seek(nStartPos + nSize);
        }
    }
};


class VCL_DLLPUBLIC EmfStretchBltAction : public EmfAction
{
    RectL       maBounds;
    PointL      maDest;
    sal_uInt32  mnDestWidth;
    sal_uInt32  mnDestHeight;
    sal_uInt32  mnBitBltRasterOperation;
    PointL      maSrc;
    XFormEMF    maXformSrc;
    ColorRef    maBkColorSrc;
    DIBColors   meUsageSrc;
    sal_uInt32  mnSourceWidth;
    sal_uInt32  mnSourceHeight;
    Bitmap      maDIB;

protected:
    virtual     ~EmfStretchBltAction() {}

public:
                EmfStretchBltAction()
                : EmfAction( EMR_STRETCHBLT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt64 nStartPos=(rIStm.Tell() - 4);

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding rectangle:");
        maBounds.Read(rIStm);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of destination rectangle:");
        maDest.Read(rIStm);

        rIStm.ReadUInt32(mnDestWidth);
        SAL_INFO("vcl.emf", "\tWidth: " << mnDestWidth);

        rIStm.ReadUInt32(mnDestHeight);
        SAL_INFO("vcl.emf", "\tHeight: " << mnDestHeight);

        rIStm.ReadUInt32(mnBitBltRasterOperation);
        SAL_INFO("vcl.emf", "\tRaster operation: 0x" << std::hex << mnBitBltRasterOperation << std::dec);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of source rectangle");
        maSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tXForm world-space to page-space transform:");
        maXformSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tBackground color of source bitmap");
        maBkColorSrc.Read(rIStm);

        sal_uInt32 nUsageSrc=0;
        rIStm.ReadUInt32(nUsageSrc);
        meUsageSrc = decodeDIBColors(nUsageSrc);

        sal_uInt32 nBitmapHeaderOffset=0, nBitmapHeaderSize=0, nBitmapOffset=0, nBitmapSize=0;
        rIStm.ReadUInt32(nBitmapHeaderOffset).ReadUInt32(nBitmapHeaderSize).ReadUInt32(nBitmapOffset).ReadUInt32(nBitmapSize);
        SAL_INFO("vcl.emf", "\tBitmap offsets:");
        SAL_INFO("vcl.emf", "\t\tHeader offset: " << nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\t\tHeader size:   " << nBitmapHeaderSize);
        SAL_INFO("vcl.emf", "\t\tBitmap offset: " << nBitmapOffset);
        SAL_INFO("vcl.emf", "\t\tBitmap size:   " << nBitmapSize);

        rIStm.ReadUInt32(mnSourceWidth);
        SAL_INFO("vcl.emf", "\tSource width: " << mnSourceWidth);

        rIStm.ReadUInt32(mnSourceHeight);
        SAL_INFO("vcl.emf", "\tSource height: " << mnSourceHeight);

        if (nBitmapHeaderOffset)
        {
            rIStm.Seek(nStartPos + nBitmapHeaderOffset);
            SAL_INFO("vcl.emf", "\tReading DIB");
            ReadDIB( maDIB, rIStm, false );
            rIStm.Seek(nStartPos + nSize);
        }
        else
        {
            SAL_INFO("vcl.emf", "\tNo DIB to read");
            rIStm.Seek(nStartPos + nSize);
        }
    }

};


class VCL_DLLPUBLIC EmfPlgBltAction : public EmfAction
{
    RectL       maBounds;
    PointL      maPointA;
    PointL      maPointB;
    PointL      maPointC;
    PointL      maSrcPoint;
    sal_uInt32  mnSrcWidth;
    sal_uInt32  mnSrcHeight;
    XFormEMF    maXformSrc;
    ColorRef    maBkColorSrc;
    DIBColors   meUsageSrc;
    PointL      maMaskPoint;
    DIBColors   meUsageMask;
    Bitmap      maSourceBitmap;
    Bitmap      maMask;

protected:
    virtual     ~EmfPlgBltAction() {}

public:
                EmfPlgBltAction()
                : EmfAction( EMR_PLGBLT_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt64 nStartPos = (rIStm.Tell() - 4);

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounds:");
        maBounds.Read(rIStm);

        SAL_INFO("vcl.emf", "\tParellogram point A:");
        maPointA.Read(rIStm);

        SAL_INFO("vcl.emf", "\tParellogram point B:");
        maPointB.Read(rIStm);

        SAL_INFO("vcl.emf", "\tParellogram point C");
        maPointC.Read(rIStm);

        SAL_INFO("vcl.emf", "\tSource point:");
        maSrcPoint.Read(rIStm);

        rIStm.ReadUInt32(mnSrcWidth);
        rIStm.ReadUInt32(mnSrcHeight);
        SAL_INFO("vcl.emf", "\tSource width: " << mnSrcWidth);
        SAL_INFO("vcl.emf", "\tSource height: " << mnSrcHeight);

        SAL_INFO("vcl.emf", "\tSource XForm:");
        maXformSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tSource color:");
        maBkColorSrc.Read(rIStm);

        sal_uInt32 nUsageSrc=0;
        rIStm.ReadUInt32(nUsageSrc);
        meUsageSrc = decodeDIBColors(nUsageSrc);

        sal_uInt32 nBitmapHeaderOffset=0, nBitmapHeaderSize=0, nBitmapOffset=0, nBitmapSize=0;
        rIStm.ReadUInt32(nBitmapHeaderOffset).ReadUInt32(nBitmapHeaderSize).ReadUInt32(nBitmapOffset).ReadUInt32(nBitmapSize);
        SAL_INFO("vcl.emf", "\tBitmap offsets:");
        SAL_INFO("vcl.emf", "\t\tHeader offset: " << nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\t\tHeader size:   " << nBitmapHeaderSize);
        SAL_INFO("vcl.emf", "\t\tBitmap offset: " << nBitmapOffset);
        SAL_INFO("vcl.emf", "\t\tBitmap size:   " << nBitmapSize);

        SAL_INFO("vcl.emf", "\tLogical upper left point of mask bitmap:");
        maMaskPoint.Read(rIStm);

        sal_uInt32 nMaskHeaderOffset=0, nMaskHeaderSize=0, nMaskOffset=0, nMaskSize=0;
        rIStm.ReadUInt32(nMaskHeaderOffset).ReadUInt32(nMaskHeaderSize).ReadUInt32(nMaskOffset).ReadUInt32(nMaskSize);

        rIStm.Seek(nStartPos + nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\tReading DIB");
        ReadDIB( maSourceBitmap, rIStm, false );

        rIStm.Seek(nStartPos + nMaskHeaderOffset);
        SAL_INFO("vcl.emf", "\tReading mask");
        ReadDIB( maMask, rIStm, false );

        rIStm.Seek(nStartPos + nSize);
    }
};


class VCL_DLLPUBLIC EmfSetDIBitsToDeviceAction : public EmfAction
{
    RectL       maBounds;
    PointL      maDestPoint;
    PointL      maSrcPoint;
    sal_uInt32  mnSrcWidth;
    sal_uInt32  mnSrcHeight;
    sal_uInt32  mnStartScan;
    sal_uInt32  mnNumScanlines;
    Bitmap      maDIB;

protected:
    virtual     ~EmfSetDIBitsToDeviceAction() {}

public:
                EmfSetDIBitsToDeviceAction()
                : EmfAction( EMR_SETDIBITSTODEVICE_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt64 nStartPos=(rIStm.Tell() - 4);

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding rectangle:");
        maBounds.Read(rIStm);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of destination rectangle:");
        maDestPoint.Read(rIStm);

        rIStm.ReadUInt32(mnSrcWidth);
        SAL_INFO("vcl.emf", "\tWidth: " << mnSrcWidth);

        rIStm.ReadUInt32(mnSrcHeight);
        SAL_INFO("vcl.emf", "\tHeight: " << mnSrcHeight);

        SAL_INFO("vcl.emf", "\tLogical upper-left point of source rectangle");
        maSrcPoint.Read(rIStm);

        sal_uInt32 nBitmapHeaderOffset=0, nBitmapHeaderSize=0, nBitmapOffset=0, nBitmapSize=0;
        rIStm.ReadUInt32(nBitmapHeaderOffset).ReadUInt32(nBitmapHeaderSize).ReadUInt32(nBitmapOffset).ReadUInt32(nBitmapSize);
        SAL_INFO("vcl.emf", "\tBitmap offsets:");
        SAL_INFO("vcl.emf", "\t\tHeader offset: " << nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\t\tHeader size:   " << nBitmapHeaderSize);
        SAL_INFO("vcl.emf", "\t\tBitmap offset: " << nBitmapOffset);
        SAL_INFO("vcl.emf", "\t\tBitmap size:   " << nBitmapSize);

        rIStm.ReadUInt32(mnStartScan);
        SAL_INFO("vcl.emf", "\tStart scanline: " << mnStartScan);

        rIStm.ReadUInt32(mnNumScanlines);
        SAL_INFO("vcl.emf", "\tNumber of scanlines: " << mnNumScanlines);

        if (nBitmapHeaderOffset)
        {
            rIStm.Seek(nStartPos + nBitmapHeaderOffset);
            SAL_INFO("vcl.emf", "\tReading DIB");
            ReadDIB( maDIB, rIStm, false );
            rIStm.Seek(nStartPos + nSize);
        }
        else
        {
            SAL_INFO("vcl.emf", "\tNo DIB to read");
            rIStm.Seek(nStartPos + nSize);
        }
    }
};


class VCL_DLLPUBLIC EmfStretchDIBitsAction : public EmfAction
{
    RectL       maBounds;
    PointL      maDestPoint;
    PointL      maSrcPoint;
    sal_uInt32  mnSrcWidth;
    sal_uInt32  mnSrcHeight;
    DIBColors   meUsageSrc;
    sal_uInt32  mnBitBltRasterOperation;
    sal_uInt32  mnDestWidth;
    sal_uInt32  mnDestHeight;
    Bitmap      maDIB;

protected:
    virtual     ~EmfStretchDIBitsAction() {}

public:
                EmfStretchDIBitsAction()
                : EmfAction( EMR_STRETCHDIBITS_ACTION ) {}

    void Read( SvStream& rIStm )
    {
        sal_uInt64 nStartPos=(rIStm.Tell() - 4);

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding rectangle:");
        maBounds.Read(rIStm);

        SAL_INFO("vcl.emf", "\tDestination point:");
        maDestPoint.Read(rIStm);

        SAL_INFO("vcl.emf", "\tSource point:");
        maSrcPoint.Read(rIStm);

        rIStm.ReadUInt32(mnSrcWidth);
        rIStm.ReadUInt32(mnSrcHeight);
        SAL_INFO("vcl.emf", "\tSource width: " << mnSrcWidth);
        SAL_INFO("vcl.emf", "\tSource height: " << mnSrcHeight);

        sal_uInt32 nUsageSrc=0;
        rIStm.ReadUInt32(nUsageSrc);
        meUsageSrc = decodeDIBColors(nUsageSrc);

        rIStm.ReadUInt32(mnDestWidth);
        rIStm.ReadUInt32(mnDestHeight);
        SAL_INFO("vcl.emf", "\tDest width: " << mnDestWidth);
        SAL_INFO("vcl.emf", "\tDest height: " << mnDestHeight);

        sal_uInt32 nBitmapHeaderOffset=0, nBitmapHeaderSize=0, nBitmapOffset=0, nBitmapSize=0;
        rIStm.ReadUInt32(nBitmapHeaderOffset).ReadUInt32(nBitmapHeaderSize).ReadUInt32(nBitmapOffset).ReadUInt32(nBitmapSize);
        SAL_INFO("vcl.emf", "\tBitmap offsets:");
        SAL_INFO("vcl.emf", "\t\tHeader offset: " << nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\t\tHeader size:   " << nBitmapHeaderSize);
        SAL_INFO("vcl.emf", "\t\tBitmap offset: " << nBitmapOffset);
        SAL_INFO("vcl.emf", "\t\tBitmap size:   " << nBitmapSize);

        if (nBitmapHeaderOffset)
        {
            rIStm.Seek(nStartPos + nBitmapHeaderOffset);
            SAL_INFO("vcl.emf", "\tReading DIB");
            ReadDIB( maDIB, rIStm, false );
            rIStm.Seek(nStartPos + nSize);
        }
        else
        {
            SAL_INFO("vcl.emf", "\tNo DIB to read");
            rIStm.Seek(nStartPos + nSize);
        }
    }
};


class VCL_DLLPUBLIC EmfTransparentBltAction : public EmfAction
{
    RectL       maBounds;
    PointL      maDestPoint;
    sal_uInt32  mnDestWidth;
    sal_uInt32  mnDestHeight;
    ColorRef    maTransparentColor;
    PointL      maSrcPoint;
    XFormEMF    maXformSrc;
    ColorRef    maBkColorSrc;
    DIBColors   meUsageSrc;
    sal_uInt32  mnSrcWidth;
    sal_uInt32  mnSrcHeight;
    Bitmap      maDIB;

protected:
    virtual     ~EmfTransparentBltAction() {}

public:
                EmfTransparentBltAction()
                : EmfAction( EMR_TRANSPARENTBLT_ACTION ) {}

    void Read( SvStream& rIStm )
    {

        sal_uInt64 nStartPos=(rIStm.Tell() - 4);

        sal_uInt32 nSize=0;
        rIStm.ReadUInt32(nSize);
        SAL_INFO("vcl.emf", "\tSize: " << nSize);

        SAL_INFO("vcl.emf", "\tBounding rectangle:");
        maBounds.Read(rIStm);

        SAL_INFO("vcl.emf", "\tDestination point:");
        maDestPoint.Read(rIStm);

        rIStm.ReadUInt32(mnDestWidth);
        rIStm.ReadUInt32(mnDestHeight);
        SAL_INFO("vcl.emf", "\tDestination width: " << mnDestWidth);
        SAL_INFO("vcl.emf", "\tDestination height: " << mnDestHeight);

        SAL_INFO("vcl.emf", "\tTransparent color:");
        maTransparentColor.Read(rIStm);

        SAL_INFO("vcl.emf", "\tSource point:");
        maSrcPoint.Read(rIStm);

        SAL_INFO("vcl.emf", "\tSource transform:");
        maXformSrc.Read(rIStm);

        SAL_INFO("vcl.emf", "\tBackground color:");
        maBkColorSrc.Read(rIStm);

        sal_uInt32 nUsageSrc=0;
        rIStm.ReadUInt32(nUsageSrc);
        meUsageSrc = decodeDIBColors(nUsageSrc);

        sal_uInt32 nBitmapHeaderOffset=0, nBitmapHeaderSize=0, nBitmapOffset=0, nBitmapSize=0;
        rIStm.ReadUInt32(nBitmapHeaderOffset).ReadUInt32(nBitmapHeaderSize).ReadUInt32(nBitmapOffset).ReadUInt32(nBitmapSize);
        SAL_INFO("vcl.emf", "\tBitmap offsets:");
        SAL_INFO("vcl.emf", "\t\tHeader offset: " << nBitmapHeaderOffset);
        SAL_INFO("vcl.emf", "\t\tHeader size:   " << nBitmapHeaderSize);
        SAL_INFO("vcl.emf", "\t\tBitmap offset: " << nBitmapOffset);
        SAL_INFO("vcl.emf", "\t\tBitmap size:   " << nBitmapSize);

        rIStm.ReadUInt32(mnSrcWidth);
        rIStm.ReadUInt32(mnSrcHeight);
        SAL_INFO("vcl.emf", "\tSource width: " << mnSrcWidth);
        SAL_INFO("vcl.emf", "\tSource height: " << mnSrcHeight);

        if (nBitmapHeaderOffset)
        {
            rIStm.Seek(nStartPos + nBitmapHeaderOffset);
            SAL_INFO("vcl.emf", "\tReading DIB");
            ReadDIB( maDIB, rIStm, false );
            rIStm.Seek(nStartPos + nSize);
        }
        else
        {
            SAL_INFO("vcl.emf", "\tNo DIB to read");
            rIStm.Seek(nStartPos + nSize);
        }
    }
};

void EmfAction::Read( SvStream& rIStm )
{
    SAL_INFO("vcl.emf", "Unknown record");

    sal_uInt64 nOrigPos = 0;
    nOrigPos = rIStm.Tell();

    sal_uInt32 nSize = 0;
    rIStm.ReadUInt32( nSize );
    rIStm.Seek( nOrigPos + nSize - 4 );
}

EmfAction* EmfAction::ReadEmfAction( SvStream& rIStm )
{
    EmfAction* pAction = NULL;
    sal_uInt32 nType = 0;

    rIStm.ReadUInt32( nType );

    SAL_INFO("vcl.emf", "ReadEmfAction - type = 0x" << std::hex << nType << std::dec << " " << emf_action_name(nType) );

    switch( nType )
    {
        // control record types
        case EMR_HEADER_ACTION:                 pAction = new EmfHeaderAction; break;
        case EMR_EOF_ACTION:                    pAction = new EmfEOFAction; break;

        // comment record type
        case EMR_COMMENT_ACTION:                pAction = new EmfCommentAction; break;

        // Bitmap record types
        case EMR_BITBLT_ACTION:                 pAction = new EmfBitBltAction; break;
        case EMR_STRETCHBLT_ACTION:             pAction = new EmfStretchBltAction; break;
        case EMR_MASKBLT_ACTION:                pAction = new EmfMaskBltAction; break;
        case EMR_SETDIBITSTODEVICE_ACTION:      pAction = new EmfSetDIBitsToDeviceAction; break;
        case EMR_STRETCHDIBITS_ACTION:          pAction = new EmfStretchDIBitsAction; break;
        case EMR_ALPHABLEND_ACTION:             pAction = new EmfAlphaBlendAction; break;
        case EMR_TRANSPARENTBLT_ACTION:         pAction = new EmfTransparentBltAction; break;

        // state record types
        case EMR_SAVEDC_ACTION:                 pAction = new EmfSaveDCAction; break;
        case EMR_RESTOREDC_ACTION:              pAction = new EmfRestoreDCAction; break;
        case EMR_SETLAYOUT_ACTION:              pAction = new EmfSetLayoutAction; break;
        case EMR_COLORMATCHTOTARGETW_ACTION:    pAction = new EmfColorMatchToTargetWAction; break;
        case EMR_FORCEUFIMAPPING_ACTION:        pAction = new EmfForceUFIMappingAction; break;
        case EMR_INVERTRGN_ACTION:              pAction = new EmfInvertRegionAction; break;
        case EMR_MOVETOEX_ACTION:               pAction = new EmfMoveToExAction; break;
        case EMR_PIXELFORMAT_ACTION:            pAction = new EmfPixelFormatAction; break;
        case EMR_SCALEVIEWPORTEXTEX_ACTION:     pAction = new EmfScaleViewportExtExAction; break;
        case EMR_SCALEWINDOWEXTEX_ACTION:       pAction = new EmfScaleWindowExtExAction; break;
        case EMR_SETARCDIRECTION_ACTION:        pAction = new EmfSetArcDirectionAction; break;
        case EMR_SETBKCOLOR_ACTION:             pAction = new EmfSetBKColorAction; break;
        case EMR_SETBKMODE_ACTION:              pAction = new EmfSetBKModeAction; break;
        case EMR_SETBRUSHORGEX_ACTION:          pAction = new EmfSetBrushOrgExAction; break;
        case EMR_SETCOLORADJUSTMENT_ACTION:     pAction = new EmfSetColorAdjustmentAction; break;
        case EMR_SETICMMODE_ACTION:             pAction = new EmfSetICMModeAction; break;
        case EMR_SETICMPROFILEA_ACTION:         pAction = new EmfSetICMProfileAAction; break;
        case EMR_SETICMPROFILEW_ACTION:         pAction = new EmfSetICMProfileWAction; break;
        case EMR_SETLINKEDUFIS_ACTION:          pAction = new EmfSetLinkedUFIsAction; break;
        case EMR_SETMAPMODE_ACTION:             pAction = new EmfSetMapModeAction; break;
        case EMR_SETMAPPERFLAGS_ACTION:         pAction = new EmfSetMapperFlagsAction; break;
        case EMR_SETMITERLIMIT_ACTION:          pAction = new EmfSetMiterLimitAction; break;
        case EMR_SETPOLYFILLMODE_ACTION:        pAction = new EmfSetPolyFillModeAction; break;
        case EMR_SETROP2_ACTION:                pAction = new EmfSetROP2Action; break;
        case EMR_SETSTRETCHBLTMODE_ACTION:      pAction = new EmfStretchBLTModeAction; break;
        case EMR_SETTEXTALIGN_ACTION:           pAction = new EmfSetTextAlignAction; break;
        case EMR_SETTEXTCOLOR_ACTION:           pAction = new EmfSetTextColorAction; break;
        case EMR_SETTEXTJUSTIFICATION_ACTION:   pAction = new EmfSetTextJustificationAction; break;
        case EMR_SETVIEWPORTEXTEX_ACTION:       pAction = new EmfSetViewportExtExAction; break;
        case EMR_SETVIEWPORTORGEX_ACTION:       pAction = new EmfSetViewportOrgExAction; break;
        case EMR_SETWINDOWEXTEX_ACTION:         pAction = new EmfSetWindowExtExAction; break;
        case EMR_SETWINDOWORGEX_ACTION:         pAction = new EmfSetWindowOrgExAction; break;

        // transform record types
        case EMR_MODIFYWORLDTRANSFORM_ACTION:   pAction = new EmfModifyWorldTransformAction; break;
        case EMR_SETWORLDTRANSFORM_ACTION:      pAction = new EmfSetWorldTransformAction; break;

        // object creation record types
        case EMR_CREATEBRUSHINDIRECT_ACTION:    pAction = new EmfCreateBrushIndirectAction; break;
        case EMR_CREATECOLORSPACE_ACTION:       pAction = new EmfCreateColorSpaceAction; break;
        case EMR_CREATECOLORSPACEW_ACTION:      pAction = new EmfCreateColorSpaceWAction; break;
        case EMR_EXTCREATEFONTINDIRECTW_ACTION: pAction = new EmfExtCreateFontIndirectWAction; break;
        case EMR_CREATEDIBPATTERNBRUSHPT_ACTION: pAction = new EmfCreateDIBPatternBrushAction; break;
        case EMR_CREATEPALETTE_ACTION:          pAction = new EmfCreatePaletteAction; break;
        case EMR_CREATEPEN_ACTION:              pAction = new EmfCreatePenAction; break;
        case EMR_EXTCREATEPEN_ACTION:           pAction = new EmfExtCreatePenAction; break;

        // object manipulation record types
        case EMR_COLORCORRECTPALETTE_ACTION:    pAction = new EmfColorCorrectPaletteAction; break;
        case EMR_DELETECOLORSPACE_ACTION:       pAction = new EmfDeleteColorSpaceAction; break;
        case EMR_DELETEOBJECT_ACTION:           pAction = new EmfDeleteObjectAction; break;
        case EMR_RESIZEPALETTE_ACTION:          pAction = new EmfResizePaletteAction; break;
        case EMR_SELECTOBJECT_ACTION:           pAction = new EmfSelectObjectAction; break;
        case EMR_SELECTPALETTE_ACTION:          pAction = new EmfSelectPaletteAction; break;
        case EMR_SETCOLORSPACE_ACTION:          pAction = new EmfSetColorSpaceAction; break;
        case EMR_SETPALETTEENTRIES_ACTION:      pAction = new EmfSetPaletteEntriesAction; break;

        // clipping record types
        case EMR_OFFSETCLIPRGN_ACTION:          pAction = new EmfOffsetClipRegionAction; break;
        case EMR_SETMETARGN_ACTION:             pAction = new EmfSetMetaRegionAction; break;
        case EMR_EXCLUDECLIPRECT_ACTION:        pAction = new EmfExcludeClipRectAction; break;
        case EMR_INTERSECTCLIPRECT_ACTION:      pAction = new EmfIntersectClipRectAction; break;
        case EMR_SELECTCLIPPATH_ACTION:         pAction = new EmfSelectClipPathAction; break;
        case EMR_EXTSELECTCLIPRGN_ACTION:       pAction = new EmfExtSelectClipRegionAction; break;

        // drawing record types
        case EMR_ANGLEARC_ACTION:               pAction = new EmfAngleArcAction; break;
        case EMR_ARC_ACTION:                    pAction = new EmfArcAction; break;
        case EMR_ARCTO_ACTION:                  pAction = new EmfArcToAction; break;
        case EMR_CHORD_ACTION:                  pAction = new EmfChordAction; break;
        case EMR_EXTFLOODFILL_ACTION:           pAction = new EmfExtFloodFillAction; break;
        case EMR_EXTTEXTOUTA_ACTION:            pAction = new EmfExtTextOutAAction; break;
        case EMR_EXTTEXTOUTW_ACTION:            pAction = new EmfExtTextOutWAction; break;
        case EMR_FILLPATH_ACTION:               pAction = new EmfFillPathAction; break;
        case EMR_FILLRGN_ACTION:                pAction = new EmfFillRegionAction; break;
        case EMR_FRAMERGN_ACTION:               pAction = new EmfFrameRegionAction; break;
        case EMR_GRADIENTFILL_ACTION:           pAction = new EmfGradientFillAction; break;
        case EMR_LINETO_ACTION:                 pAction = new EmfLineToAction; break;
        case EMR_ELLIPSE_ACTION:                pAction = new EmfEllipseAction; break;
        case EMR_PAINTRGN_ACTION:               pAction = new EmfPaintRegionAction; break;
        case EMR_PIE_ACTION:                    pAction = new EmfPieAction; break;
        case EMR_POLYBEZIER_ACTION:             pAction = new EmfPolyBezierAction; break;
        case EMR_POLYBEZIER16_ACTION:           pAction = new EmfPolyBezier16Action; break;
        case EMR_POLYBEZIERTO_ACTION:           pAction = new EmfPolyBezierAction; break;
        case EMR_POLYBEZIERTO16_ACTION:         pAction = new EmfPolyBezier16Action; break;
        case EMR_POLYDRAW_ACTION:               pAction = new EmfPolyDrawAction; break;
        case EMR_POLYDRAW16_ACTION:             pAction = new EmfPolyDraw16Action; break;
        case EMR_POLYGON_ACTION:                pAction = new EmfPolygonAction; break;
        case EMR_POLYGON16_ACTION:              pAction = new EmfPolygon16Action; break;
        case EMR_POLYLINE_ACTION:               pAction = new EmfPolylineAction; break;
        case EMR_POLYLINE16_ACTION:             pAction = new EmfPolyline16Action; break;
        case EMR_POLYLINETO_ACTION:             pAction = new EmfPolylineAction; break;
        case EMR_POLYLINETO16_ACTION:           pAction = new EmfPolyline16Action; break;
        case EMR_POLYPOLYGON_ACTION:            pAction = new EmfPolyPolygonAction; break;
        case EMR_POLYPOLYGON16_ACTION:          pAction = new EmfPolyPolygon16Action; break;
        case EMR_POLYPOLYLINE_ACTION:           pAction = new EmfPolyPolylineAction; break;
        case EMR_POLYPOLYLINE16_ACTION:         pAction = new EmfPolyPolyline16Action; break;
        case EMR_POLYTEXTOUTA_ACTION:           pAction = new EmfPolyTextOutAAction; break;
        case EMR_POLYTEXTOUTW_ACTION:           pAction = new EmfPolyTextOutWAction; break;
        case EMR_RECTANGLE_ACTION:              pAction = new EmfRectangleAction; break;
        case EMR_SETPIXELV_ACTION:              pAction = new EmfSetPixelVAction; break;
        case EMR_SMALLTEXTOUT_ACTION:           pAction = new EmfSmallTextOutAction; break;
        case EMR_STROKEANDFILLPATH_ACTION:      pAction = new EmfStrokeAndFillPathAction; break;
        case EMR_STROKEPATH_ACTION:             pAction = new EmfStrokePathAction; break;


        case EMR_DRAWESCAPE_ACTION:             pAction = new EmfDrawEscapeAction; break;
        case EMR_EXTESCAPE_ACTION:              pAction = new EmfExtEscapeAction; break;
        case EMR_NAMEDESCAPE_ACTION:            pAction = new EmfNamedEscapeAction; break;

        // OpenGL action types
        case EMR_GLSRECORD_ACTION:              pAction = new EmfGLSRecordAction; break;
        case EMR_GLSBOUNDEDRECORD_ACTION:       pAction = new EmfGLSBoundedRecordAction; break;

        // Path action types
        case EMR_BEGINPATH_ACTION:              pAction = new EmfBeginPathAction; break;
        case EMR_ENDPATH_ACTION:                pAction = new EmfEndPathAction; break;
        case EMR_CLOSEFIGURE_ACTION:            pAction = new EmfCloseFigureAction; break;
        case EMR_FLATTENPATH_ACTION:            pAction = new EmfFlattenPathAction; break;
        case EMR_WIDENPATH_ACTION:              pAction = new EmfWidenPathAction; break;
        case EMR_ABORTPATH_ACTION:              pAction = new EmfAbortPathAction; break;

        // can never be!
        case 0x0000:
            std::exit(1);

        default:
        {
            pAction = new EmfAction();
            std::exit(0);
        }
        break;
    }

    if( pAction )
        pAction->Read( rIStm );

    return pAction;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
