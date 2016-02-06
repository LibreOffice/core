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

#ifndef INCLUDED_VCL_SOURCE_FILTER_WMF_WINMTF_HXX
#define INCLUDED_VCL_SOURCE_FILTER_WMF_WINMTF_HXX

#include <sot/object.hxx>
#include <vcl/graphic.hxx>
#include <basegfx/tools/b2dclipstate.hxx>
#include <vcl/font.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/fltcall.hxx>

#define ERROR                   0
#define NULLREGION              1
#define COMPLEXREGION           3

#define RGN_AND                 1
#define RGN_OR                  2
#define RGN_XOR                 3
#define RGN_DIFF                4
#define RGN_COPY                5

enum class BkMode
{
    NONE         = 0,
    Transparent  = 1,
    OPAQUE       = 2,
    LAST         = 2
};

/* xform stuff */
#define MWT_IDENTITY            1
#define MWT_LEFTMULTIPLY        2
#define MWT_RIGHTMULTIPLY       3
#define MWT_SET                 4

#define ENHMETA_STOCK_OBJECT    0x80000000

/* Stock Logical Objects */
#define WHITE_BRUSH             0
#define LTGRAY_BRUSH            1
#define GRAY_BRUSH              2
#define DKGRAY_BRUSH            3
#define BLACK_BRUSH             4
#define NULL_BRUSH              5
#define WHITE_PEN               6
#define BLACK_PEN               7
#define NULL_PEN                8
#define ANSI_FIXED_FONT         11
#define ANSI_VAR_FONT           12
#define SYSTEM_FIXED_FONT       16

#define R2_BLACK                1
#define R2_MASKNOTPEN           3
#define R2_NOT                  6
#define R2_XORPEN               7
#define R2_NOP                  11
#define R2_COPYPEN              13

/* Mapping modes */
#define MM_TEXT                 1
#define MM_LOMETRIC             2
#define MM_HIMETRIC             3
#define MM_LOENGLISH            4
#define MM_HIENGLISH            5
#define MM_TWIPS                6
#define MM_ISOTROPIC            7
#define MM_ANISOTROPIC          8

/* Graphics modes */
#define GM_COMPATIBLE           1
#define GM_ADVANCED             2

/* StretchBlt() modes */
#define BLACKONWHITE            1
#define WHITEONBLACK            2
#define COLORONCOLOR            3
#define HALFTONE                4
#define STRETCH_ANDSCANS        BLACKONWHITE
#define STRETCH_ORSCANS         WHITEONBLACK
#define STRETCH_DELETESCANS     COLORONCOLOR

#define LF_FACESIZE             32

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
};
struct WMF_EXTERNALHEADER;

#define TA_NOUPDATECP           0x0000
#define TA_UPDATECP             0x0001
#define TA_LEFT                 0x0000
#define TA_RIGHT                0x0002
#define TA_CENTER               0x0006
#define TA_RIGHT_CENTER  (TA_RIGHT | TA_CENTER)
#define TA_TOP                  0x0000
#define TA_BOTTOM               0x0008
#define TA_BASELINE             0x0018

#define SRCCOPY                 0x00CC0020L
#define SRCPAINT                0x00EE0086L
#define SRCAND                  0x008800C6L
#define SRCINVERT               0x00660046L
#define SRCERASE                0x00440328L
#define PATCOPY                 0x00F00021L
#define PATINVERT               0x005A0049L
#define BLACKNESS               0x00000042L
#define WHITENESS               0x00FF0062L

#define PS_SOLID                0
#define PS_DASH                 1
#define PS_DOT                  2
#define PS_DASHDOT              3
#define PS_DASHDOTDOT           4
#define PS_NULL                 5
#define PS_INSIDEFRAME          6
#define PS_STYLE_MASK           15

#define PS_ENDCAP_ROUND      0x000
#define PS_ENDCAP_SQUARE     0x100
#define PS_ENDCAP_FLAT       0x200
#define PS_ENDCAP_STYLE_MASK 0xF00

