/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outdev.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_OUTDEV_HXX
#define _SV_OUTDEV_HXX

#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <tools/rc.hxx>
#include <tools/color.hxx>
#include <vcl/font.hxx>
#include <vcl/region.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/wall.hxx>
#include <vcl/settings.hxx>
#include <tools/poly.hxx>

#include <vector>

struct ImplOutDevData;
class ImplFontEntry;
struct ImplObjStack;
struct ImplKernPairData;
class ImplFontCache;
class ImplDevFontList;
class ImplGetDevFontList;
class ImplGetDevSizeList;
class ImplMultiTextLineInfo;
class SalGraphics;
class Gradient;
class Hatch;
class Bitmap;
class BitmapReadAccess;
class BitmapEx;
class Image;
class TextRectInfo;
class FontInfo;
class FontMetric;
class GDIMetaFile;
class OutDev3D;
class OpenGL;
class List;
class GfxLink;
class Line;
class LineInfo;
class AlphaMask;
class FontCharMap;
class SalLayout;
class ImplLayoutArgs;
class VirtualDevice;

namespace basegfx {
    class B2DHomMatrix;
    class B2DPolygon;
    class B2DPolyPolygon;
    typedef ::std::vector< B2DPolyPolygon > B2DPolyPolygonVector;
}
#include <com/sun/star/uno/Reference.h>

namespace com {
namespace sun {
namespace star {
namespace awt {
    class XGraphics;
} } } }

typedef std::vector< Rectangle > MetricVector;

namespace vcl { class PDFWriterImpl; }
namespace vcl { class ExtOutDevData; }

#define OUTDEV_BUFFER_SIZE  128


// ---------------------
// - OutputDevice-Data -
// ---------------------

struct ImplMapRes
{
    long                mnMapOfsX;          // Offset in X Richtung
    long                mnMapOfsY;          // Offset in Y Richtung
    long                mnMapScNumX;        // Skal.-faktor Zaehler X Richtung
    long                mnMapScNumY;        // Skal.-faktor Zaehler Y Richtung
    long                mnMapScDenomX;      // Skal.-faktor Nenner X Richtung
    long                mnMapScDenomY;      // Skal.-faktor Nenner Y Richtung
};

struct ImplThresholdRes
{
    long                mnThresLogToPixX;   // Schwellenwerte fuer Berechnung
    long                mnThresLogToPixY;   // mit BigInts
    long                mnThresPixToLogX;   // ""
    long                mnThresPixToLogY;   // ""
};

// ---------------
// - KerningPair -
// ---------------

struct KerningPair
{
    sal_Unicode         nChar1;
    sal_Unicode         nChar2;
    long                nKern;
};

// ----------------------
// - OutputDevice-Types -
// ----------------------

// Flags for Push()
#define PUSH_LINECOLOR                  ((USHORT)0x0001)
#define PUSH_FILLCOLOR                  ((USHORT)0x0002)
#define PUSH_FONT                       ((USHORT)0x0004)
#define PUSH_TEXTCOLOR                  ((USHORT)0x0008)
#define PUSH_MAPMODE                    ((USHORT)0x0010)
#define PUSH_CLIPREGION                 ((USHORT)0x0020)
#define PUSH_RASTEROP                   ((USHORT)0x0040)
#define PUSH_TEXTFILLCOLOR              ((USHORT)0x0080)
#define PUSH_TEXTALIGN                  ((USHORT)0x0100)
#define PUSH_REFPOINT                   ((USHORT)0x0200)
#define PUSH_TEXTLINECOLOR              ((USHORT)0x0400)
#define PUSH_TEXTLAYOUTMODE             ((USHORT)0x0800)
#define PUSH_TEXTLANGUAGE               ((USHORT)0x1000)
#define PUSH_ALLTEXT                    (PUSH_TEXTCOLOR | PUSH_TEXTFILLCOLOR | PUSH_TEXTLINECOLOR | PUSH_TEXTALIGN | PUSH_TEXTLAYOUTMODE | PUSH_TEXTLANGUAGE)
#define PUSH_ALLFONT                    (PUSH_ALLTEXT | PUSH_FONT)
#define PUSH_ALL                        ((USHORT)0xFFFF)

// Flags for DrawText()
#define TEXT_DRAW_DISABLE               ((USHORT)0x0001)
#define TEXT_DRAW_MNEMONIC              ((USHORT)0x0002)
#define TEXT_DRAW_MONO                  ((USHORT)0x0004)
#define TEXT_DRAW_CLIP                  ((USHORT)0x0008)
#define TEXT_DRAW_LEFT                  ((USHORT)0x0010)
#define TEXT_DRAW_CENTER                ((USHORT)0x0020)
#define TEXT_DRAW_RIGHT                 ((USHORT)0x0040)
#define TEXT_DRAW_TOP                   ((USHORT)0x0080)
#define TEXT_DRAW_VCENTER               ((USHORT)0x0100)
#define TEXT_DRAW_BOTTOM                ((USHORT)0x0200)
#define TEXT_DRAW_ENDELLIPSIS           ((USHORT)0x0400)
#define TEXT_DRAW_PATHELLIPSIS          ((USHORT)0x0800)
#define TEXT_DRAW_MULTILINE             ((USHORT)0x1000)
#define TEXT_DRAW_WORDBREAK             ((USHORT)0x2000)
#define TEXT_DRAW_NEWSELLIPSIS          ((USHORT)0x4000)

// Flags for CopyArea()
#define COPYAREA_WINDOWINVALIDATE       ((USHORT)0x0001)

// Flags for DrawImage()
#define IMAGE_DRAW_DISABLE              ((USHORT)0x0001)
#define IMAGE_DRAW_HIGHLIGHT            ((USHORT)0x0002)
#define IMAGE_DRAW_DEACTIVE             ((USHORT)0x0004)
#define IMAGE_DRAW_COLORTRANSFORM       ((USHORT)0x0008)
#define IMAGE_DRAW_SEMITRANSPARENT      ((USHORT)0x0010)
#define IMAGE_DRAW_MONOCHROME_BLACK     ((USHORT)0x0020)
#define IMAGE_DRAW_MONOCHROME_WHITE     ((USHORT)0x0040)
#define IMAGE_DRAW_3DLOOK               0
#define IMAGE_DRAW_BTNLOOK              0

// WaveLine
#define WAVE_FLAT                       1
#define WAVE_SMALL                      2
#define WAVE_NORMAL                     3

// Grid
#define GRID_DOTS                       ((ULONG)0x00000001)
#define GRID_HORZLINES                  ((ULONG)0x00000002)
#define GRID_VERTLINES                  ((ULONG)0x00000004)
#define GRID_LINES                      (GRID_HORZLINES | GRID_VERTLINES)

