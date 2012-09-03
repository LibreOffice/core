/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SV_SALGDI_HXX
#define _SV_SALGDI_HXX

#include "tools/solar.h"
#include "vcl/dllapi.h"
#include "vcl/salgtype.hxx"
#include "osl/thread.hxx"
#include "vcl/outdev.hxx"
#include "vcl/salnativewidgets.hxx"
#include "sallayout.hxx"

#include <map>

namespace rtl
{
    class OUString;
}

class ImplDevFontList;
class SalBitmap;
class FontSelectPattern;
class ImplFontMetricData;
struct ImplKernPairData;
class PhysicalFontFace;
class ImplFontCharMap;
class SalLayout;
class ImplLayoutArgs;
class Rectangle;
class FontSubsetInfo;
class OutputDevice;
class ServerFontLayout;
struct SystemGraphicsData;
struct SystemFontData;

namespace basegfx {
    class B2DVector;
    class B2DPolygon;
    class B2DPolyPolygon;
}

// ---------------------
// - SalGraphics-Codes -
// ---------------------

#define SAL_SETFONT_REMOVEANDMATCHNEW       ((sal_uInt16)0x0001)
#define SAL_SETFONT_USEDRAWTEXT             ((sal_uInt16)0x0002)
#define SAL_SETFONT_USEDRAWTEXTARRAY        ((sal_uInt16)0x0004)
#define SAL_SETFONT_UNICODE                 ((sal_uInt16)0x0008)
#define SAL_SETFONT_BADFONT                 ((sal_uInt16)0x1000)

#define SAL_COPYAREA_WINDOWINVALIDATE       ((sal_uInt16)0x0001)

// -------------------
// - common typedefs -
// -------------------

typedef sal_Unicode sal_Ucs; // TODO: use sal_UCS4 instead of sal_Unicode
typedef std::map< sal_Ucs, sal_Int32 >    Ucs2SIntMap;
typedef std::map< sal_Ucs, sal_uInt32 >   Ucs2UIntMap;
typedef std::map< sal_Ucs, rtl::OString > Ucs2OStrMap;
typedef std::vector< sal_Int32 > Int32Vector;

// ---------------
// - SalGraphics -
// ---------------

// note: if you add any new methods to class SalGraphics using coordinates
//       make sure they have a corresponding protected pure virtual method
//       which has to be implemented by the platform dependent part.
//       Add a method that performs coordinate mirroring if required, (see
//       existing methods as sample) and then calls the equivalent pure method.

// note: all positions are in pixel and relative to
// the top/left-position of the virtual output area

class VCL_PLUGIN_PUBLIC SalGraphics
{
    int                     m_nLayout; // 0: mirroring off, 1: mirror x-axis

protected:
    // flags which hold the SetAntialiasing() value from OutputDevice
    bool                    m_bAntiAliasB2DDraw;

public:
    // get/set AA
    void setAntiAliasB2DDraw(bool bNew) { m_bAntiAliasB2DDraw = bNew; }
    bool getAntiAliasB2DDraw() const { return m_bAntiAliasB2DDraw; }