#define PS_JOIN_ROUND       0x0000
#define PS_JOIN_BEVEL       0x1000
#define PS_JOIN_MITER       0x2000
#define PS_JOIN_STYLE_MASK  0xF000

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define GB2312_CHARSET          134
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255
/*WINVER >= 0x0400*/
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204
#define MAC_CHARSET             77
#define BALTIC_CHARSET          186

#define ETO_CLIPPED             0x0004
/*WINVER >= 0x0400*/
#define ETO_GLYPH_INDEX         0x0010
#define ETO_RTLREADING          0x0080
/*_WIN32_WINNT >= 0x0500*/
#define ETO_PDY                 0x2000

#define DEFAULT_PITCH           0x00
#define FIXED_PITCH             0x01
#define VARIABLE_PITCH          0x02

/* Font Families */
#define FF_DONTCARE             0x00
#define FF_ROMAN                0x10
#define FF_SWISS                0x20
#define FF_MODERN               0x30
#define FF_SCRIPT               0x40
#define FF_DECORATIVE           0x50

#define FW_THIN                 100
#define FW_EXTRALIGHT           200
#define FW_LIGHT                300
#define FW_NORMAL               400
#define FW_MEDIUM               500
#define FW_SEMIBOLD             600
#define FW_BOLD                 700
#define FW_EXTRABOLD            800
#define FW_ULTRALIGHT           200
#define FW_ULTRABOLD            800
#define FW_BLACK                900

#define BS_SOLID                0
#define BS_NULL                 1
#define BS_HOLLOW               1
#define BS_HATCHED              2
#define BS_PATTERN              3
#define BS_INDEXED              4
#define BS_DIBPATTERN           5
#define BS_DIBPATTERNPT         6
#define BS_PATTERN8X8           7
#define BS_DIBPATTERN8X8        8
#define BS_MONOPATTERN          9


#define RDH_RECTANGLES  1

#define W_MFCOMMENT             15

#define PRIVATE_ESCAPE_UNICODE  2

//Scalar constants

#define UNDOCUMENTED_WIN_RCL_RELATION 32
#define MS_FIXPOINT_BITCOUNT_28_4 4
#define HUNDREDTH_MILLIMETERS_PER_MILLIINCH 2.54
#define MILLIINCH_PER_TWIPS   1.44

//============================ WMFReader ==================================

class WinMtfClipPath
{
    basegfx::tools::B2DClipState maClip;

public:
    WinMtfClipPath(): maClip() {};

    void        setClipPath( const tools::PolyPolygon& rPolyPolygon, sal_Int32 nClippingMode );
    void        intersectClipRect( const Rectangle& rRect );
    void        excludeClipRect( const Rectangle& rRect );
    void        moveClipRegion( const Size& rSize );
    void        setDefaultClipPath();

    bool        isEmpty() const { return maClip.isCleared(); }

    basegfx::B2DPolyPolygon getClipPath() const;

    bool        operator==( const WinMtfClipPath& rPath ) const
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
    void        AddPoint( const Point& rPoint );
    void        AddPolygon( const tools::Polygon& rPoly );
    void        AddPolyLine( const tools::Polygon& rPoly );
    void        AddPolyPolygon( const tools::PolyPolygon& rPolyPolygon );
};

struct WinMtfFontStyle
{
    vcl::Font    aFont;

    explicit WinMtfFontStyle( LOGFONTW& rLogFont );
};


typedef enum
{
    FillStyleSolid,
    FillStylePattern
} WinMtfFillStyleType;

struct WinMtfFillStyle
{
    Color               aFillColor;
    bool                bTransparent;
    WinMtfFillStyleType aType;
    Bitmap              aBmp;

    WinMtfFillStyle()
        : aFillColor(Color(COL_BLACK))
        , bTransparent(false)
        , aType(FillStyleSolid)
    {}

    WinMtfFillStyle(const Color& rColor, bool bTrans = false)
        : aFillColor(rColor)
        , bTransparent(bTrans)
        , aType(FillStyleSolid)
    {}

    explicit WinMtfFillStyle(Bitmap& rBmp)
        : bTransparent(false)
        , aType(FillStylePattern)
        , aBmp(rBmp)
    {}

    bool operator==( const WinMtfFillStyle& rStyle )
    {
        return aFillColor == rStyle.aFillColor
            && bTransparent == rStyle.bTransparent
            && aType == rStyle.aType;
    }

