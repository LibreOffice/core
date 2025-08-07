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

#include <config_options.h>
#include <basegfx/utils/b2dclipstate.hxx>
#include <tools/poly.hxx>
#include <vcl/font.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/rendercontext/State.hxx>
#include <vcl/metaact.hxx>
#include <rtl/ref.hxx>

#include "emfiodllapi.h"

namespace emfio
{
    /* [MS-EMF] - v20210625 - pages 43, 107 */
    enum class RegionMode : sal_uInt32
    {
        RGN_AND  = 0x01,
        RGN_OR   = 0x02,
        RGN_XOR  = 0x03,
        RGN_DIFF = 0x04,
        RGN_COPY = 0x05
    };

    /* [MS-EMF] - v20210625 - pages 40, 198 */
    enum class BackgroundMode : sal_uInt32
    {
        NONE = 0,
        Transparent = 1,
        OPAQUE = 2,
    };

    /* [MS-EMF] - v20210625 - pages 40, 210 */
    /* xform stuff */
    enum class ModifyWorldTransformMode : sal_uInt32
    {
        MWT_IDENTITY      = 0x01,
        MWT_LEFTMULTIPLY  = 0x02,
        MWT_RIGHTMULTIPLY = 0x03,
        MWT_SET           = 0x04
    };

    constexpr sal_uInt32 ENHMETA_STOCK_OBJECT = 0x80000000;

    /* [MS-EMF] - v20210625 - pages 44, 45, 182 */
    /* Stock Logical Objects */
    enum class StockObject : sal_uInt32
    {
        WHITE_BRUSH       = 0,
        LTGRAY_BRUSH      = 1,
        GRAY_BRUSH        = 2,
        DKGRAY_BRUSH      = 3,
        BLACK_BRUSH       = 4,
        NULL_BRUSH        = 5,
        WHITE_PEN         = 6,
        BLACK_PEN         = 7,
        NULL_PEN          = 8,
        ANSI_FIXED_FONT   = 11,
        ANSI_VAR_FONT     = 12,
        SYSTEM_FIXED_FONT = 16
    };

    /* Note: This enum is incomplete compared to the specification */
    /* [MS-WMF] - v20210625 - pages 25-26 */
    enum class WMFRasterOp : sal_uInt16
    {
        NONE = 0,
        Black = 1,
        Not = 6,
        XorPen = 7,
        Nop = 11,
        CopyPen = 13
    };

    /* Note: We have MapMode elsewhere, so we use MappingMode instead */
    /* [MS-EMF] - v20210625 - pages 38-50, 202 */
    /* Mapping modes */
    enum MappingMode : sal_uInt32
    {
        MM_TEXT        = 0x01,
        MM_LOMETRIC    = 0x02,
        MM_HIMETRIC    = 0x03,
        MM_LOENGLISH   = 0x04,
        MM_HIENGLISH   = 0x05,
        MM_TWIPS       = 0x06,
        MM_ISOTROPIC   = 0x07,
        MM_ANISOTROPIC = 0x08
    };

    /* Note: No type is specified, but 32-bit unsigned is used to provide
     * to match the number of bits in the binary literal, and also the
     * previous definition of SetGfxMode() and GetGfxMode() functions */
    /* [MS-EMF] - v20210625 - pages 35 */
    /* Graphics modes */
    enum class GraphicsMode : sal_uInt32
    {
        GM_COMPATIBLE = 0x00000001,
        GM_ADVANCED   = 0x00000002
    } ;

    /* [MS-WMF] - v20210625 - pages 46 */
    /* StretchBlt() modes */
    enum class StretchMode : sal_uInt16
    {
        BLACKONWHITE        = 0x0001,
        WHITEONBLACK        = 0x0002,
        COLORONCOLOR        = 0x0003,
        HALFTONE            = 0x0004,
        STRETCH_ANDSCANS    = BLACKONWHITE,
        STRETCH_ORSCANS     = WHITEONBLACK,
        STRETCH_DELETESCANS = COLORONCOLOR
    };

