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
#include <tools/rc.hxx>
#include <tools/color.hxx>
#include <tools/poly.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <vcl/cairo.hxx>
#include <vcl/devicecoordinate.hxx>
#include <vcl/dllapi.h>
#include <vcl/font.hxx>
#include <vcl/region.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/wall.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metaactiontypes.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/outdevstate.hxx>
#include <vcl/outdevmap.hxx>

#include <basegfx/vector/b2enums.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <unotools/fontdefs.hxx>

#ifdef check
#  //some problem with MacOSX and a check define
#  undef check
#endif
#include <boost/intrusive_ptr.hpp>

#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/uno/Reference.h>

#include <memory>
#include <vector>

struct ImplOutDevData;
class ImplFontEntry;
class OutDevState;
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
struct SalTwoRect;
class VirtualDevice;
class Printer;
class FontSelectPattern;
class ImplFontMetricData;
class VCLXGraphics;
class OutDevStateStack;
struct BitmapSystemData;

namespace vcl
{
    class PDFWriterImpl;
    class ExtOutDevData;
    class ITextLayout;
    struct FontCapabilities;
    class TextLayoutCache;
    class Window;
    class FontInfo;
}

namespace com { namespace sun { namespace star { namespace rendering {
    class XCanvas;
}}}}

namespace basegfx {
    class B2DHomMatrix;
    class B2DPolygon;
    class B2DPolyPolygon;
    class B2IVector;
    typedef B2IVector B2ISize;
}

namespace com { namespace sun { namespace star { namespace awt {
    class XGraphics;
} } } }

#if defined UNX
#define GLYPH_FONT_HEIGHT   128
#else
#define GLYPH_FONT_HEIGHT   256
#endif

// Text Layout options
enum class SalLayoutFlags
{
    NONE                    = 0x0000,
    BiDiRtl                 = 0x0001,
    BiDiStrong              = 0x0002,
    RightAlign              = 0x0004,
    KerningPairs            = 0x0010,
    KerningAsian            = 0x0020,
    Vertical                = 0x0040,
    ComplexDisabled         = 0x0100,
    EnableLigatures         = 0x0200,
    SubstituteDigits        = 0x0400,
    KashidaJustification    = 0x0800,
    DisableGlyphProcessing  = 0x1000,
    ForFallback             = 0x2000,
    DrawBullet              = 0x4000,
};
namespace o3tl
{
    template<> struct typed_flags<SalLayoutFlags> : is_typed_flags<SalLayoutFlags, 0x7f77> {};
}

typedef std::vector< Rectangle > MetricVector;

// OutputDevice-Types

// Flags for DrawText()
enum class DrawTextFlags
{
    NONE                  = 0x0000,
    Disable               = 0x0001,
    Mnemonic              = 0x0002,
    Mono                  = 0x0004,
    Clip                  = 0x0008,
    Left                  = 0x0010,
    Center                = 0x0020,
    Right                 = 0x0040,
    Top                   = 0x0080,
    VCenter               = 0x0100,
    Bottom                = 0x0200,
    EndEllipsis           = 0x0400,
    PathEllipsis          = 0x0800,
    MultiLine             = 0x1000,
    WordBreak             = 0x2000,
    NewsEllipsis          = 0x4000,
    // in the long run we should make text style flags longer
    // but at the moment we can get away with this 2 bit field for ellipsis style
    CenterEllipsis        = EndEllipsis | PathEllipsis,
    WordBreakHyphenation  = 0x8000 | WordBreak,
};
namespace o3tl
{
    template<> struct typed_flags<DrawTextFlags> : is_typed_flags<DrawTextFlags, 0xffff> {};
}

// Flags for DrawImage(), these must match the definitions in css::awt::ImageDrawMode
enum class DrawImageFlags
{
    NONE                 = 0x0000,
    Disable              = 0x0001,
    Highlight            = 0x0002,
    Deactive             = 0x0004,
    ColorTransform       = 0x0008,
    SemiTransparent      = 0x0010,
};
namespace o3tl
{
    template<> struct typed_flags<DrawImageFlags> : is_typed_flags<DrawImageFlags, 0x001f> {};
}

// Flags for DrawGrid()
enum class DrawGridFlags
{
    NONE                 = 0x0000,
    Dots                 = 0x0001,
    HorzLines            = 0x0002,
    VertLines            = 0x0004,
    Lines                = HorzLines | VertLines,
};
namespace o3tl
{
    template<> struct typed_flags<DrawGridFlags> : is_typed_flags<DrawGridFlags, 0x0007> {};
}

// DrawModes
enum class DrawModeFlags : sal_uLong
{
    Default                = 0x00000000,
    BlackLine              = 0x00000001,
    BlackFill              = 0x00000002,
    BlackText              = 0x00000004,
    BlackBitmap            = 0x00000008,
    BlackGradient          = 0x00000010,
    GrayLine               = 0x00000020,
    GrayFill               = 0x00000040,
    GrayText               = 0x00000080,
    GrayBitmap             = 0x00000100,
    GrayGradient           = 0x00000200,
    NoFill                 = 0x00000400,
    NoBitmap               = 0x00000800,
    NoGradient             = 0x00001000,
    GhostedLine            = 0x00002000,
    GhostedFill            = 0x00004000,
    GhostedText            = 0x00008000,
    GhostedBitmap          = 0x00010000,
    GhostedGradient        = 0x00020000,
    WhiteLine              = 0x00100000,
    WhiteFill              = 0x00200000,
    WhiteText              = 0x00400000,
    WhiteBitmap            = 0x00800000,
    WhiteGradient          = 0x01000000,
    SettingsLine           = 0x02000000,
    SettingsFill           = 0x04000000,
    SettingsText           = 0x08000000,
    SettingsGradient       = 0x10000000,
    NoTransparency         = 0x80000000,
};
namespace o3tl
{
    template<> struct typed_flags<DrawModeFlags> : is_typed_flags<DrawModeFlags, 0x9ff3ffff> {};
}

// Antialiasing
enum class AntialiasingFlags
{
    NONE                = 0x0000,
    DisableText         = 0x0001,
    EnableB2dDraw       = 0x0002,
    PixelSnapHairline  = 0x0004,
};
namespace o3tl
{
    template<> struct typed_flags<AntialiasingFlags> : is_typed_flags<AntialiasingFlags, 0x07> {};
}

// AddFontSubstitute() flags
enum class AddFontSubstituteFlags
{
    NONE            = 0x00,
    ALWAYS          = 0x01,
    ScreenOnly      = 0x02,
};
namespace o3tl
{
    template<> struct typed_flags<AddFontSubstituteFlags> : is_typed_flags<AddFontSubstituteFlags, 0x03> {};
}

// GetDefaultFont() flags
enum GetDefaultFontFlags
{
    NONE          = 0x0000,
    OnlyOne       = 0x0001,
};
namespace o3tl
{
    template<> struct typed_flags<GetDefaultFontFlags> : is_typed_flags<GetDefaultFontFlags, 0x01> {};
}

// Flags for Invert()
#define INVERT_HIGHLIGHT                ((sal_uInt16)0x0001)
#define INVERT_50                       ((sal_uInt16)0x0002)

enum OutDevType { OUTDEV_DONTKNOW, OUTDEV_WINDOW, OUTDEV_PRINTER, OUTDEV_VIRDEV };

enum OutDevViewType { OUTDEV_VIEWTYPE_DONTKNOW, OUTDEV_VIEWTYPE_PRINTPREVIEW, OUTDEV_VIEWTYPE_SLIDESHOW };

// OutputDevice

typedef boost::intrusive_ptr< FontCharMap > FontCharMapPtr;

BmpMirrorFlags AdjustTwoRect( SalTwoRect& rTwoRect, const Size& rSizePix );
void AdjustTwoRect( SalTwoRect& rTwoRect, const Rectangle& rValidSrcRect );

