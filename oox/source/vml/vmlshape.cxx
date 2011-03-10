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

#include "oox/vml/vmlshape.hxx"

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/ole/axcontrolfragment.hxx"
#include "oox/ole/oleobjecthelper.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/vml/vmltextbox.hxx"
#include "oox/helper/containerhelper.hxx"

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star::text;

namespace oox {
namespace vml {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::uno;

using ::oox::core::XmlFilterBase;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

namespace {

const sal_Int32 VML_SHAPETYPE_PICTUREFRAME  = 75;
const sal_Int32 VML_SHAPETYPE_HOSTCONTROL   = 201;

// ----------------------------------------------------------------------------

Point lclGetAbsPoint( const Point& rRelPoint, const Rectangle& rShapeRect, const Rectangle& rCoordSys )
{
    double fWidthRatio = static_cast< double >( rShapeRect.Width ) / rCoordSys.Width;
    double fHeightRatio = static_cast< double >( rShapeRect.Height ) / rCoordSys.Height;
    Point aAbsPoint;
    aAbsPoint.X = static_cast< sal_Int32 >( rShapeRect.X + fWidthRatio * (rRelPoint.X - rCoordSys.X) + 0.5 );
    aAbsPoint.Y = static_cast< sal_Int32 >( rShapeRect.Y + fHeightRatio * (rRelPoint.Y - rCoordSys.Y) + 0.5 );
    return aAbsPoint;
}

Rectangle lclGetAbsRect( const Rectangle& rRelRect, const Rectangle& rShapeRect, const Rectangle& rCoordSys )
{
    double fWidthRatio = static_cast< double >( rShapeRect.Width ) / rCoordSys.Width;
    double fHeightRatio = static_cast< double >( rShapeRect.Height ) / rCoordSys.Height;
    Rectangle aAbsRect;
    aAbsRect.X = static_cast< sal_Int32 >( rShapeRect.X + fWidthRatio * (rRelRect.X - rCoordSys.X) + 0.5 );
    aAbsRect.Y = static_cast< sal_Int32 >( rShapeRect.Y + fHeightRatio * (rRelRect.Y - rCoordSys.Y) + 0.5 );
    aAbsRect.Width = static_cast< sal_Int32 >( fWidthRatio * rRelRect.Width + 0.5 );
    aAbsRect.Height = static_cast< sal_Int32 >( fHeightRatio * rRelRect.Height + 0.5 );
    return aAbsRect;
}

void lclInsertTextFrame( const XmlFilterBase& rFilter, const Reference< XShape >& rxShape )
{
    OSL_ENSURE( rxShape.is(), "lclInsertTextFrame - missing XShape" );
    if ( rxShape.is( ) )
    {
        try
        {
            Reference< XTextDocument > xDoc( rFilter.getModel( ), UNO_QUERY_THROW );
            Reference< XTextContent > xCtnt( rxShape, UNO_QUERY_THROW );
            xCtnt->attach( xDoc->getText( )->getStart( ) );
        }
        catch( Exception& )
        {
        }
    }
}

void lclSetXShapeRect( const Reference< XShape >& rxShape, const Rectangle& rShapeRect )
{
    OSL_ENSURE( rxShape.is(), "lclSetXShapeRect - missing XShape" );
    if( rxShape.is() )
    {
        Reference< XTextFrame > xTextFrame( rxShape, UNO_QUERY );
        if ( !xTextFrame.is( ) )
        {
            rxShape->setPosition( Point( rShapeRect.X, rShapeRect.Y ) );
            rxShape->setSize( Size( rShapeRect.Width, rShapeRect.Height ) );
        }
        else
        {
            Reference< XPropertySet > xProps( xTextFrame, UNO_QUERY_THROW );
            try
            {
                // The size
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("SizeType")), Any( SizeType::FIX ) );
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("FrameIsAutomaticHeight")), Any( sal_False ) );
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Height")), Any( rShapeRect.Height ) );
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Width")), Any( rShapeRect.Width ) );

                // The position
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("HoriOrientPosition")), Any( rShapeRect.X ) );
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("HoriOrientRelation")),
                        Any( RelOrientation::FRAME ) );
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("HoriOrient")),
                        Any( HoriOrientation::NONE ) );

                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("VertOrientPosition")), Any( rShapeRect.Y ) );
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("VertOrientRelation")),
                        Any( RelOrientation::FRAME ) );
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("VertOrient")),
                        Any( VertOrientation::NONE ) );
            }
            catch ( Exception& )
            {
            }
        }
    }
} // namespace

