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

#ifndef INCLUDED_VCL_INC_OPENGL_SALGDI_H
#define INCLUDED_VCL_INC_OPENGL_SALGDI_H


#include "salgdi.hxx"
#include <vcl/sysdata.hxx>

class OpenGLSalGraphics : public SalGraphics
{
protected:
    virtual bool drawTransformedBitmap(
                                       const basegfx::B2DPoint& /* rNull */,
                                       const basegfx::B2DPoint& /* rX */,
                                       const basegfx::B2DPoint& /* rY */,
                                       const SalBitmap& /* rSourceBitmap */,
                                       const SalBitmap* /* pAlphaBitmap */) SAL_OVERRIDE
    {
        return false;
    }
public:
                            OpenGLSalGraphics() {}
    virtual                 ~OpenGLSalGraphics() {}

    virtual bool            setClipRegion( const vcl::Region& ) SAL_OVERRIDE
    {
        return false;
    }

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void            drawPixel( long /* nX */, long /* nY */ ) SAL_OVERRIDE {}
    virtual void            drawPixel( long /* nX */, long /* nY */, SalColor /* nSalColor */ ) SAL_OVERRIDE {}
    virtual void            drawLine( long /* nX1 */, long /* nY1 */, long /* nX2 */, long /* nY2 */ ) SAL_OVERRIDE {}
    virtual void            drawRect( long /* nX */, long /* nY */, long /* nWidth */, long /* nHeight */ ) SAL_OVERRIDE {}
    virtual void            drawPolyLine( sal_uInt32 /* nPoints */, const SalPoint* /* pPtAry */ ) SAL_OVERRIDE {}
    virtual void            drawPolygon( sal_uInt32 /* nPoints */, const SalPoint* /* pPtAry */ ) SAL_OVERRIDE {}
    virtual void            drawPolyPolygon( sal_uInt32 /* nPoly */, const sal_uInt32* /* pPoints */, PCONSTSALPOINT* /* pPtAry */ ) SAL_OVERRIDE {}
    virtual bool            drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double /* fTransparency */ ) SAL_OVERRIDE
    {
        return false;
    }
    virtual bool            drawPolyLineBezier( sal_uInt32 /* nPoints */, const SalPoint* /* pPtAry */, const sal_uInt8* /* pFlgAry */ ) SAL_OVERRIDE
    {
        return false;
    }
    virtual bool            drawPolygonBezier( sal_uInt32 /* nPoints */, const SalPoint* /* pPtAry */, const sal_uInt8* /* pFlgAry */ ) SAL_OVERRIDE
    {
        return false;
    }
    virtual bool            drawPolyPolygonBezier( sal_uInt32 /* nPoly */, const sal_uInt32* /* pPoints */, const SalPoint* const* /* pPtAry */, const sal_uInt8* const* /* pFlgAry */ ) SAL_OVERRIDE
    {
        return false;
    }
    virtual bool            drawPolyLine(
                                const ::basegfx::B2DPolygon&,
                                double /* fTransparency */,
                                const ::basegfx::B2DVector& /* rLineWidths */,
                                basegfx::B2DLineJoin,
                                com::sun::star::drawing::LineCap /* eLineCap */) SAL_OVERRIDE
    {
        return false;
    }

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void            copyArea( long /* nDestX */, long /* nDestY */, long /* nSrcX */, long /* nSrcY */, long /* nSrcWidth */,
                                     long /* nSrcHeight */, sal_uInt16 /* nFlags */ ) SAL_OVERRIDE {}

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void            copyBits( const SalTwoRect& /* rPosAry */, SalGraphics* /* pSrcGraphics */ ) SAL_OVERRIDE {}
    virtual void            drawBitmap( const SalTwoRect& /* rPosAry */, const SalBitmap& /* rSalBitmap */ ) SAL_OVERRIDE {}
    virtual void            drawBitmap( const SalTwoRect& /* rPosAry */,
                                        const SalBitmap& /* rSalBitmap */,
                                       SalColor /* nTransparentColor */ ) SAL_OVERRIDE {}
    virtual void            drawBitmap( const SalTwoRect& /* rPosAry */,
                                        const SalBitmap& /* rSalBitmap */,
                                       const SalBitmap& /* rTransparentBitmap */ ) SAL_OVERRIDE {}
    virtual void            drawMask( const SalTwoRect& /* rPosAry */,
                                      const SalBitmap& /* rSalBitmap */,
                                     SalColor /* nMaskColor */ ) SAL_OVERRIDE {}

    virtual SalBitmap*      getBitmap( long /* nX */, long /* nY */, long /* nWidth */, long /* nHeight */ ) SAL_OVERRIDE
    {
        return nullptr;
    }
    virtual SalColor        getPixel( long /* nX */, long /* nY */ ) SAL_OVERRIDE
    {
        return {};
    }

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void            invert( long /* nX */, long /* nY */, long /* nWidth */, long /* nHeight */, SalInvert /* nFlags */) SAL_OVERRIDE {}
    virtual void            invert( sal_uInt32 /* nPoints */, const SalPoint* /* pPtAry */, SalInvert /* nFlags */ ) SAL_OVERRIDE {}

    virtual bool            drawEPS( long /* nX */, long /* nY */, long /* nWidth */, long /* nHeight */, void* /* pPtr */, sal_uLong /* nSize */ ) SAL_OVERRIDE;

    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& /* rSourceBitmap */,
                                            const SalBitmap& /* rAlphaBitmap */ ) SAL_OVERRIDE
    {
        return false;
    }

    virtual bool            drawAlphaRect( long /* nX */, long /* nY */, long /* nWidth */,
                                           long /* nHeight */, sal_uInt8 /* nTransparency */ ) SAL_OVERRIDE
    {
        return false;
    }

    virtual bool            hitTestNativeControl( ControlType /* nType */, ControlPart /* nPart */, const Rectangle& /* rControlRegion */,
                                                  const Point& /* aPos */, bool& /* rIsInside */ ) SAL_OVERRIDE
    {
        return false;
    }
    virtual bool            drawNativeControl( ControlType /* nType */, ControlPart /* nPart */, const Rectangle& /* rControlRegion */,
                                               ControlState /* nState */, const ImplControlValue& /* aValue */,
                                               const OUString& /* aCaption */ ) SAL_OVERRIDE

    {
        return false;
    }
    virtual bool            getNativeControlRegion( ControlType /* nType */, ControlPart /* nPart */, const Rectangle& /* rControlRegion */, ControlState /* nState */,
                                                    const ImplControlValue& /* aValue */, const OUString& /* aCaption */,
                                                    Rectangle & /* rNativeBoundingRegion */, Rectangle & /* rNativeContentRegion */ ) SAL_OVERRIDE

    {
        return false;
    }
    // get device resolution
    virtual void            GetResolution( sal_Int32& /* rDPIX */, sal_Int32& /* rDPIY */ ) SAL_OVERRIDE;
    // get the depth of the device
    virtual sal_uInt16      GetBitCount() const SAL_OVERRIDE
    {
        return 0;
    }
    // get the width of the device
    virtual long            GetGraphicsWidth() const SAL_OVERRIDE
    {
        return 0L;
    }

    // set the clip region to empty
    virtual void            ResetClipRegion() SAL_OVERRIDE {}

    // set the line color to transparent (= don't draw lines)
    virtual void            SetLineColor() SAL_OVERRIDE {}
    // set the line color to a specific color
    virtual void            SetLineColor( SalColor /* nSalColor */ ) SAL_OVERRIDE {}
    // set the fill color to transparent (= don't fill)
    virtual void            SetFillColor() SAL_OVERRIDE {}
    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void            SetFillColor( SalColor /* nSalColor */ ) SAL_OVERRIDE {}
    // enable/disable XOR drawing
    virtual void            SetXORMode( bool /* bSet */, bool /* bInvertOnly */ ) SAL_OVERRIDE {}
    // set line color for raster operations
    virtual void            SetROPLineColor( SalROPColor /* nROPColor */ ) SAL_OVERRIDE {}
    // set fill color for raster operations
    virtual void            SetROPFillColor( SalROPColor /* nROPColor */ ) SAL_OVERRIDE {}
    // set the text color to a specific color
    virtual void            SetTextColor( SalColor /* nSalColor */ ) SAL_OVERRIDE {}
    // set the font
    virtual sal_uInt16      SetFont( FontSelectPattern*, int /* nFallbackLevel */ ) SAL_OVERRIDE
    {
        return 0;
    }
    // get the current font's metrics
    virtual void            GetFontMetric( ImplFontMetricData*, int /* nFallbackLevel */ ) SAL_OVERRIDE {}
    // get the repertoire of the current font
    virtual const ImplFontCharMapPtr
                            GetImplFontCharMap() const SAL_OVERRIDE
    {
        return nullptr;
    }
    virtual bool            GetFontCapabilities(vcl::FontCapabilities & /* rFontCapabilities */) const SAL_OVERRIDE
    {
        return false;
    }
    // graphics must fill supplied font list
    virtual void            GetDevFontList( PhysicalFontCollection* ) SAL_OVERRIDE {}
    // graphics must drop any cached font info
    virtual void            ClearDevFontCache() SAL_OVERRIDE {}
    virtual bool            AddTempDevFont( PhysicalFontCollection*, const OUString& /* rFileURL */, const OUString& /* rFontName */ ) SAL_OVERRIDE
    {
        return false;
    }
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
    virtual bool            CreateFontSubset( const OUString& /* rToFile */,
                                                  const PhysicalFontFace* /* pFont */,
                                                  sal_GlyphId* /* pGlyphIds */,
                                                  sal_uInt8* /* pEncoding */,
                                                  sal_Int32* /* pWidths */,
                                                  int /* nGlyphs */,
                                                  FontSubsetInfo& /* rInfo */ // out parameter
                                                  ) SAL_OVERRIDE
    {
        return false;
    }

    // GetFontEncodingVector: a method to get the encoding map Unicode
    // to font encoded character; this is only used for type1 fonts and
    // may return NULL in case of unknown encoding vector
    // if ppNonEncoded is set and non encoded characters (that is type1
    // glyphs with only a name) exist it is set to the corresponding
    // map for non encoded glyphs; the encoding vector contains -1
    // as encoding for these cases
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** /* ppNonEncoded */ ) SAL_OVERRIDE
    {
        return nullptr;
    }

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void*     GetEmbedFontData( const PhysicalFontFace*,
                                              const sal_Ucs* /* pUnicodes */,
                                              sal_Int32* /* pWidths */,
                                              FontSubsetInfo& /* rInfo */,
                                              long* /* pDataLen */ ) SAL_OVERRIDE
    {
        return nullptr;
    }
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* /* pData */, long /* nDataLen */ ) SAL_OVERRIDE {}

    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool /* bVertical */,
                                            Int32Vector& /* rWidths */,
                                           Ucs2UIntMap& /* rUnicodeEnc */ ) SAL_OVERRIDE {}

    virtual bool            GetGlyphBoundRect( sal_GlyphId, Rectangle& ) SAL_OVERRIDE
    {
        return false;
    }
    virtual bool            GetGlyphOutline( sal_GlyphId, basegfx::B2DPolyPolygon& ) SAL_OVERRIDE
    {
        return false;
    }

    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int /*nFallbackLevel */ ) SAL_OVERRIDE
    {
        return nullptr;
    }
    virtual void            DrawServerFontLayout( const ServerFontLayout& ) SAL_OVERRIDE {}
    virtual bool            supportsOperation( OutDevSupportType ) const SAL_OVERRIDE
    {
        return false;
    }

    // Query the platform layer for control support
    virtual bool            IsNativeControlSupported( ControlType /* nType */, ControlPart /* Part */ ) SAL_OVERRIDE
    {
        return false;
    }

    virtual SystemGraphicsData
                            GetGraphicsData() const SAL_OVERRIDE
    {
        return {};
    }
    virtual SystemFontData  GetSysFontData( int /* nFallbacklevel */ ) const SAL_OVERRIDE
    {
        return {};
    }
};

#endif // INCLUDED_VCL_INC_OPENGL_SALGDI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
