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

#ifndef INCLUDED_VCL_INC_HEADLESS_SVPGDI_HXX
#define INCLUDED_VCL_INC_HEADLESS_SVPGDI_HXX

#include <basebmp/bitmapdevice.hxx>
#include <basebmp/color.hxx>
#include <vcl/sysdata.hxx>

#include "salgdi.hxx"
#include "sallayout.hxx"

#ifdef IOS
#include "quartz/salgdi.h"
#include <premac.h>
#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <postmac.h>
#endif

class ServerFont;

#ifdef IOS
// To keep changes to the CoreText code shared with AOO to a minimum,
// let's continue calling the SalGraphics subclass "AquaSalGraphics" even if it
// is used by us also on iOS, where of course the term "Aqua" has no meaning at all.
// (Note that even on OS X, using the term "Aqua" is a misunderstanding or obsolete.)
#define SvpSalGraphics AquaSalGraphics
#endif

class SvpSalGraphics : public SalGraphics
{
#ifndef IOS
    basebmp::BitmapDeviceSharedPtr       m_aDevice;
    basebmp::BitmapDeviceSharedPtr       m_aOrigDevice;

    basebmp::BitmapDeviceSharedPtr       m_aClipMap;

    bool                                 m_bUseLineColor;
    basebmp::Color                       m_aLineColor;
    bool                                 m_bUseFillColor;
    basebmp::Color                       m_aFillColor;

    basebmp::DrawMode                    m_aDrawMode;

    // These fields are used only when we use FreeType to draw into a
    // headless backend, i.e. not on iOS.
    basebmp::Color                       m_aTextColor;
    ServerFont*                          m_pServerFont[ MAX_FALLBACK ];
    basebmp::Format                      m_eTextFmt;

protected:
    basegfx::B2IVector                   GetSize() { return m_aOrigDevice->getSize(); }

private:
    bool                                 m_bClipSetup;
    struct ClipUndoHandle {
        SvpSalGraphics                &m_rGfx;
        basebmp::BitmapDeviceSharedPtr m_aDevice;
        ClipUndoHandle( SvpSalGraphics *pGfx ) : m_rGfx( *pGfx ) {}
        ~ClipUndoHandle();
    };
    bool isClippedSetup( const basegfx::B2IBox &aRange, ClipUndoHandle &rUndo );
    void ensureClip();

public:
    void setDevice( basebmp::BitmapDeviceSharedPtr& rDevice );

#else
    friend class CTLayout;

    CGLayerRef                              mxLayer;
    // mirror AquaSalVirtualDevice::mbForeignContext for SvpSalGraphics objects related to such
    bool mbForeignContext;
    CGContextRef                         mrContext;
    class XorEmulation*                     mpXorEmulation;
    int                                     mnXorMode; // 0: off 1: on 2: invert only
    int                                     mnWidth;
    int                                     mnHeight;
    int                                  mnBitmapDepth;  // zero unless bitmap

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

