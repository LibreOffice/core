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

#ifndef INCLUDED_VCL_EMFACT_HXX
#define INCLUDED_VCL_EMFACT_HXX

#include <vcl/metaact.hxx>
#include <tools/stream.hxx>

/// Control record types
#define EMR_HEADER_ACTION                   0x0001
#define EMR_EOF_ACTION                      0x000E

/// Comment record type
#define EMR_COMMENT_ACTION                  0x0046

/// State record types
#define EMR_SETWINDOWEXTEX_ACTION           0x0009
#define EMR_SETWINDOWORGEX_ACTION           0x000A
#define EMR_SETVIEWPORTEXTEX_ACTION         0x000B
#define EMR_SETVIEWPORTORGEX_ACTION         0x000C
#define EMR_SETBRUSHORGEX_ACTION            0x000D
#define EMR_SETMAPPERFLAGS_ACTION           0x0010
#define EMR_SETMAPMODE_ACTION               0x0011
#define EMR_SETBKMODE_ACTION                0x0012
#define EMR_SETPOLYFILLMODE_ACTION          0x0013
#define EMR_SETROP2_ACTION                  0x0014
#define EMR_SETSTRETCHBLTMODE_ACTION        0x0015
#define EMR_SETTEXTALIGN_ACTION             0x0016
#define EMR_SETCOLORADJUSTMENT_ACTION       0x0017
#define EMR_SETTEXTCOLOR_ACTION             0x0018
#define EMR_SETBKCOLOR_ACTION               0x0019
#define EMR_MOVETOEX_ACTION                 0x001B
#define EMR_SCALEVIEWPORTEXTEX_ACTION       0x001F
#define EMR_SCALEWINDOWEXTEX_ACTION         0x0020
#define EMR_SAVEDC_ACTION                   0x0021
#define EMR_RESTOREDC_ACTION                0x0022
#define EMR_SETMITERLIMIT_ACTION            0x003A
#define EMR_SETARCDIRECTION_ACTION          0x0039
#define EMR_INVERTRGN_ACTION                0x0049
#define EMR_SETICMMODE_ACTION               0x0062
#define EMR_PIXELFORMAT_ACTION              0x0068
#define EMR_FORCEUFIMAPPING_ACTION          0x006D
#define EMR_SETICMPROFILEA_ACTION           0x0070
#define EMR_SETICMPROFILEW_ACTION           0x0071
#define EMR_SETLAYOUT_ACTION                0x0073
#define EMR_SETLINKEDUFIS_ACTION            0x0077
#define EMR_SETTEXTJUSTIFICATION_ACTION     0x0078
#define EMR_COLORMATCHTOTARGETW_ACTION      0x0079

/// Transformation record types
#define EMR_SETWORLDTRANSFORM_ACTION        0x0023
#define EMR_MODIFYWORLDTRANSFORM_ACTION     0x0024

/// Object creation record types
#define EMR_CREATEPEN_ACTION                0x0026
#define EMR_CREATEBRUSHINDIRECT_ACTION      0x0027
#define EMR_CREATEPALETTE_ACTION            0x0031
#define EMR_EXTCREATEFONTINDIRECTW_ACTION   0x0052
#define EMR_CREATEMONOBRUSH_ACTION          0x005D
#define EMR_CREATEDIBPATTERNBRUSHPT_ACTION  0x005E
#define EMR_EXTCREATEPEN_ACTION             0x005F
#define EMR_CREATECOLORSPACE_ACTION         0x0063
#define EMR_CREATECOLORSPACEW_ACTION        0x007A

/// Object manipulation record types
#define EMR_SELECTOBJECT_ACTION             0x0025
#define EMR_DELETEOBJECT_ACTION             0x0028
#define EMR_SELECTPALETTE_ACTION            0x0030
#define EMR_SETPALETTEENTRIES_ACTION        0x0032
#define EMR_RESIZEPALETTE_ACTION            0x0033
#define EMR_SETCOLORSPACE_ACTION            0x0064
#define EMR_DELETECOLORSPACE_ACTION         0x0065
#define EMR_COLORCORRECTPALETTE_ACTION      0x006F

/// Clipping record types
#define EMR_OFFSETCLIPRGN_ACTION            0x001A
#define EMR_SETMETARGN_ACTION               0x001C
#define EMR_EXCLUDECLIPRECT_ACTION          0x001D
#define EMR_INTERSECTCLIPRECT_ACTION        0x001E
#define EMR_SELECTCLIPPATH_ACTION           0x0043
#define EMR_EXTSELECTCLIPRGN_ACTION         0x004B

