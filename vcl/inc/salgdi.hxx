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

#include <vcl/outdev.hxx>

#include "impfontmetricdata.hxx"
#include "salgdiimpl.hxx"
#include "sallayout.hxx"
#include "SalGradient.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include "WidgetDrawInterface.hxx"

#include <config_cairo_canvas.h>

#include <map>
#include <vector>

class SalBitmap;
class FontAttributes;
namespace vcl::font {
    class FontSelectPattern;
    class PhysicalFontFace;
    class PhysicalFontCollection;
}
class SalLayout;
namespace tools { class Rectangle; }
class FontSubsetInfo;
class OutputDevice;
class FreetypeFont;
struct SystemGraphicsData;

namespace basegfx {
    class B2DVector;
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace vcl
{
class AbstractTrueTypeFont;
class FileDefinitionWidgetDraw;
typedef struct TTGlobalFontInfo_ TTGlobalFontInfo;
}

typedef sal_Unicode sal_Ucs; // TODO: use sal_UCS4 instead of sal_Unicode
typedef std::map< sal_Ucs, sal_uInt32 >   Ucs2UIntMap;

// note: if you add any new methods to class SalGraphics using coordinates
//       make sure they have a corresponding protected pure virtual method
//       which has to be implemented by the platform dependent part.
//       Add a method that performs coordinate mirroring if required, (see
//       existing methods as sample) and then calls the equivalent pure method.

// note: all positions are in pixel and relative to
// the top/left-position of the virtual output area

class VCL_PLUGIN_PUBLIC SalGraphics : protected vcl::WidgetDrawInterface
{
public:
    SalGraphics();
    ~SalGraphics() COVERITY_NOEXCEPT_FALSE override;

    virtual SalGraphicsImpl*    GetImpl() const = 0;

    void                        setAntiAlias(bool bNew)
    {
        m_bAntiAlias = bNew;

        // Temporary store in both
        if (GetImpl())
            GetImpl()->setAntiAlias(bNew);
    }

    bool                        getAntiAlias() const
    {
        return m_bAntiAlias;
    }

    // public SalGraphics methods, the interface to the independent vcl part

    // get device resolution
    virtual void                GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) = 0;

    // get the depth of the device
    virtual sal_uInt16          GetBitCount() const = 0;

    // get the width of the device
    virtual tools::Long                GetGraphicsWidth() const = 0;

    // set the clip region to empty
    virtual void                ResetClipRegion() = 0;

    // set the line color to transparent (= don't draw lines)

    virtual void                SetLineColor() = 0;

    // set the line color to a specific color
    virtual void                SetLineColor( Color nColor ) = 0;

    // set the fill color to transparent (= don't fill)
    virtual void                SetFillColor() = 0;

    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void                SetFillColor( Color nColor ) = 0;

    // enable/disable XOR drawing
    virtual void                SetXORMode( bool bSet, bool bInvertOnly ) = 0;

    // set line color for raster operations
    virtual void                SetROPLineColor( SalROPColor nROPColor ) = 0;

    // set fill color for raster operations
    virtual void                SetROPFillColor( SalROPColor nROPColor ) = 0;

    // set the text color to a specific color
    virtual void                SetTextColor( Color nColor ) = 0;

    // set the font
    virtual void                SetFont(LogicalFontInstance*, int nFallbackLevel) = 0;

    // release the fonts
    void                        ReleaseFonts() { SetFont( nullptr, 0 ); }

