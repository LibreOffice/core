/*************************************************************************
 *
 *  $RCSfile: svgaction.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    void                    ImplWriteText( const Point& rPos, const String& rText, const long* pDXArray, long nWidth, const NMSP_RTL::OUString* pStyle = NULL );
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
    BOOL                    HasDoublePoints() const { return mbDoublePoints; }
};

#endif
