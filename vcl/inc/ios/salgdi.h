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

#ifndef _VCL_IOS_SALGDI_H
#define _VCL_IOS_SALGDI_H

#include "basegfx/polygon/b2dpolypolygon.hxx"

#include "ios/iosvcltypes.h"
#include "ios/salcoretextfontutils.hxx"
#include "ios/salframe.h"
#include "salgdi.hxx"

#include "ios/salgdicommon.hxx"

class CoreTextStyleInfo;

// -------------------
// - IosSalGraphics -
// -------------------
class IosSalGraphics : public SalGraphics
{
    friend class CoreTextLayout;
protected:
    IosSalFrame* mpFrame;
    CGLayerRef mxLayer;    //< Quartz graphics layer
    CGContextRef mrContext;  //< Quartz drawing context
    class XorEmulation* mpXorEmulation;
    int mnXorMode; //< 0: off 1: on 2: invert only
    int mnWidth;
    int mnHeight;
    int mnBitmapDepth;  //< zero unless bitmap
    long mnRealDPIX; //< device X-resolution of this graphics
    long mnRealDPIY; //< device Y-resolution of this graphics

    /// some graphics implementations (e.g. IosSalInfoPrinter) scale
    /// everything down by a factor (see SetupPrinterGraphics for details)
    /// so we have to compensate for it with the inverse factor
    double mfFakeDPIScale;
    double mfFontScale;


    CGMutablePathRef mxClipPath; //< path representing current clip region

    /// Drawing colors
    RGBAColor maLineColor; //< pen color RGBA
    RGBAColor maFillColor; //< brush color RGBA

    ImplCoreTextFontData* m_pCoreTextFontData; //< Device Font settings

    bool mbNonAntialiasedText; //< allows text to be rendered without antialiasing

    // Graphics types

    bool mbPrinter; //< is this a printer graphics
    bool mbVirDev; //< is this a virtual device graphics
    bool mbWindow; //< is this a window graphics

    CoreTextStyleInfo* m_style;

public:
    IosSalGraphics();
    virtual ~IosSalGraphics();

    bool IsPenVisible() const { return maLineColor.IsVisible(); }
    bool IsBrushVisible() const { return maFillColor.IsVisible(); }

    void SetWindowGraphics( IosSalFrame* pFrame );
    void SetPrinterGraphics( CGContextRef, long nRealDPIX, long nRealDPIY, double fFakeScale );
    void SetVirDevGraphics( CGLayerRef, CGContextRef, int nBitDepth = 0 );

    void initResolution( UIWindow* );
    void copyResolution( IosSalGraphics& );
    void updateResolution();

    bool IsWindowGraphics() const { return mbWindow; }
    bool IsPrinterGraphics() const { return mbPrinter; }
    bool IsVirDevGraphics() const { return mbVirDev; }
    IosSalFrame* getGraphicsFrame() const { return mpFrame; }
    void setGraphicsFrame( IosSalFrame* pFrame ) { mpFrame = pFrame; }

    void ImplDrawPixel( long nX, long nY, const RGBAColor& ); // helper to draw single pixels

    bool CheckContext();
    CGContextRef GetContext();
    void UpdateWindow( CGRect& ); // delivered in UIView coordinates
    void RefreshRect( const CGRect& );
    void RefreshRect(float lX, float lY, float lWidth, float lHeight);

    void SetState();
    void UnsetState();
    // InvalidateContext does an UnsetState and sets mrContext to 0
    void InvalidateContext();