// LayoutModes for Complex Text Layout
#define TEXT_LAYOUT_DEFAULT             ((ULONG)0x00000000)
#define TEXT_LAYOUT_BIDI_LTR            ((ULONG)0x00000000)
#define TEXT_LAYOUT_BIDI_RTL            ((ULONG)0x00000001)
#define TEXT_LAYOUT_BIDI_STRONG         ((ULONG)0x00000002)
#define TEXT_LAYOUT_TEXTORIGIN_LEFT     ((ULONG)0x00000004)
#define TEXT_LAYOUT_TEXTORIGIN_RIGHT    ((ULONG)0x00000008)
#define TEXT_LAYOUT_COMPLEX_DISABLED    ((ULONG)0x00000100)
#define TEXT_LAYOUT_ENABLE_LIGATURES    ((ULONG)0x00000200)
#define TEXT_LAYOUT_SUBSTITUTE_DIGITS   ((ULONG)0x00000400)

// DrawModes
#define DRAWMODE_DEFAULT                ((ULONG)0x00000000)
#define DRAWMODE_BLACKLINE              ((ULONG)0x00000001)
#define DRAWMODE_BLACKFILL              ((ULONG)0x00000002)
#define DRAWMODE_BLACKTEXT              ((ULONG)0x00000004)
#define DRAWMODE_BLACKBITMAP            ((ULONG)0x00000008)
#define DRAWMODE_BLACKGRADIENT          ((ULONG)0x00000010)
#define DRAWMODE_GRAYLINE               ((ULONG)0x00000020)
#define DRAWMODE_GRAYFILL               ((ULONG)0x00000040)
#define DRAWMODE_GRAYTEXT               ((ULONG)0x00000080)
#define DRAWMODE_GRAYBITMAP             ((ULONG)0x00000100)
#define DRAWMODE_GRAYGRADIENT           ((ULONG)0x00000200)
#define DRAWMODE_NOFILL                 ((ULONG)0x00000400)
#define DRAWMODE_NOBITMAP               ((ULONG)0x00000800)
#define DRAWMODE_NOGRADIENT             ((ULONG)0x00001000)
#define DRAWMODE_GHOSTEDLINE            ((ULONG)0x00002000)
#define DRAWMODE_GHOSTEDFILL            ((ULONG)0x00004000)
#define DRAWMODE_GHOSTEDTEXT            ((ULONG)0x00008000)
#define DRAWMODE_GHOSTEDBITMAP          ((ULONG)0x00010000)
#define DRAWMODE_GHOSTEDGRADIENT        ((ULONG)0x00020000)
#define DRAWMODE_WHITELINE              ((ULONG)0x00100000)
#define DRAWMODE_WHITEFILL              ((ULONG)0x00200000)
#define DRAWMODE_WHITETEXT              ((ULONG)0x00400000)
#define DRAWMODE_WHITEBITMAP            ((ULONG)0x00800000)
#define DRAWMODE_WHITEGRADIENT          ((ULONG)0x01000000)
#define DRAWMODE_SETTINGSLINE           ((ULONG)0x02000000)
#define DRAWMODE_SETTINGSFILL           ((ULONG)0x04000000)
#define DRAWMODE_SETTINGSTEXT           ((ULONG)0x08000000)
#define DRAWMODE_SETTINGSGRADIENT       ((ULONG)0x10000000)
#define DRAWMODE_NOTRANSPARENCY         ((ULONG)0x80000000)

// Antialiasing
#define ANTIALIASING_DISABLE_TEXT       ((USHORT)0x0001)

// AddFontSubstitute
#define FONT_SUBSTITUTE_ALWAYS          ((USHORT)0x0001)
#define FONT_SUBSTITUTE_SCREENONLY      ((USHORT)0x0002)

// Default-Font
#define DEFAULTFONT_SANS_UNICODE        ((USHORT)1)
#define DEFAULTFONT_SANS                ((USHORT)2)
#define DEFAULTFONT_SERIF               ((USHORT)3)
#define DEFAULTFONT_FIXED               ((USHORT)4)
#define DEFAULTFONT_SYMBOL              ((USHORT)5)
#define DEFAULTFONT_UI_SANS             ((USHORT)1000)
#define DEFAULTFONT_UI_FIXED            ((USHORT)1001)
#define DEFAULTFONT_LATIN_TEXT          ((USHORT)2000)
#define DEFAULTFONT_LATIN_PRESENTATION  ((USHORT)2001)
#define DEFAULTFONT_LATIN_SPREADSHEET   ((USHORT)2002)
#define DEFAULTFONT_LATIN_HEADING       ((USHORT)2003)
#define DEFAULTFONT_LATIN_DISPLAY       ((USHORT)2004)
#define DEFAULTFONT_LATIN_FIXED         ((USHORT)2005)
#define DEFAULTFONT_CJK_TEXT            ((USHORT)3000)
#define DEFAULTFONT_CJK_PRESENTATION    ((USHORT)3001)
#define DEFAULTFONT_CJK_SPREADSHEET     ((USHORT)3002)
#define DEFAULTFONT_CJK_HEADING         ((USHORT)3003)
#define DEFAULTFONT_CJK_DISPLAY         ((USHORT)3004)
#define DEFAULTFONT_CTL_TEXT            ((USHORT)4000)
#define DEFAULTFONT_CTL_PRESENTATION    ((USHORT)4001)
#define DEFAULTFONT_CTL_SPREADSHEET     ((USHORT)4002)
#define DEFAULTFONT_CTL_HEADING         ((USHORT)4003)
#define DEFAULTFONT_CTL_DISPLAY         ((USHORT)4004)

#define DEFAULTFONT_FLAGS_ONLYONE       ((ULONG)0x00000001)

enum OutDevType { OUTDEV_DONTKNOW, OUTDEV_WINDOW, OUTDEV_PRINTER, OUTDEV_VIRDEV };

enum OutDevViewType { OUTDEV_VIEWTYPE_DONTKNOW, OUTDEV_VIEWTYPE_PRINTPREVIEW, OUTDEV_VIEWTYPE_SLIDESHOW };

// ----------------
// - OutputDevice -
// ----------------

class VirtualDevice;
class Printer;

const char* ImplDbgCheckOutputDevice( const void* pObj );

