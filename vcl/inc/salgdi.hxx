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

#ifndef INCLUDED_VCL_INC_SALGDI_HXX
#define INCLUDED_VCL_INC_SALGDI_HXX

#include <tools/solar.h>
#include <osl/thread.hxx>

#include <vcl/dllapi.h>
#include <vcl/salgtype.hxx>
#include <vcl/outdev.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/metric.hxx>

#include "salgdiimpl.hxx"
#include "salglyphid.hxx"
#include "sallayout.hxx"

#include <map>
#include <set>

class PhysicalFontCollection;
class SalBitmap;
class FontSelectPattern;
class ImplFontMetricData;
class PhysicalFontFace;
class SalLayout;
class ImplLayoutArgs;
class Rectangle;
class FontSubsetInfo;
class OpenGLContext;
class OutputDevice;
class ServerFontLayout;
struct SystemGraphicsData;
struct SystemFontData;

namespace basegfx {
    class B2DVector;
    class B2DPolygon;
    class B2DPolyPolygon;
}

// - SalGraphics-Codes -

#define SAL_SETFONT_USEDRAWTEXTARRAY        ((sal_uInt16)0x0004)
#define SAL_SETFONT_BADFONT                 ((sal_uInt16)0x1000)

#define SAL_COPYAREA_WINDOWINVALIDATE       ((sal_uInt16)0x0001)

typedef sal_Unicode sal_Ucs; // TODO: use sal_UCS4 instead of sal_Unicode
typedef std::map< sal_Ucs, sal_Int32 >    Ucs2SIntMap;
typedef std::map< sal_Ucs, sal_uInt32 >   Ucs2UIntMap;
typedef std::map< sal_Ucs, OString > Ucs2OStrMap;
typedef std::vector< sal_Int32 > Int32Vector;

// note: if you add any new methods to class SalGraphics using coordinates
//       make sure they have a corresponding protected pure virtual method
//       which has to be implemented by the platform dependent part.
//       Add a method that performs coordinate mirroring if required, (see
//       existing methods as sample) and then calls the equivalent pure method.

// note: all positions are in pixel and relative to
// the top/left-position of the virtual output area

class VCL_PLUGIN_PUBLIC SalGraphics
{
public:
                                SalGraphics();
    virtual                     ~SalGraphics();

    virtual SalGraphicsImpl*    GetImpl() const = 0;

    /// Check that our mpImpl is OpenGL and return the context, otherwise NULL.
    rtl::Reference<OpenGLContext> GetOpenGLContext() const;

    void                        setAntiAliasB2DDraw(bool bNew) { m_bAntiAliasB2DDraw = bNew; }
    bool                        getAntiAliasB2DDraw() const { return m_bAntiAliasB2DDraw; }

    // public SalGraphics methods, the interface to the independent vcl part