    virtual bool setClipRegion( const Region& );

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void drawPixel( long nX, long nY );
    virtual void drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void drawPolyLine( sal_uLong nPoints, const SalPoint* pPtAry );
    virtual void drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry );
    virtual void drawPolyPolygon( sal_uLong nPoly, const sal_uLong* pPoints, PCONSTSALPOINT* pPtAry );
    virtual bool drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual sal_Bool drawPolyLineBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry );
    virtual sal_Bool drawPolygonBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry );
    virtual sal_Bool drawPolyPolygonBezier( sal_uLong nPoly, const sal_uLong* pPoints,
                                            const SalPoint* const* pPtAry, const sal_uInt8* const* pFlgAry );
    virtual bool drawPolyLine( const ::basegfx::B2DPolygon&, double fTransparency,
                               const ::basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin );

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                           long nSrcHeight, sal_uInt16 nFlags );

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics );
    virtual void drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap );
    virtual void drawBitmap( const SalTwoRect* pPosAry,
                             const SalBitmap& rSalBitmap,
                             SalColor nTransparentColor );
    virtual void drawBitmap( const SalTwoRect* pPosAry,
                             const SalBitmap& rSalBitmap,
                             const SalBitmap& rTransparentBitmap );
    virtual void drawMask( const SalTwoRect* pPosAry,
                           const SalBitmap& rSalBitmap,
                           SalColor nMaskColor );

    virtual SalBitmap* getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor getPixel( long nX, long nY );

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags);
    virtual void invert( sal_uLong nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual sal_Bool drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize );

    virtual bool drawAlphaBitmap( const SalTwoRect&,
                                  const SalBitmap& rSourceBitmap,
                                  const SalBitmap& rAlphaBitmap );

    virtual bool drawAlphaRect( long nX, long nY, long nWidth,
                                long nHeight, sal_uInt8 nTransparency );

    CGPoint* makeCGptArray(sal_uLong nPoints, const SalPoint*  pPtAry);
    // native widget rendering methods that require mirroring
    virtual sal_Bool hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           const Point& aPos, sal_Bool& rIsInside );
    virtual sal_Bool drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                        ControlState nState, const ImplControlValue& aValue,
                                        const rtl::OUString& aCaption );
    virtual sal_Bool getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                             ControlState nState, const ImplControlValue& aValue,
                                             const rtl::OUString& aCaption,
                                             Rectangle &rNativeBoundingRegion,
                                             Rectangle &rNativeContentRegion );

    // get device resolution
    virtual void GetResolution( long& rDPIX, long& rDPIY );
    // get the depth of the device
    virtual sal_uInt16 GetBitCount() const;
    // get the width of the device
    virtual long GetGraphicsWidth() const;

    // set the clip region to empty
    virtual void ResetClipRegion();

    // set the line color to transparent (= don't draw lines)
    virtual void SetLineColor();
    // set the line color to a specific color
    virtual void SetLineColor( SalColor nSalColor );
    // set the fill color to transparent (= don't fill)
    virtual void SetFillColor();
    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void SetFillColor( SalColor nSalColor );
    // enable/disable XOR drawing
    virtual void SetXORMode( bool bSet, bool bInvertOnly );
    // set line color for raster operations
    virtual void SetROPLineColor( SalROPColor nROPColor );
    // set fill color for raster operations
    virtual void SetROPFillColor( SalROPColor nROPColor );
    // set the text color to a specific color
    virtual void SetTextColor( SalColor nSalColor );
    // set the font
    virtual sal_uInt16 SetFont( FontSelectPattern*, int nFallbackLevel );
    // get the current font's etrics
    virtual void GetFontMetric( ImplFontMetricData*, int nFallbackLevel );
    // get kernign pairs of the current font
    // return only PairCount if (pKernPairs == NULL)
    virtual sal_uLong GetKernPairs( sal_uLong nPairs, ImplKernPairData* pKernPairs );
    // get the repertoire of the current font
    virtual const ImplFontCharMap* GetImplFontCharMap() const;
    virtual bool GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const;
    // graphics must fill supplied font list
    virtual void GetDevFontList( ImplDevFontList* );
    // graphics should call ImplAddDevFontSubstitute on supplied
    // OutputDevice for all its device specific preferred font substitutions
    virtual void GetDevFontSubstList( OutputDevice* );
    virtual bool AddTempDevFont( ImplDevFontList*, const rtl::OUString& rFileURL,
                                 const rtl::OUString& rFontName );
    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns TRUE if creation of subset was successfull
    // parameters: rToFile: contains a osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the correspoding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    virtual sal_Bool CreateFontSubset( const rtl::OUString& rToFile,
                                       const PhysicalFontFace* pFont,
                                       long* pGlyphIDs,
                                       sal_uInt8* pEncoding,
                                       sal_Int32* pWidths,
                                       int nGlyphs,
                                       FontSubsetInfo& rInfo);

    // GetFontEncodingVector: a method to get the encoding map Unicode
    // to font encoded character; this is only used for type1 fonts and
    // may return NULL in case of unknown encoding vector
    // if ppNonEncoded is set and non encoded characters (that is type1
    // glyphs with only a name) exist it is set to the corresponding
    // map for non encoded glyphs; the encoding vector contains -1
    // as encoding for these cases
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded );

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void* GetEmbedFontData( const PhysicalFontFace*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    // frees the font data again
    virtual void FreeEmbedFontData( const void* pData, long nDataLen );

    virtual void GetGlyphWidths( const PhysicalFontFace*,
                                 bool bVertical,
                                 Int32Vector& rWidths,
                                 Ucs2UIntMap& rUnicodeEnc );

    virtual sal_Bool GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& );
    virtual sal_Bool GetGlyphOutline( sal_GlyphId nIndex, basegfx::B2DPolyPolygon& );

    virtual SalLayout* GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void DrawServerFontLayout( const ServerFontLayout& );
    virtual bool supportsOperation( OutDevSupportType ) const;

    // Query the platform layer for control support
    virtual sal_Bool IsNativeControlSupported( ControlType nType, ControlPart nPart );

    virtual SystemGraphicsData GetGraphicsData() const;
    virtual SystemFontData GetSysFontData( int /* nFallbacklevel */ ) const;

private:
    // differences between VCL, Quartz and kHiThemeOrientation coordinate systems
    // make some graphics seem to be vertically-mirrored from a VCL perspective
    bool IsFlipped() const { return mbWindow; };

    void ApplyXorContext();
    void Pattern50Fill();
    UInt32 getState( ControlState nState );
    UInt32 getTrackState( ControlState nState );
    bool GetRawFontData( const PhysicalFontFace* pFontData,
                         std::vector<unsigned char>& rBuffer,
                         bool* pJustCFF );
};

inline void IosSalGraphics::RefreshRect( const CGRect& rRect )
{
    RefreshRect( rRect.origin.x, rRect.origin.y, rRect.size.width, rRect.size.height );
}

#endif // _VCL_IOS_SALGDI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