class VCL_DLLPUBLIC OutputDevice : public Resource
{
    friend class Application;
    friend class Bitmap;
    friend class ImplImageBmp;
    friend class ImplQPrinter;
    friend class OpenGL;
    friend class Printer;
    friend class SalGraphicsLayout;
    friend class System;
    friend class VirtualDevice;
    friend class Window;
    friend class WorkWindow;
    friend class vcl::PDFWriterImpl;
    friend const char* ImplDbgCheckOutputDevice( const void* pObj );
    friend void ImplHandleResize( Window* pWindow, long nNewWidth, long nNewHeight );

private:
    mutable SalGraphics*        mpGraphics;
    mutable OutputDevice*       mpPrevGraphics;
    mutable OutputDevice*       mpNextGraphics;
    GDIMetaFile*        mpMetaFile;
    mutable ImplFontEntry*      mpFontEntry;
    mutable ImplFontCache*      mpFontCache;
    mutable ImplDevFontList*    mpFontList;
    mutable ImplGetDevFontList* mpGetDevFontList;
    mutable ImplGetDevSizeList* mpGetDevSizeList;
    ImplObjStack*       mpObjStack;
    ImplOutDevData*     mpOutDevData;
    List*               mpUnoGraphicsList;
    OutDev3D*           mp3DContext;
    vcl::PDFWriterImpl* mpPDFWriter;
    vcl::ExtOutDevData* mpExtOutDevData;

    // TEMP TEMP TEMP
    VirtualDevice*      mpAlphaVDev;

    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    long                mnOutOffOrigX;
    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    long                mnOutOffLogicX;
    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    long                mnOutOffOrigY;
    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    long                mnOutOffLogicY;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    long                mnOutOffX;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    long                mnOutOffY;
    long                mnOutWidth;
    long                mnOutHeight;
    sal_Int32           mnDPIX;
    sal_Int32           mnDPIY;
    /// font specific text alignment offsets in pixel units
    mutable long        mnTextOffX;
    mutable long        mnTextOffY;
    mutable long        mnEmphasisAscent;
    mutable long        mnEmphasisDescent;
    ULONG               mnDrawMode;
    ULONG               mnTextLayoutMode;
    ImplMapRes          maMapRes;
    ImplThresholdRes    maThresRes;
    OutDevType          meOutDevType;
    OutDevViewType      meOutDevViewType;
    Region              maRegion;           // contains the clip region, see SetClipRegion(...)
    Color               maLineColor;
    Color               maFillColor;
    Font                maFont;
    Color               maTextColor;
    Color               maTextLineColor;
    TextAlign           meTextAlign;
    RasterOp            meRasterOp;
    Wallpaper           maBackground;
    AllSettings         maSettings;
    MapMode             maMapMode;
    Point               maRefPoint;
    USHORT              mnAntialiasing;
    LanguageType        meTextLanguage;
    mutable BOOL       mbMap:1,
                        mbMapIsDefault:1,
                        mbClipRegion:1,
                        mbBackground:1,
                        mbOutput:1,
                        mbDevOutput:1,
                        mbOutputClipped:1,
                        mbLineColor:1,
                        mbFillColor:1,
                        mbInitLineColor:1,
                        mbInitFillColor:1,
                        mbInitFont:1,
                        mbInitTextColor:1,
                        mbInitClipRegion:1,
                        mbClipRegionSet:1,
                        mbKerning:1,
                        mbNewFont:1,
                        mbTextLines:1,
                        mbTextSpecial:1,
                        mbRefPoint:1,
                        mbEnableRTL:1;

//#if 0 // _SOLAR__PRIVATE
public:
    SAL_DLLPRIVATE sal_Int32    ImplGetDPIX() const { return mnDPIX; }
    SAL_DLLPRIVATE sal_Int32    ImplGetDPIY() const { return mnDPIY; }
    SAL_DLLPRIVATE int          ImplGetGraphics() const;
    SAL_DLLPRIVATE void         ImplReleaseGraphics( BOOL bRelease = TRUE );
    SAL_DLLPRIVATE BOOL         ImplHasMirroredGraphics();
    SAL_DLLPRIVATE void         ImplReMirror( Point &rPoint ) const;
    SAL_DLLPRIVATE void         ImplReMirror( Rectangle &rRect ) const;
    SAL_DLLPRIVATE void         ImplReMirror( Region &rRegion ) const;
    SAL_DLLPRIVATE void         ImplInitOutDevData();
    SAL_DLLPRIVATE void         ImplDeInitOutDevData();
    SAL_DLLPRIVATE void         ImplInitLineColor();
    SAL_DLLPRIVATE void         ImplInitFillColor();
    SAL_DLLPRIVATE bool         ImplNewFont() const;
    SAL_DLLPRIVATE void         ImplInitFont() const;
    SAL_DLLPRIVATE void         ImplInitTextColor();
    SAL_DLLPRIVATE void         ImplInitClipRegion();
    SAL_DLLPRIVATE static BOOL  ImplSelectClipRegion( SalGraphics* pGraphics, const Region& rRegion, OutputDevice *pOutDev );
    SAL_DLLPRIVATE void         ImplSetClipRegion( const Region* pRegion );
    SAL_DLLPRIVATE void         ImplSetTriangleClipRegion( const PolyPolygon &rPolyPolygon );

    SAL_DLLPRIVATE SalLayout*   ImplLayout( const String&, xub_StrLen nIndex,
                                            xub_StrLen nLen, const Point& rLogicPos = Point(0,0),
                                            long nLogicWidth=0, const sal_Int32* pLogicDXArray=NULL,
                                            bool bFilter = false ) const;
    SAL_DLLPRIVATE ImplLayoutArgs ImplPrepareLayoutArgs( String&,
                                            xub_StrLen nIndex, xub_StrLen nLen,
                                            long nPixelWidth,
                                            const sal_Int32* pPixelDXArray ) const;
    SAL_DLLPRIVATE SalLayout*   ImplGlyphFallbackLayout( SalLayout*, ImplLayoutArgs& ) const;

    SAL_DLLPRIVATE long         ImplGetTextWidth( const SalLayout& ) const;
    SAL_DLLPRIVATE void         ImplDrawText( const Rectangle& rRect,
                                              const String& rOrigStr, USHORT nStyle,
                                              MetricVector* pVector, String* pDisplayText );
    SAL_DLLPRIVATE void         ImplDrawTextBackground( const SalLayout& );
    SAL_DLLPRIVATE void         ImplDrawTextLines( SalLayout&, FontStrikeout eStrikeout, FontUnderline eUnderline, BOOL bWordLine, BOOL bUnderlineAbove );
    SAL_DLLPRIVATE bool         ImplDrawRotateText( SalLayout& );
    SAL_DLLPRIVATE void         ImplDrawTextDirect( SalLayout&, BOOL bTextLines );
    SAL_DLLPRIVATE void         ImplDrawSpecialText( SalLayout& );
    SAL_DLLPRIVATE void         ImplDrawText( SalLayout& );
    SAL_DLLPRIVATE Rectangle    ImplGetTextBoundRect( const SalLayout& );
    SAL_DLLPRIVATE void         ImplDrawEmphasisMarks( SalLayout& );