    // get the current font's metrics
    virtual void                GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) = 0;

    // get the repertoire of the current font
    virtual FontCharMapRef      GetFontCharMap() const = 0;

    // get the layout capabilities of the current font
    virtual bool                GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const = 0;

    // graphics must fill supplied font list
    virtual void                GetDevFontList( vcl::font::PhysicalFontCollection* ) = 0;

    // graphics must drop any cached font info
    virtual void                ClearDevFontCache() = 0;

    virtual bool                AddTempDevFont(
                                    vcl::font::PhysicalFontCollection*,
                                    const OUString& rFileURL,
                                    const OUString& rFontName ) = 0;

    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns true if creation of subset was successful
    // parameters: rToFile: contains an osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the corresponding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    virtual bool                CreateFontSubset(
                                    const OUString& rToFile,
                                    const vcl::font::PhysicalFontFace* pFont,
                                    const sal_GlyphId* pGlyphIDs,
                                    const sal_uInt8* pEncoding,
                                    sal_Int32* pWidths,
                                    int nGlyphs,
                                    FontSubsetInfo& rInfo ) = 0;

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void*         GetEmbedFontData(const vcl::font::PhysicalFontFace* pFont, tools::Long* pDataLen) = 0;

    // free the font data again
    virtual void                FreeEmbedFontData( const void* pData, tools::Long nDataLen ) = 0;

    // get the same widths as in CreateFontSubset
    // in case of an embeddable font also fill the mapping
    // between unicode and glyph id
    // leave widths vector and mapping untouched in case of failure
    virtual void                GetGlyphWidths(
                                    const vcl::font::PhysicalFontFace* pFont,
                                    bool bVertical,
                                    std::vector< sal_Int32 >& rWidths,
                                    Ucs2UIntMap& rUnicodeEnc ) = 0;

    virtual std::unique_ptr<GenericSalLayout>
                                GetTextLayout(int nFallbackLevel) = 0;
    virtual void                DrawTextLayout( const GenericSalLayout& ) = 0;

    virtual bool                supportsOperation( OutDevSupportType ) const = 0;

    // mirroring specifics
    SalLayoutFlags              GetLayout() const { return m_nLayout; }
    void                        SetLayout( SalLayoutFlags aLayout ) { m_nLayout = aLayout;}

    void                        mirror( tools::Long& nX, const OutputDevice& rOutDev ) const;
    // only called mirror2 to avoid ambiguity
    [[nodiscard]] tools::Long   mirror2( tools::Long nX, const OutputDevice& rOutDev ) const;
    void                        mirror( tools::Long& nX, tools::Long nWidth, const OutputDevice& rOutDev, bool bBack = false ) const;
    bool                        mirror( sal_uInt32 nPoints, const Point *pPtAry, Point *pPtAry2, const OutputDevice& rOutDev ) const;
    void                        mirror( tools::Rectangle& rRect, const OutputDevice&, bool bBack = false ) const;
    void                        mirror( vcl::Region& rRgn, const OutputDevice& rOutDev ) const;
    void                        mirror( ImplControlValue&, const OutputDevice& ) const;
    basegfx::B2DPolyPolygon     mirror( const basegfx::B2DPolyPolygon& i_rPoly, const OutputDevice& rOutDev ) const;
    const basegfx::B2DHomMatrix& getMirror( const OutputDevice& rOutDev ) const;

    // non virtual methods; these do possible coordinate mirroring and
    // then delegate to protected virtual methods
    bool                        SetClipRegion( const vcl::Region&, const OutputDevice& rOutDev );

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    void                        DrawPixel( tools::Long nX, tools::Long nY, const OutputDevice& rOutDev );
    void                        DrawPixel( tools::Long nX, tools::Long nY, Color nColor, const OutputDevice& rOutDev );

    void                        DrawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2, const OutputDevice& rOutDev );

    void                        DrawRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, const OutputDevice& rOutDev );

    void                        DrawPolyLine( sal_uInt32 nPoints, Point const * pPtAry, const OutputDevice& rOutDev );

    void                        DrawPolygon( sal_uInt32 nPoints, const Point* pPtAry, const OutputDevice& rOutDev );

    void                        DrawPolyPolygon(
                                    sal_uInt32 nPoly,
                                    const sal_uInt32* pPoints,
                                    const Point** pPtAry,
                                    const OutputDevice& rOutDev );

    bool                        DrawPolyPolygon(
                                    const basegfx::B2DHomMatrix& rObjectToDevice,
                                    const basegfx::B2DPolyPolygon &i_rPolyPolygon,
                                    double i_fTransparency,
                                    const OutputDevice& i_rOutDev);

    bool                        DrawPolyLine(
                                    const basegfx::B2DHomMatrix& rObjectToDevice,
                                    const basegfx::B2DPolygon& i_rPolygon,
                                    double i_fTransparency,
                                    double i_fLineWidth,
                                    const std::vector< double >* i_pStroke, // MM01
                                    basegfx::B2DLineJoin i_eLineJoin,
                                    css::drawing::LineCap i_eLineCap,
                                    double i_fMiterMinimumAngle,
                                    bool bPixelSnapHairline,
                                    const OutputDevice& i_rOutDev);

    bool                        DrawPolyLineBezier(
                                    sal_uInt32 nPoints,
                                    const Point* pPtAry,
                                    const PolyFlags* pFlgAry,
                                    const OutputDevice& rOutDev );

    bool                        DrawPolygonBezier(
                                    sal_uInt32 nPoints,
                                    const Point* pPtAry,
                                    const PolyFlags* pFlgAry,
                                    const OutputDevice& rOutDev );

    bool                        DrawPolyPolygonBezier(
                                    sal_uInt32 nPoly,
                                    const sal_uInt32* pPoints,
                                    const Point* const* pPtAry,
                                    const PolyFlags* const* pFlgAry,
                                    const OutputDevice& rOutDev );

    bool                        DrawGradient(
                                    const tools::PolyPolygon& rPolyPoly,
                                    const Gradient& rGradient,
                                    const OutputDevice& rOutDev);

    // CopyArea --> No RasterOp, but ClipRegion
    void                        CopyArea(
                                    tools::Long nDestX, tools::Long nDestY,
                                    tools::Long nSrcX, tools::Long nSrcY,
                                    tools::Long nSrcWidth, tools::Long nSrcHeight,
                                    const OutputDevice& rOutDev );

    // CopyBits --> RasterOp and ClipRegion
    // CopyBits() CopyBits on same Graphics
    void                        CopyBits(
                                    const SalTwoRect& rPosAry,
                                    const OutputDevice& rOutDev);

    // CopyBits --> RasterOp and ClipRegion
    // CopyBits() CopyBits on different Graphics
    void                        CopyBits(
                                    const SalTwoRect& rPosAry,
                                    SalGraphics& rSrcGraphics,
                                    const OutputDevice& rOutDev,
                                    const OutputDevice& rSrcOutDev );


    void                        DrawBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const OutputDevice& rOutDev );

    void                        DrawBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap,
                                    const OutputDevice& rOutDev );

    void                        DrawMask(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    Color nMaskColor,
                                    const OutputDevice& rOutDev );

    std::shared_ptr<SalBitmap>  GetBitmap(
                                    tools::Long nX, tools::Long nY,
                                    tools::Long nWidth, tools::Long nHeight,
                                    const OutputDevice& rOutDev );

    Color                       GetPixel(
                                    tools::Long nX, tools::Long nY,
                                    const OutputDevice& rOutDev );

    // invert --> ClipRegion (only Windows)
    void                        Invert(
                                    tools::Long nX, tools::Long nY,
                                    tools::Long nWidth, tools::Long nHeight,
                                    SalInvert nFlags,
                                    const OutputDevice& rOutDev );

    void                        Invert(
                                    sal_uInt32 nPoints,
                                    const Point* pPtAry,
                                    SalInvert nFlags,
                                    const OutputDevice& rOutDev );

    bool                        DrawEPS(
                                    tools::Long nX, tools::Long nY,
                                    tools::Long nWidth, tools::Long nHeight,
                                    void* pPtr,
                                    sal_uInt32 nSize,
                                    const OutputDevice& rOutDev );

    //  native widget rendering functions

    /**
     * @see WidgetDrawInterface::isNativeControlSupported
     */
    inline bool IsNativeControlSupported(ControlType, ControlPart);

    /**
     * @see WidgetDrawInterface::hitTestNativeControl
     */
    bool                        HitTestNativeScrollbar(
                                    ControlPart nPart,
                                    const tools::Rectangle& rControlRegion,
                                    const Point& aPos,
                                    bool& rIsInside,
                                    const OutputDevice& rOutDev);

    /**
     * @see WidgetDrawInterface::drawNativeControl
     */
    bool                        DrawNativeControl(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const tools::Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    const OUString& aCaption,
                                    const OutputDevice& rOutDev,
                                    const Color& rBackgroundColor = COL_AUTO );

    /**
     * @see WidgetDrawInterface::getNativeControlRegion
     */
    bool                        GetNativeControlRegion(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const tools::Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    tools::Rectangle &rNativeBoundingRegion,
                                    tools::Rectangle &rNativeContentRegion,
                                    const OutputDevice& rOutDev );

    /**
     * @see WidgetDrawInterface::updateSettings
     */
    inline bool UpdateSettings(AllSettings&);

    bool                        BlendBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const OutputDevice& rOutDev );

    bool                        BlendAlphaBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalSrcBitmap,
                                    const SalBitmap& rSalMaskBitmap,
                                    const SalBitmap& rSalAlphaBitmap,
                                    const OutputDevice& rOutDev );

    bool                        DrawAlphaBitmap(
                                    const SalTwoRect&,
                                    const SalBitmap& rSourceBitmap,
                                    const SalBitmap& rAlphaBitmap,
                                    const OutputDevice& rOutDev );

    bool                        DrawTransformedBitmap(
                                    const basegfx::B2DPoint& rNull,
                                    const basegfx::B2DPoint& rX,
                                    const basegfx::B2DPoint& rY,
                                    const SalBitmap& rSourceBitmap,
                                    const SalBitmap* pAlphaBitmap,
                                    double fAlpha,
                                    const OutputDevice& rOutDev );

    bool                        HasFastDrawTransformedBitmap() const;

    bool                        DrawAlphaRect(
                                    tools::Long nX, tools::Long nY,
                                    tools::Long nWidth, tools::Long nHeight,
                                    sal_uInt8 nTransparency,
                                    const OutputDevice& rOutDev );

    virtual OUString getRenderBackendName() const;

    virtual SystemGraphicsData  GetGraphicsData() const = 0;

