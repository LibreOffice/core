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

#ifndef _SVGACTION_HXX
#define _SVGACTION_HXX

#include "svgcom.hxx"

// --------------
// - FastString -
// --------------

class FastString
{
private:

    NMSP_RTL::OUString          maString;
    sal_Unicode*                mpBuffer;
    sal_uInt32                  mnBufLen;
    sal_uInt32                  mnCurLen;
    sal_uInt32                  mnBufInc;
    sal_uInt32                  mnPartPos;

public:

                                FastString( sal_uInt32 nInitLen = 2048, sal_uInt32 nIncrement = 2048 );
                                FastString( sal_Char* pBufferForBase64Encoding, sal_uInt32 nBufLen );
                                ~FastString();

    FastString&                 operator+=( const NMSP_RTL::OUString& rStr );

    const NMSP_RTL::OUString&   GetString() const;
    sal_Bool                    GetFirstPartString( const sal_uInt32 nPartLen, NMSP_RTL::OUString& rPartString );
    sal_Bool                    GetNextPartString( const sal_uInt32 nPartLen, NMSP_RTL::OUString& rPartString );

    sal_uInt32                  GetLength() const { return mnCurLen; }
    void                        Clear() { mnCurLen = 0, maString = NMSP_RTL::OUString(); }
};

// ----------------------
// - SVGAttributeWriter -
// ----------------------

class SVGActionWriter;

class SVGAttributeWriter
{
private:

    Font                    maCurFont;
    Color                   maCurLineColor;
    Color                   maCurFillColor;
    SVGActionWriter&        mrParent;
    SvXMLExport&            mrExport;
    SvXMLElementExport*     mpElemFont;
    SvXMLElementExport*     mpElemPaint;

                            SVGAttributeWriter();

public:

                            SVGAttributeWriter( SVGActionWriter& rParent, SvXMLExport& rExport );
    virtual                 ~SVGAttributeWriter();

    NMSP_RTL::OUString      GetFontStyle( const Font& rFont );
    NMSP_RTL::OUString      GetPaintStyle( const Color& rLineColor, const Color& rFillColor );

    void                    SetFontAttr( const Font& rFont );
    void                    SetPaintAttr( const Color& rLineColor, const Color& rFillColor );
};

// -------------------
// - SVGActionWriter -
// -------------------

class SVGAttributeWriter;
class SvXMLExport;
class GDIMetaFile;

class SVGActionWriter
{
    friend class SVGAttributeWriter;

private:

    Stack                   maContextStack;
    SvXMLExport&            mrExport;
    const GDIMetaFile&      mrMtf;
    SVGAttributeWriter*     mpContext;
    VirtualDevice*          mpVDev;
    MapMode                 maTargetMapMode;
    sal_Int32               mnCurClipId;
    sal_Bool                mbDestroyVDev;
    sal_Bool                mbPaintAttrChanged;
    sal_Bool                mbFontAttrChanged;
    sal_Bool                mbClipAttrChanged;
    sal_Bool                mbDoublePoints;

    SVGAttributeWriter*     ImplAcquireContext() { maContextStack.Push( mpContext = new SVGAttributeWriter( *this, mrExport ) ); return mpContext; }
    void                    ImplReleaseContext() { delete (SVGAttributeWriter*) maContextStack.Pop(); mpContext = (SVGAttributeWriter*) maContextStack.Top(); }

    long                    ImplMap( sal_Int32 nVal ) const;
    Point                   ImplMap( const Point& rPt ) const;
    Size                    ImplMap( const Size& rSz ) const;
    inline Rectangle        ImplMap( const Rectangle& rRect ) const { return Rectangle( ImplMap( rRect.TopLeft() ), ImplMap( rRect.GetSize() ) ); }

    void                    ImplWriteLine( const Point& rPt1, const Point& rPt2, const Color* pLineColor = NULL, const NMSP_RTL::OUString* pStyle = NULL );
    void                    ImplWriteRect( const Rectangle& rRect, long nRadX = 0, long nRadY = 0, const NMSP_RTL::OUString* pStyle = NULL );
    void                    ImplWriteEllipse( const Point& rCenter, long nRadX, long nRadY, const NMSP_RTL::OUString* pStyle = NULL );
    void                    ImplWritePolygon( const Polygon& rPoly, sal_Bool bLineOnly, const NMSP_RTL::OUString* pStyle = NULL );
    void                    ImplWritePolyPolygon( const PolyPolygon& rPolyPoly, sal_Bool bLineOnly, const NMSP_RTL::OUString* pStyle = NULL );
    void                    ImplWriteGradientEx( const PolyPolygon& rPolyPoly, const Gradient& rGradient, const NMSP_RTL::OUString* pStyle = NULL );
    void                    ImplWriteText( const Point& rPos, const String& rText, const sal_Int32* pDXArray, long nWidth, const NMSP_RTL::OUString* pStyle = NULL );
    void                    ImplWriteBmp( const BitmapEx& rBmpEx, const Point& rPt, const Size& rSz, const Point& rSrcPt, const Size& rSrcSz, const NMSP_RTL::OUString* pStyle = NULL );

    void                    ImplCheckFontAttributes();
    void                    ImplCheckPaintAttributes();

    void                    ImplWriteActions( const GDIMetaFile& rMtf, const NMSP_RTL::OUString* pStyle = NULL );

    sal_Int32               ImplGetNextClipId() { return mnCurClipId++; }

                            SVGActionWriter();

public:

    static NMSP_RTL::OUString GetValueString( sal_Int32 nVal, sal_Bool bDoublePoints );

public:

                            SVGActionWriter( SvXMLExport& rExport, const GDIMetaFile& rMtf,
                                             VirtualDevice* pParentVDev = NULL,
                                             sal_Bool bWriteDoublePoints = sal_False );
    virtual                 ~SVGActionWriter();

    const VirtualDevice&    GetVDev() const { return *mpVDev; }
    sal_Bool                    HasDoublePoints() const { return mbDoublePoints; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