/// Drawing record types
#define EMR_POLYBEZIER_ACTION               0x0002
#define EMR_POLYGON_ACTION                  0x0003
#define EMR_POLYLINE_ACTION                 0x0004
#define EMR_POLYBEZIERTO_ACTION             0x0005
#define EMR_POLYLINETO_ACTION               0x0006
#define EMR_POLYPOLYLINE_ACTION             0x0007
#define EMR_POLYPOLYGON_ACTION              0x0008
#define EMR_SETPIXELV_ACTION                0x000F
#define EMR_ANGLEARC_ACTION                 0x0029
#define EMR_ELLIPSE_ACTION                  0x002A
#define EMR_RECTANGLE_ACTION                0x002B
#define EMR_ROUNDRECT_ACTION                0x002C
#define EMR_ARC_ACTION                      0x002D
#define EMR_CHORD_ACTION                    0x002E
#define EMR_PIE_ACTION                      0x002F
#define EMR_EXTFLOODFILL_ACTION             0x0035
#define EMR_LINETO_ACTION                   0x0036
#define EMR_ARCTO_ACTION                    0x0037
#define EMR_POLYDRAW_ACTION                 0x0038
#define EMR_FILLPATH_ACTION                 0x003E
#define EMR_STROKEANDFILLPATH_ACTION        0x003F
#define EMR_STROKEPATH_ACTION               0x0040
#define EMR_FILLRGN_ACTION                  0x0047
#define EMR_FRAMERGN_ACTION                 0x0048
#define EMR_PAINTRGN_ACTION                 0x004A
#define EMR_EXTTEXTOUTA_ACTION              0x0053
#define EMR_EXTTEXTOUTW_ACTION              0x0054
#define EMR_POLYBEZIER16_ACTION             0x0055
#define EMR_POLYGON16_ACTION                0x0056
#define EMR_POLYLINE16_ACTION               0x0057
#define EMR_POLYBEZIERTO16_ACTION           0x0058
#define EMR_POLYLINETO16_ACTION             0x0059
#define EMR_POLYPOLYLINE16_ACTION           0x005A
#define EMR_POLYPOLYGON16_ACTION            0x005B
#define EMR_POLYDRAW16_ACTION               0x005C
#define EMR_POLYTEXTOUTA_ACTION             0x0060
#define EMR_POLYTEXTOUTW_ACTION             0x0061
#define EMR_SMALLTEXTOUT_ACTION             0x006C
#define EMR_GRADIENTFILL_ACTION             0x0076

/// Bitmap record types
#define EMR_BITBLT_ACTION                   0x004C
#define EMR_STRETCHBLT_ACTION               0x004D
#define EMR_MASKBLT_ACTION                  0x004E
#define EMR_PLGBLT_ACTION                   0x004F
#define EMR_SETDIBITSTODEVICE_ACTION        0x0050
#define EMR_STRETCHDIBITS_ACTION            0x0051
#define EMR_ALPHABLEND_ACTION               0x0072
#define EMR_TRANSPARENTBLT_ACTION           0x0074

/// Escape records (note: we really do NOT want to implement these...)
#define EMR_DRAWESCAPE_ACTION               0x0069
#define EMR_EXTESCAPE_ACTION                0x006A
#define EMR_NAMEDESCAPE_ACTION              0x006E

/// OpenGL records
#define EMR_GLSRECORD_ACTION                0x0066
#define EMR_GLSBOUNDEDRECORD_ACTION         0x0067

/// Path records
#define EMR_BEGINPATH_ACTION                0x003B
#define EMR_ENDPATH_ACTION                  0x003C
#define EMR_CLOSEFIGURE_ACTION              0x003D
#define EMR_FLATTENPATH_ACTION              0x0041
#define EMR_WIDENPATH_ACTION                0x0042
#define EMR_ABORTPATH_ACTION                0x0044

/// Region data header
// #define RDH_RECTANGLES                      0x00000001

/// Pixel format descriptor bit flags (note comment at end of value is short
/// bit flag constants - see [MS-EMF] section 2.2.22)
#define PFD_NEED_PALETTE                    0x00000001  // P
#define PFD_GENERIC_FORMAT                  0x00000002  // F
#define PFD_SUPPORT_OPENGL                  0x00000004  // SO
#define PFD_SUPPORT_GDI                     0x00000008  // G
#define PFD_DRAW_TO_BITMAP                  0x00000010  // M
#define PFD_DRAW_TO_WINDOW                  0x00000020  // W
#define PFD_STEREO                          0x00000040  // S
#define PFD_DOUBLEBUFFER                    0x00000080  // D
#define PFD_SUPPORT_COMPOSITION             0x00000100  // C
#define PFD_DIRECT3D_ACCELERATED            0x00000200  // DA
#define PFD_SUPPORT_DIRECTDRAW              0x00000400  // DS
#define PFD_GENERIC_ACCELERATED             0x00000800  // A
#define PFD_SWAP_LAYER_BUFFERS              0x00001000  // SL
#define PFD_SWAP_COPY                       0x00002000  // SC
#define PFD_SWAP_EXCHANGE                   0x00004000  // SE
#define PFD_NEED_SYSTEM_PALETTE             0x00008000  // SP
#define PFD_RESERVED_BIT16                  0x00010000  // Bit 16
#define PFD_RESERVED_BIT17                  0x00020000  // Bit 17
#define PFD_RESERVED_BIT18                  0x00040000  // Bit 18
#define PFD_RESERVED_BIT19                  0x00080000  // Bit 19
#define PFD_RESERVED_BIT20                  0x00100000  // Bit 20
#define PFD_RESERVED_BIT21                  0x00200000  // Bit 21
#define PFD_RESERVED_BIT22                  0x00400000  // Bit 22
#define PFD_RESERVED_BIT23                  0x00800000  // Bit 23
#define PFD_RESERVED_BIT24                  0x01000000  // Bit 24
#define PFD_RESERVED_BIT25                  0x02000000  // Bit 25
#define PFD_RESERVED_BIT26                  0x04000000  // Bit 26
#define PFD_RESERVED_BIT27                  0x08000000  // Bit 27
#define PFD_STEREO_DONTCARE                 0x10000000  // SD
#define PFD_DOUBLEBUFFER_DONTCARE           0x20000000  // DD
#define PFD_DEPTH_DONTCARE                  0x40000000  // DP
#define PFD_RESERVED_BIT31                  0X80000000  // Bit 31

//  Pixel format descriptor types
#define PFD_TYPE_RGBA                       0x00        // Pixel format is RGBA
#define PFD_TYPE_COLORINDEX                 0x01        // Each pixel is index in a color table

class EmfClosePath
{
    bool bPathClosed;

public:

                 EmfClosePath() { bPathClosed=false; }
    virtual      ~EmfClosePath() {}

