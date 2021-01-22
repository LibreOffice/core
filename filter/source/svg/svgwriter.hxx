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

#ifndef INCLUDED_FILTER_SOURCE_SVG_SVGWRITER_HXX
#define INCLUDED_FILTER_SOURCE_SVG_SVGWRITER_HXX

#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
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

#include <memory>
#include <stack>
#include <unordered_map>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::svg;
using namespace ::com::sun::star::xml::sax;

#define SVG_DTD_STRING          "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">"

#define SVGWRITER_WRITE_FILL        0x00000001
#define SVGWRITER_WRITE_TEXT        0x00000002
#define SVGWRITER_NO_SHAPE_COMMENTS 0x01000000

// This must match the same type definition in svgexport.hxx
typedef std::unordered_map< BitmapChecksum, std::unique_ptr< GDIMetaFile > > MetaBitmapActionMap;

struct SVGState
{
    vcl::Font                               aFont;
//  Color                                   aLineColor;
//  Color                                   aFillColor;
//  basegfx::B2DLineJoin                    aLineJoin;
//  com::sun::star::drawing::LineCap        aLineCap;
    sal_Int32                               nRegionClipPathId;

    SVGState()
        : aFont()
        , nRegionClipPathId( 0 )
    {}
};
// - PartialState -

struct PartialState
{
    PushFlags                           meFlags;
    ::std::unique_ptr<vcl::Font>        mupFont;
    sal_Int32                           mnRegionClipPathId;

    const vcl::Font&        getFont( const vcl::Font& rDefaultFont ) const
                                { return mupFont ? *mupFont : rDefaultFont; }

    void                    setFont( const vcl::Font& rFont )
                                { mupFont.reset( new vcl::Font(rFont) ); }

    PartialState()
        : meFlags( PushFlags::NONE )
        , mupFont()
        , mnRegionClipPathId( 0 )
    {}

    PartialState(PartialState&& aPartialState) noexcept
        : meFlags( aPartialState.meFlags )
        , mupFont( std::move( aPartialState.mupFont ) )
        , mnRegionClipPathId( aPartialState.mnRegionClipPathId )
    {
        aPartialState.meFlags = PushFlags::NONE;
        aPartialState.mnRegionClipPathId = 0;
    }
};


// - SVGContextHandler -

class SVGContextHandler
{
private:
    ::std::stack<PartialState> maStateStack;
    SVGState maCurrentState;

public:
    PushFlags getPushFlags() const;
    SVGState& getCurrentState();
    void pushState( PushFlags eFlags );
    void popState();
};


// - SVGAttributeWriter -

class SVGActionWriter;
class SVGExport;
class SVGFontExport;


class SVGAttributeWriter final
{
private:

    SVGExport&                              mrExport;
    SVGFontExport&                          mrFontExport;
    SVGState&                               mrCurrentState;
    std::unique_ptr<SvXMLElementExport>     mpElemFont;


    static double           ImplRound( double fVal );

public:

                            SVGAttributeWriter( SVGExport& rExport, SVGFontExport& rFontExport, SVGState& rCurState );
                            ~SVGAttributeWriter();

    void                    AddColorAttr( const char* pColorAttrName, const char* pColorOpacityAttrName, const Color& rColor );
    void                    AddGradientDef( const tools::Rectangle& rObjRect,const Gradient& rGradient, OUString& rGradientId );
    void                    AddPaintAttr( const Color& rLineColor, const Color& rFillColor,
                                          const tools::Rectangle* pObjBoundRect = nullptr, const Gradient* pFillGradient = nullptr );

    void                    SetFontAttr( const vcl::Font& rFont );
    void                    startFontSettings();
    void                    endFontSettings();
    void                    setFontFamily();

    static void             ImplGetColorStr( const Color& rColor, OUString& rColorStr );
};

struct SVGShapeDescriptor
{
    tools::PolyPolygon                  maShapePolyPoly;
    Color                               maShapeFillColor;
    Color                               maShapeLineColor;
    sal_Int32                           mnStrokeWidth;
    SvtGraphicStroke::DashArray         maDashArray;
    ::std::unique_ptr< Gradient >       mapShapeGradient;
    OUString                            maId;
    basegfx::B2DLineJoin        maLineJoin;
    css::drawing::LineCap       maLineCap;


    SVGShapeDescriptor() :
        maShapeFillColor( COL_TRANSPARENT ),
        maShapeLineColor( COL_TRANSPARENT ),
        mnStrokeWidth( 0 ),
        maLineJoin(basegfx::B2DLineJoin::Miter), // miter is Svg 'stroke-linejoin' default
        maLineCap(css::drawing::LineCap_BUTT) // butt is Svg 'stroke-linecap' default
    {
    }
};


class SVGAttributeWriter;
class SVGExport;
class GDIMetaFile;


struct BulletListItemInfo
{
    long nFontSize;
    Color aColor;
    Point aPos;
    sal_Unicode cBulletChar;
};