    SalGraphics();
    virtual ~SalGraphics();

protected:
    virtual bool        setClipRegion( const Region& ) = 0;
    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void        drawPixel( long nX, long nY ) = 0;
    virtual void        drawPixel( long nX, long nY, SalColor nSalColor ) = 0;
    virtual void        drawLine( long nX1, long nY1, long nX2, long nY2 ) = 0;
    virtual void        drawRect( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual void        drawPolyLine( sal_uLong nPoints, const SalPoint* pPtAry ) = 0;
    virtual void        drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry ) = 0;
    virtual void        drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry ) = 0;
    virtual bool        drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency ) = 0;
    virtual bool        drawPolyLine( const ::basegfx::B2DPolygon&, double fTransparency, const ::basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin ) = 0;
    virtual sal_Bool    drawPolyLineBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry ) = 0;
    virtual sal_Bool    drawPolygonBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry ) = 0;
    virtual sal_Bool    drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints, const SalPoint* const* pPtAry, const sal_uInt8* const* pFlgAry ) = 0;

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void        copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                  long nSrcHeight, sal_uInt16 nFlags ) = 0;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void        copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics ) = 0;
    virtual void        drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap ) = 0;
    virtual void        drawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nTransparentColor ) = 0;
    virtual void        drawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rMaskBitmap ) = 0;
    virtual void        drawMask( const SalTwoRect* pPosAry,
                                  const SalBitmap& rSalBitmap,
                                  SalColor nMaskColor ) = 0;

    virtual SalBitmap*  getBitmap( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual SalColor    getPixel( long nX, long nY ) = 0;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void        invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags) = 0;
    virtual void        invert( sal_uLong nPoints, const SalPoint* pPtAry, SalInvert nFlags ) = 0;

    virtual sal_Bool        drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize ) = 0;

    // native widget rendering methods that require mirroring
    virtual sal_Bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                              const Point& aPos, sal_Bool& rIsInside );
    virtual sal_Bool        drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const rtl::OUString& aCaption );
    virtual sal_Bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const rtl::OUString& aCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion );

    /** Render bitmap with alpha channel

        @param rSourceBitmap
        Source bitmap to blit

        @param rAlphaBitmap
        Alpha channel to use for blitting

        @return true, if the operation succeeded, and false
        otherwise. In this case, clients should try to emulate alpha
        compositing themselves
     */
    virtual bool        drawAlphaBitmap( const SalTwoRect&,
                                         const SalBitmap& rSourceBitmap,
                                         const SalBitmap& rAlphaBitmap ) = 0;
    /** Render solid rectangle with given transparency

        @param nTransparency
        Transparency value (0-255) to use. 0 blits and opaque, 255 a
        fully transparent rectangle
     */
    virtual bool        drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency ) = 0;

