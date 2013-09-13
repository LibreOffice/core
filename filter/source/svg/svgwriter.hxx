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

#ifndef SVGWRITER_HXX
#define SVGWRITER_HXX

#include <stack>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graphictools.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/nmspmap.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
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
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/svg/XSVGWriter.hpp>

// -----------------------------------------------------------------------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::svg;
using namespace ::com::sun::star::xml::sax;

// -----------------------------------------------------------------------------

#define SVG_DTD_STRING            OUString( "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" )
#define SVG_TINY_DTD_STRING       OUString( "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG Tiny 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11-tiny.dtd\">" )

#define SVGWRITER_WRITE_NONE    0x00000000
#define SVGWRITER_WRITE_FILL    0x00000001
#define SVGWRITER_WRITE_TEXT    0x00000002
#define SVGWRITER_NO_SHAPE_COMMENTS 0x01000000

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

    basegfx::B2DLineJoin maLineJoin;
    com::sun::star::drawing::LineCap maLineCap;

                            SVGAttributeWriter();

    double                  ImplRound( double fVal, sal_Int32 nDecs = 3 );

public:

                            SVGAttributeWriter( SVGExport& rExport, SVGFontExport& rFontExport );
    virtual                 ~SVGAttributeWriter();

    void                    AddColorAttr( const char* pColorAttrName, const char* pColorOpacityAttrName, const Color& rColor );
    void                    AddGradientDef( const Rectangle& rObjRect,const Gradient& rGradient, OUString& rGradientId );
    void                    AddPaintAttr( const Color& rLineColor, const Color& rFillColor,
                                          const Rectangle* pObjBoundRect = NULL, const Gradient* pFillGradient = NULL );

    void                    SetFontAttr( const Font& rFont );
    void                    startFontSettings();
    void                    endFontSettings();
    void                    setFontFamily();

    static void             ImplGetColorStr( const Color& rColor, OUString& rColorStr );
};

struct SVGShapeDescriptor
{
    PolyPolygon                 maShapePolyPoly;
    Color                       maShapeFillColor;
    Color                       maShapeLineColor;
    sal_Int32                   mnStrokeWidth;
    SvtGraphicStroke::DashArray maDashArray;
    ::std::auto_ptr< Gradient > mapShapeGradient;
    OUString             maId;

    basegfx::B2DLineJoin                maLineJoin;
    com::sun::star::drawing::LineCap    maLineCap;

    // -------------------------------------------------------------------------

    SVGShapeDescriptor() :
        maShapeFillColor( Color( COL_TRANSPARENT ) ),
        maShapeLineColor( Color( COL_TRANSPARENT ) ),
        mnStrokeWidth( 0 ),
        maLineJoin(basegfx::B2DLINEJOIN_MITER), // miter is Svg 'stroke-linejoin' default
        maLineCap(com::sun::star::drawing::LineCap_BUTT) // butt is Svg 'stroke-linecap' default
    {
    }
};



class SVGAttributeWriter;
class SVGExport;
class GDIMetaFile;


// ---------------------------
// - BulletListItemInfo -
// ---------------------------
struct BulletListItemInfo
{
    long nFontSize;
    Color aColor;
    Point aPos;
    sal_Unicode cBulletChar;
};

// ---------------------------
// - OUStringHasher -
// ---------------------------

struct OUStringHasher
{
    size_t operator()( const OUString& oustr ) const { return static_cast< size_t >( oustr.hashCode() ); }
};


// -------------------
// - SVGTextWriter -
// -------------------
class SVGTextWriter
{
  public:
    typedef ::boost::unordered_map< OUString, BulletListItemInfo, OUStringHasher >         BulletListItemInfoMap;

  private:
    SVGExport&                                  mrExport;
    SVGAttributeWriter*                         mpContext;
    VirtualDevice*                              mpVDev;
    sal_Bool                                    mbIsTextShapeStarted;
    Reference<XText>                            mrTextShape;
    OUString                             msShapeId;
    Reference<XEnumeration>                     mrParagraphEnumeration;
    Reference<XTextContent>                     mrCurrentTextParagraph;
    Reference<XEnumeration>                     mrTextPortionEnumeration;
    Reference<XTextRange>                       mrCurrentTextPortion;
    const GDIMetaFile*                          mpTextEmbeddedBitmapMtf;
    MapMode*                                    mpTargetMapMode;
    SvXMLElementExport*                         mpTextShapeElem;
    SvXMLElementExport*                         mpTextParagraphElem;
    SvXMLElementExport*                         mpTextPositionElem;
    sal_Int32                                   mnLeftTextPortionLength;
    Point                                       maTextPos;
    long int                                    mnTextWidth;
    sal_Bool                                    mbPositioningNeeded;
    sal_Bool                                    mbIsNewListItem;
    sal_Int16                                   meNumberingType;
    sal_Unicode                                 mcBulletChar;
    BulletListItemInfoMap                       maBulletListItemMap;
    sal_Bool                                    mbIsListLevelStyleImage;
    sal_Bool                                    mbLineBreak;
    sal_Bool                                    mbIsURLField;
    OUString                             msUrl;
    OUString                             msHyperlinkIdList;
    sal_Bool                                    mbIsPlacehlolderShape;
    sal_Bool                                    mbIWS;
    Font                                        maCurrentFont;
    Font                                        maParentFont;

