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

#include <sal/config.h>

#include <o3tl/span.hxx>
#include <tools/gen.hxx>
#include <tools/ref.hxx>
#include <tools/solar.h>
#include <tools/color.hxx>
#include <tools/poly.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/cairo.hxx>
#include <vcl/devicecoordinate.hxx>
#include <vcl/dllapi.h>
#include <vcl/font.hxx>
#include <vcl/region.hxx>
#include <vcl/rendercontext/AddFontSubstituteFlags.hxx>
#include <vcl/rendercontext/AntialiasingFlags.hxx>
#include <vcl/rendercontext/SystemTextColorFlags.hxx>
#include <vcl/rendercontext/DrawGridFlags.hxx>
#include <vcl/rendercontext/DrawImageFlags.hxx>
#include <vcl/rendercontext/DrawModeFlags.hxx>
#include <vcl/rendercontext/DrawTextFlags.hxx>
#include <vcl/rendercontext/GetDefaultFontFlags.hxx>
#include <vcl/rendercontext/ImplMapRes.hxx>
#include <vcl/rendercontext/InvertFlags.hxx>
#include <vcl/rendercontext/RasterOp.hxx>
#include <vcl/rendercontext/SalLayoutFlags.hxx>
#include <vcl/rendercontext/State.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/wall.hxx>
#include <vcl/metaactiontypes.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/vclreferencebase.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <unotools/fontdefs.hxx>
#include <cppuhelper/weakref.hxx>

#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/DeviceInfo.hpp>

#include <memory>
#include <string_view>
#include <vector>

struct ImplOutDevData;
class LogicalFontInstance;
struct SystemGraphicsData;
class ImplFontCache;
class ImplMultiTextLineInfo;
class SalGraphics;
class Gradient;
class Hatch;
class AllSettings;
class BitmapReadAccess;
class BitmapEx;
class Image;
class TextRectInfo;
class FontMetric;
class GDIMetaFile;
class GfxLink;
namespace tools {
    class Line;
}
class LineInfo;
class AlphaMask;
class FontCharMap;
class SalLayout;
class VirtualDevice;
struct SalTwoRect;
class Printer;
class VCLXGraphics;
class OutDevStateStack;
class SalLayoutGlyphs;

namespace vcl
{
    class ExtOutDevData;
    class ITextLayout;
    struct FontCapabilities;
    class Window;
    class WindowOutputDevice;
    namespace font {
        struct Feature;
        class PhysicalFontCollection;
        class PhysicalFontFaceCollection;
    }

    namespace text {
        class ImplLayoutArgs;
        class TextLayoutCache;
    }
}

namespace basegfx {
    class B2DHomMatrix;
    class B2DPolygon;
    class B2IVector;
    typedef B2IVector B2ISize;
}

namespace com::sun::star::awt {
    class XGraphics;
}

namespace com::sun::star::rendering {
    class XCanvas;
    class XSpriteCanvas;
}
namespace com::sun::star::linguistic2 {
    class XHyphenator;
}
namespace com::sun::star::i18n {
    class XBreakIterator;
}

#if defined UNX
#define GLYPH_FONT_HEIGHT   128
#else
#define GLYPH_FONT_HEIGHT   256
#endif

// OutputDevice-Types

enum OutDevType { OUTDEV_WINDOW, OUTDEV_PRINTER, OUTDEV_VIRDEV, OUTDEV_PDF };

enum class OutDevViewType { DontKnow, PrintPreview, SlideShow };

// OutputDevice

typedef tools::SvRef<FontCharMap> FontCharMapRef;

BmpMirrorFlags AdjustTwoRect( SalTwoRect& rTwoRect, const Size& rSizePix );
void AdjustTwoRect( SalTwoRect& rTwoRect, const tools::Rectangle& rValidSrcRect );

class OutputDevice;

namespace vcl {
    typedef OutputDevice RenderContext;
}

VCL_DLLPUBLIC void InvertFocusRect(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);

typedef struct _cairo_surface cairo_surface_t;

/**
* Some things multiple-inherit from VclAbstractDialog and OutputDevice,
* so we need to use virtual inheritance to keep the referencing counting
* OK.
*/
class SAL_WARN_UNUSED VCL_DLLPUBLIC OutputDevice : public virtual VclReferenceBase
{
    friend class Printer;
    friend class VirtualDevice;
    friend class vcl::Window;
    friend class vcl::WindowOutputDevice;
    friend class WorkWindow;
    friend void ImplHandleResize( vcl::Window* pWindow, tools::Long nNewWidth, tools::Long nNewHeight );

private:
    OutputDevice(const OutputDevice&) = delete;
    OutputDevice& operator=(const OutputDevice&) = delete;

    mutable SalGraphics*            mpGraphics;         ///< Graphics context to draw on
    mutable VclPtr<OutputDevice>    mpPrevGraphics;     ///< Previous output device in list
    mutable VclPtr<OutputDevice>    mpNextGraphics;     ///< Next output device in list
    GDIMetaFile*                    mpMetaFile;
    mutable rtl::Reference<LogicalFontInstance> mpFontInstance;
    mutable std::unique_ptr<vcl::font::PhysicalFontFaceCollection>  mpFontFaceCollection;
    std::vector<vcl::State>        maOutDevStateStack;
    std::unique_ptr<ImplOutDevData> mpOutDevData;
    std::vector< VCLXGraphics* >*   mpUnoGraphicsList;
    vcl::ExtOutDevData*             mpExtOutDevData;
    // The canvas interface for this output device. Is persistent after the first GetCanvas() call
    mutable css::uno::WeakReference< css::rendering::XCanvas >    mxCanvas;

    // TEMP TEMP TEMP
    VclPtr<VirtualDevice>           mpAlphaVDev;

    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    tools::Long                            mnOutOffOrigX;
    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    tools::Long                            mnOutOffLogicX;
    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    tools::Long                            mnOutOffOrigY;
    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    tools::Long                            mnOutOffLogicY;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    tools::Long                            mnOutOffX;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    tools::Long                            mnOutOffY;
    tools::Long                            mnOutWidth;
    tools::Long                            mnOutHeight;
    sal_Int32                       mnDPIX;
    sal_Int32                       mnDPIY;
    sal_Int32                       mnDPIScalePercentage; ///< For HiDPI displays, we want to draw elements for a percentage larger
    /// font specific text alignment offsets in pixel units
    mutable tools::Long                    mnTextOffX;
    mutable tools::Long                    mnTextOffY;
    mutable tools::Long                    mnEmphasisAscent;
    mutable tools::Long                    mnEmphasisDescent;
    DrawModeFlags                   mnDrawMode;
    vcl::text::ComplexTextLayoutFlags mnTextLayoutMode;
    ImplMapRes                      maMapRes;
    const OutDevType                meOutDevType;
    OutDevViewType                  meOutDevViewType;
    vcl::Region                     maRegion;           // contains the clip region, see SetClipRegion(...)
    Color                           maLineColor;
    Color                           maFillColor;
    vcl::Font                       maFont;
    Color                           maTextColor;
    Color                           maTextLineColor;
    Color                           maOverlineColor;
    RasterOp                        meRasterOp;
    Wallpaper                       maBackground;
    std::unique_ptr<AllSettings>    mxSettings;
    MapMode                         maMapMode;
    Point                           maRefPoint;
    AntialiasingFlags               mnAntialiasing;
    LanguageType                    meTextLanguage;

    mutable bool                    mbMap : 1;
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
    mutable bool                    mbNewFont : 1;
    mutable bool                    mbTextLines : 1;
    mutable bool                    mbTextSpecial : 1;
    mutable bool                    mbRefPoint : 1;
    mutable bool                    mbEnableRTL : 1;

protected:
    mutable std::shared_ptr<vcl::font::PhysicalFontCollection> mxFontCollection;
    mutable std::shared_ptr<ImplFontCache> mxFontCache;

