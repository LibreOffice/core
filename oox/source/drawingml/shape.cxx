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

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/customshapeproperties.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/effectproperties.hxx"
#include "oox/drawingml/shapepropertymap.hxx"
#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/table/tableproperties.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/drawingml/chart/chartspacefragment.hxx"
#include "oox/drawingml/chart/chartspacemodel.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertyset.hxx"

#include <tools/solar.h>
#include <tools/gen.hxx>
#include <tools/mapunit.hxx>
#include <editeng/unoprnms.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <svl/outstrm.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/fltrcfg.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::style;

namespace oox { namespace drawingml {

// ============================================================================

Shape::Shape( const sal_Char* pServiceName )
: mbIsChild( false )
, mpLinePropertiesPtr( new LineProperties )
, mpFillPropertiesPtr( new FillProperties )
, mpGraphicPropertiesPtr( new GraphicProperties )
, mpCustomShapePropertiesPtr( new CustomShapeProperties )
, mpEffectPropertiesPtr( new EffectProperties )
, mpMasterTextListStyle( new TextListStyle )
, mnSubType( 0 )
, meFrameType( FRAMETYPE_GENERIC )
, mnRotation( 0 )
, mbFlipH( false )
, mbFlipV( false )
, mbHidden( false )
, mbHiddenMasterShape( false )
, mbLockedCanvas( false )
, maDiagramDoms( 0 )
{
    if ( pServiceName )
        msServiceName = OUString::createFromAscii( pServiceName );
    setDefaults();
}

Shape::Shape( const ShapePtr& pSourceShape )
: maChildren()
, mbIsChild( pSourceShape->mbIsChild )
, mpTextBody(pSourceShape->mpTextBody)
, mpLinePropertiesPtr( pSourceShape->mpLinePropertiesPtr )
, mpFillPropertiesPtr( pSourceShape->mpFillPropertiesPtr )
, mpGraphicPropertiesPtr( pSourceShape->mpGraphicPropertiesPtr )
, mpCustomShapePropertiesPtr( pSourceShape->mpCustomShapePropertiesPtr )
, mpTablePropertiesPtr( pSourceShape->mpTablePropertiesPtr )
, mp3DPropertiesPtr( pSourceShape->mp3DPropertiesPtr )
, mpEffectPropertiesPtr (pSourceShape->mpEffectPropertiesPtr)
, maShapeProperties( pSourceShape->maShapeProperties )
, mpMasterTextListStyle( pSourceShape->mpMasterTextListStyle )
, mxShape()
, msServiceName( pSourceShape->msServiceName )
, msName( pSourceShape->msName )
, msId( pSourceShape->msId )
, mnSubType( pSourceShape->mnSubType )
, moSubTypeIndex( pSourceShape->moSubTypeIndex )
, maShapeStyleRefs( pSourceShape->maShapeStyleRefs )
, maSize( pSourceShape->maSize )
, maPosition( pSourceShape->maPosition )
, meFrameType( pSourceShape->meFrameType )
, mnRotation( pSourceShape->mnRotation )
, mbFlipH( pSourceShape->mbFlipH )
, mbFlipV( pSourceShape->mbFlipV )
, mbHidden( pSourceShape->mbHidden )
, mbHiddenMasterShape( pSourceShape->mbHiddenMasterShape )
, mbLockedCanvas( pSourceShape->mbLockedCanvas )
, maDiagramDoms( pSourceShape->maDiagramDoms )
{}


Shape::~Shape()
{
}

table::TablePropertiesPtr Shape::getTableProperties()
{
    if ( !mpTablePropertiesPtr.get() )
        mpTablePropertiesPtr.reset( new table::TableProperties() );
    return mpTablePropertiesPtr;
}

void Shape::setDefaults()
{
    maDefaultShapeProperties[ PROP_TextAutoGrowHeight ] <<= false;
    maDefaultShapeProperties[ PROP_TextWordWrap ] <<= true;
    maDefaultShapeProperties[ PROP_TextLeftDistance ]  <<= static_cast< sal_Int32 >( 250 );
    maDefaultShapeProperties[ PROP_TextUpperDistance ] <<= static_cast< sal_Int32 >( 125 );
    maDefaultShapeProperties[ PROP_TextRightDistance ] <<= static_cast< sal_Int32 >( 250 );
    maDefaultShapeProperties[ PROP_TextLowerDistance ] <<= static_cast< sal_Int32 >( 125 );
    maDefaultShapeProperties[ PROP_CharHeight ] <<= static_cast< float >( 18.0 );
    maDefaultShapeProperties[ PROP_TextVerticalAdjust ] <<= TextVerticalAdjust_TOP;
    maDefaultShapeProperties[ PROP_ParaAdjust ] <<= static_cast< sal_Int16 >( ParagraphAdjust_LEFT ); // check for RTL?
}

::oox::vml::OleObjectInfo& Shape::setOleObjectType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setOleObjectType - multiple frame types" );
    meFrameType = FRAMETYPE_OLEOBJECT;
    mxOleObjectInfo.reset( new ::oox::vml::OleObjectInfo( true ) );
    return *mxOleObjectInfo;
}

ChartShapeInfo& Shape::setChartType( bool bEmbedShapes )
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setChartType - multiple frame types" );
    meFrameType = FRAMETYPE_CHART;
    msServiceName = "com.sun.star.drawing.OLE2Shape";
    mxChartShapeInfo.reset( new ChartShapeInfo( bEmbedShapes ) );
    return *mxChartShapeInfo;
}