    constexpr sal_Int32 LF_FACESIZE = 32;

    struct LOGFONTW
    {
        sal_Int32       lfHeight;
        sal_Int32       lfWidth;
        sal_Int32       lfEscapement;
        sal_Int32       lfOrientation;
        sal_Int32       lfWeight;
        sal_uInt8       lfItalic;
        sal_uInt8       lfUnderline;
        sal_uInt8       lfStrikeOut;
        sal_uInt8       lfCharSet;
        sal_uInt8       lfOutPrecision;
        sal_uInt8       lfClipPrecision;
        sal_uInt8       lfQuality;
        sal_uInt8       lfPitchAndFamily;
        OUString        alfFaceName;
        LOGFONTW()
            : lfHeight(0)
            , lfWidth(0)
            , lfEscapement(0)
            , lfOrientation(0)
            , lfWeight(0)
            , lfItalic(0)
            , lfUnderline(0)
            , lfStrikeOut(0)
            , lfCharSet(0)
            , lfOutPrecision(0)
            , lfClipPrecision(0)
            , lfQuality(0)
            , lfPitchAndFamily(0)
        {
        }
    };

    /* [MS-WMF] - v20210625 - pages 153 */
    enum TextAlignmentMode : sal_uInt16
    {
        TA_NOUPDATECP   = 0x0000,
        TA_UPDATECP     = 0x0001,
        TA_LEFT         = 0x0000,
        TA_RIGHT        = 0x0002,
        TA_CENTER       = 0x0006,
        TA_RIGHT_CENTER = (TA_RIGHT | TA_CENTER),
        TA_TOP          = 0x0000,
        TA_BOTTOM       = 0x0008,
        // In [MS-WMF] 2.1.2.3, TA_BASELINE value is wrong.
        // It is 0x0018 and it should be 0x0010.
        TA_BASELINE     = 0x0010,
        TA_RTLREADING   = 0x0100
    };

    /* Note: This enum is incomplete compared to the specification */
    /* [MS-EMF] - v20210625 - pages 47-50, 126 */
    /* Ternary raster operations */
    enum TernaryRasterOperation : sal_uInt32
    {
        SRCCOPY   = 0x00CC0020L,
        SRCPAINT  = 0x00EE0086L,
        SRCAND    = 0x008800C6L,
        SRCINVERT = 0x00660046L,
        SRCERASE  = 0x00440328L,
        PATCOPY   = 0x00F00021L,
        PATINVERT = 0x005A0049L,
        BLACKNESS = 0x00000042L,
        WHITENESS = 0x00FF0062L
    };

    /* [MS-EMF] - v20210625 - pages 40, 41, 65 */
    enum PenStyle : sal_uInt32
    {
        PS_COSMETIC          = 0x00000000,
        PS_SOLID             = 0x00000000,
        PS_DASH              = 0x00000001,
        PS_DOT               = 0x00000002,
        PS_DASHDOT           = 0x00000003,
        PS_DASHDOTDOT        = 0x00000004,
        PS_NULL              = 0x00000005,
        PS_INSIDEFRAME       = 0x00000006,
        PS_USERSTYLE         = 0x00000007,
        PS_ALTERNATE         = 0x00000008,
        PS_STYLE_MASK        = 0x0000000F,

        PS_ENDCAP_ROUND      = 0x00000000,
        PS_ENDCAP_SQUARE     = 0x00000100,
        PS_ENDCAP_FLAT       = 0x00000200,
        PS_ENDCAP_STYLE_MASK = 0x00000F00,

        PS_JOIN_ROUND        = 0x00000000,
        PS_JOIN_BEVEL        = 0x00001000,
        PS_JOIN_MITER        = 0x00002000,
        PS_JOIN_STYLE_MASK   = 0x0000F000,

        PS_GEOMETRIC         = 0x00010000
    };