    virtual void ClosePath() { bPathClosed=true; }
    virtual int  IsPathClosed() { return bPathClosed; }
};

enum PointEMF
{
    PT_CLOSEFIGURE      = 0x01,
    PT_LINETO           = 0x02,
    PT_BEZIERTO         = 0x04,
    PT_MOVETO           = 0x06
};

struct PointL
{
    sal_Int32  mnX;
    sal_Int32  mnY;

    void Read( SvStream& rIStm );
};

struct PointS
{
    sal_Int16  mnX;
    sal_Int16  mnY;

    void Read( SvStream& rIStm );
};

struct RectL
{
    sal_uInt32 mnLeft;
    sal_uInt32 mnTop;
    sal_uInt32 mnRight;
    sal_uInt32 mnBottom;

    void Read( SvStream& rIStm );
};

struct SizeL
{
    sal_uInt32 mnX;
    sal_uInt32 mnY;

    void Read( SvStream& rIStm );
};

// TernaryRasterOperation (MS-WMF 2.1.1.31)

#define BLACKNESS_EMF   0x00
#define DPSOON_EMF      0x01
#define DPSONA_EMF      0x02
#define PSON_EMF        0x03
#define SDPONA_EMF      0x04
#define DPON_EMF        0x05
#define PDSXNON_EMF     0x06
#define PDSAON_EMF      0x07
#define SDPNAA_EMF      0x08
#define PDSXON_EMF      0x09
#define DPNA_EMF        0x0A
#define PSDNAON_EMF     0x0B
#define SPNA_EMF        0x0C
#define PDSNAON_EMF     0x0D
#define PDSONON_EMF     0x0E
#define PN_EMF          0x0F
#define PDSONA_EMF      0x10
#define NOTSRCERASE_EMF 0x11
#define SDPXNON_EMF     0x12
#define SDPAON_EMF      0x13
#define DPSXNON_EMF     0x14
#define DPSAON_EMF      0x15
#define PSDPSANAXX_EMF  0x16
#define SSPXDSXAXN_EMF  0x17
#define SPXPDXA_EMF     0x18
#define SDPSANAXN_EMF   0x19
#define PDSPAOX_EMF     0x1A
#define SDPSXAXN_EMF    0x1B
#define PSDPAOX_EMF     0x1C
#define DSPDXAXN_EMF    0x1D
#define PDSOX_EMF       0x1E
#define PDSOAN_EMF      0x1F
#define DPSNAA_EMF      0x20
#define SDPXON_EMF      0x21
#define DSNA_EMF        0x22
#define SPDNAON_EMF     0x23
#define SPXDSXA_EMF     0x24
#define PDSPANAXN_EMF   0x25
#define SDPSAOX_EMF     0x26
#define SDPSXNOX_EMF    0x27
#define DPSXA_EMF       0x28
#define PSDPSAOXXN_EMF  0x29
#define DPSANA_EMF      0x2A
#define SSPXPDXAXN_EMF  0x2B
#define SPDSOAX_EMF     0x2C
#define PSDNOX_EMF      0x2D
#define PSDPXOX_EMF     0x2E
#define PSDNOAN_EMF     0x2F
#define PSNA_EMF        0x30
#define SDPNAON_EMF     0x31
#define SDPSOOX_EMF     0x32
#define NOTSRCCOPY_EMF  0x33
#define SPDSAOX_EMF     0x34
#define SPDSXNOX_EMF    0x35
#define SDPOX_EMF       0x36
#define SDPOAN_EMF      0x37
#define PSDPOAX_EMF     0x38
#define SPDNOX_EMF      0x39
#define SPDSXOX_EMF     0x3A
#define SPDNOAN_EMF     0x3B
#define PSX_EMF         0x3C
#define SPDSONOX_EMF    0x3D
#define SPDSNAOX_EMF    0x3E
#define PSAN_EMF        0x3F
#define PSDNAA_EMF      0x40
#define DPSXON_EMF      0x41
#define SDXPDXA_EMF     0x42
#define SPDSANAXN_EMF   0x43
#define SRCERASE_EMF    0x44
#define DPSNAON_EMF     0x45
#define DSPDAOX_EMF     0x46
#define PSDPXAXN_EMF    0x47
#define SDPXA_EMF       0x48
#define PDSPDAOXXN_EMF  0x49
#define DPSDOAX_EMF     0x4A
#define PDSNOX_EMF      0x4B
#define SDPANA_EMF      0x4C
#define SSPXDSXOXN_EMF  0x4D
#define PDSPXOX_EMF     0x4E
#define PDSNOAN_EMF     0x4F
#define PDNA_EMF        0x50
#define DSPNAON_EMF     0x51
#define DPSDAOX_EMF     0x52
#define SPDSXAXN_EMF    0x53
#define DPSONON_EMF     0x54
#define DSTINVERT_EMF   0x55
#define DPSOX_EMF       0x56
#define DPSOAN_EMF      0x57
#define PDSPOAX_EMF     0x58
#define DPSNOX_EMF      0x59
#define PATINVERT_EMF   0x5A
#define DPSDONOX_EMF    0x5B
#define DPSDXOX_EMF     0x5C
#define DPSNOAN_EMF     0x5D
#define DPSDNAOX_EMF    0x5E
#define DPAN_EMF        0x5F
#define PDSXA_EMF       0x60
#define DSPDSAOXXN_EMF  0x61
#define DSPDOAX_EMF     0x62
#define SDPNOX_EMF      0x63
#define SDPSOAX_EMF     0x64
#define DSPNOX_EMF      0x65
#define SRCINVERT_EMF   0x66
#define SDPSONOX_EMF    0x67
#define DSPDSONOXXN_EMF 0x68
#define PDSXXN_EMF      0x69
#define DPSAX_EMF       0x6A
#define PSDPSOAXXN_EMF  0x6B
#define SDPAX_EMF       0x6C
#define PDSPDOAXXN_EMF  0x6D
#define SDPSNOAX_EMF    0x6E
#define PDXNAN_EMF      0x6F
#define PDSANA_EMF      0x70
#define SSDXPDXAXN_EMF  0x71
#define SDPSXOX_EMF     0x72
#define SDPNOAN_EMF     0x73
#define DSPDXOX_EMF     0x74
#define DSPNOAN_EMF     0x75
#define SDPSNAOX_EMF    0x76
#define DSAN_EMF        0x77
#define PDSAX_EMF       0x78
#define DSPDSOAXXN_EMF  0x79
#define DPSDNOAX_EMF    0x7A
#define SDPXNAN_EMF     0x7B
#define SPDSNOAX_EMF    0x7C
#define DPSXNAN_EMF     0x7D
#define SPXDSXO_EMF     0x7E
#define DPSAAN_EMF      0x7F
#define DPSAA_EMF       0x80
#define SPXDSXON_EMF    0x81
#define DPSXNA_EMF      0x82
#define SPDSNOAXN_EMF   0x83
#define SDPXNA_EMF      0x84
#define PDSPNOAXN_EMF   0x85
#define DSPDSOAXX_EMF   0x86
#define PDSAXN_EMF      0x87
#define SRCAND_EMF      0x88
#define SDPSNAOXN_EMF   0x89
#define DSPNOA_EMF      0x8A
#define DSPDXOXN_EMF    0x8B
#define SDPNOA_EMF      0x8C
#define SDPSXOXN_EMF    0x8D
#define SSDXPDXAX_EMF   0x8E
#define PDSANAN_EMF     0x8F
#define PDSXNA_EMF      0x90
#define SDPSNOAXN_EMF   0x91
#define DPSDPOAXX_EMF   0x92
#define SPDAXN_EMF      0x93
#define PSDPSOAXX_EMF   0x94
#define DPSAXN_EMF      0x95
#define DPSXX_EMF       0x96
#define PSDPSONOXX_EMF  0x97
#define SDPSONOXN_EMF   0x98
#define DSXN_EMF        0x99
#define DPSNAX_EMF      0x9A
#define SDPSOAXN_EMF    0x9B
#define SPDNAX_EMF      0x9C
#define DSPDOAXN_EMF    0x9D
#define DSPDSAOXX_EMF   0x9E
#define PDSXAN_EMF      0x9F
#define DPA_EMF         0xA0
#define PDSPNAOXN_EMF   0xA1
#define DPSNOA_EMF      0xA2
#define DPSDXOXN_EMF    0xA3
#define PDSPONOXN_EMF   0xA4
#define PDXN_EMF        0xA5
#define DSPNAX_EMF      0xA6
#define PDSPOAXN_EMF    0xA7
#define DPSOA_EMF       0xA8
#define DPSOXN_EMF      0xA9
#define D_EMF           0xAA
#define DPSONO_EMF      0xAB
#define SPDSXAX_EMF     0xAC
#define DPSDAOXN_EMF    0xAD
#define DSPNAO_EMF      0xAE
#define DPNO_EMF        0xAF
#define PDSNOA_EMF      0xB0
#define PDSPXOXN_EMF    0xB1
#define SSPXDSXOX_EMF   0xB2
#define SDPANAN_EMF     0xB3
#define PSDNAX_EMF      0xB4
#define DPSDOAXN_EMF    0xB5
#define DPSDPAOXX_EMF   0xB6
#define SDPXAN_EMF      0xB7
#define PSDPXAX_EMF     0xB8
#define DSPDAOXN_EMF    0xB9
#define DPSNAO_EMF      0xBA
#define MERGEPAINT_EMF  0xBB
#define SPDSANAX_EMF    0xBC
#define SDXPDXAN_EMF    0xBD
#define DPSXO_EMF       0xBE
#define DPSANO_EMF      0xBF
#define MERGECOPY_EMF   0xC0
#define SPDSNAOXN_EMF   0xC1
#define SPDSONOXN_EMF   0xC2
#define PSXN_EMF        0xC3
#define SPDNOA_EMF      0xC4
#define SPDSXOXN_EMF    0xC5
#define SDPNAX_EMF      0xC6
#define PSDPOAXN_EMF    0xC7
#define SDPOA_EMF       0xC8
#define SPDOXN_EMF      0xC9
#define DPSDXAX_EMF     0xCA
#define SPDSAOXN_EMF    0xCB
#define SRCCOPY_EMF     0xCC
#define SDPONO_EMF      0xCD
#define SDPNAO_EMF      0xCE
#define SPNO_EMF        0xCF
#define PSDNOA_EMF      0xD0
#define PSDPXOXN_EMF    0xD1
#define PDSNAX_EMF      0xD2
#define SPDSOAXN_EMF    0xD3
#define SSPXPDXAX_EMF   0xD4
#define DPSANAN_EMF     0xD5
#define PSDPSAOXX_EMF   0xD6
#define DPSXAN_EMF      0xD7
#define PDSPXAX_EMF     0xD8
#define SDPSAOXN_EMF    0xD9
#define DPSDANAX_EMF    0xDA
#define SPXDSXAN_EMF    0xDB
#define SPDNAO_EMF      0xDC
#define SDNO_EMF        0xDD
#define SDPXO_EMF       0xDE
#define SDPANO_EMF      0xDF
#define PDSOA_EMF       0xE0
#define PDSOXN_EMF      0xE1
#define DSPDXAX_EMF     0xE2
#define PSDPAOXN_EMF    0xE3
#define SDPSXAX_EMF     0xE4
#define PDSPAOXN_EMF    0xE5
#define SDPSANAX_EMF    0xE6
#define SPXPDXAN_EMF    0xE7
#define SSPXDSXAX_EMF   0xE8
#define DSPDSANAXXN_EMF 0xE9
#define DPSAO_EMF       0xEA
#define DPSXNO_EMF      0xEB
#define SDPAO_EMF       0xEC
#define SDPXNO_EMF      0xED
#define SRCPAINT_EMF    0xEE
#define SDPNOO_EMF      0xEF
#define PATCOPY_EMF     0xF0
#define PDSONO_EMF      0xF1
#define PDSNAO_EMF      0xF2
#define PSNO_EMF        0xF3
#define PSDNAO_EMF      0xF4
#define PDNO_EMF        0xF5
#define PDSXO_EMF       0xF6
#define PDSANO_EMF      0xF7
#define PDSAO_EMF       0xF8
#define PDSXNO_EMF      0xF9
#define VDPO_EMF        0xFA
#define PATPAINT_EMF    0xFB
#define PSO_EMF         0xFC
#define PSDNOO_EMF      0xFD
#define DPSOO_EMF       0xFE
#define WHITENESS_EMF   0xFF