    /** @name Initialization and accessor functions
     */
    ///@{

protected:
                                OutputDevice(OutDevType eOutDevType);
    virtual                     ~OutputDevice() override;
    virtual void                dispose() override;

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
    css::uno::Any               GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const;
    css::uno::Any               GetSystemGfxDataAny() const;

    void                        SetRefPoint();
    void                        SetRefPoint( const Point& rRefPoint );
    const Point&                GetRefPoint() const { return maRefPoint; }
    bool                        IsRefPoint() const { return mbRefPoint; }

    virtual bool                IsScreenComp() const { return true; }

    virtual sal_uInt16          GetBitCount() const;

    Size                        GetOutputSizePixel() const
                                    { return Size( mnOutWidth, mnOutHeight ); }
    tools::Long                        GetOutputWidthPixel() const { return mnOutWidth; }
    tools::Long                        GetOutputHeightPixel() const { return mnOutHeight; }
    tools::Long                        GetOutOffXPixel() const { return mnOutOffX; }
    tools::Long                        GetOutOffYPixel() const { return mnOutOffY; }
    void                        SetOutOffXPixel(tools::Long nOutOffX);
    void                        SetOutOffYPixel(tools::Long nOutOffY);
    Point                       GetOutputOffPixel() const
                                    { return Point( mnOutOffX, mnOutOffY ); }
    tools::Rectangle            GetOutputRectPixel() const
                                    { return tools::Rectangle(GetOutputOffPixel(), GetOutputSizePixel() ); }

    Size                        GetOutputSize() const
                                    { return PixelToLogic( GetOutputSizePixel() ); }

    css::uno::Reference< css::awt::XGraphics >
                                CreateUnoGraphics();
    std::vector< VCLXGraphics* > *GetUnoGraphicsList() const  { return mpUnoGraphicsList; }
    std::vector< VCLXGraphics* > *CreateUnoGraphicsList();

    virtual size_t               GetSyncCount() const { return 0xffffffff; }

    /// request XCanvas render interface
    css::uno::Reference< css::rendering::XCanvas > GetCanvas() const;
    /// request XSpriteCanvas render interface
    css::uno::Reference< css::rendering::XSpriteCanvas > GetSpriteCanvas() const;

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

    float GetDPIScaleFactor() const
    {
        return mnDPIScalePercentage / 100.0f;
    }

    sal_Int32 GetDPIScalePercentage() const
    {
        return mnDPIScalePercentage;
    }

    OutDevType                  GetOutDevType() const { return meOutDevType; }
    virtual bool IsVirtual() const;

    /** Query an OutputDevice to see whether it supports a specific operation

     @returns true if operation supported, else false
    */
    bool                        SupportsOperation( OutDevSupportType ) const;

    void                        SetExtOutDevData( vcl::ExtOutDevData* pExtOutDevData ) { mpExtOutDevData = pExtOutDevData; }
    vcl::ExtOutDevData*         GetExtOutDevData() const { return mpExtOutDevData; }

    ///@}

public:
    virtual Size GetButtonBorderSize() { return Size(1, 1); };
    virtual Color GetMonochromeButtonColor() { return COL_WHITE; }

    /** @name Direct OutputDevice drawing functions
     */
    ///@{

public:
    virtual void                Flush() {}

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

protected:

    virtual void                CopyDeviceArea( SalTwoRect& aPosAry, bool bWindowInvalidate);

    virtual tools::Rectangle    GetBackgroundComponentBounds() const;

    virtual const OutputDevice* DrawOutDevDirectCheck(const OutputDevice& rSrcDev) const;

    virtual void                DrawOutDevDirectProcess(const OutputDevice& rSrcDev, SalTwoRect& rPosAry, SalGraphics* pSrcGraphics);

    SAL_DLLPRIVATE void         drawOutDevDirect(const OutputDevice& rSrcDev, SalTwoRect& rPosAry);

    SAL_DLLPRIVATE bool         is_double_buffered_window() const;

    virtual css::uno::Reference< css::rendering::XCanvas > ImplGetCanvas( bool bSpriteCanvas ) const;
    SAL_DLLPRIVATE void         ImplDisposeCanvas();

private:

    // not implemented; to detect misuses of DrawOutDev(...OutputDevice&);
    SAL_DLLPRIVATE void         DrawOutDev( const Point&, const Size&, const Point&,  const Size&, const Printer&) = delete;
    ///@}


    /** @name OutputDevice state functions
     */
    ///@{

public:

    void                        Push( vcl::PushFlags nFlags = vcl::PushFlags::ALL );
    void                        Pop();
    void                        ClearStack();

    void                        EnableOutput( bool bEnable = true );
    bool                        IsOutputEnabled() const { return mbOutput; }
    bool                        IsDeviceOutputNecessary() const { return (mbOutput && mbDevOutput); }

    void                        SetAntialiasing( AntialiasingFlags nMode );
    AntialiasingFlags           GetAntialiasing() const { return mnAntialiasing; }

    void                        SetDrawMode( DrawModeFlags nDrawMode );
    DrawModeFlags               GetDrawMode() const { return mnDrawMode; }

    void                        SetLayoutMode( vcl::text::ComplexTextLayoutFlags nTextLayoutMode );
    vcl::text::ComplexTextLayoutFlags GetLayoutMode() const { return mnTextLayoutMode; }

    void                        SetDigitLanguage( LanguageType );
    LanguageType                GetDigitLanguage() const { return meTextLanguage; }

    void                        SetRasterOp( RasterOp eRasterOp );
    RasterOp                    GetRasterOp() const { return meRasterOp; }

    /**
    If this OutputDevice is used for displaying a Print Preview
    the OutDevViewType should be set to 'OutDevViewType::PrintPreview'.

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
    virtual void                SaveBackground(VirtualDevice& rSaveDevice,
                                               const Point& rPos, const Size& rSize, const Size& rBackgroundSize) const;

    const Wallpaper&            GetBackground() const { return maBackground; }
    virtual Color               GetBackgroundColor() const;
    virtual Color               GetReadableFontColor(const Color& rFontColor, const Color& rBgColor) const;
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
    bool                        SelectClipRegion( const vcl::Region&, SalGraphics* pGraphics = nullptr );

    bool                        IsClipRegion() const { return mbClipRegion; }

    void                        MoveClipRegion( tools::Long nHorzMove, tools::Long nVertMove );
    void                        IntersectClipRegion( const tools::Rectangle& rRect );
    void                        IntersectClipRegion( const vcl::Region& rRegion );

    virtual vcl::Region         GetActiveClipRegion() const;
    virtual vcl::Region         GetOutputBoundsClipRegion() const;

protected:

    virtual void                InitClipRegion();

    /** Perform actual rect clip against outdev dimensions, to generate
        empty clips whenever one of the values is completely off the device.

        @param aRegion      region to be clipped to the device dimensions
        @returns            region clipped to the device bounds
     **/
    virtual vcl::Region         ClipToDeviceBounds(vcl::Region aRegion) const;
    virtual void                ClipToPaintRegion    ( tools::Rectangle& rDstRect );

private:

    SAL_DLLPRIVATE void         SetDeviceClipRegion( const vcl::Region* pRegion );
    ///@}

public:
    virtual void                DrawBorder(tools::Rectangle aBorderRect);


    /** @name Pixel functions
     */
    ///@{

public:

    void                        DrawPixel( const Point& rPt );
    void                        DrawPixel( const Point& rPt, const Color& rColor );

    Color                       GetPixel( const Point& rPt ) const;
    ///@}


    /** @name Rectangle functions
     */
    ///@{

public:

    void                        DrawRect( const tools::Rectangle& rRect );
    void                        DrawRect( const tools::Rectangle& rRect,
                                          sal_uLong nHorzRount, sal_uLong nVertRound );

    /// Fill the given rectangle with checkered rectangles of size nLen x nLen using the colors aStart and aEnd
    void                        DrawCheckered(
                                    const Point& rPos,
                                    const Size& rSize,
                                    sal_uInt32 nLen = 8,
                                    Color aStart = COL_WHITE,
                                    Color aEnd = COL_BLACK);

    void                        DrawGrid( const tools::Rectangle& rRect, const Size& rDist, DrawGridFlags nFlags );

    ///@}

    /** @name Invert functions
     */
    ///@{
public:
    void Invert( const tools::Rectangle& rRect, InvertFlags nFlags = InvertFlags::NONE );
    void Invert( const tools::Polygon& rPoly, InvertFlags nFlags = InvertFlags::NONE );
    ///@}

    /** @name Line functions
     */
    ///@{

public:

    void                        DrawLine( const Point& rStartPt, const Point& rEndPt );

    void                        DrawLine( const Point& rStartPt, const Point& rEndPt,
                                          const LineInfo& rLineInfo );

protected:
    virtual void DrawHatchLine_DrawLine(const Point& rStartPoint, const Point& rEndPoint);

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
    void                        DrawPolyLine( const tools::Polygon& rPoly );

    void                        DrawPolyLine(
                                    const basegfx::B2DPolygon&,
                                    double fLineWidth = 0.0,
                                    basegfx::B2DLineJoin eLineJoin = basegfx::B2DLineJoin::Round,
                                    css::drawing::LineCap eLineCap = css::drawing::LineCap_BUTT,
                                    double fMiterMinimumAngle = basegfx::deg2rad(15.0));

    /** Render the given polygon as a line stroke

        The given polygon is stroked with the current LineColor, start
        and end point are not automatically connected. The line is
        rendered according to the specified LineInfo, e.g. supplying a
        dash pattern, or a line thickness.

        @see DrawPolygon
        @see DrawPolyPolygon
     */
    void                        DrawPolyLine( const tools::Polygon& rPoly,
                                              const LineInfo& rLineInfo );

    // #i101491#
    // Helper who tries to use SalGDI's DrawPolyLine direct and returns it's bool.
    bool                        DrawPolyLineDirect(
                                    const basegfx::B2DHomMatrix& rObjectTransform,
                                    const basegfx::B2DPolygon& rB2DPolygon,
                                    double fLineWidth = 0.0,
                                    double fTransparency = 0.0,
                                    const std::vector< double >* = nullptr, // MM01
                                    basegfx::B2DLineJoin eLineJoin = basegfx::B2DLineJoin::NONE,
                                    css::drawing::LineCap eLineCap = css::drawing::LineCap_BUTT,
                                    double fMiterMinimumAngle = basegfx::deg2rad(15.0));

private:

    // #i101491#
    // Helper which holds the old line geometry creation and is extended to use AA when
    // switched on. Advantage is that line geometry is only temporarily used for paint
    SAL_DLLPRIVATE void         drawPolyLine(const tools::Polygon& rPoly, const LineInfo& rLineInfo);

    ///@}

    bool                        DrawPolyLineDirectInternal(
                                    const basegfx::B2DHomMatrix& rObjectTransform,
                                    const basegfx::B2DPolygon& rB2DPolygon,
                                    double fLineWidth = 0.0,
                                    double fTransparency = 0.0,
                                    const std::vector< double >* = nullptr, // MM01
                                    basegfx::B2DLineJoin eLineJoin = basegfx::B2DLineJoin::NONE,
                                    css::drawing::LineCap eLineCap = css::drawing::LineCap_BUTT,
                                    double fMiterMinimumAngle = basegfx::deg2rad(15.0));

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
    void                        DrawPolygon( const tools::Polygon& rPoly );
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

    SAL_DLLPRIVATE void         ImplDrawPolygon( const tools::Polygon& rPoly, const tools::PolyPolygon* pClipPolyPoly = nullptr );
    SAL_DLLPRIVATE void         ImplDrawPolyPolygon( const tools::PolyPolygon& rPolyPoly, const tools::PolyPolygon* pClipPolyPoly );
    SAL_DLLPRIVATE void         ImplDrawPolyPolygon( sal_uInt16 nPoly, const tools::PolyPolygon& rPolyPoly );
    // #i101491#
    // Helper who implements the DrawPolyPolygon functionality for basegfx::B2DPolyPolygon
    // without MetaFile processing
    SAL_DLLPRIVATE void         ImplDrawPolyPolygonWithB2DPolyPolygon(const basegfx::B2DPolyPolygon& rB2DPolyPoly);
    ///@}

    SAL_DLLPRIVATE void         ImplDrawWaveLineBezier(tools::Long nStartX, tools::Long nStartY, tools::Long nEndX, tools::Long nEndY, tools::Long nWaveHeight, double fOrientation, tools::Long nLineWidth);


    /** @name Curved shape functions
     */
    ///@{

public:

    void                        DrawEllipse( const tools::Rectangle& rRect );

    void                        DrawArc(
                                    const tools::Rectangle& rRect,
                                    const Point& rStartPt, const Point& rEndPt );

    void                        DrawPie(
                                    const tools::Rectangle& rRect,
                                    const Point& rStartPt, const Point& rEndPt );

    void                        DrawChord(
                                    const tools::Rectangle& rRect,
                                    const Point& rStartPt, const Point& rEndPt );

    ///@}


    /** @name Gradient functions
     */
    ///@{

public:
    void                        DrawGradient( const tools::Rectangle& rRect, const Gradient& rGradient );
    void                        DrawGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient );

    void                        AddGradientActions(
                                    const tools::Rectangle& rRect,
                                    const Gradient& rGradient,
                                    GDIMetaFile& rMtf );

protected:

    virtual bool                UsePolyPolygonForComplexGradient() = 0;

    virtual tools::Long                GetGradientStepCount( tools::Long nMinRect );

private:

    SAL_DLLPRIVATE void         DrawLinearGradient( const tools::Rectangle& rRect, const Gradient& rGradient, const tools::PolyPolygon* pClipPolyPoly );
    SAL_DLLPRIVATE void         DrawComplexGradient( const tools::Rectangle& rRect, const Gradient& rGradient, const tools::PolyPolygon* pClipPolyPoly );

    SAL_DLLPRIVATE void         DrawGradientToMetafile( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient );
    SAL_DLLPRIVATE void         DrawLinearGradientToMetafile( const tools::Rectangle& rRect, const Gradient& rGradient );
    SAL_DLLPRIVATE void         DrawComplexGradientToMetafile( const tools::Rectangle& rRect, const Gradient& rGradient );

    SAL_DLLPRIVATE tools::Long  GetLinearGradientSteps( const Gradient& rGradient, const tools::Rectangle& rRect, bool bMtf);
    SAL_DLLPRIVATE tools::Long  GetComplexGradientSteps( const Gradient& rGradient, const tools::Rectangle& rRect, bool bMtf);

    SAL_DLLPRIVATE Color        GetSingleColorGradientFill();
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

    SAL_DLLPRIVATE void         CalcHatchValues( const tools::Rectangle& rRect, tools::Long nDist, Degree10 nAngle10, Point& rPt1, Point& rPt2, Size& rInc, Point& rEndPt1 );
    SAL_DLLPRIVATE void         DrawHatchLine( const tools::Line& rLine, const tools::PolyPolygon& rPolyPoly, Point* pPtBuffer, bool bMtf );
    ///@}


    /** @name Wallpaper functions
     */
    ///@{