extern const sal_uLong nVCLRLut[ 6 ];
extern const sal_uLong nVCLGLut[ 6 ];
extern const sal_uLong nVCLBLut[ 6 ];
extern const sal_uLong nVCLDitherLut[ 256 ];
extern const sal_uLong nVCLLut[ 256 ];

class OutputDevice;

namespace vcl {
    typedef OutputDevice RenderContext;
}

class VCL_DLLPUBLIC OutputDevice
{
    friend class Printer;
    friend class VirtualDevice;
    friend class vcl::Window;
    friend class WorkWindow;
    friend class vcl::PDFWriterImpl;
    friend void ImplHandleResize( vcl::Window* pWindow, long nNewWidth, long nNewHeight );

    // All of this will need to be replicated in Window
    // or a shared base-class as/when we can break the
    // OutputDevice -> Window inheritance.
private:
    mutable int mnRefCnt;         // reference count

    template<typename T> friend class ::rtl::Reference;
    template<typename T> friend class ::VclPtr;

    inline void acquire() const
    {
        assert(mnRefCnt>0);
        mnRefCnt++;
    }

    inline void release() const
    {
        assert(mnRefCnt>0);
        if (!--mnRefCnt)
            delete this;
    }

private:
    OutputDevice(const OutputDevice&) SAL_DELETED_FUNCTION;
    OutputDevice& operator=(const OutputDevice&) SAL_DELETED_FUNCTION;

    mutable SalGraphics*            mpGraphics;         ///< Graphics context to draw on
    mutable VclPtr<OutputDevice>    mpPrevGraphics;     ///< Previous output device in list
    mutable VclPtr<OutputDevice>    mpNextGraphics;     ///< Next output device in list
    GDIMetaFile*                    mpMetaFile;
    mutable ImplFontEntry*          mpFontEntry;
    mutable ImplFontCache*          mpFontCache;
    mutable PhysicalFontCollection* mpFontCollection;
    mutable ImplGetDevFontList*     mpGetDevFontList;
    mutable ImplGetDevSizeList*     mpGetDevSizeList;
    OutDevStateStack*               mpOutDevStateStack;
    ImplOutDevData*                 mpOutDevData;
    std::vector< VCLXGraphics* >*   mpUnoGraphicsList;
    vcl::PDFWriterImpl*             mpPDFWriter;
    vcl::ExtOutDevData*             mpExtOutDevData;

    // TEMP TEMP TEMP
    VclPtr<VirtualDevice>           mpAlphaVDev;

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
    DrawModeFlags                   mnDrawMode;
    ComplexTextLayoutMode           mnTextLayoutMode;
    ImplMapRes                      maMapRes;
    ImplThresholdRes                maThresRes;
    OutDevType                      meOutDevType;
    OutDevViewType                  meOutDevViewType;
    vcl::Region                     maRegion;           // contains the clip region, see SetClipRegion(...)
    Color                           maLineColor;
    Color                           maFillColor;
    vcl::Font                       maFont;
    Color                           maTextColor;
    Color                           maTextLineColor;
    Color                           maOverlineColor;
    TextAlign                       meTextAlign;
    RasterOp                        meRasterOp;
    Wallpaper                       maBackground;
    std::unique_ptr<AllSettings>    mxSettings;
    MapMode                         maMapMode;
    Point                           maRefPoint;
    AntialiasingFlags               mnAntialiasing;
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
    mutable bool                    mbTextBackground : 1;
    mutable bool                    mbTextSpecial : 1;
    mutable bool                    mbRefPoint : 1;
    mutable bool                    mbEnableRTL : 1;
    mutable bool                    mbDisposed : 1;

    /** @name Initialization and accessor functions
     */
    ///@{

protected:
                                OutputDevice();
public:
    virtual                     ~OutputDevice();

protected:
    /// release all references to other objects.
    virtual void                dispose();

public:
    /// call the dispose() method if we have not already been disposed.
    void                        disposeOnce();
    bool                        isDisposed() const { return mbDisposed; }

public:

    /** Get the graphic context that the output device uses to draw on.

     If no graphics device exists, then initialize it.

     @returns SalGraphics instance.
     */
    SalGraphics const           *GetGraphics() const;
    SalGraphics*                GetGraphics();

    void                        SetConnectMetaFile( GDIMetaFile* pMtf );
    GDIMetaFile*                GetConnectMetaFile() const { return mpMetaFile; }

    virtual void                SetSettings( const AllSettings& rSettings );
    const AllSettings&          GetSettings() const { return *mxSettings; }

    SystemGraphicsData          GetSystemGfxData() const;
    bool                        SupportsCairo() const;
    /// Create Surface from given cairo surface
    cairo::SurfaceSharedPtr     CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const;
    /// Create surface with given dimensions
    cairo::SurfaceSharedPtr     CreateSurface(int x, int y, int width, int height) const;
    /// Create Surface for given bitmap data
    cairo::SurfaceSharedPtr     CreateBitmapSurface(const BitmapSystemData& rData, const Size& rSize) const;
    /// Return native handle for underlying surface
    css::uno::Any               GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const ::basegfx::B2ISize& rSize) const;
    css::uno::Any               GetSystemGfxDataAny() const;

    void                        SetRefPoint();
    void                        SetRefPoint( const Point& rRefPoint );
    const Point&                GetRefPoint() const { return maRefPoint; }
    bool                        IsRefPoint() const { return mbRefPoint; }

    virtual sal_uInt16          GetBitCount() const;

    Size                        GetOutputSizePixel() const
                                    { return Size( mnOutWidth, mnOutHeight ); }
    long                        GetOutputWidthPixel() const { return mnOutWidth; }
    long                        GetOutputHeightPixel() const { return mnOutHeight; }
    long                        GetOutOffXPixel() const { return mnOutOffX; }
    long                        GetOutOffYPixel() const { return mnOutOffY; }
    void                        SetOutOffXPixel(long nOutOffX);
    void                        SetOutOffYPixel(long nOutOffY);

    Size                        GetOutputSize() const
                                    { return PixelToLogic( GetOutputSizePixel() ); }

    sal_uLong                   GetColorCount() const;


    /// request XCanvas render interface for this OutputDevice
    css::uno::Reference< css::rendering::XCanvas >
                                GetCanvas() const;

    css::uno::Reference< css::awt::XGraphics >
                                CreateUnoGraphics();
    std::vector< VCLXGraphics* > *GetUnoGraphicsList() const  { return mpUnoGraphicsList; }
    std::vector< VCLXGraphics* > *CreateUnoGraphicsList();

protected:

    /** Acquire a graphics device that the output device uses to draw on.

     There is an LRU of OutputDevices that is used to get the graphics. The
     actual creation of a SalGraphics instance is done via the SalFrame
     implementation.

     However, the SalFrame instance will only return a valid SalGraphics
     instance if it is not in use or there wasn't one in the first place. When
     this happens, AcquireGraphics finds the least recently used OutputDevice
     in a different frame and "steals" it (releases it then starts using it).

     If there are no frames to steal an OutputDevice's SalGraphics instance from
     then it blocks until the graphics is released.

     Once it has acquired a graphics instance, then we add the OutputDevice to
     the LRU.

     @returns true if was able to initialize the graphics device, false otherwise.
     */
    virtual bool                AcquireGraphics() const = 0;

    /** Release the graphics device, and remove it from the graphics device
     list.

     @param         bRelease    Determines whether to release the fonts of the
                                physically released graphics device.
     */
    virtual void                ReleaseGraphics( bool bRelease = true ) = 0;
    ///@}


    /** @name Helper functions
     */
    ///@{