enum LogicalColorSpace
{
    LCS_CALIBRATED_RGB = 0x00000000,
    LCS_sRGB = 0x73524742,
    LCS_WINDOWS_COLOR_SPACE = 0x57696E20
};

enum GamutMappingIntent
{
    LCS_GM_ABS_COLORIMETRIC = 0x00000008,
    LCS_GM_BUSINESS = 0x00000001,
    LCS_GM_GRAPHICS = 0x00000002,
    LCS_GM_IMAGES = 0x00000004
};

struct CIEXYZ
{
    sal_uInt32 mnCiexyzX;
    sal_uInt32 mnCiexyzY;
    sal_uInt32 mnCiexyzZ;

    void Read( SvStream& rIStm );
};

struct CIEXYZTriple
{
    CIEXYZ maCiexyzRed;
    CIEXYZ maCiexyzBlue;
    CIEXYZ maCiexyzGreen;

    void Read( SvStream& rIStm );
};

class LogColorSpaceBase
{
public:
    virtual     ~LogColorSpaceBase() {}

    sal_uInt32 mnSignature;
    sal_uInt32 mnSize;
    sal_uInt32 mnVersion;
    LogicalColorSpace meColorSpaceType;
    GamutMappingIntent meIntent;
    CIEXYZTriple maEndpoints;
    sal_uInt32 mnGammaRed;
    sal_uInt32 mnGammaGreen;
    sal_uInt32 mnGammaBlue;
    OUString maFileName;

