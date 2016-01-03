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

#ifndef INCLUDED_VCL_INC_QUARTZ_SALGDI_H
#define INCLUDED_VCL_INC_QUARTZ_SALGDI_H

#include <vector>

#include "basegfx/polygon/b2dpolypolygon.hxx"

#include "premac.h"
#ifdef MACOSX
#include <ApplicationServices/ApplicationServices.h>
#include "osx/osxvcltypes.h"
#include "osx/salframe.h"
#else
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#endif
#include "postmac.h"

#include <vcl/fontcapabilities.hxx>
#include <vcl/metric.hxx>

#include "outfont.hxx"
#include "PhysicalFontFace.hxx"
#include "salgdi.hxx"

#include "quartz/salgdicommon.hxx"
#include <unordered_map>

class AquaSalFrame;
class ImplFontAttributes;
class CoreTextStyle;
class XorEmulation;

typedef sal_uInt32 sal_GlyphId;
typedef std::vector<unsigned char> ByteVector;

// CoreText-specific physically available font face
class CoreTextFontData : public PhysicalFontFace
{
public:
                                    CoreTextFontData( const ImplFontAttributes&, sal_IntPtr nFontID );
    virtual                         ~CoreTextFontData();

    PhysicalFontFace*               Clone() const override;
    ImplFontEntry*                  CreateFontInstance( FontSelectPattern& ) const override;
    sal_IntPtr                      GetFontId() const override;

    int                             GetFontTable( const char pTagName[5], unsigned char* ) const;

    const FontCharMapPtr            GetFontCharMap() const;
    bool                            GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const;
    bool                            HasChar( sal_uInt32 cChar ) const;

    void                            ReadOs2Table() const;
    void                            ReadMacCmapEncoding() const;

protected:
                                    CoreTextFontData( const CoreTextFontData& );

private:
    const sal_IntPtr                mnFontId;
    mutable FontCharMapPtr          mpCharMap;
    mutable vcl::FontCapabilities   maFontCapabilities;
    mutable bool                    mbOs2Read;       // true if OS2-table related info is valid
    mutable bool                    mbHasOs2Table;
    mutable bool                    mbCmapEncodingRead; // true if cmap encoding of Mac font is read
    mutable bool                    mbFontCapabilitiesRead;
};

class CoreTextStyle
{
public:
    CoreTextStyle( const FontSelectPattern& );
    ~CoreTextStyle( void );

    SalLayout* GetTextLayout( void ) const;

    void       GetFontAttributes( ImplFontAttributes& ) const;
    bool       GetGlyphBoundRect( sal_GlyphId, Rectangle& ) const;
    bool       GetGlyphOutline( sal_GlyphId, basegfx::B2DPolyPolygon& ) const;

    const CoreTextFontData*  mpFontData;
    /// <1.0: font is squeezed, >1.0 font is stretched, else 1.0
    float               mfFontStretch;
    /// text rotation in radian
    float               mfFontRotation;

private:
    /// CoreText text style object
    CFMutableDictionaryRef  mpStyleDict;

    friend class CTLayout;
    CFMutableDictionaryRef  GetStyleDict( void ) const { return mpStyleDict; }
};

// - SystemFontList -
// TODO: move into cross-platform headers

class SystemFontList
{
public:
    SystemFontList( void );
    ~SystemFontList( void );

    bool        Init( void );
    void        AddFont( CoreTextFontData* );

    void    AnnounceFonts( PhysicalFontCollection& ) const;
    CoreTextFontData* GetFontDataFromId( sal_IntPtr nFontId ) const;

private:
    CTFontCollectionRef mpCTFontCollection;
    CFArrayRef mpCTFontArray;

    typedef std::unordered_map<sal_IntPtr,CoreTextFontData*> CTFontContainer;
    CTFontContainer maFontContainer;
};

// - AquaSalGraphics -

class AquaSalGraphics : public SalGraphics
{
    friend class CTLayout;
protected:
    CGLayerRef                              mxLayer;    // Quartz graphics layer
    CGContextRef                            mrContext;  // Quartz drawing context
#ifdef MACOSX
    AquaSalFrame*                           mpFrame;
#endif
    int                                     mnContextStackDepth;
    XorEmulation*                           mpXorEmulation;
    int                                     mnXorMode; // 0: off 1: on 2: invert only
    int                                     mnWidth;
    int                                     mnHeight;
    int                                     mnBitmapDepth;  // zero unless bitmap
    /// device resolution of this graphics
    long                                    mnRealDPIX;
    long                                    mnRealDPIY;

    /// path representing current clip region
    CGMutablePathRef                        mxClipPath;

