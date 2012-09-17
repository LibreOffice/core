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

#include "oox/vml/vmlshape.hxx"

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include "oox/drawingml/shapepropertymap.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/ole/axcontrolfragment.hxx"
#include "oox/ole/oleobjecthelper.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/vml/vmltextbox.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/containerhelper.hxx"

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;

namespace oox {
namespace vml {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

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

} // namespace

// ============================================================================

ShapeTypeModel::ShapeTypeModel():
    mbAutoHeight( sal_False ),
    mbVisible( sal_True )
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

    sal_Int32 nHeight = ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maHeight, 0, false, true );
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
    if( !maTypeModel.maShapeName.isEmpty() )
        return maTypeModel.maShapeName;

    OUString aBaseName = mrDrawing.getShapeBaseName( *this );
    if( !aBaseName.isEmpty() )
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
                // set imported or generated shape name (not supported by form controls)
                PropertySet aShapeProp( xShape );
                if( aShapeProp.hasProperty( PROP_Name ) )
                    aShapeProp.setProperty( PROP_Name, getShapeName() );
                Reference< XControlShape > xControlShape( xShape, uno::UNO_QUERY );
                if ( xControlShape.is() && !getTypeModel().mbVisible )
                {
                    PropertySet aControlShapeProp( xControlShape->getControl() );
                    aControlShapeProp.setProperty( PROP_EnableVisible, uno::makeAny( sal_False ) );
                }
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
    ::oox::drawingml::ShapePropertyMap aPropMap( mrDrawing.getFilter().getModelObjectHelper() );
    const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();
    maTypeModel.maStrokeModel.pushToPropMap( aPropMap, rGraphicHelper );
    maTypeModel.maFillModel.pushToPropMap( aPropMap, rGraphicHelper );

    // TextFrames have FillColor, not BackColor
    uno::Reference<lang::XServiceInfo> xSInfo(rxShape, uno::UNO_QUERY_THROW);
    if (xSInfo->supportsService("com.sun.star.text.TextFrame") && aPropMap.hasProperty(PROP_FillColor))
    {
        aPropMap.setProperty(PROP_BackColor, aPropMap[PROP_FillColor]);
        aPropMap.erase(PROP_FillColor);
    }

    PropertySet( rxShape ).setProperties( aPropMap );
}

// ============================================================================

SimpleShape::SimpleShape( Drawing& rDrawing, const OUString& rService ) :
    ShapeBase( rDrawing ),
    maService( rService )
{
}

void lcl_SetAnchorType(PropertySet& rPropSet, const ShapeTypeModel& rTypeModel)
{
    if ( rTypeModel.maPosition == "absolute" )
    {
        if (rTypeModel.moWrapAnchorX.get() == "page" && rTypeModel.moWrapAnchorY.get() == "page")
        {
            // I'm not sure if AT_PAGE is always correct here (not sure what the parent that
            // the spec talks about can be), but with Writer SwXDrawPage::add()
            // always in practice uses this because of pDoc->GetCurrentLayout() being NULL at this point.
            rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AT_PAGE);
        }
        else
        {
            // Map to as-character by default, that fixes vertical position of some textframes.
            rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AT_CHARACTER);
        }
        // Vertical placement relative to margin, because parent style must not modify vertical position
        rPropSet.setProperty(PROP_VertOrientRelation, text::RelOrientation::FRAME);
    }
    else if( rTypeModel.maPosition == "relative" )
    {   // I'm not very sure this is correct either.
        rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AT_PARAGRAPH);
    }
    else // static (is the default) means anchored inline
    {
        rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AS_CHARACTER);
    }
}

Reference< XShape > SimpleShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    Rectangle aShapeRect(rShapeRect);
    if (!maTypeModel.maFlip.isEmpty())
    {
        if (maTypeModel.maFlip.equalsAscii("x"))
        {
            aShapeRect.X += aShapeRect.Width;
            aShapeRect.Width *= -1;
        }
        else if (maTypeModel.maFlip.equalsAscii("y"))
        {
            aShapeRect.Y += aShapeRect.Height;
            aShapeRect.Height *= -1;
        }
    }

    Reference< XShape > xShape = mrDrawing.createAndInsertXShape( maService, rxShapes, aShapeRect );
    convertShapeProperties( xShape );

    if ( maService.equalsAscii( "com.sun.star.text.TextFrame" ) )
    {
        PropertySet( xShape ).setAnyProperty( PROP_FrameIsAutomaticHeight, makeAny( maTypeModel.mbAutoHeight ) );
        PropertySet( xShape ).setAnyProperty( PROP_SizeType, makeAny( maTypeModel.mbAutoHeight ? SizeType::MIN : SizeType::FIX ) );
        if (maTypeModel.maPositionHorizontal == "center")
            PropertySet(xShape).setAnyProperty(PROP_HoriOrient, makeAny(text::HoriOrientation::CENTER));
    }

    // Import Legacy Fragments (if any)
    if( xShape.is() && !maShapeModel.maLegacyDiagramPath.isEmpty() )
    {
        Reference< XInputStream > xInStrm( mrDrawing.getFilter().openInputStream( maShapeModel.maLegacyDiagramPath ), UNO_SET_THROW );
        if( xInStrm.is() )
            PropertySet( xShape ).setProperty( PROP_LegacyFragment, xInStrm );
    }

    PropertySet aPropertySet(xShape);
    if (xShape.is() && !maTypeModel.maRotation.isEmpty())
    {
        aPropertySet.setAnyProperty(PROP_RotateAngle, makeAny(maTypeModel.maRotation.toInt32() * 100));
        // If rotation is used, simple setPosition() is not enough.
        aPropertySet.setAnyProperty(PROP_HoriOrientPosition, makeAny( aShapeRect.X ) );
        aPropertySet.setAnyProperty(PROP_VertOrientPosition, makeAny( aShapeRect.Y ) );
    }

    lcl_SetAnchorType(aPropertySet, maTypeModel);

    return xShape;
}