public:
    // public SalGraphics methods, the interface to the independent vcl part

    // get device resolution
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) = 0;
    // get the depth of the device
    virtual sal_uInt16          GetBitCount() const = 0;
    // get the width of the device
    virtual long            GetGraphicsWidth() const = 0;

    // set the clip region to empty
    virtual void            ResetClipRegion() = 0;

    // set the line color to transparent (= don't draw lines)
    virtual void            SetLineColor() = 0;
    // set the line color to a specific color
    virtual void            SetLineColor( SalColor nSalColor ) = 0;
    // set the fill color to transparent (= don't fill)
    virtual void            SetFillColor() = 0;
    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void            SetFillColor( SalColor nSalColor ) = 0;
    // enable/disable XOR drawing
    virtual void            SetXORMode( bool bSet, bool bInvertOnly ) = 0;
    // set line color for raster operations
    virtual void            SetROPLineColor( SalROPColor nROPColor ) = 0;
    // set fill color for raster operations
    virtual void            SetROPFillColor( SalROPColor nROPColor ) = 0;
    // set the text color to a specific color
    virtual void            SetTextColor( SalColor nSalColor ) = 0;
    // set the font
    virtual sal_uInt16         SetFont( FontSelectPattern*, int nFallbackLevel ) = 0;
    // release the fonts
    void                   ReleaseFonts() { SetFont( NULL, 0 ); }
    // get the current font's metrics
    virtual void            GetFontMetric( ImplFontMetricData*, int nFallbackLevel = 0 ) = 0;

    // get kernign pairs of the current font
    // return only PairCount if (pKernPairs == NULL)
    virtual sal_uLong           GetKernPairs( sal_uLong nMaxPairCount, ImplKernPairData* ) = 0;
    // get the repertoire of the current font
    virtual const ImplFontCharMap* GetImplFontCharMap() const = 0;
    // get the layout capabilities of the current font
    virtual bool GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const = 0;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( ImplDevFontList* ) = 0;
    // graphics must drop any cached font info
    virtual void ClearDevFontCache() = 0;
    // graphics should call ImplAddDevFontSubstitute on supplied
    // OutputDevice for all its device specific preferred font substitutions
    virtual void            GetDevFontSubstList( OutputDevice* ) = 0;
    virtual bool            AddTempDevFont( ImplDevFontList*, const rtl::OUString& rFileURL, const rtl::OUString& rFontName ) = 0;
    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns sal_True if creation of subset was successfull
    // parameters: rToFile: contains a osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the correspoding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    virtual sal_Bool            CreateFontSubset( const rtl::OUString& rToFile,
                                              const PhysicalFontFace* pFont,
                                              sal_Int32* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo // out parameter
                                              ) = 0;

    // GetFontEncodingVector: a method to get the encoding map Unicode
    // to font encoded character; this is only used for type1 fonts and
    // may return NULL in case of unknown encoding vector
    // if ppNonEncoded is set and non encoded characters (that is type1
    // glyphs with only a name) exist it is set to the corresponding
    // map for non encoded glyphs; the encoding vector contains -1
    // as encoding for these cases
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded ) = 0;

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pUnicodes: contains the Unicodes assigned to
    //             code points 0 to 255; must contain at least 256 members
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void* GetEmbedFontData( const PhysicalFontFace* pFont,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen ) = 0;
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen ) = 0;

    // get the same widths as in CreateFontSubset and GetEmbedFontData
    // in case of an embeddable font also fill the mapping
    // between unicode and glyph id
    // leave widths vector and mapping untouched in case of failure
    virtual void            GetGlyphWidths( const PhysicalFontFace* pFont,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) = 0;

    virtual sal_Bool                    GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& ) = 0;
    virtual sal_Bool                    GetGlyphOutline( sal_GlyphId nIndex, basegfx::B2DPolyPolygon& ) = 0;

    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) = 0;
    virtual void                     DrawServerFontLayout( const ServerFontLayout& ) = 0;
    /** Filter text from DrawText commands in a device specific manner
        <p>
        This function allows a device (or rather the corresponding SalGraphics
        implementation) to prevent text portions from being drawn. This currently
        is used only for filtering out the fax number in a document that is printed
        to one of psprint's specialized "fax" printers.
        </p>

        @param rOrigText
        The original text

        @param rNewText
        A String that will be filled with the adjusted version

        @param nIndex
        The index inside <code>rOrigText</code> that marks the first draw character

        @param rLen
        in: length of text beginning at <code>nIndex</code> to be drawn
        out: length of <code>rNewText</code> containing the substituted text

        @param rCutStart
        out: index at which the cutout portion of <code>rOrigText</code> begins

        @param rCutStop
        out: index at which the cutout portion of <code>rOrigText</code> ends

        @returns
        true: a substitution has taken place and rNewText rLen, rCutStart and rCutStop have been filled accordingly
        false: no substitution has taken place, rNewText, rLen, rCutStart, rCutStop remain unchanged
     */
    virtual bool            filterText( const rtl::OUString& rOrigText, rtl::OUString& rNewText, xub_StrLen nIndex, xub_StrLen& rLen, xub_StrLen& rCutStart, xub_StrLen& rCutStop );

    virtual bool            supportsOperation( OutDevSupportType ) const = 0;

    // mirroring specifica
    int                     GetLayout() { return m_nLayout; }
    void                    SetLayout( int aLayout ) { m_nLayout = aLayout;}

    void                    mirror( long& nX, const OutputDevice *pOutDev, bool bBack = false ) const;
    void                    mirror( long& nX, long& nWidth, const OutputDevice *pOutDev, bool bBack = false ) const;
    sal_Bool                    mirror( sal_uInt32 nPoints, const SalPoint *pPtAry, SalPoint *pPtAry2, const OutputDevice *pOutDev, bool bBack = false ) const;
    void                    mirror( Rectangle& rRect, const OutputDevice*, bool bBack = false ) const;
    void                    mirror( Region& rRgn, const OutputDevice *pOutDev, bool bBack = false ) const;
    void                    mirror( ImplControlValue&, const OutputDevice*, bool bBack = false ) const;
    basegfx::B2DPoint       mirror( const basegfx::B2DPoint& i_rPoint, const OutputDevice *pOutDev, bool bBack = false ) const;
    basegfx::B2DPolygon     mirror( const basegfx::B2DPolygon& i_rPoly, const OutputDevice *pOutDev, bool bBack = false ) const;
    basegfx::B2DPolyPolygon mirror( const basegfx::B2DPolyPolygon& i_rPoly, const OutputDevice *pOutDev, bool bBack = false ) const;

    // non virtual methods; these do possible coordinate mirroring and
    // then delegate to protected virtual methods
    bool                    SetClipRegion( const Region&, const OutputDevice *pOutDev );

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    void                    DrawPixel( long nX, long nY, const OutputDevice *pOutDev );
    void                    DrawPixel( long nX, long nY, SalColor nSalColor, const OutputDevice *pOutDev );
    void                    DrawLine( long nX1, long nY1, long nX2, long nY2, const OutputDevice *pOutDev );
    void                    DrawRect( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev );
    void                    DrawPolyLine( sal_uLong nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev );
    void                    DrawPolygon( sal_uLong nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev );
    void                    DrawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry,
                                             const OutputDevice *pOutDev );
    bool                    DrawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency, const OutputDevice* );
    bool                    DrawPolyLine( const basegfx::B2DPolygon&, double fTransparency, const basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin, const OutputDevice* );
    sal_Bool                DrawPolyLineBezier( sal_uLong nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry,
                                                const OutputDevice *pOutDev );
    sal_Bool                DrawPolygonBezier( sal_uLong nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry,
                                               const OutputDevice *pOutDev );
    sal_Bool                DrawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const sal_uInt8* const* pFlgAry,
                                                   const OutputDevice *pOutDev );

    // CopyArea --> No RasterOp, but ClipRegion
    void                    CopyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      sal_uInt16 nFlags,
                                      const OutputDevice *pOutDev );

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    void                    CopyBits( const SalTwoRect* pPosAry,
                                      SalGraphics* pSrcGraphics,
                                      const OutputDevice *pOutDev,
                                      const OutputDevice *pSrcOutDev );
    void                    DrawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const OutputDevice *pOutDev );
    void                    DrawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap,
                                        const OutputDevice *pOutDev );

    void                    DrawMask( const SalTwoRect* pPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor,
                                      const OutputDevice *pOutDev );

    SalBitmap*              GetBitmap( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev );
    SalColor                GetPixel( long nX, long nY, const OutputDevice *pOutDev );

    // invert --> ClipRegion (only Windows)
    void                    Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags, const OutputDevice *pOutDev );
    void                    Invert( sal_uLong nPoints, const SalPoint* pPtAry, SalInvert nFlags, const OutputDevice *pOutDev );

    sal_Bool                    DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize, const OutputDevice *pOutDev );

    //-------------------------------------
    //  Native Widget Rendering functions
    //-------------------------------------

    // Query the platform layer for control support
    virtual sal_Bool IsNativeControlSupported( ControlType nType, ControlPart nPart );

    // Query the native control to determine if it was acted upon
    sal_Bool HitTestNativeControl( ControlType nType,
                                      ControlPart nPart,
                                      const Rectangle& rControlRegion,
                                      const Point& aPos,
                                      sal_Bool& rIsInside,
                                      const OutputDevice *pOutDev );

    // Request rendering of a particular control and/or part
    sal_Bool DrawNativeControl( ControlType nType,
                                    ControlPart nPart,
                                    const Rectangle& rControlRegion,
                                    ControlState nState,
                                    const ImplControlValue& aValue,
                                    const rtl::OUString& aCaption,
                                    const OutputDevice *pOutDev );

    // Query the native control's actual drawing region (including adornment)
    sal_Bool GetNativeControlRegion( ControlType nType,
                                         ControlPart nPart,
                                         const Rectangle& rControlRegion,
                                         ControlState nState,
                                         const ImplControlValue& aValue,
                                         const rtl::OUString& aCaption,
                                         Rectangle &rNativeBoundingRegion,
                                         Rectangle &rNativeContentRegion,
                                         const OutputDevice *pOutDev );

    bool DrawAlphaBitmap( const SalTwoRect&,
                          const SalBitmap& rSourceBitmap,
                          const SalBitmap& rAlphaBitmap,
                          const OutputDevice *pOutDev );

    bool DrawAlphaRect( long nX, long nY, long nWidth, long nHeight,
                        sal_uInt8 nTransparency, const OutputDevice *pOutDev );

    virtual SystemGraphicsData   GetGraphicsData() const = 0;
    virtual SystemFontData       GetSysFontData( int nFallbacklevel ) const = 0;
};

#endif // _SV_SALGDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