public:

    /** Get the output device's DPI x-axis value.

     @returns x-axis DPI value
     */
    SAL_DLLPRIVATE sal_Int32    GetDPIX() const { return mnDPIX; }

    /** Get the output device's DPI y-axis value.

     @returns y-axis DPI value
     */
    SAL_DLLPRIVATE sal_Int32    GetDPIY() const { return mnDPIY; }

    SAL_DLLPRIVATE void         SetDPIX( sal_Int32 nDPIX ) { mnDPIX = nDPIX; }
    SAL_DLLPRIVATE void         SetDPIY( sal_Int32 nDPIY ) { mnDPIY = nDPIY; }

    sal_Int32                   GetDPIScaleFactor() const { return mnDPIScaleFactor; }

    OutDevType                  GetOutDevType() const { return meOutDevType; }

    /** Query an OutputDevice to see whether it supports a specific operation

     @returns true if operation supported, else false
    */
    bool                        SupportsOperation( OutDevSupportType ) const;

    vcl::PDFWriterImpl*         GetPDFWriter() const { return mpPDFWriter; }

    void                        SetExtOutDevData( vcl::ExtOutDevData* pExtOutDevData ) { mpExtOutDevData = pExtOutDevData; }
    vcl::ExtOutDevData*         GetExtOutDevData() const { return mpExtOutDevData; }

    ///@}


    /** @Name Direct OutputDevice drawing functions
     */
    ///@{

public:

    virtual void                DrawOutDev(
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPt,  const Size& rSrcSize );

    virtual void                DrawOutDev(
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPt,  const Size& rSrcSize,
                                    const OutputDevice& rOutDev );

    virtual void                CopyArea(
                                    const Point& rDestPt,
                                    const Point& rSrcPt,  const Size& rSrcSize,
                                    bool bWindowInvalidate = false );

    /**
     * Instantiate across a paint operation to defer flushing
     * to the end.
     *
     * NB. holding a handle avoids problems with
     * the underlying SalGraphics and it's implementation
     * changing.
     */
    class PaintScope {
        void *pHandle;
    public:
        PaintScope(OutputDevice *);
        ~PaintScope();
        void flush();
    };

protected:

    virtual void                CopyDeviceArea( SalTwoRect& aPosAry, bool bWindowInvalidate = false);

    SAL_DLLPRIVATE void         drawOutDevDirect ( const OutputDevice* pSrcDev, SalTwoRect& rPosAry );

    SAL_DLLPRIVATE bool         is_double_buffered_window() const;

private:

    // not implemented; to detect misuses of DrawOutDev(...OutputDevice&);
    SAL_DLLPRIVATE void         DrawOutDev( const Point&, const Size&, const Point&,  const Size&, const Printer&) SAL_DELETED_FUNCTION;
    ///@}


    /** @name OutputDevice state functions
     */
    ///@{

public:

    void                        Push( PushFlags nFlags = PushFlags::ALL );
    void                        Pop();

    // returns the current stack depth; that is the number of Push() calls minus the number of Pop() calls
    // this should not normally be used since Push and Pop must always be used symmetrically
    // however this may be e.g. a help when debugging code in which this somehow is not the case
    sal_uInt32                  GetGCStackDepth() const;

    void                        EnableOutput( bool bEnable = true );
    bool                        IsOutputEnabled() const { return mbOutput; }
    bool                        IsDeviceOutput() const { return mbDevOutput; }
    bool                        IsDeviceOutputNecessary() const { return (mbOutput && mbDevOutput); }
    bool                        IsOutputNecessary() const { return ((mbOutput && mbDevOutput) || (mpMetaFile != NULL)); }

    void                        SetAntialiasing( AntialiasingFlags nMode = AntialiasingFlags::NONE );
    AntialiasingFlags           GetAntialiasing() const { return mnAntialiasing; }

    void                        SetDrawMode( DrawModeFlags nDrawMode );
    DrawModeFlags               GetDrawMode() const { return mnDrawMode; }

    void                        SetLayoutMode( ComplexTextLayoutMode nTextLayoutMode );
    ComplexTextLayoutMode       GetLayoutMode() const { return mnTextLayoutMode; }

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

    void                        SetFont( const vcl::Font& rNewFont );
    const vcl::Font&            GetFont() const { return maFont; }

protected:

    virtual void                ImplReleaseFonts();

private:

    SAL_DLLPRIVATE void         InitLineColor();

    SAL_DLLPRIVATE void         InitFillColor();

    ///@}


    /** @name Clipping functions
     */
    ///@{

public:

    vcl::Region                 GetClipRegion() const;
    void                        SetClipRegion();
    void                        SetClipRegion( const vcl::Region& rRegion );
    bool                        SelectClipRegion( const vcl::Region&, SalGraphics* pGraphics = NULL );

    bool                        IsClipRegion() const { return mbClipRegion; }

    void                        MoveClipRegion( long nHorzMove, long nVertMove );
    void                        IntersectClipRegion( const Rectangle& rRect );
    void                        IntersectClipRegion( const vcl::Region& rRegion );

    virtual vcl::Region         GetActiveClipRegion() const;

protected:

    virtual void                InitClipRegion();
    virtual void                ClipToPaintRegion    ( Rectangle& rDstRect );

private:

    SAL_DLLPRIVATE void         SetDeviceClipRegion( const vcl::Region* pRegion );
    ///@}


    /** @name Pixel functions
     */
    ///@{

public:

    void                        DrawPixel( const Point& rPt );
    void                        DrawPixel( const Point& rPt, const Color& rColor );
    void                        DrawPixel( const Polygon& rPts, const Color* pColors = NULL );
    void                        DrawPixel( const Polygon& rPts, const Color& rColor );

    Color                       GetPixel( const Point& rPt ) const;
    ///@}


    /** @name Rectangle functions
     */
    ///@{

public:

    void                        DrawRect( const Rectangle& rRect );
    void                        DrawRect( const Rectangle& rRect,
                                          sal_uLong nHorzRount, sal_uLong nVertRound );

    /// Fill the given rectangle with checkered rectangles of size nLen x nLen using the colors aStart and aEnd
    void                        DrawCheckered(
                                    const Point& rPos,
                                    const Size& rSize,
                                    sal_uInt32 nLen = 8,
                                    Color aStart = Color(COL_WHITE),
                                    Color aEnd = Color(COL_BLACK));

    void                        DrawGrid( const Rectangle& rRect, const Size& rDist, DrawGridFlags nFlags );

    ///@}

    /** @name Invert functions
     */
    ///@{