    SAL_DLLPRIVATE void         ImplDrawTextRect( long nBaseX, long nBaseY, long nX, long nY, long nWidth, long nHeight );

    SAL_DLLPRIVATE void         ImplInitTextLineSize();
    SAL_DLLPRIVATE void         ImplInitAboveTextLineSize();
    SAL_DLLPRIVATE void         ImplDrawWaveLine( long nBaseX, long nBaseY, long nStartX, long nStartY, long nWidth, long nHeight, long nLineWidth, short nOrientation, const Color& rColor );
    SAL_DLLPRIVATE void         ImplDrawTextLine( long nBaseX, long nX, long nY, long nWidth, FontStrikeout eStrikeout, FontUnderline eUnderline, BOOL bUnderlineAbove );
    SAL_DLLPRIVATE void         ImplDrawMnemonicLine( long nX, long nY, long nWidth );
    SAL_DLLPRIVATE void         ImplGetEmphasisMark( PolyPolygon& rPolyPoly, BOOL& rPolyLine, Rectangle& rRect1, Rectangle& rRect2, long& rYOff, long& rWidth, FontEmphasisMark eEmphasis, long nHeight, short nOrient );
    SAL_DLLPRIVATE void         ImplDrawEmphasisMark( long nBaseX, long nX, long nY, const PolyPolygon& rPolyPoly, BOOL bPolyLine, const Rectangle& rRect1, const Rectangle& rRect2 );
    SAL_DLLPRIVATE long         ImplGetTextLines( ImplMultiTextLineInfo& rLineInfo, long nWidth, const XubString& rStr, USHORT nStyle ) const;
    SAL_DLLPRIVATE void         ImplInitFontList() const;
    SAL_DLLPRIVATE void         ImplUpdateFontData( bool bNewFontLists );
    SAL_DLLPRIVATE static void  ImplUpdateAllFontData( bool bNewFontLists );
    SAL_DLLPRIVATE long         ImplLogicXToDevicePixel( long nX ) const;
    SAL_DLLPRIVATE long         ImplLogicYToDevicePixel( long nY ) const;
    SAL_DLLPRIVATE long         ImplLogicWidthToDevicePixel( long nWidth ) const;
    SAL_DLLPRIVATE long         ImplLogicHeightToDevicePixel( long nHeight ) const;
    SAL_DLLPRIVATE long         ImplDevicePixelToLogicWidth( long nWidth ) const;
    SAL_DLLPRIVATE long         ImplDevicePixelToLogicHeight( long nHeight ) const;
    SAL_DLLPRIVATE float            ImplFloatLogicWidthToDevicePixel( float ) const;
    SAL_DLLPRIVATE float            ImplFloatLogicHeightToDevicePixel( float ) const;
    SAL_DLLPRIVATE float            ImplFloatDevicePixelToLogicWidth( float ) const;
    SAL_DLLPRIVATE float            ImplFloatDevicePixelToLogicHeight( float ) const;
    SAL_DLLPRIVATE Point        ImplLogicToDevicePixel( const Point& rLogicPt ) const;
    SAL_DLLPRIVATE Size         ImplLogicToDevicePixel( const Size& rLogicSize ) const;
    SAL_DLLPRIVATE Rectangle    ImplLogicToDevicePixel( const Rectangle& rLogicRect ) const;
    SAL_DLLPRIVATE Polygon      ImplLogicToDevicePixel( const Polygon& rLogicPoly ) const;
    SAL_DLLPRIVATE PolyPolygon  ImplLogicToDevicePixel( const PolyPolygon& rLogicPolyPoly ) const;
    SAL_DLLPRIVATE LineInfo     ImplLogicToDevicePixel( const LineInfo& rLineInfo ) const;
    SAL_DLLPRIVATE Rectangle    ImplDevicePixelToLogic( const Rectangle& rLogicRect ) const;
    SAL_DLLPRIVATE Region       ImplPixelToDevicePixel( const Region& rRegion ) const;
    SAL_DLLPRIVATE void         ImplDrawPolygon( const Polygon& rPoly, const PolyPolygon* pClipPolyPoly = NULL );
    SAL_DLLPRIVATE void         ImplDrawPolyPolygon( const PolyPolygon& rPolyPoly, const PolyPolygon* pClipPolyPoly = NULL );
    SAL_DLLPRIVATE void         ImplDrawPolyPolygon( USHORT nPoly, const PolyPolygon& rPolyPoly );
    SAL_DLLPRIVATE void         ImplDrawLinearGradient( const Rectangle& rRect, const Gradient& rGradient, BOOL bMtf, const PolyPolygon* pClipPolyPoly );
    SAL_DLLPRIVATE void         ImplDrawComplexGradient( const Rectangle& rRect, const Gradient& rGradient, BOOL bMtf, const PolyPolygon* pClipPolyPoly );