class SVGTextWriter final
{
  public:
    typedef std::unordered_map< OUString, BulletListItemInfo >         BulletListItemInfoMap;

  private:
    SVGExport&                                  mrExport;
    SVGAttributeWriter&                         mrAttributeWriter;
    VclPtr<VirtualDevice>                       mpVDev;
    bool                                        mbIsTextShapeStarted;
    Reference<XText>                            mrTextShape;
    OUString                                    msShapeId;
    Reference<XEnumeration>                     mrParagraphEnumeration;
    Reference<XTextContent>                     mrCurrentTextParagraph;
    Reference<XEnumeration>                     mrTextPortionEnumeration;
    Reference<XTextRange>                       mrCurrentTextPortion;
    const GDIMetaFile*                          mpTextEmbeddedBitmapMtf;
    MapMode*                                    mpTargetMapMode;
    std::unique_ptr<SvXMLElementExport>         mpTextShapeElem;
    std::unique_ptr<SvXMLElementExport>         mpTextParagraphElem;
    std::unique_ptr<SvXMLElementExport>         mpTextPositionElem;
    sal_Int32                                   mnLeftTextPortionLength;
    Point                                       maTextPos;
    long int                                    mnTextWidth;
    bool                                        mbPositioningNeeded;
    bool                                        mbIsNewListItem;
    sal_Int16                                   meNumberingType;
    sal_Unicode                                 mcBulletChar;
    BulletListItemInfoMap                       maBulletListItemMap;
    bool                                        mbIsListLevelStyleImage;
    bool                                        mbLineBreak;
    bool                                        mbIsURLField;
    OUString                                    msUrl;
    OUString                                    msHyperlinkIdList;
    OUString                                    msPageCount;
    OUString                                    msDateTimeType;
    bool                                        mbIsPlaceholderShape;
    static const bool                           mbIWS = false;
    vcl::Font                                   maCurrentFont;
    vcl::Font                                   maParentFont;

  public:
    explicit SVGTextWriter( SVGExport& rExport, SVGAttributeWriter& rAttributeWriter );
    ~SVGTextWriter();

    sal_Int32 setTextPosition( const GDIMetaFile& rMtf, sal_uLong& nCurAction );
    void setTextProperties( const GDIMetaFile& rMtf, sal_uLong nCurAction );
    void addFontAttributes( bool bIsTextContainer );

    void createParagraphEnumeration();
    bool nextParagraph();
    bool nextTextPortion();

    bool isTextShapeStarted() const { return mbIsTextShapeStarted; }
    void startTextShape();
    void endTextShape();
    void startTextParagraph();
    void endTextParagraph();
    void startTextPosition( bool bExportX = true, bool bExportY = true);
    void endTextPosition();
    void implExportHyperlinkIds();
    void implWriteBulletChars();
    template< typename MetaBitmapActionType >
    void writeBitmapPlaceholder( const MetaBitmapActionType* pAction );
    void implWriteEmbeddedBitmaps();
    void writeTextPortion( const Point& rPos, const OUString& rText );
    void implWriteTextPortion( const Point& rPos, const OUString& rText,
                               Color aTextColor );

    void setVirtualDevice( VirtualDevice* pVDev, MapMode& rTargetMapMode )
    {
        if( !pVDev )
            OSL_FAIL( "SVGTextWriter::setVirtualDevice: invalid virtual device." );
        mpVDev = pVDev;
        mpTargetMapMode = &rTargetMapMode;
    }

    void setTextShape( const Reference<XText>& rxText,
                       const GDIMetaFile* pTextEmbeddedBitmapMtf )
    {
        mrTextShape.set( rxText );
        mpTextEmbeddedBitmapMtf = pTextEmbeddedBitmapMtf;
    }

  private:
    void implMap( const Size& rSz, Size& rDstSz ) const;
    void implMap( const Point& rPt, Point& rDstPt ) const;
    void implSetCurrentFont();
    void implSetFontFamily();

    template< typename SubType >
    bool implGetTextPosition( const MetaAction* pAction, Point& raPos, bool& bEmpty );
    template< typename SubType >
    bool implGetTextPositionFromBitmap( const MetaAction* pAction, Point& raPos, bool& rbEmpty );

    void implRegisterInterface( const Reference< XInterface >& rxIf );
    const OUString & implGetValidIDFromInterface( const Reference< XInterface >& rxIf );
};


class SVGActionWriter final
{
private:

    sal_Int32                                   mnCurGradientId;
    sal_Int32                                   mnCurMaskId;
    sal_Int32                                   mnCurPatternId;
    sal_Int32                                   mnCurClipPathId;
    ::std::unique_ptr< SvXMLElementExport >     mpCurrentClipRegionElem;
    ::std::unique_ptr< SVGShapeDescriptor >     mapCurShape;
    SVGExport&                                  mrExport;
    SVGContextHandler                           maContextHandler;
    SVGState&                                   mrCurrentState;
    SVGAttributeWriter                          maAttributeWriter;
    SVGTextWriter                               maTextWriter;
    VclPtr<VirtualDevice>                       mpVDev;
    MapMode                                     maTargetMapMode;
    bool                                        mbClipAttrChanged;
    bool                                        mbIsPlaceholderShape;
    const MetaBitmapActionMap*                  mpEmbeddedBitmapsMap;