// ============================================================================

ShapeTypeModel::ShapeTypeModel()
{
}

void ShapeTypeModel::assignUsed( const ShapeTypeModel& rSource )
{
    moShapeType.assignIfUsed( rSource.moShapeType );
    moCoordPos.assignIfUsed( rSource.moCoordPos );
    moCoordSize.assignIfUsed( rSource.moCoordSize );
    /*  The style properties position, left, top, width, height, margin-left,
        margin-top are not derived from shape template to shape. */
    maStrokeModel.assignUsed( rSource.maStrokeModel );
    maFillModel.assignUsed( rSource.maFillModel );
    moGraphicPath.assignIfUsed( rSource.moGraphicPath );
    moGraphicTitle.assignIfUsed( rSource.moGraphicTitle );
}

// ----------------------------------------------------------------------------

ShapeType::ShapeType( Drawing& rDrawing ) :
    mrDrawing( rDrawing )
{
}

ShapeType::~ShapeType()
{
}

sal_Int32 ShapeType::getShapeType() const
{
    return maTypeModel.moShapeType.get( 0 );
}

OUString ShapeType::getGraphicPath() const
{
    return maTypeModel.moGraphicPath.get( OUString() );
}

Rectangle ShapeType::getCoordSystem() const
{
    Int32Pair aCoordPos = maTypeModel.moCoordPos.get( Int32Pair( 0, 0 ) );
    Int32Pair aCoordSize = maTypeModel.moCoordSize.get( Int32Pair( 1000, 1000 ) );
    return Rectangle( aCoordPos.first, aCoordPos.second, aCoordSize.first, aCoordSize.second );
}

Rectangle ShapeType::getRectangle( const ShapeParentAnchor* pParentAnchor ) const
{
    return pParentAnchor ?
        lclGetAbsRect( getRelRectangle(), pParentAnchor->maShapeRect, pParentAnchor->maCoordSys ) :
        getAbsRectangle();
}

Rectangle ShapeType::getAbsRectangle() const
{
    const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();

    sal_Int32 nWidth = ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maWidth, 0, true, true );
    if ( nWidth == 0 )
        nWidth = 1;

    sal_Int32 nHeight = ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maHeight, 0, true, true );
    if ( nHeight == 0 )
        nHeight = 1;

    return Rectangle(
        ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maLeft, 0, true, true ) + ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maMarginLeft, 0, true, true ),
        ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maTop, 0, false, true ) + ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maMarginTop, 0, false, true ),
        nWidth, nHeight );
}

Rectangle ShapeType::getRelRectangle() const
{
    return Rectangle(
        maTypeModel.maLeft.toInt32(),
        maTypeModel.maTop.toInt32(),
        maTypeModel.maWidth.toInt32(),
        maTypeModel.maHeight.toInt32() );
}

// ============================================================================

ClientData::ClientData() :
    mnObjType( XML_TOKEN_INVALID ),
    mnTextHAlign( XML_Left ),
    mnTextVAlign( XML_Top ),
    mnCol( -1 ),
    mnRow( -1 ),
    mnChecked( VML_CLIENTDATA_UNCHECKED ),
    mnDropStyle( XML_Combo ),
    mnDropLines( 1 ),
    mnVal( 0 ),
    mnMin( 0 ),
    mnMax( 0 ),
    mnInc( 0 ),
    mnPage( 0 ),
    mnSelType( XML_Single ),
    mnVTEdit( VML_CLIENTDATA_TEXT ),
    mbPrintObject( true ),
    mbVisible( false ),
    mbDde( false ),
    mbNo3D( false ),
    mbNo3D2( false ),
    mbMultiLine( false ),
    mbVScroll( false ),
    mbSecretEdit( false )
{
}

// ----------------------------------------------------------------------------

ShapeModel::ShapeModel()
{
}

ShapeModel::~ShapeModel()
{
}

TextBox& ShapeModel::createTextBox()
{
    mxTextBox.reset( new TextBox );
    return *mxTextBox;
}

ClientData& ShapeModel::createClientData()
{
    mxClientData.reset( new ClientData );
    return *mxClientData;
}

// ----------------------------------------------------------------------------

ShapeBase::ShapeBase( Drawing& rDrawing ) :
    ShapeType( rDrawing )
{
}