    // get device resolution
    virtual void                GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) = 0;

    // get the depth of the device
    virtual sal_uInt16          GetBitCount() const = 0;

    // get the width of the device
    virtual long                GetGraphicsWidth() const = 0;

    // set the clip region to empty
    virtual void                ResetClipRegion() = 0;

    // set the line color to transparent (= don't draw lines)

    virtual void                SetLineColor() = 0;

    // set the line color to a specific color
    virtual void                SetLineColor( SalColor nSalColor ) = 0;

    // set the fill color to transparent (= don't fill)
    virtual void                SetFillColor() = 0;

    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void                SetFillColor( SalColor nSalColor ) = 0;

    // enable/disable XOR drawing
    virtual void                SetXORMode( bool bSet, bool bInvertOnly ) = 0;

    // set line color for raster operations
    virtual void                SetROPLineColor( SalROPColor nROPColor ) = 0;

    // set fill color for raster operations
    virtual void                SetROPFillColor( SalROPColor nROPColor ) = 0;

    // set the text color to a specific color
    virtual void                SetTextColor( SalColor nSalColor ) = 0;

    // set the font
    virtual sal_uInt16          SetFont( FontSelectPattern*, int nFallbackLevel ) = 0;

    // release the fonts
    void                        ReleaseFonts() { SetFont( NULL, 0 ); }

    // get the current font's metrics
    virtual void                GetFontMetric( ImplFontMetricData*, int nFallbackLevel = 0 ) = 0;

    // get the repertoire of the current font
    virtual const FontCharMapPtr GetFontCharMap() const = 0;

    // get the layout capabilities of the current font
    virtual bool                GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const = 0;

    // graphics must fill supplied font list
    virtual void                GetDevFontList( PhysicalFontCollection* ) = 0;

    // graphics must drop any cached font info
    virtual void                ClearDevFontCache() = 0;

    virtual bool                AddTempDevFont(
                                    PhysicalFontCollection*,
                                    const OUString& rFileURL,
                                    const OUString& rFontName ) = 0;

    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns true if creation of subset was successful
    // parameters: rToFile: contains a osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the correspoding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    virtual bool                CreateFontSubset(
                                    const OUString& rToFile,
                                    const PhysicalFontFace* pFont,
                                    const sal_GlyphId* pGlyphIDs,
                                    const sal_uInt8* pEncoding,
                                    sal_Int32* pWidths,
                                    int nGlyphs,
                                    FontSubsetInfo& rInfo ) = 0;

    // GetFontEncodingVector: a method to get the encoding map Unicode
    // to font encoded character; this is only used for type1 fonts and
    // may return NULL in case of unknown encoding vector
    // if ppNonEncoded is set and non encoded characters (that is type1
    // glyphs with only a name) exist it is set to the corresponding
    // map for non encoded glyphs; the encoding vector contains -1
    // as encoding for these cases
    virtual const Ucs2SIntMap*  GetFontEncodingVector(
                                    const PhysicalFontFace*,
                                    const Ucs2OStrMap** ppNonEncoded,
                                    std::set<sal_Unicode> const** ppPriority) = 0;

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pUnicodes: contains the Unicodes assigned to code points 0 to 255
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //             nLen: the number of elements in each of pWidths and pUnicodes
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void*         GetEmbedFontData(
                                    const PhysicalFontFace* pFont,
                                    const sal_Ucs* pUnicodes,
                                    sal_Int32* pWidths,
                                    size_t nLen,
                                    FontSubsetInfo& rInfo,
                                    long* pDataLen ) = 0;

    // free the font data again
    virtual void                FreeEmbedFontData( const void* pData, long nDataLen ) = 0;

    // get the same widths as in CreateFontSubset and GetEmbedFontData
    // in case of an embeddable font also fill the mapping
    // between unicode and glyph id
    // leave widths vector and mapping untouched in case of failure
    virtual void                GetGlyphWidths(
                                    const PhysicalFontFace* pFont,
                                    bool bVertical,
                                    Int32Vector& rWidths,
                                    Ucs2UIntMap& rUnicodeEnc ) = 0;

    virtual bool                GetGlyphBoundRect( sal_GlyphId, Rectangle& ) = 0;
    virtual bool                GetGlyphOutline( sal_GlyphId, basegfx::B2DPolyPolygon& ) = 0;

    virtual SalLayout*          GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) = 0;
    virtual void                DrawServerFontLayout( const ServerFontLayout& ) = 0;

    virtual bool                supportsOperation( OutDevSupportType ) const = 0;

    // mirroring specifics
    SalLayoutFlags              GetLayout() { return m_nLayout; }
    void                        SetLayout( SalLayoutFlags aLayout ) { m_nLayout = aLayout;}

    void                        mirror( long& nX, const OutputDevice *pOutDev, bool bBack = false ) const;
    void                        mirror( long& nX, long& nWidth, const OutputDevice *pOutDev, bool bBack = false ) const;
    bool                        mirror( sal_uInt32 nPoints, const SalPoint *pPtAry, SalPoint *pPtAry2, const OutputDevice *pOutDev, bool bBack = false ) const;
    void                        mirror( Rectangle& rRect, const OutputDevice*, bool bBack = false ) const;
    void                        mirror( vcl::Region& rRgn, const OutputDevice *pOutDev, bool bBack = false ) const;
    void                        mirror( ImplControlValue&, const OutputDevice*, bool bBack = false ) const;
    basegfx::B2DPoint           mirror( const basegfx::B2DPoint& i_rPoint, const OutputDevice *pOutDev, bool bBack = false ) const;
    basegfx::B2DPolygon         mirror( const basegfx::B2DPolygon& i_rPoly, const OutputDevice *pOutDev, bool bBack = false ) const;
    basegfx::B2DPolyPolygon     mirror( const basegfx::B2DPolyPolygon& i_rPoly, const OutputDevice *pOutDev, bool bBack = false ) const;

    // non virtual methods; these do possible coordinate mirroring and
    // then delegate to protected virtual methods
    bool                        SetClipRegion( const vcl::Region&, const OutputDevice *pOutDev );

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    void                        DrawPixel( long nX, long nY, const OutputDevice *pOutDev );
    void                        DrawPixel( long nX, long nY, SalColor nSalColor, const OutputDevice *pOutDev );

    void                        DrawLine( long nX1, long nY1, long nX2, long nY2, const OutputDevice *pOutDev );

    void                        DrawRect( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev );

    void                        DrawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev );

    void                        DrawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev );

    void                        DrawPolyPolygon(
                                    sal_uInt32 nPoly,
                                    const sal_uInt32* pPoints,
                                    PCONSTSALPOINT* pPtAry,
                                    const OutputDevice *pOutDev );

    bool                        DrawPolyPolygon(
                                    const basegfx::B2DPolyPolygon &i_rPolyPolygon,
                                    double i_fTransparency,
                                    const OutputDevice *i_pOutDev);

    bool                        DrawPolyLine(
                                    const basegfx::B2DPolygon& i_rPolygon,
                                    double i_fTransparency,
                                    const basegfx::B2DVector& i_rLineWidth,
                                    basegfx::B2DLineJoin i_eLineJoin,
                                    com::sun::star::drawing::LineCap i_eLineCap,
                                    const OutputDevice* i_pOutDev);

    bool                        DrawPolyLineBezier(
                                    sal_uInt32 nPoints,
                                    const SalPoint* pPtAry,
                                    const sal_uInt8* pFlgAry,
                                    const OutputDevice *pOutDev );

    bool                        DrawPolygonBezier(
                                    sal_uInt32 nPoints,
                                    const SalPoint* pPtAry,
                                    const sal_uInt8* pFlgAry,
                                    const OutputDevice *pOutDev );

    bool                        DrawPolyPolygonBezier(
                                    sal_uInt32 nPoly,
                                    const sal_uInt32* pPoints,
                                    const SalPoint* const* pPtAry,
                                    const sal_uInt8* const* pFlgAry,
                                    const OutputDevice *pOutDev );

    bool                        DrawGradient(
                                    const tools::PolyPolygon& rPolyPoly,
                                    const Gradient& rGradient,
                                    OutputDevice* );


    // CopyArea --> No RasterOp, but ClipRegion
    void                        CopyArea(
                                    long nDestX, long nDestY,
                                    long nSrcX, long nSrcY,
                                    long nSrcWidth, long nSrcHeight,
                                    sal_uInt16 nFlags,
                                    const OutputDevice *pOutDev );

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    void                        CopyBits(
                                    const SalTwoRect& rPosAry,
                                    SalGraphics* pSrcGraphics,
                                    const OutputDevice *pOutDev,
                                    const OutputDevice *pSrcOutDev );

    void                        DrawBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const OutputDevice *pOutDev );

    void                        DrawBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap,
                                    const OutputDevice *pOutDev );

    void                        DrawMask(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nMaskColor,
                                    const OutputDevice *pOutDev );

    SalBitmap*                  GetBitmap(
                                    long nX, long nY,
                                    long nWidth, long nHeight,
                                    const OutputDevice *pOutDev );

    SalColor                    GetPixel(
                                    long nX, long nY,
                                    const OutputDevice *pOutDev );

    // invert --> ClipRegion (only Windows)
    void                        Invert(
                                    long nX, long nY,
                                    long nWidth, long nHeight,
                                    SalInvert nFlags,
                                    const OutputDevice *pOutDev );

    void                        Invert(
                                    sal_uInt32 nPoints,
                                    const SalPoint* pPtAry,
                                    SalInvert nFlags,
                                    const OutputDevice *pOutDev );

    bool                        DrawEPS(
                                    long nX, long nY,
                                    long nWidth, long nHeight,
                                    void* pPtr,
                                    sal_uLong nSize,
                                    const OutputDevice *pOutDev );

    //  native widget rendering functions

    // Query the platform layer for control support
    virtual bool                IsNativeControlSupported( ControlType nType, ControlPart nPart );

    // Query the native control to determine if it was acted upon
    bool                        HitTestNativeControl(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    const Point& aPos,
                                    bool& rIsInside,
                                    const OutputDevice *pOutDev );

    // Request rendering of a particular control and/or part
    bool                        DrawNativeControl(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    const OUString& aCaption,
                                    const OutputDevice *pOutDev );

    // Query the native control's actual drawing region (including adornment)
    bool                        GetNativeControlRegion(
                                    ControlType nType,
                                    ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    const OUString& aCaption,
                                    Rectangle &rNativeBoundingRegion,
                                    Rectangle &rNativeContentRegion,
                                    const OutputDevice *pOutDev );

    bool                        BlendBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const OutputDevice *pOutDev );

    bool                        BlendAlphaBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalSrcBitmap,
                                    const SalBitmap& rSalMaskBitmap,
                                    const SalBitmap& rSalAlphaBitmap,
                                    const OutputDevice *pOutDev );

    bool                        DrawAlphaBitmap(
                                    const SalTwoRect&,
                                    const SalBitmap& rSourceBitmap,
                                    const SalBitmap& rAlphaBitmap,
                                    const OutputDevice *pOutDev );

    bool                        DrawTransformedBitmap(
                                    const basegfx::B2DPoint& rNull,
                                    const basegfx::B2DPoint& rX,
                                    const basegfx::B2DPoint& rY,
                                    const SalBitmap& rSourceBitmap,
                                    const SalBitmap* pAlphaBitmap,
                                    const OutputDevice* pOutDev );

    bool                        DrawAlphaRect(
                                    long nX, long nY,
                                    long nWidth, long nHeight,
                                    sal_uInt8 nTransparency,
                                    const OutputDevice *pOutDev );

    virtual OpenGLContext      *BeginPaint() { return NULL; }

    virtual SystemGraphicsData  GetGraphicsData() const = 0;

    /// Check whether cairo will work
    virtual bool                SupportsCairo() const = 0;
    /// Create Surface from given cairo surface
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const = 0;
    /// Create surface with given dimensions
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const = 0;
    /// Create Surface for given bitmap data
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const = 0;
    virtual css::uno::Any       GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const ::basegfx::B2ISize& rSize) const = 0;

    virtual SystemFontData      GetSysFontData( int nFallbacklevel ) const = 0;