public:
    void                                Invert( const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    void                                Invert( const Polygon& rPoly, sal_uInt16 nFlags = 0 );
    ///@}

    /** @name Line functions
     */
    ///@{

public:

    void                        DrawLine( const Point& rStartPt, const Point& rEndPt );

    void                        DrawLine( const Point& rStartPt, const Point& rEndPt,
                                          const LineInfo& rLineInfo );

private:

    /** Helper for line geometry paint with support for graphic expansion (pattern and fat_to_area)
     */
    SAL_DLLPRIVATE void         drawLine( basegfx::B2DPolyPolygon aLinePolyPolygon, const LineInfo& rInfo );
    ///@}


    /** @name Polyline functions
     */
    ///@{

public:

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
                                    basegfx::B2DLineJoin eLineJoin = basegfx::B2DLINEJOIN_ROUND,
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

    // #i101491#
    // Helper who tries to use SalGDI's DrawPolyLine direct and returns it's bool.
    bool                        DrawPolyLineDirect(
                                    const basegfx::B2DPolygon& rB2DPolygon,
                                    double fLineWidth = 0.0,
                                    double fTransparency = 0.0,
                                    basegfx::B2DLineJoin eLineJoin = basegfx::B2DLINEJOIN_NONE,
                                    css::drawing::LineCap eLineCap = css::drawing::LineCap_BUTT,
                                    bool bBypassAACheck = false );
private:

    // #i101491#
    // Helper which holds the old line geometry creation and is extended to use AA when
    // switched on. Advantage is that line geometry is only temporarily used for paint
    SAL_DLLPRIVATE void         drawPolyLine(const Polygon& rPoly, const LineInfo& rLineInfo);

    ///@}


    /** @name Polygon functions
     */
    ///@{

public:

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
    void                        DrawPolyPolygon( const tools::PolyPolygon& rPolyPoly );
    void                        DrawPolyPolygon( const basegfx::B2DPolyPolygon& );

private:

    SAL_DLLPRIVATE void         ImplDrawPolygon( const Polygon& rPoly, const tools::PolyPolygon* pClipPolyPoly = NULL );
    SAL_DLLPRIVATE void         ImplDrawPolyPolygon( const tools::PolyPolygon& rPolyPoly, const tools::PolyPolygon* pClipPolyPoly = NULL );
    SAL_DLLPRIVATE void         ImplDrawPolyPolygon( sal_uInt16 nPoly, const tools::PolyPolygon& rPolyPoly );
    // #i101491#
    // Helper who implements the DrawPolyPolygon functionality for basegfx::B2DPolyPolygon
    // without MetaFile processing
    SAL_DLLPRIVATE void         ImplDrawPolyPolygonWithB2DPolyPolygon(const basegfx::B2DPolyPolygon& rB2DPolyPoly);
    ///@}


    /** @name Curved shape functions
     */
    ///@{

public:

    void                        DrawEllipse( const Rectangle& rRect );

    void                        DrawArc(
                                    const Rectangle& rRect,
                                    const Point& rStartPt, const Point& rEndPt );

    void                        DrawPie(
                                    const Rectangle& rRect,
                                    const Point& rStartPt, const Point& rEndPt );

    void                        DrawChord(
                                    const Rectangle& rRect,
                                    const Point& rStartPt, const Point& rEndPt );

    ///@}


    /** @name Gradient functions
     */
    ///@{

public:
    void                        DrawGradient( const Rectangle& rRect, const Gradient& rGradient );
    void                        DrawGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient );

    void                        AddGradientActions(
                                    const Rectangle& rRect,
                                    const Gradient& rGradient,
                                    GDIMetaFile& rMtf );

protected:

    virtual bool                UsePolyPolygonForComplexGradient() = 0;

    virtual long                GetGradientStepCount( long nMinRect );

private:

    SAL_DLLPRIVATE void         DrawLinearGradient( const Rectangle& rRect, const Gradient& rGradient, const tools::PolyPolygon* pClipPolyPoly );
    SAL_DLLPRIVATE void         DrawComplexGradient( const Rectangle& rRect, const Gradient& rGradient, const tools::PolyPolygon* pClipPolyPoly );

    SAL_DLLPRIVATE void         DrawGradientToMetafile( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient );
    SAL_DLLPRIVATE void         DrawLinearGradientToMetafile( const Rectangle& rRect, const Gradient& rGradient );
    SAL_DLLPRIVATE void         DrawComplexGradientToMetafile( const Rectangle& rRect, const Gradient& rGradient );

    SAL_DLLPRIVATE long         GetGradientSteps( const Gradient& rGradient, const Rectangle& rRect, bool bMtf, bool bComplex=false );

    SAL_DLLPRIVATE Color        GetSingleColorGradientFill();
    SAL_DLLPRIVATE void         SetGrayscaleColors( Gradient &rGradient );
    ///@}


    /** @name Hatch functions
     */
    ///@{

public:

#ifdef _MSC_VER
    void                        DrawHatch( const tools::PolyPolygon& rPolyPoly, const ::Hatch& rHatch );
    void                        AddHatchActions( const tools::PolyPolygon& rPolyPoly,
                                                 const ::Hatch& rHatch,
                                                 GDIMetaFile& rMtf );
#else
    void                        DrawHatch( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch );
    void                        AddHatchActions( const tools::PolyPolygon& rPolyPoly,
                                                 const Hatch& rHatch,
                                                 GDIMetaFile& rMtf );
#endif

    void                        DrawHatch( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch, bool bMtf );

private:

    SAL_DLLPRIVATE void         CalcHatchValues( const Rectangle& rRect, long nDist, sal_uInt16 nAngle10, Point& rPt1, Point& rPt2, Size& rInc, Point& rEndPt1 );
    SAL_DLLPRIVATE void         DrawHatchLine( const Line& rLine, const tools::PolyPolygon& rPolyPoly, Point* pPtBuffer, bool bMtf );
    ///@}


    /** @name Wallpaper functions
     */
    ///@{

public:
    void                        DrawWallpaper( const Rectangle& rRect, const Wallpaper& rWallpaper );

    virtual void                Erase();
    virtual void                Erase( const Rectangle& rRect ) { DrawWallpaper( rRect, GetBackground() ); }

protected:
    virtual void                DrawGradientWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );

private:
    SAL_DLLPRIVATE void         DrawWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         DrawColorWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         DrawBitmapWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper );
    ///@}


    /** @name Text functions
     */
    ///@{