void ShapeBase::finalizeFragmentImport()
{
    // resolve shape template reference
    if( (maShapeModel.maType.getLength() > 1) && (maShapeModel.maType[ 0 ] == '#') )
        if( const ShapeType* pShapeType = mrDrawing.getShapes().getShapeTypeById( maShapeModel.maType.copy( 1 ), true ) )
            maTypeModel.assignUsed( pShapeType->getTypeModel() );
}

OUString ShapeBase::getShapeName() const
{
    if( maTypeModel.maShapeName.getLength() > 0 )
        return maTypeModel.maShapeName;

    OUString aBaseName = mrDrawing.getShapeBaseName( *this );
    if( aBaseName.getLength() > 0 )
    {
        sal_Int32 nShapeIdx = mrDrawing.getLocalShapeIndex( getShapeId() );
        if( nShapeIdx > 0 )
            return OUStringBuffer( aBaseName ).append( sal_Unicode( ' ' ) ).append( nShapeIdx ).makeStringAndClear();
    }

    return OUString();
}

const ShapeType* ShapeBase::getChildTypeById( const OUString& ) const
{
    return 0;
}

const ShapeBase* ShapeBase::getChildById( const OUString& ) const
{
    return 0;
}

Reference< XShape > ShapeBase::convertAndInsert( const Reference< XShapes >& rxShapes, const ShapeParentAnchor* pParentAnchor ) const
{
    Reference< XShape > xShape;
    if( mrDrawing.isShapeSupported( *this ) )
    {
        /*  Calculate shape rectangle. Applications may do something special
            according to some imported shape client data (e.g. Excel cell anchor). */
        Rectangle aShapeRect = calcShapeRectangle( pParentAnchor );

        // convert the shape, if the calculated rectangle is not empty
        if( ((aShapeRect.Width > 0) || (aShapeRect.Height > 0)) && rxShapes.is() )
        {
            xShape = implConvertAndInsert( rxShapes, aShapeRect );
            if( xShape.is() )
            {
                // set shape name (imported or generated)
                PropertySet aShapeProp( xShape );
                aShapeProp.setProperty( PROP_Name, getShapeName() );

                /*  Notify the drawing that a new shape has been inserted. For
                    convenience, pass the rectangle that contains position and
                    size of the shape. */
                bool bGroupChild = pParentAnchor != 0;
                mrDrawing.notifyXShapeInserted( xShape, aShapeRect, *this, bGroupChild );
            }
        }
    }
    return xShape;
}

void ShapeBase::convertFormatting( const Reference< XShape >& rxShape, const ShapeParentAnchor* pParentAnchor ) const
{
    if( rxShape.is() )
    {
        /*  Calculate shape rectangle. Applications may do something special
            according to some imported shape client data (e.g. Excel cell anchor). */
        Rectangle aShapeRect = calcShapeRectangle( pParentAnchor );

        // convert the shape, if the calculated rectangle is not empty
        if( (aShapeRect.Width > 0) || (aShapeRect.Height > 0) )
        {
            rxShape->setPosition( Point( aShapeRect.X, aShapeRect.Y ) );
            rxShape->setSize( Size( aShapeRect.Width, aShapeRect.Height ) );
            convertShapeProperties( rxShape );
        }
    }
}

// protected ------------------------------------------------------------------

Rectangle ShapeBase::calcShapeRectangle( const ShapeParentAnchor* pParentAnchor ) const
{
    /*  Calculate shape rectangle. Applications may do something special
        according to some imported shape client data (e.g. Excel cell anchor). */
    Rectangle aShapeRect;
    const ClientData* pClientData = getClientData();
    if( !pClientData || !mrDrawing.convertClientAnchor( aShapeRect, pClientData->maAnchor ) )
        aShapeRect = getRectangle( pParentAnchor );
    return aShapeRect;
}

void ShapeBase::convertShapeProperties( const Reference< XShape >& rxShape ) const
{
    ModelObjectHelper& rModelObjectHelper = mrDrawing.getFilter().getModelObjectHelper();
    const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();

    PropertyMap aPropMap;
    maTypeModel.maStrokeModel.pushToPropMap( aPropMap, rModelObjectHelper, rGraphicHelper );
    maTypeModel.maFillModel.pushToPropMap( aPropMap, rModelObjectHelper, rGraphicHelper );

    PropertySet aPropSet( rxShape );
    aPropSet.setProperties( aPropMap );
}

// ============================================================================