    /* [MS-WMF] - v20210625 - pages 30, 82 */
    /* Character Sets */
    enum CharacterSet : sal_uInt8
    {
        ANSI_CHARSET        = 0x00000000,
        DEFAULT_CHARSET     = 0x00000001,
        SYMBOL_CHARSET      = 0x00000002,
        SHIFTJIS_CHARSET    = 0x00000080,
        HANGUL_CHARSET      = 0x00000081,
        GB2312_CHARSET      = 0x00000086,
        CHINESEBIG5_CHARSET = 0x00000088,
        OEM_CHARSET         = 0x000000FF,
        /* WINVER >= 0x0400 */
        MAC_CHARSET         = 0x0000004D,
        JOHAB_CHARSET       = 0x00000082,
        GREEK_CHARSET       = 0x000000A1,
        TURKISH_CHARSET     = 0x000000A2,
        VIETNAMESE_CHARSET  = 0x000000A3,
        HEBREW_CHARSET      = 0x000000B1,
        ARABIC_CHARSET      = 0x000000B2,
        BALTIC_CHARSET      = 0x000000BA,
        RUSSIAN_CHARSET     = 0x000000CC,
        THAI_CHARSET        = 0x000000DE,
        EASTEUROPE_CHARSET  = 0x000000EE
    };

    /* Note: This enum is incomplete compared to the specification */
    /* [MS-EMF] - v20210625 - pages 32, 283 */
    enum ExtTextOutOptions : sal_uInt32
    {
        ETO_OPAQUE      = 0x0002,
        ETO_CLIPPED     = 0x0004,
        /* WINVER >= 0x0400 */
        ETO_GLYPH_INDEX = 0x0010,
        ETO_RTLREADING  = 0x0080,
        /* _WIN32_WINNT >= 0x0500 */
        ETO_NO_RECT     = 0x0100,
        ETO_PDY         = 0x2000
    };

    /* [MS-WMF] - v20210625 - pages 44, 96 */
    /* This is packed into a byte as 2 bits */
    enum PitchFont : sal_uInt8
    {
        DEFAULT_PITCH  = 0,
        FIXED_PITCH    = 1,
        VARIABLE_PITCH = 2
    };

    /* [MS-WMF] - v20210625 - pages 33,  */
    enum FamilyFont : sal_uInt8
    {
        FF_DONTCARE   = 0x00,
        FF_ROMAN      = 0x01,
        FF_SWISS      = 0x02,
        FF_MODERN     = 0x03,
        FF_SCRIPT     = 0x04,
        FF_DECORATIVE = 0x05
    };

    enum WeightFont
    {
        FW_THIN       = 100,
        FW_EXTRALIGHT = 200,
        FW_LIGHT      = 300,
        FW_NORMAL     = 400,
        FW_MEDIUM     = 500,
        FW_SEMIBOLD   = 600,
        FW_BOLD       = 700,
        FW_EXTRABOLD  = 800,
        FW_ULTRALIGHT = 200,
        FW_ULTRABOLD  = 800,
        FW_BLACK      = 900
    };

    /* [MS-WMF] - v20210625 - pages 29, 30, 182 */
    enum class BrushStyle : sal_uInt16
    {
        BS_SOLID         = 0,
        BS_NULL          = 1,
        BS_HOLLOW        = 1,
        BS_HATCHED       = 2,
        BS_PATTERN       = 3,
        BS_INDEXED       = 4,
        BS_DIBPATTERN    = 5,
        BS_DIBPATTERNPT  = 6,
        BS_PATTERN8X8    = 7,
        BS_DIBPATTERN8X8 = 8,
        BS_MONOPATTERN   = 9
    };

    constexpr sal_Int32 RDH_RECTANGLES = 1;
    constexpr sal_Int32 W_MFCOMMENT = 15;
    constexpr sal_Int32 PRIVATE_ESCAPE_UNICODE = 2;