    long                    ImplMap( sal_Int32 nVal ) const;
    Point&                  ImplMap( const Point& rPt, Point& rDstPt ) const;
    Size&                   ImplMap( const Size& rSz, Size& rDstSz ) const;
    void                    ImplMap( const tools::Rectangle& rRect, tools::Rectangle& rDstRect ) const;
    tools::Polygon&         ImplMap( const tools::Polygon& rPoly, tools::Polygon& rDstPoly ) const;
    tools::PolyPolygon&     ImplMap( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rDstPolyPoly ) const;

    void                    ImplWriteLine( const Point& rPt1, const Point& rPt2, const Color* pLineColor = nullptr );
    void                    ImplWriteRect( const tools::Rectangle& rRect, long nRadX = 0, long nRadY = 0 );
    void                    ImplWriteEllipse( const Point& rCenter, long nRadX, long nRadY );
    void                    ImplWritePattern( const tools::PolyPolygon& rPolyPoly, const Hatch* pHatch, const Gradient* pGradient, sal_uInt32 nWriteFlags );
    void                    ImplAddLineAttr( const LineInfo &rAttrs );
    void                    ImplWritePolyPolygon( const tools::PolyPolygon& rPolyPoly, bool bLineOnly,
                                                  bool bApplyMapping = true );
    void                    ImplWriteShape( const SVGShapeDescriptor& rShape );
    void                    ImplCreateClipPathDef( const tools::PolyPolygon& rPolyPoly );
    void                    ImplStartClipRegion(sal_Int32 nClipPathId);
    void                    ImplEndClipRegion();
    void                    ImplWriteClipPath( const tools::PolyPolygon& rPolyPoly );
    void                    ImplWriteGradientEx( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient, sal_uInt32 nWriteFlags);
    void                    ImplWriteGradientLinear( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient );
    void                    ImplWriteGradientStop( const Color& rColor, double fOffset );
    static Color            ImplGetColorWithIntensity( const Color& rColor, sal_uInt16 nIntensity );
    static Color            ImplGetGradientColor( const Color& rStartColor, const Color& rEndColor, double fOffset );
    void                    ImplWriteMask( GDIMetaFile& rMtf, const Point& rDestPt, const Size& rDestSize, const Gradient& rGradient, sal_uInt32 nWriteFlags );
    void                    ImplWriteText( const Point& rPos, const OUString& rText, const long* pDXArray, long nWidth );
    void                    ImplWriteText( const Point& rPos, const OUString& rText, const long* pDXArray, long nWidth, Color aTextColor );
    void                    ImplWriteBmp( const BitmapEx& rBmpEx, const Point& rPt, const Size& rSz, const Point& rSrcPt, const Size& rSrcSz, const css::uno::Reference<css::drawing::XShape>* pShape);

    void                    ImplWriteActions( const GDIMetaFile& rMtf,
                                              sal_uInt32 nWriteFlags,
                                              const OUString* pElementId,
                                              const Reference< css::drawing::XShape >* pXShape = nullptr,
                                              const GDIMetaFile* pTextEmbeddedBitmapMtf = nullptr );

    vcl::Font               ImplSetCorrectFontHeight() const;

public:

    static OUString         GetPathString( const tools::PolyPolygon& rPolyPoly, bool bLine );
    static BitmapChecksum   GetChecksum( const MetaAction* pAction );

public:
                            SVGActionWriter( SVGExport& rExport, SVGFontExport& rFontExport );
                            ~SVGActionWriter();

    void                    WriteMetaFile( const Point& rPos100thmm,
                                           const Size& rSize100thmm,
                                           const GDIMetaFile& rMtf,
                                           sal_uInt32 nWriteFlags,
                                           const OUString* pElementId = nullptr,
                                           const Reference< css::drawing::XShape >* pXShape = nullptr,
                                           const GDIMetaFile* pTextEmbeddedBitmapMtf = nullptr );

    void                    SetEmbeddedBitmapRefs( const MetaBitmapActionMap* pEmbeddedBitmapsMap );
};


class SVGWriter : public cppu::WeakImplHelper< XSVGWriter >
{
private:
    Reference< XComponentContext >                      mxContext;
    Sequence< css::beans::PropertyValue >    maFilterData;

public:
    explicit SVGWriter( const Sequence<Any>& args,
                        const Reference< XComponentContext >& rxCtx );
    virtual ~SVGWriter() override;

    // XSVGWriter
    virtual void SAL_CALL write( const Reference<XDocumentHandler>& rxDocHandler,
                                 const Sequence<sal_Int8>& rMtfSeq ) override;
};

#endif // INCLUDED_FILTER_SOURCE_SVG_SVGWRITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