public:
    void                        DrawWallpaper( const tools::Rectangle& rRect, const Wallpaper& rWallpaper );

    void                        Erase();
    void                        Erase(const tools::Rectangle& rRect);

protected:
    void                        DrawGradientWallpaper( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, const Wallpaper& rWallpaper );

private:
    SAL_DLLPRIVATE void         DrawWallpaper( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         DrawColorWallpaper( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, const Wallpaper& rWallpaper );
    SAL_DLLPRIVATE void         DrawBitmapWallpaper( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, const Wallpaper& rWallpaper );
    ///@}


    /** @name Text functions
     */
    ///@{

public:

    void                        DrawText( const Point& rStartPt, const OUString& rStr,
                                          sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                          std::vector< tools::Rectangle >* pVector = nullptr, OUString* pDisplayText = nullptr,
                                          const SalLayoutGlyphs* pLayoutCache = nullptr );

    void                        DrawText( const tools::Rectangle& rRect,
                                          const OUString& rStr, DrawTextFlags nStyle = DrawTextFlags::NONE,
                                          std::vector< tools::Rectangle >* pVector = nullptr, OUString* pDisplayText = nullptr,
                                          vcl::ITextLayout* _pTextLayout = nullptr );

    static void                 ImplDrawText( OutputDevice& rTargetDevice, const tools::Rectangle& rRect,
                                              const OUString& rOrigStr, DrawTextFlags nStyle,
                                              std::vector< tools::Rectangle >* pVector, OUString* pDisplayText, vcl::ITextLayout& _rLayout );

    void                        ImplDrawText( SalLayout& );

    void                        ImplDrawTextBackground( const SalLayout& );

    void                        DrawCtrlText( const Point& rPos, const OUString& rStr,
                                              sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                              DrawTextFlags nStyle = DrawTextFlags::Mnemonic, std::vector< tools::Rectangle >* pVector = nullptr, OUString* pDisplayText = nullptr,
                                              const SalLayoutGlyphs* pGlyphs = nullptr);

    void                        DrawTextLine( const Point& rPos, tools::Long nWidth,
                                              FontStrikeout eStrikeout,
                                              FontLineStyle eUnderline,
                                              FontLineStyle eOverline,
                                              bool bUnderlineAbove = false );

    void                        ImplDrawTextLine( tools::Long nBaseX, tools::Long nX, tools::Long nY, DeviceCoordinate nWidth,
                                                  FontStrikeout eStrikeout, FontLineStyle eUnderline,
                                                  FontLineStyle eOverline, bool bUnderlineAbove );

    void                        ImplDrawTextLines( SalLayout&, FontStrikeout eStrikeout, FontLineStyle eUnderline,
                                                   FontLineStyle eOverline, bool bWordLine, bool bUnderlineAbove );

    void                        DrawWaveLine( const Point& rStartPos, const Point& rEndPos, tools::Long nLineWidth = 1, tools::Long nWaveHeight = 3);

    bool                        ImplDrawRotateText( SalLayout& );

    tools::Rectangle                   GetTextRect( const tools::Rectangle& rRect,
                                             const OUString& rStr, DrawTextFlags nStyle = DrawTextFlags::WordBreak,
                                             TextRectInfo* pInfo = nullptr,
                                             const vcl::ITextLayout* _pTextLayout = nullptr ) const;

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

        tools::Rectangle aRect;
        aDevice.GetTextBoundRect(aRect, aText);
        aDevice.SetOutputSize(Size(aRect.Right() + 1, aRect.Bottom() + 1));
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
    bool                        GetTextBoundRect( tools::Rectangle& rRect,
                                                  const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                                  sal_uLong nLayoutWidth = 0, o3tl::span<const sal_Int32> pDXArray = {},
                                                  const SalLayoutGlyphs* pGlyphs = nullptr ) const;

    tools::Rectangle            ImplGetTextBoundRect( const SalLayout& ) const;

    bool                        GetTextOutline( tools::PolyPolygon&,
                                                const OUString& rStr ) const;

    bool                        GetTextOutlines( PolyPolyVector&,
                                                 const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0,
                                                 sal_Int32 nLen = -1,
                                                 sal_uLong nLayoutWidth = 0, o3tl::span<const sal_Int32> pDXArray = {} ) const;

    bool                        GetTextOutlines( basegfx::B2DPolyPolygonVector &rVector,
                                                 const OUString& rStr, sal_Int32 nBase, sal_Int32 nIndex = 0,
                                                 sal_Int32 nLen = -1,
                                                 sal_uLong nLayoutWidth = 0, o3tl::span<const sal_Int32> pDXArray = {} ) const;


    OUString                    GetEllipsisString( const OUString& rStr, tools::Long nMaxWidth,
                                                   DrawTextFlags nStyle = DrawTextFlags::EndEllipsis ) const;

    tools::Long                        GetCtrlTextWidth( const OUString& rStr,
                                                  const SalLayoutGlyphs* pLayoutCache = nullptr ) const;

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
    void                        AddTextRectActions( const tools::Rectangle& rRect,
                                                    const OUString&  rOrigStr,
                                                    DrawTextFlags    nStyle,
                                                    GDIMetaFile&     rMtf );

    void                        SetTextColor( const Color& rColor );
    virtual void                SetSystemTextColor(SystemTextColorFlags nFlags, bool bEnabled);
    const Color&                GetTextColor() const { return maTextColor; }

    void                        SetTextFillColor();
    void                        SetTextFillColor( const Color& rColor );
    Color                       GetTextFillColor() const;
    bool                        IsTextFillColor() const { return !maFont.IsTransparent(); }

    void                        SetTextLineColor();
    void                        SetTextLineColor( const Color& rColor );
    const Color&                GetTextLineColor() const { return maTextLineColor; }
    bool                        IsTextLineColor() const { return !maTextLineColor.IsTransparent(); }

    void                        SetOverlineColor();
    void                        SetOverlineColor( const Color& rColor );
    const Color&                GetOverlineColor() const { return maOverlineColor; }
    bool                        IsOverlineColor() const { return !maOverlineColor.IsTransparent(); }

    void                        SetTextAlign( TextAlign eAlign );
    TextAlign                   GetTextAlign() const { return maFont.GetAlignment(); }

    /** Width of the text.

        See also GetTextBoundRect() for more explanation + code examples.
    */
    tools::Long                        GetTextWidth( const OUString& rStr, sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                  vcl::text::TextLayoutCache const* = nullptr,
                                  SalLayoutGlyphs const*const pLayoutCache = nullptr) const;

    /** Height where any character of the current font fits; in logic coordinates.

        See also GetTextBoundRect() for more explanation + code examples.
    */
    tools::Long                        GetTextHeight() const;
    float                       approximate_digit_width() const;

    void                        DrawTextArray( const Point& rStartPt, const OUString& rStr,
                                               o3tl::span<const sal_Int32> pDXAry,
                                               sal_Int32 nIndex = 0,
                                               sal_Int32 nLen = -1,
                                               SalLayoutFlags flags = SalLayoutFlags::NONE,
                                               const SalLayoutGlyphs* pLayoutCache = nullptr);
    tools::Long                        GetTextArray( const OUString& rStr, std::vector<sal_Int32>* pDXAry,
                                              sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                              vcl::text::TextLayoutCache const* = nullptr,
                                              SalLayoutGlyphs const*const pLayoutCache = nullptr) const;

    void                        GetCaretPositions( const OUString&, sal_Int32* pCaretXArray,
                                              sal_Int32 nIndex, sal_Int32 nLen,
                                              const SalLayoutGlyphs* pGlyphs = nullptr ) const;
    void                        DrawStretchText( const Point& rStartPt, sal_uLong nWidth,
                                                 const OUString& rStr,
                                                 sal_Int32 nIndex = 0, sal_Int32 nLen = -1);
    sal_Int32                   GetTextBreak( const OUString& rStr, tools::Long nTextWidth,
                                              sal_Int32 nIndex, sal_Int32 nLen = -1,
                                              tools::Long nCharExtra = 0,
                                              vcl::text::TextLayoutCache const* = nullptr,
                                              const SalLayoutGlyphs* pGlyphs = nullptr) const;
    sal_Int32                   GetTextBreak( const OUString& rStr, tools::Long nTextWidth,
                                              sal_Unicode nExtraChar, sal_Int32& rExtraCharPos,
                                              sal_Int32 nIndex, sal_Int32 nLen,
                                              tools::Long nCharExtra,
                                              vcl::text::TextLayoutCache const* = nullptr) const;
    static std::shared_ptr<vcl::text::TextLayoutCache> CreateTextLayoutCache(OUString const&);

protected:
    SAL_DLLPRIVATE void         ImplInitTextLineSize();
    SAL_DLLPRIVATE void         ImplInitAboveTextLineSize();
    static
    SAL_DLLPRIVATE tools::Long  ImplGetTextLines( const tools::Rectangle& rRect, tools::Long nTextHeight, ImplMultiTextLineInfo& rLineInfo, tools::Long nWidth, const OUString& rStr, DrawTextFlags nStyle, const vcl::ITextLayout& _rLayout );
    static
    SAL_DLLPRIVATE sal_Int32    ImplBreakLinesWithIterator(const tools::Long nWidth, const OUString& rStr, const vcl::ITextLayout& _rLayout,
                                    const css::uno::Reference<css::linguistic2::XHyphenator>& xHyph,
                                    const css::uno::Reference<css::i18n::XBreakIterator>& xBI,
                                    const bool bHyphenate,
                                    const sal_Int32 nPos, sal_Int32 nBreakPos);
    static
    SAL_DLLPRIVATE sal_Int32    ImplBreakLinesSimple( const tools::Long nWidth, const OUString& rStr,
                                    const vcl::ITextLayout& _rLayout, const sal_Int32 nPos, sal_Int32 nBreakPos, tools::Long& nLineWidth );
    SAL_DLLPRIVATE float        approximate_char_width() const;

    virtual bool shouldDrawWavePixelAsRect(tools::Long nLineWidth) const;
    virtual void SetWaveLineColors(Color const& rColor, tools::Long nLineWidth);
    virtual Size GetWaveLineSize(tools::Long nLineWidth) const;

private:
    SAL_DLLPRIVATE void         ImplInitTextColor();

    SAL_DLLPRIVATE void         ImplDrawTextDirect( SalLayout&, bool bTextLines);
    SAL_DLLPRIVATE void         ImplDrawSpecialText( SalLayout& );
    SAL_DLLPRIVATE void         ImplDrawTextRect( tools::Long nBaseX, tools::Long nBaseY, tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight );

    SAL_DLLPRIVATE void  ImplDrawWavePixel( tools::Long nOriginX, tools::Long nOriginY, tools::Long nCurX, tools::Long nCurY, tools::Long nWidth, Degree10 nOrientation, SalGraphics* pGraphics, const OutputDevice& rOutDev, tools::Long nPixWidth, tools::Long nPixHeight );

    SAL_DLLPRIVATE void         ImplDrawWaveLine( tools::Long nBaseX, tools::Long nBaseY, tools::Long nStartX, tools::Long nStartY, tools::Long nWidth, tools::Long nHeight, tools::Long nLineWidth, Degree10 nOrientation, const Color& rColor );
    SAL_DLLPRIVATE void         ImplDrawWaveTextLine( tools::Long nBaseX, tools::Long nBaseY, tools::Long nX, tools::Long nY, tools::Long nWidth, FontLineStyle eTextLine, Color aColor, bool bIsAbove );
    SAL_DLLPRIVATE void         ImplDrawStraightTextLine( tools::Long nBaseX, tools::Long nBaseY, tools::Long nX, tools::Long nY, tools::Long nWidth, FontLineStyle eTextLine, Color aColor, bool bIsAbove );
    SAL_DLLPRIVATE void         ImplDrawStrikeoutLine( tools::Long nBaseX, tools::Long nBaseY, tools::Long nX, tools::Long nY, tools::Long nWidth, FontStrikeout eStrikeout, Color aColor );
    SAL_DLLPRIVATE void         ImplDrawStrikeoutChar( tools::Long nBaseX, tools::Long nBaseY, tools::Long nX, tools::Long nY, tools::Long nWidth, FontStrikeout eStrikeout, Color aColor );
    SAL_DLLPRIVATE void         ImplDrawMnemonicLine( tools::Long nX, tools::Long nY, tools::Long nWidth );


    ///@}


    /** @name Font functions
     */
    ///@{

public:

    FontMetric                  GetFontMetricFromCollection( int nDevFontIndex ) const;
    int                         GetFontFaceCollectionCount() const;

    bool                        IsFontAvailable( std::u16string_view rFontName ) const;

    bool                        AddTempDevFont( const OUString& rFileURL, const OUString& rFontName );
    void                        RefreshFontData( const bool bNewFontLists );

    FontMetric                  GetFontMetric() const;
    FontMetric                  GetFontMetric( const vcl::Font& rFont ) const;

    bool                        GetFontCharMap( FontCharMapRef& rxFontCharMap ) const;
    bool                        GetFontCapabilities( vcl::FontCapabilities& rFontCapabilities ) const;

    bool GetFontFeatures(std::vector<vcl::font::Feature>& rFontFeatures) const;

    SAL_DLLPRIVATE void         ImplGetEmphasisMark( tools::PolyPolygon& rPolyPoly, bool& rPolyLine, tools::Rectangle& rRect1, tools::Rectangle& rRect2,
                                                     tools::Long& rYOff, tools::Long& rWidth, FontEmphasisMark eEmphasis, tools::Long nHeight );
    SAL_DLLPRIVATE static FontEmphasisMark
                                ImplGetEmphasisMarkStyle( const vcl::Font& rFont );

    bool                        GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex,
                                                    int nLen, std::vector< tools::Rectangle >& rVector ) const;

    sal_Int32                   HasGlyphs( const vcl::Font& rFont, const OUString& rStr,
                                           sal_Int32 nIndex = 0, sal_Int32 nLen = -1 ) const;

    tools::Long                        GetMinKashida() const;

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
                                                   AddFontSubstituteFlags nFlags );
    static void                 RemoveFontsSubstitute();

    static vcl::Font            GetDefaultFont( DefaultFontType nType,
                                                LanguageType eLang,
                                                GetDefaultFontFlags nFlags,
                                                const OutputDevice* pOutDev = nullptr );

    SAL_DLLPRIVATE void         ImplInitFontList() const;
    SAL_DLLPRIVATE void         ImplUpdateFontData();

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
    SAL_DLLPRIVATE const LogicalFontInstance* GetFontInstance() const;
    SAL_DLLPRIVATE tools::Long GetEmphasisAscent() const { return mnEmphasisAscent; }
    SAL_DLLPRIVATE tools::Long GetEmphasisDescent() const { return mnEmphasisDescent; }

    SAL_DLLPRIVATE bool InitFont() const;
    virtual void                SetFontOrientation( LogicalFontInstance* const pFontInstance ) const;
    virtual tools::Long                GetFontExtLeading() const;

    virtual void ImplClearFontData(bool bNewFontLists);
    virtual void ImplRefreshFontData(bool bNewFontLists);
    void ReleaseFontCache();
    void ReleaseFontCollection();
    void SetFontCollectionFromSVData();
    void ResetNewFontCache();

    virtual bool ImplNewFont() const;