void Shape::setDiagramType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setDiagramType - multiple frame types" );
    meFrameType = FRAMETYPE_DIAGRAM;
    msServiceName = "com.sun.star.drawing.GroupShape";
    mnSubType = 0;
}

void Shape::setTableType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setTableType - multiple frame types" );
    meFrameType = FRAMETYPE_TABLE;
    msServiceName = "com.sun.star.drawing.TableShape";
    mnSubType = 0;
}

void Shape::setServiceName( const sal_Char* pServiceName )
{
    if ( pServiceName )
        msServiceName = OUString::createFromAscii( pServiceName );
}


const ShapeStyleRef* Shape::getShapeStyleRef( sal_Int32 nRefType ) const
{
    ShapeStyleRefMap::const_iterator aIt = maShapeStyleRefs.find( nRefType );
    return (aIt == maShapeStyleRefs.end()) ? 0 : &aIt->second;
}

void Shape::addShape(
        ::oox::core::XmlFilterBase& rFilterBase,
        const Theme* pTheme,
        const Reference< XShapes >& rxShapes,
        basegfx::B2DHomMatrix& aTransformation,
        FillProperties& rShapeOrParentShapeFillProps,
        const awt::Rectangle* pShapeRect,
        ShapeIdMap* pShapeMap )
{
    SAL_INFO("oox.drawingml", OSL_THIS_FUNC << " id: " << msId);

    try
    {
        OUString sServiceName( msServiceName );
        if( !sServiceName.isEmpty() )
        {
            basegfx::B2DHomMatrix aMatrix( aTransformation );
            Reference< XShape > xShape( createAndInsert( rFilterBase, sServiceName, pTheme, rxShapes, pShapeRect, sal_False, sal_False, aMatrix, rShapeOrParentShapeFillProps ) );

            if( pShapeMap && !msId.isEmpty() )
            {
                (*pShapeMap)[ msId ] = shared_from_this();
            }

            // if this is a group shape, we have to add also each child shape
            Reference< XShapes > xShapes( xShape, UNO_QUERY );
            if ( xShapes.is() )
                addChildren( rFilterBase, *this, pTheme, xShapes, pShapeRect ? *pShapeRect : awt::Rectangle( maPosition.X, maPosition.Y, maSize.Width, maSize.Height ), pShapeMap, aMatrix );


            if( meFrameType == FRAMETYPE_DIAGRAM )
            {
                if( !SvtFilterOptions::Get().IsSmartArt2Shape() )
                    keepDiagramCompatibilityInfo( rFilterBase );
            }
        }
    }
    catch( const Exception& e )
    {
        SAL_WARN( "oox.drawingml", OSL_THIS_FUNC << "Exception: " << e.Message );
    }
}

void Shape::setLockedCanvas(bool bLockedCanvas)
{
    mbLockedCanvas = bLockedCanvas;
}

bool Shape::getLockedCanvas()
{
    return mbLockedCanvas;
}

void Shape::applyShapeReference( const Shape& rReferencedShape, bool bUseText )
{
    SAL_INFO("oox", OSL_THIS_FUNC << "apply shape reference: " << rReferencedShape.msId << " to shape id: " << msId);

    if ( rReferencedShape.mpTextBody.get() && bUseText )
        mpTextBody = TextBodyPtr( new TextBody( *rReferencedShape.mpTextBody.get() ) );
    else
        mpTextBody.reset();
    maShapeProperties = rReferencedShape.maShapeProperties;
    mpLinePropertiesPtr = LinePropertiesPtr( new LineProperties( *rReferencedShape.mpLinePropertiesPtr.get() ) );
    mpFillPropertiesPtr = FillPropertiesPtr( new FillProperties( *rReferencedShape.mpFillPropertiesPtr.get() ) );
    mpCustomShapePropertiesPtr = CustomShapePropertiesPtr( new CustomShapeProperties( *rReferencedShape.mpCustomShapePropertiesPtr.get() ) );
    mpTablePropertiesPtr = table::TablePropertiesPtr( rReferencedShape.mpTablePropertiesPtr.get() ? new table::TableProperties( *rReferencedShape.mpTablePropertiesPtr.get() ) : NULL );
    mpEffectPropertiesPtr = EffectPropertiesPtr( new EffectProperties( *rReferencedShape.mpEffectPropertiesPtr.get() ) );
    mpMasterTextListStyle = TextListStylePtr( new TextListStyle( *rReferencedShape.mpMasterTextListStyle.get() ) );
    maShapeStyleRefs = rReferencedShape.maShapeStyleRefs;
    maSize = rReferencedShape.maSize;
    maPosition = rReferencedShape.maPosition;
    mnRotation = rReferencedShape.mnRotation;
    mbFlipH = rReferencedShape.mbFlipH;
    mbFlipV = rReferencedShape.mbFlipV;
    mbHidden = rReferencedShape.mbHidden;
}