    virtual void Read( SvStream& rIStm );
};

class LogColorSpace : public LogColorSpaceBase
{
public:
    virtual void Read( SvStream& rIStm );
};

class LogColorSpaceW : public LogColorSpaceBase
{
public:
    virtual void Read( SvStream& rIStm );
};

template < class PointType >
struct EmfPolygon
{
    std::vector<PointType> maPoints;
};

enum ICMMode
{
    ICM_OFF = 0x01,
    ICM_ON = 0x02,
    ICM_QUERY = 0x03,
    ICM_DONE_OUTSIDEDC = 0x04
};

struct ColorRef
{
    sal_uInt8 mnRed;
    sal_uInt8 mnGreen;
    sal_uInt8 mnBlue;

    void Read( SvStream& rIStm );
};

struct ColorAdjustment
{
    sal_uInt16 mnSize;
    sal_uInt16 mnValues;
    sal_uInt16 mnIlluminantIndex;
    sal_uInt16 mnRedGamma;
    sal_uInt16 mnGreenGamma;
    sal_uInt16 mnBlueGamma;
    sal_uInt16 mnReferenceBlack;
    sal_uInt16 mnReferenceWhite;
    sal_Int16 mnContrast;
    sal_Int16 mnBrightness;
    sal_Int16 mnColorfulness;
    sal_Int16 mnRedGreenTint;

    void Read( SvStream& rIStm );
};

enum CharacterSet
{
    ANSI_CHARSET_EMF            = 0x00000000,
    DEFAULT_CHARSET_EMF         = 0x00000001,
    SYMBOL_CHARSET_EMF          = 0x00000002,
    MAC_CHARSET_EMF             = 0x0000004D,
    SHIFTJIS_CHARSET_EMF        = 0x00000080,
    HANGUL_CHARSET_EMF          = 0x00000081,
    JOHAB_CHARSET_EMF           = 0x00000082,
    GB2312_CHARSET_EMF          = 0x00000086,
    CHINESEBIG5_CHARSET_EMF     = 0x00000088,
    GREEK_CHARSET_EMF           = 0x000000A1,
    TURKISH_CHARSET_EMF         = 0x000000A2,
    VIETNAMESE_CHARSET_EMF      = 0x000000A3,
    HEBREW_CHARSET_EMF          = 0x000000B1,
    ARABIC_CHARSET_EMF          = 0x000000B2,
    BALTIC_CHARSET_EMF          = 0x000000BA,
    RUSSIAN_CHARSET_EMF         = 0x000000CC,
    THAI_CHARSET_EMF            = 0x000000DE,
    EASTEUROPE_CHARSET_EMF      = 0x000000EE,
    OEM_CHARSET_EMF             = 0x000000FF
};

enum OutPrecision
{
    OUT_DEFAULT_PRECIS          = 0x00000000,
    OUT_STRING_PRECIS           = 0x00000001,
    OUT_STROKE_PRECIS           = 0x00000003,
    OUT_TT_PRECIS               = 0x00000004,
    OUT_DEVICE_PRECIS           = 0x00000005,
    OUT_RASTER_PRECIS           = 0x00000006,
    OUT_TT_ONLY_PRECIS          = 0x00000007,
    OUT_OUTLINE_PRECIS          = 0x00000008,
    OUT_SCREEN_OUTLINE_PRECIS   = 0x00000009,
    OUT_PS_ONLY_PRECIS          = 0x0000000A
};