    bool operator==(WinMtfFillStyle* pStyle)
    {
        return aFillColor == pStyle->aFillColor
            && bTransparent == pStyle->bTransparent
            && aType == pStyle->aType;
    }

    WinMtfFillStyle& operator=(const WinMtfFillStyle& rStyle)
    {
        aFillColor = rStyle.aFillColor;
        bTransparent = rStyle.bTransparent;
        aBmp = rStyle.aBmp;
        aType = rStyle.aType;
        return *this;
    }

    WinMtfFillStyle& operator=(WinMtfFillStyle* pStyle)
    {
        aFillColor = pStyle->aFillColor;
        bTransparent = pStyle->bTransparent;
        aBmp = pStyle->aBmp;
        aType = pStyle->aType;
        return *this;
    }
};

struct WinMtfLineStyle
{
    Color       aLineColor;
    LineInfo    aLineInfo;
    bool        bTransparent;

    WinMtfLineStyle()
        : aLineColor  (COL_BLACK)
        , bTransparent(false)
    {}

    WinMtfLineStyle(const Color& rColor, bool bTrans = false)
        : aLineColor  (rColor)
        , bTransparent(bTrans)
    {}

    WinMtfLineStyle( const Color& rColor, const LineInfo& rStyle, bool bTrans = false)
        : aLineColor  (rColor)
        , aLineInfo   (rStyle)
        , bTransparent(bTrans)
    {}

    bool operator==( const WinMtfLineStyle& rStyle )
    {
        return aLineColor == rStyle.aLineColor
            && bTransparent == rStyle.bTransparent
            && aLineInfo == rStyle.aLineInfo;
    }

    bool operator==(WinMtfLineStyle* pStyle)
    {
        return aLineColor == pStyle->aLineColor
            && bTransparent == pStyle->bTransparent
            && aLineInfo == pStyle->aLineInfo;
    }

    WinMtfLineStyle& operator=( const WinMtfLineStyle& rStyle )
    {
        aLineColor = rStyle.aLineColor;
        bTransparent = rStyle.bTransparent;
        aLineInfo = rStyle.aLineInfo;
        return *this;
    }