Reference< XShape > SimpleShape::createPictureObject( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect, OUString& rGraphicPath ) const
{
    Reference< XShape > xShape = mrDrawing.createAndInsertXShape( "com.sun.star.drawing.GraphicObjectShape", rxShapes, rShapeRect );
    if( xShape.is() )
    {
        XmlFilterBase& rFilter = mrDrawing.getFilter();
        OUString aGraphicUrl = rFilter.getGraphicHelper().importEmbeddedGraphicObject( rGraphicPath );
        PropertySet aPropSet( xShape );
        if( !aGraphicUrl.isEmpty() )
        {
            aPropSet.setProperty( PROP_GraphicURL, aGraphicUrl );
        }
        // If the shape has an absolute position, set the properties accordingly.
        if ( maTypeModel.maPosition == "absolute" )
        {
            aPropSet.setProperty(PROP_HoriOrientPosition, rShapeRect.X);
            aPropSet.setProperty(PROP_VertOrientPosition, rShapeRect.Y);
            aPropSet.setProperty(PROP_Opaque, sal_False);
        }

        lcl_SetAnchorType(aPropSet, maTypeModel);

        if ( maTypeModel.maPositionVerticalRelative == "page" )
        {
            aPropSet.setProperty(PROP_VertOrientRelation, text::RelOrientation::PAGE_FRAME);
        }
    }
    return xShape;
}

// ============================================================================

RectangleShape::RectangleShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, CREATE_OUSTRING( "com.sun.star.drawing.RectangleShape" ) )
{
}

Reference<XShape> RectangleShape::implConvertAndInsert(const Reference<XShapes>& rxShapes, const Rectangle& rShapeRect) const
{
    OUString aGraphicPath = getGraphicPath();

    // try to create a picture object
    if(!aGraphicPath.isEmpty())
        return SimpleShape::createPictureObject(rxShapes, rShapeRect, aGraphicPath);

    // default: try to create a rectangle shape
    return SimpleShape::implConvertAndInsert(rxShapes, rShapeRect);
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

LineShape::LineShape(Drawing& rDrawing)
    : SimpleShape(rDrawing, "com.sun.star.drawing.LineShape")
{
}

Reference<XShape> LineShape::implConvertAndInsert(const Reference<XShapes>& rxShapes, const Rectangle& rShapeRect) const
{
    const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();
    Rectangle aShapeRect(rShapeRect);
    sal_Int32 nIndex = 0;

    aShapeRect.X = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maShapeModel.maFrom.getToken(0, ',', nIndex), 0, true, true);
    aShapeRect.Y = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maShapeModel.maFrom.getToken(0, ',', nIndex), 0, false, true);
    nIndex = 0;
    aShapeRect.Width = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maShapeModel.maTo.getToken(0, ',', nIndex), 0, true, true) - aShapeRect.X;
    aShapeRect.Height = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maShapeModel.maTo.getToken(0, ',', nIndex), 0, false, true) - aShapeRect.Y;

    return SimpleShape::implConvertAndInsert(rxShapes, aShapeRect);
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
                if( !aGraphicPath.isEmpty() )
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
    if( pControlInfo && !pControlInfo->maFragmentPath.isEmpty() )
    {
        OSL_ENSURE( nShapeType == VML_SHAPETYPE_HOSTCONTROL, "ComplexShape::implConvertAndInsert - unexpected shape type" );
        OUString aShapeName = getShapeName();
        if( !aShapeName.isEmpty() )
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
    if( !aGraphicPath.isEmpty() )
        return SimpleShape::createPictureObject(rxShapes, rShapeRect, aGraphicPath);

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
