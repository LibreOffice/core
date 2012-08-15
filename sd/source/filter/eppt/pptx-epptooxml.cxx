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

#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>
#include <stdio.h>
#include <oox/drawingml/chart/chartconverter.hxx>
#include <oox/token/tokens.hxx>
#include <oox/ole/vbaproject.hxx>
#include <epptooxml.hxx>
#include <epptdef.hxx>
#include <oox/export/shapes.hxx>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <sax/fshelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <filter/msfilter/escherex.hxx>
#include <tools/poly.hxx>

#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/AnimationValueType.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/animations/XAnimateSet.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/office/XAnnotationEnumeration.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <oox/export/utils.hxx>

#include "pptexanimations.hxx"

// presentation namespaces
#define PNMSS         FSNS( XML_xmlns, XML_a ), "http://schemas.openxmlformats.org/drawingml/2006/main", \
                      FSNS( XML_xmlns, XML_p ), "http://schemas.openxmlformats.org/presentationml/2006/main", \
                      FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships"

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::geometry;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::ppt;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::task::XStatusIndicator;
using ::com::sun::star::text::XSimpleText;
using ::sax_fastparser::FastSerializerHelper;
using ::sax_fastparser::FSHelperPtr;

void dump_pset(Reference< XPropertySet > rXPropSet);

#define IDS(x) (OString(#x " ") + OString::valueOf( mnShapeIdMax++ )).getStr()

namespace oox {
    using namespace drawingml;
    namespace core {

class PowerPointShapeExport : public ShapeExport
{
    PowerPointExport&   mrExport;
    PageType            mePageType;
    sal_Bool            mbMaster;
public:
    PowerPointShapeExport( FSHelperPtr pFS, ShapeHashMap* pShapeMap, PowerPointExport* pFB );
    void                SetMaster( sal_Bool bMaster );
    void                SetPageType( PageType ePageType );
    ShapeExport&        WriteNonVisualProperties( Reference< XShape > xShape );
    ShapeExport&        WriteTextShape( Reference< XShape > xShape );
    ShapeExport&        WriteUnknownShape( Reference< XShape > xShape );
    ShapeExport&        WritePlaceholderShape( Reference< XShape > xShape, PlaceholderType ePlaceholder );
    ShapeExport&        WritePageShape( Reference< XShape > xShape, PageType ePageType, sal_Bool bPresObj );

    // helper parts
    sal_Bool WritePlaceholder( Reference< XShape > xShape, PlaceholderType ePlaceholder, sal_Bool bMaster );
};

enum PPTXLayout {
    LAYOUT_BLANK,
    LAYOUT_TITLE_SLIDE,
    LAYOUT_TITLE_CONTENT,
    LAYOUT_TITLE_2CONTENT,
    LAYOUT_TITLE,
    LAYOUT_CENTERED_TEXT,
    LAYOUT_TITLE_2CONTENT_CONTENT,
    LAYOUT_TITLE_CONTENT_2CONTENT,
    LAYOUT_TITLE_2CONTENT_OVER_CONTENT,
    LAYOUT_TITLE_CONTENT_OVER_CONTENT,
    LAYOUT_TITLE_4CONTENT,
    LAYOUT_TITLE_6CONTENT,
    LAYOUT_SIZE
};

struct PPTXLayoutInfo {
    int nType;
    const char* sName;
    const char* sType;
};

static PPTXLayoutInfo aLayoutInfo[LAYOUT_SIZE] = {
    { 20, "Blank Slide", "blank" },
    { 0, "Title Slide", "tx" },
    { 1, "Title, Content", "obj" },
    { 3, "Title, 2 Content", "twoObj" },
    { 19, "Title Only", "titleOnly" },
    { 32, "Centered Text", "objOnly" },                       // not exactly, but close
    { 15, "Title, 2 Content and Content", "twoObjAndObj" },
    { 12, "Title Content and 2 Content", "objAndTwoObj" },
    { 16, "Title, 2 Content over Content", "twoObjOverTx" },      // not exactly, but close
    { 14, "Title, Content over Content", "objOverTx" },           // not exactly, but close
    { 18, "Title, 4 Content", "fourObj" },
    { 33, "Title, 6 Content", "blank" }                           // not defined => blank
};

int PowerPointExport::GetPPTXLayoutId( int nOffset )
{
    int nId = LAYOUT_BLANK;

    DBG(printf("GetPPTXLayoutId %d\n", nOffset));

    switch( nOffset ) {
        case 0:
            nId = LAYOUT_TITLE_SLIDE;
            break;
        case 1:
            nId = LAYOUT_TITLE_CONTENT;
            break;
        case 3:
            nId = LAYOUT_TITLE_2CONTENT;
            break;
        case 19:
            nId = LAYOUT_TITLE;
            break;
        case 15:
            nId = LAYOUT_TITLE_2CONTENT_CONTENT;
            break;
        case 12:
            nId = LAYOUT_TITLE_CONTENT_2CONTENT;
            break;
        case 16:
            nId = LAYOUT_TITLE_2CONTENT_OVER_CONTENT;
            break;
        case 14:
            nId = LAYOUT_TITLE_CONTENT_OVER_CONTENT;
            break;
        case 18:
            nId = LAYOUT_TITLE_4CONTENT;
            break;
        case 32:
            nId = LAYOUT_CENTERED_TEXT;
            break;
        case 33:
            nId = LAYOUT_TITLE_6CONTENT;
            break;
        case 20:
        default:
            nId = LAYOUT_BLANK;
            break;
    }

    return nId;
}

PowerPointShapeExport::PowerPointShapeExport( FSHelperPtr pFS, ShapeHashMap* pShapeMap, PowerPointExport* pFB )
        : ShapeExport( XML_p, pFS, pShapeMap, pFB )
    , mrExport( *pFB )
{
}

void PowerPointShapeExport::SetMaster( sal_Bool bMaster )
{
    mbMaster = bMaster;
}

void PowerPointShapeExport::SetPageType( PageType ePageType )
{
    mePageType = ePageType;
}

ShapeExport& PowerPointShapeExport::WriteNonVisualProperties( Reference< XShape > )
{
    GetFS()->singleElementNS( XML_p, XML_nvPr, FSEND );

    return *this;
}

ShapeExport& PowerPointShapeExport::WriteTextShape( Reference< XShape > xShape )
{
    OUString sShapeType = xShape->getShapeType();

    DBG(printf( "shape(text): %s\n", USS(sShapeType) ));

    if ( sShapeType == "com.sun.star.drawing.TextShape" )
    {
    ShapeExport::WriteTextShape( xShape );
    }
    else if ( sShapeType == "com.sun.star.presentation.DateTimeShape" )
    {
        if( !WritePlaceholder( xShape, DateAndTime, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if ( sShapeType == "com.sun.star.presentation.FooterShape" )
    {
        if( !WritePlaceholder( xShape, Footer, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if ( sShapeType == "com.sun.star.presentation.HeaderShape" )
    {
        if( !WritePlaceholder( xShape, Header, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if ( sShapeType == "com.sun.star.presentation.NotesShape" )
    {
        if( mePageType == NOTICE && mrExport.GetPresObj() )
            WritePlaceholderShape( xShape, Notes );
        else
            ShapeExport::WriteTextShape( xShape );
    }
    else if ( sShapeType == "com.sun.star.presentation.OutlinerShape" )
    {
        if( !WritePlaceholder( xShape, Outliner, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if ( sShapeType == "com.sun.star.presentation.SlideNumberShape" )
    {
        if( !WritePlaceholder( xShape, SlideNumber, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }
    else if ( sShapeType == "com.sun.star.presentation.TitleTextShape" )
    {
        if( !WritePlaceholder( xShape, Title, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }

    return *this;
}

ShapeExport& PowerPointShapeExport::WriteUnknownShape( Reference< XShape > xShape )
{
    OUString sShapeType = xShape->getShapeType();

    DBG(printf( "shape(unknown): %s\n", USS(sShapeType) ));

    if ( sShapeType == "com.sun.star.drawing.GroupShape" )
    {
        Reference< XIndexAccess > rXIndexAccess( xShape, UNO_QUERY );

        mrExport.EnterGroup( rXIndexAccess );
        DBG(printf( "enter group\n" ));
    }
    else if ( sShapeType == "com.sun.star.drawing.PageShape" )
    {
        WritePageShape( xShape, mePageType, mrExport.GetPresObj() );
    }
    else if ( sShapeType == "com.sun.star.presentation.SubtitleShape" )
    {
        if( !WritePlaceholder( xShape, Subtitle, mbMaster ) )
            ShapeExport::WriteTextShape( xShape );
    }

    return *this;
}

PowerPointExport::PowerPointExport( const Reference< XComponentContext > & rxCtxt  )
    : XmlFilterBase( rxCtxt ),
      PPTWriterBase(),
      mnLayoutFileIdMax( 1 ),
      mnSlideIdMax( 1 << 8 ),
      mnSlideMasterIdMax( 1 << 31 ),
      mnAnimationNodeIdMax( 1 )
{
    memset( mLayoutInfo, 0, sizeof(mLayoutInfo) );
}

PowerPointExport::~PowerPointExport()
{
}

bool PowerPointExport::importDocument() throw()
{
    return false;
}

bool PowerPointExport::exportDocument() throw()
{
    DrawingML::ResetCounters();
    maShapeMap.clear ();

    addRelation( US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" ), S( "ppt/presentation.xml" ) );

    mPresentationFS = openFragmentStreamWithSerializer( US( "ppt/presentation.xml" ),
                                                    US( "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml" ) );

    addRelation( mPresentationFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" ),
                 US( "theme/theme1.xml" ) );

    mPresentationFS->startElementNS( XML_p, XML_presentation, PNMSS, FSEND );

    mXModel.set( getModel(), UNO_QUERY );
    mXStatusIndicator.set( getStatusIndicator(), UNO_QUERY );

    rtl::OUString sBaseURI( "BaseURI");
    std::vector< PropertyValue > aProperties;
    PropertyValue aProperty;
    aProperty.Name = sBaseURI;
    aProperty.Value = makeAny( getFileUrl() );
    aProperties.push_back( aProperty );

    exportPPT(aProperties);

    mPresentationFS->singleElementNS( XML_p, XML_sldSz,
                                      XML_cx, IS( PPTtoEMU( maDestPageSize.Width ) ),
                                      XML_cy, IS( PPTtoEMU( maDestPageSize.Height ) ),
                                      FSEND );
    // for some reason if added before slides list it will not load the slides (alas with error reports) in mso
    mPresentationFS->singleElementNS( XML_p, XML_notesSz,
                                      XML_cx, IS( PPTtoEMU( maNotesPageSize.Width ) ),
                                      XML_cy, IS( PPTtoEMU( maNotesPageSize.Height ) ),
                                      FSEND );

    WriteAuthors();

    mPresentationFS->endElementNS( XML_p, XML_presentation );
    mPresentationFS.reset();

    commitStorage();

    maShapeMap.clear();
    maAuthors.clear();

    return true;
}

::oox::ole::VbaProject* PowerPointExport::implCreateVbaProject() const
{
    return new ::oox::ole::VbaProject( getComponentContext(), getModel(), "Impress" );
}

void PowerPointExport::ImplWriteBackground( FSHelperPtr pFS, Reference< XPropertySet > rXPropSet )
{
    FillStyle aFillStyle( FillStyle_NONE );
    if ( ImplGetPropertyValue( rXPropSet, S( "FillStyle" ) ) )
        mAny >>= aFillStyle;

    if( aFillStyle == FillStyle_NONE ||
        aFillStyle == FillStyle_GRADIENT ||
        aFillStyle == FillStyle_HATCH )
        return;

    pFS->startElementNS( XML_p, XML_bg, FSEND );
    pFS->startElementNS( XML_p, XML_bgPr, FSEND );

    PowerPointShapeExport( pFS, &maShapeMap, this ).WriteFill( rXPropSet );

    pFS->endElementNS( XML_p, XML_bgPr );
    pFS->endElementNS( XML_p, XML_bg );
}

#define MINIMAL_SPTREE "<p:spTree>\
      <p:nvGrpSpPr>\
        <p:cNvPr id=\"1\" name=\"\"/>\
        <p:cNvGrpSpPr/>\
        <p:nvPr/>\
      </p:nvGrpSpPr>\
      <p:grpSpPr>\
        <a:xfrm>\
          <a:off x=\"0\" y=\"0\"/>\
          <a:ext cx=\"0\" cy=\"0\"/>\
          <a:chOff x=\"0\" y=\"0\"/>\
          <a:chExt cx=\"0\" cy=\"0\"/>\
        </a:xfrm>\
      </p:grpSpPr>\
    </p:spTree>"

#define MAIN_GROUP \
     "<p:nvGrpSpPr>\
        <p:cNvPr id=\"1\" name=\"\"/>\
        <p:cNvGrpSpPr/>\
        <p:nvPr/>\
      </p:nvGrpSpPr>\
      <p:grpSpPr>\
        <a:xfrm>\
          <a:off x=\"0\" y=\"0\"/>\
          <a:ext cx=\"0\" cy=\"0\"/>\
          <a:chOff x=\"0\" y=\"0\"/>\
          <a:chExt cx=\"0\" cy=\"0\"/>\
        </a:xfrm>\
      </p:grpSpPr>"

#define GETA(propName) \
    ImplGetPropertyValue( mXPagePropSet, rtl::OUString( #propName ) )

#define GET(variable, propName) \
    if ( GETA(propName) ) \
        mAny >>= variable;

const char* PowerPointExport::GetSideDirection( sal_uInt8 nDirection )
{
    const char* pDirection = NULL;

    switch( nDirection ) {
    case 0:
        pDirection = "r";
        break;
    case 1:
        pDirection = "d";
        break;
    case 2:
        pDirection = "l";
        break;
    case 3:
        pDirection = "u";
        break;
    }

    return pDirection;
}

const char* PowerPointExport::GetCornerDirection( sal_uInt8 nDirection )
{
    const char* pDirection = NULL;

    switch( nDirection ) {
    case 4:
        pDirection = "rd";
        break;
    case 5:
        pDirection = "ld";
        break;
    case 6:
        pDirection = "ru";
        break;
    case 7:
        pDirection = "lu";
        break;
    }

    return pDirection;
}

const char* PowerPointExport::Get8Direction( sal_uInt8 nDirection )
{
    const char* pDirection = GetSideDirection( nDirection );

    if( !pDirection )
    pDirection = GetCornerDirection( nDirection );

    return pDirection;
}

void PowerPointExport::WriteTransition( FSHelperPtr pFS )
{
    FadeEffect eFadeEffect = FadeEffect_NONE;
    GET( eFadeEffect, Effect );

    OSL_TRACE("fade effect %d", eFadeEffect);

    sal_Int16 nTransitionType = 0, nTransitionSubtype = 0;
    sal_Int8 nPPTTransitionType = 0;
    sal_uInt8 nDirection = 0;

    if( GETA( TransitionType ) && ( mAny >>= nTransitionType ) &&
    GETA( TransitionSubtype ) && ( mAny >>= nTransitionSubtype ) )
    nPPTTransitionType = GetTransition( nTransitionType, nTransitionSubtype, eFadeEffect, nDirection );

    if( !nPPTTransitionType && eFadeEffect != FadeEffect_NONE )
    nPPTTransitionType = GetTransition( eFadeEffect, nDirection );

    if( nPPTTransitionType ) {
    AnimationSpeed animationSpeed = AnimationSpeed_MEDIUM;
    const char* speed = NULL;
    sal_Int32 advanceTiming = -1;
    sal_Int32 changeType = 0;

    if( GETA( Speed ) ) {
        mAny >>= animationSpeed;

        switch( animationSpeed ) {
        default:
        case AnimationSpeed_MEDIUM:
            speed = "med";
            break;
        case AnimationSpeed_SLOW:
            speed = "slow";
            break;
        case AnimationSpeed_FAST:
            break;
        }
    }

    if( GETA( Change ) )
        mAny >>= changeType;

    // 1 means automatic, 2 half automatic - not sure what it means - at least I don't see it in UI
    if( changeType == 1 && GETA( Duration ) )
        mAny >>= advanceTiming;

    pFS->startElementNS( XML_p, XML_transition,
                 XML_spd, speed,
                 XML_advTm, advanceTiming != -1 ? I32S( advanceTiming*1000 ) : NULL,
                 FSEND );

    sal_Int32 nTransition = 0;
    const char* pDirection = NULL;
    const char* pOrientation = NULL;
    const char* pThruBlk = NULL;
    const char* pSpokes = NULL;
    char pSpokesTmp[2] = "0";

    switch( nPPTTransitionType ) {
        case PPT_TRANSITION_TYPE_BLINDS:
        nTransition = XML_blinds;
        pDirection = ( nDirection == 0) ? "vert" : "horz";
        break;
        case PPT_TRANSITION_TYPE_CHECKER:
        nTransition = XML_checker;
        pDirection = ( nDirection == 1) ? "vert" : "horz";
        break;
        case PPT_TRANSITION_TYPE_CIRCLE:
        nTransition = XML_circle;
        break;
        case PPT_TRANSITION_TYPE_COMB:
        nTransition = XML_comb;
        pDirection = ( nDirection == 1) ? "vert" : "horz";
        break;
        case PPT_TRANSITION_TYPE_COVER:
        nTransition = XML_cover;
        pDirection = Get8Direction( nDirection );
        break;
        case PPT_TRANSITION_TYPE_DIAMOND:
        nTransition = XML_diamond;
        break;
        case PPT_TRANSITION_TYPE_DISSOLVE:
        nTransition = XML_dissolve;
        break;
        case PPT_TRANSITION_TYPE_FADE:
        nTransition = XML_fade;
        pThruBlk = "true";
        break;
        case PPT_TRANSITION_TYPE_SMOOTHFADE:
        nTransition = XML_fade;
        break;
        case PPT_TRANSITION_TYPE_NEWSFLASH:
        nTransition = XML_newsflash;
        break;
        case PPT_TRANSITION_TYPE_PLUS:
        nTransition = XML_plus;
        break;
        case PPT_TRANSITION_TYPE_PULL:
        nTransition = XML_pull;
        pDirection = Get8Direction( nDirection );
        break;
        case PPT_TRANSITION_TYPE_PUSH:
        nTransition = XML_push;
        pDirection = GetSideDirection( nDirection );
        break;
        case PPT_TRANSITION_TYPE_RANDOM:
        nTransition = XML_random;
        break;
        case PPT_TRANSITION_TYPE_RANDOM_BARS:
        nTransition = XML_randomBar;
        pDirection = ( nDirection == 1) ? "vert" : "horz";
        break;
        case PPT_TRANSITION_TYPE_SPLIT:
        nTransition = XML_split;
        pDirection = ( nDirection & 1) ? "in" : "out";
        pOrientation = ( nDirection < 2) ? "horz" : "vert";
        break;
        case PPT_TRANSITION_TYPE_STRIPS:
        nTransition = XML_strips;
        pDirection = GetCornerDirection( nDirection );
        break;
        case PPT_TRANSITION_TYPE_WEDGE:
        nTransition = XML_wedge;
        break;
        case PPT_TRANSITION_TYPE_WHEEL:
        nTransition = XML_wheel;
        if( nDirection != 4 && nDirection <= 9 ) {
            pSpokesTmp[0] = '0' + nDirection;
            pSpokes = pSpokesTmp;
        }
        break;
        case PPT_TRANSITION_TYPE_WIPE:
        nTransition = XML_wipe;
        pDirection = GetSideDirection( nDirection );
        break;
        case PPT_TRANSITION_TYPE_ZOOM:
        nTransition = XML_zoom;
        pDirection = ( nDirection == 1) ? "in" : "out";
        break;
        case PPT_TRANSITION_TYPE_NONE:
        default:
        nTransition = 0;
    }

    if( nTransition )
        pFS->singleElementNS( XML_p, nTransition,
                  XML_dir, pDirection,
                  XML_orient, pOrientation,
                  XML_spokes, pSpokes,
                  XML_thruBlk, pThruBlk,
                  FSEND );

    pFS->endElementNS( XML_p, XML_transition );
    }
}

void PowerPointExport::WriteAnimationProperty( FSHelperPtr pFS, const Any& rAny )
{
    if( !rAny.hasValue() )
    return;

    switch( rAny.getValueType().getTypeClass() ) {
    case TypeClass_STRING:
        pFS->singleElementNS( XML_p, XML_strVal,
                  XML_val, USS( *static_cast< const OUString* >( rAny.getValue() ) ),
                  FSEND );
        break;
    default:
        break;
    }
}

void PowerPointExport::WriteAnimateValues( FSHelperPtr pFS, const Reference< XAnimate >& rXAnimate )
{
    const Sequence< double > aKeyTimes = rXAnimate->getKeyTimes();
    if( aKeyTimes.getLength() <= 0 )
    return;
    const Sequence< Any > aValues = rXAnimate->getValues();
    const OUString& sFormula = rXAnimate->getFormula();
    const OUString& rAttributeName = rXAnimate->getAttributeName();

    DBG(printf("animate values, formula: %s\n", USS( sFormula )));

    pFS->startElementNS( XML_p, XML_tavLst, FSEND );

    for( int i = 0; i < aKeyTimes.getLength(); i++ ) {
    DBG(printf("animate value %d: %f\n", i, aKeyTimes[ i ]));
    if( aValues[ i ].hasValue() ) {
        pFS->startElementNS( XML_p, XML_tav,
                 XML_fmla, sFormula.isEmpty() ? NULL : USS( sFormula ),
                 XML_tm, I32S( ( sal_Int32 )( aKeyTimes[ i ]*100000.0 ) ),
                 FSEND );
        pFS->startElementNS( XML_p, XML_val, FSEND );
        ValuePair aPair;
        if( aValues[ i ] >>= aPair ) {
        WriteAnimationProperty( pFS, AnimationExporter::convertAnimateValue( aPair.First, rAttributeName ) );
        WriteAnimationProperty( pFS, AnimationExporter::convertAnimateValue( aPair.Second, rAttributeName ) );
        } else
        WriteAnimationProperty( pFS, AnimationExporter::convertAnimateValue( aValues[ i ], rAttributeName ) );

        pFS->endElementNS( XML_p, XML_val );
        pFS->endElementNS( XML_p, XML_tav );
    }
    }

    pFS->endElementNS( XML_p, XML_tavLst );
}

void PowerPointExport::WriteAnimateTo( FSHelperPtr pFS, Any aValue, const OUString& rAttributeName )
{
    if( !aValue.hasValue() )
    return;

    DBG(printf("to attribute name: %s\n", USS( rAttributeName )));

    pFS->startElementNS( XML_p, XML_to, FSEND );

    WriteAnimationProperty( pFS, AnimationExporter::convertAnimateValue( aValue, rAttributeName ) );

    pFS->endElementNS( XML_p, XML_to );
}

void PowerPointExport::WriteAnimationAttributeName( FSHelperPtr pFS, const OUString& rAttributeName )
{
    if( rAttributeName.isEmpty() )
    return;

    pFS->startElementNS( XML_p, XML_attrNameLst, FSEND );

    DBG(printf("write attribute name: %s\n", USS( rAttributeName )));

    const char* sAttributeName = NULL;
    if ( rAttributeName == "Visibility" ) {
    sAttributeName = "style.visibility";
    } else if ( rAttributeName == "X" ) {
    sAttributeName = "ppt_x";
    } else if ( rAttributeName == "Y" ) {
    sAttributeName = "ppt_y";
    }

    pFS->startElementNS( XML_p, XML_attrName, FSEND );
    pFS->writeEscaped( sAttributeName );
    pFS->endElementNS( XML_p, XML_attrName );

    pFS->endElementNS( XML_p, XML_attrNameLst );
}

void PowerPointExport::WriteAnimationTarget( FSHelperPtr pFS, Any aTarget )
{
    sal_Int32 nBegin = -1, nEnd = -1;
    sal_Bool bParagraphTarget;
    Reference< XShape > rXShape = AnimationExporter::getTargetElementShape( aTarget, nBegin, nEnd, bParagraphTarget );

    if( rXShape.is() ) {
    pFS->startElementNS( XML_p, XML_tgtEl, FSEND );
    pFS->startElementNS( XML_p, XML_spTgt,
                 XML_spid, I32S( ShapeExport::GetShapeID( rXShape, &maShapeMap ) ),
                 FSEND );
    if( bParagraphTarget ) {
        pFS->startElementNS( XML_p, XML_txEl, FSEND );
        pFS->singleElementNS( XML_p, XML_pRg,
                  XML_st, I32S( nBegin ),
                  XML_end, I32S( nEnd ),
                  FSEND );
        pFS->endElementNS( XML_p, XML_txEl );
    }
    pFS->endElementNS( XML_p, XML_spTgt );
    pFS->endElementNS( XML_p, XML_tgtEl );
    }
}

void PowerPointExport::WriteAnimationNodeAnimate( FSHelperPtr pFS, const Reference< XAnimationNode >& rXNode, sal_Int32 nXmlNodeType, sal_Bool bMainSeqChild )
{
    Reference< XAnimate > rXAnimate( rXNode, UNO_QUERY );
    if( !rXAnimate.is() )
    return;

    const char* pCalcMode = NULL;
    const char* pValueType = NULL;
    sal_Bool bSimple = ( nXmlNodeType != XML_anim );

    if( !bSimple ) {
    switch( rXAnimate->getCalcMode() ) {
        case AnimationCalcMode::DISCRETE:
        pCalcMode = "discrete";
        break;
        case AnimationCalcMode::LINEAR:
        pCalcMode = "lin";
        break;
    }

    switch( AnimationExporter::GetValueTypeForAttributeName( rXAnimate->getAttributeName() ) ) {
        case AnimationValueType::STRING:
        pValueType = "str";
        break;
        case AnimationValueType::NUMBER:
        pValueType = "num";
        break;
        case AnimationValueType::COLOR:
        pValueType = "clr";
        break;
    }
    }

    pFS->startElementNS( XML_p, nXmlNodeType,
             XML_calcmode, pCalcMode,
             XML_valueType, pValueType,
             FSEND );
    WriteAnimationNodeAnimateInside( pFS, rXNode, bMainSeqChild, bSimple );
    pFS->endElementNS( XML_p, nXmlNodeType );
}

void PowerPointExport::WriteAnimationNodeAnimateInside( FSHelperPtr pFS, const Reference< XAnimationNode >& rXNode, sal_Bool bMainSeqChild, sal_Bool bSimple )
{
    Reference< XAnimate > rXAnimate( rXNode, UNO_QUERY );
    if( !rXAnimate.is() )
    return;

    const char* pAdditive = NULL;

    if( !bSimple ) {
    switch( rXAnimate->getAdditive() ) {
        case AnimationAdditiveMode::BASE:
        pAdditive = "base";
        break;
        case AnimationAdditiveMode::SUM:
        pAdditive = "sum";
        break;
        case AnimationAdditiveMode::REPLACE:
        pAdditive = "repl";
        break;
        case AnimationAdditiveMode::MULTIPLY:
        pAdditive = "mult";
        break;
        case AnimationAdditiveMode::NONE:
        pAdditive = "none";
        break;
    }
    }

    pFS->startElementNS( XML_p, XML_cBhvr,
             XML_additive, pAdditive,
             FSEND );
    WriteAnimationNodeCommonPropsStart( pFS, rXNode, sal_True, bMainSeqChild );
    WriteAnimationTarget( pFS, rXAnimate->getTarget() );
    WriteAnimationAttributeName( pFS, rXAnimate->getAttributeName() );
    pFS->endElementNS( XML_p, XML_cBhvr );
    WriteAnimateValues( pFS, rXAnimate );
    WriteAnimateTo( pFS, rXAnimate->getTo(), rXAnimate->getAttributeName() );
}

void PowerPointExport::WriteAnimationCondition( FSHelperPtr pFS, const char* pDelay, const char* pEvent, double fDelay, sal_Bool bHasFDelay )
{
    if( bHasFDelay || pDelay || pEvent ) {
    if( !pEvent )
        pFS->singleElementNS( XML_p, XML_cond,
                  XML_delay, bHasFDelay ? I64S( (sal_uInt32) (fDelay*1000.0) ) : pDelay,
                  FSEND );
    else {
        pFS->startElementNS( XML_p, XML_cond,
                 XML_delay, bHasFDelay ? I64S( (sal_uInt32) (fDelay*1000.0) ) : pDelay,
                 XML_evt, pEvent,
                 FSEND );

        pFS->startElementNS( XML_p, XML_tgtEl, FSEND );
        pFS->singleElementNS( XML_p, XML_sldTgt, FSEND );
        pFS->endElementNS( XML_p, XML_tgtEl );

        pFS->endElementNS( XML_p, XML_cond );
    }
    }
}

void PowerPointExport::WriteAnimationCondition( FSHelperPtr pFS, Any& rAny, sal_Bool bWriteEvent, sal_Bool bMainSeqChild )
{
    sal_Bool bHasFDelay = sal_False;
    double fDelay = 0;
    Timing eTiming;
    Event aEvent;
    const char* pDelay = NULL;
    const char* pEvent = NULL;

    if( rAny >>= fDelay )
    bHasFDelay = sal_True;
    else if( rAny >>= eTiming ) {
    if( eTiming == Timing_INDEFINITE )
        pDelay = "indefinite";
    } else if( rAny >>= aEvent ) {
    // TODO
    DBG(printf ("animation condition event: TODO\n"));
    DBG(printf ("event offset has value: %d triger: %d source has value: %d\n", aEvent.Offset.hasValue(), aEvent.Trigger, aEvent.Source.hasValue()));

    if( !bWriteEvent && aEvent.Trigger == EventTrigger::ON_NEXT && bMainSeqChild )
        pDelay = "indefinite";
    else if( bWriteEvent ) {
        switch( aEvent.Trigger ) {
        case EventTrigger::ON_NEXT:
            pEvent = "onNext";
            break;
        case EventTrigger::ON_PREV:
            pEvent = "onPrev";
            break;
        case EventTrigger::BEGIN_EVENT:
            pEvent = "begin";
            break;
        case EventTrigger::END_EVENT:
            pEvent = "end";
            break;
        case EventTrigger::ON_BEGIN:
            pEvent = "onBegin";
            break;
        case EventTrigger::ON_END:
            pEvent = "onEnd";
            break;
        case EventTrigger::ON_CLICK:
            pEvent = "onClick";
            break;
        case EventTrigger::ON_DBL_CLICK:
            pEvent = "onDblClick";
            break;
        case EventTrigger::ON_STOP_AUDIO:
            pEvent = "onStopAudio";
            break;
        case EventTrigger::ON_MOUSE_ENTER:
            pEvent = "onMouseOver";   // not exact?
            break;
        case EventTrigger::ON_MOUSE_LEAVE:
            pEvent = "onMouseOut";
            break;
        }
    }

    if( aEvent.Offset >>= fDelay ) {
        bHasFDelay = sal_True;
        DBG(printf ("event offset: %f\n", fDelay));
    } else if( aEvent.Offset >>= eTiming ) {
        if( eTiming == Timing_INDEFINITE )
        pDelay = "indefinite";
        DBG(printf ("event offset timing: %d\n", eTiming));
    }
    }

    WriteAnimationCondition( pFS, pDelay, pEvent, fDelay, bHasFDelay );
}

void PowerPointExport::WriteAnimationNodeCommonPropsStart( FSHelperPtr pFS, const Reference< XAnimationNode >& rXNode, sal_Bool bSingle, sal_Bool bMainSeqChild )
{
    const char* pDuration = NULL;
    const char* pRestart = NULL;
    const char* pNodeType = NULL;
    const char* pPresetClass = NULL;
    const char* pFill = NULL;
    double fDuration = 0;
    Any aAny;

    aAny = rXNode->getDuration();
    if( aAny.hasValue() ) {
    Timing eTiming;

    if( aAny >>= eTiming ) {
        if( eTiming == Timing_INDEFINITE )
        pDuration = "indefinite";
    } else
        aAny >>= fDuration;
    }

    switch( rXNode->getRestart() ) {
    case AnimationRestart::ALWAYS:
        pRestart = "always";
        break;
    case AnimationRestart::WHEN_NOT_ACTIVE:
        pRestart = "whenNotActive";
        break;
    case AnimationRestart::NEVER:
        pRestart = "never";
        break;
    }

    const Sequence< NamedValue > aUserData = rXNode->getUserData();
    const Any* pAny[ DFF_ANIM_PROPERTY_ID_COUNT ];
    AnimationExporter::GetUserData( aUserData, pAny, sizeof( pAny ) );

    sal_Int16 nType = 0;
    if( pAny[ DFF_ANIM_NODE_TYPE ] && ( *pAny[ DFF_ANIM_NODE_TYPE ] >>= nType ) ) {
    switch( nType ) {
        case EffectNodeType::TIMING_ROOT:
        pNodeType = "tmRoot";
        if( !pDuration )
            pDuration = "indefinite";
        if( !pRestart )
            pRestart = "never";
        break;
        case EffectNodeType::MAIN_SEQUENCE:
        pNodeType = "mainSeq";
        break;
        case EffectNodeType::ON_CLICK:
        pNodeType = "clickEffect";
        break;
        case EffectNodeType::AFTER_PREVIOUS:
        pNodeType = "afterEffect";
        break;
        case EffectNodeType::WITH_PREVIOUS:
        pNodeType = "withEffect";
        break;
        case EffectNodeType::INTERACTIVE_SEQUENCE:
        pNodeType = "interactiveSeq";
        break;
    }
    }

    sal_uInt32 nPresetClass = DFF_ANIM_PRESS_CLASS_USER_DEFINED;
    if ( pAny[ DFF_ANIM_PRESET_CLASS ] ) {
    if ( *pAny[ DFF_ANIM_PRESET_CLASS ] >>= nPresetClass ) {
        switch( nPresetClass ) {
        case EffectPresetClass::ENTRANCE:
            pPresetClass = "entr";
            break;
        case EffectPresetClass::EXIT:
            pPresetClass = "exit";
            break;
        case EffectPresetClass::EMPHASIS:
            pPresetClass = "emph";
            break;
        case EffectPresetClass::MOTIONPATH:
            pPresetClass = "path";
            break;
        case EffectPresetClass::OLEACTION:
            pPresetClass = "verb";  // ?
            break;
        case EffectPresetClass::MEDIACALL:
            pPresetClass = "mediacall";
            break;
        }
    }
    }

    sal_uInt32 nPresetId = 0;
    sal_Bool bPresetId = sal_False;
    if ( pAny[ DFF_ANIM_PRESET_ID ] ) {
    rtl::OUString sPreset;
    if ( *pAny[ DFF_ANIM_PRESET_ID ] >>= sPreset )
        nPresetId = AnimationExporter::GetPresetID( sPreset, nPresetClass, bPresetId );
    }

    sal_uInt32 nPresetSubType = 0;
    sal_Bool bPresetSubType = sal_False;
    if ( pAny[ DFF_ANIM_PRESET_SUB_TYPE ] ) {
    rtl::OUString sPresetSubType;
    if ( *pAny[ DFF_ANIM_PRESET_SUB_TYPE ] >>= sPresetSubType ) {
        nPresetSubType = AnimationExporter::TranslatePresetSubType( nPresetClass, nPresetId, sPresetSubType );
        bPresetSubType = sal_True;
    }
    }

    if( nType != EffectNodeType::TIMING_ROOT && nType != EffectNodeType::MAIN_SEQUENCE ) {
    // it doesn't seem to work right on root and mainseq nodes
    sal_Int16 nFill = AnimationExporter::GetFillMode( rXNode, AnimationFill::AUTO );
    switch( nFill ) {
        case AnimationFill::FREEZE:
        pFill = "freeze";
        break;
        case AnimationFill::HOLD:
        pFill = "hold";
        break;
        case AnimationFill::REMOVE:
        pFill = "remove";
        break;
        case AnimationFill::TRANSITION:
        pFill = "transition";
        break;
    }
    }

    pFS->startElementNS( XML_p, XML_cTn,
             XML_id, I64S( mnAnimationNodeIdMax ++ ),
             XML_dur, fDuration != 0 ? I32S( (sal_Int32) ( fDuration * 1000.0 ) ) : pDuration,
             XML_restart, pRestart,
             XML_nodeType, pNodeType,
             XML_fill, pFill,
             XML_presetClass, pPresetClass,
             XML_presetID, bPresetId ? I64S( nPresetId ) : NULL,
             XML_presetSubtype, bPresetSubType ? I64S( nPresetSubType ) : NULL,
             FSEND );

    aAny = rXNode->getBegin();
    if( aAny.hasValue() ) {
    Sequence< Any > aCondSeq;

    pFS->startElementNS( XML_p, XML_stCondLst, FSEND );
    if( aAny >>= aCondSeq ) {
        for( int i = 0; i < aCondSeq.getLength(); i ++ )
        WriteAnimationCondition( pFS, aCondSeq[ i ], sal_False, bMainSeqChild );
    } else
        WriteAnimationCondition( pFS, aAny, sal_False, bMainSeqChild );
    pFS->endElementNS( XML_p, XML_stCondLst );
    }

    aAny = rXNode->getEnd();
    if( aAny.hasValue() ) {
    Sequence< Any > aCondSeq;

    pFS->startElementNS( XML_p, XML_endCondLst, FSEND );
    if( aAny >>= aCondSeq ) {
        for( int i = 0; i < aCondSeq.getLength(); i ++ )
        WriteAnimationCondition( pFS, aCondSeq[ i ], sal_False, bMainSeqChild );
    } else
        WriteAnimationCondition( pFS, aAny, sal_False, bMainSeqChild );
    pFS->endElementNS( XML_p, XML_stCondLst );
    }

    Reference< XEnumerationAccess > xEnumerationAccess( rXNode, UNO_QUERY );
    if( xEnumerationAccess.is() ) {
    Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
    if( xEnumeration.is() ) {
        DBG(printf ("-----\n"));

        if( xEnumeration->hasMoreElements() ) {
            pFS->startElementNS( XML_p, XML_childTnLst, FSEND );

            while( xEnumeration->hasMoreElements() ) {
                Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY );
                if( xChildNode.is() )
                    WriteAnimationNode( pFS, xChildNode, nType == EffectNodeType::MAIN_SEQUENCE );
            }

            pFS->endElementNS( XML_p, XML_childTnLst );
        }
        DBG(printf ("-----\n"));
    }
    }

    if( bSingle )
    pFS->endElementNS( XML_p, XML_cTn );
}

void PowerPointExport::WriteAnimationNodeSeq( FSHelperPtr pFS, const Reference< XAnimationNode >& rXNode, sal_Int32, sal_Bool bMainSeqChild )
{
    DBG(printf ("write animation node SEQ\n"));

    pFS->startElementNS( XML_p, XML_seq, FSEND );

    WriteAnimationNodeCommonPropsStart( pFS, rXNode, sal_True, bMainSeqChild );

    pFS->startElementNS( XML_p, XML_prevCondLst, FSEND );
    WriteAnimationCondition( pFS, NULL, "onPrev", 0, sal_True );
    pFS->endElementNS( XML_p, XML_prevCondLst );

    pFS->startElementNS( XML_p, XML_nextCondLst, FSEND );
    WriteAnimationCondition( pFS, NULL, "onNext", 0, sal_True );
    pFS->endElementNS( XML_p, XML_nextCondLst );

    pFS->endElementNS( XML_p, XML_seq );
}

void PowerPointExport::WriteAnimationNodeEffect( FSHelperPtr pFS, const Reference< XAnimationNode >& rXNode, sal_Int32, sal_Bool bMainSeqChild )
{
    DBG(printf ("write animation node FILTER\n"));

    Reference< XTransitionFilter > xFilter( rXNode, UNO_QUERY );
    if ( xFilter.is() ) {
    const char* pFilter = ppt::AnimationExporter::FindTransitionName( xFilter->getTransition(), xFilter->getSubtype(), xFilter->getDirection() );
    const char* pDirection = xFilter->getDirection() ? "in" : "out";
    pFS->startElementNS( XML_p, XML_animEffect,
                 XML_filter, pFilter,
                 XML_transition, pDirection,
                 FSEND );

    WriteAnimationNodeAnimateInside( pFS, rXNode, bMainSeqChild, sal_False );

    pFS->endElementNS( XML_p, XML_animEffect );
    }
}

void PowerPointExport::WriteAnimationNode( FSHelperPtr pFS, const Reference< XAnimationNode >& rXNode, sal_Bool bMainSeqChild )
{
    DBG(printf ("export node type: %d\n", rXNode->getType()));
    sal_Int32 xmlNodeType = -1;
    typedef void (PowerPointExport::*AnimationNodeWriteMethod)( FSHelperPtr, const Reference< XAnimationNode >&, sal_Int32, sal_Bool );
    AnimationNodeWriteMethod pMethod = NULL;

    switch( rXNode->getType() ) {
    case AnimationNodeType::PAR:
        xmlNodeType = XML_par;
        break;
    case AnimationNodeType::SEQ:
        pMethod = &PowerPointExport::WriteAnimationNodeSeq;
        break;
    case AnimationNodeType::ANIMATE:
        xmlNodeType = XML_anim;
        pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
        break;
    case AnimationNodeType::SET:
        xmlNodeType = XML_set;
        pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
        break;
    case AnimationNodeType::TRANSITIONFILTER:
        xmlNodeType = XML_animEffect;
        pMethod = &PowerPointExport::WriteAnimationNodeEffect;
        break;
    }

    if( pMethod ) {
    (this->*(pMethod))( pFS, rXNode, xmlNodeType, bMainSeqChild );
    return;
    }

    if( xmlNodeType == -1 )
    return;

    pFS->startElementNS( XML_p, xmlNodeType, FSEND );

    WriteAnimationNodeCommonPropsStart( pFS, rXNode, sal_True, bMainSeqChild );

    pFS->endElementNS( XML_p, xmlNodeType );
}

void PowerPointExport::WriteAnimations( FSHelperPtr pFS )
{
    Reference< XAnimationNodeSupplier > xNodeSupplier( mXDrawPage, UNO_QUERY );
    if( xNodeSupplier.is() ) {
    const Reference< XAnimationNode > xNode( xNodeSupplier->getAnimationNode() );
    if( xNode.is() ) {
        Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY );
        if( xEnumerationAccess.is() ) {
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
        if( xEnumeration.is() && xEnumeration->hasMoreElements() ) {

            pFS->startElementNS( XML_p, XML_timing, FSEND );
            pFS->startElementNS( XML_p, XML_tnLst, FSEND );

            WriteAnimationNode( pFS, xNode, sal_False );

            pFS->endElementNS( XML_p, XML_tnLst );
            pFS->endElementNS( XML_p, XML_timing );
        }
        }
    }
    }
}


static OUString lcl_GetInitials( OUString sName )
{
    OUStringBuffer sRet;

    if ( !sName.isEmpty() ) {
        sRet.append ( sName[0] );
        sal_Int32 nStart = 0, nOffset;

        while ( ( nOffset = sName.indexOf ( ' ', nStart ) ) != -1 ) {
            if ( nOffset + 1 < sName.getLength() )
                sRet.append ( sName[ nOffset + 1 ] );
            nStart = nOffset + 1;
        }
    }

    return sRet.makeStringAndClear();
}

void PowerPointExport::WriteAuthors()
{
    if ( maAuthors.size() <= 0 )
        return;

    FSHelperPtr pFS = openFragmentStreamWithSerializer( US( "ppt/commentAuthors.xml" ),
                                                        US( "application/vnd.openxmlformats-officedocument.presentationml.commentAuthors+xml" ) );
    addRelation( mPresentationFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/commentAuthors" ),
                 US( "commentAuthors.xml" ) );

    pFS->startElementNS( XML_p, XML_cmAuthorLst,
                         FSNS( XML_xmlns, XML_p ), "http://schemas.openxmlformats.org/presentationml/2006/main",
                         FSEND );

    BOOST_FOREACH( AuthorsMap::value_type i, maAuthors ) {
        pFS->singleElementNS( XML_p, XML_cmAuthor,
                              XML_id, I32S( i.second.nId ),
                              XML_name, USS( i.first ),
                              XML_initials, USS( lcl_GetInitials( i.first ) ),
                              XML_lastIdx, I32S( i.second.nLastIndex ),
                              XML_clrIdx, I32S( i.second.nId ),
                              FSEND );
    }

    pFS->endElementNS( XML_p, XML_cmAuthorLst );
}

sal_Int32 PowerPointExport::GetAuthorIdAndLastIndex( OUString sAuthor, sal_Int32& nLastIndex )
{
    if ( maAuthors.count( sAuthor ) <= 0 ) {
        struct AuthorComments aAuthorComments;

        aAuthorComments.nId = maAuthors.size();
        aAuthorComments.nLastIndex = 0;

        maAuthors[ sAuthor ] = aAuthorComments;
    }

    nLastIndex = ++maAuthors[ sAuthor ].nLastIndex;

    return maAuthors[ sAuthor ].nId;
}

bool PowerPointExport::WriteComments( sal_uInt32 nPageNum )
{
    Reference< XAnnotationAccess > xAnnotationAccess( mXDrawPage, uno::UNO_QUERY );
    if ( xAnnotationAccess.is() )
    {
        Reference< XAnnotationEnumeration > xAnnotationEnumeration( xAnnotationAccess->createAnnotationEnumeration() );

        if ( xAnnotationEnumeration->hasMoreElements() )
        {
            FSHelperPtr pFS = openFragmentStreamWithSerializer( OUStringBuffer()
                                                                .appendAscii( "ppt/comments/comment" )
                                                                .append( (sal_Int32) nPageNum + 1 )
                                                                .appendAscii( ".xml" )
                                                                .makeStringAndClear(),
                                                                US( "application/vnd.openxmlformats-officedocument.presentationml.comments+xml" ) );

            pFS->startElementNS( XML_p, XML_cmLst,
                                 FSNS( XML_xmlns, XML_p ), "http://schemas.openxmlformats.org/presentationml/2006/main",
                                 FSEND );

            do {
                Reference< XAnnotation > xAnnotation( xAnnotationEnumeration->nextElement() );
                DateTime aDateTime( xAnnotation->getDateTime() );
                RealPoint2D aRealPoint2D( xAnnotation->getPosition() );
                Reference< XText > xText( xAnnotation->getTextRange() );
                sal_Int32 nLastIndex;
                sal_Int32 nId = GetAuthorIdAndLastIndex ( xAnnotation->getAuthor(), nLastIndex );
                char cDateTime[32];

                snprintf(cDateTime, 31, "%02d-%02d-%02dT%02d:%02d:%02d.%03d", aDateTime.Year, aDateTime.Month, aDateTime.Day, aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds, aDateTime.HundredthSeconds);

                pFS->startElementNS( XML_p, XML_cm,
                                     XML_authorId, I32S( nId ),
                                     XML_dt, cDateTime,
                                     XML_idx, I32S( nLastIndex ),
                                     FSEND );

                pFS->singleElementNS( XML_p, XML_pos,
                                      XML_x, I64S( ( (sal_Int64) ( 57600*aRealPoint2D.X + 1270 )/2540.0 ) ),
                                      XML_y, I64S( ( (sal_Int64) ( 57600*aRealPoint2D.Y + 1270 )/2540.0 ) ),
                                      FSEND );

                pFS->startElementNS( XML_p, XML_text,
                                     FSEND );
                pFS->write( xText->getString() );
                pFS->endElementNS( XML_p, XML_text );

                pFS->endElementNS( XML_p, XML_cm );

            } while ( xAnnotationEnumeration->hasMoreElements() );

            pFS->endElementNS( XML_p, XML_cmLst );

            return true;
        }
    }

    return false;
}

void PowerPointExport::ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 /* nMode */,
                                       sal_Bool bHasBackground, Reference< XPropertySet > aXBackgroundPropSet )
{
    DBG(printf("write slide: %" SAL_PRIuUINT32 "\n----------------\n", nPageNum));

    // slides list
    if( nPageNum == 0 )
        mPresentationFS->startElementNS( XML_p, XML_sldIdLst, FSEND );

    // add explicit relation of presentation to this slide
    OUString sRelId = addRelation( mPresentationFS->getOutputStream(),
                                   US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" ),
                                   OUStringBuffer()
                                   .appendAscii( "slides/slide" )
                                   .append( (sal_Int32) nPageNum + 1 )
                                   .appendAscii( ".xml" )
                                   .makeStringAndClear() );

    mPresentationFS->singleElementNS( XML_p, XML_sldId,
                                      XML_id, I32S( GetNewSlideId() ),
                                      FSNS( XML_r, XML_id ), USS( sRelId ),
                                      FSEND );

    if( nPageNum == mnPages - 1 )
        mPresentationFS->endElementNS( XML_p, XML_sldIdLst );

    FSHelperPtr pFS = openFragmentStreamWithSerializer( OUStringBuffer()
                                                        .appendAscii( "ppt/slides/slide" )
                                                        .append( (sal_Int32) nPageNum + 1 )
                                                        .appendAscii( ".xml" )
                                                        .makeStringAndClear(),
                                                        US( "application/vnd.openxmlformats-officedocument.presentationml.slide+xml" ) );

    if( mpSlidesFSArray.size() < mnPages )
    mpSlidesFSArray.resize( mnPages );
    mpSlidesFSArray[ nPageNum ] = pFS;

    const char* pShow = NULL;

    if( GETA( Visible ) ) {
    sal_Bool bShow(sal_False);
    if( ( mAny >>= bShow ) && !bShow )
        pShow = "0";
    }

    pFS->startElementNS( XML_p, XML_sld, PNMSS,
             XML_show, pShow,
             FSEND );

    pFS->startElementNS( XML_p, XML_cSld, FSEND );

    // background
    if( bHasBackground ) {
        ImplWriteBackground( pFS, aXBackgroundPropSet );
    }

    WriteShapeTree( pFS, NORMAL, sal_False );

    pFS->endElementNS( XML_p, XML_cSld );

    WriteTransition( pFS );
    WriteAnimations( pFS );

    pFS->endElementNS( XML_p, XML_sld );

    // add implicit relation to slide layout
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" ),
                 OUStringBuffer()
                 .appendAscii( "../slideLayouts/slideLayout" )
                 .append( GetLayoutFileId( GetPPTXLayoutId( GetLayoutOffset( mXPagePropSet ) ), nMasterNum ) )
                 .appendAscii( ".xml" )
                 .makeStringAndClear() );

    if ( WriteComments( nPageNum ) )
        // add implicit relation to slide comments
        addRelation( pFS->getOutputStream(),
                     US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments" ),
                     OUStringBuffer()
                     .appendAscii( "../comments/comment" )
                     .append( (sal_Int32) nPageNum + 1 )
                     .appendAscii( ".xml" )
                     .makeStringAndClear() );

    DBG(printf("----------------\n"));
}

void PowerPointExport::ImplWriteNotes( sal_uInt32 nPageNum )
{
    if( !mbCreateNotes || !ContainsOtherShapeThanPlaceholders( sal_True ) )
    return;

    DBG(printf("write Notes %" SAL_PRIuUINT32 "\n----------------\n", nPageNum));

    FSHelperPtr pFS = openFragmentStreamWithSerializer( OUStringBuffer()
                                                        .appendAscii( "ppt/notesSlides/notesSlide" )
                                                        .append( (sal_Int32) nPageNum + 1 )
                                                        .appendAscii( ".xml" )
                                                        .makeStringAndClear(),
                                                        US( "application/vnd.openxmlformats-officedocument.presentationml.notesSlide+xml" ) );

    pFS->startElementNS( XML_p, XML_notes, PNMSS, FSEND );

    pFS->startElementNS( XML_p, XML_cSld, FSEND );

    WriteShapeTree( pFS, NOTICE, sal_False );

    pFS->endElementNS( XML_p, XML_cSld );

    pFS->endElementNS( XML_p, XML_notes );

    // add implicit relation to slide
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" ),
                 OUStringBuffer()
                 .appendAscii( "../slides/slide" )
                 .append( (sal_Int32) nPageNum + 1 )
                 .appendAscii( ".xml" )
                 .makeStringAndClear() );

    // add slide implicit relation to notes
    if( mpSlidesFSArray.size() >= nPageNum )
    addRelation( mpSlidesFSArray[ nPageNum ]->getOutputStream(),
             US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/notesSlide" ),
             OUStringBuffer()
             .appendAscii( "../notesSlides/notesSlide" )
             .append( (sal_Int32) nPageNum + 1 )
             .appendAscii( ".xml" )
             .makeStringAndClear() );

    // add implicit relation to notes master
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/notesMaster" ),
         US( "../notesMasters/notesMaster1.xml" ) );

    DBG(printf("----------------\n"));
}

void PowerPointExport::AddLayoutIdAndRelation( FSHelperPtr pFS, sal_Int32 nLayoutFileId )
{
    // add implicit relation of slide master to slide layout
    OUString sRelId = addRelation( pFS->getOutputStream(),
                   US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" ),
                   OUStringBuffer()
                   .appendAscii( "../slideLayouts/slideLayout" )
                   .append( nLayoutFileId )
                   .appendAscii( ".xml" )
                   .makeStringAndClear() );

    pFS->singleElementNS( XML_p, XML_sldLayoutId,
              XML_id, I64S( GetNewSlideMasterId() ),
              FSNS( XML_r, XML_id ), USS( sRelId ),
              FSEND );
}

sal_Int32 PowerPointExport::nStyleLevelToken[5] =
{
    XML_lvl1pPr,
    XML_lvl2pPr,
    XML_lvl3pPr,
    XML_lvl4pPr,
    XML_lvl5pPr
};

void PowerPointExport::ImplWriteSlideMaster( sal_uInt32 nPageNum, Reference< XPropertySet > aXBackgroundPropSet )
{
    DBG(printf("write slide master: %" SAL_PRIuUINT32 "\n----------------\n", nPageNum));

    // slides list
    if( nPageNum == 0 )
        mPresentationFS->startElementNS( XML_p, XML_sldMasterIdLst, FSEND );

    OUString sRelId = addRelation( mPresentationFS->getOutputStream(),
                                   US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" ),
                                   OUStringBuffer()
                                   .appendAscii( "slideMasters/slideMaster" )
                                   .append( (sal_Int32) nPageNum + 1 )
                                   .appendAscii( ".xml" )
                                   .makeStringAndClear() );

    mPresentationFS->singleElementNS( XML_p, XML_sldMasterId,
                                      XML_id, OString::valueOf( (sal_Int64) GetNewSlideMasterId() ).getStr(),
                                      FSNS( XML_r, XML_id ), USS( sRelId ),
                                      FSEND );

    if( nPageNum == mnMasterPages - 1 )
        mPresentationFS->endElementNS( XML_p, XML_sldMasterIdLst );

    FSHelperPtr pFS =
    openFragmentStreamWithSerializer( OUStringBuffer()
                      .appendAscii( "ppt/slideMasters/slideMaster" )
                      .append( (sal_Int32) nPageNum + 1 )
                      .appendAscii( ".xml" )
                      .makeStringAndClear(),
                      US( "application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml" ) );
    if( mpMasterFSArray.size() < mnMasterPages )
    mpMasterFSArray.resize( mnMasterPages );
    mpMasterFSArray[ nPageNum ] = pFS;

    // write theme per master
    WriteTheme( nPageNum );

    // add implicit relation to the presentation theme
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" ),
         OUStringBuffer()
         .appendAscii( "../theme/theme" )
         .append( (sal_Int32) nPageNum + 1 )
         .appendAscii( ".xml" )
         .makeStringAndClear() );

    pFS->startElementNS( XML_p, XML_sldMaster, PNMSS, FSEND );

    pFS->startElementNS( XML_p, XML_cSld, FSEND );

    ImplWriteBackground( pFS, aXBackgroundPropSet );
    WriteShapeTree( pFS, LAYOUT, sal_True );

    pFS->endElementNS( XML_p, XML_cSld );

    // color map - now it uses colors from hardcoded theme, once we eventually generate theme, this might need update
    pFS->singleElementNS( XML_p, XML_clrMap,
                          XML_bg1, "lt1",
                          XML_bg2, "lt2",
                          XML_tx1, "dk1",
                          XML_tx2, "dk2",
                          XML_accent1, "accent1",
                          XML_accent2, "accent2",
                          XML_accent3, "accent3",
                          XML_accent4, "accent4",
                          XML_accent5, "accent5",
                          XML_accent6, "accent6",
                          XML_hlink, "hlink",
                          XML_folHlink, "folHlink",
                          FSEND );

    // use master's id type as they have same range, mso does that as well
    pFS->startElementNS( XML_p, XML_sldLayoutIdLst, FSEND );

    int nCount = 0;
    for( int i = 0; i < LAYOUT_SIZE; i++) {
        sal_Int32 nLayoutFileId = GetLayoutFileId( i, nPageNum );
        if( nLayoutFileId > 0 ) {
            AddLayoutIdAndRelation( pFS, nLayoutFileId );
            nCount++;
        } else {
            ImplWritePPTXLayout( i, nPageNum );
            AddLayoutIdAndRelation( pFS, GetLayoutFileId( i, nPageNum ) );
        }
    }

    pFS->endElementNS( XML_p, XML_sldLayoutIdLst );

    pFS->endElementNS( XML_p, XML_sldMaster );

    DBG(printf("----------------\n"));
}

sal_Int32 PowerPointExport::GetLayoutFileId( sal_Int32 nOffset, sal_uInt32 nMasterNum )
{
    DBG(printf("GetLayoutFileId offset: %" SAL_PRIdINT32 " master: %" SAL_PRIuUINT32 "", nOffset, nMasterNum));
    if( mLayoutInfo[ nOffset ].mnFileIdArray.size() <= nMasterNum )
        return 0;

    return mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ];
}

void PowerPointExport::ImplWriteLayout( sal_Int32 /*nOffset*/, sal_uInt32 /*nMasterNum*/ )
{
    // we write all the layouts together with master(s)
    // ImplWritePPTXLayout( GetPPTXLayoutId( nOffset ), nMasterNum );
}

void PowerPointExport::ImplWritePPTXLayout( sal_Int32 nOffset, sal_uInt32 nMasterNum )
{
    DBG(printf("write layout: %" SAL_PRIdINT32 "\n", nOffset));

    Reference< drawing::XDrawPagesSupplier > xDPS( getModel(), uno::UNO_QUERY );
    Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY );
    Reference< drawing::XDrawPage > xSlide;
    Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );

    xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );

#ifdef DEBUG
    if( xSlide.is() )
        printf("new page created\n");
#endif

    Reference< beans::XPropertySet > xPropSet( xSlide, uno::UNO_QUERY );
    xPropSet->setPropertyValue( US( "Layout" ), makeAny( short( aLayoutInfo[ nOffset ].nType ) ) );
    DBG(dump_pset( xPropSet ));

    mXPagePropSet = Reference< XPropertySet >( xSlide, UNO_QUERY );
    mXShapes = Reference< XShapes >( xSlide, UNO_QUERY );

    if( mLayoutInfo[ nOffset ].mnFileIdArray.size() < mnMasterPages ) {
    mLayoutInfo[ nOffset ].mnFileIdArray.resize( mnMasterPages );
    }

    if( mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ] != 0 )
    return;

    FSHelperPtr pFS
        = openFragmentStreamWithSerializer( OUStringBuffer()
                                            .appendAscii( "ppt/slideLayouts/slideLayout" )
                                            .append( (sal_Int32) mnLayoutFileIdMax )
                                            .appendAscii( ".xml" )
                                            .makeStringAndClear(),
                                            US( "application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml" ) );

    // add implicit relation of slide layout to slide master
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" ),
                 OUStringBuffer()
                 .appendAscii( "../slideMasters/slideMaster" )
                 .append( (sal_Int32) nMasterNum + 1 )
                 .appendAscii( ".xml" )
                 .makeStringAndClear() );

    pFS->startElementNS( XML_p, XML_sldLayout,
                         PNMSS,
                         XML_type, aLayoutInfo[ nOffset ].sType,
                         XML_preserve, "1",
                         FSEND );

    pFS->startElementNS( XML_p, XML_cSld,
                         XML_name, aLayoutInfo[ nOffset ].sName,
                         FSEND );
    //pFS->write( MINIMAL_SPTREE ); // TODO: write actual shape tree
    WriteShapeTree( pFS, LAYOUT, sal_True );

    pFS->endElementNS( XML_p, XML_cSld );

    pFS->endElementNS( XML_p, XML_sldLayout );

    mLayoutInfo[ nOffset ].mnFileIdArray[ nMasterNum ] = mnLayoutFileIdMax;

    mnLayoutFileIdMax ++;

    xDrawPages->remove( xSlide );
}

void PowerPointExport::WriteShapeTree( FSHelperPtr pFS, PageType ePageType, sal_Bool bMaster )
{
    PowerPointShapeExport aDML( pFS, &maShapeMap, this );
    aDML.SetMaster( bMaster );
    aDML.SetPageType( ePageType );
    sal_uInt32 nShapes;

    pFS->startElementNS( XML_p, XML_spTree, FSEND );
    pFS->write( MAIN_GROUP );

    ResetGroupTable( nShapes = mXShapes->getCount() );

    while( GetNextGroupEntry() ) {

        sal_uInt32 nGroups = GetGroupsClosed();
        for ( sal_uInt32 i = 0; i < nGroups; i++ ) {
            DBG(printf( "leave group\n" ));
        }

        if ( GetShapeByIndex( GetCurrentGroupIndex(), sal_True ) ) {
            DBG(printf( "mType: \"%s\"\n", mType.getStr() ));
            aDML.WriteShape( mXShape );
        }
    }

    pFS->endElementNS( XML_p, XML_spTree );
}

#define BEGIN_SHAPE mpFS->startElementNS( XML_p, XML_sp, FSEND )
#define END_SHAPE mpFS->endElementNS( XML_p, XML_sp )

ShapeExport& PowerPointShapeExport::WritePageShape( Reference< XShape > xShape, PageType ePageType, sal_Bool bPresObj )
{
    if( ( ePageType == NOTICE && bPresObj ) || ePageType == LAYOUT )
    return WritePlaceholderShape( xShape, SlideImage );

    return WriteTextShape( xShape );
}

sal_Bool PowerPointShapeExport::WritePlaceholder( Reference< XShape > xShape, PlaceholderType ePlaceholder, sal_Bool bMaster )
{
    DBG(printf("WritePlaceholder %d %d\n", bMaster, ShapeExport::NonEmptyText( xShape )));
    if( bMaster && ShapeExport::NonEmptyText( xShape ) ) {
    WritePlaceholderShape( xShape, ePlaceholder );

    return sal_True;
    }

    return sal_False;
}

ShapeExport& PowerPointShapeExport::WritePlaceholderShape( Reference< XShape > xShape, PlaceholderType ePlaceholder )
{
    BEGIN_SHAPE;

    // non visual shape properties
    mpFS->startElementNS( XML_p, XML_nvSpPr, FSEND );
    WriteNonVisualDrawingProperties( xShape, IDS( PlaceHolder ) );
    mpFS->startElementNS( XML_p, XML_cNvSpPr, FSEND );
    mpFS->singleElementNS( XML_a, XML_spLocks, XML_noGrp, "1", FSEND );
    mpFS->endElementNS( XML_p, XML_cNvSpPr );
    mpFS->startElementNS( XML_p, XML_nvPr, FSEND );

    const char* pType = NULL;
    switch( ePlaceholder ) {
    case SlideImage:
        pType = "sldImg";
        break;
    case Notes:
        pType = "body";
        break;
    case Header:
        pType = "hdr";
        break;
    case Footer:
        pType = "ftr";
        break;
    case SlideNumber:
        pType = "sldNum";
        break;
    case DateAndTime:
        pType = "dt";
        break;
    case Outliner:
        pType = "body";
        break;
    case Title:
        pType = "title";
        break;
    case Subtitle:
        pType = "subTitle";
        break;
    default:
        DBG(printf("warning: unhandled placeholder type: %d\n", ePlaceholder));
    }
    DBG(printf("write placeholder %s\n", pType));
    mpFS->singleElementNS( XML_p, XML_ph, XML_type, pType, FSEND );
    mpFS->endElementNS( XML_p, XML_nvPr );
    mpFS->endElementNS( XML_p, XML_nvSpPr );

    // visual shape properties
    mpFS->startElementNS( XML_p, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a );
    WritePresetShape( "rect" );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() )
    WriteBlipFill( xProps, S( "GraphicURL" ) );
    mpFS->endElementNS( XML_p, XML_spPr );

    WriteTextBox( xShape, XML_p );

    END_SHAPE;

    return *this;
}

#define MINIMAL_THEME "  <a:themeElements>\
    <a:clrScheme name=\"Office\">\
      <a:dk1>\
        <a:sysClr val=\"windowText\" lastClr=\"000000\"/>\
      </a:dk1>\
      <a:lt1>\
        <a:sysClr val=\"window\" lastClr=\"FFFFFF\"/>\
      </a:lt1>\
      <a:dk2>\
        <a:srgbClr val=\"1F497D\"/>\
      </a:dk2>\
      <a:lt2>\
        <a:srgbClr val=\"EEECE1\"/>\
      </a:lt2>\
      <a:accent1>\
        <a:srgbClr val=\"4F81BD\"/>\
      </a:accent1>\
      <a:accent2>\
        <a:srgbClr val=\"C0504D\"/>\
      </a:accent2>\
      <a:accent3>\
        <a:srgbClr val=\"9BBB59\"/>\
      </a:accent3>\
      <a:accent4>\
        <a:srgbClr val=\"8064A2\"/>\
      </a:accent4>\
      <a:accent5>\
        <a:srgbClr val=\"4BACC6\"/>\
      </a:accent5>\
      <a:accent6>\
        <a:srgbClr val=\"F79646\"/>\
      </a:accent6>\
      <a:hlink>\
        <a:srgbClr val=\"0000FF\"/>\
      </a:hlink>\
      <a:folHlink>\
        <a:srgbClr val=\"800080\"/>\
      </a:folHlink>\
    </a:clrScheme>\
    <a:fontScheme name=\"Office\">\
      <a:majorFont>\
        <a:latin typeface=\"Arial\"/>\
        <a:ea typeface=\"DejaVu Sans\"/>\
        <a:cs typeface=\"DejaVu Sans\"/>\
      </a:majorFont>\
      <a:minorFont>\
        <a:latin typeface=\"Arial\"/>\
        <a:ea typeface=\"DejaVu Sans\"/>\
        <a:cs typeface=\"DejaVu Sans\"/>\
      </a:minorFont>\
    </a:fontScheme>\
    <a:fmtScheme name=\"Office\">\
      <a:fillStyleLst>\
        <a:solidFill>\
          <a:schemeClr val=\"phClr\"/>\
        </a:solidFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"50000\"/>\
                <a:satMod val=\"300000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"35000\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"37000\"/>\
                <a:satMod val=\"300000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"15000\"/>\
                <a:satMod val=\"350000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:lin ang=\"16200000\" scaled=\"1\"/>\
        </a:gradFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"51000\"/>\
                <a:satMod val=\"130000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"80000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"93000\"/>\
                <a:satMod val=\"130000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"94000\"/>\
                <a:satMod val=\"135000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:lin ang=\"16200000\" scaled=\"0\"/>\
        </a:gradFill>\
      </a:fillStyleLst>\
      <a:lnStyleLst>\
        <a:ln w=\"9525\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\
          <a:solidFill>\
            <a:schemeClr val=\"phClr\">\
              <a:shade val=\"95000\"/>\
              <a:satMod val=\"105000\"/>\
            </a:schemeClr>\
          </a:solidFill>\
          <a:prstDash val=\"solid\"/>\
        </a:ln>\
        <a:ln w=\"25400\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\
          <a:solidFill>\
            <a:schemeClr val=\"phClr\"/>\
          </a:solidFill>\
          <a:prstDash val=\"solid\"/>\
        </a:ln>\
        <a:ln w=\"38100\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\
          <a:solidFill>\
            <a:schemeClr val=\"phClr\"/>\
          </a:solidFill>\
          <a:prstDash val=\"solid\"/>\
        </a:ln>\
      </a:lnStyleLst>\
      <a:effectStyleLst>\
        <a:effectStyle>\
          <a:effectLst>\
            <a:outerShdw blurRad=\"40000\" dist=\"20000\" dir=\"5400000\" rotWithShape=\"0\">\
              <a:srgbClr val=\"000000\">\
                <a:alpha val=\"38000\"/>\
              </a:srgbClr>\
            </a:outerShdw>\
          </a:effectLst>\
        </a:effectStyle>\
        <a:effectStyle>\
          <a:effectLst>\
            <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\
              <a:srgbClr val=\"000000\">\
                <a:alpha val=\"35000\"/>\
              </a:srgbClr>\
            </a:outerShdw>\
          </a:effectLst>\
        </a:effectStyle>\
        <a:effectStyle>\
          <a:effectLst>\
            <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\
              <a:srgbClr val=\"000000\">\
                <a:alpha val=\"35000\"/>\
              </a:srgbClr>\
            </a:outerShdw>\
          </a:effectLst>\
          <a:scene3d>\
            <a:camera prst=\"orthographicFront\">\
              <a:rot lat=\"0\" lon=\"0\" rev=\"0\"/>\
            </a:camera>\
            <a:lightRig rig=\"threePt\" dir=\"t\">\
              <a:rot lat=\"0\" lon=\"0\" rev=\"1200000\"/>\
            </a:lightRig>\
          </a:scene3d>\
          <a:sp3d>\
            <a:bevelT w=\"63500\" h=\"25400\"/>\
          </a:sp3d>\
        </a:effectStyle>\
      </a:effectStyleLst>\
      <a:bgFillStyleLst>\
        <a:solidFill>\
          <a:schemeClr val=\"phClr\"/>\
        </a:solidFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"40000\"/>\
                <a:satMod val=\"350000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"40000\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"45000\"/>\
                <a:shade val=\"99000\"/>\
                <a:satMod val=\"350000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"20000\"/>\
                <a:satMod val=\"255000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:path path=\"circle\">\
            <a:fillToRect l=\"50000\" t=\"-80000\" r=\"50000\" b=\"180000\"/>\
          </a:path>\
        </a:gradFill>\
        <a:gradFill rotWithShape=\"1\">\
          <a:gsLst>\
            <a:gs pos=\"0\">\
              <a:schemeClr val=\"phClr\">\
                <a:tint val=\"80000\"/>\
                <a:satMod val=\"300000\"/>\
              </a:schemeClr>\
            </a:gs>\
            <a:gs pos=\"100000\">\
              <a:schemeClr val=\"phClr\">\
                <a:shade val=\"30000\"/>\
                <a:satMod val=\"200000\"/>\
              </a:schemeClr>\
            </a:gs>\
          </a:gsLst>\
          <a:path path=\"circle\">\
            <a:fillToRect l=\"50000\" t=\"50000\" r=\"50000\" b=\"50000\"/>\
          </a:path>\
        </a:gradFill>\
      </a:bgFillStyleLst>\
    </a:fmtScheme>\
  </a:themeElements>"

void PowerPointExport::WriteTheme( sal_Int32 nThemeNum )
{
    FSHelperPtr pFS = openFragmentStreamWithSerializer( OUStringBuffer()
                                .appendAscii( "ppt/theme/theme" )
                                .append( (sal_Int32) nThemeNum + 1 )
                                .appendAscii( ".xml" )
                                .makeStringAndClear(),
                                                        US( "application/vnd.openxmlformats-officedocument.theme+xml" ) );

    pFS->startElementNS( XML_a, XML_theme,
                         FSNS( XML_xmlns, XML_a), "http://schemas.openxmlformats.org/drawingml/2006/main",
                         XML_name, "Office Theme",
                         FSEND );

    pFS->write( MINIMAL_THEME );
    pFS->endElementNS( XML_a, XML_theme );
}

sal_Bool PowerPointExport::ImplCreateDocument()
{
    mbCreateNotes = sal_False;

    for( sal_uInt32 i = 0; i < mnPages; i++ )
    {
        if ( !GetPageByIndex( i, NOTICE ) )
            return sal_False;

    if( ContainsOtherShapeThanPlaceholders( sal_True ) ) {
        mbCreateNotes = sal_True;
        break;
    }
    }

    return sal_True;
}

sal_Bool PowerPointExport::WriteNotesMaster()
{
    DBG(printf("write Notes master\n----------------\n"));

    mPresentationFS->startElementNS( XML_p, XML_notesMasterIdLst, FSEND );

    OUString sRelId = addRelation( mPresentationFS->getOutputStream(),
                                   US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/notesMaster" ),
                                   US( "notesMasters/notesMaster1.xml" ) );

    mPresentationFS->singleElementNS( XML_p, XML_notesMasterId,
                                      FSNS( XML_r, XML_id ), USS( sRelId ),
                                      FSEND );

    mPresentationFS->endElementNS( XML_p, XML_notesMasterIdLst );

    FSHelperPtr pFS =
    openFragmentStreamWithSerializer( US( "ppt/notesMasters/notesMaster1.xml" ),
                      US( "application/vnd.openxmlformats-officedocument.presentationml.notesMaster+xml" ) );
    // write theme per master
    WriteTheme( mnMasterPages );

    // add implicit relation to the presentation theme
    addRelation( pFS->getOutputStream(),
                 US( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" ),
         OUStringBuffer()
         .appendAscii( "../theme/theme" )
         .append( (sal_Int32) mnMasterPages + 1 )
         .appendAscii( ".xml" )
         .makeStringAndClear() );

    pFS->startElementNS( XML_p, XML_notesMaster, PNMSS, FSEND );

    pFS->startElementNS( XML_p, XML_cSld, FSEND );

    Reference< XPropertySet > aXBackgroundPropSet;
    if( ImplGetPropertyValue( mXPagePropSet, rtl::OUString( "Background" ) ) &&
    ( mAny >>= aXBackgroundPropSet ) )
    ImplWriteBackground( pFS, aXBackgroundPropSet );

    WriteShapeTree( pFS, NOTICE, sal_True );

    pFS->endElementNS( XML_p, XML_cSld );

    // color map - now it uses colors from hardcoded theme, once we eventually generate theme, this might need update
    pFS->singleElementNS( XML_p, XML_clrMap,
                          XML_bg1, "lt1",
                          XML_bg2, "lt2",
                          XML_tx1, "dk1",
                          XML_tx2, "dk2",
                          XML_accent1, "accent1",
                          XML_accent2, "accent2",
                          XML_accent3, "accent3",
                          XML_accent4, "accent4",
                          XML_accent5, "accent5",
                          XML_accent6, "accent6",
                          XML_hlink, "hlink",
                          XML_folHlink, "folHlink",
                          FSEND );

    pFS->endElementNS( XML_p, XML_notesMaster );

    DBG(printf("----------------\n"));

    return sal_True;
}

sal_Bool PowerPointExport::ImplCreateMainNotes()
{
    if( mbCreateNotes )
    return WriteNotesMaster();

    return sal_True;
}

OUString SAL_CALL PowerPointExport_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Impress.oox.PowerPointExport" );
}

uno::Sequence< OUString > SAL_CALL PowerPointExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( "com.sun.star.comp.ooxpptx" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL PowerPointExport_createInstance(const uno::Reference< XComponentContext > & rxCtxt ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new PowerPointExport( rxCtxt );
}

OUString PowerPointExport::implGetImplementationName() const
{
    return PowerPointExport_getImplementationName();
}
}
}

// UNO component

static struct cppu::ImplementationEntry g_entries[] =
{
    {
        oox::core::PowerPointExport_createInstance,
        oox::core::PowerPointExport_getImplementationName,
       oox::core::PowerPointExport_getSupportedServiceNames,
        cppu::createSingleComponentFactory,
        0 , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

#ifdef __cplusplus
extern "C"
{
#endif

SAL_DLLPUBLIC_EXPORT void* SAL_CALL sdfilt_component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

#ifdef __cplusplus
}
#endif

DBG(
void dump_pset(Reference< XPropertySet > rXPropSet)
{
    Reference< XPropertySetInfo > info = rXPropSet->getPropertySetInfo ();
    Sequence< beans::Property > props = info->getProperties ();

    for (int i=0; i < props.getLength (); i++) {
        OString name = OUStringToOString( props [i].Name, RTL_TEXTENCODING_UTF8);
        printf ("%30s = ", name.getStr() );

        Any value = rXPropSet->getPropertyValue( props [i].Name );

        OUString strValue;
        sal_Int32 intValue;
        bool boolValue;
        RectanglePoint pointValue;

        if( value >>= strValue )
            printf ("\"%s\"\n", USS( strValue ) );
        else if( value >>= intValue )
            printf ("%" SAL_PRIdINT32 "            (hex: %" SAL_PRIxUINT32 ")\n", intValue, intValue);
        else if( value >>= boolValue )
            printf ("%d            (bool)\n", boolValue);
        else if( value >>= pointValue )
            printf ("%d            (RectanglePoint)\n", pointValue);
        else
            printf ("???           <unhandled type>\n");
    }
}
);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