    WinMtfLineStyle& operator=( WinMtfLineStyle* pStyle )
    {
        aLineColor = pStyle->aLineColor;
        bTransparent = pStyle->bTransparent;
        aLineInfo = pStyle->aLineInfo;
        return *this;
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

struct SaveStruct
{
    BkMode              nBkMode;
    sal_uInt32          nMapMode, nGfxMode;
    ComplexTextLayoutMode nTextLayoutMode;
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
    WinMtfPathObj       aPathObj;
    WinMtfClipPath      aClipPath;
    XForm               aXForm;

    bool                bFillStyleSelected;
};

typedef std::shared_ptr<SaveStruct> SaveStructPtr;

struct BSaveStruct
{
    BitmapEx        aBmpEx;
    Rectangle       aOutRect;
    sal_uInt32      nWinRop;
    WinMtfFillStyle aStyle;

    BSaveStruct(const Bitmap& rBmp, const Rectangle& rOutRect,
                sal_uInt32 nRop, WinMtfFillStyle& rStyle)
        : aBmpEx(rBmp)
        , aOutRect(rOutRect)
        , nWinRop(nRop)
        , aStyle (rStyle)
    {}

    BSaveStruct(const BitmapEx& rBmpEx, const Rectangle& rOutRect,
                sal_uInt32 nRop, WinMtfFillStyle& rStyle)
        : aBmpEx(rBmpEx)
        , aOutRect(rOutRect)
        , nWinRop(nRop)
        , aStyle (rStyle)
    {}
};

typedef std::vector<std::unique_ptr<BSaveStruct>> BSaveStructList_impl;

enum GDIObjectType
{
    GDI_DUMMY = 0,
    GDI_PEN = 1,
    GDI_BRUSH = 2,
    GDI_FONT = 3,
    GDI_PALETTE = 4,
    GDI_BITMAP = 5,
    GDI_REGION = 6
};

struct GDIObj
{
    void*           pStyle;
    GDIObjectType   eType;

    GDIObj()
        : pStyle (nullptr)
        , eType  (GDI_DUMMY)
    {}

    GDIObj(GDIObjectType eT, void* pS)
    {
        pStyle = pS;
        eType = eT;
    }

    void Set(GDIObjectType eT, void* pS)
    {
        pStyle = pS;
        eType = eT;
    }

    void Delete()
    {
        if (pStyle == nullptr)
            return;

        switch (eType)
        {
            case GDI_PEN :
                delete static_cast<WinMtfLineStyle*>(pStyle);
            break;
            case GDI_BRUSH :
                delete static_cast<WinMtfFillStyle*>(pStyle);
            break;
            case GDI_FONT :
                delete static_cast<WinMtfFontStyle*>(pStyle);
            break;

            default:
                OSL_FAIL( "unsupported style deleted" );
                break;
        }
        pStyle = nullptr;
    }

    ~GDIObj()
    {
        Delete();
    }
};

class WinMtfOutput
{
    WinMtfPathObj       aPathObj;
    WinMtfClipPath      aClipPath;

    WinMtfLineStyle     maLatestLineStyle;
    WinMtfLineStyle     maLineStyle;
    WinMtfLineStyle     m_NopLineStyle;
    WinMtfFillStyle     maLatestFillStyle;
    WinMtfFillStyle     maFillStyle;
    WinMtfFillStyle     m_NopFillStyle;
    vcl::Font           maLatestFont;
    vcl::Font           maFont;
    sal_uInt32          mnLatestTextAlign;
    sal_uInt32          mnTextAlign;
    Color               maLatestTextColor;
    Color               maTextColor;
    Color               maLatestBkColor;
    Color               maBkColor;
    ComplexTextLayoutMode  mnLatestTextLayoutMode;
    ComplexTextLayoutMode  mnTextLayoutMode;
    BkMode              mnLatestBkMode;
    BkMode              mnBkMode;
    RasterOp            meLatestRasterOp;
    RasterOp            meRasterOp;

    std::vector< GDIObj* > vGDIObj;

    Point               maActPos;

    sal_uInt32          mnRop;
    bool            mbNopMode;
    bool            mbFillStyleSelected;
    bool            mbClipNeedsUpdate;
    bool            mbComplexClip;

    std::vector< SaveStructPtr > vSaveStack;

    sal_uInt32          mnGfxMode;
    sal_uInt32          mnMapMode;

    XForm               maXForm;
    sal_Int32           mnDevOrgX, mnDevOrgY;
    sal_Int32           mnDevWidth, mnDevHeight;
    sal_Int32           mnWinOrgX, mnWinOrgY;       // aktuel window origin
    sal_Int32           mnWinExtX, mnWinExtY;       // aktuel window extend
    bool            mbIsMapWinSet;
    bool            mbIsMapDevSet;

    sal_Int32           mnPixX, mnPixY;             // Reference Device in pixel
    sal_Int32           mnMillX, mnMillY;           // Reference Device in Mill
    Rectangle           mrclFrame;                  // rectangle in logical units 1/100th mm
    Rectangle           mrclBounds;

    GDIMetaFile*        mpGDIMetaFile;

    void                UpdateLineStyle();
    void                UpdateFillStyle();

    Point               ImplMap( const Point& rPt );
    Point               ImplScale( const Point& rPt );
    Size                ImplMap( const Size& rSize, bool bDoWorldTransform = true);
    Rectangle           ImplMap( const Rectangle& rRectangle );
    void                ImplMap( vcl::Font& rFont );
    tools::Polygon&     ImplMap( tools::Polygon& rPolygon );
    tools::PolyPolygon& ImplMap( tools::PolyPolygon& rPolyPolygon );
    void                ImplScale( tools::Polygon& rPolygon );
    tools::PolyPolygon& ImplScale( tools::PolyPolygon& rPolyPolygon );
    void                ImplResizeObjectArry( sal_uInt32 nNewEntry );
    void                ImplSetNonPersistentLineColorTransparenz();
    void                ImplDrawClippedPolyPolygon( const tools::PolyPolygon& rPolyPoly );
    void                ImplDrawBitmap( const Point& rPos, const Size& rSize, const BitmapEx& rBitmap );

public:

    void                SetDevByWin(); //Hack to set varying defaults for incompletely defined files.
    void                SetDevOrg( const Point& rPoint );
    void                SetDevOrgOffset( sal_Int32 nXAdd, sal_Int32 nYAdd );
    void                SetDevExt( const Size& rSize ,bool regular = true);
    void                ScaleDevExt( double fX, double fY );

    void                SetWinOrg( const Point& rPoint , bool bIsEMF = false);
    void                SetWinOrgOffset( sal_Int32 nX, sal_Int32 nY );
    void                SetWinExt( const Size& rSize , bool bIsEMF = false);
    void                ScaleWinExt( double fX, double fY );

    void                SetrclBounds( const Rectangle& rRect );
    void                SetrclFrame( const Rectangle& rRect );
    void                SetRefPix( const Size& rSize );
    void                SetRefMill( const Size& rSize );

    void                SetMapMode( sal_uInt32 mnMapMode );
    void                SetWorldTransform( const XForm& rXForm );
    void                ModifyWorldTransform( const XForm& rXForm, sal_uInt32 nMode );

    void                Push();
    void                Pop();

    sal_uInt32          SetRasterOp( sal_uInt32 nRasterOp );
    void                StrokeAndFillPath( bool bStroke, bool bFill );

    void                SetGfxMode( sal_Int32 nGfxMode ){ mnGfxMode = nGfxMode; };
    sal_Int32           GetGfxMode() const { return mnGfxMode; };
    void                SetBkMode( BkMode nMode );
    void                SetBkColor( const Color& rColor );
    void                SetTextColor( const Color& rColor );
    void                SetTextAlign( sal_uInt32 nAlign );
    void                CreateObject( GDIObjectType, void* pStyle = nullptr );
    void                CreateObject( sal_Int32 nIndex, GDIObjectType, void* pStyle = nullptr );
    void                DeleteObject( sal_Int32 nIndex );
    void                SelectObject( sal_Int32 nIndex );
    rtl_TextEncoding    GetCharSet(){ return maFont.GetCharSet(); };
    WinMtfFillStyle&    GetFillStyle () { return maFillStyle; }
    const vcl::Font&    GetFont() const { return maFont;}
    void                SetTextLayoutMode( ComplexTextLayoutMode nLayoutMode );

    void                ClearPath(){ aPathObj.Init(); };
    void                ClosePath(){ aPathObj.ClosePath(); };
    const tools::PolyPolygon& GetPathObj(){ return aPathObj; };

    void                MoveTo( const Point& rPoint, bool bRecordPath = false );
    void                LineTo( const Point& rPoint, bool bRecordPath = false );
    void                DrawPixel( const Point& rSource, const Color& rColor );
    void                DrawRect( const Rectangle& rRect, bool bEdge = true );
    void                DrawRoundRect( const Rectangle& rRect, const Size& rSize );
    void                DrawEllipse( const Rectangle& rRect );
    void                DrawArc(
                            const Rectangle& rRect,
                            const Point& rStartAngle,
                            const Point& rEndAngle,
                            bool bDrawTo = false
                        );
    void                DrawPie(
                            const Rectangle& rRect,
                            const Point& rStartAngle,
                            const Point& rEndAngle
                        );
    void                DrawChord(
                            const Rectangle& rRect,
                            const Point& rStartAngle,
                            const Point& rEndAngle
                        );
    void                DrawPolygon( tools::Polygon& rPolygon, bool bRecordPath = false );
    void                DrawPolygon( tools::Polygon& rPolygon, bool /*bDrawTo*/, bool bRecordPath)
                        {
                            //For ReadAndDrawPolygon template compatibility
                            DrawPolygon(rPolygon, bRecordPath);
                        }
    void                DrawPolyPolygon( tools::PolyPolygon& rPolyPolygon, bool bRecordPath = false );
    void                DrawPolyLine( tools::Polygon& rPolygon,
                                      bool bDrawTo = false,
                                      bool bRecordPath = false
                        );
    void                DrawPolyBezier( tools::Polygon& rPolygin,
                                        bool bDrawTo = false,
                                        bool bRecordPath = false
                        );
    void                DrawText( Point& rPosition,
                                  OUString& rString,
                                  long* pDXArry = nullptr,
                                  bool bRecordPath = false,
                                  sal_Int32 nGraphicsMode = GM_COMPATIBLE);

    void                ResolveBitmapActions( BSaveStructList_impl& rSaveList );

    void                IntersectClipRect( const Rectangle& rRect );
    void                ExcludeClipRect( const Rectangle& rRect );
    void                MoveClipRegion( const Size& rSize );
    void                SetClipPath(
                            const tools::PolyPolygon& rPolyPoly,
                            sal_Int32 nClippingMode,
                            bool bIsMapped
                        );
    void                SetDefaultClipPath();
    void                UpdateClipRegion();
    void                AddFromGDIMetaFile( GDIMetaFile& rGDIMetaFile );

    void                PassEMFPlus( void* pBuffer, sal_uInt32 nLength );
    void                PassEMFPlusHeaderInfo();

    explicit            WinMtfOutput( GDIMetaFile& rGDIMetaFile );
    virtual             ~WinMtfOutput();
};

class WinMtf
{
protected:

    WinMtfOutput*           pOut;
    SvStream*               pWMF;               // the WMF/EMF file to be read

    sal_uInt32              nStartPos, nEndPos;
    BSaveStructList_impl    aBmpSaveList;

    FilterConfigItem*   pFilterConfigItem;

    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

    // assures aSampledBrush is the actual brush of the GDIMetaFile

    Color               ReadColor();
    void                Callback( sal_uInt16 nPercent );

                        WinMtf(
                            WinMtfOutput* pOut,
                            SvStream& rStreamWMF,
                            FilterConfigItem* pConfigItem = nullptr
                        );
                        ~WinMtf();
};

class EnhWMFReader : public WinMtf
{
    bool        bRecordPath;
    sal_Int32   nRecordCount;
    bool        bEMFPlus;

    bool        ReadHeader();
                    // reads and converts the rectangle
    static Rectangle ReadRectangle( sal_Int32, sal_Int32, sal_Int32, sal_Int32 );

public:
    EnhWMFReader(SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile, FilterConfigItem* pConfigItem = nullptr);
    ~EnhWMFReader();

    bool ReadEnhWMF();
    void ReadEMFPlusComment(sal_uInt32 length, bool& bHaveDC);
private:
    template <class T> void ReadAndDrawPolyPolygon();
    template <class T> void ReadAndDrawPolyLine();
    template <class T> tools::Polygon ReadPolygon(sal_uInt32 nStartIndex, sal_uInt32 nPoints);
    template <class T, class Drawer> void ReadAndDrawPolygon(Drawer drawer, const bool skipFirst);

    Rectangle ReadRectangle();
};

class WMFReader : public WinMtf
{
private:

    sal_uInt16      nUnitsPerInch;
    sal_uInt32      nRecSize;

    // embedded EMF data
    SvMemoryStream* pEMFStream;

    // total number of comment records containing EMF data
    sal_uInt32      nEMFRecCount;

    // number of EMF records read
    sal_uInt32      nEMFRec;

    // total size of embedded EMF data
    sal_uInt32      nEMFSize;

    sal_uInt32      nSkipActions;
    sal_uInt32      nCurrentAction;
    sal_uInt32      nUnicodeEscapeAction;

    WMF_EXTERNALHEADER* pExternalHeader;

    // reads header of the WMF-Datei
    bool        ReadHeader();

    // reads parameters of the record with the functionnumber nFunction.
    void            ReadRecordParams( sal_uInt16 nFunction );

    Point           ReadPoint();                // reads and converts a point (first X then Y)
    Point           ReadYX();                   // reads and converts a point (first Y then X)
    Rectangle       ReadRectangle();            // reads and converts a rectangle
    Size            ReadYXExt();
    bool        GetPlaceableBound( Rectangle& rSize, SvStream* pStrm );

public:

    WMFReader(SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile,
              FilterConfigItem* pConfigItem = nullptr,
              WMF_EXTERNALHEADER* pExtHeader = nullptr);
    ~WMFReader();

    // read WMF file from stream and fill the GDIMetaFile
    void ReadWMF();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