#define CLIP_DEFAULT_PRECIS     0x00000000
#define CLIP_CHARACTER_PRECIS   0x00000001
#define CLIP_STROKE_PRECIS      0x00000002
#define CLIP_LH_ANGLES          0x00000010
#define CLIP_TT_ALWAYS          0x00000020
#define CLIP_DFA_DISABLE        0x00000040
#define CLIP_EMBEDDED           0x00000080

enum LayoutMode
{
    LAYOUT_LTR                  = 0x00000000,
    LAYOUT_RTL                  = 0x00000001,
    LAYOUT_BITMAPORIENTATIONPRESERVED = 0x00000008
};

enum FamilyFont
{
    FF_DONTCARE_EMF             = 0x00,
    FF_ROMAN_EMF                = 0x01,
    FF_SWISS_EMF                = 0x02,
    FF_MODERN_EMF               = 0x03,
    FF_SCRIPT_EMF               = 0x04,
    FF_DECORATIVE_EMF           = 0x05
};

enum PitchFont
{
    DEFAULT_PITCH_EMF           = 0,
    FIXED_PITCH_EMF             = 1,
    VARIABLE_PITCH_EMF          = 2
};

struct PitchAndFamily
{
    FamilyFont meFamily;
    PitchFont  mePitch;

    void Read( SvStream& rIStm );
};

enum FontQuality
{
    DEFAULT_QUALITY             = 0x00,
    DRAFT_QUALITY               = 0x01,
    PROOF_QUALITY               = 0x02,
    NONANTIALIASED_QUALITY      = 0x03,
    ANTIALIASED_QUALITY         = 0x04,
    CLEARTYPE_QUALITY           = 0x05
};

class LogFont
{
public:
                   LogFont();
    virtual        ~LogFont() {};

    sal_Int32      mnHeight;
    sal_Int32      mnWidth;
    sal_Int32      mnEscapement;
    sal_Int32      mnOrientation;
    sal_Int32      mnWeight;
    bool           mbItalic;
    bool           mbUnderline;
    bool           mbStrikeout;
    CharacterSet   meCharSet;
    OutPrecision   meOutPrecision;
    sal_uInt8      mnClipPrecision;
    FontQuality    meQuality;
    PitchAndFamily maPitchAndFamily;
    OUString       maFacename;

    virtual void Read( SvStream& rIStm );
};

class LogFontEx : public LogFont
{
public:
                LogFontEx();
    virtual     ~LogFontEx() {};

    OUString    maFullName;
    OUString    maStyle;
    OUString    maScript;

    virtual void Read( SvStream& rIStm );
};

struct DesignVector
{
    sal_uInt32 mnSignature;
    sal_uInt32 mnNumAxes;
    std::vector<sal_uInt32> maValues;

    void Read( SvStream& rIStm );
};

class LogFontExDv : public LogFontEx
{
public:
                 LogFontExDv();
    virtual      ~LogFontExDv() {};

    DesignVector maDesignVector;

    virtual void Read( SvStream& rIStm );
};

enum BrushStyle
{
    BS_SOLID_EMF               = 0x0000,
    BS_NULL_EMF                = 0x0001,
    BS_HATCHED_EMF             = 0x0002,
    BS_PATTERN_EMF             = 0x0003,
    BS_INDEXED_EMF             = 0x0004,
    BS_DIBPATTERN_EMF          = 0x0005,
    BS_DIBPATTERNPT_EMF        = 0x0006,
    BS_PATTERN8X8_EMF          = 0x0007,
    BS_DIBPATTERN8X8_EMF       = 0x0008,
    BS_MONOPATTERN_EMF         = 0x0009
};

enum HatchStyleEMF
{
    HS_SOLIDCLR_EMF            = 0x0006,
    HS_DITHEREDCLR_EMF         = 0x0007,
    HS_SOLIDTEXTCLR_EMF        = 0x0008,
    HS_DITHEREDTEXTCLR_EMF     = 0x0009,
    HS_SOLIDBKCLR_EMF          = 0x000A,
    HS_DITHEREDBKCLR_EMF       = 0x000B
};

struct LogBrushEx
{
    BrushStyle  meBrushStyle;
    ColorRef    maColor;
    HatchStyleEMF meHatchStyle;

    void Read( SvStream& rIStm);
};

#define PS_COSMETIC_EMF        0x00000000
#define PS_ENDCAP_ROUND_EMF    0x00000000
#define PS_JOIN_ROUND_EMF      0x00000000
#define PS_SOLID_EMF           0x00000000
#define PS_DASH_EMF            0x00000001
#define PS_DOT_EMF             0x00000002
#define PS_DASHDOT_EMF         0x00000003
#define PS_DASHDOTDOT_EMF      0x00000004
#define PS_NULL_EMF            0x00000005
#define PS_INSIDEFRAME_EMF     0x00000006
#define PS_USERSTYLE_EMF       0x00000007
#define PS_ALTERNATE_EMF       0x00000008
#define PS_ENDCAP_SQUARE_EMF   0x00000100
#define PS_ENDCAP_FLAT_EMF     0x00000200
#define PS_JOIN_BEVEL_EMF      0x00001000
#define PS_JOIN_MITER_EMF      0x00002000
#define PS_GEOMETRIC_EMF       0x00010000

struct LogPen
{
    sal_uInt32 mnPenStyle;
    sal_uInt32 mnWidth;
    ColorRef maColorRef;