SimpleShape::SimpleShape( Drawing& rDrawing, const OUString& rService ) :
    ShapeBase( rDrawing ),
    maService( rService )
{
}

Reference< XShape > SimpleShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    Reference< XShape > xShape = mrDrawing.createAndInsertXShape( maService, rxShapes, rShapeRect );
    convertShapeProperties( xShape );
    return xShape;
}

// ============================================================================

RectangleShape::RectangleShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, CREATE_OUSTRING( "com.sun.star.drawing.RectangleShape" ) )
{
}

// ============================================================================

EllipseShape::EllipseShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, CREATE_OUSTRING( "com.sun.star.drawing.EllipseShape" ) )
{
}

// ============================================================================

PolyLineShape::PolyLineShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, CREATE_OUSTRING( "com.sun.star.drawing.PolyLineShape" ) )
{
}

Reference< XShape > PolyLineShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    Reference< XShape > xShape = SimpleShape::implConvertAndInsert( rxShapes, rShapeRect );
    // polygon path
    Rectangle aCoordSys = getCoordSystem();
    if( !maShapeModel.maPoints.empty() && (aCoordSys.Width > 0) && (aCoordSys.Height > 0) )
    {
        ::std::vector< Point > aAbsPoints;
        for( ShapeModel::PointVector::const_iterator aIt = maShapeModel.maPoints.begin(), aEnd = maShapeModel.maPoints.end(); aIt != aEnd; ++aIt )
            aAbsPoints.push_back( lclGetAbsPoint( *aIt, rShapeRect, aCoordSys ) );
        PointSequenceSequence aPointSeq( 1 );
        aPointSeq[ 0 ] = ContainerHelper::vectorToSequence( aAbsPoints );
        PropertySet aPropSet( xShape );
        aPropSet.setProperty( PROP_PolyPolygon, aPointSeq );
    }
    return xShape;
}

// ============================================================================

CustomShape::CustomShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, CREATE_OUSTRING( "com.sun.star.drawing.CustomShape" ) )
{
}

Reference< XShape > CustomShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    // try to create a custom shape
    Reference< XShape > xShape = SimpleShape::implConvertAndInsert( rxShapes, rShapeRect );
    if( xShape.is() ) try
    {
        // create the custom shape geometry
        Reference< XEnhancedCustomShapeDefaulter > xDefaulter( xShape, UNO_QUERY_THROW );
        xDefaulter->createCustomShapeDefaults( OUString::valueOf( getShapeType() ) );
        // convert common properties
        convertShapeProperties( xShape );
    }
    catch( Exception& )
    {
    }
    return xShape;
}

// ============================================================================

ComplexShape::ComplexShape( Drawing& rDrawing ) :
    CustomShape( rDrawing )
{
}

