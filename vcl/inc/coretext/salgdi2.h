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

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H

#include <vector>

#include "basegfx/polygon/b2dpolypolygon.hxx"

#include "premac.h"
#ifdef MACOSX
#include <ApplicationServices/ApplicationServices.h>
#include "aqua/aquavcltypes.h"
#include "aqua/salframe.h"
#else
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#endif
#include "postmac.h"

#include <vcl/fontcapabilities.hxx>

#include "outfont.hxx"
#include "salgdi.hxx"

#include "quartz/salgdicommon.hxx"

class AquaSalFrame;
class ImplDevFontAttributes;
class ImplMacTextStyle;

struct CGRect;

typedef sal_uInt32 sal_GlyphId;
typedef std::vector<unsigned char> ByteVector;

#ifndef CGFLOAT_TYPE
typedef float CGFloat;
#endif

// mac specific physically available font face
class ImplMacFontData : public PhysicalFontFace
{
public:
    ImplMacFontData( const ImplDevFontAttributes&, sal_IntPtr nFontID );

    virtual ~ImplMacFontData();

    virtual PhysicalFontFace*   Clone() const = 0;
    virtual ImplFontEntry*  CreateFontInstance( FontSelectPattern& ) const;
    virtual sal_IntPtr      GetFontId() const;

    virtual ImplMacTextStyle* CreateMacTextStyle( const FontSelectPattern& ) const = 0;
    virtual int             GetFontTable( const char pTagName[5], unsigned char* ) const = 0;

    const ImplFontCharMap*  GetImplFontCharMap() const;
    bool                    GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const;
    bool                    HasChar( sal_uInt32 cChar ) const;

    void                    ReadOs2Table() const;
    void                    ReadMacCmapEncoding() const;

protected:
    ImplMacFontData( const ImplMacFontData&);
private:
    const sal_IntPtr            mnFontId;
    mutable const ImplFontCharMap*  mpCharMap;
    mutable vcl::FontCapabilities maFontCapabilities;
    mutable bool                mbOs2Read;       // true if OS2-table related info is valid
    mutable bool                mbHasOs2Table;
    mutable bool                mbCmapEncodingRead; // true if cmap encoding of Mac font is read
    mutable bool                mbFontCapabilitiesRead;
};

// --------------------
// - ImplMacTextStyle -
// --------------------
class ImplMacTextStyle
{
public:
    explicit        ImplMacTextStyle( const FontSelectPattern& );
    virtual         ~ImplMacTextStyle( void );

    virtual SalLayout* GetTextLayout( void ) const = 0;

    virtual void    GetFontMetric( float fPDIY, ImplFontMetricData& ) const = 0;
    virtual bool    GetGlyphBoundRect( sal_GlyphId, Rectangle& ) const = 0;
    virtual bool    GetGlyphOutline( sal_GlyphId, basegfx::B2DPolyPolygon& ) const = 0;

    virtual void    SetTextColor( const RGBAColor& ) = 0;

//###protected:
    const ImplMacFontData*  mpFontData;
    /// <1.0: font is squeezed, >1.0 font is stretched, else 1.0
    float               mfFontStretch;
    /// text rotation in radian
    float               mfFontRotation;
};

// ------------------
// - SystemFontList -
// TODO: move into cross-platform headers
// ------------------
class SystemFontList
{
public:
    virtual ~SystemFontList( void );

    virtual void    AnnounceFonts( ImplDevFontList& ) const = 0;
    virtual ImplMacFontData* GetFontDataFromId( sal_IntPtr nFontId ) const = 0;
};

#ifdef MACOSX

// -------------------
// - AquaSalGraphics -
// -------------------
class AquaSalGraphics : public SalGraphics
{
    friend class ATSLayout;
    friend class CTLayout;
protected:
    AquaSalFrame*                           mpFrame;
    CGLayerRef                              mxLayer;    // Quartz graphics layer
    CGContextRef                            mrContext;  // Quartz drawing context
    class XorEmulation*                     mpXorEmulation;
    int                                     mnXorMode; // 0: off 1: on 2: invert only
    int                                     mnWidth;
    int                                     mnHeight;
    int                                     mnBitmapDepth;  // zero unless bitmap
    /// device resolution of this graphics
    long                                    mnRealDPIX;
    long                                    mnRealDPIY;
    /// some graphics implementations (e.g. AquaSalInfoPrinter) scale
    /// everything down by a factor (see SetupPrinterGraphics for details)
    /// so we have to compensate for it with the inverse factor
    double                                  mfFakeDPIScale;