    SAL_DLLPRIVATE void         ImplDrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch, BOOL bMtf );
    SAL_DLLPRIVATE void         ImplCalcHatchValues( const Rectangle& rRect, long nDist, USHORT nAngle10, Point& rPt1, Point& rPt2, Size& rInc, Point& rEndPt1 );
    SAL_DLLPRIVATE void         ImplDrawHatchLine( const Line& rLine, const PolyPolygon& rPolyPoly, Point* pPtBuffer, BOOL bMtf );

    SAL_DLLPRIVATE void         ImplDrawWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         ImplDrawColorWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         ImplDrawBitmapWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         ImplDrawGradientWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         ImplDraw2ColorFrame( const Rectangle& rRect, const Color& rLeftTopColor, const Color& rRightBottomColor );

    SAL_DLLPRIVATE void         ImplDrawOutDevDirect( const OutputDevice* pSrcDev, void* pPosAry );
    SAL_DLLPRIVATE void         ImplDrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                        const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                        const Bitmap& rBitmap, const ULONG nAction );
    SAL_DLLPRIVATE void         ImplDrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                        const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                        const BitmapEx& rBitmapEx, const ULONG nAction );
    SAL_DLLPRIVATE void         ImplDrawMask( const Point& rDestPt, const Size& rDestSize,
                                        const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                        const Bitmap& rBitmap, const Color& rMaskColor,
                                        const ULONG nAction );
    SAL_DLLPRIVATE void         ImplDrawAlpha( const Bitmap& rBmp, const AlphaMask& rAlpha,
                                        const Point& rDestPt, const Size& rDestSize,
                                        const Point& rSrcPtPixel, const Size& rSrcSizePixel );
    SAL_DLLPRIVATE Bitmap       ImplBlend( Bitmap              aBmp,
                                           BitmapReadAccess*   pP,
                                           BitmapReadAccess*   pA,
                                           const sal_Int32     nOffY,
                                           const sal_Int32     nDstHeight,
                                           const sal_Int32     nOffX,
                                           const sal_Int32     nDstWidth,
                                           const Rectangle&    aBmpRect,
                                           const Size&         aOutSz,
                                           const bool          bHMirr,
                                           const bool          bVMirr,
                                           const long*         pMapX,
                                           const long*         pMapY );
    SAL_DLLPRIVATE Bitmap       ImplBlendWithAlpha( Bitmap              aBmp,
                                                    BitmapReadAccess*   pP,
                                                    BitmapReadAccess*   pA,
                                                    const Rectangle&    aDstRect,
                                                    const sal_Int32     nOffY,
                                                    const sal_Int32     nDstHeight,
                                                    const sal_Int32     nOffX,
                                                    const sal_Int32     nDstWidth,
                                                    const long*         pMapX,
                                                    const long*         pMapY );
    SAL_DLLPRIVATE void         ImplPrintTransparent( const Bitmap& rBmp, const Bitmap& rMask,
                                        const Point& rDestPt, const Size& rDestSize,
                                        const Point& rSrcPtPixel, const Size& rSrcSizePixel );
    SAL_DLLPRIVATE void         ImplPrintMask( const Bitmap& rMask, const Color& rMaskColor,
                                        const Point& rDestPt, const Size& rDestSize,
                                        const Point& rSrcPtPixel, const Size& rSrcSizePixel );
    SAL_DLLPRIVATE void         ImplDrawFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                        const OutputDevice& rOutDev, const Region& rRegion );
    SAL_DLLPRIVATE void         ImplGetFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                        OutputDevice& rOutDev );
    SAL_DLLPRIVATE void         ImplGetFrameBitmap( const Point& rPt, const Size& rSize,
                                        Bitmap& rBitmap ) const;

    SAL_DLLPRIVATE BOOL         ImplIsRecordLayout() const;

                   void         ImplAddDevFontSubstitute( const XubString& rFontName,
                                                  const XubString& rReplaceFontName,
                                                  USHORT nFlags = 0 );

    SAL_DLLPRIVATE static FontEmphasisMark ImplGetEmphasisMarkStyle( const Font& rFont );
    SAL_DLLPRIVATE static BOOL ImplIsUnderlineAbove( const Font& );
//#endif

protected:
                        OutputDevice();

private:
    SAL_DLLPRIVATE                OutputDevice( const OutputDevice& rOutDev );
    SAL_DLLPRIVATE OutputDevice&  operator =( const OutputDevice& rOutDev );