void Shape::addChildren( ::oox::core::XmlFilterBase& rFilterBase,
                         const Theme* pTheme,
                         const Reference< XShapes >& rxShapes,
                         basegfx::B2DHomMatrix& aTransformation,
                         const awt::Rectangle* pShapeRect,
                         ShapeIdMap* pShapeMap )
{
    addChildren(rFilterBase, *this, pTheme, rxShapes,
                pShapeRect ?
                 *pShapeRect :
                 awt::Rectangle( maPosition.X, maPosition.Y, maSize.Width, maSize.Height ),
                pShapeMap, aTransformation);
}

struct ActionLockGuard
{
    explicit ActionLockGuard(Reference<drawing::XShape> const& xShape)
        : m_xLockable(xShape, UNO_QUERY)
    {
        if (m_xLockable.is()) {
            m_xLockable->addActionLock();
        }
    }
    ~ActionLockGuard()
    {
        if (m_xLockable.is()) {
            m_xLockable->removeActionLock();
        }
    }
private:
    Reference<document::XActionLockable> m_xLockable;
};

// for group shapes, the following method is also adding each child
void Shape::addChildren(
        XmlFilterBase& rFilterBase,
        Shape& rMaster,
        const Theme* pTheme,
        const Reference< XShapes >& rxShapes,
        const awt::Rectangle&,
        ShapeIdMap* pShapeMap,
        basegfx::B2DHomMatrix& aTransformation )
{
    basegfx::B2DHomMatrix aChildTransformation;

    aChildTransformation.translate(-maChPosition.X, -maChPosition.Y);
    aChildTransformation.scale(1/(maChSize.Width ? maChSize.Width : 1.0), 1/(maChSize.Height ? maChSize.Height : 1.0));
    aChildTransformation *= aTransformation;

    SAL_INFO("oox.drawingml", OSL_THIS_FUNC << "parent matrix:\n"
             << aChildTransformation.get(0, 0)
             << aChildTransformation.get(0, 1)
             << aChildTransformation.get(0, 2) << "\n"
             << aChildTransformation.get(1, 0)
             << aChildTransformation.get(1, 1)
             << aChildTransformation.get(1, 2)
             << aChildTransformation.get(2, 0)
             << aChildTransformation.get(2, 1)
             << aChildTransformation.get(2, 2));

    std::vector< ShapePtr >::iterator aIter( rMaster.maChildren.begin() );
    while( aIter != rMaster.maChildren.end() ) {
        (*aIter)->setMasterTextListStyle( mpMasterTextListStyle );
        (*aIter++)->addShape( rFilterBase, pTheme, rxShapes, aChildTransformation, getFillProperties(), NULL, pShapeMap );
    }
}