    /// path representing current clip region
    CGMutablePathRef                        mxClipPath;

    /// Drawing colors
    /// pen color RGBA
    RGBAColor                               maLineColor;
    /// brush color RGBA
    RGBAColor                               maFillColor;

    // Device Font settings
     const ImplMacFontData*                  mpMacFontData;
    ImplMacTextStyle*                       mpMacTextStyle;
    RGBAColor                               maTextColor;
    /// allows text to be rendered without antialiasing
    bool                                    mbNonAntialiasedText;

    // Graphics types

    /// is this a printer graphics
    bool                                    mbPrinter;
    /// is this a virtual device graphics
    bool                                    mbVirDev;
    /// is this a window graphics
    bool                                    mbWindow;

public:
    AquaSalGraphics();
    virtual ~AquaSalGraphics();

    bool                IsPenVisible() const    { return maLineColor.IsVisible(); }
    bool                IsBrushVisible() const  { return maFillColor.IsVisible(); }

    void                SetWindowGraphics( AquaSalFrame* pFrame );
    void                SetPrinterGraphics( CGContextRef, long nRealDPIX, long nRealDPIY, double fFakeScale );
    void                SetVirDevGraphics( CGLayerRef, CGContextRef, int nBitDepth = 0 );

    void                initResolution( NSWindow* );
    void                copyResolution( AquaSalGraphics& );
    void                updateResolution();

    bool                IsWindowGraphics()      const   { return mbWindow; }
    AquaSalFrame*       getGraphicsFrame() const { return mpFrame; }
    void                setGraphicsFrame( AquaSalFrame* pFrame ) { mpFrame = pFrame; }

    void                ImplDrawPixel( long nX, long nY, const RGBAColor& ); // helper to draw single pixels

    bool                CheckContext();
    CGContextRef        GetContext();
    void                UpdateWindow( NSRect& ); // delivered in NSView coordinates
#if !defined(__LP64__) && !defined(NS_BUILD_32_LIKE_64)
    void                RefreshRect( const CGRect& );
#endif
    void                RefreshRect( const NSRect& );
    void                RefreshRect(float lX, float lY, float lWidth, float lHeight);

    void                SetState();
    void                UnsetState();
    // InvalidateContext does an UnsetState and sets mrContext to 0
    void                InvalidateContext();