public:

    void                        DrawText( const Point& rStartPt, const OUString& rStr,
                                          sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                          MetricVector* pVector = NULL, OUString* pDisplayText = NULL );

    void                        DrawText( const Rectangle& rRect,
                                          const OUString& rStr, DrawTextFlags nStyle = DrawTextFlags::NONE,
                                          MetricVector* pVector = NULL, OUString* pDisplayText = NULL,
                                          vcl::ITextLayout* _pTextLayout = NULL );

    static void                 ImplDrawText( OutputDevice& rTargetDevice, const Rectangle& rRect,
                                              const OUString& rOrigStr, DrawTextFlags nStyle,
                                              MetricVector* pVector, OUString* pDisplayText, vcl::ITextLayout& _rLayout );

    void                        ImplDrawText( SalLayout& );

    void                        ImplDrawTextBackground( const SalLayout& );

    void                        DrawCtrlText( const Point& rPos, const OUString& rStr,
                                              sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                              DrawTextFlags nStyle = DrawTextFlags::Mnemonic, MetricVector* pVector = NULL, OUString* pDisplayText = NULL );

    void                        DrawTextLine( const Point& rPos, long nWidth,
                                              FontStrikeout eStrikeout,
                                              FontUnderline eUnderline,
                                              FontUnderline eOverline,
                                              bool bUnderlineAbove = false );

    void                        ImplDrawTextLine( long nBaseX, long nX, long nY, DeviceCoordinate nWidth,
                                                  FontStrikeout eStrikeout, FontUnderline eUnderline,
                                                  FontUnderline eOverline, bool bUnderlineAbove );

    void                        ImplDrawTextLines( SalLayout&, FontStrikeout eStrikeout, FontUnderline eUnderline,
                                                   FontUnderline eOverline, bool bWordLine, bool bUnderlineAbove );

    void                        DrawWaveLine( const Point& rStartPos, const Point& rEndPos );

    bool                        ImplDrawRotateText( SalLayout& );

    Rectangle                   GetTextRect( const Rectangle& rRect,
                                             const OUString& rStr, DrawTextFlags nStyle = DrawTextFlags::WordBreak,
                                             TextRectInfo* pInfo = NULL,
                                             const vcl::ITextLayout* _pTextLayout = NULL ) const;

    /** Return the exact bounding rectangle of rStr.

        The text is then drawn exactly from rRect.TopLeft() to
        rRect.BottomRight(), don't assume that rRect.TopLeft() is [0, 0].

        Please note that you don't always want to use GetTextBoundRect(); in
        many cases you actually want to use GetTextHeight(), because
        GetTextBoundRect() gives you the exact bounding rectangle regardless
        what is the baseline of the text.

        Code snippet to get just exactly the text (no filling around that) as
        a bitmap via a VirtualDevice (regardless what is the baseline):

        <code>
        VirtualDevice aDevice;
        vcl::Font aFont = aDevice.GetFont();
        aFont.SetSize(Size(0, 96));
        aFont.SetColor(COL_BLACK);
        aDevice.SetFont(aFont);
        aDevice.Erase();

        ::Rectangle aRect;
        aDevice.GetTextBoundRect(aRect, aText);
        aDevice.SetOutputSize(Size(aRect.BottomRight().X() + 1, aRect.BottomRight().Y() + 1));
        aDevice.SetBackground(Wallpaper(COL_TRANSPARENT));
        aDevice.DrawText(Point(0,0), aText);

        // exactly only the text, regardless of the baseline
        Bitmap aBitmap(aDevice.GetBitmap(aRect.TopLeft(), aRect.GetSize()));
        </code>

        Code snippet to get the text as a bitmap via a Virtual device that
        contains even the filling so that the baseline is always preserved
        (ie. the text will not jump up and down according to whether it
        contains 'y' or not etc.)

        <code>
        VirtualDevice aDevice;
        // + the appropriate font / device setup, see above

        aDevice.SetOutputSize(Size(aDevice.GetTextWidth(aText), aDevice.GetTextHeight()));
        aDevice.SetBackground(Wallpaper(COL_TRANSPARENT));
        aDevice.DrawText(Point(0,0), aText);

        // bitmap that contains even the space around the text,
        // that means, preserves the baseline etc.
        Bitmap aBitmap(aDevice.GetBitmap(Point(0, 0), aDevice.GetOutputSize()));
        </code>
    */
    bool                        GetTextBoundRect( Rectangle& rRect,
                                                  const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                                  sal_uLong nLayoutWidth = 0, const long* pDXArray = NULL ) const;

    Rectangle                   ImplGetTextBoundRect( const SalLayout& );

    bool                        GetTextOutline( tools::PolyPolygon&,
                                                const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0,
                                                sal_Int32 nLen = -1, bool bOptimize = true,
                                                sal_uLong nLayoutWidth = 0, const long* pDXArray = NULL ) const;

    bool                        GetTextOutlines( PolyPolyVector&,
                                                 const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0,
                                                 sal_Int32 nLen = -1, bool bOptimize = true,
                                                 sal_uLong nLayoutWidth = 0, const long* pDXArray = NULL ) const;

    bool                        GetTextOutlines( ::basegfx::B2DPolyPolygonVector &rVector,
                                                 const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0,
                                                 sal_Int32 nLen = -1, bool bOptimize = true,
                                                 sal_uLong nLayoutWidth = 0, const long* pDXArray = NULL ) const;


    OUString                    GetEllipsisString( const OUString& rStr, long nMaxWidth,
                                                   DrawTextFlags nStyle = DrawTextFlags::EndEllipsis ) const;

    long                        GetCtrlTextWidth( const OUString& rStr, sal_Int32 nIndex = 0,
                                                  sal_Int32 nLen = -1,
                                                  DrawTextFlags nStyle = DrawTextFlags::Mnemonic ) const;

    static OUString             GetNonMnemonicString( const OUString& rStr, sal_Int32& rMnemonicPos );

    static OUString             GetNonMnemonicString( const OUString& rStr )
                                            { sal_Int32 nDummy; return GetNonMnemonicString( rStr, nDummy ); }

    /** Generate MetaTextActions for the text rect

        This method splits up the text rect into multiple
        MetaTextActions, one for each line of text. This is comparable
        to AddGradientActions(), which splits up a gradient into its
        constituent polygons. Parameter semantics fully compatible to
        DrawText().
     */
    void                        AddTextRectActions( const Rectangle& rRect,
                                                    const OUString&  rOrigStr,
                                                    DrawTextFlags    nStyle,
                                                    GDIMetaFile&     rMtf );

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

    /** Width of the text.

        See also GetTextBoundRect() for more explanation + code examples.
    */
    long                        GetTextWidth( const OUString& rStr, sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                  vcl::TextLayoutCache const* = nullptr) const;

    /** Height where any character of the current font fits; in logic coordinates.

        See also GetTextBoundRect() for more explanation + code examples.
    */
    long                        GetTextHeight() const;
    float                       approximate_char_width() const;

    void                        DrawTextArray( const Point& rStartPt, const OUString& rStr,
                                               const long* pDXAry = NULL,
                                               sal_Int32 nIndex = 0,
                                               sal_Int32 nLen = -1,
                                               SalLayoutFlags flags = SalLayoutFlags::NONE);
    long                        GetTextArray( const OUString& rStr, long* pDXAry = NULL,
                                              sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                              vcl::TextLayoutCache const* = nullptr) const;

    bool                        GetCaretPositions( const OUString&, long* pCaretXArray,
                                              sal_Int32 nIndex, sal_Int32 nLen,
                                              long* pDXAry = NULL, long nWidth = 0,
                                              bool bCellBreaking = true ) const;
    void                        DrawStretchText( const Point& rStartPt, sal_uLong nWidth,
                                                 const OUString& rStr,
                                                 sal_Int32 nIndex = 0, sal_Int32 nLen = -1);
    sal_Int32                   GetTextBreak( const OUString& rStr, long nTextWidth,
                                              sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                              long nCharExtra = 0,
                                              vcl::TextLayoutCache const* = nullptr) const;
    sal_Int32                   GetTextBreak( const OUString& rStr, long nTextWidth,
                                              sal_Unicode nExtraChar, sal_Int32& rExtraCharPos,
                                              sal_Int32 nIndex, sal_Int32 nLen,
                                              long nCharExtra = 0,
                                              vcl::TextLayoutCache const* = nullptr) const;
    std::shared_ptr<vcl::TextLayoutCache> CreateTextLayoutCache(OUString const&) const;

private:
    SAL_DLLPRIVATE void         ImplInitTextColor();

    SAL_DLLPRIVATE void         ImplInitTextLineSize();
    SAL_DLLPRIVATE void         ImplInitAboveTextLineSize();


    SAL_DLLPRIVATE bool         ImplDrawTextDirect( SalLayout&, bool bTextLines, sal_uInt32 flags = 0 );
    SAL_DLLPRIVATE void         ImplDrawSpecialText( SalLayout& );
    SAL_DLLPRIVATE void         ImplDrawTextRect( long nBaseX, long nBaseY, long nX, long nY, long nWidth, long nHeight );

    SAL_DLLPRIVATE void         ImplDrawWavePixel( long nOriginX, long nOriginY, long nCurX, long nCurY, short nOrientation, SalGraphics* pGraphics, OutputDevice* pOutDev,
                                                   bool bDrawPixAsRect, long nPixWidth, long nPixHeight );
    SAL_DLLPRIVATE void         ImplDrawWaveLine( long nBaseX, long nBaseY, long nStartX, long nStartY, long nWidth, long nHeight, long nLineWidth, short nOrientation, const Color& rColor );
    SAL_DLLPRIVATE void         ImplDrawWaveTextLine( long nBaseX, long nBaseY, long nX, long nY, long nWidth, FontUnderline eTextLine, Color aColor, bool bIsAbove );
    SAL_DLLPRIVATE void         ImplDrawStraightTextLine( long nBaseX, long nBaseY, long nX, long nY, long nWidth, FontUnderline eTextLine, Color aColor, bool bIsAbove );
    SAL_DLLPRIVATE void         ImplDrawStrikeoutLine( long nBaseX, long nBaseY, long nX, long nY, long nWidth, FontStrikeout eStrikeout, Color aColor );
    SAL_DLLPRIVATE void         ImplDrawStrikeoutChar( long nBaseX, long nBaseY, long nX, long nY, long nWidth, FontStrikeout eStrikeout, Color aColor );
    SAL_DLLPRIVATE void         ImplDrawMnemonicLine( long nX, long nY, long nWidth );

    SAL_DLLPRIVATE static bool  ImplIsUnderlineAbove( const vcl::Font& );

    static
    SAL_DLLPRIVATE long         ImplGetTextLines( ImplMultiTextLineInfo& rLineInfo, long nWidth, const OUString& rStr, DrawTextFlags nStyle, const vcl::ITextLayout& _rLayout );
    ///@}


    /** @name Font functions
     */
    ///@{

