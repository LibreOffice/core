/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#ifndef SVGWRITER_HXX
#define SVGWRITER_HXX

#include <stack>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/rendergraphicrasterizer.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/nmspmap.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>

// -----------------------------------------------------------------------------

#define NMSP_CPPU               cppu
#define NMSP_UNO                com::sun::star::uno
#define NMSP_LANG               com::sun::star::lang
#define NMSP_SAX                com::sun::star::xml::sax
#define NMSP_REGISTRY           com::sun::star::registry


#define REF( _def_Obj )           NMSP_UNO::Reference< _def_Obj >
#define SEQ( _def_Obj )           NMSP_UNO::Sequence< _def_Obj >
#define B2UCONST( _def_pChar )    (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(_def_pChar )))
#define SVG_DTD_STRING            B2UCONST( "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" )
#define SVG_TINY_DTD_STRING     B2UCONST( "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG Tiny 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11-tiny.dtd\">" )

#define SVGWRITER_WRITE_NONE    0x00000000
#define SVGWRITER_WRITE_FILL    0x00000001
#define SVGWRITER_WRITE_TEXT    0x00000002
#define SVGWRITER_NO_SHAPE_COMMENTS 0x01000000
#define SVGWRITER_WRITE_ALL     0xFFFFFFFF

// ----------------------
// - SVGAttributeWriter -
// ----------------------

class SVGActionWriter;
class SVGExport;
class SVGFontExport;

class SVGAttributeWriter
{
private:

    Font                       maCurFont;
    Color                      maCurLineColor;
    Color                      maCurFillColor;
    SVGExport&                 mrExport;
    SVGFontExport&             mrFontExport;
    SvXMLElementExport*        mpElemFont;
    SvXMLElementExport*        mpElemPaint;

                            SVGAttributeWriter();

    double                  ImplRound( double fVal, sal_Int32 nDecs = 3 );

public:

                            SVGAttributeWriter( SVGExport& rExport, SVGFontExport& rFontExport );
    virtual                 ~SVGAttributeWriter();

    ::rtl::OUString         GetFontStyle( const Font& rFont );
    ::rtl::OUString         GetPaintStyle( const Color& rLineColor, const Color& rFillColor, const LineInfo* pLineInfo );
    void                    AddColorAttr( const char* pColorAttrName, const char* pColorOpacityAttrName, const Color& rColor );
    void                    AddGradientDef( const Rectangle& rObjRect,const Gradient& rGradient, ::rtl::OUString& rGradientId );
    void                    AddPaintAttr( const Color& rLineColor, const Color& rFillColor,
                                          const Rectangle* pObjBoundRect = NULL, const Gradient* pFillGradient = NULL );

    void                    SetFontAttr( const Font& rFont );
    void                    startFontSettings();
    void                    endFontSettings();
    void                    setFontFamily();

    static void             ImplGetColorStr( const Color& rColor, ::rtl::OUString& rColorStr );
};

struct SVGShapeDescriptor
{
    PolyPolygon                 maShapePolyPoly;
    Color                       maShapeFillColor;
    Color                       maShapeLineColor;
    sal_Int32                   mnStrokeWidth;
    SvtGraphicStroke::DashArray maDashArray;
    ::std::auto_ptr< Gradient > mapShapeGradient;
    ::rtl::OUString             maId;

    // -------------------------------------------------------------------------

    SVGShapeDescriptor() :
        maShapeFillColor( Color( COL_TRANSPARENT ) ),
        maShapeLineColor( Color( COL_TRANSPARENT ) ),
        mnStrokeWidth( 0 )
    {
    }
};

// -------------------
// - SVGActionWriter -
// -------------------

class SVGAttributeWriter;
class GDIMetaFile;

// -------------------
// - SVGTextWriter -
// -------------------
class SVGTextWriter
{
  private:
    SVGExport&                                  mrExport;
    SVGFontExport&                              mrFontExport;
    SVGAttributeWriter*                         mpContext;
    VirtualDevice*                              mpVDev;
    MapMode*                                    mpTargetMapMode;
    SvXMLElementExport*                         mpTextShapeElem;
    SvXMLElementExport*                         mpTextParagraphElem;
    SvXMLElementExport*                         mpTextPositionElem;
    Point                                       maTextPos;
    long int                                    mnTextWidth;
    sal_Bool                                    mbIsPlacehlolderShape;
    sal_Bool                                    mbIWS;
    Font                                        maCurrentFont;
    Font                                        maParentFont;

  public:
    SVGTextWriter( SVGExport& rExport, SVGFontExport& rFontExport );
    virtual ~SVGTextWriter();

    sal_Int32 setTextPosition( const GDIMetaFile& rMtf, sal_uLong& nCurAction );
    void setTextProperties( const GDIMetaFile& rMtf, sal_uLong nCurAction );
    void addFontAttributes( sal_Bool bIsTextContainer );

    void startTextShape();
    void endTextShape();
    void startTextParagraph();
    void endTextParagraph();
    void startTextPosition( sal_Bool bExportX = sal_True, sal_Bool bExportY = sal_True);
    void endTextPosition();
    void writeTextPortion( const Point& rPos, const String& rText,
                           sal_Bool bApplyMapping = sal_True );
    void implWriteTextPortion( const Point& rPos, const String& rText,
                               Color aTextColor, sal_Bool bApplyMapping );

    void setVirtualDevice( VirtualDevice* pVDev, MapMode& rTargetMapMode )
    {
        if( !pVDev )
            OSL_FAIL( "SVGTextWriter::setVirtualDevice: invalid virtual device." );
        mpVDev = pVDev;
        mpTargetMapMode = &rTargetMapMode;
    }

    void setContext( SVGAttributeWriter* pContext )
    {
        mpContext = pContext;
    }