Reference< XShape > Shape::createAndInsert(
        ::oox::core::XmlFilterBase& rFilterBase,
        const OUString& rServiceName,
        const Theme* pTheme,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
        const awt::Rectangle* /* pShapeRect */,
        sal_Bool bClearText,
        sal_Bool bDoNotInsertEmptyTextBody,
        basegfx::B2DHomMatrix& aParentTransformation,
        FillProperties& rShapeOrParentShapeFillProps )
{
    bool bIsEmbMedia = false;
    SAL_INFO("oox.drawingml", OSL_THIS_FUNC << " id: " << msId);

    awt::Rectangle aShapeRectHmm( maPosition.X / 360, maPosition.Y / 360, maSize.Width / 360, maSize.Height / 360 );

    OUString aServiceName;
    if( rServiceName == "com.sun.star.drawing.GraphicObjectShape" &&
        mpGraphicPropertiesPtr && !mpGraphicPropertiesPtr->maAudio.msEmbed.isEmpty() )
    {
        aServiceName = finalizeServiceName( rFilterBase, "com.sun.star.presentation.MediaShape", aShapeRectHmm );
        bIsEmbMedia = true;
    }
    else
    {
        aServiceName = finalizeServiceName( rFilterBase, rServiceName, aShapeRectHmm );
    }
    sal_Bool bIsCustomShape = ( aServiceName == "com.sun.star.drawing.CustomShape" ||
                                aServiceName == "com.sun.star.drawing.ConnectorShape" );

    basegfx::B2DHomMatrix aTransformation;

    if( maSize.Width != 1 || maSize.Height != 1)
    {
        // take care there are no zeros used by error
        aTransformation.scale(
            maSize.Width ? maSize.Width : 1.0,
            maSize.Height ? maSize.Height : 1.0 );
    }

    if( mbFlipH || mbFlipV || mnRotation != 0)
    {
        // calculate object's center
        basegfx::B2DPoint aCenter(0.5, 0.5);
        aCenter *= aTransformation;

        // center object at origin
        aTransformation.translate( -aCenter.getX(), -aCenter.getY() );

        if( !bIsCustomShape && ( mbFlipH || mbFlipV ) )
        {
            // mirror around object's center
            aTransformation.scale( mbFlipH ? -1.0 : 1.0, mbFlipV ? -1.0 : 1.0 );
        }

        if( mnRotation != 0 )
        {
            // rotate around object's center
            aTransformation.rotate( F_PI180 * ( (double)mnRotation / 60000.0 ) );
        }

        // move object back from center
        aTransformation.translate( aCenter.getX(), aCenter.getY() );
    }

    if( maPosition.X != 0 || maPosition.Y != 0)
    {
        // if global position is used, add it to transformation
        aTransformation.translate( maPosition.X, maPosition.Y );
    }

    aTransformation = aParentTransformation*aTransformation;
    aParentTransformation = aTransformation;
    aTransformation.scale(1/360.0, 1/360.0);

    // special for lineshape
    if ( aServiceName == "com.sun.star.drawing.LineShape" )
    {
        ::basegfx::B2DPolygon aPoly;
        aPoly.insert( 0, ::basegfx::B2DPoint( 0, 0 ) );
        aPoly.insert( 1, ::basegfx::B2DPoint( maSize.Width ? 1 : 0, maSize.Height ? 1 : 0 ) );
        aPoly.transform( aTransformation );

        // now creating the corresponding PolyPolygon
        sal_Int32 i, nNumPoints = aPoly.count();
        uno::Sequence< awt::Point > aPointSequence( nNumPoints );
        awt::Point* pPoints = aPointSequence.getArray();
        for( i = 0; i < nNumPoints; ++i )
        {
            const ::basegfx::B2DPoint aPoint( aPoly.getB2DPoint( i ) );
            pPoints[ i ] = awt::Point( static_cast< sal_Int32 >( aPoint.getX() ), static_cast< sal_Int32 >( aPoint.getY() ) );
        }
        uno::Sequence< uno::Sequence< awt::Point > > aPolyPolySequence( 1 );
        aPolyPolySequence.getArray()[ 0 ] = aPointSequence;

        maShapeProperties[ PROP_PolyPolygon ] <<= aPolyPolySequence;
    }
    else if ( aServiceName == "com.sun.star.drawing.ConnectorShape" )
    {
        ::basegfx::B2DPolygon aPoly;
        aPoly.insert( 0, ::basegfx::B2DPoint( 0, 0 ) );
        aPoly.insert( 1, ::basegfx::B2DPoint( maSize.Width ? 1 : 0, maSize.Height ? 1 : 0 ) );
        aPoly.transform( aTransformation );

        basegfx::B2DPoint aStartPosition( aPoly.getB2DPoint( 0 ) );
        basegfx::B2DPoint aEndPosition( aPoly.getB2DPoint( 1 ) );
        awt::Point aAWTStartPosition( static_cast< sal_Int32 >( aStartPosition.getX() ), static_cast< sal_Int32 >( aStartPosition.getY() ) );
        awt::Point aAWTEndPosition( static_cast< sal_Int32 >( aEndPosition.getX() ), static_cast< sal_Int32 >( aEndPosition.getY() ) );

        maShapeProperties[ PROP_StartPosition ] <<= aAWTStartPosition;
        maShapeProperties[ PROP_EndPosition ] <<= aAWTEndPosition;
    }
    else
    {
        // now set transformation for this object
        HomogenMatrix3 aMatrix;


        aMatrix.Line1.Column1 = aTransformation.get(0,0);
        aMatrix.Line1.Column2 = aTransformation.get(0,1);
        aMatrix.Line1.Column3 = aTransformation.get(0,2);

        aMatrix.Line2.Column1 = aTransformation.get(1,0);
        aMatrix.Line2.Column2 = aTransformation.get(1,1);
        aMatrix.Line2.Column3 = aTransformation.get(1,2);

        aMatrix.Line3.Column1 = aTransformation.get(2,0);
        aMatrix.Line3.Column2 = aTransformation.get(2,1);
        aMatrix.Line3.Column3 = aTransformation.get(2,2);

        maShapeProperties[ PROP_Transformation ] <<= aMatrix;
    }

    Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
    if ( !mxShape.is() )
        mxShape = Reference< drawing::XShape >( xServiceFact->createInstance( aServiceName ), UNO_QUERY_THROW );

    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    if( mxShape.is() && xSet.is() )
    {
        if( !msName.isEmpty() )
        {
            Reference< container::XNamed > xNamed( mxShape, UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( msName );
        }
        rxShapes->add( mxShape );

        if ( mbHidden || mbHiddenMasterShape )
        {
            SAL_INFO("oox.drawingml", OSL_THIS_FUNC << "invisible shape with id: " << msId);
            const OUString sVisible( "Visible" );
            xSet->setPropertyValue( sVisible, Any( sal_False ) );
        }

        ActionLockGuard const alg(mxShape);

        // sj: removing default text of placeholder objects such as SlideNumberShape or HeaderShape
        if ( bClearText )
        {
            uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
            if ( xText.is() )
            {
                OUString aEmpty;
                xText->setString( aEmpty );
            }
        }

        const GraphicHelper& rGraphicHelper = rFilterBase.getGraphicHelper();

        LineProperties aLineProperties;
        aLineProperties.maLineFill.moFillType = XML_noFill;
        sal_Int32 nLinePhClr = -1;
        FillProperties aFillProperties;
        aFillProperties.moFillType = XML_noFill;
        sal_Int32 nFillPhClr = -1;
        EffectProperties aEffectProperties;
        // TODO: use ph color when applying effect properties
        //sal_Int32 nEffectPhClr = -1;

        if( pTheme )
        {
            if( const ShapeStyleRef* pLineRef = getShapeStyleRef( XML_lnRef ) )
            {
                if( const LineProperties* pLineProps = pTheme->getLineStyle( pLineRef->mnThemedIdx ) )
                    aLineProperties.assignUsed( *pLineProps );
                nLinePhClr = pLineRef->maPhClr.getColor( rGraphicHelper );
            }
            if( const ShapeStyleRef* pFillRef = getShapeStyleRef( XML_fillRef ) )
            {
                if( const FillProperties* pFillProps = pTheme->getFillStyle( pFillRef->mnThemedIdx ) )
                    aFillProperties.assignUsed( *pFillProps );
                nFillPhClr = pFillRef->maPhClr.getColor( rGraphicHelper );
            }
            if( const ShapeStyleRef* pEffectRef = getShapeStyleRef( XML_effectRef ) )
            {
                if( const EffectProperties* pEffectProps = pTheme->getEffectStyle( pEffectRef->mnThemedIdx ) )
                    aEffectProperties.assignUsed( *pEffectProps );
                // TODO: use ph color when applying effect properties
                // nEffectPhClr = pEffectRef->maPhClr.getColor( rGraphicHelper );
            }
        }

        aLineProperties.assignUsed( getLineProperties() );

        // group fill inherits from parent
        if ( getFillProperties().moFillType.has() && getFillProperties().moFillType.get() == XML_grpFill )
            getFillProperties().assignUsed( rShapeOrParentShapeFillProps );
        aFillProperties.assignUsed( getFillProperties() );
        aEffectProperties.assignUsed ( getEffectProperties() );

        ShapePropertyMap aShapeProps( rFilterBase.getModelObjectHelper() );

        // add properties from textbody to shape properties
        if( mpTextBody.get() )
        {
            mpTextBody->getTextProperties().pushRotationAdjustments( mnRotation );
            aShapeProps.assignUsed( mpTextBody->getTextProperties().maPropertyMap );
            // Push char properties as well - specifically useful when this is a placeholder
            if( mpMasterTextListStyle &&  mpMasterTextListStyle->getListStyle()[0]->getTextCharacterProperties().moHeight.has() )
                aShapeProps[ PROP_CharHeight ] <<= GetFontHeight( mpMasterTextListStyle->getListStyle()[0]->getTextCharacterProperties().moHeight.get() );
        }

        // applying properties
        aShapeProps.assignUsed( getShapeProperties() );
        aShapeProps.assignUsed( maDefaultShapeProperties );
        if ( bIsEmbMedia || aServiceName == "com.sun.star.drawing.GraphicObjectShape" || aServiceName == "com.sun.star.drawing.OLE2Shape" )
            mpGraphicPropertiesPtr->pushToPropMap( aShapeProps, rGraphicHelper );
        if ( mpTablePropertiesPtr.get() && aServiceName == "com.sun.star.drawing.TableShape" )
            mpTablePropertiesPtr->pushToPropSet( rFilterBase, xSet, mpMasterTextListStyle );
        aFillProperties.pushToPropMap( aShapeProps, rGraphicHelper, mnRotation, nFillPhClr, mbFlipH, mbFlipV );
        aLineProperties.pushToPropMap( aShapeProps, rGraphicHelper, nLinePhClr );
        // TODO: use ph color when applying effect properties
        aEffectProperties.pushToPropMap( aShapeProps, rGraphicHelper );

        // applying autogrowheight property before setting shape size, because
        // the shape size might be changed if currently autogrowheight is true
        // we must also check that the PropertySet supports the property.
        Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
        const OUString& rPropName = PropertyMap::getPropertyName( PROP_TextAutoGrowHeight );
        if( xSetInfo.is() && xSetInfo->hasPropertyByName( rPropName ) )
            if( aShapeProps.hasProperty( PROP_TextAutoGrowHeight ) )
                xSet->setPropertyValue( rPropName, Any( false ) );

        // do not set properties at a group shape (this causes
        // assertions from svx) ...
        if( aServiceName != "com.sun.star.drawing.GroupShape" )
        {
            PropertySet( xSet ).setProperties( aShapeProps );
            if (mbLockedCanvas && aServiceName == "com.sun.star.drawing.LineShape")
            {
                // It seems the position and size for lines inside a locked canvas is absolute.
                mxShape->setPosition(awt::Point(aShapeRectHmm.X, aShapeRectHmm.Y));
                mxShape->setSize(awt::Size(aShapeRectHmm.Width, aShapeRectHmm.Height));
            }
        }

        if( bIsCustomShape )
        {
            if ( mbFlipH )
                mpCustomShapePropertiesPtr->setMirroredX( sal_True );
            if ( mbFlipV )
                mpCustomShapePropertiesPtr->setMirroredY( sal_True );
            if( getTextBody() )
            {
                sal_Int32 nTextRotateAngle = static_cast< sal_Int32 >( getTextBody()->getTextProperties().moRotation.get( 0 ) );
                mpCustomShapePropertiesPtr->setTextRotateAngle( nTextRotateAngle / 60000 );
            }

            SAL_INFO("oox.cscode", "==cscode== shape name: '" << msName << "'");
            mpCustomShapePropertiesPtr->pushToPropSet( rFilterBase, xSet, mxShape, maSize );
        }
        else if( getTextBody() )
            getTextBody()->getTextProperties().pushVertSimulation();

        // in some cases, we don't have any text body.
        if( getTextBody() && ( !bDoNotInsertEmptyTextBody || !mpTextBody->isEmpty() ) )
        {
            Reference < XText > xText( mxShape, UNO_QUERY );
            if ( xText.is() )   // not every shape is supporting an XText interface (e.g. GroupShape)
            {
                TextCharacterProperties aCharStyleProperties;
                if( const ShapeStyleRef* pFontRef = getShapeStyleRef( XML_fontRef ) )
                {
                    if( pTheme )
                        if( const TextCharacterProperties* pCharProps = pTheme->getFontStyle( pFontRef->mnThemedIdx ) )
                            aCharStyleProperties.assignUsed( *pCharProps );
                    SAL_INFO("oox.drawingml", OSL_THIS_FUNC << "use font color");
                    aCharStyleProperties.maCharColor.assignIfUsed( pFontRef->maPhClr );
                }

                Reference < XTextCursor > xAt = xText->createTextCursor();
                getTextBody()->insertAt( rFilterBase, xText, xAt, aCharStyleProperties, mpMasterTextListStyle );
            }
        }
    }

    if( mxShape.is() )
        finalizeXShape( rFilterBase, rxShapes );

    return mxShape;
}

void Shape::keepDiagramCompatibilityInfo( XmlFilterBase& rFilterBase )
{
    try
    {
        if( !maDiagramDoms.hasElements() )
            return;

        Reference < XPropertySet > xSet( mxShape, UNO_QUERY_THROW );
        Reference < XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
        if ( !xSetInfo.is() )
            return;

        const OUString aGrabBagPropName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
        if( !xSetInfo->hasPropertyByName( aGrabBagPropName ) )
            return;

        Sequence < PropertyValue > aGrabBag;
        xSet->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;

        // We keep the previous items, if present
        if ( aGrabBag.hasElements() )
        {
            sal_Int32 length = aGrabBag.getLength();
            aGrabBag.realloc( length+maDiagramDoms.getLength() );

            for( sal_Int32 i = 0; i < maDiagramDoms.getLength(); ++i )
                aGrabBag[length+i] = maDiagramDoms[i];

            xSet->setPropertyValue( aGrabBagPropName, Any( aGrabBag ) );
        } else
            xSet->setPropertyValue( aGrabBagPropName, Any( maDiagramDoms ) );

        xSet->setPropertyValue( OUString( "MoveProtect" ), Any( sal_True ) );
        xSet->setPropertyValue( OUString( "SizeProtect" ), Any( sal_True ) );

        // Replace existing shapes with a new Graphic Object rendered
        // from them
        Reference < XShape > xShape( renderDiagramToGraphic( rFilterBase ) );
        Reference < XShapes > xShapes( mxShape, UNO_QUERY_THROW );
        while( xShapes->hasElements() )
            xShapes->remove( Reference < XShape > ( xShapes->getByIndex( 0 ),  UNO_QUERY_THROW ) );
        xShapes->add( xShape );
    }
    catch( const Exception& e )
    {
        SAL_WARN( "oox.drawingml", OSL_THIS_FUNC << "Exception: " << e.Message );
    }
}

Reference < XShape > Shape::renderDiagramToGraphic( XmlFilterBase& rFilterBase )
{
    Reference< XShape > xShape;

    try
    {
        if( !maDiagramDoms.hasElements() )
            return xShape;

        // Stream in which to place the rendered shape
        SvMemoryStream mpTempStream;
        Reference < io::XStream > xStream( new utl::OStreamWrapper( mpTempStream ) );
        Reference < io::XOutputStream > xOutputStream( xStream->getOutputStream() );

        // Rendering format
        OUString sFormat( "SVM" );

        // Size of the rendering
        awt::Size aActualSize = mxShape->getSize();
        Size aResolution( Application::GetDefaultDevice()->LogicToPixel( Size( 100, 100 ), MAP_CM ) );
        double fPixelsPer100thmm = static_cast < double > ( aResolution.Width() ) / 100000.0;
        awt::Size aSize = awt::Size( static_cast < sal_Int32 > ( ( fPixelsPer100thmm * aActualSize.Width ) + 0.5 ),
                                     static_cast < sal_Int32 > ( ( fPixelsPer100thmm * aActualSize.Height ) + 0.5 ) );

        Sequence< PropertyValue > aFilterData( 4 );
        aFilterData[ 0 ].Name = "PixelWidth";
        aFilterData[ 0 ].Value <<= aSize.Width;
        aFilterData[ 1 ].Name = "PixelHeight";
        aFilterData[ 1 ].Value <<= aSize.Height;
        aFilterData[ 2 ].Name = "LogicalWidth";
        aFilterData[ 2 ].Value <<= aActualSize.Width;
        aFilterData[ 3 ].Name = "LogicalHeight";
        aFilterData[ 3 ].Value <<= aActualSize.Height;

        Sequence < PropertyValue > aDescriptor( 3 );
        aDescriptor[ 0 ].Name = "OutputStream";
        aDescriptor[ 0 ].Value <<= xOutputStream;
        aDescriptor[ 1 ].Name = "FilterName";
        aDescriptor[ 1 ].Value <<= sFormat;
        aDescriptor[ 2 ].Name = "FilterData";
        aDescriptor[ 2 ].Value <<= aFilterData;

        Reference < lang::XComponent > xSourceDoc( mxShape, UNO_QUERY_THROW );
        Reference < XGraphicExportFilter > xGraphicExporter = GraphicExportFilter::create( rFilterBase.getComponentContext() );
        xGraphicExporter->setSourceDocument( xSourceDoc );
        xGraphicExporter->filter( aDescriptor );

        mpTempStream.Seek( STREAM_SEEK_TO_BEGIN );

        Graphic aGraphic;
        GraphicFilter aFilter( sal_False );
        if ( aFilter.ImportGraphic( aGraphic, "", mpTempStream, GRFILTER_FORMAT_NOTFOUND, NULL, 0, static_cast < Sequence < PropertyValue >* > ( NULL ), NULL ) != GRFILTER_OK )
        {
            SAL_WARN( "oox.drawingml", OSL_THIS_FUNC
                      << "Unable to import rendered stream into graphic object" );
            return xShape;
        }

        Reference < graphic::XGraphic > xGraphic( aGraphic.GetXGraphic() );
        Reference < lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
        xShape = Reference < XShape > ( xServiceFact->createInstance( OUString( "com.sun.star.drawing.GraphicObjectShape" ) ), UNO_QUERY_THROW );
        Reference < XPropertySet > xPropSet( xShape, UNO_QUERY_THROW );
        xPropSet->setPropertyValue( OUString( "Graphic" ), Any( xGraphic ) );
        xPropSet->setPropertyValue( OUString( "MoveProtect" ), Any( sal_True ) );
        xPropSet->setPropertyValue( OUString( "SizeProtect" ), Any( sal_True ) );
        xPropSet->setPropertyValue( OUString( "Name" ), Any( OUString( "RenderedShapes" ) ) );
    }
    catch( const Exception& e )
    {
        SAL_WARN( "oox.drawingml", OSL_THIS_FUNC << "Exception: " << e.Message );
    }

    return xShape;
}

void Shape::setTextBody(const TextBodyPtr & pTextBody)
{
    mpTextBody = pTextBody;
}


TextBodyPtr Shape::getTextBody()
{
    return mpTextBody;
}

void Shape::setMasterTextListStyle( const TextListStylePtr& pMasterTextListStyle )
{
    SAL_INFO("oox.drawingml", OSL_THIS_FUNC << "set master text list style to shape id: " << msId);

    mpMasterTextListStyle = pMasterTextListStyle;
}

OUString Shape::finalizeServiceName( XmlFilterBase& rFilter, const OUString& rServiceName, const awt::Rectangle& rShapeRect )
{
    OUString aServiceName = rServiceName;
    switch( meFrameType )
    {
        case FRAMETYPE_OLEOBJECT:
        {
            awt::Size aOleSize( rShapeRect.Width, rShapeRect.Height );
            if( rFilter.getOleObjectHelper().importOleObject( maShapeProperties, *mxOleObjectInfo, aOleSize ) )
                aServiceName = "com.sun.star.drawing.OLE2Shape";

            // get the path to the representation graphic
            OUString aGraphicPath;
            if( !mxOleObjectInfo->maShapeId.isEmpty() )
                if( ::oox::vml::Drawing* pVmlDrawing = rFilter.getVmlDrawing() )
                    if( const ::oox::vml::ShapeBase* pVmlShape = pVmlDrawing->getShapes().getShapeById( mxOleObjectInfo->maShapeId, true ) )
                        aGraphicPath = pVmlShape->getGraphicPath();

            // import and store the graphic
            if( !aGraphicPath.isEmpty() )
            {
                Reference< graphic::XGraphic > xGraphic = rFilter.getGraphicHelper().importEmbeddedGraphic( aGraphicPath );
                if( xGraphic.is() )
                    maShapeProperties[ PROP_Graphic ] <<= xGraphic;
            }
        }
        break;

        default:;
    }
    return aServiceName;
}

void Shape::finalizeXShape( XmlFilterBase& rFilter, const Reference< XShapes >& rxShapes )
{
    switch( meFrameType )
    {
        case FRAMETYPE_CHART:
        {
            OSL_ENSURE( !mxChartShapeInfo->maFragmentPath.isEmpty(), "Shape::finalizeXShape - missing chart fragment" );
            if( mxShape.is() && !mxChartShapeInfo->maFragmentPath.isEmpty() ) try
            {
                // set the chart2 OLE class ID at the OLE shape
                PropertySet aShapeProp( mxShape );
                aShapeProp.setProperty( PROP_CLSID, OUString( "12dcae26-281f-416f-a234-c3086127382e" ) );

                // get the XModel interface of the embedded object from the OLE shape
                Reference< frame::XModel > xDocModel;
                aShapeProp.getProperty( xDocModel, PROP_Model );
                Reference< chart2::XChartDocument > xChartDoc( xDocModel, UNO_QUERY_THROW );

                // load the chart data from the XML fragment
                chart::ChartSpaceModel aModel;
                rFilter.importFragment( new chart::ChartSpaceFragment( rFilter, mxChartShapeInfo->maFragmentPath, aModel ) );

                // convert imported chart model to chart document
                Reference< drawing::XShapes > xExternalPage;
                if( !mxChartShapeInfo->mbEmbedShapes )
                    xExternalPage = rxShapes;
                if( rFilter.getChartConverter() )
                {
                    rFilter.getChartConverter()->convertFromModel( rFilter, aModel, xChartDoc, xExternalPage, mxShape->getPosition(), mxShape->getSize() );
                    if( !xChartDoc->hasInternalDataProvider() )
                    {
                        Reference< chart2::data::XDataReceiver > xDataRec( xChartDoc, UNO_QUERY );
                        Reference< chart2::data::XDataSource > xData( xDataRec->getUsedData(), UNO_QUERY );
                        if( xData->getDataSequences().getLength() <= 0 || xData->getDataSequences()[0]->getValues()->getData().getLength() <= 0 )
                        {
                            rFilter.useInternalChartDataTable( true );
                            rFilter.getChartConverter()->convertFromModel( rFilter, aModel, xChartDoc, xExternalPage, mxShape->getPosition(), mxShape->getSize() );
                            rFilter.useInternalChartDataTable( false );
                        }
                    }

                }
            }
            catch( Exception& )
            {
            }
        }
        break;

        default:;
    }
}

// ============================================================================

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