protected:
    virtual bool                setClipRegion( const vcl::Region& ) = 0;

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void                drawPixel( long nX, long nY ) = 0;
    virtual void                drawPixel( long nX, long nY, SalColor nSalColor ) = 0;

    virtual void                drawLine( long nX1, long nY1, long nX2, long nY2 ) = 0;

    virtual void                drawRect( long nX, long nY, long nWidth, long nHeight ) = 0;

    virtual void                drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) = 0;

    virtual void                drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) = 0;

    virtual void                drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry ) = 0;
    virtual bool                drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency ) = 0;

    virtual bool                drawPolyLine(
                                    const ::basegfx::B2DPolygon&,
                                    double fTransparency,
                                    const ::basegfx::B2DVector& rLineWidths,
                                    basegfx::B2DLineJoin,
                                    com::sun::star::drawing::LineCap) = 0;

    virtual bool                drawPolyLineBezier(
                                    sal_uInt32 nPoints,
                                    const SalPoint* pPtAry,
                                    const sal_uInt8* pFlgAry ) = 0;

    virtual bool                drawPolygonBezier(
                                    sal_uInt32 nPoints,
                                    const SalPoint* pPtAry,
                                    const sal_uInt8* pFlgAry ) = 0;

    virtual bool                drawPolyPolygonBezier(
                                    sal_uInt32 nPoly,
                                    const sal_uInt32* pPoints,
                                    const SalPoint* const* pPtAry,
                                    const sal_uInt8* const* pFlgAry ) = 0;


    virtual bool                drawGradient(
                                    const tools::PolyPolygon& rPolyPoly,
                                    const Gradient& rGradient ) = 0;

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void                copyArea(
                                    long nDestX, long nDestY,
                                    long nSrcX, long nSrcY,
                                    long nSrcWidth, long nSrcHeight,
                                    sal_uInt16 nFlags ) = 0;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void                copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) = 0;

    virtual void                drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) = 0;

    virtual void                drawBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nTransparentColor ) = 0;

    virtual void                drawBitmap(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rMaskBitmap ) = 0;

    virtual void                drawMask(
                                    const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nMaskColor ) = 0;

    virtual SalBitmap*          getBitmap( long nX, long nY, long nWidth, long nHeight ) = 0;

    virtual SalColor            getPixel( long nX, long nY ) = 0;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void                invert(
                                    long nX, long nY,
                                    long nWidth, long nHeight,
                                    SalInvert nFlags) = 0;

    virtual void                invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) = 0;

    virtual bool                drawEPS(
                                    long nX, long nY,
                                    long nWidth, long nHeight,
                                    void* pPtr,
                                    sal_uLong nSize ) = 0;

    // native widget rendering methods that require mirroring
    virtual bool                hitTestNativeControl(
                                    ControlType nType, ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    const Point& aPos,
                                    bool& rIsInside );

    virtual bool                drawNativeControl(
                                    ControlType nType, ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    const OUString& aCaption );

    virtual bool                getNativeControlRegion(
                                    ControlType nType, ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    const OUString& aCaption,
                                    Rectangle &rNativeBoundingRegion,
                                    Rectangle &rNativeContentRegion );

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

    /** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system */
    virtual bool                drawTransformedBitmap(
                                    const basegfx::B2DPoint& rNull,
                                    const basegfx::B2DPoint& rX,
                                    const basegfx::B2DPoint& rY,
                                    const SalBitmap& rSourceBitmap,
                                    const SalBitmap* pAlphaBitmap) = 0;

    /** Render solid rectangle with given transparency

        @param nTransparency
        Transparency value (0-255) to use. 0 blits and opaque, 255 a
        fully transparent rectangle
     */
    virtual bool                drawAlphaRect(
                                    long nX, long nY,
                                    long nWidth, long nHeight,
                                    sal_uInt8 nTransparency ) = 0;

private:
    SalLayoutFlags              m_nLayout; //< 0: mirroring off, 1: mirror x-axis

protected:
    /// flags which hold the SetAntialiasing() value from OutputDevice
    bool                        m_bAntiAliasB2DDraw : 1;

};

#endif // INCLUDED_VCL_INC_SALGDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
