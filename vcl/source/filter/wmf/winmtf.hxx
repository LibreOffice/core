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

#ifndef _WINMTF_HXX
#define _WINMTF_HXX

#ifdef DBG_UTIL
#define WIN_MTF_ASSERT
#endif

#include <sot/object.hxx>
#include <boost/shared_ptr.hpp>
#include <vcl/graph.hxx>
#include <basegfx/tools/b2dclipstate.hxx>
#include <vcl/font.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/fltcall.hxx>

#define ERROR                   0
#define NULLREGION              1
#define SIMPLEREGION            2
#define COMPLEXREGION           3

#define RGN_AND                 1
#define RGN_OR                  2
#define RGN_XOR                 3
#define RGN_DIFF                4
#define RGN_COPY                5

#define TRANSPARENT             1
#define OPAQUE                  2
#define BKMODE_LAST             2

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
#define HOLLOW_BRUSH            NULL_BRUSH
#define WHITE_PEN               6
#define BLACK_PEN               7
#define NULL_PEN                8
#define OEM_FIXED_FONT          10
#define ANSI_FIXED_FONT         11
#define ANSI_VAR_FONT           12
#define SYSTEM_FONT             13
#define DEVICE_DEFAULT_FONT     14
#define DEFAULT_PALETTE         15
#define SYSTEM_FIXED_FONT       16


#define R2_BLACK                1
#define R2_NOTMERGEPEN          2
#define R2_MASKNOTPEN           3
#define R2_NOTCOPYPEN           4
#define R2_MASKPENNOT           5
#define R2_NOT                  6
#define R2_XORPEN               7
#define R2_NOTMASKPEN           8
#define R2_MASKPEN              9
#define R2_NOTXORPEN            10
#define R2_NOP                  11
#define R2_MERGENOTPEN          12
#define R2_COPYPEN              13
#define R2_MERGEPENNOT          14
#define R2_MERGEPEN             15
#define R2_WHITE                16

/* Mapping Modes */
#define MM_TEXT                 1
#define MM_LOMETRIC             2
#define MM_HIMETRIC             3
#define MM_LOENGLISH            4
#define MM_HIENGLISH            5
#define MM_TWIPS                6
#define MM_ISOTROPIC            7
#define MM_ANISOTROPIC          8


/* Graphics Modes */
#define GM_COMPATIBLE           1
#define GM_ADVANCED             2
#define GM_LAST                 2

/* StretchBlt() Modes */
#define BLACKONWHITE            1
#define WHITEONBLACK            2
#define COLORONCOLOR            3
#define HALFTONE                4
#define MAXSTRETCHBLTMODE       4
#define STRETCH_ANDSCANS        BLACKONWHITE
#define STRETCH_ORSCANS         WHITEONBLACK
#define STRETCH_DELETESCANS     COLORONCOLOR
#define STRETCH_HALFTONE        HALFTONE

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
#define NOTSRCCOPY              0x00330008L
#define NOTSRCERASE             0x001100A6L
#define MERGECOPY               0x00C000CAL
#define MERGEPAINT              0x00BB0226L
#define PATCOPY                 0x00F00021L
#define PATPAINT                0x00FB0A09L
#define PATINVERT               0x005A0049L
#define DSTINVERT               0x00550009L
#define BLACKNESS               0x00000042L
#define WHITENESS               0x00FF0062L

#define PS_SOLID                0
#define PS_DASH                 1
#define PS_DOT                  2
#define PS_DASHDOT              3
#define PS_DASHDOTDOT           4
#define PS_NULL                 5
#define PS_INSIDEFRAME          6
#define PS_USERSTYLE            7
#define PS_ALTERNATE            8
#define PS_STYLE_MASK           15

#define PS_ENDCAP_ROUND     0x000
#define PS_ENDCAP_SQUARE    0x100
#define PS_ENDCAP_FLAT      0x200
#define PS_ENDCAP_MASK      0xF00

#define PS_JOIN_ROUND       0x0000
#define PS_JOIN_BEVEL       0x1000
#define PS_JOIN_MITER       0x2000
#define PS_JOIN_MASK        0xF000

#define PS_COSMETIC         0x00000
#define PS_GEOMETRIC        0x10000
#define PS_TYPE_MASK        0xF0000

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

