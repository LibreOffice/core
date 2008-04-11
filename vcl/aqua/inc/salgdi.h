/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salgdi.h,v $
 * $Revision: 1.37 $
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

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H

#include "premac.h"
#include <ApplicationServices/ApplicationServices.h>
#include "postmac.h"

#include "vcl/sv.h"
#include "vcl/outfont.hxx"
#include "vcl/salgdi.hxx"
#include "aquavcltypes.h"

#include "basebmp/bitmapdevice.hxx"

#include <vector>

class AquaSalFrame;
class AquaSalBitmap;
class ImplDevFontAttributes;

// mac specific physically available font face
class ImplMacFontData : public ImplFontData
{
public:
    ImplMacFontData( const ImplDevFontAttributes&, ATSUFontID );

    virtual ~ImplMacFontData();

    virtual ImplFontData*   Clone() const;
    virtual ImplFontEntry*  CreateFontInstance( ImplFontSelectData& ) const;
    virtual sal_IntPtr      GetFontId() const;

    ImplFontCharMap*        GetImplFontCharMap() const;
    bool                    HasChar( sal_uInt32 cChar ) const;

    void                    ReadOs2Table() const;
    void                    ReadMacCmapEncoding() const;
    bool                    HasCJKSupport() const;

private:
    const ATSUFontID            mnFontId;
    mutable ImplFontCharMap*    mpCharMap;
    mutable bool                mbOs2Read;       // true if OS2-table related info is valid
    mutable bool                mbHasOs2Table;
    mutable bool                mbCmapEncodingRead; // true if cmap encoding of Mac font is read
    mutable bool                mbHasCJKSupport; // #i78970# CJK fonts need extra leading
};


// -------------------
// - AquaSalGraphics -
// -------------------
class AquaSalGraphics : public SalGraphics
{
    friend class ATSLayout;
protected:
    AquaSalFrame*                           mpFrame;
    /// graphics context for Quartz 2D
    CGContextRef                            mrContext;
    /// device resolution of this graphics
    long                                    mnRealDPIX;
    long                                    mnRealDPIY;
    /// some graphics implementations (e.g. AquaSalInfoPrinter) scale
    /// everything down by a factor (see SetupPrinterGraphics for details)
    /// so we have to compensate for it with the inverse factor
    double                                  mfFakeDPIScale;

    /// memory for graphics bitmap context (window or virdev)
    boost::shared_array< sal_uInt8 >        maContextMemory;
    /// basebmp::BitmapDevice used for XOR rendering
    basebmp::BitmapDeviceSharedPtr          maXORDevice;
    basebmp::BitmapDeviceSharedPtr          maXORClipMask;

    /// path representing current clip region
    CGMutablePathRef                        mrClippingPath;
    std::vector< CGRect >                   maClippingRects;

    /// Drawing colors
    /// pen color RGBA
    float                                   mpLineColor[4];
    /// brush color RGBA
    float                                   mpFillColor[4];
    /// is XOR mode enabled ?
    bool                                    mbXORMode;

    // Device Font settings
     const ImplMacFontData*                  mpMacFontData;
    /// ATSU style object which carries all font attributes
    ATSUStyle                               maATSUStyle;
    /// text rotation as ATSU angle
    Fixed                                   mnATSUIRotation;
    /// workaround to prevent ATSU overflows for huge font sizes
    float                                   mfFontScale;
    /// <1.0: font is squeezed, >1.0 font is stretched, else 1.0
    float                                   mfFontStretch;
    /// allows text to be rendered without antialiasing
    bool                                    mbNonAntialiasedText;

    // Graphics types

    /// is this a printer graphics
    bool                                    mbPrinter;
    /// is this a virtual device graphics
    bool                                    mbVirDev;
    /// is this a window graphics
    bool                                    mbWindow;

private:
    /** returns the display id this window is mostly visible on */
    CGDirectDisplayID   GetWindowDisplayID() const;
public:
    AquaSalGraphics();
    virtual ~AquaSalGraphics();

    bool                IsPenTransparent() const        { return (mpLineColor[3] == 0.0); }
    bool                IsBrushTransparent() const      { return (mpFillColor[3] == 0.0); }

    void                SetWindowGraphics( AquaSalFrame* pFrame );
    void                SetPrinterGraphics( CGContextRef, long nRealDPIX, long nRealDPIY, double fFakeScale );
    void                SetVirDevGraphics( CGContextRef xContext, bool bSCreenCompatible );

    void                updateResolution();

    bool                IsWindowGraphics()      const   { return mbWindow; }
    bool                IsPrinterGraphics()     const   { return mbPrinter; }
    bool                IsVirDevGraphics()      const   { return mbVirDev; }

    void                ImplDrawPixel( long nX, long nY, float pColor[] ); // helper to draw single pixels