public:

    vcl::FontInfo               GetDevFont( int nDevFontIndex ) const;
    int                         GetDevFontCount() const;

    bool                        IsFontAvailable( const OUString& rFontName ) const;

    Size                        GetDevFontSize( const vcl::Font& rFont, int nSizeIndex ) const;
    int                         GetDevFontSizeCount( const vcl::Font& ) const;

    bool                        AddTempDevFont( const OUString& rFileURL, const OUString& rFontName );

    FontMetric                  GetFontMetric() const;
    FontMetric                  GetFontMetric( const vcl::Font& rFont ) const;

    bool                        GetFontCharMap( FontCharMapPtr& rFontCharMap ) const;
    bool                        GetFontCapabilities( vcl::FontCapabilities& rFontCapabilities ) const;

    /** Retrieve detailed font information in platform independent structure

        @param  nFallbacklevel      Fallback font level (0 = best matching font)

        @return SystemFontData
     */
    SystemFontData              GetSysFontData( int nFallbacklevel ) const;

    SAL_DLLPRIVATE void         ImplGetEmphasisMark( tools::PolyPolygon& rPolyPoly, bool& rPolyLine, Rectangle& rRect1, Rectangle& rRect2,
                                                     long& rYOff, long& rWidth, FontEmphasisMark eEmphasis, long nHeight, short nOrient );
    SAL_DLLPRIVATE static FontEmphasisMark
                                ImplGetEmphasisMarkStyle( const vcl::Font& rFont );

    bool                        GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex,
                                                    int nLen, int nBase, MetricVector& rVector );

    sal_Int32                   HasGlyphs( const vcl::Font& rFont, const OUString& rStr,
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

    static void                 BeginFontSubstitution();
    static void                 EndFontSubstitution();
    static void                 AddFontSubstitute( const OUString& rFontName,
                                                   const OUString& rReplaceFontName,
                                                   AddFontSubstituteFlags nFlags = AddFontSubstituteFlags::NONE );
    static void                 RemoveFontSubstitute( sal_uInt16 n );
    static sal_uInt16           GetFontSubstituteCount();

    static vcl::Font            GetDefaultFont( DefaultFontType nType,
                                                LanguageType eLang,
                                                GetDefaultFontFlags nFlags,
                                                const OutputDevice* pOutDev = NULL );

    SAL_DLLPRIVATE void         ImplInitFontList() const;
    SAL_DLLPRIVATE void         ImplUpdateFontData( bool bNewFontLists );

    //drop font data for all outputdevices.
    //If bNewFontLists is true then empty lists of system fonts
    SAL_DLLPRIVATE static void  ImplClearAllFontData( bool bNewFontLists );
    //fetch font data for all outputdevices
    //If bNewFontLists is true then fetch lists of system fonts
    SAL_DLLPRIVATE static void  ImplRefreshAllFontData( bool bNewFontLists );
    //drop and fetch font data for all outputdevices
    //If bNewFontLists is true then drop and refetch lists of system fonts
    SAL_DLLPRIVATE static void  ImplUpdateAllFontData( bool bNewFontLists );

protected:

    virtual void                InitFont() const;
    virtual void                SetFontOrientation( ImplFontEntry* const pFontEntry ) const;
    virtual long                GetFontExtLeading() const;


private:

    typedef void ( OutputDevice::* FontUpdateHandler_t )( bool );

    SAL_DLLPRIVATE bool         ImplNewFont() const;

    SAL_DLLPRIVATE void         ImplClearFontData( bool bNewFontLists );
    SAL_DLLPRIVATE void         ImplRefreshFontData( bool bNewFontLists );
    SAL_DLLPRIVATE static void  ImplUpdateFontDataForAllFrames( FontUpdateHandler_t pHdl, bool bNewFontLists );

    static
    SAL_DLLPRIVATE OUString     ImplGetEllipsisString( const OutputDevice& rTargetDevice, const OUString& rStr,
                                                       long nMaxWidth, DrawTextFlags nStyle, const vcl::ITextLayout& _rLayout );

    SAL_DLLPRIVATE void         ImplDrawEmphasisMark( long nBaseX, long nX, long nY, const tools::PolyPolygon& rPolyPoly, bool bPolyLine, const Rectangle& rRect1, const Rectangle& rRect2 );
    SAL_DLLPRIVATE void         ImplDrawEmphasisMarks( SalLayout& );
    ///@}


    /** @name Layout functions
     */
    ///@{

public:

    SystemTextLayoutData        GetSysTextLayoutData( const Point& rStartPt, const OUString& rStr,
                                                      sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                                      const long* pDXAry = NULL ) const;

    SAL_DLLPRIVATE bool         ImplIsAntiparallel() const ;
    SAL_DLLPRIVATE void         ReMirror( Point &rPoint ) const;
    SAL_DLLPRIVATE void         ReMirror( Rectangle &rRect ) const;
    SAL_DLLPRIVATE void         ReMirror( vcl::Region &rRegion ) const;
    SAL_DLLPRIVATE bool         ImplIsRecordLayout() const;
    virtual bool                HasMirroredGraphics() const;
    SAL_DLLPRIVATE SalLayout*   ImplLayout( const OUString&, sal_Int32 nIndex, sal_Int32 nLen,
                                            const Point& rLogicPos = Point(0,0), long nLogicWidth=0,
                                            const long* pLogicDXArray=NULL, SalLayoutFlags flags = SalLayoutFlags::NONE,
                                            vcl::TextLayoutCache const* = nullptr) const;
    SAL_DLLPRIVATE ImplLayoutArgs ImplPrepareLayoutArgs( OUString&, const sal_Int32 nIndex, const sal_Int32 nLen,
                                                         DeviceCoordinate nPixelWidth, const DeviceCoordinate* pPixelDXArray,
                                                         SalLayoutFlags flags = SalLayoutFlags::NONE,
                                                         vcl::TextLayoutCache const* = nullptr) const;
    SAL_DLLPRIVATE SalLayout*   ImplGlyphFallbackLayout( SalLayout*, ImplLayoutArgs& ) const;
    // tells whether this output device is RTL in an LTR UI or LTR in a RTL UI
    SAL_DLLPRIVATE SalLayout*   getFallbackFont(ImplFontEntry &rFallbackFont,
                                    FontSelectPattern &rFontSelData, int nFallbackLevel,
                                    ImplLayoutArgs& rLayoutArgs) const;


    // Enabling/disabling RTL only makes sense for OutputDevices that use a mirroring SalGraphisLayout
    virtual void                EnableRTL( bool bEnable = true);
    bool                        IsRTLEnabled() const { return mbEnableRTL; }

    bool                        GetTextIsRTL( const OUString&, sal_Int32 nIndex, sal_Int32 nLen ) const;

    ///@}


    /** @name Bitmap functions
     */
    ///@{

public:

    /** @overload
        void DrawBitmap(
                const Point& rDestPt,
                const Size& rDestSize,
                const Point& rSrcPtPixel,
                const Size& rSecSizePixel,
                const Bitmap& rBitmap,
                MetaActionType nAction = MetaActionType::BMPSCALEPART)
      */
    void                        DrawBitmap(
                                    const Point& rDestPt,
                                    const Bitmap& rBitmap );

    /** @overload
        void DrawBitmap(
                const Point& rDestPt,
                const Size& rDestSize,
                const Point& rSrcPtPixel,
                const Size& rSecSizePixel,
                const Bitmap& rBitmap,
                MetaActionType nAction = MetaActionType::BMPSCALEPART)
      */
    void                        DrawBitmap(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const Bitmap& rBitmap );

    void                        DrawBitmap(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const Point& rSrcPtPixel,
                                    const Size& rSrcSizePixel,
                                    const Bitmap& rBitmap,
                                    MetaActionType nAction = MetaActionType::BMPSCALEPART );

    /** @overload
        void DrawBitmapEx(
                const Point& rDestPt,
                const Size& rDestSize,
                const Point& rSrcPtPixel,
                const Size& rSecSizePixel,
                const BitmapEx& rBitmapEx,
                MetaActionType nAction = MetaActionType::BMPEXSCALEPART)
     */
    void                        DrawBitmapEx(
                                    const Point& rDestPt,
                                    const BitmapEx& rBitmapEx );


    /** @overload
        void DrawBitmapEx(
                const Point& rDestPt,
                const Size& rDestSize,
                const Point& rSrcPtPixel,
                const Size& rSecSizePixel,
                const BitmapEx& rBitmapEx,
                MetaActionType nAction = MetaActionType::BMPEXSCALEPART)
     */
    void                        DrawBitmapEx(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const BitmapEx& rBitmapEx );

    void                        DrawBitmapEx(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const Point& rSrcPtPixel,
                                    const Size& rSrcSizePixel,
                                    const BitmapEx& rBitmapEx,
                                    MetaActionType nAction = MetaActionType::BMPEXSCALEPART );

    /** @overload
        virtual void DrawImage(
                        const Point& rPos,
                        const Size& rSize,
                        const Image& rImage,
                        sal_uInt16 nStyle = 0)
     */
    virtual void                DrawImage(
                                    const Point& rPos,
                                    const Image& rImage,
                                    DrawImageFlags nStyle = DrawImageFlags::NONE );

    virtual void                DrawImage(
                                    const Point& rPos,
                                    const Size& rSize,
                                    const Image& rImage,
                                    DrawImageFlags nStyle = DrawImageFlags::NONE );


    virtual Bitmap              GetBitmap( const Point& rSrcPt, const Size& rSize ) const;

    /** Query extended bitmap (with alpha channel, if available).
     */
    BitmapEx                    GetBitmapEx( const Point& rSrcPt, const Size& rSize ) const;


    /** Draw BitmapEx transformed

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


protected:

    virtual void                DrawDeviceBitmap(
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                    BitmapEx& rBitmapEx );

    virtual void                ScaleBitmap ( Bitmap &rBmp, SalTwoRect &rPosAry );

    /** Transform and draw a bitmap directly

     @param     aFullTransform      The B2DHomMatrix used for the transformation
     @param     rBitmapEx           Reference to the bitmap to be transformed and drawn

     @return true if it was able to draw the bitmap, false if not
     */
    virtual bool                DrawTransformBitmapExDirect(
                                    const basegfx::B2DHomMatrix& aFullTransform,
                                    const BitmapEx& rBitmapEx);

    /** Transform and reduce the area that needs to be drawn of the bitmap and return the new
        visible range and the maximum area.


      @param     aFullTransform      B2DHomMatrix used for transformation
      @param     aVisibleRange       The new visible area of the bitmap
      @param     fMaximumArea        The maximum area of the bitmap

      @returns true if there is an area to be drawn, otherwise nothing is left to be drawn
        so return false
      */
    virtual bool                TransformAndReduceBitmapExToTargetRange(
                                    const basegfx::B2DHomMatrix& aFullTransform,
                                    basegfx::B2DRange &aVisibleRange,
                                    double &fMaximumArea);

private:

    SAL_DLLPRIVATE void         DrawDeviceAlphaBitmap(
                                    const Bitmap& rBmp,
                                    const AlphaMask& rAlpha,
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const Point& rSrcPtPixel,
                                    const Size& rSrcSizePixel );

    SAL_DLLPRIVATE void DrawDeviceAlphaBitmapSlowPath(
                                const Bitmap& rBitmap, const AlphaMask& rAlpha,
                                Rectangle aDstRect, Rectangle aBmpRect,
                                Size& aOutSz, Point& aOutPt);


    SAL_DLLPRIVATE bool         BlendBitmap(
                                    const SalTwoRect&   rPosAry,
                                    const Bitmap&       rBmp );

    SAL_DLLPRIVATE Bitmap       BlendBitmap(
                                    Bitmap&             aBmp,
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

    SAL_DLLPRIVATE Bitmap       BlendBitmapWithAlpha(
                                    Bitmap&             aBmp,
                                    BitmapReadAccess*   pP,
                                    BitmapReadAccess*   pA,
                                    const Rectangle&    aDstRect,
                                    const sal_Int32     nOffY,
                                    const sal_Int32     nDstHeight,
                                    const sal_Int32     nOffX,
                                    const sal_Int32     nDstWidth,
                                    const long*         pMapX,
                                    const long*         pMapY );

    /** Retrieve downsampled and cropped bitmap

        @attention This method ignores negative rDstSz values, thus
        mirroring must happen outside this method (e.g. in DrawBitmap)
     */
    SAL_DLLPRIVATE Bitmap       GetDownsampledBitmap(
                                    const Size& rDstSz,
                                    const Point& rSrcPt,
                                    const Size& rSrcSz,
                                    const Bitmap& rBmp,
                                    long nMaxBmpDPIX,
                                    long nMaxBmpDPIY );

    ///@}


    /** @name Transparency functions
     */
    ///@{

public:

    /** Query availability of alpha channel

        @return sal_True, if this device has an alpha channel.
     */
    bool                        HasAlpha();


    /** helper method removing transparencies from a metafile (e.g. for printing)

        @returns
        true: transparencies were removed
        false: output metafile is unchanged input metafile

        @attention this is a member method, so current state can influence the result !
        @attention the output metafile is prepared in pixel mode for the currentOutputDevice
                   state. It can not be moved or rotated reliably anymore.
    */
    bool                        RemoveTransparenciesFromMetaFile(
                                    const GDIMetaFile& rInMtf, GDIMetaFile& rOutMtf,
                                    long nMaxBmpDPIX, long nMaxBmpDPIY,
                                    bool bReduceTransparency,
                                    bool bTransparencyAutoMode,
                                    bool bDownsampleBitmaps,
                                    const Color& rBackground = Color( COL_TRANSPARENT ) );

    SAL_DLLPRIVATE void         ImplPrintTransparent (
                                    const Bitmap& rBmp, const Bitmap& rMask,
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPtPixel, const Size& rSrcSizePixel );

    SAL_DLLPRIVATE Color        ImplDrawModeToColor  ( const Color& rColor ) const;


    /** Query the existence and depth of the alpha channel

        @return 0, if no alpha channel available, and the bit depth of
        the alpha channel otherwise.
     */
    virtual sal_uInt16          GetAlphaBitCount() const;


    void                        DrawTransparent( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent );
    void                        DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency);
    void                        DrawTransparent(
                                        const GDIMetaFile& rMtf, const Point& rPos, const Size& rSize,
                                        const Gradient& rTransparenceGradient );

protected:

    virtual void                EmulateDrawTransparent( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent );
    void                        DrawInvisiblePolygon( const tools::PolyPolygon& rPolyPoly );

    virtual void                ClipAndDrawGradientMetafile ( const Gradient &rGradient, const tools::PolyPolygon &rPolyPoly );

private:

    SAL_DLLPRIVATE bool         DrawTransparentNatively( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent );
    ///@}


    /** @name Mask functions
     */
    ///@{

public:

    void                        DrawMask( const Point& rDestPt,
                                          const Bitmap& rBitmap, const Color& rMaskColor );

    void                        DrawMask( const Point& rDestPt, const Size& rDestSize,
                                          const Bitmap& rBitmap, const Color& rMaskColor );

    void                        DrawMask( const Point& rDestPt, const Size& rDestSize,
                                          const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                          const Bitmap& rBitmap, const Color& rMaskColor,
                                          MetaActionType nAction );

protected:

    virtual void                DrawDeviceMask (
                                         const Bitmap& rMask, const Color& rMaskColor,
                                         const Point& rDestPt, const Size& rDestSize,
                                         const Point& rSrcPtPixel, const Size& rSrcSizePixel );
    ///@}


    /** @name Map functions
     */
    ///@{

public:

    void                        EnableMapMode( bool bEnable = true );
    bool                        IsMapModeEnabled() const { return mbMap; }

    void                        SetMapMode();
    virtual void                SetMapMode( const MapMode& rNewMapMode );
    void                        SetRelativeMapMode( const MapMode& rNewMapMode );
    const MapMode&              GetMapMode() const { return maMapMode; }

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
    Size                        GetPixelOffset() const { return Size(mnOutOffOrigX, mnOutOffOrigY);}

    Point                       LogicToPixel( const Point& rLogicPt ) const;
    Size                        LogicToPixel( const Size& rLogicSize ) const;
    Rectangle                   LogicToPixel( const Rectangle& rLogicRect ) const;
    Polygon                     LogicToPixel( const Polygon& rLogicPoly ) const;
    tools::PolyPolygon          LogicToPixel( const tools::PolyPolygon& rLogicPolyPoly ) const;
    basegfx::B2DPolyPolygon     LogicToPixel( const basegfx::B2DPolyPolygon& rLogicPolyPoly ) const;
    vcl::Region                      LogicToPixel( const vcl::Region& rLogicRegion )const;
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
    tools::PolyPolygon          LogicToPixel( const tools::PolyPolygon& rLogicPolyPoly,
                                              const MapMode& rMapMode ) const;
    basegfx::B2DPolyPolygon     LogicToPixel( const basegfx::B2DPolyPolygon& rLogicPolyPoly,
                                              const MapMode& rMapMode ) const;
    vcl::Region                 LogicToPixel( const vcl::Region& rLogicRegion,
                                              const MapMode& rMapMode ) const;
    basegfx::B2DPolygon         LogicToPixel( const basegfx::B2DPolygon& rLogicPoly ) const;

    Point                       PixelToLogic( const Point& rDevicePt ) const;
    Size                        PixelToLogic( const Size& rDeviceSize ) const;
    Rectangle                   PixelToLogic( const Rectangle& rDeviceRect ) const;
    Polygon                     PixelToLogic( const Polygon& rDevicePoly ) const;
    tools::PolyPolygon          PixelToLogic( const tools::PolyPolygon& rDevicePolyPoly ) const;
    basegfx::B2DPolyPolygon     PixelToLogic( const basegfx::B2DPolyPolygon& rDevicePolyPoly ) const;
    vcl::Region                 PixelToLogic( const vcl::Region& rDeviceRegion ) const;
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
    tools::PolyPolygon          PixelToLogic( const tools::PolyPolygon& rDevicePolyPoly,
                                              const MapMode& rMapMode ) const;
    basegfx::B2DPolyPolygon     PixelToLogic( const basegfx::B2DPolyPolygon& rDevicePolyPoly,
                                              const MapMode& rMapMode ) const;
    vcl::Region                 PixelToLogic( const vcl::Region& rDeviceRegion,
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

    /** Convert a logical rectangle to a rectangle in physical device pixel units.

     @param         rLogicSize  Const reference to a rectangle in logical units

     @returns Rectangle based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE Rectangle    ImplLogicToDevicePixel( const Rectangle& rLogicRect ) const;

    /** Convert a logical point to a physical point on the device.

     @param         rLogicPt    Const reference to a point in logical units.

     @returns Physical point on the device.
     */
    SAL_DLLPRIVATE Point        ImplLogicToDevicePixel( const Point& rLogicPt ) const;

    /** Convert a logical width to a width in units of device pixels.

     To get the number of device pixels, it must calculate the X-DPI of the device and
     the map scaling factor. If there is no mapping, then it just returns the
     width as nothing more needs to be done.

     @param         nWidth      Logical width

     @returns Width in units of device pixels.
     */
    SAL_DLLPRIVATE long         ImplLogicWidthToDevicePixel( long nWidth ) const;

    SAL_DLLPRIVATE DeviceCoordinate LogicWidthToDeviceCoordinate( long nWidth ) const;
    SAL_DLLPRIVATE DeviceCoordinate LogicHeightToDeviceCoordinate( long nHeight ) const;

protected:
    /**
     * Notification about some rectangle of the output device got invalidated.
     *
     * @param pRectangle If 0, that means the whole area, otherwise the area in logic coordinates.
     */
    virtual void LogicInvalidate(const Rectangle* /*pRectangle*/) { }

private:

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

    /** Convert a logical size to the size on the physical device.

     @param         rLogicSize  Const reference to a size in logical units

     @returns Physical size on the device.
     */
    SAL_DLLPRIVATE Size         ImplLogicToDevicePixel( const Size& rLogicSize ) const;

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
    SAL_DLLPRIVATE tools::PolyPolygon  ImplLogicToDevicePixel( const tools::PolyPolygon& rLogicPolyPoly ) const;

    /** Convert a line in logical units to a line in physical device pixel units.

     @param         rLineInfo   Const refernece to a line in logical units

     @returns Line based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE LineInfo     ImplLogicToDevicePixel( const LineInfo& rLineInfo ) const;

    /** Convert a region in pixel units to a region in device pixel units and coords.

     @param         rPixelRect  Const reference to region.

     @returns vcl::Region based on device pixel coordinates and units.
     */
    SAL_DLLPRIVATE vcl::Region       ImplPixelToDevicePixel( const vcl::Region& rRegion ) const;

    /** Invalidate the view transformation.

     @since AOO bug 75163 (OpenOffice.org 2.4.3 - OOH 680 milestone 212)
     */
    SAL_DLLPRIVATE void         ImplInvalidateViewTransform();

    /** Get device transformation.

     @since AOO bug 75163 (OpenOffice.org 2.4.3 - OOH 680 milestone 212)
     */
    SAL_DLLPRIVATE basegfx::B2DHomMatrix ImplGetDeviceTransformation() const;
    ///@}


    /** @name Native Widget Rendering functions

        These all just call through to the private mpGraphics functions of the same name.
     */
    ///@{

public:

    /** Query the platform layer for control support
     */
    bool                        IsNativeControlSupported( ControlType nType, ControlPart nPart ) const;

    /** Query the native control to determine if it was acted upon
     */
    bool                        HitTestNativeControl(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    const Point& aPos,
                                    bool& rIsInside ) const;

    /** Request rendering of a particular control and/or part
     */
    bool                        DrawNativeControl(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    const OUString& aCaption );

    /** Query the native control's actual drawing region (including adornment)
     */
    bool                        GetNativeControlRegion(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    const OUString& aCaption,
                                    Rectangle &rNativeBoundingRegion,
                                    Rectangle &rNativeContentRegion ) const;
    ///@}

    /** @name EPS functions
     */
    ///@{

public:

    /** @returns boolean value to see if EPS could be painted directly.
        Theoreticaly, handing over a matrix would be needed to handle
        painting rotated EPS files (e.g. contained in Metafiles). This
        would then need to be supported for Mac and PS printers, but
        that's too much for now, wrote \#i107046# for this */
    bool                        DrawEPS(
                                    const Point& rPt, const Size& rSz,
                                    const GfxLink& rGfxLink, GDIMetaFile* pSubst = NULL );
    ///@}
};

#endif // INCLUDED_VCL_OUTDEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