#define ETO_OPAQUE              0x0002
#define ETO_CLIPPED             0x0004
/*WINVER >= 0x0400*/
#define ETO_GLYPH_INDEX         0x0010
#define ETO_RTLREADING          0x0080
#define ETO_NUMERICSLOCAL       0x0400
#define ETO_NUMERICSLATIN       0x0800
#define ETO_IGNORELANGUAGE      0x1000
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

#define FW_DONTCARE             0
#define FW_THIN                 100
#define FW_EXTRALIGHT           200
#define FW_LIGHT                300
#define FW_NORMAL               400
#define FW_MEDIUM               500
#define FW_SEMIBOLD             600
#define FW_BOLD                 700
#define FW_EXTRABOLD            800
#define FW_HEAVY                900
#define FW_ULTRALIGHT           200
#define FW_REGULAR              400
#define FW_DEMIBOLD             600
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

#define W_HS_HORIZONTAL         0
#define W_HS_VERTICAL           1
#define W_HS_FDIAGONAL          2
#define W_HS_BDIAGONAL          3
#define W_HS_CROSS              4
#define W_HS_DIAGCROSS          5

#define RDH_RECTANGLES  1

#define W_MFCOMMENT             15

#define PRIVATE_ESCAPE_UNICODE  2

//Scalar constants

#define UNDOCUMENTED_WIN_RCL_RELATION 32
#define MS_FIXPOINT_BITCOUNT_28_4 4
#define HUNDREDTH_MILLIMETERS_PER_MILLIINCH 2.54
#define MILLIINCH_PER_TWIPS   1.44

//============================ WMFReader ==================================

#ifdef WIN_MTF_ASSERT
#define WIN_MTF_ASSERT_INIT     0x80000000
#define WIN_MTF_ASSERT_ONCE     0x40000000
#define WIN_MTF_ASSERT_MIFE     0x20000000

void WinMtfAssertHandler( const sal_Char*, sal_uInt32 nFlags = WIN_MTF_ASSERT_MIFE );
#else
inline void WinMtfAssertHandler( const sal_Char*, sal_uInt32 = 0 ) {}
#endif

class WinMtfClipPath
{
    basegfx::tools::B2DClipState maClip;

public :
    WinMtfClipPath(): maClip() {};

    void        setClipPath( const PolyPolygon& rPolyPolygon, sal_Int32 nClippingMode );
    void        intersectClipRect( const Rectangle& rRect );
    void        excludeClipRect( const Rectangle& rRect );
    void        moveClipRegion( const Size& rSize );

    bool        isEmpty() const { return maClip.isCleared(); }

    basegfx::B2DPolyPolygon getClipPath() const;

    bool        operator==( const WinMtfClipPath& rPath ) const
                {
                    return maClip == rPath.maClip;
                };
};

class WinMtfPathObj : public PolyPolygon
{
    sal_Bool    bClosed;

public :

                WinMtfPathObj() { bClosed = sal_True; }
    void        Init() { Clear(); bClosed = sal_True; };
    void        ClosePath();
    void        AddPoint( const Point& rPoint );
    void        AddPolygon( const Polygon& rPoly );
    void        AddPolyLine( const Polygon& rPoly );
    void        AddPolyPolygon( const PolyPolygon& rPolyPolygon );
};

struct WinMtfFontStyle
{
    Font    aFont;

    WinMtfFontStyle( LOGFONTW& rLogFont );
};

// -----------------------------------------------------------------------------

typedef enum {
    FillStyleSolid,
    FillStylePattern
} WinMtfFillStyleType;

struct WinMtfFillStyle
{
    Color               aFillColor;
    sal_Bool            bTransparent;
    WinMtfFillStyleType aType;
    Bitmap              aBmp;

    WinMtfFillStyle()
        : aFillColor(Color(COL_BLACK))
        , bTransparent(sal_False)
        , aType(FillStyleSolid)
    {
    }

    WinMtfFillStyle( const Color& rColor, sal_Bool bTrans = sal_False )
        : aFillColor(rColor)
        , bTransparent(bTrans)
        , aType(FillStyleSolid)
    {
    }

    WinMtfFillStyle(Bitmap& rBmp)
        : aType(FillStylePattern)
        , aBmp(rBmp)
    {
    }