#if ENABLE_CAIRO_CANVAS

    /// Check whether cairo will work
    virtual bool                SupportsCairo() const = 0;
    /// Create Surface from given cairo surface
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const = 0;
    /// Create surface with given dimensions
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const = 0;
    /// Create Surface for given bitmap data
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const = 0;
    virtual css::uno::Any       GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const = 0;

#endif // ENABLE_CAIRO_CANVAS

protected:

    friend class vcl::FileDefinitionWidgetDraw;

    virtual bool                setClipRegion( const vcl::Region& ) = 0;

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void                drawPixel( tools::Long nX, tools::Long nY ) = 0;
    virtual void                drawPixel( tools::Long nX, tools::Long nY, Color nColor ) = 0;

    virtual void                drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 ) = 0;

    virtual void                drawRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) = 0;

    virtual void                drawPolyLine( sal_uInt32 nPoints, const Point* pPtAry ) = 0;

    virtual void                drawPolygon( sal_uInt32 nPoints, const Point* pPtAry ) = 0;

    virtual void                drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, const Point** pPtAry ) = 0;

    virtual bool                drawPolyPolygon(
                                    const basegfx::B2DHomMatrix& rObjectToDevice,
                                    const basegfx::B2DPolyPolygon&,
                                    double fTransparency) = 0;

    virtual bool                drawPolyLine(
                                    const basegfx::B2DHomMatrix& rObjectToDevice,
                                    const basegfx::B2DPolygon&,
                                    double fTransparency,
                                    double fLineWidth,
                                    const std::vector< double >* pStroke, // MM01
                                    basegfx::B2DLineJoin,
                                    css::drawing::LineCap,
                                    double fMiterMinimumAngle,
                                    bool bPixelSnapHairline) = 0;

    virtual bool                drawPolyLineBezier(
                                    sal_uInt32 nPoints,
                                    const Point* pPtAry,
                                    const PolyFlags* pFlgAry ) = 0;

    virtual bool                drawPolygonBezier(
                                    sal_uInt32 nPoints,
                                    const Point* pPtAry,
                                    const PolyFlags* pFlgAry ) = 0;

    virtual bool                drawPolyPolygonBezier(
                                    sal_uInt32 nPoly,
                                    const sal_uInt32* pPoints,
                                    const Point* const* pPtAry,
                                    const PolyFlags* const* pFlgAry ) = 0;

    virtual bool                drawGradient(
                                    const tools::PolyPolygon& rPolyPoly,
                                    const Gradient& rGradient ) = 0;

    virtual bool implDrawGradient(basegfx::B2DPolyPolygon const & /*rPolyPolygon*/,
                                  SalGradient const & /*rGradient*/)
    {
        return false;
    }

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void                copyArea(
                                    tools::Long nDestX, tools::Long nDestY,
                                    tools::Long nSrcX, tools::Long nSrcY,
                                    tools::Long nSrcWidth, tools::Long nSrcHeight,
                                    bool bWindowInvalidate ) = 0;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void                copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) = 0;

    virtual void                drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) = 0;

    virtual void                drawBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rMaskBitmap ) = 0;

    virtual void                drawMask(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    Color nMaskColor ) = 0;

    virtual std::shared_ptr<SalBitmap> getBitmap( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) = 0;

    virtual Color               getPixel( tools::Long nX, tools::Long nY ) = 0;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void                invert(
                                    tools::Long nX, tools::Long nY,
                                    tools::Long nWidth, tools::Long nHeight,
                                    SalInvert nFlags) = 0;

    virtual void                invert( sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags ) = 0;

    virtual bool                drawEPS(
                                    tools::Long nX, tools::Long nY,
                                    tools::Long nWidth, tools::Long nHeight,
                                    void* pPtr,
                                    sal_uInt32 nSize ) = 0;

    /** Blend the bitmap with the current buffer */
    virtual bool                blendBitmap(
                                    const SalTwoRect&,
                                    const SalBitmap& rBitmap ) = 0;

    /** Draw the bitmap by blending using the mask and alpha channel */
    virtual bool                blendAlphaBitmap(
                                    const SalTwoRect&,
                                    const SalBitmap& rSrcBitmap,
                                    const SalBitmap& rMaskBitmap,
                                    const SalBitmap& rAlphaBitmap ) = 0;

    /** Render bitmap with alpha channel

        @param rSourceBitmap
        Source bitmap to blit

        @param rAlphaBitmap
        Alpha channel to use for blitting

        @return true, if the operation succeeded, and false
        otherwise. In this case, clients should try to emulate alpha
        compositing themselves
     */
    virtual bool                drawAlphaBitmap(
                                    const SalTwoRect&,
                                    const SalBitmap& rSourceBitmap,
                                    const SalBitmap& rAlphaBitmap ) = 0;

    /** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system

      @param fAlpha additional alpha (0 to 1) to apply while drawing
    */
    virtual bool                drawTransformedBitmap(
                                    const basegfx::B2DPoint& rNull,
                                    const basegfx::B2DPoint& rX,
                                    const basegfx::B2DPoint& rY,
                                    const SalBitmap& rSourceBitmap,
                                    const SalBitmap* pAlphaBitmap,
                                    double fAlpha) = 0;

    /// Returns true if the drawTransformedBitmap() call is fast, and so it should
    /// be used directly without trying to optimize some calls e.g. by calling drawBitmap()
    /// instead (which is faster for most VCL backends).
    /// See also tdf#138068 or the comment at https://gerrit.libreoffice.org/c/core/+/126480.
    virtual bool hasFastDrawTransformedBitmap() const = 0;

    /** Render solid rectangle with given transparency
     *
     * @param nX             Top left coordinate of rectangle
     * @param nY             Bottom right coordinate of rectangle
     * @param nWidth         Width of rectangle
     * @param nHeight        Height of rectangle
     * @param nTransparency  Transparency value (0-255) to use. 0 blits and opaque, 255 a
     *                       fully transparent rectangle
     * @returns true if successfully drawn, false if not able to draw rectangle
     */
    virtual bool                drawAlphaRect(
                                    tools::Long nX, tools::Long nY,
                                    tools::Long nWidth, tools::Long nHeight,
                                    sal_uInt8 nTransparency ) = 0;