public:
    virtual             ~OutputDevice();

    OutDevType          GetOutDevType() const { return meOutDevType; }

    /** query an <code>OutputDevice</code> whether it spports a specific operation

    @return
    true if operation supported, else false
    */
    bool                supportsOperation( OutDevSupportType ) const;

    void                Set3DContext( OutDev3D* p3DContext ) { mp3DContext = p3DContext; }

    OutDev3D*           Get3DContext() const { return mp3DContext; }
    OpenGL*             GetOpenGL();
    vcl::PDFWriterImpl* GetPDFWriter() const { return mpPDFWriter; }

    void                SetExtOutDevData( vcl::ExtOutDevData* pExtOutDevData ) { mpExtOutDevData = pExtOutDevData; }
    vcl::ExtOutDevData* GetExtOutDevData() const { return mpExtOutDevData; }

    void                DrawTextLine( const Point& rPos, long nWidth,
                                      FontStrikeout eStrikeout,
                                      FontUnderline eUnderline,
                                      BOOL bUnderlineAbove = FALSE );
    static BOOL         IsTextUnderlineAbove( const Font& rFont );

    void                DrawText( const Point& rStartPt, const XubString& rStr,
                                  xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN,
                                  MetricVector* pVector = NULL, String* pDisplayText = NULL );
    long                GetTextWidth( const XubString& rStr, xub_StrLen nIndex = 0,
                                      xub_StrLen nLen = STRING_LEN ) const;
    long                GetTextHeight() const;
    void                DrawTextArray( const Point& rStartPt, const XubString& rStr,
                                       const sal_Int32* pDXAry = NULL,
                                       xub_StrLen nIndex = 0,
                                       xub_StrLen nLen = STRING_LEN );
    long                GetTextArray( const XubString& rStr, sal_Int32* pDXAry = NULL,
                                      xub_StrLen nIndex = 0,
                                      xub_StrLen nLen = STRING_LEN ) const;
    bool                GetCaretPositions( const XubString&, sal_Int32* pCaretXArray,
                                      xub_StrLen nIndex, xub_StrLen nLen,
                                      sal_Int32* pDXAry = NULL, long nWidth = 0,
                                      BOOL bCellBreaking = TRUE ) const;
    void                DrawStretchText( const Point& rStartPt, ULONG nWidth,
                                         const XubString& rStr,
                                         xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN );
    xub_StrLen          GetTextBreak( const XubString& rStr, long nTextWidth,
                                      xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN,
                                      long nCharExtra = 0, BOOL bCellBreaking = TRUE ) const;
    xub_StrLen          GetTextBreak( const XubString& rStr, long nTextWidth,
                                      USHORT nExtraChar, xub_StrLen& rExtraCharPos,
                                      xub_StrLen nIndex, xub_StrLen nLen,
                                      long nCharExtra = 0 ) const;
    /** Generate MetaTextActions for the text rect

        This method splits up the text rect into multiple
        MetaTextActions, one for each line of text. This is comparable
        to AddGradientActions(), which splits up a gradient into its
        constituing polygons. Parameter semantics fully compatible to
        DrawText().
     */
    void                AddTextRectActions( const Rectangle& rRect,
                                            const String&    rOrigStr,
                                            USHORT           nStyle,
                                            GDIMetaFile&     rMtf );
    void                DrawText( const Rectangle& rRect,
                                  const XubString& rStr, USHORT nStyle = 0,
                                  MetricVector* pVector = NULL, String* pDisplayText = NULL );
    Rectangle           GetTextRect( const Rectangle& rRect,
                                     const XubString& rStr, USHORT nStyle = TEXT_DRAW_WORDBREAK,
                                     TextRectInfo* pInfo = NULL ) const;
    XubString           GetEllipsisString( const XubString& rStr, long nMaxWidth,
                                           USHORT nStyle = TEXT_DRAW_ENDELLIPSIS ) const;
    void                DrawCtrlText( const Point& rPos, const XubString& rStr,
                                      xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN,
                                      USHORT nStyle = TEXT_DRAW_MNEMONIC, MetricVector* pVector = NULL, String* pDisplayText = NULL );
    long                GetCtrlTextWidth( const XubString& rStr, xub_StrLen nIndex = 0,
                                          xub_StrLen nLen = STRING_LEN,
                                          USHORT nStyle = TEXT_DRAW_MNEMONIC ) const;
    static XubString    GetNonMnemonicString( const XubString& rStr, xub_StrLen& rMnemonicPos );
    static XubString    GetNonMnemonicString( const XubString& rStr )
                            { xub_StrLen nDummy; return GetNonMnemonicString( rStr, nDummy ); }

    ULONG               GetKerningPairCount() const;
    void                GetKerningPairs( ULONG nPairs, KerningPair* pKernPairs ) const;

    BOOL                GetTextBoundRect( Rectangle& rRect,
                            const String& rStr, xub_StrLen nBase = 0, xub_StrLen nIndex = 0,
                            xub_StrLen nLen = STRING_LEN ) const;
    BOOL                GetTextOutline( PolyPolygon&,
                            const String& rStr, xub_StrLen nBase = 0, xub_StrLen nIndex = 0,
                            xub_StrLen nLen = STRING_LEN, BOOL bOptimize = TRUE,
                const ULONG nWidth = 0, const sal_Int32* pDXArray = NULL ) const;
    BOOL                GetTextOutlines( PolyPolyVector&,
                            const String& rStr, xub_StrLen nBase = 0, xub_StrLen nIndex = 0,
                            xub_StrLen nLen = STRING_LEN, BOOL bOptimize = TRUE,
                const ULONG nWidth = 0, const sal_Int32* pDXArray = NULL ) const;
    BOOL                GetTextOutlines( ::basegfx::B2DPolyPolygonVector&,
                            const String& rStr, xub_StrLen nBase = 0, xub_StrLen nIndex = 0,
                            xub_StrLen nLen = STRING_LEN, BOOL bOptimize = TRUE,
                const ULONG nWidth = 0, const sal_Int32* pDXArray = NULL ) const;
    BOOL                GetGlyphBoundRects( const Point& rOrigin, const String& rStr, int nIndex,
                            int nLen, int nBase, MetricVector& rVector );

    void                DrawPixel( const Point& rPt );
    void                DrawPixel( const Point& rPt, const Color& rColor );
    void                DrawPixel( const Polygon& rPts, const Color* pColors = NULL );
    void                DrawPixel( const Polygon& rPts, const Color& rColor );

    void                DrawLine( const Point& rStartPt, const Point& rEndPt );
    void                DrawLine( const Point& rStartPt, const Point& rEndPt,
                                  const LineInfo& rLineInfo );

    /** Render the given polygon as a line stroke

        The given polygon is stroked with the current LineColor, start
        and end point are not automatically connected

        @see DrawPolygon
        @see DrawPolyPolygon
     */
    void                DrawPolyLine( const Polygon& rPoly );

    /** Render the given polygon as a line stroke

        The given polygon is stroked with the current LineColor, start
        and end point are not automatically connected. The line is
        rendered according to the specified LineInfo, e.g. supplying a
        dash pattern, or a line thickness.

        @see DrawPolygon
        @see DrawPolyPolygon
     */
    void                DrawPolyLine( const Polygon& rPoly,
                                      const LineInfo& rLineInfo );

    /** Render the given polygon

        The given polygon is stroked with the current LineColor, and
        filled with the current FillColor. If one of these colors are
        transparent, the corresponding stroke or fill stays
        invisible. Start and end point of the polygon are
        automatically connected.

        @see DrawPolyLine
     */
    void                DrawPolygon( const Polygon& rPoly );

    /** Render the given poly-polygon

        The given poly-polygon is stroked with the current LineColor,
        and filled with the current FillColor. If one of these colors
        are transparent, the corresponding stroke or fill stays
        invisible. Start and end points of the contained polygons are
        automatically connected.

        @see DrawPolyLine
     */
    void                DrawPolyPolygon( const PolyPolygon& rPolyPoly );
    void                DrawRect( const Rectangle& rRect );
    void                DrawRect( const Rectangle& rRect,
                                  ULONG nHorzRount, ULONG nVertRound );
    void                DrawEllipse( const Rectangle& rRect );
    void                DrawArc( const Rectangle& rRect,
                                 const Point& rStartPt, const Point& rEndPt );
    void                DrawPie( const Rectangle& rRect,
                                 const Point& rStartPt, const Point& rEndPt );
    void                DrawChord( const Rectangle& rRect,
                                   const Point& rStartPt, const Point& rEndPt );

    void                DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPt,  const Size& rSrcSize );
    void                DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPt,  const Size& rSrcSize,
                                    const OutputDevice& rOutDev );
    void                CopyArea( const Point& rDestPt,
                                  const Point& rSrcPt,  const Size& rSrcSize,
                                  USHORT nFlags = 0 );

    void                DrawBitmap( const Point& rDestPt,
                                    const Bitmap& rBitmap );
    void                DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                    const Bitmap& rBitmap );
    void                DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                    const Bitmap& rBitmap );

    void                DrawBitmapEx( const Point& rDestPt,
                                      const BitmapEx& rBitmapEx );
    void                DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                      const BitmapEx& rBitmapEx );
    void                DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                      const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                      const BitmapEx& rBitmapEx );

    void                DrawMask( const Point& rDestPt,
                                  const Bitmap& rBitmap, const Color& rMaskColor );
    void                DrawMask( const Point& rDestPt, const Size& rDestSize,
                                  const Bitmap& rBitmap, const Color& rMaskColor );
    void                DrawMask( const Point& rDestPt, const Size& rDestSize,
                                  const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                  const Bitmap& rBitmap, const Color& rMaskColor );

    void                DrawImage( const Point& rPos,
                                   const Image& rImage, USHORT nStyle = 0 );
    void                DrawImage( const Point& rPos, const Size& rSize,
                                   const Image& rImage, USHORT nStyle = 0 );

    void                DrawGradient( const Rectangle& rRect, const Gradient& rGradient );
    void                DrawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient );
    void                AddGradientActions( const Rectangle& rRect,
                                            const Gradient& rGradient,
                                            GDIMetaFile& rMtf );

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    void                DrawHatch( const PolyPolygon& rPolyPoly, const ::Hatch& rHatch );
    void                AddHatchActions( const PolyPolygon& rPolyPoly,
                                         const ::Hatch& rHatch,
                                         GDIMetaFile& rMtf );
#else
    void                DrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch );
    void                AddHatchActions( const PolyPolygon& rPolyPoly,
                                         const Hatch& rHatch,
                                         GDIMetaFile& rMtf );
#endif

    void                DrawWallpaper( const Rectangle& rRect, const Wallpaper& rWallpaper );
    void                DrawWaveLine( const Point& rStartPos, const Point& rEndPos, USHORT nStyle );
    void                DrawGrid( const Rectangle& rRect, const Size& rDist, ULONG nFlags );

    void                DrawTransparent( const PolyPolygon& rPolyPoly,
                                         USHORT nTransparencePercent );
    void                DrawTransparent( const GDIMetaFile& rMtf,
                                         const Point& rPos, const Size& rSize,
                                         const Gradient& rTransparenceGradient );

    Color               GetPixel( const Point& rPt ) const;
    Color*              GetPixel( const Polygon& rPts ) const;

    Bitmap              GetBitmap( const Point& rSrcPt, const Size& rSize ) const;

    /** Query extended bitmap (with alpha channel, if available).
     */
    BitmapEx            GetBitmapEx( const Point& rSrcPt, const Size& rSize ) const;

    void                EnableMapMode( BOOL bEnable = TRUE );
    BOOL                IsMapModeEnabled() const { return mbMap; }

    // Enabling/disabling RTL only makes sense for OutputDevices that use a mirroring SalGraphisLayout
    void                EnableRTL( BOOL bEnable = TRUE);
    BOOL                IsRTLEnabled() const { return mbEnableRTL; }

    void                SetConnectMetaFile( GDIMetaFile* pMtf );
    GDIMetaFile*        GetConnectMetaFile() const { return mpMetaFile; }

    void                EnableOutput( BOOL bEnable = TRUE );
    BOOL                IsOutputEnabled() const { return mbOutput; }
    BOOL                IsDeviceOutput() const { return mbDevOutput; }
    BOOL                IsDeviceOutputNecessary() const { return (mbOutput && mbDevOutput); }
    BOOL                IsOutputNecessary() const { return ((mbOutput && mbDevOutput) || (mpMetaFile != NULL)); }

    void                SetClipRegion();
    void                SetClipRegion( const Region& rRegion );
    void                SetTriangleClipRegion( const PolyPolygon &rRegion );
    Region              GetClipRegion() const;
    BOOL                IsClipRegion() const { return mbClipRegion; }
    Region              GetActiveClipRegion() const;

    void                MoveClipRegion( long nHorzMove, long nVertMove );
    void                IntersectClipRegion( const Rectangle& rRect );
    void                IntersectClipRegion( const Region& rRegion );

    void                SetAntialiasing( USHORT nMode =  0 );
    USHORT              GetAntialiasing() const { return mnAntialiasing; }

    void                SetDrawMode( ULONG nDrawMode );
    ULONG               GetDrawMode() const { return mnDrawMode; }

    void                SetLayoutMode( ULONG nTextLayoutMode );
    ULONG               GetLayoutMode() const { return mnTextLayoutMode; }

    void                SetDigitLanguage( LanguageType );
    LanguageType        GetDigitLanguage() const { return meTextLanguage; }

    void                SetRasterOp( RasterOp eRasterOp );
    RasterOp            GetRasterOp() const { return meRasterOp; }

    /**
    If this OutputDevice is used for displaying a Print Preview
    the OutDevViewType should be set to 'OUTDEV_VIEWTYPE_PRINTPREVIEW'.

    A View than can make painting decisions dependent on this OutDevViewType.
    E.g. text colors need to be handled different, dependent on wether it's a PrintPreview or not. (see #106611# for more)
    */
    void                SetOutDevViewType( OutDevViewType eOutDevViewType ) { meOutDevViewType=eOutDevViewType; }
    OutDevViewType      GetOutDevViewType() const { return meOutDevViewType; }

    void                SetLineColor();
    void                SetLineColor( const Color& rColor );
    const Color&        GetLineColor() const { return maLineColor; }
    BOOL                IsLineColor() const { return mbLineColor; }

    void                SetFillColor();
    void                SetFillColor( const Color& rColor );
    const Color&        GetFillColor() const { return maFillColor; }
    BOOL                IsFillColor() const { return mbFillColor; }

    void                SetBackground();
    void                SetBackground( const Wallpaper& rBackground );

    const Wallpaper&    GetBackground() const { return maBackground; }
    BOOL                IsBackground() const { return mbBackground; }

    void                SetFont( const Font& rNewFont );
    const Font&         GetFont() const { return maFont; }
    void                SetTextColor( const Color& rColor );
    const Color&        GetTextColor() const { return maTextColor; }
    void                SetTextFillColor();
    void                SetTextFillColor( const Color& rColor );

    Color               GetTextFillColor() const;
    BOOL                IsTextFillColor() const { return !maFont.IsTransparent(); }
    void                SetTextLineColor();
    void                SetTextLineColor( const Color& rColor );
    const Color&        GetTextLineColor() const { return maTextLineColor; }
    BOOL                IsTextLineColor() const { return (maTextLineColor.GetTransparency() == 0); }
    void                SetTextAlign( TextAlign eAlign );
    TextAlign           GetTextAlign() const { return maFont.GetAlign(); }

    virtual void        SetSettings( const AllSettings& rSettings );
    const AllSettings&  GetSettings() const { return maSettings; }

    virtual void        SetMapMode();
    virtual void        SetMapMode( const MapMode& rNewMapMode );
    virtual void        SetRelativeMapMode( const MapMode& rNewMapMode );
    const MapMode&      GetMapMode() const { return maMapMode; }
    BOOL                IsMapMode() const { return mbMap; }

    void                SetRefPoint();
    void                SetRefPoint( const Point& rRefPoint );
    const Point&        GetRefPoint() const { return maRefPoint; }
    BOOL                IsRefPoint() const { return mbRefPoint; }

     // #i75163#
    basegfx::B2DHomMatrix GetViewTransformation() const;
    basegfx::B2DHomMatrix GetInverseViewTransformation() const;
    SAL_DLLPRIVATE void ImplInvalidateViewTransform();

    /** Set an offset in pixel

        This method offsets every drawing operation that converts its
        coordinates to pixel by the given value. Normally, the effect
        can be achieved by setting a MapMode with a different
        origin. Unfortunately, this origin is in logical coordinates
        and can lead to rounding errors (see #102532# for details).

        @attention This offset is only applied when converting to
        pixel, i.e. some output modes such as metafile recordings
        might be completely unaffected by this method! Use with
        care. Furthermore, if the OutputDevice's MapMode is the
        default (that's MAP_PIXEL), then, too, any pixel offset set is
        ignored. This might be unintuitive for cases, but would have
        been far more fragile to implement. What's more, the reason
        why the pixel offset was introduced (avoiding rounding errors)
        does not apply for MAP_PIXEL, because one can always use the
        MapMode origin then.

        @param rOffset
        The offset in pixel
     */
    void                SetPixelOffset( const Size& rOffset );

    /** Get the offset in pixel

        @see OutputDevice::SetPixelOffset for details

        @return the current offset in pixel
     */
    Size                GetPixelOffset() const;

    Point               LogicToPixel( const Point& rLogicPt ) const;
    Size                LogicToPixel( const Size& rLogicSize ) const;
    Rectangle           LogicToPixel( const Rectangle& rLogicRect ) const;
    Polygon             LogicToPixel( const Polygon& rLogicPoly ) const;
    PolyPolygon         LogicToPixel( const PolyPolygon& rLogicPolyPoly ) const;
    Region              LogicToPixel( const Region& rLogicRegion )const;
    Point               LogicToPixel( const Point& rLogicPt,
                                      const MapMode& rMapMode ) const;
    Size                LogicToPixel( const Size& rLogicSize,
                                      const MapMode& rMapMode ) const;
    Rectangle           LogicToPixel( const Rectangle& rLogicRect,
                                      const MapMode& rMapMode ) const;
    Polygon             LogicToPixel( const Polygon& rLogicPoly,
                                      const MapMode& rMapMode ) const;
    PolyPolygon         LogicToPixel( const PolyPolygon& rLogicPolyPoly,
                                      const MapMode& rMapMode ) const;
    Region              LogicToPixel( const Region& rLogicRegion,
                                      const MapMode& rMapMode ) const;
    Point               PixelToLogic( const Point& rDevicePt ) const;
    Size                PixelToLogic( const Size& rDeviceSize ) const;
    Rectangle           PixelToLogic( const Rectangle& rDeviceRect ) const;
    Polygon             PixelToLogic( const Polygon& rDevicePoly ) const;
    PolyPolygon         PixelToLogic( const PolyPolygon& rDevicePolyPoly ) const;
    Region              PixelToLogic( const Region& rDeviceRegion ) const;
    Point               PixelToLogic( const Point& rDevicePt,
                                      const MapMode& rMapMode ) const;
    Size                PixelToLogic( const Size& rDeviceSize,
                                      const MapMode& rMapMode ) const;
    Rectangle           PixelToLogic( const Rectangle& rDeviceRect,
                                      const MapMode& rMapMode ) const;
    Polygon             PixelToLogic( const Polygon& rDevicePoly,
                                      const MapMode& rMapMode ) const;
    PolyPolygon         PixelToLogic( const PolyPolygon& rDevicePolyPoly,
                                      const MapMode& rMapMode ) const;
    Region              PixelToLogic( const Region& rDeviceRegion,
                                      const MapMode& rMapMode ) const;

    Point               LogicToLogic( const Point&      rPtSource,
                                      const MapMode*    pMapModeSource,
                                      const MapMode*    pMapModeDest ) const;
    Size                LogicToLogic( const Size&       rSzSource,
                                      const MapMode*    pMapModeSource,
                                      const MapMode*    pMapModeDest ) const;
    Rectangle           LogicToLogic( const Rectangle&  rRectSource,
                                      const MapMode*    pMapModeSource,
                                      const MapMode*    pMapModeDest ) const;
    long*               LogicToLogic( long*             pX,
                                      USHORT            nCount,
                                      const MapMode*    pMapModeSource,
                                      const MapMode*    pMapModeDest ) const;
    static Point        LogicToLogic( const Point&      rPtSource,
                                      const MapMode&    rMapModeSource,
                                      const MapMode&    rMapModeDest );
    static Size         LogicToLogic( const Size&       rSzSource,
                                      const MapMode&    rMapModeSource,
                                      const MapMode&    rMapModeDest );
    static Rectangle    LogicToLogic( const Rectangle&  rRectSource,
                                      const MapMode&    rMapModeSource,
                                      const MapMode&    rMapModeDest );
    static long         LogicToLogic( long              nLongSource,
                                      MapUnit           eUnitSource,
                                      MapUnit           eUnitDest );

    Size                GetOutputSizePixel() const
                            { return Size( mnOutWidth, mnOutHeight ); }
    long                GetOutputWidthPixel() const { return mnOutWidth; }
    long                GetOutputHeightPixel() const { return mnOutHeight; }
    long                GetOutOffXPixel() const { return mnOutOffX; }
    long                GetOutOffYPixel() const { return mnOutOffY; }

    Size                GetOutputSize() const
                            { return PixelToLogic( GetOutputSizePixel() ); }

    void                Erase();
    void                Erase( const Rectangle& rRect ) { DrawWallpaper( rRect, GetBackground() ); }

    BOOL                AddTempDevFont( const String& rFileURL, const String& rFontName );
    int                 GetDevFontCount() const;
    FontInfo            GetDevFont( int nDevFontIndex ) const;
    int                 GetDevFontSizeCount( const Font& ) const;
    Size                GetDevFontSize( const Font& rFont, int nSizeIndex ) const;
    BOOL                IsFontAvailable( const String& rFontName ) const;

    FontMetric          GetFontMetric() const;
    FontMetric          GetFontMetric( const Font& rFont ) const;
    BOOL                GetFontCharMap( FontCharMap& rFontCharMap ) const;

    xub_StrLen          HasGlyphs( const Font& rFont, const String& rStr,
                            xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN ) const;

    USHORT              GetBitCount() const;

    BOOL                GetTextIsRTL( const String&, xub_StrLen nIndex,
                            xub_StrLen nLen ) const;

    /** Query the existence and depth of the alpha channel

        @return 0, if no alpha channel available, and the bit depth of
        the alpha channel otherwise.
     */
    USHORT              GetAlphaBitCount() const;
    ULONG               GetColorCount() const;

    void                Push( USHORT nFlags = PUSH_ALL );
    void                Pop();

    /** Query availability of alpha channel

        @return TRUE, if this device has an alpha channel.
     */
    BOOL                HasAlpha();

    void                DrawEPS( const Point& rPt, const Size& rSz,
                                 const GfxLink& rGfxLink, GDIMetaFile* pSubst = NULL );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >    CreateUnoGraphics();
    List*               GetUnoGraphicsList() const      { return mpUnoGraphicsList; }
    List*               CreateUnoGraphicsList()         { mpUnoGraphicsList = new List; return mpUnoGraphicsList; }

    static void         BeginFontSubstitution();
    static void         EndFontSubstitution();
    static void         AddFontSubstitute( const XubString& rFontName,
                                           const XubString& rReplaceFontName,
                                           USHORT nFlags = 0 );
    static void         RemoveFontSubstitute( USHORT n );
    static USHORT       GetFontSubstituteCount();
    static void         GetFontSubstitute( USHORT n,
                                           XubString& rFontName,
                                           XubString& rReplaceFontName,
                                           USHORT& rFlags );

    static Font         GetDefaultFont( USHORT nType,
                                        LanguageType eLang,
                                        ULONG nFlags,
                                        const OutputDevice* pOutDev = NULL );
};

#endif // _SV_OUTDEV_HXX