    sal_Bool operator==( const WinMtfFillStyle& rStyle )
        {
            return (  ( aFillColor == rStyle.aFillColor )
                   && ( bTransparent == rStyle.bTransparent )
                   && ( aType == rStyle.aType )
                   );
        }
    sal_Bool operator==( WinMtfFillStyle* pStyle )
        {
            return (  ( aFillColor == pStyle->aFillColor )
                   && ( bTransparent == pStyle->bTransparent )
                   && ( aType == pStyle->aType )
                   );
        }
    WinMtfFillStyle& operator=( const WinMtfFillStyle& rStyle )
        {
            aFillColor = rStyle.aFillColor;
            bTransparent = rStyle.bTransparent;
            aBmp = rStyle.aBmp;
            aType = rStyle.aType;
            return *this;
        }
    WinMtfFillStyle& operator=( WinMtfFillStyle* pStyle )
        {
            aFillColor = pStyle->aFillColor;
            bTransparent = pStyle->bTransparent;
            aBmp = pStyle->aBmp;
            aType = pStyle->aType;
            return *this;
        }
};

// -----------------------------------------------------------------------------

struct WinMtfLineStyle
{
    Color       aLineColor;
    LineInfo    aLineInfo;
    sal_Bool        bTransparent;

    WinMtfLineStyle() :
        aLineColor  ( COL_BLACK ),
        bTransparent( sal_False ) {}

    WinMtfLineStyle( const Color& rColor, sal_Bool bTrans = sal_False ) :
        aLineColor  ( rColor ),
        bTransparent( bTrans ) {}

    WinMtfLineStyle( const Color& rColor, const LineInfo& rStyle, sal_Bool bTrans = sal_False ) :
        aLineColor  ( rColor ),
        aLineInfo   ( rStyle ),
        bTransparent( bTrans ) {}