private:
    SalLayoutFlags              m_nLayout; //< 0: mirroring off, 1: mirror x-axis

    // for buffering the Mirror-Matrix, see ::getMirror
    basegfx::B2DHomMatrix       m_aLastMirror;
    tools::Long                 m_aLastMirrorW;
    tools::Long                 m_nLastMirrorDeviceLTRButBiDiRtlTranslate;
    bool                        m_bLastMirrorDeviceLTRButBiDiRtlSet;

protected:
    /// flags which hold the SetAntialiasing() value from OutputDevice
    bool                        m_bAntiAlias : 1;

    inline tools::Long GetDeviceWidth(const OutputDevice& rOutDev) const;

    /**
     * Handle damage done by drawing with a widget draw override
     *
     * If a m_pWidgetDraw is set and successfully draws using drawNativeControl,
     * this function is called to handle the damage done to the graphics buffer.
     *
     * @param rDamagedRegion the region damaged by drawNativeControl.
     **/
    virtual inline void handleDamage(const tools::Rectangle& rDamagedRegion);

    // native controls
    bool initWidgetDrawBackends(bool bForce = false);

    std::unique_ptr<vcl::WidgetDrawInterface> m_pWidgetDraw;
    vcl::WidgetDrawInterface* forWidget() { return m_pWidgetDraw ? m_pWidgetDraw.get() : this; }

    static void GetGlyphWidths(const vcl::AbstractTrueTypeFont& rTTF,
                               const vcl::font::PhysicalFontFace& rFontFace, bool bVertical,
                               std::vector<sal_Int32>& rWidths, Ucs2UIntMap& rUnicodeEnc);

    static bool CreateTTFfontSubset(vcl::AbstractTrueTypeFont& aTTF, const OString& rSysPath,
                                    const bool bVertical, const sal_GlyphId* pGlyphIds,
                                    const sal_uInt8* pEncoding, sal_Int32* pGlyphWidths,
                                    int nGlyphCount);

    static bool CreateCFFfontSubset(const unsigned char* pFontBytes, int nByteLength,
                                    const OString& rSysPath, const sal_GlyphId* pGlyphIds,
                                    const sal_uInt8* pEncoding, sal_Int32* pGlyphWidths,
                                    int nGlyphCount, FontSubsetInfo& rInfo);

    static void FillFontSubsetInfo(const vcl::TTGlobalFontInfo& rTTInfo, const OUString& pPSName,
                                   FontSubsetInfo& rInfo);
};