private:

    typedef void ( OutputDevice::* FontUpdateHandler_t )( bool );

    SAL_DLLPRIVATE static void  ImplUpdateFontDataForAllFrames( FontUpdateHandler_t pHdl, bool bNewFontLists );

    static
    SAL_DLLPRIVATE OUString     ImplGetEllipsisString( const OutputDevice& rTargetDevice, const OUString& rStr,
                                                       tools::Long nMaxWidth, DrawTextFlags nStyle, const vcl::ITextLayout& _rLayout );

    SAL_DLLPRIVATE void         ImplDrawEmphasisMark( tools::Long nBaseX, tools::Long nX, tools::Long nY, const tools::PolyPolygon& rPolyPoly, bool bPolyLine, const tools::Rectangle& rRect1, const tools::Rectangle& rRect2 );
    SAL_DLLPRIVATE void         ImplDrawEmphasisMarks( SalLayout& );
    ///@}


    /** @name Layout functions
     */
    ///@{

public:

    // tells whether this output device is RTL in an LTR UI or LTR in a RTL UI
    SAL_DLLPRIVATE bool         ImplIsAntiparallel() const ;
    SAL_DLLPRIVATE void         ReMirror( Point &rPoint ) const;
    SAL_DLLPRIVATE void         ReMirror( tools::Rectangle &rRect ) const;
    SAL_DLLPRIVATE void         ReMirror( vcl::Region &rRegion ) const;
    SAL_DLLPRIVATE bool         ImplIsRecordLayout() const;
    virtual bool                HasMirroredGraphics() const;
    std::unique_ptr<SalLayout>
                                ImplLayout( const OUString&, sal_Int32 nIndex, sal_Int32 nLen,
                                            const Point& rLogicPos = Point(0,0), tools::Long nLogicWidth=0,
                                            o3tl::span<const sal_Int32> pLogicDXArray={}, SalLayoutFlags flags = SalLayoutFlags::NONE,
                                            vcl::text::TextLayoutCache const* = nullptr,
                                            const SalLayoutGlyphs* pGlyphs = nullptr) const;
    SAL_DLLPRIVATE vcl::text::ImplLayoutArgs ImplPrepareLayoutArgs( OUString&, const sal_Int32 nIndex, const sal_Int32 nLen,
                                                         DeviceCoordinate nPixelWidth, const DeviceCoordinate* pPixelDXArray,
                                                         SalLayoutFlags flags = SalLayoutFlags::NONE,
                                                         vcl::text::TextLayoutCache const* = nullptr) const;
    SAL_DLLPRIVATE std::unique_ptr<SalLayout>
                                ImplGlyphFallbackLayout( std::unique_ptr<SalLayout>,
                                                         vcl::text::ImplLayoutArgs&,
                                                         const SalLayoutGlyphs* ) const;
    SAL_DLLPRIVATE std::unique_ptr<SalLayout>
                                getFallbackLayout(
                                    LogicalFontInstance* pLogicalFont, int nFallbackLevel,
                                    vcl::text::ImplLayoutArgs& rLayoutArgs, const SalLayoutGlyphs* ) const;

    /*
     These functions allow collecting information on how fonts are mapped when used, such as what
     replacements are used when a requested font is missing or which fonts are used as fallbacks
     when a font doesn't provide all necessary glyphs.
     After StartTrackingFontMappingUse() is called, VCL starts collecting font usage for all
     text layout calls, FinishTrackingFontMappingUse() will stop collecting and providing
     the collected information.
     Each item is a mapping from a requested font to a list of actually used fonts and the number
     of times this mapping was done.
    */
    struct FontMappingUseItem
    {
        OUString mOriginalFont;
        std::vector<OUString> mUsedFonts;
        int mCount;
    };
    typedef std::vector<FontMappingUseItem> FontMappingUseData;
    static void StartTrackingFontMappingUse();
    static FontMappingUseData FinishTrackingFontMappingUse();

    // Enabling/disabling RTL only makes sense for OutputDevices that use a mirroring SalGraphicsLayout
    virtual void                EnableRTL( bool bEnable = true);
    bool                        IsRTLEnabled() const { return mbEnableRTL; }

    bool                        GetTextIsRTL( const OUString&, sal_Int32 nIndex, sal_Int32 nLen ) const;

    ///@}


    /** @name Bitmap functions
     */
    ///@{

public:
    void                        DrawBitmap(
                                    const Point& rDestPt,
                                    const Bitmap& rBitmap );

    void                        DrawBitmap(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const Bitmap& rBitmap );

    void                        DrawBitmap(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const Point& rSrcPtPixel,
                                    const Size& rSrcSizePixel,
                                    const Bitmap& rBitmap);

    void                        DrawBitmap(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const Point& rSrcPtPixel,
                                    const Size& rSrcSizePixel,
                                    const Bitmap& rBitmap,
                                    MetaActionType nAction );

    void                        DrawBitmapEx(
                                    const Point& rDestPt,
                                    const BitmapEx& rBitmapEx );


    void                        DrawBitmapEx(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const BitmapEx& rBitmapEx );

    void                        DrawBitmapEx(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const Point& rSrcPtPixel,
                                    const Size& rSrcSizePixel,
                                    const BitmapEx& rBitmapEx);

    void                        DrawBitmapEx(
                                    const Point& rDestPt,
                                    const Size& rDestSize,
                                    const Point& rSrcPtPixel,
                                    const Size& rSrcSizePixel,
                                    const BitmapEx& rBitmapEx,
                                    MetaActionType nAction );

    /** @overload
        virtual void DrawImage(
                        const Point& rPos,
                        const Size& rSize,
                        const Image& rImage,
                        sal_uInt16 nStyle = 0)
     */
    void                        DrawImage(
                                    const Point& rPos,
                                    const Image& rImage,
                                    DrawImageFlags nStyle = DrawImageFlags::NONE );

    void                        DrawImage(
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

        @param fAlpha
        Optional additional alpha to use for drawing (0 to 1, 1 being no change).
    */
    void                        DrawTransformedBitmapEx(
                                    const basegfx::B2DHomMatrix& rTransformation,
                                    const BitmapEx& rBitmapEx,
                                    double fAlpha = 1.0);

    /** Return true if DrawTransformedBitmapEx() is implemented. It may still fail,
        e.g when fAlpha != 1.0 and that aspect is not implemented. This is a hint
        that calling it is worth a try
    */
    bool                        ImplementsFastDrawTransformedBitmap() const;

protected:

    virtual void                DrawDeviceBitmapEx(
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                    BitmapEx& rBitmapEx );

    virtual bool                CanSubsampleBitmap() const { return true; }

    /** Transform and draw a bitmap directly

     @param     aFullTransform      The B2DHomMatrix used for the transformation
     @param     rBitmapEx           Reference to the bitmap to be transformed and drawn

     @return true if it was able to draw the bitmap, false if not
     */
    virtual bool                DrawTransformBitmapExDirect(
                                    const basegfx::B2DHomMatrix& aFullTransform,
                                    const BitmapEx& rBitmapEx,
                                    double fAlpha = 1.0);

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
                                tools::Rectangle aDstRect, tools::Rectangle aBmpRect,
                                Size const & aOutSz, Point const & aOutPt);


    SAL_DLLPRIVATE void         BlendBitmap(
                                    const SalTwoRect&   rPosAry,
                                    const Bitmap&       rBmp );

    SAL_DLLPRIVATE Bitmap       BlendBitmap(
                                    Bitmap&             aBmp,
                                    BitmapReadAccess const * pP,
                                    BitmapReadAccess const * pA,
                                    const sal_Int32     nOffY,
                                    const sal_Int32     nDstHeight,
                                    const sal_Int32     nOffX,
                                    const sal_Int32     nDstWidth,
                                    const tools::Rectangle&    aBmpRect,
                                    const Size&         aOutSz,
                                    const bool          bHMirr,
                                    const bool          bVMirr,
                                    const sal_Int32*         pMapX,
                                    const sal_Int32*         pMapY );

    SAL_DLLPRIVATE Bitmap       BlendBitmapWithAlpha(
                                    Bitmap&             aBmp,
                                    BitmapReadAccess const *   pP,
                                    BitmapReadAccess const *   pA,
                                    const tools::Rectangle&    aDstRect,
                                    const sal_Int32     nOffY,
                                    const sal_Int32     nDstHeight,
                                    const sal_Int32     nOffX,
                                    const sal_Int32     nDstWidth,
                                    const sal_Int32*         pMapX,
                                    const sal_Int32*         pMapY );

    ///@}


    /** @name Transparency functions
     */
    ///@{

