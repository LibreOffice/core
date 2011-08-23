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

#ifndef SVGWRITER_HXX
#define SVGWRITER_HXX

#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <tools/stack.hxx>
#ifndef _SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif
#ifndef _GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif
#ifndef _METAACT_HXX
#include <vcl/metaact.hxx>
#endif
#ifndef _METAACT_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#include <xmloff/xmlexp.hxx>
#include <xmloff/nmspmap.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

// -----------------------------------------------------------------------------

#define _SVG_USE_NATIVE_TEXTDECORATION
#define _SVG_USE_TSPANS 1
#undef  _SVG_WRITE_EXTENTS
#define _SVG_EMBED_FONTS 1

// -----------------------------------------------------------------------------

#define NMSP_CPPU				cppu
#define NMSP_RTL				rtl
#define NMSP_UNO				com::sun::star::uno
#define NMSP_LANG				com::sun::star::lang
#define NMSP_SAX				com::sun::star::xml::sax
#define NMSP_SVG				com::sun::star::svg
#define NMSP_REGISTRY			com::sun::star::registry


#define REF( _def_Obj )			NMSP_UNO::Reference< _def_Obj >
#define SEQ( _def_Obj )			NMSP_UNO::Sequence< _def_Obj > 
#define B2UCONST( _def_pChar )	(NMSP_RTL::OUString(RTL_CONSTASCII_USTRINGPARAM(_def_pChar )))
#define SVG_DTD_STRING			B2UCONST( "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" )

#define SVGWRITER_WRITE_NONE	0x00000000
#define SVGWRITER_WRITE_FILL	0x00000001
#define SVGWRITER_WRITE_TEXT	0x00000002
#define SVGWRITER_WRITE_ALL		0xFFFFFFFF

// --------------
// - FastString -
// --------------

class FastString
{
private:

    sal_uInt32					mnBufLen;
    sal_uInt32					mnCurLen;
    sal_uInt32					mnBufInc;
    sal_Unicode*				mpBuffer;
    sal_uInt32					mnPartPos;
    rtl::OUString				maString;
                                
public:							
                                
                                FastString( sal_uInt32 nInitLen = 2048, sal_uInt32 nIncrement = 2048 );
                                FastString( sal_Char* pBufferForBase64Encoding, sal_uInt32 nBufLen );
                                ~FastString();
                                
    FastString&					operator+=( const ::rtl::OUString& rStr );
                        
    const ::rtl::OUString&		GetString() const;
    sal_Bool					GetFirstPartString( const sal_uInt32 nPartLen, ::rtl::OUString& rPartString );
    sal_Bool					GetNextPartString( const sal_uInt32 nPartLen, ::rtl::OUString& rPartString );

    sal_uInt32					GetLength() const { return mnCurLen; }
    void						Clear() { mnCurLen = 0, maString = ::rtl::OUString(); }
};

// ----------------------
// - SVGAttributeWriter -
// ----------------------

class SVGActionWriter;
class SVGFontExport;

class SVGAttributeWriter
{
private:

    Font					maCurFont;
    Color					maCurLineColor;
    Color					maCurFillColor;
    SvXMLExport&			mrExport;
    SVGFontExport&          mrFontExport;
    SvXMLElementExport*		mpElemFont;
    SvXMLElementExport*		mpElemPaint;

                            SVGAttributeWriter();

public:

                            SVGAttributeWriter( SvXMLExport& rExport, SVGFontExport& rFontExport );
    virtual					~SVGAttributeWriter();

    ::rtl::OUString			GetFontStyle( const Font& rFont );
    ::rtl::OUString			GetPaintStyle( const Color& rLineColor, const Color& rFillColor, const LineInfo* pLineInfo );

    void					SetFontAttr( const Font& rFont );
    void					SetPaintAttr( const Color& rLineColor, const Color& rFillColor, const LineInfo* pLineInfo = 0);
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

    SvXMLExport&			mrExport;
    SVGFontExport&          mrFontExport;
    SVGAttributeWriter*		mpContext;
    sal_Bool				mbClipAttrChanged;
    sal_Int32				mnCurClipId;
    Stack					maContextStack;
    VirtualDevice*			mpVDev;
    MapMode					maTargetMapMode;
    sal_Bool				mbDestroyVDev;
    sal_Bool				mbPaintAttrChanged;
    sal_Bool				mbFontAttrChanged;

    SVGAttributeWriter*		ImplAcquireContext() { maContextStack.Push( mpContext = new SVGAttributeWriter( mrExport, mrFontExport ) ); return mpContext; }
    void					ImplReleaseContext() { delete (SVGAttributeWriter*)	maContextStack.Pop(); mpContext = (SVGAttributeWriter*) maContextStack.Top(); }

    long					ImplMap( sal_Int32 nVal ) const; 
    Point					ImplMap( const Point& rPt ) const;
    Size					ImplMap( const Size& rSz ) const;
    LineInfo				ImplMap( const LineInfo& rLineInfo ) const;
    inline Rectangle		ImplMap( const Rectangle& rRect ) const { return Rectangle( ImplMap( rRect.TopLeft() ), ImplMap( rRect.GetSize() ) ); }

    void					ImplWriteLine( const Point& rPt1, const Point& rPt2, const Color* pLineColor = NULL, const ::rtl::OUString* pStyle = NULL );
    void					ImplWriteRect( const Rectangle& rRect, long nRadX = 0, long nRadY = 0, const ::rtl::OUString* pStyle = NULL );
    void					ImplWriteEllipse( const Point& rCenter, long nRadX, long nRadY, const ::rtl::OUString* pStyle = NULL );
    void					ImplWritePolyPolygon( const PolyPolygon& rPolyPoly, sal_Bool bLineOnly, const ::rtl::OUString* pStyle = NULL );
    void					ImplWriteGradientEx( const PolyPolygon& rPolyPoly, const Gradient& rGradient, const ::rtl::OUString* pStyle, sal_uInt32 nWriteFlags );
    void					ImplWriteText( const Point& rPos, const String& rText, const sal_Int32* pDXArray, long nWidth, const ::rtl::OUString* pStyle = NULL );
    void					ImplWriteBmp( const BitmapEx& rBmpEx, const Point& rPt, const Size& rSz, const Point& rSrcPt, const Size& rSrcSz, const ::rtl::OUString* pStyle = NULL );

    void					ImplCheckFontAttributes();
    void					ImplCheckPaintAttributes();
    
    void					ImplWriteActions( const GDIMetaFile& rMtf, const ::rtl::OUString* pStyle, sal_uInt32 nWriteFlags );
    sal_Int32				ImplGetNextClipId() { return mnCurClipId++; }

public:

    static ::rtl::OUString	GetValueString( sal_Int32 nVal );
    static ::rtl::OUString	GetPathString( const PolyPolygon& rPolyPoly, sal_Bool bLine );

public:

                            SVGActionWriter( SvXMLExport& rExport, SVGFontExport& rFontExport );
    virtual					~SVGActionWriter();

    void					WriteMetaFile( const Point& rPos100thmm, 
                                           const Size& rSize100thmm,
                                           const GDIMetaFile& rMtf, 
                                           sal_uInt32 nWriteFlags = SVGWRITER_WRITE_ALL );
};

#endif