bool SalGraphics::IsNativeControlSupported(ControlType eType, ControlPart ePart)
{
    return forWidget()->isNativeControlSupported(eType, ePart);
}

bool SalGraphics::UpdateSettings(AllSettings& rSettings)
{
    return forWidget()->updateSettings(rSettings);
}

void SalGraphics::handleDamage(const tools::Rectangle&) {}


class VCL_DLLPUBLIC SalGraphicsAutoDelegateToImpl : public SalGraphics
{
public:
    sal_uInt16 GetBitCount() const override
    {
        return GetImpl()->GetBitCount();
    }

    tools::Long GetGraphicsWidth() const override
    {
        return GetImpl()->GetGraphicsWidth();
    }

    void ResetClipRegion() override
    {
        GetImpl()->ResetClipRegion();
    }

    bool setClipRegion( const vcl::Region& i_rClip ) override
    {
        return GetImpl()->setClipRegion(i_rClip);
    }

    void SetLineColor() override
    {
        GetImpl()->SetLineColor();
    }

    void SetLineColor( Color nColor ) override
    {
        GetImpl()->SetLineColor(nColor);
    }

    void SetFillColor() override
    {
        GetImpl()->SetFillColor();
    }

    void SetFillColor( Color nColor ) override
    {
        GetImpl()->SetFillColor (nColor);
    }