    sal_Bool operator==( const WinMtfLineStyle& rStyle )
        {
            return (  ( aLineColor == rStyle.aLineColor )
                   && ( bTransparent == rStyle.bTransparent )
                   && ( aLineInfo == rStyle.aLineInfo )
                   );
        }
    sal_Bool operator==( WinMtfLineStyle* pStyle )
        {
            return (  ( aLineColor == pStyle->aLineColor )
                   && ( bTransparent == pStyle->bTransparent )
                   && ( aLineInfo == pStyle->aLineInfo )
                   );
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

// -----------------------------------------------------------------------------

struct XForm
{
    float   eM11;
    float   eM12;
    float   eM21;
    float   eM22;
    float   eDx;
    float   eDy;
    XForm()
    {
        eM11 =  eM22 = 1.0f;
        eDx = eDy = eM12 = eM21 = 0.0f;
    }

    friend SvStream& operator>>( SvStream& rIn, XForm& rXForm );
};

// -----------------------------------------------------------------------------

struct SaveStruct
{
    sal_uInt32          nBkMode, nMapMode, nGfxMode, nTextLayoutMode;
    sal_Int32           nWinOrgX, nWinOrgY, nWinExtX, nWinExtY;
    sal_Int32           nDevOrgX, nDevOrgY, nDevWidth, nDevHeight;

    WinMtfLineStyle     aLineStyle;
    WinMtfFillStyle     aFillStyle;

    Font                aFont;
    Color               aBkColor;
    Color               aTextColor;
    sal_uInt32          nTextAlign;
    RasterOp            eRasterOp;

    Point               aActPos;
    WinMtfPathObj       aPathObj;
    WinMtfClipPath      aClipPath;
    XForm               aXForm;

    sal_Bool            bRecordPath;
    sal_Bool            bFillStyleSelected;
};

typedef ::boost::shared_ptr< SaveStruct > SaveStructPtr;

// -----------------------------------------------------------------------------

struct BSaveStruct
{
    Bitmap          aBmp;
    Rectangle       aOutRect;
    sal_uInt32      nWinRop;
    WinMtfFillStyle aStyle;

        BSaveStruct(
            const Bitmap& rBmp,
            const Rectangle& rOutRect,
            sal_uInt32 nRop,
            WinMtfFillStyle& rStyle
        )
            : aBmp( rBmp )
            , aOutRect( rOutRect )
            , nWinRop( nRop )
            , aStyle ( rStyle )
        {}
};

typedef ::std::vector< BSaveStruct* > BSaveStructList_impl;

// -----------------------------------------------------------------------------

enum GDIObjectType {
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

    GDIObj() :
        pStyle  ( NULL ),
        eType   ( GDI_DUMMY )
    {
    }

    GDIObj( GDIObjectType eT, void* pS ) { pStyle = pS; eType = eT; }
    void Set( GDIObjectType eT, void* pS ) { pStyle = pS; eType = eT; }
    void Delete()
    {
        if ( pStyle )
        {
            switch ( eType )
            {
                case GDI_PEN :
                    delete (WinMtfLineStyle*)pStyle;
                break;
                case GDI_BRUSH :
                    delete (WinMtfFillStyle*)pStyle;
                break;
                case GDI_FONT :
                    delete (WinMtfFontStyle*)pStyle;
                break;

                default:
                    OSL_FAIL( "unsupported style deleted" );
                    break;
            }
            pStyle = NULL;
        }
    }

    ~GDIObj()
    {
        Delete();
    }
};

// -----------------------------------------------------------------------------

class WinMtfOutput
{

    WinMtfPathObj       aPathObj;
    WinMtfClipPath      aClipPath;

    WinMtfLineStyle     maLatestLineStyle;
    WinMtfLineStyle     maLineStyle;
    WinMtfFillStyle     maLatestFillStyle;
    WinMtfFillStyle     maFillStyle;
    Font                maLatestFont;
    Font                maFont;
    sal_uInt32          mnLatestTextAlign;
    sal_uInt32          mnTextAlign;
    Color               maLatestTextColor;
    Color               maTextColor;
    Color               maLatestBkColor;
    Color               maBkColor;
    sal_uInt32          mnLatestTextLayoutMode;
    sal_uInt32          mnTextLayoutMode;
    sal_uInt32          mnLatestBkMode;
    sal_uInt32          mnBkMode;
    RasterOp            meLatestRasterOp;
    RasterOp            meRasterOp;

    std::vector< GDIObj* > vGDIObj;

    Point               maActPos;

    sal_uInt32          mnRop;
    sal_Bool            mbNopMode;
    sal_Bool            mbFillStyleSelected;
    sal_Bool            mbClipNeedsUpdate;
    sal_Bool            mbComplexClip;

    std::vector< SaveStructPtr > vSaveStack;

    sal_uInt32          mnGfxMode;
    sal_uInt32          mnMapMode;
    sal_uInt16          mnUnitsPerInch;

    XForm               maXForm;
    sal_Int32           mnDevOrgX, mnDevOrgY;
    sal_Int32           mnDevWidth, mnDevHeight;
    sal_Int32           mnWinOrgX, mnWinOrgY;       // aktuelles Window-Origin
    sal_Int32           mnWinExtX, mnWinExtY;       // aktuelles Window-Extent
    sal_Bool            mbIsMapWinSet;
    sal_Bool            mbIsMapDevSet;

    sal_Int32           mnPixX, mnPixY;             // Reference Device in pixel
    sal_Int32           mnMillX, mnMillY;           // Reference Device in Mill
    Rectangle           mrclFrame;                  // rectangle in logical units 1/100th mm
    Rectangle           mrclBounds;

    GDIMetaFile*        mpGDIMetaFile;

    void                UpdateLineStyle();
    void                UpdateFillStyle();

    Point               ImplMap( const Point& rPt );
    Point               ImplScale( const Point& rPt );
    Size                ImplMap( const Size& rSz );
    Rectangle           ImplMap( const Rectangle& rRectangle );
    void                ImplMap( Font& rFont );
    Polygon&            ImplMap( Polygon& rPolygon );
    PolyPolygon&        ImplMap( PolyPolygon& rPolyPolygon );
    Polygon&            ImplScale( Polygon& rPolygon );
    PolyPolygon&        ImplScale( PolyPolygon& rPolyPolygon );
    void                ImplResizeObjectArry( sal_uInt32 nNewEntry );
    void                ImplSetNonPersistentLineColorTransparenz();
    void                ImplDrawClippedPolyPolygon( const PolyPolygon& rPolyPoly );
    void                ImplDrawBitmap( const Point& rPos, const Size& rSize, const BitmapEx rBitmap );

public:

    void                SetDevByWin(); //Hack to set varying defaults for incompletely defined files.
    void                SetDevOrg( const Point& rPoint );
    void                SetDevOrgOffset( sal_Int32 nXAdd, sal_Int32 nYAdd );
    void                SetDevExt( const Size& rSize ,sal_Bool regular = true);
    void                ScaleDevExt( double fX, double fY );

    void                SetWinOrg( const Point& rPoint , sal_Bool bIsEMF = false);
    void                SetWinOrgOffset( sal_Int32 nX, sal_Int32 nY );
    void                SetWinExt( const Size& rSize , sal_Bool bIsEMF = false);
    void                ScaleWinExt( double fX, double fY );

    void                SetrclBounds( const Rectangle& rRect );
    void                SetrclFrame( const Rectangle& rRect );
    void                SetRefPix( const Size& rSize );
    void                SetRefMill( const Size& rSize );

    sal_uInt32          GetMapMode() const { return mnMapMode; };
    void                SetMapMode( sal_uInt32 mnMapMode );
    void                SetUnitsPerInch( sal_uInt16 nUnitsPerInch );
    void                SetWorldTransform( const XForm& rXForm );
    const XForm&        GetWorldTransform() const { return maXForm; }
    void                ModifyWorldTransform( const XForm& rXForm, sal_uInt32 nMode );

    void                Push();
    void                Pop();

    sal_uInt32          SetRasterOp( sal_uInt32 nRasterOp );
    void                StrokeAndFillPath( sal_Bool bStroke, sal_Bool bFill );

    void                SetGfxMode( sal_Int32 nGfxMode ){ mnGfxMode = nGfxMode; };
    sal_Int32           GetGfxMode() const { return mnGfxMode; };
    void                SetBkMode( sal_uInt32 nMode );
    void                SetBkColor( const Color& rColor );
    void                SetTextColor( const Color& rColor );
    void                SetTextAlign( sal_uInt32 nAlign );
    void                CreateObject( GDIObjectType, void* pStyle = NULL );
    void                CreateObject( sal_Int32 nIndex, GDIObjectType, void* pStyle = NULL );
    void                DeleteObject( sal_Int32 nIndex );
    void                SelectObject( sal_Int32 nIndex );
    CharSet             GetCharSet(){ return maFont.GetCharSet(); };
    WinMtfFillStyle&    GetFillStyle () { return maFillStyle; }
    const Font&         GetFont() const;
    void                SetTextLayoutMode( const sal_uInt32 nLayoutMode );

    void                ClearPath(){ aPathObj.Init(); };
    void                ClosePath(){ aPathObj.ClosePath(); };
    const PolyPolygon&  GetPathObj(){ return aPathObj; };

    void                MoveTo( const Point& rPoint, sal_Bool bRecordPath = sal_False );
    void                LineTo( const Point& rPoint, sal_Bool bRecordPath = sal_False );
    void                DrawPixel( const Point& rSource, const Color& rColor );
    void                DrawRect( const Rectangle& rRect, sal_Bool bEdge = sal_True );
    void                DrawRoundRect( const Rectangle& rRect, const Size& rSize );
    void                DrawEllipse( const Rectangle& rRect );
    void                DrawArc(
                            const Rectangle& rRect,
                            const Point& rStartAngle,
                            const Point& rEndAngle,
                            sal_Bool bDrawTo = sal_False
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
    void                DrawPolygon( Polygon& rPolygon, sal_Bool bRecordPath = sal_False );
    void                DrawPolygon( Polygon& rPolygon, sal_Bool /*bDrawTo*/, sal_Bool bRecordPath)
                        {
                            //For ReadAndDrawPolygon template compatibility
                            DrawPolygon(rPolygon, bRecordPath);
                        }
    void                DrawPolyPolygon( PolyPolygon& rPolyPolygon, sal_Bool bRecordPath = sal_False );
    void                DrawPolyLine(
                            Polygon& rPolygon,
                            sal_Bool bDrawTo = sal_False,
                            sal_Bool bRecordPath = sal_False
                        );
    void                DrawPolyBezier(
                            Polygon& rPolygin,
                            sal_Bool bDrawTo = sal_False,
                            sal_Bool bRecordPath = sal_False
                        );
    void                DrawText(
                            Point& rPosition,
                            OUString& rString,
                            sal_Int32* pDXArry = NULL,
                            sal_Bool bRecordPath = sal_False,
                            sal_Int32 nGraphicsMode = GM_COMPATIBLE
                        );
    void                ResolveBitmapActions( BSaveStructList_impl& rSaveList );

    void                IntersectClipRect( const Rectangle& rRect );
    void                ExcludeClipRect( const Rectangle& rRect );
    void                MoveClipRegion( const Size& rSize );
    void                SetClipPath(
                            const PolyPolygon& rPolyPoly,
                            sal_Int32 nClippingMode,
                            sal_Bool bIsMapped
                        );
    void                UpdateClipRegion();
    void                AddFromGDIMetaFile( GDIMetaFile& rGDIMetaFile );

    void                PassEMFPlus( void* pBuffer, sal_uInt32 nLength );
    void                PassEMFPlusHeaderInfo();

                        WinMtfOutput( GDIMetaFile& rGDIMetaFile );
    virtual             ~WinMtfOutput();
};

// -----------------------------------------------------------------------------

class WinMtf
{
protected:

    WinMtfOutput*           pOut;
    SvStream*               pWMF;               // Die einzulesende WMF/EMF-Datei

    sal_uInt32              nStartPos, nEndPos;
    BSaveStructList_impl    aBmpSaveList;

    FilterConfigItem*   pFilterConfigItem;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    // Sorgt dafuer, das aSampledBrush der aktuelle Brush des GDIMetaFiles ist.

    Color               ReadColor();
    void                Callback( sal_uInt16 nPercent );

                        WinMtf(
                            WinMtfOutput* pOut,
                            SvStream& rStreamWMF,
                            FilterConfigItem* pConfigItem = NULL
                        );
                        ~WinMtf();
};

//============================ EMFReader ==================================

class EnhWMFReader : public WinMtf
{
    sal_Bool        bRecordPath;
    sal_Int32       nRecordCount;
    sal_Bool        bEMFPlus;


    sal_Bool        ReadHeader();
                    // Liesst und konvertiert ein Rechteck
    Rectangle       ReadRectangle( sal_Int32, sal_Int32, sal_Int32, sal_Int32 );
    void            ImplExtTextOut( sal_Bool bWideCharakter );

public:
                    EnhWMFReader(
                        SvStream& rStreamWMF,
                        GDIMetaFile& rGDIMetaFile,
                        FilterConfigItem* pConfigItem = NULL
                    )
                        : WinMtf( new WinMtfOutput( rGDIMetaFile )
                        , rStreamWMF
                        , pConfigItem )
                        , bRecordPath( sal_False )
                        , bEMFPlus( sal_False )
                    {};
                    ~EnhWMFReader();

    sal_Bool        ReadEnhWMF();
    void            ReadEMFPlusComment(sal_uInt32 length, sal_Bool& bHaveDC);
private:
    template <class T> void ReadAndDrawPolyPolygon();
    template <class T> void ReadAndDrawPolyLine();
    template <class T> Polygon ReadPolygon(sal_uInt32 nStartIndex, sal_uInt32 nPoints);
    template <class T, class Drawer> void ReadAndDrawPolygon(Drawer drawer, const sal_Bool skipFirst);
};

//============================ WMFReader ==================================

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

    // Liesst den Kopf der WMF-Datei
    sal_Bool        ReadHeader();

    // Liesst die Parameter des Rocords mit der Funktionsnummer nFunction.
    void            ReadRecordParams( sal_uInt16 nFunction );

    Point           ReadPoint();                // Liesst und konvertiert einen Punkt (erst X dann Y)
    Point           ReadYX();                   // Liesst und konvertiert einen Punkt (erst Y dann X)
    Rectangle       ReadRectangle();            // Liesst und konvertiert ein Rechteck
    Size            ReadYXExt();
    sal_Bool        GetPlaceableBound( Rectangle& rSize, SvStream* pStrm );

public:

                    WMFReader(
                        SvStream& rStreamWMF,
                        GDIMetaFile& rGDIMetaFile,
                        FilterConfigItem* pConfigItem = NULL,
                        WMF_EXTERNALHEADER* pExtHeader = NULL
                    )
                        : WinMtf( new WinMtfOutput( rGDIMetaFile ), rStreamWMF, pConfigItem )
                        , pEMFStream(NULL),
                        pExternalHeader(pExtHeader)
                    {}

                    ~WMFReader();

    // Liesst aus dem Stream eine WMF-Datei und fuellt das GDIMetaFile
    void            ReadWMF();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