    void Read( SvStream& rIStm );
};

struct LogPenEx
{
    sal_uInt32 mnPenStyle;
    sal_uInt32 mnWidth;
    LogBrushEx maBrush;
    std::vector< sal_uInt32 > maStyleEntries;

    void Read( SvStream& rIStm );
};

enum BackgroundMode
{
    TRANSPARENT                 = 0x0001,
    OPAQUE                      = 0x0002
};

#ifndef MM_TEXT

#define MM_TEXT                 0x01
#define MM_LOMETRIC             0x02
#define MM_HIMETRIC             0x03
#define MM_LOENGLISH            0x04
#define MM_HIENGLISH            0x05
#define MM_TWIPS                0x06
#define MM_ISOTROPIC            0x07
#define MM_ANISOTROPIC          0x08

#endif

/// TextAlignmentMode flags (see [MS-WMF] section 2.1.2.3)
#define TA_NOUPDATE_CP          0x0000
#define TA_LEFT                 0x0000
#define TA_TOP                  0x0000
#define TA_UPDATECP             0x0001
#define TA_RIGHT                0x0002
#define TA_CENTER               0x0006
#define TA_BOTTOM               0x0008
#define TA_BASELINE             0x0018
#define TA_RTLREADING           0x0100

/// VerticalTextAlignmentMode flags (see [MS-WMF] section 2.1.2.4)
#define VTA_TOP                 0x0000
#define VTA_RIGHT               0x0000
#define VTA_BOTTOM              0x0002
#define VTA_CENTER              0x0006
#define VTA_LEFT                0x0008
#define VTA_BASELINE            0x0018

enum DIBColors
{
    DIB_RGB_COLORS              = 0x00,
    DIB_PAL_COLORS              = 0x01,
    DIB_PAL_INDICES             = 0x02
};

struct LogPaletteEntry
{
    sal_uInt8 mnBlue;
    sal_uInt8 mnGreen;
    sal_uInt8 mnRed;

    void Read( SvStream& rIStm );
};

struct LogPalette
{
    std::vector< LogPaletteEntry > maPaletteEntries;

    void Read( SvStream& rIStm );
};

enum PolygonFillMode
{
    ALTERNATE                   = 0x01,
    WINDING                     = 0x02
};

enum BinaryRasterOperations
{
    R2_BLACK_EMF                = 0x0001,
    R2_NOTMERGEPEN_EMF          = 0x0002,
    R2_MASKNOTPEN_EMF           = 0x0003,
    R2_NOTCOPYPEN_EMF           = 0x0004,
    R2_MASKPENNOT_EMF           = 0x0005,
    R2_NOT_EMF                  = 0x0006,
    R2_XORPEN_EMF               = 0x0007,
    R2_NOTMASKPEN_EMF           = 0x0008,
    R2_MASKPEN_EMF              = 0x0009,
    R2_NOTXORPEN_EMF            = 0x000A,
    R2_NOP_EMF                  = 0x000B,
    R2_MERGENOTPEN_EMF          = 0x000C,
    R2_COPYPEN_EMF              = 0x000D,
    R2_MERGEPENNOT_EMF          = 0x000E,
    R2_MERGEPEN_EMF             = 0x000F,
    R2_WHITE_EMF                = 0x0010
};

enum StretchMode
{
    STRETCH_ANDSCANS_EMF        = 0x01,
    STRETCH_ORSCANS_EMF         = 0x02,
    STRETCH_DELETESCANS_EMF     = 0x03,
    STRETCH_HALFTONE_EMF        = 0x04
};

enum RegionMode
{
    RGN_AND_EMF                 = 0x01,
    RGN_OR_EMF                  = 0x02,
    RGN_XOR_EMF                 = 0x03,
    RGN_DIFF_EMF                = 0x04,
    RGN_COPY_EMF                = 0x05
};

struct RegionDataHeader
{
    sal_uInt32 mnSize;
    sal_uInt32 mnType;
    sal_uInt32 mnCountRects;
    sal_uInt32 mnRgnSize;
    RectL      maBounds;

    void Read( SvStream& rIStm );
};

struct RegionData
{
    RegionDataHeader*  mpRegionDataHeader;
    std::vector<RectL> maData;

    void Read( SvStream& rIStm );
};

enum FloodFill
{
    FLOODFILLBORDER             = 0x00000000,
    FLOODFILLSURFACE            = 0x00000001
};

enum GradientFill
{
    GRADIENT_FILL_RECT_H        = 0x00000000,
    GRADIENT_FILL_RECT_V        = 0x00000001,
    GRADIENT_FILL_TRIANGLE      = 0x00000002
};

struct TriVertex
{
    sal_Int32 mnX;
    sal_Int32 mnY;
    sal_uInt16 mnRed;
    sal_uInt16 mnGreen;
    sal_uInt16 mnBlue;
    sal_uInt16 mnAlpha;

    void Read(SvStream& rIStm);
};

class GradientObject
{
public:
            GradientObject() {};
            virtual ~GradientObject() {};

    virtual void Read(SvStream&) {};
};

class GradientRectangle : public GradientObject
{
public:
    sal_uInt32 mnUpper;
    sal_uInt32 mnLower;

    virtual void Read(SvStream& rIStm) SAL_OVERRIDE;
};

class GradientTriangle : public GradientObject
{
public:
    sal_uInt32 mnVertex1;
    sal_uInt32 mnVertex2;
    sal_uInt32 mnVertex3;

    virtual void Read(SvStream& rIStm) SAL_OVERRIDE;
};

class VertexObjects
{
public:
    std::vector< GradientObject > maVertexObjects;

    void Read(SvStream& rIStm, sal_uInt32 nNumObjects, bool bGradRectsUsed);
};