    /// is this a printer graphics
    bool                                    mbPrinter;
    /// is this a virtual device graphics
    bool                                    mbVirDev;
#endif

protected:
    Region                               m_aClipRegion;

protected:
    virtual bool drawAlphaBitmap( const SalTwoRect&, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;
    virtual bool drawTransformedBitmap(
        const basegfx::B2DPoint& rNull,
        const basegfx::B2DPoint& rX,
        const basegfx::B2DPoint& rY,
        const SalBitmap& rSourceBitmap,
        const SalBitmap* pAlphaBitmap) SAL_OVERRIDE;
    virtual bool drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency ) SAL_OVERRIDE;

public:
    SvpSalGraphics();
    virtual ~SvpSalGraphics();

    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) SAL_OVERRIDE;
    virtual sal_uInt16      GetBitCount() const SAL_OVERRIDE;
    virtual long            GetGraphicsWidth() const SAL_OVERRIDE;

    virtual void            ResetClipRegion() SAL_OVERRIDE;
    virtual bool            setClipRegion( const Region& ) SAL_OVERRIDE;

    virtual void            SetLineColor() SAL_OVERRIDE;
    virtual void            SetLineColor( SalColor nSalColor ) SAL_OVERRIDE;
    virtual void            SetFillColor() SAL_OVERRIDE;
    virtual void            SetFillColor( SalColor nSalColor ) SAL_OVERRIDE;

    virtual void            SetXORMode( bool bSet, bool ) SAL_OVERRIDE;

    virtual void            SetROPLineColor( SalROPColor nROPColor ) SAL_OVERRIDE;
    virtual void            SetROPFillColor( SalROPColor nROPColor ) SAL_OVERRIDE;

    virtual void            SetTextColor( SalColor nSalColor ) SAL_OVERRIDE;
    virtual sal_uInt16      SetFont( FontSelectPattern*, int nFallbackLevel ) SAL_OVERRIDE;
    virtual void            GetFontMetric( ImplFontMetricData*, int nFallbackLevel ) SAL_OVERRIDE;
    virtual const ImplFontCharMap* GetImplFontCharMap() const SAL_OVERRIDE;
    virtual bool GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const SAL_OVERRIDE;
    virtual void            GetDevFontList( PhysicalFontCollection* ) SAL_OVERRIDE;
    virtual void ClearDevFontCache() SAL_OVERRIDE;
    virtual bool            AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) SAL_OVERRIDE;
    virtual bool        CreateFontSubset( const OUString& rToFile,
                                              const PhysicalFontFace*,
                                              sal_GlyphId* pGlyphIds,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              ) SAL_OVERRIDE;
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded ) SAL_OVERRIDE;
    virtual const void*     GetEmbedFontData( const PhysicalFontFace*,
                                              const sal_Ucs* pUnicodes,
                                              sal_Int32* pWidths,
                                              FontSubsetInfo& rInfo,
                                              long* pDataLen ) SAL_OVERRIDE;
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen ) SAL_OVERRIDE;
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) SAL_OVERRIDE;
    virtual bool            GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& ) SAL_OVERRIDE;
    virtual bool            GetGlyphOutline( sal_GlyphId nIndex, ::basegfx::B2DPolyPolygon& ) SAL_OVERRIDE;
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) SAL_OVERRIDE;
    virtual void            DrawServerFontLayout( const ServerFontLayout& ) SAL_OVERRIDE;
    virtual bool            supportsOperation( OutDevSupportType ) const SAL_OVERRIDE;
    virtual void            drawPixel( long nX, long nY ) SAL_OVERRIDE;
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor ) SAL_OVERRIDE;
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 ) SAL_OVERRIDE;
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual bool            drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency ) SAL_OVERRIDE;
    virtual bool            drawPolyLine( const ::basegfx::B2DPolygon&,
                                          double fTransparency,
                                          const ::basegfx::B2DVector& rLineWidths,
                                          basegfx::B2DLineJoin,
                                          com::sun::star::drawing::LineCap) SAL_OVERRIDE;
    virtual void            drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;
    virtual void            drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry ) SAL_OVERRIDE;
    virtual bool        drawPolyLineBezier( sal_uInt32 nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry ) SAL_OVERRIDE;
    virtual bool        drawPolygonBezier( sal_uInt32 nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry ) SAL_OVERRIDE;
    virtual bool        drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const sal_uInt8* const* pFlgAry ) SAL_OVERRIDE;

    virtual void            copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      sal_uInt16 nFlags ) SAL_OVERRIDE;
    virtual void            copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics ) SAL_OVERRIDE;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap ) SAL_OVERRIDE;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor ) SAL_OVERRIDE;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap ) SAL_OVERRIDE;
    virtual void            drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor ) SAL_OVERRIDE;
    virtual SalBitmap*      getBitmap( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual SalColor        getPixel( long nX, long nY ) SAL_OVERRIDE;
    virtual void            invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags ) SAL_OVERRIDE;
    virtual void            invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) SAL_OVERRIDE;

    virtual bool        drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize ) SAL_OVERRIDE;

    virtual SystemGraphicsData GetGraphicsData() const SAL_OVERRIDE;
    virtual SystemFontData  GetSysFontData( int nFallbacklevel ) const SAL_OVERRIDE;

#ifdef IOS
    void                SetVirDevGraphics( CGLayerRef xLayer, CGContextRef xContext, int = 0 );

    bool CheckContext();
    CGContextRef GetContext();
    bool GetRawFontData( const PhysicalFontFace* pFontData,
                         std::vector<unsigned char>& rBuffer,
                         bool* pJustCFF );
    void                RefreshRect( const CGRect& ) { };
    void                RefreshRect(float lX, float lY, float lWidth, float lHeight);
    void                SetState();
    void                UnsetState();
    void                InvalidateContext();
    bool                IsPenVisible() const    { return maLineColor.IsVisible(); }
    bool                IsBrushVisible() const  { return maFillColor.IsVisible(); }
    void                ImplDrawPixel( long nX, long nY, const RGBAColor& ); // helper to draw single pixels
    CGPoint*                makeCGptArray(sal_uLong nPoints, const SalPoint*  pPtAry);
    bool IsFlipped() const { return false; }
    void ApplyXorContext();
    void Pattern50Fill();
#endif
};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPGDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