    virtual bool        setClipRegion( const Region& );

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void        drawPixel( long nX, long nY );
    virtual void        drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void        drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void        drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void        drawPolyLine( sal_uLong nPoints, const SalPoint* pPtAry );
    virtual void        drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry );
    virtual void        drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry );
    virtual bool        drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual sal_Bool    drawPolyLineBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry );
    virtual sal_Bool    drawPolygonBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry );
    virtual sal_Bool    drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints, const SalPoint* const* pPtAry, const sal_uInt8* const* pFlgAry );
    virtual bool        drawPolyLine(
        const ::basegfx::B2DPolygon&,
        double fTransparency,
        const ::basegfx::B2DVector& rLineWidths,
        basegfx::B2DLineJoin,
        com::sun::star::drawing::LineCap eLineCap);

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void        copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                  long nSrcHeight, sal_uInt16 nFlags );

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void        copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics );
    virtual void        drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap );
    virtual void        drawBitmap( const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nTransparentColor );
    virtual void        drawBitmap( const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap );
    virtual void        drawMask( const SalTwoRect& rPosAry,
                                  const SalBitmap& rSalBitmap,
                                  SalColor nMaskColor );

    virtual SalBitmap*  getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor    getPixel( long nX, long nY );

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void        invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags);
    virtual void        invert( sal_uLong nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual sal_Bool        drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize );

    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap );

    bool drawTransformedBitmap(
        const basegfx::B2DPoint& rNull,
        const basegfx::B2DPoint& rX,
        const basegfx::B2DPoint& rY,
        const SalBitmap& rSourceBitmap,
        const SalBitmap* pAlphaBitmap);

    virtual bool            drawAlphaRect( long nX, long nY, long nWidth,
                                           long nHeight, sal_uInt8 nTransparency );

    CGPoint*                makeCGptArray(sal_uLong nPoints, const SalPoint*  pPtAry);
    // native widget rendering methods that require mirroring
    virtual sal_Bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                              const Point& aPos, sal_Bool& rIsInside );
    virtual sal_Bool        drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const OUString& aCaption );
    virtual sal_Bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const OUString& aCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion );

    // get device resolution
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    // get the depth of the device
    virtual sal_uInt16          GetBitCount() const;
    // get the width of the device
    virtual long            GetGraphicsWidth() const;

    // set the clip region to empty
    virtual void            ResetClipRegion();

    // set the line color to transparent (= don't draw lines)
    virtual void            SetLineColor();
    // set the line color to a specific color
    virtual void            SetLineColor( SalColor nSalColor );
    // set the fill color to transparent (= don't fill)
    virtual void            SetFillColor();
    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void            SetFillColor( SalColor nSalColor );
    // enable/disable XOR drawing
    virtual void            SetXORMode( bool bSet, bool bInvertOnly );
    // set line color for raster operations
    virtual void            SetROPLineColor( SalROPColor nROPColor );
    // set fill color for raster operations
    virtual void            SetROPFillColor( SalROPColor nROPColor );
    // set the text color to a specific color
    virtual void            SetTextColor( SalColor nSalColor );
    // set the font
    virtual sal_uInt16         SetFont( FontSelectPattern*, int nFallbackLevel );
    // get the current font's metrics
    virtual void            GetFontMetric( ImplFontMetricData*, int nFallbackLevel );
    // get the repertoire of the current font
    virtual const ImplFontCharMap* GetImplFontCharMap() const;
    virtual bool GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( ImplDevFontList* );
    // graphics must drop any cached font info
    virtual void ClearDevFontCache();
    // graphics should call ImplAddDevFontSubstitute on supplied
    // OutputDevice for all its device specific preferred font substitutions
    virtual void            GetDevFontSubstList( OutputDevice* );
    virtual bool            AddTempDevFont( ImplDevFontList*, const OUString& rFileURL, const OUString& rFontName );
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
    virtual sal_Bool            CreateFontSubset( const OUString& rToFile,
                                              const PhysicalFontFace* pFont,
                                              sal_Int32* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo // out parameter
                                              );

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
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );

    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );

    virtual sal_Bool                    GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& );
    virtual sal_Bool                    GetGlyphOutline( sal_GlyphId nIndex, basegfx::B2DPolyPolygon& );

    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void                     DrawServerFontLayout( const ServerFontLayout& );
    virtual bool                    supportsOperation( OutDevSupportType ) const;

    // Query the platform layer for control support
    virtual sal_Bool IsNativeControlSupported( ControlType nType, ControlPart nPart );

    virtual SystemGraphicsData    GetGraphicsData() const;
    virtual SystemFontData        GetSysFontData( int /* nFallbacklevel */ ) const;

private:
    // differences between VCL, Quartz and kHiThemeOrientation coordinate systems
    // make some graphics seem to be vertically-mirrored from a VCL perspective
    bool IsFlipped() const { return mbWindow; }

    void ApplyXorContext();
    void Pattern50Fill();
    UInt32 getState( ControlState nState );
    UInt32 getTrackState( ControlState nState );
    bool GetRawFontData( const PhysicalFontFace* pFontData,
                         std::vector<unsigned char>& rBuffer,
                         bool* pJustCFF );
};

// --- some trivial inlines

#if !defined(__LP64__) && !defined(NS_BUILD_32_LIKE_64)

inline void AquaSalGraphics::RefreshRect( const CGRect& rRect )
{
    RefreshRect( rRect.origin.x, rRect.origin.y, rRect.size.width, rRect.size.height );
}

#endif

inline void AquaSalGraphics::RefreshRect( const NSRect& rRect )
{
    RefreshRect( rRect.origin.x, rRect.origin.y, rRect.size.width, rRect.size.height );
}

#endif

#endif // _SV_SALGDI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