    /// Drawing colors
    /// pen color RGBA
    RGBAColor                               maLineColor;
    /// brush color RGBA
    RGBAColor                               maFillColor;

    // Device Font settings
    const CoreTextFontData*                 mpFontData;
    CoreTextStyle*                          mpTextStyle;
    RGBAColor                               maTextColor;
    /// allows text to be rendered without antialiasing
    bool                                    mbNonAntialiasedText;

    // Graphics types

    /// is this a printer graphics
    bool                                    mbPrinter;
    /// is this a virtual device graphics
    bool                                    mbVirDev;
#ifdef MACOSX
    /// is this a window graphics
    bool                                    mbWindow;

#else // IOS

    // mirror AquaSalVirtualDevice::mbForeignContext for SvpSalGraphics objects related to such
    bool mbForeignContext;

#endif

public:
                            AquaSalGraphics();
    virtual                 ~AquaSalGraphics();

    bool                    IsPenVisible() const    { return maLineColor.IsVisible(); }
    bool                    IsBrushVisible() const  { return maFillColor.IsVisible(); }

    void                    SetWindowGraphics( AquaSalFrame* pFrame );
    void                    SetPrinterGraphics( CGContextRef, long nRealDPIX, long nRealDPIY );
    void                    SetVirDevGraphics( CGLayerRef, CGContextRef, int nBitDepth = 0 );
#ifdef MACOSX
    void                    initResolution( NSWindow* );
    void                    copyResolution( AquaSalGraphics& );
    void                    updateResolution();

    bool                    IsWindowGraphics()      const   { return mbWindow; }
    AquaSalFrame*           getGraphicsFrame() const { return mpFrame; }
    void                    setGraphicsFrame( AquaSalFrame* pFrame ) { mpFrame = pFrame; }
#endif

    void                    ImplDrawPixel( long nX, long nY, const RGBAColor& ); // helper to draw single pixels

    bool                    CheckContext();
    CGContextRef            GetContext();
#ifdef MACOSX
    void                    UpdateWindow( NSRect& ); // delivered in NSView coordinates
    void                    RefreshRect( const NSRect& );
#else
    void                    RefreshRect( const CGRect& ) {}
#endif
    void                    RefreshRect(float lX, float lY, float lWidth, float lHeight);

    void                    SetState();
    void                    UnsetState();
    // InvalidateContext does an UnsetState and sets mrContext to 0
    void                    InvalidateContext();

    virtual SalGraphicsImpl* GetImpl() const override;

    virtual bool            setClipRegion( const vcl::Region& ) override;

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void            drawPixel( long nX, long nY ) override;
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor ) override;
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 ) override;
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight ) override;
    virtual void            drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void            drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void            drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry ) override;
    virtual bool            drawPolyPolygon( const basegfx::B2DPolyPolygon&, double fTransparency ) override;
    virtual bool            drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry ) override;
    virtual bool            drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry ) override;
    virtual bool            drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints, const SalPoint* const* pPtAry, const sal_uInt8* const* pFlgAry ) override;
    virtual bool            drawPolyLine(
                                const basegfx::B2DPolygon&,
                                double fTransparency,
                                const basegfx::B2DVector& rLineWidths,
                                basegfx::B2DLineJoin,
                                css::drawing::LineCap eLineCap) override;
    virtual bool            drawGradient( const tools::PolyPolygon&, const Gradient& ) override { return false; };

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void            copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                      long nSrcHeight, sal_uInt16 nFlags ) override;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void            copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) override;
    virtual void            drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) override;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap ) override;
    virtual void            drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor ) override;

    virtual SalBitmap*      getBitmap( long nX, long nY, long nWidth, long nHeight ) override;
    virtual SalColor        getPixel( long nX, long nY ) override;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void            invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags) override;
    virtual void            invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) override;

    virtual bool            drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize ) override;

    virtual bool            blendBitmap( const SalTwoRect&,
                                         const SalBitmap& rBitmap ) override;

    virtual bool            blendAlphaBitmap( const SalTwoRect&,
                                              const SalBitmap& rSrcBitmap,
                                              const SalBitmap& rMaskBitmap,
                                              const SalBitmap& rAlphaBitmap ) override;

    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap ) override;

    bool                    drawTransformedBitmap(
                                            const basegfx::B2DPoint& rNull,
                                            const basegfx::B2DPoint& rX,
                                            const basegfx::B2DPoint& rY,
                                            const SalBitmap& rSourceBitmap,
                                            const SalBitmap* pAlphaBitmap) override;

    virtual bool            drawAlphaRect( long nX, long nY, long nWidth,
                                           long nHeight, sal_uInt8 nTransparency ) override;

    // native widget rendering methods that require mirroring