    //Scalar constants
    constexpr sal_Int32 UNDOCUMENTED_WIN_RCL_RELATION = 32;
    constexpr sal_Int32 MS_FIXPOINT_BITCOUNT_28_4 = 4;
}

//============================ WmfReader ==================================

namespace emfio
{
    class WinMtfClipPath
    {
        basegfx::utils::B2DClipState maClip;

    public:
        WinMtfClipPath() : maClip() {};

        void        setClipPath(const basegfx::B2DPolyPolygon&, RegionMode nClippingMode);
        void        intersectClip(const basegfx::B2DPolyPolygon& rPolyPolygon);
        void        excludeClip(const basegfx::B2DPolyPolygon& rPolyPolygon);
        void        moveClipRegion(const Size& rSize);
        void        setDefaultClipPath();

        bool        isEmpty() const { return maClip.isCleared(); }

        basegfx::utils::B2DClipState const & getClip() const { return maClip; }
        basegfx::B2DPolyPolygon const & getClipPath() const;

        bool        operator==(const WinMtfClipPath& rPath) const
        {
            return maClip == rPath.maClip;
        };
    };

    class WinMtfPathObj : public tools::PolyPolygon
    {
        bool    bClosed;

    public:

        WinMtfPathObj() :
            bClosed(true)
        {}

        void        Init()
        {
            Clear();
            bClosed = true;
        }

        void        ClosePath();
        void        AddPoint(const Point& rPoint);
        void        AddPolygon(const tools::Polygon& rPoly);
        void        AddPolyLine(const tools::Polygon& rPoly);
        void        AddPolyPolygon(const tools::PolyPolygon& rPolyPolygon);
    };

    struct EMFIO_DLLPUBLIC GDIObj
    {
        GDIObj() = default;
        GDIObj(GDIObj const &) = default;
        virtual ~GDIObj() = default; // Polymorphic base class
        GDIObj & operator =(GDIObj const &) = default;
    };

    struct EMFIO_DLLPUBLIC WinMtfFontStyle final : GDIObj
    {
        vcl::Font    aFont;

        explicit WinMtfFontStyle(LOGFONTW const & rLogFont);

        ~WinMtfFontStyle() override;
    };

    enum class WinMtfFillStyleType
    {
        Solid, Pattern
    };

    struct WinMtfFillStyle final : GDIObj
    {
        Color               aFillColor;
        bool                bTransparent;
        WinMtfFillStyleType aType;
        Bitmap              aBmp;

        WinMtfFillStyle()
            : aFillColor(COL_BLACK)
            , bTransparent(false)
            , aType(WinMtfFillStyleType::Solid)
        {}

        WinMtfFillStyle(const Color& rColor, bool bTrans = false)
            : aFillColor(rColor)
            , bTransparent(bTrans)
            , aType(WinMtfFillStyleType::Solid)
        {}

        explicit WinMtfFillStyle(Bitmap const & rBmp)
            : bTransparent(false)
            , aType(WinMtfFillStyleType::Pattern)
            , aBmp(rBmp)
        {}

        bool operator==(const WinMtfFillStyle& rStyle) const
        {
            return aFillColor == rStyle.aFillColor
                && bTransparent == rStyle.bTransparent
                && aType == rStyle.aType;
        }
    };


    struct WinMtfPalette final : GDIObj
    {
        std::vector< Color > aPaletteColors;

        WinMtfPalette()
            : aPaletteColors(std::vector< Color >{})
        {}

        WinMtfPalette(std::vector<Color> paletteColors)
            : aPaletteColors(std::move(paletteColors))
        {}

    };


    struct WinMtfLineStyle final : GDIObj
    {
        Color       aLineColor;
        LineInfo    aLineInfo;
        bool        bTransparent;

        WinMtfLineStyle()
            : aLineColor(COL_BLACK)
            , bTransparent(false)
        {}

        WinMtfLineStyle(const Color& rColor, bool bTrans = false)
            : aLineColor(rColor)
            , bTransparent(bTrans)
        {}