Reference< XShape > ComplexShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    XmlFilterBase& rFilter = mrDrawing.getFilter();
    sal_Int32 nShapeType = getShapeType();
    OUString aGraphicPath = getGraphicPath();

    // try to find registered OLE object info
    if( const OleObjectInfo* pOleObjectInfo = mrDrawing.getOleObjectInfo( maTypeModel.maShapeId ) )
    {
        OSL_ENSURE( nShapeType == VML_SHAPETYPE_PICTUREFRAME, "ComplexShape::implConvertAndInsert - unexpected shape type" );

        // if OLE object is embedded into a DrawingML shape (PPTX), do not create it here
        if( pOleObjectInfo->mbDmlShape )
            return Reference< XShape >();

        PropertyMap aOleProps;
        Size aOleSize( rShapeRect.Width, rShapeRect.Height );
        if( rFilter.getOleObjectHelper().importOleObject( aOleProps, *pOleObjectInfo, aOleSize ) )
        {
            Reference< XShape > xShape = mrDrawing.createAndInsertXShape( CREATE_OUSTRING( "com.sun.star.drawing.OLE2Shape" ), rxShapes, rShapeRect );
            if( xShape.is() )
            {
                // set the replacement graphic
                if( aGraphicPath.getLength() > 0 )
                {
                    Reference< XGraphic > xGraphic = rFilter.getGraphicHelper().importEmbeddedGraphic( aGraphicPath );
                    if( xGraphic.is() )
                        aOleProps[ PROP_Graphic ] <<= xGraphic;
                }

                PropertySet aPropSet( xShape );
                aPropSet.setProperties( aOleProps );

                return xShape;
            }
        }
    }

    // try to find registered form control info
    const ControlInfo* pControlInfo = mrDrawing.getControlInfo( maTypeModel.maShapeId );
    if( pControlInfo && (pControlInfo->maFragmentPath.getLength() > 0) )
    {
        OSL_ENSURE( nShapeType == VML_SHAPETYPE_HOSTCONTROL, "ComplexShape::implConvertAndInsert - unexpected shape type" );
        OUString aShapeName = getShapeName();
        if( aShapeName.getLength() > 0 )
        {
            OSL_ENSURE( aShapeName == pControlInfo->maName, "ComplexShape::implConvertAndInsert - control name mismatch" );
            // load the control properties from fragment
            ::oox::ole::EmbeddedControl aControl( aShapeName );
            if( rFilter.importFragment( new ::oox::ole::AxControlFragment( rFilter, pControlInfo->maFragmentPath, aControl ) ) )
            {
                // create and return the control shape (including control model)
                sal_Int32 nCtrlIndex = -1;
                Reference< XShape > xShape = mrDrawing.createAndInsertXControlShape( aControl, rxShapes, rShapeRect, nCtrlIndex );
                // on error, proceed and try to create picture from replacement image
                if( xShape.is() )
                    return xShape;
            }
        }
    }

    // host application wants to create the shape (do not try failed OLE controls again)
    if( (nShapeType == VML_SHAPETYPE_HOSTCONTROL) && !pControlInfo )
    {
        OSL_ENSURE( getClientData(), "ComplexShape::implConvertAndInsert - missing client data" );
        Reference< XShape > xShape = mrDrawing.createAndInsertClientXShape( *this, rxShapes, rShapeRect );
        if( xShape.is() )
            return xShape;
    }

    // try to create a picture object
    if( aGraphicPath.getLength() > 0 )
    {
        Reference< XShape > xShape = mrDrawing.createAndInsertXShape( CREATE_OUSTRING( "com.sun.star.drawing.GraphicObjectShape" ), rxShapes, rShapeRect );
        if( xShape.is() )
        {
            OUString aGraphicUrl = rFilter.getGraphicHelper().importEmbeddedGraphicObject( aGraphicPath );
            if( aGraphicUrl.getLength() > 0 )
            {
                PropertySet aPropSet( xShape );
                aPropSet.setProperty( PROP_GraphicURL, aGraphicUrl );
            }
        }
        return xShape;
    }

    // default: try to create a custom shape
    return CustomShape::implConvertAndInsert( rxShapes, rShapeRect );
}

// ============================================================================

GroupShape::GroupShape( Drawing& rDrawing ) :
    ShapeBase( rDrawing ),
    mxChildren( new ShapeContainer( rDrawing ) )
{
}

GroupShape::~GroupShape()
{
}

void GroupShape::finalizeFragmentImport()
{
    // basic shape processing
    ShapeBase::finalizeFragmentImport();
    // finalize all child shapes
    mxChildren->finalizeFragmentImport();
}

const ShapeType* GroupShape::getChildTypeById( const OUString& rShapeId ) const
{
    return mxChildren->getShapeTypeById( rShapeId, true );
}

const ShapeBase* GroupShape::getChildById( const OUString& rShapeId ) const
{
    return mxChildren->getShapeById( rShapeId, true );
}

Reference< XShape > GroupShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    Reference< XShape > xGroupShape;
    // check that this shape contains children and a valid coordinate system
    ShapeParentAnchor aParentAnchor;
    aParentAnchor.maShapeRect = rShapeRect;
    aParentAnchor.maCoordSys = getCoordSystem();
    if( !mxChildren->empty() && (aParentAnchor.maCoordSys.Width > 0) && (aParentAnchor.maCoordSys.Height > 0) ) try
    {
        xGroupShape = mrDrawing.createAndInsertXShape( CREATE_OUSTRING( "com.sun.star.drawing.GroupShape" ), rxShapes, rShapeRect );
        Reference< XShapes > xChildShapes( xGroupShape, UNO_QUERY_THROW );
        mxChildren->convertAndInsert( xChildShapes, &aParentAnchor );
        // no child shape has been created - delete the group shape
        if( !xChildShapes->hasElements() )
        {
            rxShapes->remove( xGroupShape );
            xGroupShape.clear();
        }
    }
    catch( Exception& )
    {
    }
    return xGroupShape;
}

// ============================================================================

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */