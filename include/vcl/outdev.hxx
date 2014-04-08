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

#ifndef INCLUDED_VCL_OUTDEV_HXX
#define INCLUDED_VCL_OUTDEV_HXX

#include <tools/gen.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <tools/rc.hxx>
#include <tools/color.hxx>
#include <vcl/font.hxx>
#include <vcl/region.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/wall.hxx>
#include <vcl/metaact.hxx>
#include <vcl/salnativewidgets.hxx>
#include <tools/poly.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <com/sun/star/uno/Reference.h>
#include <unotools/fontdefs.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <boost/scoped_ptr.hpp>
#include <com/sun/star/drawing/LineCap.hpp>

#include <vector>

struct ImplOutDevData;
class ImplFontEntry;
struct ImplObjStack;
struct SystemGraphicsData;
struct SystemFontData;
struct SystemTextLayoutData;
class ImplFontCache;
class PhysicalFontCollection;
class ImplGetDevFontList;
class ImplGetDevSizeList;
class ImplMultiTextLineInfo;
class SalGraphics;
class Gradient;
class Hatch;
class AllSettings;
class Bitmap;
class BitmapReadAccess;
class BitmapEx;
class Image;
class TextRectInfo;
class FontInfo;
class FontMetric;
class GDIMetaFile;
class GfxLink;
class Line;
class LineInfo;
class AlphaMask;
class FontCharMap;
class SalLayout;
class ImplLayoutArgs;
class ImplFontAttributes;
class VirtualDevice;
class Window;
struct SalTwoRect;

namespace com {
namespace sun {
namespace star {
namespace rendering {
    class XCanvas;
}}}}
namespace basegfx {
    class B2DHomMatrix;
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace com {
namespace sun {
namespace star {
namespace awt {
    class XGraphics;
} } } }

typedef std::vector< Rectangle > MetricVector;

namespace vcl
{
    class PDFWriterImpl;
    class ExtOutDevData;
    class ITextLayout;
    struct FontCapabilities;
}

// OutputDevice-Data

struct ImplMapRes
{
    long                mnMapOfsX;          // Offset in X direction
    long                mnMapOfsY;          // Offset in Y direction
    long                mnMapScNumX;        // Scaling factor - numerator in X direction
    long                mnMapScNumY;        // Scaling factor - numerator in Y direction
    long                mnMapScDenomX;      // Scaling factor - denominator in X direction
    long                mnMapScDenomY;      // Scaling factor - denominator in Y direction
};

struct ImplThresholdRes
{
    long                mnThresLogToPixX;   // Thresholds for calculation
    long                mnThresLogToPixY;   // with BigInts
    long                mnThresPixToLogX;   // ""
    long                mnThresPixToLogY;   // ""
};

// OutputDevice-Types

// Flags for Push()
#define PUSH_LINECOLOR                  ((sal_uInt16)0x0001)
#define PUSH_FILLCOLOR                  ((sal_uInt16)0x0002)
#define PUSH_FONT                       ((sal_uInt16)0x0004)
#define PUSH_TEXTCOLOR                  ((sal_uInt16)0x0008)
#define PUSH_MAPMODE                    ((sal_uInt16)0x0010)
#define PUSH_CLIPREGION                 ((sal_uInt16)0x0020)
#define PUSH_RASTEROP                   ((sal_uInt16)0x0040)
#define PUSH_TEXTFILLCOLOR              ((sal_uInt16)0x0080)
#define PUSH_TEXTALIGN                  ((sal_uInt16)0x0100)
#define PUSH_REFPOINT                   ((sal_uInt16)0x0200)
#define PUSH_TEXTLINECOLOR              ((sal_uInt16)0x0400)
#define PUSH_TEXTLAYOUTMODE             ((sal_uInt16)0x0800)
#define PUSH_TEXTLANGUAGE               ((sal_uInt16)0x1000)
#define PUSH_OVERLINECOLOR              ((sal_uInt16)0x2000)
#define PUSH_ALLTEXT                    (PUSH_TEXTCOLOR | PUSH_TEXTFILLCOLOR | PUSH_TEXTLINECOLOR | PUSH_OVERLINECOLOR | PUSH_TEXTALIGN | PUSH_TEXTLAYOUTMODE | PUSH_TEXTLANGUAGE)
#define PUSH_ALLFONT                    (PUSH_ALLTEXT | PUSH_FONT)
#define PUSH_ALL                        ((sal_uInt16)0xFFFF)

// Flags for DrawText()
#define TEXT_DRAW_DISABLE               ((sal_uInt16)0x0001)
#define TEXT_DRAW_MNEMONIC              ((sal_uInt16)0x0002)
#define TEXT_DRAW_MONO                  ((sal_uInt16)0x0004)
#define TEXT_DRAW_CLIP                  ((sal_uInt16)0x0008)
#define TEXT_DRAW_LEFT                  ((sal_uInt16)0x0010)
#define TEXT_DRAW_CENTER                ((sal_uInt16)0x0020)
#define TEXT_DRAW_RIGHT                 ((sal_uInt16)0x0040)
#define TEXT_DRAW_TOP                   ((sal_uInt16)0x0080)
#define TEXT_DRAW_VCENTER               ((sal_uInt16)0x0100)
#define TEXT_DRAW_BOTTOM                ((sal_uInt16)0x0200)
#define TEXT_DRAW_ENDELLIPSIS           ((sal_uInt16)0x0400)
#define TEXT_DRAW_PATHELLIPSIS          ((sal_uInt16)0x0800)
#define TEXT_DRAW_MULTILINE             ((sal_uInt16)0x1000)
#define TEXT_DRAW_WORDBREAK             ((sal_uInt16)0x2000)
#define TEXT_DRAW_NEWSELLIPSIS          ((sal_uInt16)0x4000)
// in the long run we should make text style flags longer
// but at the moment we can get away with this 2 bit field for ellipsis style
#define TEXT_DRAW_CENTERELLIPSIS        (TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_PATHELLIPSIS)

#define TEXT_DRAW_WORDBREAK_HYPHENATION (((sal_uInt16)0x8000) | TEXT_DRAW_WORDBREAK)

// Flags for CopyArea()
#define COPYAREA_WINDOWINVALIDATE       ((sal_uInt16)0x0001)

// Flags for DrawImage()
#define IMAGE_DRAW_DISABLE              ((sal_uInt16)0x0001)
#define IMAGE_DRAW_HIGHLIGHT            ((sal_uInt16)0x0002)
#define IMAGE_DRAW_DEACTIVE             ((sal_uInt16)0x0004)
#define IMAGE_DRAW_COLORTRANSFORM       ((sal_uInt16)0x0008)
#define IMAGE_DRAW_SEMITRANSPARENT      ((sal_uInt16)0x0010)

// Grid
#define GRID_DOTS                       ((sal_uLong)0x00000001)
#define GRID_HORZLINES                  ((sal_uLong)0x00000002)
#define GRID_VERTLINES                  ((sal_uLong)0x00000004)
#define GRID_LINES                      (GRID_HORZLINES | GRID_VERTLINES)

// LayoutModes for Complex Text Layout
#define TEXT_LAYOUT_DEFAULT             ((sal_uLong)0x00000000)
#define TEXT_LAYOUT_BIDI_LTR            ((sal_uLong)0x00000000)
#define TEXT_LAYOUT_BIDI_RTL            ((sal_uLong)0x00000001)
#define TEXT_LAYOUT_BIDI_STRONG         ((sal_uLong)0x00000002)
#define TEXT_LAYOUT_TEXTORIGIN_LEFT     ((sal_uLong)0x00000004)
#define TEXT_LAYOUT_TEXTORIGIN_RIGHT    ((sal_uLong)0x00000008)
#define TEXT_LAYOUT_COMPLEX_DISABLED    ((sal_uLong)0x00000100)
#define TEXT_LAYOUT_ENABLE_LIGATURES    ((sal_uLong)0x00000200)
#define TEXT_LAYOUT_SUBSTITUTE_DIGITS   ((sal_uLong)0x00000400)

// DrawModes
#define DRAWMODE_DEFAULT                ((sal_uLong)0x00000000)
#define DRAWMODE_BLACKLINE              ((sal_uLong)0x00000001)
#define DRAWMODE_BLACKFILL              ((sal_uLong)0x00000002)
#define DRAWMODE_BLACKTEXT              ((sal_uLong)0x00000004)
#define DRAWMODE_BLACKBITMAP            ((sal_uLong)0x00000008)
#define DRAWMODE_BLACKGRADIENT          ((sal_uLong)0x00000010)
#define DRAWMODE_GRAYLINE               ((sal_uLong)0x00000020)
#define DRAWMODE_GRAYFILL               ((sal_uLong)0x00000040)
#define DRAWMODE_GRAYTEXT               ((sal_uLong)0x00000080)
#define DRAWMODE_GRAYBITMAP             ((sal_uLong)0x00000100)
#define DRAWMODE_GRAYGRADIENT           ((sal_uLong)0x00000200)
#define DRAWMODE_NOFILL                 ((sal_uLong)0x00000400)
#define DRAWMODE_NOBITMAP               ((sal_uLong)0x00000800)
#define DRAWMODE_NOGRADIENT             ((sal_uLong)0x00001000)
#define DRAWMODE_GHOSTEDLINE            ((sal_uLong)0x00002000)
#define DRAWMODE_GHOSTEDFILL            ((sal_uLong)0x00004000)
#define DRAWMODE_GHOSTEDTEXT            ((sal_uLong)0x00008000)
#define DRAWMODE_GHOSTEDBITMAP          ((sal_uLong)0x00010000)
#define DRAWMODE_GHOSTEDGRADIENT        ((sal_uLong)0x00020000)
#define DRAWMODE_WHITELINE              ((sal_uLong)0x00100000)
#define DRAWMODE_WHITEFILL              ((sal_uLong)0x00200000)
#define DRAWMODE_WHITETEXT              ((sal_uLong)0x00400000)
#define DRAWMODE_WHITEBITMAP            ((sal_uLong)0x00800000)
#define DRAWMODE_WHITEGRADIENT          ((sal_uLong)0x01000000)
#define DRAWMODE_SETTINGSLINE           ((sal_uLong)0x02000000)
#define DRAWMODE_SETTINGSFILL           ((sal_uLong)0x04000000)
#define DRAWMODE_SETTINGSTEXT           ((sal_uLong)0x08000000)
#define DRAWMODE_SETTINGSGRADIENT       ((sal_uLong)0x10000000)
#define DRAWMODE_NOTRANSPARENCY         ((sal_uLong)0x80000000)

// Antialiasing
#define ANTIALIASING_DISABLE_TEXT       ((sal_uInt16)0x0001)
#define ANTIALIASING_ENABLE_B2DDRAW     ((sal_uInt16)0x0002)
#define ANTIALIASING_PIXELSNAPHAIRLINE  ((sal_uInt16)0x0004)

// AddFontSubstitute
#define FONT_SUBSTITUTE_ALWAYS          ((sal_uInt16)0x0001)
#define FONT_SUBSTITUTE_SCREENONLY      ((sal_uInt16)0x0002)

#define DEFAULTFONT_FLAGS_ONLYONE       ((sal_uLong)0x00000001)

enum OutDevType { OUTDEV_DONTKNOW, OUTDEV_WINDOW, OUTDEV_PRINTER, OUTDEV_VIRDEV };

enum OutDevViewType { OUTDEV_VIEWTYPE_DONTKNOW, OUTDEV_VIEWTYPE_PRINTPREVIEW, OUTDEV_VIEWTYPE_SLIDESHOW };

// OutputDevice

class VirtualDevice;
class Printer;
class FontSelectPattern;
class ImplFontMetricData;
class VCLXGraphics;

typedef ::std::vector< VCLXGraphics* > VCLXGraphicsList_impl;

const char* ImplDbgCheckOutputDevice( const void* pObj );

class VCL_DLLPUBLIC OutputDevice
{
    friend class Application;
    friend class Bitmap;
    friend class ImplImageBmp;
    friend class Printer;
    friend class System;
    friend class VirtualDevice;
    friend class Window;
    friend class WorkWindow;
    friend class vcl::PDFWriterImpl;
    friend const char* ImplDbgCheckOutputDevice( const void* pObj );
    friend void ImplHandleResize( Window* pWindow, long nNewWidth, long nNewHeight );

private:
    mutable SalGraphics*            mpGraphics;         ///< Graphics context to draw on
    mutable OutputDevice*           mpPrevGraphics;     ///< Previous output device in list
    mutable OutputDevice*           mpNextGraphics;     ///< Next output device in list
    GDIMetaFile*                    mpMetaFile;
    mutable ImplFontEntry*          mpFontEntry;
    mutable ImplFontCache*          mpFontCache;
    mutable PhysicalFontCollection* mpFontCollection;
    mutable ImplGetDevFontList*     mpGetDevFontList;
    mutable ImplGetDevSizeList*     mpGetDevSizeList;
    ImplObjStack*                   mpObjStack;
    ImplOutDevData*                 mpOutDevData;
    VCLXGraphicsList_impl*          mpUnoGraphicsList;
    vcl::PDFWriterImpl*             mpPDFWriter;
    vcl::ExtOutDevData*             mpExtOutDevData;

    // TEMP TEMP TEMP
    VirtualDevice*                  mpAlphaVDev;

    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    long                            mnOutOffOrigX;
    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    long                            mnOutOffLogicX;
    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    long                            mnOutOffOrigY;
    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    long                            mnOutOffLogicY;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    long                            mnOutOffX;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    long                            mnOutOffY;
    long                            mnOutWidth;
    long                            mnOutHeight;
    sal_Int32                       mnDPIX;
    sal_Int32                       mnDPIY;
    sal_Int32                       mnDPIScaleFactor; ///< For Hi-DPI displays, we want to draw everything mnDPIScaleFactor-times larger
    /// font specific text alignment offsets in pixel units
    mutable long                    mnTextOffX;
    mutable long                    mnTextOffY;
    mutable long                    mnEmphasisAscent;
    mutable long                    mnEmphasisDescent;
    sal_uLong                       mnDrawMode;
    sal_uLong                       mnTextLayoutMode;
    ImplMapRes                      maMapRes;
    ImplThresholdRes                maThresRes;
    OutDevType                      meOutDevType;
    OutDevViewType                  meOutDevViewType;
    Region                          maRegion;           // contains the clip region, see SetClipRegion(...)
    Color                           maLineColor;
    Color                           maFillColor;
    Font                            maFont;
    Color                           maTextColor;
    Color                           maTextLineColor;
    Color                           maOverlineColor;
    TextAlign                       meTextAlign;
    RasterOp                        meRasterOp;
    Wallpaper                       maBackground;
    boost::scoped_ptr<AllSettings>  mxSettings;
    MapMode                         maMapMode;
    Point                           maRefPoint;
    sal_uInt16                      mnAntialiasing;
    LanguageType                    meTextLanguage;

    /// bitfield
    mutable bool                    mbMap : 1;
    mutable bool                    mbMapIsDefault : 1;
    mutable bool                    mbClipRegion : 1;
    mutable bool                    mbBackground : 1;
    mutable bool                    mbOutput : 1;
    mutable bool                    mbDevOutput : 1;
    mutable bool                    mbOutputClipped : 1;
    mutable bool                    mbLineColor : 1;
    mutable bool                    mbFillColor : 1;
    mutable bool                    mbInitLineColor : 1;
    mutable bool                    mbInitFillColor : 1;
    mutable bool                    mbInitFont : 1;
    mutable bool                    mbInitTextColor : 1;
    mutable bool                    mbInitClipRegion : 1;
    mutable bool                    mbClipRegionSet : 1;
    mutable bool                    mbKerning : 1;
    mutable bool                    mbNewFont : 1;
    mutable bool                    mbTextLines : 1;
    mutable bool                    mbTextSpecial : 1;
    mutable bool                    mbRefPoint : 1;
    mutable bool                    mbEnableRTL : 1;


protected:
     virtual void                   ImplReleaseFonts();

public:
    /** @name Initialization and accessor functions
     */
    ///@{

    /** Get the graphic context that the output device uses to draw on.

     If no graphics device exists, then initialize it.

     @returns SalGraphics instance.
     */
    SAL_DLLPRIVATE SalGraphics const *ImplGetGraphics() const;
    SAL_DLLPRIVATE SalGraphics*     ImplGetGraphics();

    /** Initialize the graphics device that the output device uses to draw on.

     There is an LRU of OutputDevices that is used to get the graphics. The
     actual creation of a SalGraphics instance is done via the SalFrame
     implementation.

     However, the SalFrame instance will only return a valid SalGraphics
     instance if it is not in use or there wasn't one in the first place. When
     this happens, ImplInitGraphics finds the least recently used OutputDevice
     in a different frame and "steals" it (releases it then starts using it).

     If there are no frames to steal an OutputDevice's SalGraphics instance from
     then it blocks until the graphics is released.

     Once it has acquired a graphics instance, then we add the OutputDevice to
     the LRU.

     @returns true if was able to initialize the graphics device, false otherwise.
     */
    virtual bool                ImplInitGraphics() const = 0;

    /** Release the graphics device, and remove it from the graphics device
     list.

     @param         bRelease    Determines whether to release the fonts of the
                                physically released graphics device.
     */
    virtual void                ImplReleaseGraphics( bool bRelease = true ) = 0;

    /** Initialize the graphics device's data structures.
     */
    SAL_DLLPRIVATE void         ImplInitOutDevData();

    /** De-initialize the graphics device's data structures.
     */
    SAL_DLLPRIVATE void         ImplDeInitOutDevData();
    ///@}


    /** @name Helper functions
     */
    ///@{

    /** Get the output device's DPI x-axis value.

     @returns x-axis DPI value
     */
    SAL_DLLPRIVATE sal_Int32    ImplGetDPIX() const { return mnDPIX; }

    /** Get the output device's DPI y-axis value.

     @returns y-axis DPI value
     */
    SAL_DLLPRIVATE sal_Int32    ImplGetDPIY() const { return mnDPIY; }

    /** Convert a logical X coordinate to a device pixel's X coordinate.

     To get the device's X coordinate, it must calculate the mapping offset
     coordinate X position (if there is one - if not then it just adds
     the pseudo-window offset to the logical X coordinate), the X-DPI of
     the device and the mapping's X scaling factor.

     @param         nX          Logical X coordinate

     @returns Device's X pixel coordinate
     */
    SAL_DLLPRIVATE long         ImplLogicXToDevicePixel( long nX ) const;

    /** Convert a logical Y coordinate to a device pixel's Y coordinate.

     To get the device's Y coordinate, it must calculate the mapping offset
     coordinate Y position (if there is one - if not then it just adds
     the pseudo-window offset to the logical Y coordinate), the Y-DPI of
     the device and the mapping's Y scaling factor.

     @param         nY          Logical Y coordinate

     @returns Device's Y pixel coordinate
     */
    SAL_DLLPRIVATE long         ImplLogicYToDevicePixel( long nY ) const;

    /** Convert a logical width to a width in units of device pixels.

     To get the number of device pixels, it must calculate the X-DPI of the device and
     the map scaling factor. If there is no mapping, then it just returns the
     width as nothing more needs to be done.

     @param         nWidth      Logical width

     @returns Width in units of device pixels.
     */
    SAL_DLLPRIVATE long         ImplLogicWidthToDevicePixel( long nWidth ) const;

    /** Convert a logical height to a height in units of device pixels.

     To get the number of device pixels, it must calculate the Y-DPI of the device and
     the map scaling factor. If there is no mapping, then it just returns the
     height as nothing more needs to be done.

     @param         nHeight     Logical height

     @returns Height in units of device pixels.
     */
    SAL_DLLPRIVATE long         ImplLogicHeightToDevicePixel( long nHeight ) const;

    /** Convert device pixels to a width in logical units.

     To get the logical width, it must calculate the X-DPI of the device and the
     map scaling factor.

     @param         nWidth      Width in device pixels

     @returns Width in logical units.
     */
    SAL_DLLPRIVATE long         ImplDevicePixelToLogicWidth( long nWidth ) const;

    /** Convert device pixels to a height in logical units.

     To get the logical height, it must calculate the Y-DPI of the device and the
     map scaling factor.

     @param         nHeight     Height in device pixels

     @returns Height in logical units.
     */
    SAL_DLLPRIVATE long         ImplDevicePixelToLogicHeight( long nHeight ) const;

    /** Convert logical height to device pixels, with exact sub-pixel value.

     To get the \em exact pixel height, it must calculate the Y-DPI of the device and the
     map scaling factor.

     @param         nHeight     Exact height in logical units.

     @returns Exact height in pixels - returns as a float to provide for subpixel value.
     */
    SAL_DLLPRIVATE float        ImplFloatLogicHeightToDevicePixel( float ) const;

    /** Convert a logical point to a physical point on the device.

     @param         rLogicPt    Const reference to a point in logical units.

     @returns Physical point on the device.
     */
    SAL_DLLPRIVATE Point        ImplLogicToDevicePixel( const Point& rLogicPt ) const;

    /** Convert a logical size to the size on the physical device.

     @param         rLogicSize  Const reference to a size in logical units

     @returns Physical size on the device.
     */
    SAL_DLLPRIVATE Size         ImplLogicToDevicePixel( const Size& rLogicSize ) const;

    /** Convert a logical rectangle to a rectangle in physical device pixel units.

     @param         rLogicSize  Const reference to a rectangle in logical units

     @returns Rectangle based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE Rectangle    ImplLogicToDevicePixel( const Rectangle& rLogicRect ) const;

    /** Convert a rectangle in physical pixel units to a rectangle in physical pixel units and coords.

     @param         rPixelRect  Const reference to rectangle in logical units and coords.

     @returns Rectangle based on logical coordinates and units.
     */
    SAL_DLLPRIVATE Rectangle    ImplDevicePixelToLogic( const Rectangle& rPixelRect ) const;

    /** Convert a logical B2DPolygon to a B2DPolygon in physical device pixel units.

     @param         rLogicSize  Const reference to a B2DPolygon in logical units

     @returns B2DPolyPolygon based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE ::basegfx::B2DPolygon ImplLogicToDevicePixel( const ::basegfx::B2DPolygon& rLogicPoly ) const;

    /** Convert a logical B2DPolyPolygon to a B2DPolyPolygon in physical device pixel units.

     @param         rLogicPolyPoly  Const reference to a B2DPolyPolygon in logical units

     @returns B2DPolyPolygon based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE ::basegfx::B2DPolyPolygon ImplLogicToDevicePixel( const ::basegfx::B2DPolyPolygon& rLogicPolyPoly ) const;

    /** Convert a logical polygon to a polygon in physical device pixel units.

     @param         rLogicPoly  Const reference to a polygon in logical units

     @returns Polygon based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE Polygon      ImplLogicToDevicePixel( const Polygon& rLogicPoly ) const;

    /** Convert a logical polypolygon to a polypolygon in physical device pixel units.

     @param         rLogicPolyPoly  Const reference to a polypolygon in logical units

     @returns Polypolygon based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE PolyPolygon  ImplLogicToDevicePixel( const PolyPolygon& rLogicPolyPoly ) const;

    /** Convert a line in logical units to a line in physical device pixel units.

     @param         rLineInfo   Const refernece to a line in logical units

     @returns Line based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE LineInfo     ImplLogicToDevicePixel( const LineInfo& rLineInfo ) const;

    /** Convert a region in pixel units to a region in device pixel units and coords.

     @param         rPixelRect  Const reference to region.

     @returns Region based on device pixel coordinates and units.
     */
    SAL_DLLPRIVATE Region       ImplPixelToDevicePixel( const Region& rRegion ) const;

    /** Invalidate the view transformation.

     @since AOO bug 75163 (OpenOffice.org 2.4.3 - OOH 680 milestone 212)
     */
    SAL_DLLPRIVATE void         ImplInvalidateViewTransform();

    /** Get device transformation.

     @since AOO bug 75163 (OpenOffice.org 2.4.3 - OOH 680 milestone 212)
     */
    SAL_DLLPRIVATE basegfx::B2DHomMatrix ImplGetDeviceTransformation() const;
    ///@}

    /** @name Clipping functions
     */
    ///@{

    SAL_DLLPRIVATE void         ImplInitClipRegion();
    SAL_DLLPRIVATE bool         ImplSelectClipRegion( const Region&, SalGraphics* pGraphics = NULL );
    SAL_DLLPRIVATE void         ImplSetClipRegion( const Region* pRegion );

    ///@}

    /** @name Text and font functions
     */
    ///@{
    SAL_DLLPRIVATE bool         ImplNewFont() const;
    SAL_DLLPRIVATE void         ImplInitFont() const;
    SAL_DLLPRIVATE void         ImplInitTextColor();

    static
    SAL_DLLPRIVATE void         ImplDrawText( OutputDevice& rTargetDevice, const Rectangle& rRect,
                                              const OUString& rOrigStr, sal_uInt16 nStyle,
                                              MetricVector* pVector, OUString* pDisplayText, ::vcl::ITextLayout& _rLayout );
    SAL_DLLPRIVATE void         ImplDrawTextBackground( const SalLayout& );
    SAL_DLLPRIVATE void         ImplDrawTextLines( SalLayout&, FontStrikeout eStrikeout, FontUnderline eUnderline, FontUnderline eOverline, bool bWordLine, bool bUnderlineAbove );
    SAL_DLLPRIVATE bool         ImplDrawRotateText( SalLayout& );
    SAL_DLLPRIVATE bool         ImplDrawTextDirect( SalLayout&, bool bTextLines, sal_uInt32 flags = 0 );
    SAL_DLLPRIVATE void         ImplDrawSpecialText( SalLayout& );
    SAL_DLLPRIVATE void         ImplDrawText( SalLayout& );
    SAL_DLLPRIVATE Rectangle    ImplGetTextBoundRect( const SalLayout& );
    SAL_DLLPRIVATE void         ImplDrawEmphasisMarks( SalLayout& );
    static
    SAL_DLLPRIVATE OUString     ImplGetEllipsisString( const OutputDevice& rTargetDevice, const OUString& rStr,
                                                       long nMaxWidth, sal_uInt16 nStyle, const ::vcl::ITextLayout& _rLayout );

    SAL_DLLPRIVATE void         ImplDrawTextRect( long nBaseX, long nBaseY, long nX, long nY, long nWidth, long nHeight );

    SAL_DLLPRIVATE void         ImplInitTextLineSize();
    SAL_DLLPRIVATE void         ImplInitAboveTextLineSize();
    SAL_DLLPRIVATE void         ImplDrawWaveLine( long nBaseX, long nBaseY, long nStartX, long nStartY, long nWidth, long nHeight, long nLineWidth, short nOrientation, const Color& rColor );
    SAL_DLLPRIVATE void         ImplDrawWaveTextLine( long nBaseX, long nBaseY, long nX, long nY, long nWidth, FontUnderline eTextLine, Color aColor, bool bIsAbove );
    SAL_DLLPRIVATE void         ImplDrawStraightTextLine( long nBaseX, long nBaseY, long nX, long nY, long nWidth, FontUnderline eTextLine, Color aColor, bool bIsAbove );
    SAL_DLLPRIVATE void         ImplDrawStrikeoutLine( long nBaseX, long nBaseY, long nX, long nY, long nWidth, FontStrikeout eStrikeout, Color aColor );
    SAL_DLLPRIVATE void         ImplDrawStrikeoutChar( long nBaseX, long nBaseY, long nX, long nY, long nWidth, FontStrikeout eStrikeout, Color aColor );
    SAL_DLLPRIVATE void         ImplDrawTextLine( long nBaseX, long nX, long nY, long nWidth, FontStrikeout eStrikeout, FontUnderline eUnderline, FontUnderline eOverline, bool bUnderlineAbove );
    SAL_DLLPRIVATE void         ImplDrawMnemonicLine( long nX, long nY, long nWidth );
    SAL_DLLPRIVATE static bool  ImplIsUnderlineAbove( const Font& );

    SAL_DLLPRIVATE static       FontEmphasisMark ImplGetEmphasisMarkStyle( const Font& rFont );
    SAL_DLLPRIVATE void         ImplGetEmphasisMark( PolyPolygon& rPolyPoly, bool& rPolyLine, Rectangle& rRect1, Rectangle& rRect2, long& rYOff, long& rWidth, FontEmphasisMark eEmphasis, long nHeight, short nOrient );
    SAL_DLLPRIVATE void         ImplDrawEmphasisMark( long nBaseX, long nX, long nY, const PolyPolygon& rPolyPoly, bool bPolyLine, const Rectangle& rRect1, const Rectangle& rRect2 );
    static
    SAL_DLLPRIVATE long         ImplGetTextLines( ImplMultiTextLineInfo& rLineInfo, long nWidth, const OUString& rStr, sal_uInt16 nStyle, const ::vcl::ITextLayout& _rLayout );
    SAL_DLLPRIVATE void         ImplInitFontList() const;
    SAL_DLLPRIVATE void         ImplUpdateFontData( bool bNewFontLists );
    SAL_DLLPRIVATE static void  ImplUpdateAllFontData( bool bNewFontLists );
    ///@}

    /** @name Line functions
     */
    ///@{
    SAL_DLLPRIVATE void         ImplInitLineColor();
    SAL_DLLPRIVATE void         ImplInitFillColor();

    // #i101491#
    // Helper which holds the old line geometry creation and is extended to use AA when
    // switched on. Advantage is that line geometry is only temporarily used for paint
    SAL_DLLPRIVATE void         ImplDrawPolyLineWithLineInfo(const Polygon& rPoly, const LineInfo& rLineInfo);

    // #i101491#
    // Helper who tries to use SalGDI's DrawPolyLine direct and returns it's bool. Contains no AA check.
    SAL_DLLPRIVATE bool         ImplTryDrawPolyLineDirect(
                                    const basegfx::B2DPolygon& rB2DPolygon,
                                    double fLineWidth = 0.0,
                                    double fTransparency = 0.0,
                                    basegfx::B2DLineJoin eLineJoin = basegfx::B2DLINEJOIN_NONE,
                                    css::drawing::LineCap eLineCap = css::drawing::LineCap_BUTT);

    // Helper for line geometry paint with support for graphic expansion (pattern and fat_to_area)
    void                        ImplPaintLineGeometryWithEvtlExpand(const LineInfo& rInfo, basegfx::B2DPolyPolygon aLinePolyPolygon);
    ///@}

    /** @name Polygon functions
     */
    ///@{
    SAL_DLLPRIVATE void         ImplDrawPolygon( const Polygon& rPoly, const PolyPolygon* pClipPolyPoly = NULL );
    SAL_DLLPRIVATE void         ImplDrawPolyPolygon( const PolyPolygon& rPolyPoly, const PolyPolygon* pClipPolyPoly = NULL );
    SAL_DLLPRIVATE void         ImplDrawPolyPolygon( sal_uInt16 nPoly, const PolyPolygon& rPolyPoly );
    // #i101491#
    // Helper who implements the DrawPolyPolygon functionality for basegfx::B2DPolyPolygon
    // without MetaFile processing
    SAL_DLLPRIVATE void         ImplDrawPolyPolygonWithB2DPolyPolygon(const basegfx::B2DPolyPolygon& rB2DPolyPoly);


    ///@}

    /** @name Gradient functions
     */
    ///@{
    SAL_DLLPRIVATE void         ImplDrawLinearGradient( const Rectangle& rRect, const Gradient& rGradient, bool bMtf, const PolyPolygon* pClipPolyPoly );
    SAL_DLLPRIVATE void         ImplDrawComplexGradient( const Rectangle& rRect, const Gradient& rGradient, bool bMtf, const PolyPolygon* pClipPolyPoly );
    ///@}

    /** @name Hatch functions
     */
    ///@{
    SAL_DLLPRIVATE void         ImplDrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch, bool bMtf );
    SAL_DLLPRIVATE void         ImplCalcHatchValues( const Rectangle& rRect, long nDist, sal_uInt16 nAngle10, Point& rPt1, Point& rPt2, Size& rInc, Point& rEndPt1 );
    SAL_DLLPRIVATE void         ImplDrawHatchLine( const Line& rLine, const PolyPolygon& rPolyPoly, Point* pPtBuffer, bool bMtf );
    ///@}

    /** @name Wallpaper functions
     */
    ///@{
    SAL_DLLPRIVATE void         ImplDrawWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         ImplDrawColorWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         ImplDrawBitmapWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         ImplDrawGradientWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    ///@}

    /** @name Bitmap functions
     */
    ///@{
    SAL_DLLPRIVATE void         ImplDrawOutDevDirect ( const OutputDevice* pSrcDev, SalTwoRect& rPosAry );
    SAL_DLLPRIVATE void         ImplDrawBitmap       ( const Point& rDestPt, const Size& rDestSize,
                                                       const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                                       const Bitmap& rBitmap, const sal_uLong nAction );
    SAL_DLLPRIVATE void         ImplDrawBitmapEx     ( const Point& rDestPt, const Size& rDestSize,
                                                       const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                                       const BitmapEx& rBitmapEx, const sal_uLong nAction );
    SAL_DLLPRIVATE void         ImplDrawAlpha        ( const Bitmap& rBmp, const AlphaMask& rAlpha,
                                                       const Point& rDestPt, const Size& rDestSize,
                                                       const Point& rSrcPtPixel, const Size& rSrcSizePixel );
    virtual void                ClipToPaintRegion    ( Rectangle& rDstRect );
    SAL_DLLPRIVATE Bitmap       ImplBlend            ( Bitmap              aBmp,
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
    SAL_DLLPRIVATE Bitmap       ImplBlendWithAlpha   ( Bitmap              aBmp,
                                                       BitmapReadAccess*   pP,
                                                       BitmapReadAccess*   pA,
                                                       const Rectangle&    aDstRect,
                                                       const sal_Int32     nOffY,
                                                       const sal_Int32     nDstHeight,
                                                       const sal_Int32     nOffX,
                                                       const sal_Int32     nDstWidth,
                                                       const long*         pMapX,
                                                       const long*         pMapY );
    SAL_DLLPRIVATE void         ImplPrintTransparent ( const Bitmap& rBmp, const Bitmap& rMask,
                                                       const Point& rDestPt, const Size& rDestSize,
                                                       const Point& rSrcPtPixel, const Size& rSrcSizePixel );
    SAL_DLLPRIVATE Color        ImplDrawModeToColor  ( const Color& rColor ) const;
    ///@}

    /** @name Frame functions
     */
    ///@{
    SAL_DLLPRIVATE void         ImplGetFrameDev     ( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                                      OutputDevice& rOutDev );
    SAL_DLLPRIVATE void         ImplDrawFrameDev    ( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                                      const OutputDevice& rOutDev, const Region& rRegion );
    ///@}

    /** @name Layout functions
     */
    ///@{
    SAL_DLLPRIVATE bool         ImplIsRecordLayout() const;
    virtual bool                HasMirroredGraphics() const;
    SAL_DLLPRIVATE void         ReMirror( Point &rPoint ) const;
    SAL_DLLPRIVATE void         ReMirror( Rectangle &rRect ) const;
    SAL_DLLPRIVATE void         ReMirror( Region &rRegion ) const;
    SAL_DLLPRIVATE SalLayout*   ImplLayout( const OUString&, sal_Int32 nIndex, sal_Int32 nLen,
                                            const Point& rLogicPos = Point(0,0), long nLogicWidth=0,
                                            const sal_Int32* pLogicDXArray=NULL ) const;
    SAL_DLLPRIVATE ImplLayoutArgs ImplPrepareLayoutArgs( OUString&, const sal_Int32 nIndex, const sal_Int32 nLen,
                                            long nPixelWidth, const sal_Int32* pPixelDXArray ) const;
    SAL_DLLPRIVATE SalLayout*   ImplGlyphFallbackLayout( SalLayout*, ImplLayoutArgs& ) const;
    // tells whether this output device is RTL in an LTR UI or LTR in a RTL UI
    SAL_DLLPRIVATE bool         ImplIsAntiparallel() const ;
    SAL_DLLPRIVATE SalLayout*   getFallbackFont(ImplFontEntry &rFallbackFont,
                                    FontSelectPattern &rFontSelData, int nFallbackLevel,
                                    ImplLayoutArgs& rLayoutArgs) const;

    ///@}

protected:
                                OutputDevice();

    virtual void                ImplPrintMask ( const Bitmap& rMask, const Color& rMaskColor,
                                                const Point& rDestPt, const Size& rDestSize,
                                                const Point& rSrcPtPixel, const Size& rSrcSizePixel );

    virtual long                ImplGetGradientStepCount( long nMinRect );
    virtual bool                UsePolyPolygonForComplexGradient() = 0;

    /** Transform and draw a bitmap directly

     @param     aFullTransform      The B2DHomMatrix used for the transformation
     @param     rBitmapEx           Reference to the bitmap to be transformed and drawn

     @return true if it was able to draw the bitmap, false if not
     */
    virtual bool                DrawTransformBitmapExDirect(
                                    const basegfx::B2DHomMatrix& aFullTransform,
                                    const BitmapEx& rBitmapEx);

    /** Reduce the area that needs to be drawn of the bitmap and return the new
        visible range and the maximum area.


      @param     aFullTransform      B2DHomMatrix used for transformation
      @param     aVisibleRange       The new visible area of the bitmap
      @param     fMaximumArea        The maximum area of the bitmap

      @returns true if there is an area to be drawn, otherwise nothing is left to be drawn
        so return false
      */
    virtual bool                TransformReduceBitmapExTargetRange(
                                    const basegfx::B2DHomMatrix& aFullTransform,
                                    basegfx::B2DRange &aVisibleRange,
                                    double &fMaximumArea);

    virtual void                ScaleBitmap ( Bitmap &rBmp, SalTwoRect &rPosAry );

    virtual void                DrawDeviceBitmap(
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                    BitmapEx& rBitmapEx );

    virtual void                EmulateDrawTransparent( const PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent );
    void                        DrawInvisiblePolygon( const PolyPolygon& rPolyPoly );

    virtual void                ClipAndDrawGradientToBounds( Gradient &rGradient, const PolyPolygon &rPolyPoly );
    void                        ClipAndDrawGradient( Gradient &rGradient, const PolyPolygon &rPolyPoly );
    void                        XORClipAndDrawGradient( Gradient &rGradient, const PolyPolygon &rPolyPoly );

    virtual void                ClipAndDrawGradientMetafile ( const Gradient &rGradient, const PolyPolygon &rPolyPoly );

private:
    typedef void ( OutputDevice::* FontUpdateHandler_t )( bool );

    SAL_DLLPRIVATE              OutputDevice( const OutputDevice& rOutDev );
    SAL_DLLPRIVATE OutputDevice& operator =( const OutputDevice& rOutDev );

    SAL_DLLPRIVATE void         ImplClearFontData( bool bNewFontLists );
    SAL_DLLPRIVATE void         ImplRefreshFontData( bool bNewFontLists );
    SAL_DLLPRIVATE static void  ImplUpdateFontDataForAllFrames( FontUpdateHandler_t pHdl, bool bNewFontLists );
    SAL_DLLPRIVATE long         ImplGetGradientSteps( const Gradient& rGradient, const Rectangle& rRect, bool bMtf, bool bComplex=false );

    // not implemented; to detect misuses of DrawOutDev(...OutputDevice&);
    void                        DrawOutDev( const Point&, const Size&, const Point&,  const Size&, const Printer&);

    bool                        DrawTransparentNatively( const PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent );

    Color                       GetSingleColorGradientFill();
    void                        SetGrayscaleColors( Gradient &rGradient );

public:
    virtual                     ~OutputDevice();

    OutDevType                  GetOutDevType() const { return meOutDevType; }

    /** query an <code>OutputDevice</code> to see whether it supports a specific operation

    @return
    true if operation supported, else false
    */
    bool                        supportsOperation( OutDevSupportType ) const;

    sal_Int32                   GetDPIScaleFactor() const { return mnDPIScaleFactor; }

    vcl::PDFWriterImpl*         GetPDFWriter() const { return mpPDFWriter; }

    void                        SetExtOutDevData( vcl::ExtOutDevData* pExtOutDevData ) { mpExtOutDevData = pExtOutDevData; }
    vcl::ExtOutDevData*         GetExtOutDevData() const { return mpExtOutDevData; }

    void                        DrawTextLine( const Point& rPos, long nWidth,
                                              FontStrikeout eStrikeout,
                                              FontUnderline eUnderline,
                                              FontUnderline eOverline,
                                              bool bUnderlineAbove = false );

    void                        DrawText( const Point& rStartPt, const OUString& rStr,
                                          sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                          MetricVector* pVector = NULL, OUString* pDisplayText = NULL );
    long                        GetTextWidth( const OUString& rStr, sal_Int32 nIndex = 0, sal_Int32 nLen = -1 ) const;
    /// Height where any character of the current font fits; in logic coordinates.
    long                        GetTextHeight() const;
    float                       approximate_char_width() const;
    void                        DrawTextArray( const Point& rStartPt, const OUString& rStr,
                                               const sal_Int32* pDXAry = NULL,
                                               sal_Int32 nIndex = 0,
                                               sal_Int32 nLen = -1 );
    long                        GetTextArray( const OUString& rStr, sal_Int32* pDXAry = NULL,
                                              sal_Int32 nIndex = 0, sal_Int32 nLen = -1 ) const;
    bool                        GetCaretPositions( const OUString&, sal_Int32* pCaretXArray,
                                              sal_Int32 nIndex, sal_Int32 nLen,
                                              sal_Int32* pDXAry = NULL, long nWidth = 0,
                                              bool bCellBreaking = true ) const;
    void                        DrawStretchText( const Point& rStartPt, sal_uLong nWidth,
                                                 const OUString& rStr,
                                                 sal_Int32 nIndex = 0, sal_Int32 nLen = -1 );
    sal_Int32                   GetTextBreak( const OUString& rStr, long nTextWidth,
                                              sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                              long nCharExtra = 0 ) const;
    sal_Int32                   GetTextBreak( const OUString& rStr, long nTextWidth,
                                              sal_Unicode nExtraChar, sal_Int32& rExtraCharPos,
                                              sal_Int32 nIndex, sal_Int32 nLen,
                                              long nCharExtra = 0 ) const;
    /** Generate MetaTextActions for the text rect

        This method splits up the text rect into multiple
        MetaTextActions, one for each line of text. This is comparable
        to AddGradientActions(), which splits up a gradient into its
        constituent polygons. Parameter semantics fully compatible to
        DrawText().
     */
    void                        AddTextRectActions( const Rectangle& rRect,
                                                    const OUString&    rOrigStr,
                                                    sal_uInt16           nStyle,
                                                    GDIMetaFile&     rMtf );
    void                        DrawText( const Rectangle& rRect,
                                          const OUString& rStr, sal_uInt16 nStyle = 0,
                                          MetricVector* pVector = NULL, OUString* pDisplayText = NULL,
                                          ::vcl::ITextLayout* _pTextLayout = NULL );
    Rectangle                   GetTextRect( const Rectangle& rRect,
                                             const OUString& rStr, sal_uInt16 nStyle = TEXT_DRAW_WORDBREAK,
                                             TextRectInfo* pInfo = NULL,
                                             const ::vcl::ITextLayout* _pTextLayout = NULL ) const;
    OUString                    GetEllipsisString( const OUString& rStr, long nMaxWidth,
                                                   sal_uInt16 nStyle = TEXT_DRAW_ENDELLIPSIS ) const;
    void                        DrawCtrlText( const Point& rPos, const OUString& rStr,
                                              sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                              sal_uInt16 nStyle = TEXT_DRAW_MNEMONIC, MetricVector* pVector = NULL, OUString* pDisplayText = NULL );
    long                        GetCtrlTextWidth( const OUString& rStr, sal_Int32 nIndex = 0,
                                                  sal_Int32 nLen = -1,
                                                  sal_uInt16 nStyle = TEXT_DRAW_MNEMONIC ) const;
    static OUString             GetNonMnemonicString( const OUString& rStr, sal_Int32& rMnemonicPos );
    static OUString             GetNonMnemonicString( const OUString& rStr )
                                            { sal_Int32 nDummy; return GetNonMnemonicString( rStr, nDummy ); }

    bool                        GetTextBoundRect( Rectangle& rRect,
                                                  const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                                  sal_uLong nLayoutWidth = 0, const sal_Int32* pDXArray = NULL ) const;

    bool                        GetTextOutline( PolyPolygon&,
                                                const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0,
                                                sal_Int32 nLen = -1, bool bOptimize = true,
                                                sal_uLong nLayoutWidth = 0, const sal_Int32* pDXArray = NULL ) const;

    bool                        GetTextOutlines( PolyPolyVector&,
                                                 const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0,
                                                 sal_Int32 nLen = -1, bool bOptimize = true,
                                                 sal_uLong nLayoutWidth = 0, const sal_Int32* pDXArray = NULL ) const;

    bool                        GetTextOutlines( ::basegfx::B2DPolyPolygonVector &rVector,
                                                 const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0,
                                                 sal_Int32 nLen = -1, bool bOptimize = true,
                                                 sal_uLong nLayoutWidth = 0, const sal_Int32* pDXArray = NULL ) const;

    bool                        GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex,
                                                    int nLen, int nBase, MetricVector& rVector );

    void                        DrawPixel( const Point& rPt );
    void                        DrawPixel( const Point& rPt, const Color& rColor );
    void                        DrawPixel( const Polygon& rPts, const Color* pColors = NULL );
    void                        DrawPixel( const Polygon& rPts, const Color& rColor );

    void                        DrawLine( const Point& rStartPt, const Point& rEndPt );
    void                        DrawLine( const Point& rStartPt, const Point& rEndPt,
                                          const LineInfo& rLineInfo );

    /** Render the given polygon as a line stroke

        The given polygon is stroked with the current LineColor, start
        and end point are not automatically connected

        @see DrawPolygon
        @see DrawPolyPolygon
     */
    void                        DrawPolyLine( const Polygon& rPoly );
    void                        DrawPolyLine(
                                    const basegfx::B2DPolygon&,
                                    double fLineWidth = 0.0,
                                    basegfx::B2DLineJoin = basegfx::B2DLINEJOIN_ROUND,
                                    css::drawing::LineCap = css::drawing::LineCap_BUTT);
    bool                        TryDrawPolyLineDirect(
                                    const basegfx::B2DPolygon& rB2DPolygon,
                                    double fLineWidth = 0.0,
                                    double fTransparency = 0.0,
                                    basegfx::B2DLineJoin eLineJoin = basegfx::B2DLINEJOIN_NONE,
                                    css::drawing::LineCap eLineCap = css::drawing::LineCap_BUTT);

    /** Render the given polygon as a line stroke

        The given polygon is stroked with the current LineColor, start
        and end point are not automatically connected. The line is
        rendered according to the specified LineInfo, e.g. supplying a
        dash pattern, or a line thickness.

        @see DrawPolygon
        @see DrawPolyPolygon
     */
    void                        DrawPolyLine( const Polygon& rPoly,
                                              const LineInfo& rLineInfo );

    /** Render the given polygon

        The given polygon is stroked with the current LineColor, and
        filled with the current FillColor. If one of these colors are
        transparent, the corresponding stroke or fill stays
        invisible. Start and end point of the polygon are
        automatically connected.

        @see DrawPolyLine
     */
    void                        DrawPolygon( const Polygon& rPoly );
    void                        DrawPolygon( const basegfx::B2DPolygon& );

    /** Render the given poly-polygon

        The given poly-polygon is stroked with the current LineColor,
        and filled with the current FillColor. If one of these colors
        are transparent, the corresponding stroke or fill stays
        invisible. Start and end points of the contained polygons are
        automatically connected.

        @see DrawPolyLine
     */
    void                        DrawPolyPolygon( const PolyPolygon& rPolyPoly );
    void                        DrawPolyPolygon( const basegfx::B2DPolyPolygon& );

    void                        DrawRect( const Rectangle& rRect );
    void                        DrawRect( const Rectangle& rRect,
                                          sal_uLong nHorzRount, sal_uLong nVertRound );
    void                        DrawEllipse( const Rectangle& rRect );
    void                        DrawArc( const Rectangle& rRect,
                                         const Point& rStartPt, const Point& rEndPt );
    void                        DrawPie( const Rectangle& rRect,
                                         const Point& rStartPt, const Point& rEndPt );
    void                        DrawChord( const Rectangle& rRect,
                                           const Point& rStartPt, const Point& rEndPt );

    virtual void                DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                                            const Point& rSrcPt,  const Size& rSrcSize );
    virtual void                DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                                            const Point& rSrcPt,  const Size& rSrcSize,
                                            const OutputDevice& rOutDev );
    virtual void                CopyArea( const Point& rDestPt,
                                          const Point& rSrcPt,  const Size& rSrcSize,
                                          sal_uInt16 nFlags = 0 );
protected:
    virtual void                CopyAreaFinal( SalTwoRect& aPosAry, sal_uInt32 nFlags);

public:
    void                        DrawBitmap( const Point& rDestPt,
                                            const Bitmap& rBitmap );
    void                        DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                            const Bitmap& rBitmap );
    void                        DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                            const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                            const Bitmap& rBitmap );

    void                        DrawBitmapEx( const Point& rDestPt,
                                              const BitmapEx& rBitmapEx );
    void                        DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                              const BitmapEx& rBitmapEx );
    void                        DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                              const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                              const BitmapEx& rBitmapEx );

    /** Draw BitampEx transformed

        @param rTransformation
        The transformation describing the target positioning of the given bitmap. Transforming
        the unit object coordinates (0, 0, 1, 1) with this matrix is the transformation to
        discrete coordinates

        @param rBitmapEx
        The BitmapEx to be painted
    */
    void                        DrawTransformedBitmapEx(
                                    const basegfx::B2DHomMatrix& rTransformation,
                                    const BitmapEx& rBitmapEx);

    void                        DrawMask( const Point& rDestPt,
                                          const Bitmap& rBitmap, const Color& rMaskColor );
    void                        DrawMask( const Point& rDestPt, const Size& rDestSize,
                                          const Bitmap& rBitmap, const Color& rMaskColor );
    void                        DrawMask( const Point& rDestPt, const Size& rDestSize,
                                          const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                          const Bitmap& rBitmap, const Color& rMaskColor,
                                          sal_uLong nAction );

    void                        DrawImage( const Point& rPos,
                                           const Image& rImage, sal_uInt16 nStyle = 0 );
    void                        DrawImage( const Point& rPos, const Size& rSize,
                                           const Image& rImage, sal_uInt16 nStyle = 0 );

    void                        DrawGradient( const Rectangle& rRect, const Gradient& rGradient );
    void                        DrawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient );
    void                        AddGradientActions( const Rectangle& rRect,
                                                    const Gradient& rGradient,
                                                    GDIMetaFile& rMtf );

#ifdef _MSC_VER
    void                        DrawHatch( const PolyPolygon& rPolyPoly, const ::Hatch& rHatch );
    void                        AddHatchActions( const PolyPolygon& rPolyPoly,
                                                 const ::Hatch& rHatch,
                                                 GDIMetaFile& rMtf );
#else
    void                        DrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch );
    void                        AddHatchActions( const PolyPolygon& rPolyPoly,
                                                 const Hatch& rHatch,
                                                 GDIMetaFile& rMtf );