        WinMtfLineStyle(const Color& rColor, const sal_uInt32 nStyle, const sal_Int32 nPenWidth)
            : aLineColor(rColor)
        {
            // According to documentation: nStyle = PS_COSMETIC = 0x0 - line with a width of one logical unit and a style that is a solid color
            // tdf#140271 Based on observed behaviour the line width is not constant with PS_COSMETIC

            // Width 0 means default width for LineInfo (HairLine) with 1 pixel wide
            aLineInfo.SetWidth(nPenWidth);
            switch (nStyle & PS_STYLE_MASK)
            {
                case PS_DASHDOTDOT:
                    aLineInfo.SetStyle(LineStyle::Dash);
                    aLineInfo.SetDashCount(1);
                    aLineInfo.SetDotCount(2);
                    break;
                case PS_DASHDOT:
                    aLineInfo.SetStyle(LineStyle::Dash);
                    aLineInfo.SetDashCount(1);
                    aLineInfo.SetDotCount(1);
                    break;
                case PS_DOT:
                    aLineInfo.SetStyle(LineStyle::Dash);
                    aLineInfo.SetDashCount(0);
                    aLineInfo.SetDotCount(1);
                    break;
                case PS_DASH:
                    aLineInfo.SetStyle(LineStyle::Dash);
                    aLineInfo.SetDashCount(1);
                    aLineInfo.SetDotCount(0);
                    break;
                case PS_NULL:
                    aLineInfo.SetStyle(LineStyle::NONE);
                    break;
                case PS_INSIDEFRAME: // TODO Implement PS_INSIDEFRAME
                case PS_SOLID:
                default:
                    aLineInfo.SetStyle(LineStyle::Solid);
            }
            if (nPenWidth)
                switch (nStyle & PS_ENDCAP_STYLE_MASK)
                {
                    case PS_ENDCAP_ROUND:
                        aLineInfo.SetLineCap(css::drawing::LineCap_ROUND);
                        break;
                    case PS_ENDCAP_SQUARE:
                        aLineInfo.SetLineCap(css::drawing::LineCap_SQUARE);
                        break;
                    case PS_ENDCAP_FLAT:
                    default:
                        aLineInfo.SetLineCap(css::drawing::LineCap_BUTT);
                }
            else
                aLineInfo.SetLineCap(css::drawing::LineCap_BUTT);
            switch (nStyle & PS_JOIN_STYLE_MASK)
            {
                case PS_JOIN_ROUND:
                    aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Round);
                    break;
                case PS_JOIN_BEVEL:
                    aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Bevel);
                    break;
                // Undocumented but based on experiments with MS Paint and MS Word,
                // the default Join Style is PS_JOIN_MITER
                case PS_JOIN_MITER:
                default:
                    aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Miter);
            }
            bTransparent = aLineInfo.GetStyle() == LineStyle::NONE;
        }

        bool operator==(const WinMtfLineStyle& rStyle) const
        {
            return aLineColor == rStyle.aLineColor
                && bTransparent == rStyle.bTransparent
                && aLineInfo == rStyle.aLineInfo;
        }
    };

    struct XForm
    {
        float   eM11;
        float   eM12;
        float   eM21;
        float   eM22;
        float   eDx;
        float   eDy;

        XForm()
            : eM11(1.0f)
            , eM12(0.0f)
            , eM21(0.0f)
            , eM22(1.0f)
            , eDx(0.0f)
            , eDy(0.0f)
        {}
    };

    SvStream& operator >> (SvStream& rInStream, XForm& rXForm);

    struct SaveStruct
    {
        BackgroundMode              nBkMode;
        MappingMode         eMapMode;
        GraphicsMode        eGfxMode;
        vcl::text::ComplexTextLayoutFlags nTextLayoutMode;
        sal_Int32           nWinOrgX, nWinOrgY, nWinExtX, nWinExtY;
        sal_Int32           nDevOrgX, nDevOrgY, nDevWidth, nDevHeight;

        WinMtfLineStyle     aLineStyle;
        WinMtfFillStyle     aFillStyle;

        vcl::Font           aFont;
        Color               aBkColor;
        Color               aTextColor;
        sal_uInt32          nTextAlign;
        RasterOp            eRasterOp;

        Point               aActPos;
        WinMtfPathObj       maPathObj;
        WinMtfClipPath      maClipPath;
        XForm               aXForm;

        bool                bClockWiseArcDirection;
        bool                bFillStyleSelected;
    };

    struct BSaveStruct
    {
        BitmapEx            aBmpEx;
        tools::Rectangle    aOutRect;
        sal_uInt32          nWinRop;
        bool m_bForceAlpha = false;

        BSaveStruct(const Bitmap& rBmp, const tools::Rectangle& rOutRect, sal_uInt32 nRop)
            : aBmpEx(rBmp)
            , aOutRect(rOutRect)
            , nWinRop(nRop)
        {}

        BSaveStruct(const BitmapEx& rBmpEx, const tools::Rectangle& rOutRect, sal_uInt32 nRop,
                    bool bForceAlpha = false)
            : aBmpEx(rBmpEx)
            , aOutRect(rOutRect)
            , nWinRop(nRop)
            , m_bForceAlpha(bForceAlpha)
        {}
    };

    // tdf#127471 implement detection and correction of wrongly scaled
    // fonts in own-written, old (before this fix) EMF/WMF files
    class ScaledFontDetectCorrectHelper
    {
    private:
        rtl::Reference<MetaFontAction>                                  maCurrentMetaFontAction;
        std::vector<double>                                             maAlternativeFontScales;
        std::vector<std::pair<rtl::Reference<MetaFontAction>, double>>  maPositiveIdentifiedCases;
        std::vector<std::pair<rtl::Reference<MetaFontAction>, double>>  maNegativeIdentifiedCases;

    public:
        ScaledFontDetectCorrectHelper();
        void endCurrentMetaFontAction();
        void newCurrentMetaFontAction(const rtl::Reference<MetaFontAction>& rNewMetaFontAction);
        void evaluateAlternativeFontScale(OUString const & rText, tools::Long nImportedTextLength);
        void applyAlternativeFontScale();
    };

    class MtfTools
    {
        MtfTools(MtfTools const &) = delete;
        MtfTools& operator =(MtfTools const &) = delete;

    protected:
        WinMtfPathObj       maPathObj;
        WinMtfClipPath      maClipPath;

        WinMtfLineStyle     maLatestLineStyle;
        WinMtfLineStyle     maLineStyle;
        WinMtfLineStyle     maNopLineStyle;
        WinMtfFillStyle     maLatestFillStyle;
        WinMtfFillStyle     maFillStyle;
        WinMtfFillStyle     maNopFillStyle;
        WinMtfPalette       maPalette;

        vcl::Font           maLatestFont;
        vcl::Font           maFont;
        sal_uInt32          mnLatestTextAlign;
        sal_uInt32          mnTextAlign;
        Color               maLatestTextColor;
        Color               maTextColor;
        Color               maLatestBkColor;
        Color               maBkColor;
        vcl::text::ComplexTextLayoutFlags  mnLatestTextLayoutMode;
        vcl::text::ComplexTextLayoutFlags  mnTextLayoutMode;
        BackgroundMode      mnLatestBkMode;
        BackgroundMode      mnBkMode;
        RasterOp            meLatestRasterOp;
        RasterOp            meRasterOp;

        std::vector< std::unique_ptr<GDIObj> > mvGDIObj;
        Point               maActPos;
        WMFRasterOp         mnRop;
        std::vector< std::shared_ptr<SaveStruct> > mvSaveStack;

        GraphicsMode        meGfxMode;
        MappingMode         meMapMode;

        XForm               maXForm;
        sal_Int32           mnDevOrgX;
        sal_Int32           mnDevOrgY;
        sal_Int32           mnDevWidth;
        sal_Int32           mnDevHeight;
        sal_Int32           mnWinOrgX;
        sal_Int32           mnWinOrgY;
        sal_Int32           mnWinExtX;
        sal_Int32           mnWinExtY;

        sal_Int32           mnPixX;            // Reference Device in pixel
        sal_Int32           mnPixY;            // Reference Device in pixel
        sal_Int32           mnMillX;           // Reference Device in Mill
        sal_Int32           mnMillY;           // Reference Device in Mill
        tools::Rectangle    mrclFrame;
        tools::Rectangle    mrclBounds;

        GDIMetaFile*        mpGDIMetaFile;

        SvStream*           mpInputStream;               // the WMF/EMF file to be read
        sal_uInt32          mnStartPos;
        sal_uInt32          mnEndPos;
        std::vector<BSaveStruct> maBmpSaveList;

        // tdf#127471 always try to detect - only used with ScaledText
        ScaledFontDetectCorrectHelper maScaledFontHelper;

        bool                mbNopMode : 1;
        bool                mbClockWiseArcDirection : 1;
        bool                mbFillStyleSelected : 1;
        bool                mbClipNeedsUpdate : 1;
        bool                mbComplexClip : 1;
        bool                mbIsMapWinSet : 1;
        bool                mbIsMapDevSet : 1;

        void                UpdateLineStyle();
        void                UpdateFillStyle();

        Point               ImplMap(const Point& rPt);
        Point               ImplScale(const Point& rPt);
        Size                ImplMap(const Size& rSize, bool bDoWorldTransform = true);
        tools::Rectangle    ImplMap(const tools::Rectangle& rRectangle);
        void                ImplMap(vcl::Font& rFont);
        tools::Polygon&     ImplMap(tools::Polygon& rPolygon);
        tools::PolyPolygon& ImplMap(tools::PolyPolygon& rPolyPolygon);
        void                ImplScale(tools::Polygon& rPolygon);
        tools::PolyPolygon& ImplScale(tools::PolyPolygon& rPolyPolygon);
        void                ImplResizeObjectArry(sal_uInt32 nNewEntry);
        void                ImplSetNonPersistentLineColorTransparenz();
        void                ImplDrawClippedPolyPolygon(const tools::PolyPolygon& rPolyPoly);
        void                ImplDrawBitmap(const Point& rPos, const Size& rSize, const BitmapEx& rBitmap);

    public:

        void                SetDevByWin(); //Hack to set varying defaults for incompletely defined files.
        void                SetDevOrg(const Point& rPoint);
        void                SetDevOrgOffset(sal_Int32 nXAdd, sal_Int32 nYAdd);
        void                SetDevExt(const Size& rSize, bool regular = true);
        void                ScaleDevExt(double fX, double fY);

        void                SetWinOrg(const Point& rPoint, bool bIsEMF = false);
        Point               GetWinOrg() { return Point(mnWinOrgX, mnWinOrgY); }
        void                SetWinOrgOffset(sal_Int32 nX, sal_Int32 nY);
        void                SetWinExt(const Size& rSize, bool bIsEMF = false);
        void                ScaleWinExt(double fX, double fY);

        void                SetrclBounds(const tools::Rectangle& rRect);
        void                SetrclFrame(const tools::Rectangle& rRect);
        void                SetRefPix(const Size& rSize);
        void                SetRefMill(const Size& rSize);

        void                SetMapMode(MappingMode mnMapMode);
        void                SetWorldTransform(const XForm& rXForm);
        void                ModifyWorldTransform(const XForm& rXForm, ModifyWorldTransformMode nMode);

        void                Push();
        void                Pop( const sal_Int32 nSavedDC = -1 );

        WMFRasterOp         SetRasterOp(WMFRasterOp nRasterOp);
        void                StrokeAndFillPath(bool bStroke, bool bFill);

        void                SetArcDirection(bool bCounterClockWise);
        bool                IsArcDirectionClockWise() { return mbClockWiseArcDirection; };
        void                SetGfxMode(GraphicsMode nGfxMode) { meGfxMode = nGfxMode; };
        GraphicsMode        GetGfxMode() const { return meGfxMode; };
        void                SetBkMode(BackgroundMode nMode);
        void                SetBkColor(const Color& rColor);
        void                SetTextColor(const Color& rColor);
        void                SetTextAlign(sal_uInt32 nAlign);

        void                CreateObject(std::unique_ptr<GDIObj> pObject);
        void                CreateObjectIndexed(sal_uInt32 nIndex, std::unique_ptr<GDIObj> pObject);
        void                CreateObject();

        void                DeleteObject(sal_uInt32 nIndex);
        void                SelectObject(sal_uInt32 nIndex);
        rtl_TextEncoding    GetCharSet() const { return maFont.GetCharSet(); };
        const vcl::Font&    GetFont() const { return maFont; }
        void                SetTextLayoutMode(vcl::text::ComplexTextLayoutFlags nLayoutMode);

        void                ClearPath() { maPathObj.Init(); };
        void                ClosePath() { maPathObj.ClosePath(); };
        const tools::PolyPolygon& GetPathObj() const { return maPathObj; };

        void                MoveTo(const Point& rPoint, bool bRecordPath = false);
        void                LineTo(const Point& rPoint, bool bRecordPath = false);
        void                DrawPixel(const Point& rSource, const Color& rColor);
        void                DrawRect(const tools::Rectangle& rRect, bool bEdge = true);
        void                DrawRectWithBGColor(const tools::Rectangle& rRect);
        void                DrawRoundRect(const tools::Rectangle& rRect, const Size& rSize);
        void                DrawEllipse(const tools::Rectangle& rRect);
        void                DrawArc(
            const tools::Rectangle& rRect,
            const Point& rStartAngle,
            const Point& rEndAngle,
            bool bDrawTo = false
        );
        void                DrawPie(
            const tools::Rectangle& rRect,
            const Point& rStartAngle,
            const Point& rEndAngle
        );
        void                DrawChord(
            const tools::Rectangle& rRect,
            const Point& rStartAngle,
            const Point& rEndAngle
        );
        void                DrawPolygon(tools::Polygon rPolygon, bool bRecordPath);
        void                DrawPolyPolygon(tools::PolyPolygon& rPolyPolygon, bool bRecordPath = false);
        void                DrawPolyLine(tools::Polygon rPolygon,
            bool bDrawTo = false,
            bool bRecordPath = false
        );
        void                DrawPolyBezier(tools::Polygon rPolygon,
            bool bDrawTo,
            bool bRecordPath
        );
        void                DrawText(Point& rPosition,
            OUString const & rString,
            KernArray* pDXArry = nullptr,
            tools::Long* pDYArry = nullptr,
            bool bRecordPath = false,
            GraphicsMode nGraphicsMode = GraphicsMode::GM_COMPATIBLE);

        void                ResolveBitmapActions(std::vector<BSaveStruct>& rSaveList);

        void                IntersectClipRect(const tools::Rectangle& rRect);
        void                ExcludeClipRect(const tools::Rectangle& rRect);
        void                MoveClipRegion(const Size& rSize);
        void                SetClipPath(
            const tools::PolyPolygon& rPolyPoly,
            RegionMode nClippingMode,
            bool bIsMapped
        );
        void                SetDefaultClipPath();
        void                UpdateClipRegion();
        void                AddFromGDIMetaFile(GDIMetaFile& rGDIMetaFile);

        void                PassEMFPlus(void const * pBuffer, sal_uInt32 nLength);
        void                PassEMFPlusHeaderInfo();

        Color               ReadColor();

        explicit            MtfTools(GDIMetaFile& rGDIMetaFile, SvStream& rStreamWMF);
        ~MtfTools() COVERITY_NOEXCEPT_FALSE;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
