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

    void setTextRenderModeForResolutionIndependentLayout(bool bNew)
    {
        m_bTextRenderModeForResolutionIndependentLayout = bNew;
    }

    bool getTextRenderModeForResolutionIndependentLayoutEnabled() const
    {
        return m_bTextRenderModeForResolutionIndependentLayout;
    }

    // public SalGraphics methods, the interface to the independent vcl part

    // get device resolution
    virtual void                GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) = 0;

    // get the depth of the device
    sal_uInt16 GetBitCount() const
    {
        return GetImpl()->GetBitCount();
    }

    // get the width of the device
    tools::Long GetGraphicsWidth() const
    {
        return GetImpl()->GetGraphicsWidth();
    }

    // set the clip region to empty
    void ResetClipRegion()
    {
        GetImpl()->ResetClipRegion();
    }

    // set the line color to transparent (= don't draw lines)
    void SetLineColor()
    {
        GetImpl()->SetLineColor();
    }

    // set the line color to a specific color
    void SetLineColor(Color nColor)
    {
        GetImpl()->SetLineColor(nColor);
    }

    // set the fill color to transparent (= don't fill)
    void SetFillColor()
    {
        GetImpl()->SetFillColor();
    }

    // set the fill color to a specific color, shapes will be
    // filled accordingly
    void SetFillColor(Color nColor)
    {
        GetImpl()->SetFillColor(nColor);
    }

    // enable/disable XOR drawing
    void SetXORMode(bool bSet, bool bInvertOnly)
    {
        GetImpl()->SetXORMode(bSet, bInvertOnly);
    }

    // set line color for raster operations
    void SetROPLineColor(SalROPColor nROPColor)
    {
        GetImpl()->SetROPLineColor(nROPColor);
    }

    // set fill color for raster operations
    void SetROPFillColor(SalROPColor nROPColor)
    {
        GetImpl()->SetROPFillColor(nROPColor);
    }

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

    bool supportsOperation(OutDevSupportType eType) const
    {
        return GetImpl()->supportsOperation(eType);
    }

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

    bool HasFastDrawTransformedBitmap() const
    {
        return GetImpl()->hasFastDrawTransformedBitmap();
    }

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
    bool                        m_bTextRenderModeForResolutionIndependentLayout : 1;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