#ifdef MACOSX
    virtual bool            hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                                  const Point& aPos, bool& rIsInside ) override;
    virtual bool            drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               const OUString& aCaption ) override;
    virtual bool            getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                    const ImplControlValue& aValue, const OUString& aCaption,
                                                    Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion ) override;
#endif

    // get device resolution
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;
    // get the depth of the device
    virtual sal_uInt16      GetBitCount() const override;
    // get the width of the device
    virtual long            GetGraphicsWidth() const override;

    // set the clip region to empty
    virtual void            ResetClipRegion() override;

    // set the line color to transparent (= don't draw lines)
    virtual void            SetLineColor() override;
    // set the line color to a specific color
    virtual void            SetLineColor( SalColor nSalColor ) override;
    // set the fill color to transparent (= don't fill)
    virtual void            SetFillColor() override;
    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void            SetFillColor( SalColor nSalColor ) override;
    // enable/disable XOR drawing
    virtual void            SetXORMode( bool bSet, bool bInvertOnly ) override;
    // set line color for raster operations
    virtual void            SetROPLineColor( SalROPColor nROPColor ) override;
    // set fill color for raster operations
    virtual void            SetROPFillColor( SalROPColor nROPColor ) override;
    // set the text color to a specific color
    virtual void            SetTextColor( SalColor nSalColor ) override;
    // set the font
    virtual sal_uInt16      SetFont( FontSelectPattern*, int nFallbackLevel ) override;
    // get the current font's metrics
    virtual void            GetFontAttributes( ImplFontAttributes*, int nFallbackLevel ) override;
    // get the repertoire of the current font
    virtual const FontCharMapPtr GetFontCharMap() const override;
    virtual bool            GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( PhysicalFontCollection* ) override;
    // graphics must drop any cached font info
    virtual void            ClearDevFontCache() override;
    virtual bool            AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;
    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns TRUE if creation of subset was successful
    // parameters: rToFile: contains a osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the correspoding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    virtual bool            CreateFontSubset( const OUString& rToFile,
                                                  const PhysicalFontFace* pFont,
                                                  const sal_GlyphId* pGlyphIds,
                                                  const sal_uInt8* pEncoding,
                                                  sal_Int32* pWidths,
                                                  int nGlyphs,
                                                  FontSubsetInfo& rInfo // out parameter
                                                  ) override;

    // GetFontEncodingVector: a method to get the encoding map Unicode
    // to font encoded character; this is only used for type1 fonts and
    // may return NULL in case of unknown encoding vector
    // if ppNonEncoded is set and non encoded characters (that is type1
    // glyphs with only a name) exist it is set to the corresponding
    // map for non encoded glyphs; the encoding vector contains -1
    // as encoding for these cases
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded, std::set<sal_Unicode> const** ) override;

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void*     GetEmbedFontData( const PhysicalFontFace*,
                                              const sal_Ucs* pUnicodes,
                                              sal_Int32* pWidths,
                                              size_t nLen,
                                              FontSubsetInfo& rInfo,
                                              long* pDataLen ) override;
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen ) override;

    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) override;

    virtual bool            GetGlyphBoundRect( sal_GlyphId, Rectangle& ) override;
    virtual bool            GetGlyphOutline( sal_GlyphId, basegfx::B2DPolyPolygon& ) override;

    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) override;
    virtual void            DrawServerFontLayout( const ServerFontLayout& ) override;
    virtual bool            supportsOperation( OutDevSupportType ) const override;

#ifdef MACOSX
    // Query the platform layer for control support
    virtual bool            IsNativeControlSupported( ControlType nType, ControlPart nPart ) override;
#endif

    virtual SystemGraphicsData
                            GetGraphicsData() const override;

private:
    // differences between VCL, Quartz and kHiThemeOrientation coordinate systems
    // make some graphics seem to be vertically-mirrored from a VCL perspective
    bool                    IsFlipped() const;

    void                    ApplyXorContext();
    void                    Pattern50Fill();
    UInt32                  getState( ControlState nState );
    UInt32                  getTrackState( ControlState nState );
    static bool             GetRawFontData( const PhysicalFontFace* pFontData,
                                std::vector<unsigned char>& rBuffer,
                                bool* pJustCFF );
};

// --- some trivial inlines

#ifdef MACOSX

inline void AquaSalGraphics::RefreshRect( const NSRect& rRect )
{
    RefreshRect( rRect.origin.x, rRect.origin.y, rRect.size.width, rRect.size.height );
}

#endif

#endif // INCLUDED_VCL_INC_QUARTZ_SALGDI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