#endif

    void                        DrawWallpaper( const Rectangle& rRect, const Wallpaper& rWallpaper );
    void                        DrawWaveLine( const Point& rStartPos, const Point& rEndPos );
    void                        DrawGrid( const Rectangle& rRect, const Size& rDist, sal_uLong nFlags );

    void                        DrawTransparent( const PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent );
    void                        DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency);
    void                        DrawTransparent( const GDIMetaFile& rMtf, const Point& rPos, const Size& rSize,
                                         const Gradient& rTransparenceGradient );

    /** Added return value to see if EPS could be painted directly.
        Theoreticaly, handing over a matrix would be needed to handle
        painting rotated EPS files (e.g. contained in Metafiles). This
        would then need to be supported for Mac and PS printers, but
        that's too much for now, wrote \#i107046# for this */
    bool                        DrawEPS( const Point& rPt, const Size& rSz,
                                         const GfxLink& rGfxLink, GDIMetaFile* pSubst = NULL );

    /// Fill the given rectangle with checkered rectangles of size nLen x nLen using the colors aStart and aEnd
    void                        DrawCheckered(
                                    const Point& rPos,
                                    const Size& rSize,
                                    sal_uInt32 nLen = 8,
                                    Color aStart = Color(COL_WHITE),
                                    Color aEnd = Color(COL_BLACK));

    Color                       GetPixel( const Point& rPt ) const;

    Bitmap                      GetBitmap( const Point& rSrcPt, const Size& rSize ) const;

    /** Query extended bitmap (with alpha channel, if available).
     */
    BitmapEx                    GetBitmapEx( const Point& rSrcPt, const Size& rSize ) const;

    void                        EnableMapMode( bool bEnable = true );
    bool                        IsMapModeEnabled() const { return mbMap; }

    // Enabling/disabling RTL only makes sense for OutputDevices that use a mirroring SalGraphisLayout
    virtual void                EnableRTL( bool bEnable = true);
    bool                        IsRTLEnabled() const { return mbEnableRTL; }

    void                        SetConnectMetaFile( GDIMetaFile* pMtf );
    GDIMetaFile*                GetConnectMetaFile() const { return mpMetaFile; }

    void                        EnableOutput( bool bEnable = true );
    bool                        IsOutputEnabled() const { return mbOutput; }
    bool                        IsDeviceOutput() const { return mbDevOutput; }
    bool                        IsDeviceOutputNecessary() const { return (mbOutput && mbDevOutput); }
    bool                        IsOutputNecessary() const { return ((mbOutput && mbDevOutput) || (mpMetaFile != NULL)); }

    void                        SetClipRegion();
    void                        SetClipRegion( const Region& rRegion );
    Region                      GetClipRegion() const;
    bool                        IsClipRegion() const { return mbClipRegion; }
    Region                      GetActiveClipRegion() const;

    void                        MoveClipRegion( long nHorzMove, long nVertMove );
    void                        IntersectClipRegion( const Rectangle& rRect );
    void                        IntersectClipRegion( const Region& rRegion );

    void                        SetAntialiasing( sal_uInt16 nMode =  0 );
    sal_uInt16                  GetAntialiasing() const { return mnAntialiasing; }

    void                        SetDrawMode( sal_uLong nDrawMode );
    sal_uLong                   GetDrawMode() const { return mnDrawMode; }

    void                        SetLayoutMode( sal_uLong nTextLayoutMode );
    sal_uLong                   GetLayoutMode() const { return mnTextLayoutMode; }

    void                        SetDigitLanguage( LanguageType );
    LanguageType                GetDigitLanguage() const { return meTextLanguage; }

    void                        SetRasterOp( RasterOp eRasterOp );
    RasterOp                    GetRasterOp() const { return meRasterOp; }

    /**
    If this OutputDevice is used for displaying a Print Preview
    the OutDevViewType should be set to 'OUTDEV_VIEWTYPE_PRINTPREVIEW'.

    A View can then make painting decisions dependent on this OutDevViewType.
    E.g. text colors need to be handled differently, dependent on whether it's a PrintPreview or not. (see #106611# for more)
    */
    void                        SetOutDevViewType( OutDevViewType eOutDevViewType ) { meOutDevViewType=eOutDevViewType; }
    OutDevViewType              GetOutDevViewType() const { return meOutDevViewType; }

    void                        SetLineColor();
    void                        SetLineColor( const Color& rColor );
    const Color&                GetLineColor() const { return maLineColor; }
    bool                        IsLineColor() const { return mbLineColor; }

    void                        SetFillColor();
    void                        SetFillColor( const Color& rColor );
    const Color&                GetFillColor() const { return maFillColor; }
    bool                        IsFillColor() const { return mbFillColor; }

    void                        SetBackground();
    void                        SetBackground( const Wallpaper& rBackground );

    const Wallpaper&            GetBackground() const { return maBackground; }
    bool                        IsBackground() const { return mbBackground; }

    void                        SetFont( const Font& rNewFont );
    const Font&                 GetFont() const { return maFont; }

    SystemFontData              GetSysFontData( int nFallbacklevel ) const;
    SystemTextLayoutData        GetSysTextLayoutData( const Point& rStartPt, const OUString& rStr,
                                                      sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                                      const sal_Int32* pDXAry = NULL ) const;

    void                        SetTextColor( const Color& rColor );
    const Color&                GetTextColor() const { return maTextColor; }
    void                        SetTextFillColor();
    void                        SetTextFillColor( const Color& rColor );

    Color                       GetTextFillColor() const;
    bool                        IsTextFillColor() const { return !maFont.IsTransparent(); }
    void                        SetTextLineColor();
    void                        SetTextLineColor( const Color& rColor );
    const Color&                GetTextLineColor() const { return maTextLineColor; }
    bool                        IsTextLineColor() const { return (maTextLineColor.GetTransparency() == 0); }
    void                        SetOverlineColor();
    void                        SetOverlineColor( const Color& rColor );
    const Color&                GetOverlineColor() const { return maOverlineColor; }
    bool                        IsOverlineColor() const { return (maOverlineColor.GetTransparency() == 0); }
    void                        SetTextAlign( TextAlign eAlign );
    TextAlign                   GetTextAlign() const { return maFont.GetAlign(); }

    virtual void                SetSettings( const AllSettings& rSettings );
    const AllSettings&          GetSettings() const { return *mxSettings; }

    SystemGraphicsData          GetSystemGfxData() const;
    css::uno::Any               GetSystemGfxDataAny() const;

    virtual void                SetMapMode();
    virtual void                SetMapMode( const MapMode& rNewMapMode );
    virtual void                SetRelativeMapMode( const MapMode& rNewMapMode );
    const MapMode&              GetMapMode() const { return maMapMode; }
    bool                        IsMapMode() const { return mbMap; }

    void                        SetRefPoint();
    void                        SetRefPoint( const Point& rRefPoint );
    const Point&                GetRefPoint() const { return maRefPoint; }
    bool                        IsRefPoint() const { return mbRefPoint; }

     // #i75163#
    basegfx::B2DHomMatrix       GetViewTransformation() const;
    basegfx::B2DHomMatrix       GetInverseViewTransformation() const;

    basegfx::B2DHomMatrix       GetViewTransformation( const MapMode& rMapMode ) const;
    basegfx::B2DHomMatrix       GetInverseViewTransformation( const MapMode& rMapMode ) const;


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
        default (that's MAP_PIXEL), then any pixel offset set is
        ignored also. This might be unintuitive for cases, but would
        have been far more fragile to implement. What's more, the
        reason why the pixel offset was introduced (avoiding rounding
        errors) does not apply for MAP_PIXEL, because one can always
        use the MapMode origin then.

        @param rOffset
        The offset in pixel
     */
    void                        SetPixelOffset( const Size& rOffset );

    /** Get the offset in pixel

        @see OutputDevice::SetPixelOffset for details

        @return the current offset in pixel
     */
    Size                        GetPixelOffset() const;

    Point                       LogicToPixel( const Point& rLogicPt ) const;
    Size                        LogicToPixel( const Size& rLogicSize ) const;
    Rectangle                   LogicToPixel( const Rectangle& rLogicRect ) const;
    Polygon                     LogicToPixel( const Polygon& rLogicPoly ) const;
    PolyPolygon                 LogicToPixel( const PolyPolygon& rLogicPolyPoly ) const;
    basegfx::B2DPolyPolygon     LogicToPixel( const basegfx::B2DPolyPolygon& rLogicPolyPoly ) const;
    Region                      LogicToPixel( const Region& rLogicRegion )const;
    Point                       LogicToPixel( const Point& rLogicPt,
                                              const MapMode& rMapMode ) const;
    Size                        LogicToPixel( const Size& rLogicSize,
                                              const MapMode& rMapMode ) const;
    Rectangle                   LogicToPixel( const Rectangle& rLogicRect,
                                              const MapMode& rMapMode ) const;
    Polygon                     LogicToPixel( const Polygon& rLogicPoly,
                                              const MapMode& rMapMode ) const;
    basegfx::B2DPolygon         LogicToPixel( const basegfx::B2DPolygon& rLogicPoly,
                                              const MapMode& rMapMode ) const;
    PolyPolygon                 LogicToPixel( const PolyPolygon& rLogicPolyPoly,
                                              const MapMode& rMapMode ) const;
    basegfx::B2DPolyPolygon     LogicToPixel( const basegfx::B2DPolyPolygon& rLogicPolyPoly,
                                              const MapMode& rMapMode ) const;
    Region                      LogicToPixel( const Region& rLogicRegion,
                                              const MapMode& rMapMode ) const;
    Point                       PixelToLogic( const Point& rDevicePt ) const;
    Size                        PixelToLogic( const Size& rDeviceSize ) const;
    Rectangle                   PixelToLogic( const Rectangle& rDeviceRect ) const;
    Polygon                     PixelToLogic( const Polygon& rDevicePoly ) const;
    basegfx::B2DPolygon         LogicToPixel( const basegfx::B2DPolygon& rLogicPoly ) const;
    PolyPolygon                 PixelToLogic( const PolyPolygon& rDevicePolyPoly ) const;
    basegfx::B2DPolyPolygon     PixelToLogic( const basegfx::B2DPolyPolygon& rDevicePolyPoly ) const;
    Region                      PixelToLogic( const Region& rDeviceRegion ) const;
    Point                       PixelToLogic( const Point& rDevicePt,
                                              const MapMode& rMapMode ) const;
    Size                        PixelToLogic( const Size& rDeviceSize,
                                              const MapMode& rMapMode ) const;
    Rectangle                   PixelToLogic( const Rectangle& rDeviceRect,
                                              const MapMode& rMapMode ) const;
    Polygon                     PixelToLogic( const Polygon& rDevicePoly,
                                              const MapMode& rMapMode ) const;
    basegfx::B2DPolygon         PixelToLogic( const basegfx::B2DPolygon& rDevicePoly,
                                              const MapMode& rMapMode ) const;
    PolyPolygon                 PixelToLogic( const PolyPolygon& rDevicePolyPoly,
                                              const MapMode& rMapMode ) const;
    basegfx::B2DPolyPolygon     PixelToLogic( const basegfx::B2DPolyPolygon& rDevicePolyPoly,
                                              const MapMode& rMapMode ) const;
    Region                      PixelToLogic( const Region& rDeviceRegion,
                                              const MapMode& rMapMode ) const;
    Point                       LogicToLogic( const Point&      rPtSource,
                                              const MapMode*    pMapModeSource,
                                              const MapMode*    pMapModeDest ) const;
    Size                        LogicToLogic( const Size&       rSzSource,
                                              const MapMode*    pMapModeSource,
                                              const MapMode*    pMapModeDest ) const;
    Rectangle                   LogicToLogic( const Rectangle&  rRectSource,
                                              const MapMode*    pMapModeSource,
                                              const MapMode*    pMapModeDest ) const;
    static Point                LogicToLogic( const Point&      rPtSource,
                                              const MapMode&    rMapModeSource,
                                              const MapMode&    rMapModeDest );
    static Size                 LogicToLogic( const Size&       rSzSource,
                                              const MapMode&    rMapModeSource,
                                              const MapMode&    rMapModeDest );
    static Rectangle            LogicToLogic( const Rectangle&  rRectSource,
                                              const MapMode&    rMapModeSource,
                                              const MapMode&    rMapModeDest );
    static long                 LogicToLogic( long              nLongSource,
                                              MapUnit           eUnitSource,
                                              MapUnit           eUnitDest );

    static basegfx::B2DPolygon  LogicToLogic( const basegfx::B2DPolygon& rPoly,
                                              const MapMode&    rMapModeSource,
                                              const MapMode&    rMapModeDest );
    static basegfx::B2DPolyPolygon LogicToLogic( const basegfx::B2DPolyPolygon& rPolyPoly,
                                                 const MapMode&    rMapModeSource,
                                                 const MapMode&    rMapModeDest );

    // create a mapping transformation from rMapModeSource to rMapModeDest (the above methods
    // for B2DPoly/Polygons use this internally anyway to transform the B2DPolygon)
    static basegfx::B2DHomMatrix LogicToLogic(const MapMode& rMapModeSource, const MapMode& rMapModeDest);

    Size                        GetOutputSizePixel() const
                                    { return Size( mnOutWidth, mnOutHeight ); }
    long                        GetOutputWidthPixel() const { return mnOutWidth; }
    long                        GetOutputHeightPixel() const { return mnOutHeight; }
    long                        GetOutOffXPixel() const { return mnOutOffX; }
    long                        GetOutOffYPixel() const { return mnOutOffY; }

    Size                        GetOutputSize() const
                                    { return PixelToLogic( GetOutputSizePixel() ); }

    void                        Erase();
    void                        Erase( const Rectangle& rRect ) { DrawWallpaper( rRect, GetBackground() ); }

    bool                        AddTempDevFont( const OUString& rFileURL, const OUString& rFontName );
    int                         GetDevFontCount() const;
    FontInfo                    GetDevFont( int nDevFontIndex ) const;
    int                         GetDevFontSizeCount( const Font& ) const;
    Size                        GetDevFontSize( const Font& rFont, int nSizeIndex ) const;
    bool                        IsFontAvailable( const OUString& rFontName ) const;

    FontMetric                  GetFontMetric() const;
    FontMetric                  GetFontMetric( const Font& rFont ) const;
    bool                        GetFontCharMap( FontCharMap& rFontCharMap ) const;
    bool                        GetFontCapabilities( vcl::FontCapabilities& rFontCapabilities ) const;

    sal_Int32                   HasGlyphs( const Font& rFont, const OUString& rStr,
                                           sal_Int32 nIndex = 0, sal_Int32 nLen = -1 ) const;

    long                        GetMinKashida() const;

    // i60594
    // validate kashida positions against the current font
    // returns count of invalid kashida positions
    sal_Int32                   ValidateKashidas( const OUString& rTxt, sal_Int32 nIdx, sal_Int32 nLen,
                                                  sal_Int32 nKashCount, // number of suggested kashida positions (in)
                                                  const sal_Int32* pKashidaPos, // suggested kashida positions (in)
                                                  sal_Int32* pKashidaPosDropped // invalid kashida positions (out)
                                                ) const;

    virtual sal_uInt16          GetBitCount() const;

    bool                        GetTextIsRTL( const OUString&, sal_Int32 nIndex, sal_Int32 nLen ) const;

    /** Query the existence and depth of the alpha channel

        @return 0, if no alpha channel available, and the bit depth of
        the alpha channel otherwise.
     */
    virtual sal_uInt16          GetAlphaBitCount() const;
    sal_uLong                   GetColorCount() const;

    void                        Push( sal_uInt16 nFlags = PUSH_ALL );
    void                        Pop();
    // returns the current stack depth; that is the number of Push() calls minus the number of Pop() calls
    // this should not normally be used since Push and Pop must always be used symmetrically
    // however this may be e.g. a help when debugging code in which this somehow is not the case
    sal_uInt32                  GetGCStackDepth() const;

    /** Query availability of alpha channel

        @return sal_True, if this device has an alpha channel.
     */
    bool                        HasAlpha();

    /// request XCanvas render interface for this OutputDevice
    css::uno::Reference< css::rendering::XCanvas >
                                GetCanvas() const;

    css::uno::Reference< css::awt::XGraphics >
                                CreateUnoGraphics();
    VCLXGraphicsList_impl*      GetUnoGraphicsList() const  { return mpUnoGraphicsList; }
    VCLXGraphicsList_impl*      CreateUnoGraphicsList()
                                    {
                                        mpUnoGraphicsList = new VCLXGraphicsList_impl();
                                        return mpUnoGraphicsList;
                                    }

    static void                 BeginFontSubstitution();
    static void                 EndFontSubstitution();
    static void                 AddFontSubstitute( const OUString& rFontName,
                                                   const OUString& rReplaceFontName,
                                                   sal_uInt16 nFlags = 0 );
    static void                 RemoveFontSubstitute( sal_uInt16 n );
    static sal_uInt16           GetFontSubstituteCount();

    static Font                 GetDefaultFont( sal_uInt16 nType,
                                                LanguageType eLang,
                                                sal_uLong nFlags,
                                                const OutputDevice* pOutDev = NULL );

    /** helper method removing transparencies from a metafile (e.g. for printing)

        @returns
        true: transparencies were removed
        false: output metafile is unchanged input metafile

        @attention this is a member method, so current state can influence the result !
        @attention the output metafile is prepared in pixel mode for the currentOutputDevice
                   state. It can not be moved or rotated reliably anymore.
    */
    bool                        RemoveTransparenciesFromMetaFile( const GDIMetaFile& rInMtf, GDIMetaFile& rOutMtf,
                                                                  long nMaxBmpDPIX, long nMaxBmpDPIY,
                                                                  bool bReduceTransparency,
                                                                  bool bTransparencyAutoMode,
                                                                  bool bDownsampleBitmaps,
                                                                  const Color& rBackground = Color( COL_TRANSPARENT )
                                                                );
    /** Retrieve downsampled and cropped bitmap

        @attention This method ignores negative rDstSz values, thus
        mirroring must happen outside this method (e.g. in DrawBitmap)
     */
    Bitmap                      GetDownsampledBitmap( const Size& rDstSz,
                                                      const Point& rSrcPt, const Size& rSrcSz,
                                                      const Bitmap& rBmp, long nMaxBmpDPIX, long nMaxBmpDPIY );

    //  Native Widget Rendering functions

    // These all just call through to the private mpGraphics functions of the same name.

    // Query the platform layer for control support
    bool                        IsNativeControlSupported( ControlType nType, ControlPart nPart ) const;

    // Query the native control to determine if it was acted upon
    bool                        HitTestNativeControl(   ControlType nType,
                                                        ControlPart nPart,
                                                        const Rectangle& rControlRegion,
                                                        const Point& aPos,
                                                        bool& rIsInside ) const;

    // Request rendering of a particular control and/or part
    bool                        DrawNativeControl(  ControlType nType,
                                                    ControlPart nPart,
                                                    const Rectangle& rControlRegion,
                                                    ControlState nState,
                                                    const ImplControlValue& aValue,
                                                    const OUString& aCaption );

    // Query the native control's actual drawing region (including adornment)
    bool                        GetNativeControlRegion( ControlType nType,
                                                        ControlPart nPart,
                                                        const Rectangle& rControlRegion,
                                                        ControlState nState,
                                                        const ImplControlValue& aValue,
                                                        const OUString& aCaption,
                                                        Rectangle &rNativeBoundingRegion,
                                                        Rectangle &rNativeContentRegion ) const;

};

#endif // INCLUDED_VCL_OUTDEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