    void SetROPLineColor(SalROPColor aColor) override
    {
        GetImpl()->SetROPLineColor(aColor);
    }

    void SetROPFillColor( SalROPColor aColor) override
    {
        GetImpl()->SetROPFillColor(aColor);
    }

    void SetXORMode(bool bSet, bool bInvertOnly) override
    {
        GetImpl()->SetXORMode(bSet, bInvertOnly);
    }

    void drawPixel( tools::Long nX, tools::Long nY ) override
    {
        GetImpl()->drawPixel(nX, nY);
    }

    void drawPixel( tools::Long nX, tools::Long nY, Color nColor ) override
    {
        GetImpl()->drawPixel(nX, nY, nColor);
    }

    void drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 ) override
    {
        GetImpl()->drawLine(nX1, nY1, nX2, nY2);
    }

    void drawRect( tools::Long nX, tools::Long nY, tools::Long nDX, tools::Long nDY ) override
    {
        GetImpl()->drawRect(nX, nY, nDX, nDY);
    }

    void drawPolyLine( sal_uInt32 nPoints, const Point *pPtAry ) override
    {
        GetImpl()->drawPolyLine(nPoints, pPtAry);
    }

    void drawPolygon( sal_uInt32 nPoints, const Point* pPtAry ) override
    {
        GetImpl()->drawPolygon(nPoints, pPtAry);
    }

    void drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints, const Point** pPtAry) override
    {
        GetImpl()->drawPolyPolygon (nPoly, pPoints, pPtAry);
    }

    bool drawPolyPolygon(
        const basegfx::B2DHomMatrix& rObjectToDevice,
        const basegfx::B2DPolyPolygon& rPolyPolygon,
        double fTransparency) override
    {
        return GetImpl()->drawPolyPolygon(rObjectToDevice, rPolyPolygon, fTransparency);
    }

    bool drawPolyLine(
        const basegfx::B2DHomMatrix& rObjectToDevice,
        const basegfx::B2DPolygon& rPolygon,
        double fTransparency,
        double fLineWidth,
        const std::vector< double >* pStroke,
        basegfx::B2DLineJoin eJoin,
        css::drawing::LineCap eLineCap,
        double fMiterMinimumAngle,
        bool bPixelSnapHairline) override
    {
        return GetImpl()->drawPolyLine(rObjectToDevice, rPolygon, fTransparency, fLineWidth, pStroke, eJoin, eLineCap, fMiterMinimumAngle, bPixelSnapHairline);
    }

    bool drawPolyLineBezier( sal_uInt32 nPoints, const Point* pPtAry, const PolyFlags* pFlgAry ) override
    {
        return GetImpl()->drawPolyLineBezier(nPoints, pPtAry, pFlgAry);
    }

    bool drawPolygonBezier( sal_uInt32 nPoints, const Point* pPtAry, const PolyFlags* pFlgAry ) override
    {
        return GetImpl()->drawPolygonBezier(nPoints, pPtAry, pFlgAry);
    }

    bool drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                 const sal_uInt32* pPoints,
                                                 const Point* const* pPtAry,
                                                 const PolyFlags* const* pFlgAry) override
    {
        return GetImpl()->drawPolyPolygonBezier(nPoly, pPoints, pPtAry, pFlgAry);
    }

    void invert(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                                        SalInvert nFlags) override
    {
       GetImpl()->invert(nX, nY, nWidth, nHeight, nFlags);
    }

    void invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags) override
    {
       GetImpl()->invert(nPoints, pPtAry, nFlags);
    }

    bool drawEPS(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                   tools::Long nHeight, void* pPtr, sal_uInt32 nSize) override
    {
        return GetImpl()->drawEPS(nX, nY, nWidth, nHeight, pPtr, nSize);
    }

    void copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics) override
    {
        GetImpl()->copyBits(rPosAry, pSrcGraphics);
    }

    void copyArea (tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX,
                                    tools::Long nSrcY, tools::Long nSrcWidth, tools::Long nSrcHeight,
                                    bool bWindowInvalidate) override
    {
        GetImpl()->copyArea(nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, bWindowInvalidate);
    }

    void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap) override
    {
        GetImpl()->drawBitmap(rPosAry, rSalBitmap);
    }

    void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, const SalBitmap& rMaskBitmap) override
    {
        GetImpl()->drawBitmap(rPosAry, rSalBitmap, rMaskBitmap);
    }

    void drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, Color nMaskColor) override
    {
        GetImpl()->drawMask(rPosAry, rSalBitmap, nMaskColor);
    }

    std::shared_ptr<SalBitmap> getBitmap(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight) override
    {
        return GetImpl()->getBitmap(nX, nY, nWidth, nHeight);
    }

    Color getPixel(tools::Long nX, tools::Long nY) override
    {
        return GetImpl()->getPixel(nX, nY);
    }

    bool blendBitmap(const SalTwoRect& rPosAry, const SalBitmap& rBitmap) override
    {
        return GetImpl()->blendBitmap(rPosAry, rBitmap);
    }

    bool blendAlphaBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap,
                                          const SalBitmap& rMaskBitmap, const SalBitmap& rAlphaBitmap) override
    {
        return GetImpl()->blendAlphaBitmap(rPosAry, rSourceBitmap, rMaskBitmap, rAlphaBitmap);
    }

    bool drawAlphaBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap,
                                         const SalBitmap& rAlphaBitmap) override
    {
        return GetImpl()->drawAlphaBitmap(rPosAry, rSourceBitmap, rAlphaBitmap);
    }

    bool drawTransformedBitmap(const basegfx::B2DPoint& rNull,
                                                 const basegfx::B2DPoint& rX,
                                                 const basegfx::B2DPoint& rY,
                                                 const SalBitmap& rSourceBitmap,
                                                 const SalBitmap* pAlphaBitmap, double fAlpha) override
    {
        return GetImpl()->drawTransformedBitmap(rNull, rX, rY, rSourceBitmap, pAlphaBitmap, fAlpha);
    }

    bool hasFastDrawTransformedBitmap() const override
    {
        return GetImpl()->hasFastDrawTransformedBitmap();
    }

    bool drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                       tools::Long nHeight, sal_uInt8 nTransparency) override
    {
        return GetImpl()->drawAlphaRect(nX, nY, nWidth, nHeight, nTransparency);
    }

    bool drawGradient(const tools::PolyPolygon& rPolygon, const Gradient& rGradient) override
    {
        return GetImpl()->drawGradient(rPolygon, rGradient);
    }

    bool implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                                            SalGradient const& rGradient) override
    {
        return GetImpl()->implDrawGradient(rPolyPolygon, rGradient);
    }

    bool supportsOperation(OutDevSupportType eType) const override
    {
        return GetImpl()->supportsOperation(eType);
    }

    OUString getRenderBackendName() const override
    {
        return GetImpl()->getRenderBackendName();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