public:

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
                                    tools::Long nMaxBmpDPIX, tools::Long nMaxBmpDPIY,
                                    bool bReduceTransparency,
                                    bool bTransparencyAutoMode,
                                    bool bDownsampleBitmaps,
                                    const Color& rBackground = COL_TRANSPARENT );

    void                        DrawTransparent( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent );

    void                        DrawTransparent(
                                    const basegfx::B2DHomMatrix& rObjectTransform,
                                    const basegfx::B2DPolyPolygon& rB2DPolyPoly,
                                    double fTransparency);

    void                        DrawTransparent(
                                        const GDIMetaFile& rMtf, const Point& rPos, const Size& rSize,
                                        const Gradient& rTransparenceGradient );

protected:

    virtual void                EmulateDrawTransparent( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent );

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
                                          const Bitmap& rBitmap, const Color& rMaskColor);

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
    void                        SetMapMode( const MapMode& rNewMapMode );
    void                        SetRelativeMapMode( const MapMode& rNewMapMode );
    virtual void                SetMetafileMapMode(const MapMode& rNewMapMode, bool bIsRecord);
    const MapMode&              GetMapMode() const { return maMapMode; }

protected:
    virtual void ImplInitMapModeObjects();

public:
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
        default (that's MapUnit::MapPixel), then any pixel offset set is
        ignored also. This might be unintuitive for cases, but would
        have been far more fragile to implement. What's more, the
        reason why the pixel offset was introduced (avoiding rounding
        errors) does not apply for MapUnit::MapPixel, because one can always
        use the MapMode origin then.

        @param rOffset
        The offset in pixel
     */
    void                        SetPixelOffset( const Size& rOffset );

    /** Get the offset in pixel

        @see OutputDevice::SetPixelOffset for details

        @return the current offset in pixel
     */
    SAL_WARN_UNUSED_RESULT Size GetPixelOffset() const { return Size(mnOutOffOrigX, mnOutOffOrigY);}

    SAL_WARN_UNUSED_RESULT Point LogicToPixel(const Point& rLogicPt) const;
    SAL_WARN_UNUSED_RESULT Size  LogicToPixel(const Size& rLogicSize) const;
    SAL_WARN_UNUSED_RESULT tools::Rectangle LogicToPixel(const tools::Rectangle& rLogicRect) const;
    SAL_WARN_UNUSED_RESULT tools::Polygon LogicToPixel(const tools::Polygon& rLogicPoly) const;
    SAL_WARN_UNUSED_RESULT tools::PolyPolygon LogicToPixel(const tools::PolyPolygon& rLogicPolyPoly) const;
    SAL_WARN_UNUSED_RESULT basegfx::B2DPolyPolygon LogicToPixel(const basegfx::B2DPolyPolygon& rLogicPolyPoly) const;
    SAL_WARN_UNUSED_RESULT vcl::Region LogicToPixel(const vcl::Region& rLogicRegion)const;
    SAL_WARN_UNUSED_RESULT Point LogicToPixel(const Point& rLogicPt, const MapMode& rMapMode) const;
    SAL_WARN_UNUSED_RESULT Size LogicToPixel(const Size& rLogicSize, const MapMode& rMapMode) const;
    SAL_WARN_UNUSED_RESULT tools::Rectangle LogicToPixel(const tools::Rectangle& rLogicRect,
                                                         const MapMode& rMapMode) const;
    SAL_WARN_UNUSED_RESULT tools::Polygon LogicToPixel(const tools::Polygon& rLogicPoly,
                                                       const MapMode& rMapMode) const;
    SAL_WARN_UNUSED_RESULT basegfx::B2DPolyPolygon LogicToPixel(const basegfx::B2DPolyPolygon& rLogicPolyPoly,
                                                                const MapMode& rMapMode) const;

    SAL_WARN_UNUSED_RESULT Point PixelToLogic(const Point& rDevicePt) const;
    SAL_WARN_UNUSED_RESULT Size PixelToLogic(const Size& rDeviceSize) const;
    SAL_WARN_UNUSED_RESULT tools::Rectangle PixelToLogic(const tools::Rectangle& rDeviceRect) const;
    SAL_WARN_UNUSED_RESULT tools::Polygon PixelToLogic(const tools::Polygon& rDevicePoly) const;
    SAL_WARN_UNUSED_RESULT tools::PolyPolygon PixelToLogic(const tools::PolyPolygon& rDevicePolyPoly) const;
    SAL_WARN_UNUSED_RESULT basegfx::B2DPolyPolygon PixelToLogic(const basegfx::B2DPolyPolygon& rDevicePolyPoly) const;
    SAL_WARN_UNUSED_RESULT vcl::Region PixelToLogic(const vcl::Region& rDeviceRegion) const;
    SAL_WARN_UNUSED_RESULT Point PixelToLogic(const Point& rDevicePt, const MapMode& rMapMode) const;
    SAL_WARN_UNUSED_RESULT Size PixelToLogic(const Size& rDeviceSize, const MapMode& rMapMode) const;
    SAL_WARN_UNUSED_RESULT tools::Rectangle PixelToLogic(const tools::Rectangle& rDeviceRect,
                                                         const MapMode& rMapMode) const;
    SAL_WARN_UNUSED_RESULT tools::Polygon PixelToLogic(const tools::Polygon& rDevicePoly,
                                                       const MapMode& rMapMode) const;
    SAL_WARN_UNUSED_RESULT basegfx::B2DPolygon PixelToLogic(const basegfx::B2DPolygon& rDevicePoly,
                                                            const MapMode& rMapMode) const;
    SAL_WARN_UNUSED_RESULT basegfx::B2DPolyPolygon PixelToLogic(const basegfx::B2DPolyPolygon& rDevicePolyPoly,
                                                                const MapMode& rMapMode) const;

    SAL_WARN_UNUSED_RESULT Point LogicToLogic(const Point& rPtSource,
                                              const MapMode* pMapModeSource,
                                              const MapMode* pMapModeDest) const;
    SAL_WARN_UNUSED_RESULT Size LogicToLogic(const Size& rSzSource,
                                             const MapMode* pMapModeSource,
                                             const MapMode* pMapModeDest) const;
    SAL_WARN_UNUSED_RESULT tools::Rectangle LogicToLogic(const tools::Rectangle& rRectSource,
                                                         const MapMode* pMapModeSource,
                                                         const MapMode* pMapModeDest) const;
    SAL_WARN_UNUSED_RESULT static Point LogicToLogic(const Point& rPtSource,
                                                     const MapMode& rMapModeSource,
                                                     const MapMode& rMapModeDest);
    SAL_WARN_UNUSED_RESULT static Size LogicToLogic(const Size& rSzSource,
                                                    const MapMode& rMapModeSource,
                                                    const MapMode& rMapModeDest);
    SAL_WARN_UNUSED_RESULT static tools::Rectangle LogicToLogic(const tools::Rectangle& rRectSource,
                                                                const MapMode& rMapModeSource,
                                                                const MapMode& rMapModeDest);
    SAL_WARN_UNUSED_RESULT static tools::Long LogicToLogic(tools::Long nLongSource,
                                                           MapUnit eUnitSource,
                                                           MapUnit eUnitDest);

    SAL_WARN_UNUSED_RESULT static basegfx::B2DPolygon LogicToLogic(const basegfx::B2DPolygon& rPoly,
                                                                   const MapMode& rMapModeSource,
                                                                   const MapMode& rMapModeDest);

    // create a mapping transformation from rMapModeSource to rMapModeDest (the above methods
    // for B2DPoly/Polygons use this internally anyway to transform the B2DPolygon)
    SAL_WARN_UNUSED_RESULT static basegfx::B2DHomMatrix LogicToLogic(const MapMode& rMapModeSource,
                                                                     const MapMode& rMapModeDest);

    /** Convert a logical rectangle to a rectangle in physical device pixel units.

     @param         rLogicRect  Const reference to a rectangle in logical units

     @returns Rectangle based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE tools::Rectangle    ImplLogicToDevicePixel( const tools::Rectangle& rLogicRect ) const;

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
    SAL_DLLPRIVATE tools::Long         ImplLogicWidthToDevicePixel( tools::Long nWidth ) const;

    SAL_DLLPRIVATE DeviceCoordinate LogicWidthToDeviceCoordinate( tools::Long nWidth ) const;

    /** Convert a logical X coordinate to a device pixel's X coordinate.

     To get the device's X coordinate, it must calculate the mapping offset
     coordinate X position (if there is one - if not then it just adds
     the pseudo-window offset to the logical X coordinate), the X-DPI of
     the device and the mapping's X scaling factor.

     @param         nX          Logical X coordinate

     @returns Device's X pixel coordinate
     */
    SAL_DLLPRIVATE tools::Long         ImplLogicXToDevicePixel( tools::Long nX ) const;

    /** Convert a logical Y coordinate to a device pixel's Y coordinate.

     To get the device's Y coordinate, it must calculate the mapping offset
     coordinate Y position (if there is one - if not then it just adds
     the pseudo-window offset to the logical Y coordinate), the Y-DPI of
     the device and the mapping's Y scaling factor.

     @param         nY          Logical Y coordinate

     @returns Device's Y pixel coordinate
     */
    SAL_DLLPRIVATE tools::Long         ImplLogicYToDevicePixel( tools::Long nY ) const;

    /** Convert a logical height to a height in units of device pixels.

     To get the number of device pixels, it must calculate the Y-DPI of the device and
     the map scaling factor. If there is no mapping, then it just returns the
     height as nothing more needs to be done.

     @param         nHeight     Logical height

     @returns Height in units of device pixels.
     */
    SAL_DLLPRIVATE tools::Long         ImplLogicHeightToDevicePixel( tools::Long nHeight ) const;

    /** Convert device pixels to a width in logical units.

     To get the logical width, it must calculate the X-DPI of the device and the
     map scaling factor.

     @param         nWidth      Width in device pixels

     @returns Width in logical units.
     */
    SAL_DLLPRIVATE tools::Long         ImplDevicePixelToLogicWidth( tools::Long nWidth ) const;

    /** Convert device pixels to a height in logical units.

     To get the logical height, it must calculate the Y-DPI of the device and the
     map scaling factor.

     @param         nHeight     Height in device pixels

     @returns Height in logical units.
     */
    SAL_DLLPRIVATE tools::Long         ImplDevicePixelToLogicHeight( tools::Long nHeight ) const;

    /** Convert logical height to device pixels, with exact sub-pixel value.

     To get the \em exact pixel height, it must calculate the Y-DPI of the device and the
     map scaling factor.

     @param         fLogicHeight     Exact height in logical units.

     @returns Exact height in pixels - returns as a float to provide for subpixel value.
     */
    SAL_DLLPRIVATE float        ImplFloatLogicHeightToDevicePixel( float fLogicHeight ) const;

    /** Convert a logical size to the size on the physical device.

     @param         rLogicSize  Const reference to a size in logical units

     @returns Physical size on the device.
     */
    SAL_DLLPRIVATE Size         ImplLogicToDevicePixel( const Size& rLogicSize ) const;

    /** Convert a rectangle in physical pixel units to a rectangle in physical pixel units and coords.

     @param         rPixelRect  Const reference to rectangle in logical units and coords.

     @returns Rectangle based on logical coordinates and units.
     */
    SAL_DLLPRIVATE tools::Rectangle    ImplDevicePixelToLogic( const tools::Rectangle& rPixelRect ) const;

    /** Convert a logical polygon to a polygon in physical device pixel units.

     @param         rLogicPoly  Const reference to a polygon in logical units

     @returns Polygon based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE tools::Polygon ImplLogicToDevicePixel( const tools::Polygon& rLogicPoly ) const;

    /** Convert a logical B2DPolygon to a B2DPolygon in physical device pixel units.

     @param         rLogicSize  Const reference to a B2DPolygon in logical units

     @returns B2DPolyPolygon based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE ::basegfx::B2DPolygon ImplLogicToDevicePixel( const ::basegfx::B2DPolygon& rLogicPoly ) const;

    /** Convert a logical polypolygon to a polypolygon in physical device pixel units.

     @param         rLogicPolyPoly  Const reference to a polypolygon in logical units

     @returns Polypolygon based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE tools::PolyPolygon  ImplLogicToDevicePixel( const tools::PolyPolygon& rLogicPolyPoly ) const;

    /** Convert a line in logical units to a line in physical device pixel units.

     @param         rLineInfo   Const reference to a line in logical units

     @returns Line based on physical device pixel coordinates and units.
     */
    SAL_DLLPRIVATE LineInfo     ImplLogicToDevicePixel( const LineInfo& rLineInfo ) const;

    /** Convert a region in pixel units to a region in device pixel units and coords.

     @param         rRegion  Const reference to region.

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

    /** Determine if native widgets can be enabled
     */
    virtual bool                CanEnableNativeWidget() const { return false; }

    /** Query the platform layer for control support
     */
    bool                        IsNativeControlSupported( ControlType nType, ControlPart nPart ) const;

    /** Query the native control to determine if it was acted upon
     */
    bool                        HitTestNativeScrollbar(
                                    ControlPart nPart,
                                    const tools::Rectangle& rControlRegion,
                                    const Point& aPos,
                                    bool& rIsInside ) const;

    /** Request rendering of a particular control and/or part
     */
    bool                        DrawNativeControl(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const tools::Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    const OUString& aCaption,
                                    const Color& rBackgroundColor = COL_AUTO );

    /** Query the native control's actual drawing region (including adornment)
     */
    bool                        GetNativeControlRegion(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const tools::Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    tools::Rectangle &rNativeBoundingRegion,
                                    tools::Rectangle &rNativeContentRegion ) const;
    ///@}

    /** @name EPS functions
     */
    ///@{

public:

    /** @returns boolean value to see if EPS could be painted directly.
        Theoretically, handing over a matrix would be needed to handle
        painting rotated EPS files (e.g. contained in Metafiles). This
        would then need to be supported for Mac and PS printers, but
        that's too much for now, wrote \#i107046# for this */
    bool                        DrawEPS(
                                    const Point& rPt, const Size& rSz,
                                    const GfxLink& rGfxLink, GDIMetaFile* pSubst = nullptr );
    ///@}

public:
    virtual css::awt::DeviceInfo GetDeviceInfo() const;

    /** Get the vcl::Window that this OutputDevice belongs to, if any */
    virtual vcl::Window* GetOwnerWindow() const { return nullptr; }

protected:
    css::awt::DeviceInfo GetCommonDeviceInfo(Size const& aDevSize) const;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