    void setPlaceholderShapeFlag( sal_Bool bState )
    {
        mbIsPlacehlolderShape = bState;
    }

  private:
    void implMap( const Size& rSz, Size& rDstSz ) const;
    void implMap( const Point& rPt, Point& rDstPt ) const;
    void implSetCurrentFont();
    void implSetFontFamily();
    template< typename SubType >
    sal_Bool implGetTextPosition( const MetaAction* pAction, Point& raPos, sal_Bool& bEmpty );

};


class SVGActionWriter
{
private:

    sal_Int32                                   mnCurGradientId;
    sal_Int32                                   mnCurMaskId;
    sal_Int32                                   mnCurPatternId;
    ::std::stack< SVGAttributeWriter* >         maContextStack;
    ::std::auto_ptr< SVGShapeDescriptor >       mapCurShape;
    SVGExport&                                  mrExport;
    SVGFontExport&                              mrFontExport;
    SVGAttributeWriter*                         mpContext;
    SVGTextWriter                               maTextWriter;
    VirtualDevice*                              mpVDev;
    MapMode                                     maTargetMapMode;
    sal_uInt32                                  mnInnerMtfCount;
    sal_Bool                                    mbDestroyVDev;
    sal_Bool                                    mbPaintAttrChanged;
    sal_Bool                                    mbFontAttrChanged;
    sal_Bool                                    mbClipAttrChanged;
    sal_Bool                                    mbIsPlacehlolderShape;


    SVGAttributeWriter*     ImplAcquireContext()
    {
        maContextStack.push( mpContext = new SVGAttributeWriter( mrExport, mrFontExport ) );
        maTextWriter.setContext( mpContext );
        return mpContext;
    }
    void                    ImplReleaseContext()
    {
        if (!maContextStack.empty())
        {
            delete maContextStack.top();
            maContextStack.pop();
        }
        mpContext = (maContextStack.empty() ? NULL : maContextStack.top());
        maTextWriter.setContext( mpContext );
    }

    long                    ImplMap( sal_Int32 nVal ) const;
    Point&                  ImplMap( const Point& rPt, Point& rDstPt ) const;
    Size&                   ImplMap( const Size& rSz, Size& rDstSz ) const;
    Rectangle&              ImplMap( const Rectangle& rRect, Rectangle& rDstRect ) const;
    Polygon&                ImplMap( const Polygon& rPoly, Polygon& rDstPoly ) const;
    PolyPolygon&            ImplMap( const PolyPolygon& rPolyPoly, PolyPolygon& rDstPolyPoly ) const;

    void                    ImplWriteLine( const Point& rPt1, const Point& rPt2, const Color* pLineColor = NULL,
                                           sal_Bool bApplyMapping = sal_True );
    void                    ImplWriteRect( const Rectangle& rRect, long nRadX = 0, long nRadY = 0,
                                           sal_Bool bApplyMapping = sal_True );
    void                    ImplWriteEllipse( const Point& rCenter, long nRadX, long nRadY,
                                              sal_Bool bApplyMapping = sal_True );
    void                    ImplWritePattern( const PolyPolygon& rPolyPoly, const Hatch* pHatch, const Gradient* pGradient, sal_uInt32 nWriteFlags );
    void                    ImplWritePolyPolygon( const PolyPolygon& rPolyPoly, sal_Bool bLineOnly,
                                                  sal_Bool bApplyMapping = sal_True );
    void                    ImplWriteShape( const SVGShapeDescriptor& rShape, sal_Bool bApplyMapping = sal_True );
    void                    ImplWriteGradientEx( const PolyPolygon& rPolyPoly, const Gradient& rGradient, sal_uInt32 nWriteFlags,
                                                 sal_Bool bApplyMapping = sal_True );
    void                    ImplWriteGradientLinear( const PolyPolygon& rPolyPoly, const Gradient& rGradient );
    void                    ImplWriteGradientStop( const Color& rColor, double fOffset );
    Color                   ImplGetColorWithIntensity( const Color& rColor, sal_uInt16 nIntensity );
    Color                   ImplGetGradientColor( const Color& rStartColor, const Color& rEndColor, double fOffset );
    void                    ImplWriteMask( GDIMetaFile& rMtf, const Point& rDestPt, const Size& rDestSize, const Gradient& rGradient, sal_uInt32 nWriteFlags );
    void                    ImplWriteText( const Point& rPos, const String& rText, const sal_Int32* pDXArray, long nWidth, sal_Bool bApplyMapping = sal_True );
    void                    ImplWriteText( const Point& rPos, const String& rText, const sal_Int32* pDXArray, long nWidth, Color aTextColor, sal_Bool bApplyMapping );
    void                    ImplWriteBmp( const BitmapEx& rBmpEx, const Point& rPt, const Size& rSz, const Point& rSrcPt, const Size& rSrcSz,
                                          sal_Bool bApplyMapping = sal_True );

    void                    ImplCheckFontAttributes();
    void                    ImplCheckPaintAttributes();

    void                    ImplWriteActions( const GDIMetaFile& rMtf, sal_uInt32 nWriteFlags, const ::rtl::OUString* pElementId );

    Font                    ImplSetCorrectFontHeight() const;

public:

    static ::rtl::OUString  GetPathString( const PolyPolygon& rPolyPoly, sal_Bool bLine );

public:

                            SVGActionWriter( SVGExport& rExport, SVGFontExport& rFontExport );
    virtual                 ~SVGActionWriter();

    void                    WriteMetaFile( const Point& rPos100thmm,
                                           const Size& rSize100thmm,
                                           const GDIMetaFile& rMtf,
                                           sal_uInt32 nWriteFlags,
                                           const ::rtl::OUString* pElementId = NULL );
    sal_Bool bIsTextShape;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