  public:
    SVGTextWriter( SVGExport& rExport );
    virtual ~SVGTextWriter();

    sal_Int32 setTextPosition( const GDIMetaFile& rMtf, sal_uLong& nCurAction );
    void setTextProperties( const GDIMetaFile& rMtf, sal_uLong nCurAction );
    void addFontAttributes( sal_Bool bIsTextContainer );

    sal_Bool createParagraphEnumeration();
    sal_Bool nextParagraph();
    sal_Bool nextTextPortion();

    sal_Bool isTextShapeStarted() { return mbIsTextShapeStarted; }
    void startTextShape();
    void endTextShape();
    void startTextParagraph();
    void endTextParagraph();
    void startTextPosition( sal_Bool bExportX = sal_True, sal_Bool bExportY = sal_True);
    void endTextPosition();
    void implExportHyperlinkIds();
    void implWriteBulletChars();
    template< typename MetaBitmapActionType >
    void writeBitmapPlaceholder( const MetaBitmapActionType* pAction );
    void implWriteEmbeddedBitmaps();
    void writeTextPortion( const Point& rPos, const OUString& rText,
                           sal_Bool bApplyMapping = sal_True );
    void implWriteTextPortion( const Point& rPos, const OUString& rText,
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

    void setTextShape( const Reference<XText>& rxText,
                       const GDIMetaFile* pTextEmbeddedBitmapMtf )
    {
        mrTextShape.set( rxText );
        mpTextEmbeddedBitmapMtf = pTextEmbeddedBitmapMtf;
    }

    const Reference<XText>& getTextShape() const
    {
        return mrTextShape;
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
    template< typename SubType >
    sal_Bool implGetTextPositionFromBitmap( const MetaAction* pAction, Point& raPos, sal_Bool& rbEmpty );

    void implRegisterInterface( const Reference< XInterface >& rxIf );
    const OUString & implGetValidIDFromInterface( const Reference< XInterface >& rxIf );


};

// -------------------
// - SVGActionWriter -
// -------------------

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
    void                    ImplAddLineAttr( const LineInfo &rAttrs,
                                             sal_Bool bApplyMapping = sal_True );
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
    void                    ImplWriteText( const Point& rPos, const OUString& rText, const sal_Int32* pDXArray, long nWidth, sal_Bool bApplyMapping = sal_True );
    void                    ImplWriteText( const Point& rPos, const OUString& rText, const sal_Int32* pDXArray, long nWidth, Color aTextColor, sal_Bool bApplyMapping );
    void                    ImplWriteBmp( const BitmapEx& rBmpEx, const Point& rPt, const Size& rSz, const Point& rSrcPt, const Size& rSrcSz,
                                          sal_Bool bApplyMapping = sal_True );

    void                    ImplCheckFontAttributes();
    void                    ImplCheckPaintAttributes();

    void                    ImplWriteActions( const GDIMetaFile& rMtf,
                                              sal_uInt32 nWriteFlags,
                                              const OUString* pElementId,
                                              const Reference< XShape >* pXShape = NULL,
                                              const GDIMetaFile* pTextEmbeddedBitmapMtf = NULL );

    Font                    ImplSetCorrectFontHeight() const;

public:

    static OUString  GetPathString( const PolyPolygon& rPolyPoly, sal_Bool bLine );
    static sal_uLong        GetChecksum( const MetaAction* pAction );

public:

                            SVGActionWriter( SVGExport& rExport, SVGFontExport& rFontExport );
    virtual                 ~SVGActionWriter();

    void                    WriteMetaFile( const Point& rPos100thmm,
                                           const Size& rSize100thmm,
                                           const GDIMetaFile& rMtf,
                                           sal_uInt32 nWriteFlags,
                                           const OUString* pElementId = NULL,
                                           const Reference< XShape >* pXShape = NULL,
                                           const GDIMetaFile* pTextEmbeddedBitmapMtf = NULL );
};

class SVGWriter : public cppu::WeakImplHelper1< XSVGWriter >
{
private:
    Reference< XComponentContext >                      mxContext;
    Sequence< com::sun::star::beans::PropertyValue >    maFilterData;
    SVGWriter();

public:
    explicit SVGWriter( const Sequence<Any>& args,
                        const Reference< XComponentContext >& rxCtx );
    virtual ~SVGWriter();

    // XSVGWriter
    virtual void SAL_CALL write( const Reference<XDocumentHandler>& rxDocHandler,
        const Sequence<sal_Int8>& rMtfSeq ) throw( RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