    bool                CheckContext();
    void                UpdateWindow( NSRect& rRect ); // delivered in NSView coordinates
    void                RefreshRect(float lX, float lY, float lWidth, float lHeight);

    void                SetState();

    virtual BOOL        unionClipRegion( long nX, long nY, long nWidth, long nHeight );
    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void        drawPixel( long nX, long nY );
    virtual void        drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void        drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void        drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void        drawPolyLine( ULONG nPoints, const SalPoint* pPtAry );
    virtual void        drawPolygon( ULONG nPoints, const SalPoint* pPtAry );
    virtual void        drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry );
    virtual sal_Bool    drawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry );
    virtual sal_Bool    drawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry );
    virtual sal_Bool    drawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints, const SalPoint* const* pPtAry, const BYTE* const* pFlgAry );

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void        copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                  long nSrcHeight, USHORT nFlags );

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void        copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics );
    virtual void        drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap );
    virtual void        drawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nTransparentColor );
    virtual void        drawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap );
    virtual void        drawMask( const SalTwoRect* pPosAry,
                                  const SalBitmap& rSalBitmap,
                                  SalColor nMaskColor );

    virtual SalBitmap*  getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor    getPixel( long nX, long nY );

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void        invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags);
    virtual void        invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual BOOL        drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize );

    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap );

    virtual bool            drawAlphaRect( long nX, long nY, long nWidth,
                                           long nHeight, sal_uInt8 nTransparency );

    CGPoint*                makeCGptArray(ULONG nPoints, const SalPoint*  pPtAry);
    // native widget rendering methods that require mirroring
    virtual BOOL        hitTestNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                              const Point& aPos, SalControlHandle& rControlHandle, BOOL& rIsInside );
    virtual BOOL        drawNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                           const rtl::OUString& aCaption );
    virtual BOOL        drawNativeControlText( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               SalControlHandle& rControlHandle, const rtl::OUString& aCaption );
    virtual BOOL        getNativeControlRegion( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, SalControlHandle& rControlHandle, const rtl::OUString& aCaption,
                                                Region &rNativeBoundingRegion, Region &rNativeContentRegion );

public:
    // public SalGraphics methods, the interface to teh independent vcl part

    // get device resolution
    virtual void            GetResolution( long& rDPIX, long& rDPIY );
    // get resolution for fonts (an implementations specific adjustment,
    // ideally would be the same as the Resolution)
    virtual void            GetScreenFontResolution( long& rDPIX, long& rDPIY );
    // get the depth of the device
    virtual USHORT          GetBitCount();
    // get the width of the device
    virtual long            GetGraphicsWidth() const;

    // set the clip region to empty
    virtual void            ResetClipRegion();
    // begin setting the clip region, add rectangles to the
    // region with the UnionClipRegion call
    virtual void            BeginSetClipRegion( ULONG nCount );
    // all rectangles were added and the clip region should be set now
    virtual void            EndSetClipRegion();

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
    virtual void            SetXORMode( BOOL bSet );
    // set line color for raster operations
    virtual void            SetROPLineColor( SalROPColor nROPColor );
    // set fill color for raster operations
    virtual void            SetROPFillColor( SalROPColor nROPColor );
    // set the text color to a specific color
    virtual void            SetTextColor( SalColor nSalColor );
    // set the font
    virtual USHORT         SetFont( ImplFontSelectData*, int nFallbackLevel );
    // get the current font's etrics
    virtual void            GetFontMetric( ImplFontMetricData* );
    // get kernign pairs of the current font
    // return only PairCount if (pKernPairs == NULL)
    virtual ULONG           GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs );
    // get the repertoire of the current font
    virtual ImplFontCharMap* GetImplFontCharMap() const;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( ImplDevFontList* );
    // graphics should call ImplAddDevFontSubstitute on supplied
    // OutputDevice for all its device specific preferred font substitutions
    virtual void            GetDevFontSubstList( OutputDevice* );
    virtual bool            AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName );
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
    virtual BOOL            CreateFontSubset( const rtl::OUString& rToFile,
                                              const ImplFontData* pFont,
                                              long* pGlyphIDs,
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
    virtual const Ucs2SIntMap* GetFontEncodingVector( const ImplFontData*, const Ucs2OStrMap** ppNonEncoded );

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void* GetEmbedFontData( const ImplFontData*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );

    virtual void            GetGlyphWidths( const ImplFontData*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );

    virtual BOOL                    GetGlyphBoundRect( long nIndex, Rectangle& );
    virtual BOOL                    GetGlyphOutline( long nIndex, basegfx::B2DPolyPolygon& );

    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void                     DrawServerFontLayout( const ServerFontLayout& );
    virtual bool                    supportsOperation( OutDevSupportType ) const;

    // Query the platform layer for control support
    virtual BOOL IsNativeControlSupported( ControlType nType, ControlPart nPart );
};

#endif // _SV_SALGDI_H