enum GraphicsMode
{
    GM_COMPATIBLE_EMF           = 0x00000001,
    GM_ADVANCED_EMF             = 0x00000002
};

struct PixelFormatDescriptor
{
    sal_uInt16  mnSize;
    sal_uInt16  mnVersion;
    sal_uInt32  mnFlags;
    sal_uInt8   mnPixelType;
    sal_uInt8   mnColorBits;
    sal_uInt8   mnRedBits;
    sal_uInt8   mnRedShift;
    sal_uInt8   mnGreenBits;
    sal_uInt8   mnGreenShift;
    sal_uInt8   mnBlueBits;
    sal_uInt8   mnBlueShift;
    sal_uInt8   mnAlphaBits;
    sal_uInt8   mnAlphaShift;
    sal_uInt8   mnAccumBits;
    sal_uInt8   mnAccumRedBits;
    sal_uInt8   mnAccumGreenBits;
    sal_uInt8   mnAccumBlueBits;
    sal_uInt8   mnDepthBits;
    sal_uInt8   mnStencilBits;
    sal_uInt8   mnAuxBuffers;
    sal_uInt8   mnLayerType;
    sal_uInt8   mnOverlayPlanes;     // from the "reserved" field
    sal_uInt8   mnUnderlayPlanes;    // from the "reserved" field
    sal_uInt32  mnLayerMask;
    sal_uInt32  mnVisibleMask;
    sal_uInt32  mnDamageMask;

    void Read( SvStream& rIStm );
};

struct BLENDFUNCTION
{
    sal_uInt8   mnBlendOperation;
    sal_uInt8   mnBlendFlags;
    sal_uInt8   mnSrcConstantAlpha;
    sal_uInt8   mnAlphaFormat;

    void Read( SvStream& rIStm );
};

struct ROP4
{
    sal_uInt8   mnBackgroundROP3;
    sal_uInt8   mnForegroundROP3;

    void Read( SvStream& rIStm );
};

struct XFormEMF {
    float       mfM11;
    float       mfM12;
    float       mfM21;
    float       mfM22;
    float       mfDx;
    float       mfDy;

    void Read( SvStream& rIStm );

    /// from [MS-EMF] section 2.2.28
    float       transformX(float fX, float fY) { return mfM11 * fX + mfM21 * fY + mfDx; }
    /// from [MS-EMF] section 2.2.28
    float       transformY(float fX, float fY) { return mfM12 * fX + mfM22 * fY + mfDy; }
};

enum ModifyWorldTransformMode {
    MWT_IDENTITY_EMF            = 0x01,
    MWT_LEFTMULTIPLY_EMF        = 0x02,
    MWT_RIGHTMULTIPLY_EMF       = 0x03,
    MWT_SET_EMF                 = 0x04
};

enum HeaderType { NORMAL, EXT1, EXT2 };

class EmfDeviceContext
{
    // TODO: needs to store pointers to objects - bitmaps, brushes palettes, fonts,
    // pens, regions, drawing modes and mapping modes
public:
                        EmfDeviceContext() {}

    void                SaveDC() { SAL_INFO("vcl.emf", "Saving Device Context"); }
    void                RestoreDC() { SAL_INFO("vcl.emf", "Restoring Device Context"); }
};

/** See [MS-EMF] section 2.2.27 UniversalFontId object */
struct UniversalFontId
{
    sal_uInt32  mnChecksum;
    sal_uInt32  mnIndex;

    sal_uLong ComputeFileviewCheckSum(void* /* pvView */, sal_uLong* /* cjView */) { return 0L; }

    void Read( SvStream& rIStm )
    {
        rIStm.ReadUInt32(mnChecksum);
        rIStm.ReadUInt32(mnIndex);

        SAL_INFO("vcl.emf", "\t\tindex: " << mnIndex);
    }
};

// Defined in [MS-EMF] section 2.1.11 (ExtTextOutOptions Enumeration)
// It's really a bitmask field, don't think it's a good idea to use
// an enum

#define ETO_OPAQUE_EMF                      0x00000002
#define ETO_CLIPPED_EMF                     0x00000004
#define ETO_GLYPH_INDEX_EMF                 0x00000010
#define ETO_RTLREADING_EMF                  0x00000080
#define ETO_NO_RECT_EMF                     0x00000100
#define ETO_SMALL_CHARS_EMF                 0x00000200
#define ETO_NUMERICSLOCAL_EMF               0x00000400
#define ETO_NUMERICSLATIN_EMF               0x00000800
#define ETO_IGNORELANGUAGE_EMF              0x00001000
#define ETO_PDY_EMF                         0x00002000
#define ETO_REVERSE_INDEX_MAP_EMF           0x00010000

struct EmrText
{
    PointL                  maReference;
    sal_uInt32              mnOptions;
    RectL                   maRectangle;
    OUString                maStringBuffer;
    std::vector<sal_uInt32> maOutputDx;

    void Read( SvStream& rIStm, sal_uInt64 nStartRecordOffset, bool bUnicode=true);
};

class VCL_DLLPUBLIC EmfAction
{
private:
    sal_uInt32          mnType;
    EmfDeviceContext*   mpDeviceContext;

protected:
    virtual             ~EmfAction();

public:
    explicit            EmfAction();
    explicit            EmfAction( sal_uInt32 nType );

    sal_uInt16          GetType() const { return mnType; }

    void                SetDeviceContext( EmfDeviceContext* pDC );
    EmfDeviceContext*   GetDeviceContext() { return mpDeviceContext; }

public:
    static EmfAction*   ReadEmfAction( SvStream& rIStm );
    virtual void        Read( SvStream& );
};

#endif // INCLUDED_VCL_EMFACT_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
