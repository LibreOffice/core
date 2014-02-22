/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <algorithm>
#include <boost/optional.hpp>

#include "oox/vml/vmlshape.hxx"

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <svx/svdtrans.hxx>
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



using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

using ::oox::core::XmlFilterBase;



namespace {

const sal_Int32 VML_SHAPETYPE_PICTUREFRAME  = 75;
const sal_Int32 VML_SHAPETYPE_HOSTCONTROL   = 201;



awt::Point lclGetAbsPoint( const awt::Point& rRelPoint, const awt::Rectangle& rShapeRect, const awt::Rectangle& rCoordSys )
{
    double fWidthRatio = static_cast< double >( rShapeRect.Width ) / rCoordSys.Width;
    double fHeightRatio = static_cast< double >( rShapeRect.Height ) / rCoordSys.Height;
    awt::Point aAbsPoint;
    aAbsPoint.X = static_cast< sal_Int32 >( rShapeRect.X + fWidthRatio * (rRelPoint.X - rCoordSys.X) + 0.5 );
    aAbsPoint.Y = static_cast< sal_Int32 >( rShapeRect.Y + fHeightRatio * (rRelPoint.Y - rCoordSys.Y) + 0.5 );
    return aAbsPoint;
}

awt::Rectangle lclGetAbsRect( const awt::Rectangle& rRelRect, const awt::Rectangle& rShapeRect, const awt::Rectangle& rCoordSys )
{
    double fWidthRatio = static_cast< double >( rShapeRect.Width ) / rCoordSys.Width;
    double fHeightRatio = static_cast< double >( rShapeRect.Height ) / rCoordSys.Height;
    awt::Rectangle aAbsRect;
    aAbsRect.X = static_cast< sal_Int32 >( rShapeRect.X + fWidthRatio * (rRelRect.X - rCoordSys.X) + 0.5 );
    aAbsRect.Y = static_cast< sal_Int32 >( rShapeRect.Y + fHeightRatio * (rRelRect.Y - rCoordSys.Y) + 0.5 );
    aAbsRect.Width = static_cast< sal_Int32 >( fWidthRatio * rRelRect.Width + 0.5 );
    aAbsRect.Height = static_cast< sal_Int32 >( fHeightRatio * rRelRect.Height + 0.5 );
    return aAbsRect;
}

} 



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

awt::Rectangle ShapeType::getCoordSystem() const
{
    Int32Pair aCoordPos = maTypeModel.moCoordPos.get( Int32Pair( 0, 0 ) );
    Int32Pair aCoordSize = maTypeModel.moCoordSize.get( Int32Pair( 1000, 1000 ) );
    if( aCoordSize.first == 0 )
        aCoordSize.first = 1;
    if( aCoordSize.second == 0 )
        aCoordSize.second = 1;
    return awt::Rectangle( aCoordPos.first, aCoordPos.second, aCoordSize.first, aCoordSize.second );
}

awt::Rectangle ShapeType::getRectangle( const ShapeParentAnchor* pParentAnchor ) const
{
    return pParentAnchor ?
        lclGetAbsRect( getRelRectangle(), pParentAnchor->maShapeRect, pParentAnchor->maCoordSys ) :
        getAbsRectangle();
}

awt::Rectangle ShapeType::getAbsRectangle() const
{
    const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();

    sal_Int32 nWidth = ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maWidth, 0, true, true );
    if ( nWidth == 0 )
        nWidth = 1;

    sal_Int32 nHeight = ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maHeight, 0, false, true );
    if ( nHeight == 0 )
        nHeight = 1;

    sal_Int32 nLeft = ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maLeft, 0, true, true )
        + ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maMarginLeft, 0, true, true );
    if (nLeft == 0 && maTypeModel.maPosition == "absolute")
        nLeft = 1;

    return awt::Rectangle(
        nLeft,
        ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maTop, 0, false, true ) + ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maTypeModel.maMarginTop, 0, false, true ),
        nWidth, nHeight );
}

awt::Rectangle ShapeType::getRelRectangle() const
{
    sal_Int32 nWidth = maTypeModel.maWidth.toInt32();
    if ( nWidth == 0 )
        nWidth = 1;

    sal_Int32 nHeight = maTypeModel.maHeight.toInt32();
    if ( nHeight == 0 )
        nHeight = 1;

    return awt::Rectangle(
        maTypeModel.maLeft.toInt32(),
        maTypeModel.maTop.toInt32(),
        nWidth, nHeight );
}



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



ShapeModel::ShapeModel()
{
}

ShapeModel::~ShapeModel()
{
}

TextBox& ShapeModel::createTextBox(ShapeTypeModel& rModel)
{
    mxTextBox.reset( new TextBox(rModel) );
    return *mxTextBox;
}

ClientData& ShapeModel::createClientData()
{
    mxClientData.reset( new ClientData );
    return *mxClientData;
}



ShapeBase::ShapeBase( Drawing& rDrawing ) :
    ShapeType( rDrawing )
{
}

void ShapeBase::finalizeFragmentImport()
{
    if( maShapeModel.maType.getLength() > 1 )
    {
        OUString aType = maShapeModel.maType;
        if (aType[ 0 ] == '#')
            aType = aType.copy(1);
        if( const ShapeType* pShapeType = mrDrawing.getShapes().getShapeTypeById( aType, true ) )
            maTypeModel.assignUsed( pShapeType->getTypeModel() );
    }
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
            return OUStringBuffer( aBaseName ).append( ' ' ).append( nShapeIdx ).makeStringAndClear();
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
        awt::Rectangle aShapeRect = calcShapeRectangle( pParentAnchor );

        if( ((aShapeRect.Width > 0) || (aShapeRect.Height > 0)) && rxShapes.is() )
        {
            xShape = implConvertAndInsert( rxShapes, aShapeRect );
            if( xShape.is() )
            {
                
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
        else
            SAL_WARN("oox", "not converting shape, as calculated rectangle is empty");
    }
    return xShape;
}

void ShapeBase::convertFormatting( const Reference< XShape >& rxShape, const ShapeParentAnchor* pParentAnchor ) const
{
    if( rxShape.is() )
    {
        /*  Calculate shape rectangle. Applications may do something special
            according to some imported shape client data (e.g. Excel cell anchor). */
        awt::Rectangle aShapeRect = calcShapeRectangle( pParentAnchor );

        
        if( (aShapeRect.Width > 0) || (aShapeRect.Height > 0) )
        {
            rxShape->setPosition( awt::Point( aShapeRect.X, aShapeRect.Y ) );
            rxShape->setSize( awt::Size( aShapeRect.Width, aShapeRect.Height ) );
            convertShapeProperties( rxShape );
        }
    }
}



awt::Rectangle ShapeBase::calcShapeRectangle( const ShapeParentAnchor* pParentAnchor ) const
{
    /*  Calculate shape rectangle. Applications may do something special
        according to some imported shape client data (e.g. Excel cell anchor). */
    awt::Rectangle aShapeRect;
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

    uno::Reference<lang::XServiceInfo> xSInfo(rxShape, uno::UNO_QUERY_THROW);
    if (xSInfo->supportsService("com.sun.star.text.TextFrame"))
    {
        
        maTypeModel.maShadowModel.pushToPropMap(aPropMap, rGraphicHelper);
        
        if (aPropMap.hasProperty(PROP_FillColor))
        {
            aPropMap.setProperty(PROP_BackColor, aPropMap[PROP_FillColor]);
            aPropMap.erase(PROP_FillColor);
        }
        
        if (aPropMap.hasProperty(PROP_FillTransparence))
        {
            aPropMap.setProperty(PROP_BackColorTransparency, aPropMap[PROP_FillTransparence]);
            aPropMap.erase(PROP_FillTransparence);
        }
        
        boost::optional<sal_Int32> oLineWidth;
        if (maTypeModel.maStrokeModel.moWeight.has())
            oLineWidth.reset(ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maTypeModel.maStrokeModel.moWeight.get(), 0, false, false));
        if (aPropMap.hasProperty(PROP_LineColor))
        {
            uno::Reference<beans::XPropertySet> xPropertySet(rxShape, uno::UNO_QUERY);
            static const sal_Int32 aBorders[] = {
                PROP_TopBorder, PROP_LeftBorder, PROP_BottomBorder, PROP_RightBorder
            };
            for (unsigned int i = 0; i < SAL_N_ELEMENTS(aBorders); ++i)
            {
                table::BorderLine2 aBorderLine = xPropertySet->getPropertyValue(PropertyMap::getPropertyName(aBorders[i])).get<table::BorderLine2>();
                aBorderLine.Color = aPropMap[PROP_LineColor].get<sal_Int32>();
                if (oLineWidth)
                    aBorderLine.LineWidth = *oLineWidth;
                aPropMap.setProperty(aBorders[i], uno::makeAny(aBorderLine));
            }
            aPropMap.erase(PROP_LineColor);
        }
    }
    else if (xSInfo->supportsService("com.sun.star.drawing.CustomShape"))
        maTypeModel.maTextpathModel.pushToPropMap(aPropMap, rxShape);

    PropertySet( rxShape ).setProperties( aPropMap );
}



SimpleShape::SimpleShape( Drawing& rDrawing, const OUString& rService ) :
    ShapeBase( rDrawing ),
    maService( rService )
{
}

void lcl_setSurround(PropertySet& rPropSet, const ShapeTypeModel& rTypeModel)
{
    sal_Int32 nSurround = com::sun::star::text::WrapTextMode_THROUGHT;
    if ( rTypeModel.moWrapType.get() == "square" || rTypeModel.moWrapType .get()== "tight" ||
         rTypeModel.moWrapType.get() == "through" )
    {
        nSurround = com::sun::star::text::WrapTextMode_PARALLEL;
        if ( rTypeModel.moWrapSide.get() == "left" )
            nSurround = com::sun::star::text::WrapTextMode_LEFT;
        else if ( rTypeModel.moWrapSide.get() == "right" )
            nSurround = com::sun::star::text::WrapTextMode_RIGHT;
    }
    else if ( rTypeModel.moWrapType.get() == "topAndBottom" )
        nSurround = com::sun::star::text::WrapTextMode_NONE;

    rPropSet.setProperty(PROP_Surround, nSurround);
}

void lcl_SetAnchorType(PropertySet& rPropSet, const ShapeTypeModel& rTypeModel)
{
    if ( rTypeModel.maPositionHorizontal == "center" )
        rPropSet.setAnyProperty(PROP_HoriOrient, makeAny(text::HoriOrientation::CENTER));

    if ( rTypeModel.maPositionHorizontalRelative == "page" )
        rPropSet.setAnyProperty(PROP_HoriOrientRelation, makeAny(text::RelOrientation::PAGE_FRAME));

    if ( rTypeModel.maPositionVertical == "center" )
        rPropSet.setAnyProperty(PROP_VertOrient, makeAny(text::VertOrientation::CENTER));

    if ( rTypeModel.maPosition == "absolute" )
    {
        
        rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AT_CHARACTER);

        if ( rTypeModel.maPositionVerticalRelative == "page" )
        {
            rPropSet.setProperty(PROP_VertOrientRelation, text::RelOrientation::PAGE_FRAME);
        }
        else if ( rTypeModel.maPositionVerticalRelative == "margin" )
        {
            rPropSet.setProperty(PROP_VertOrientRelation, text::RelOrientation::PAGE_PRINT_AREA);
        }
        else
        {
            
            rPropSet.setProperty(PROP_VertOrientRelation, text::RelOrientation::FRAME);
        }
    }
    else if( rTypeModel.maPosition == "relative" )
    {   
        rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AT_PARAGRAPH);
    }
    else 
    {
        rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AS_CHARACTER);
    }
    lcl_setSurround( rPropSet, rTypeModel );
}

void lcl_SetRotation(PropertySet& rPropSet, const sal_Int32 nRotation)
{
    
    
    rPropSet.setAnyProperty(PROP_RotateAngle, makeAny(sal_Int32(NormAngle360(nRotation * -100))));
}

Reference< XShape > SimpleShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    awt::Rectangle aShapeRect(rShapeRect);
    boost::optional<sal_Int32> oRotation;
    bool bFlipX = false, bFlipY = false;
    if (!maTypeModel.maRotation.isEmpty())
        oRotation.reset(maTypeModel.maRotation.toInt32());
    if (!maTypeModel.maFlip.isEmpty())
    {
        if (maTypeModel.maFlip.equalsAscii("x"))
        {
            bFlipX = true;
        }
        else if (maTypeModel.maFlip.equalsAscii("y"))
        {
            bFlipY = true;
        }
    }

    Reference< XShape > xShape = mrDrawing.createAndInsertXShape( maService, rxShapes, aShapeRect );
    convertShapeProperties( xShape );

    
    
    
    
    const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();
    OUString aWrapDistanceLeft = OUString::number(0x0001BE7C);
    if (!maTypeModel.maWrapDistanceLeft.isEmpty())
        aWrapDistanceLeft = maTypeModel.maWrapDistanceLeft;
    sal_Int32 nWrapDistanceLeft = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, aWrapDistanceLeft, 0, true, false);
    PropertySet(xShape).setAnyProperty(PROP_LeftMargin, uno::makeAny(nWrapDistanceLeft));
    OUString aWrapDistanceRight = OUString::number(0x0001BE7C);
    if (!maTypeModel.maWrapDistanceRight.isEmpty())
        aWrapDistanceRight = maTypeModel.maWrapDistanceRight;
    sal_Int32 nWrapDistanceRight = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, aWrapDistanceRight, 0, true, false);
    PropertySet(xShape).setAnyProperty(PROP_RightMargin, uno::makeAny(nWrapDistanceRight));
    sal_Int32 nWrapDistanceTop = 0;
    if (!maTypeModel.maWrapDistanceTop.isEmpty())
        nWrapDistanceTop = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maTypeModel.maWrapDistanceTop, 0, false, true);
    PropertySet(xShape).setAnyProperty(PROP_TopMargin, uno::makeAny(nWrapDistanceTop));
    sal_Int32 nWrapDistanceBottom = 0;
    if (!maTypeModel.maWrapDistanceBottom.isEmpty())
        nWrapDistanceBottom = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maTypeModel.maWrapDistanceBottom, 0, false, true);
    PropertySet(xShape).setAnyProperty(PROP_BottomMargin, uno::makeAny(nWrapDistanceBottom));

    if ( maService.equalsAscii( "com.sun.star.text.TextFrame" ) )
    {
        PropertySet( xShape ).setAnyProperty( PROP_FrameIsAutomaticHeight, makeAny( maTypeModel.mbAutoHeight ) );
        PropertySet( xShape ).setAnyProperty( PROP_SizeType, makeAny( maTypeModel.mbAutoHeight ? SizeType::MIN : SizeType::FIX ) );
        if( getTextBox()->borderDistanceSet )
        {
            PropertySet( xShape ).setAnyProperty( PROP_LeftBorderDistance, makeAny( sal_Int32( getTextBox()->borderDistanceLeft )));
            PropertySet( xShape ).setAnyProperty( PROP_TopBorderDistance, makeAny( sal_Int32( getTextBox()->borderDistanceTop )));
            PropertySet( xShape ).setAnyProperty( PROP_RightBorderDistance, makeAny( sal_Int32( getTextBox()->borderDistanceRight )));
            PropertySet( xShape ).setAnyProperty( PROP_BottomBorderDistance, makeAny( sal_Int32( getTextBox()->borderDistanceBottom )));
        }
        if (!maTypeModel.maLayoutFlowAlt.isEmpty())
        {
            
            uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
            uno::Sequence<beans::PropertyValue> aGrabBag;
            xPropertySet->getPropertyValue("FrameInteropGrabBag") >>= aGrabBag;
            beans::PropertyValue aPair;
            aPair.Name = "mso-layout-flow-alt";
            aPair.Value = uno::makeAny(maTypeModel.maLayoutFlowAlt);
            if (aGrabBag.hasElements())
            {
                sal_Int32 nLength = aGrabBag.getLength();
                aGrabBag.realloc(nLength + 1);
                aGrabBag[nLength] = aPair;
            }
            else
            {
                aGrabBag.realloc(1);
                aGrabBag[0] = aPair;
            }
            xPropertySet->setPropertyValue("FrameInteropGrabBag", uno::makeAny(aGrabBag));
        }
    }
    else
    {
        
        
        

        
        if ( !maTypeModel.maWidthPercent.isEmpty( ) )
        {
            
            if ( maTypeModel.maWidthRelative.isEmpty() || maTypeModel.maWidthRelative == "page" )
            {
                sal_Int16 nWidth = maTypeModel.maWidthPercent.toInt32() / 10;
                
                if ( nWidth )
                    PropertySet( xShape ).setAnyProperty(PROP_RelativeWidth, makeAny( nWidth ) );
            }
        }
        if ( !maTypeModel.maHeightPercent.isEmpty( ) )
        {
            
            if ( maTypeModel.maHeightRelative.isEmpty() || maTypeModel.maHeightRelative == "page" )
            {
                sal_Int16 nHeight = maTypeModel.maHeightPercent.toInt32() / 10;
                
                if ( nHeight )
                    PropertySet( xShape ).setAnyProperty(PROP_RelativeHeight, makeAny( nHeight ) );
            }
        }

        
        drawing::TextVerticalAdjust eTextVerticalAdjust = drawing::TextVerticalAdjust_TOP;
        if (maTypeModel.maVTextAnchor == "middle")
            eTextVerticalAdjust = drawing::TextVerticalAdjust_CENTER;
        else if (maTypeModel.maVTextAnchor == "bottom")
            eTextVerticalAdjust = drawing::TextVerticalAdjust_BOTTOM;
        PropertySet(xShape).setAnyProperty(PROP_TextVerticalAdjust, makeAny(eTextVerticalAdjust));

        if (getTextBox())
        {
            getTextBox()->convert(xShape);
            if (getTextBox()->borderDistanceSet)
            {
                awt::Size aSize = xShape->getSize();
                PropertySet(xShape).setAnyProperty(PROP_TextLeftDistance, makeAny(sal_Int32(getTextBox()->borderDistanceLeft)));
                PropertySet(xShape).setAnyProperty(PROP_TextUpperDistance, makeAny(sal_Int32(getTextBox()->borderDistanceTop)));
                PropertySet(xShape).setAnyProperty(PROP_TextRightDistance, makeAny(sal_Int32(getTextBox()->borderDistanceRight)));
                PropertySet(xShape).setAnyProperty(PROP_TextLowerDistance, makeAny(sal_Int32(getTextBox()->borderDistanceBottom)));
                xShape->setSize(aSize);
            }
        }
    }

    
    if( xShape.is() && !maShapeModel.maLegacyDiagramPath.isEmpty() )
    {
        Reference< XInputStream > xInStrm( mrDrawing.getFilter().openInputStream( maShapeModel.maLegacyDiagramPath ), UNO_SET_THROW );
        if( xInStrm.is() )
            PropertySet( xShape ).setProperty( PROP_LegacyFragment, xInStrm );
    }

    PropertySet aPropertySet(xShape);
    if (xShape.is())
    {
        if (oRotation)
        {
            lcl_SetRotation(aPropertySet, *oRotation);
            uno::Reference<lang::XServiceInfo> xServiceInfo(rxShapes, uno::UNO_QUERY);
            if (!xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
            {
                
                aPropertySet.setAnyProperty(PROP_HoriOrientPosition, makeAny(aShapeRect.X));
                aPropertySet.setAnyProperty(PROP_VertOrientPosition, makeAny(aShapeRect.Y));
            }
        }

        
        
        
        if (bFlipX || bFlipY)
        {
            com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aPropSequence (2);
            int nPropertyIndex = 0;
            if (bFlipX)
            {
                aPropSequence [nPropertyIndex].Name = "MirroredX";
                aPropSequence [nPropertyIndex].Value = makeAny (bFlipX);
                nPropertyIndex++;
            }
            if (bFlipY)
            {
                aPropSequence [nPropertyIndex].Name = "MirroredY";
                aPropSequence [nPropertyIndex].Value = makeAny (bFlipY);
                nPropertyIndex++;
            }
            aPropertySet.setAnyProperty(PROP_CustomShapeGeometry, makeAny( aPropSequence ) );
        }
    }

    lcl_SetAnchorType(aPropertySet, maTypeModel);

    return xShape;
}

Reference< XShape > SimpleShape::createPictureObject( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect, OUString& rGraphicPath ) const
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
        uno::Reference< lang::XServiceInfo > xServiceInfo(rxShapes, uno::UNO_QUERY);
        
        if ( maTypeModel.maPosition == "absolute" && !xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
        {
            aPropSet.setProperty(PROP_HoriOrientPosition, rShapeRect.X);
            aPropSet.setProperty(PROP_VertOrientPosition, rShapeRect.Y);
            aPropSet.setProperty(PROP_Opaque, sal_False);
        }
        
        if ( !maTypeModel.maRotation.isEmpty() )
            lcl_SetRotation( aPropSet, maTypeModel.maRotation.toInt32() );

        lcl_SetAnchorType(aPropSet, maTypeModel);
    }
    return xShape;
}



RectangleShape::RectangleShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, "com.sun.star.drawing.RectangleShape" )
{
}

Reference<XShape> RectangleShape::implConvertAndInsert(const Reference<XShapes>& rxShapes, const awt::Rectangle& rShapeRect) const
{
    OUString aGraphicPath = getGraphicPath();

    
    if(!aGraphicPath.isEmpty())
        return SimpleShape::createPictureObject(rxShapes, rShapeRect, aGraphicPath);

    
    Reference<XShape> xShape = SimpleShape::implConvertAndInsert(rxShapes, rShapeRect);
    OUString sArcsize = maTypeModel.maArcsize;
    if ( !sArcsize.isEmpty( ) )
    {
        sal_Unicode cLastChar = sArcsize[sArcsize.getLength() - 1];
        sal_Int32 nValue = sArcsize.copy( 0, sArcsize.getLength() - 1 ).toInt32( );
        
        double size = std::min( rShapeRect.Height, rShapeRect.Width ) / 2.0;
        sal_Int32 nRadius = 0;
        if ( cLastChar == 'f' )
            nRadius = size * nValue / 65536;
        else if ( cLastChar == '%' )
            nRadius = size * nValue / 100;
        PropertySet( xShape ).setAnyProperty( PROP_CornerRadius, makeAny( nRadius ) );
    }
    return xShape;
}



EllipseShape::EllipseShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, "com.sun.star.drawing.EllipseShape" )
{
}



PolyLineShape::PolyLineShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, "com.sun.star.drawing.PolyLineShape" )
{
}

Reference< XShape > PolyLineShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    Reference< XShape > xShape = SimpleShape::implConvertAndInsert( rxShapes, rShapeRect );
    
    awt::Rectangle aCoordSys = getCoordSystem();
    if( !maShapeModel.maPoints.empty() && (aCoordSys.Width > 0) && (aCoordSys.Height > 0) )
    {
        ::std::vector< awt::Point > aAbsPoints;
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

awt::Rectangle LineShape::getAbsRectangle() const
{
    const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();
    awt::Rectangle aShapeRect;
    sal_Int32 nIndex = 0;

    aShapeRect.X = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maShapeModel.maFrom.getToken(0, ',', nIndex), 0, true, true);
    aShapeRect.Y = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maShapeModel.maFrom.getToken(0, ',', nIndex), 0, false, true);
    nIndex = 0;
    aShapeRect.Width = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maShapeModel.maTo.getToken(0, ',', nIndex), 0, true, true) - aShapeRect.X;
    aShapeRect.Height = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maShapeModel.maTo.getToken(0, ',', nIndex), 0, false, true) - aShapeRect.Y;
    return aShapeRect;
}

awt::Rectangle LineShape::getRelRectangle() const
{
    awt::Rectangle aShapeRect;
    sal_Int32 nIndex = 0;

    aShapeRect.X = maShapeModel.maFrom.getToken(0, ',', nIndex).toInt32();
    aShapeRect.Y = maShapeModel.maFrom.getToken(0, ',', nIndex).toInt32();
    nIndex = 0;
    aShapeRect.Width = maShapeModel.maTo.getToken(0, ',', nIndex).toInt32() - aShapeRect.X;
    aShapeRect.Height = maShapeModel.maTo.getToken(0, ',', nIndex).toInt32() - aShapeRect.Y;
    return aShapeRect;
}



BezierShape::BezierShape(Drawing& rDrawing)
    : SimpleShape(rDrawing, "com.sun.star.drawing.OpenBezierShape")
{
}

Reference< XShape > BezierShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    
    sal_Int32 nPos = maShapeModel.maVmlPath.lastIndexOf(',');
    if ( nPos != -1 && maShapeModel.maVmlPath.copy(nPos).indexOf('x') != -1 )
    {
        const_cast<BezierShape*>( this )->setService( "com.sun.star.drawing.ClosedBezierShape" );
    }

    awt::Rectangle aCoordSys = getCoordSystem();
    PolyPolygonBezierCoords aBezierCoords;

    if( (aCoordSys.Width > 0) && (aCoordSys.Height > 0) )
    {
        const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();

        
        typedef ::std::vector< ::std::vector< awt::Point > > SubPathList;
        typedef ::std::vector< ::std::vector< PolygonFlags > > FlagsList;
        SubPathList aCoordLists;
        FlagsList aFlagLists;
        sal_Int32 nIndex = 0;

        
        if ( maShapeModel.maVmlPath.isEmpty() )
        {
            aCoordLists.push_back( ::std::vector< awt::Point >() );
            aFlagLists.push_back( ::std::vector< PolygonFlags >() );

            
            aCoordLists[ 0 ].push_back(
                awt::Point(ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maFrom.getToken( 0, ',', nIndex ), 0, true, true ),
                  ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maFrom.getToken( 0, ',', nIndex ), 0, false, true ) ) );
            
            aCoordLists[ 0 ].push_back(
                awt::Point( ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maControl1.getToken( 0, ',', nIndex ), 0, true, true ),
                      ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maControl1.getToken( 0, ',', nIndex ), 0, false, true ) ) );
            
            aCoordLists[ 0 ].push_back(
                awt::Point( ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maControl2.getToken( 0, ',', nIndex ), 0, true, true ),
                      ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maControl2.getToken( 0, ',', nIndex ), 0, false, true ) ) );
            
            aCoordLists[ 0 ].push_back(
                awt::Point( ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maTo.getToken( 0, ',', nIndex ), 0, true, true ),
                      ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maTo.getToken( 0, ',', nIndex ), 0, false, true ) ) );

            
            aFlagLists[ 0 ].resize( aCoordLists[ 0 ].size(), PolygonFlags_CONTROL );
            aFlagLists[ 0 ][ 0 ] = PolygonFlags_NORMAL;
            aFlagLists[ 0 ].back() = PolygonFlags_NORMAL;
        }
        
        else
        {
            
            ConversionHelper::decodeVmlPath( aCoordLists, aFlagLists, maShapeModel.maVmlPath );

            for ( SubPathList::iterator aListIt = aCoordLists.begin(); aListIt != aCoordLists.end(); ++aListIt )
                for ( ::std::vector< awt::Point >::iterator aPointIt = (*aListIt).begin(); aPointIt != (*aListIt).end(); ++aPointIt)
                {
                    (*aPointIt) = lclGetAbsPoint( (*aPointIt), rShapeRect, aCoordSys );
                }
        }

        aBezierCoords.Coordinates.realloc( aCoordLists.size() );
        for ( unsigned int i = 0; i < aCoordLists.size(); i++ )
            aBezierCoords.Coordinates[i] = ContainerHelper::vectorToSequence( aCoordLists[i] );

        aBezierCoords.Flags.realloc( aFlagLists.size() );
        for ( unsigned int i = 0; i < aFlagLists.size(); i++ )
            aBezierCoords.Flags[i] = ContainerHelper::vectorToSequence( aFlagLists[i] );

        if( !aCoordLists.front().empty() && !aCoordLists.back().empty()
            && aCoordLists.front().front().X == aCoordLists.back().back().X
            && aCoordLists.front().front().Y == aCoordLists.back().back().Y )
        { 
          
            const_cast< BezierShape* >( this )->setService( "com.sun.star.drawing.ClosedBezierShape" );
        }
    }

    Reference< XShape > xShape = SimpleShape::implConvertAndInsert( rxShapes, rShapeRect );

    if( aBezierCoords.Coordinates.hasElements())
    {
        PropertySet aPropSet( xShape );
        aPropSet.setProperty( PROP_PolyPolygonBezier, aBezierCoords );
    }

    
    xShape->setSize( awt::Size( rShapeRect.Width, rShapeRect.Height ) );
    xShape->setPosition( awt::Point( rShapeRect.X, rShapeRect.Y ) );
    return xShape;
}



CustomShape::CustomShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, "com.sun.star.drawing.CustomShape" )
{
}

Reference< XShape > CustomShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    
    Reference< XShape > xShape = SimpleShape::implConvertAndInsert( rxShapes, rShapeRect );
    if( xShape.is() ) try
    {
        
        Reference< XEnhancedCustomShapeDefaulter > xDefaulter( xShape, UNO_QUERY_THROW );
        xDefaulter->createCustomShapeDefaults( OUString::number( getShapeType() ) );
        
        convertShapeProperties( xShape );
    }
    catch( Exception& )
    {
    }
    return xShape;
}



ComplexShape::ComplexShape( Drawing& rDrawing ) :
    CustomShape( rDrawing )
{
}

Reference< XShape > ComplexShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    XmlFilterBase& rFilter = mrDrawing.getFilter();
    sal_Int32 nShapeType = getShapeType();
    OUString aGraphicPath = getGraphicPath();

    
    if( const OleObjectInfo* pOleObjectInfo = mrDrawing.getOleObjectInfo( maTypeModel.maShapeId ) )
    {
        SAL_WARN_IF(
            nShapeType != VML_SHAPETYPE_PICTUREFRAME, "oox",
            "ComplexShape::implConvertAndInsert - unexpected shape type");

        
        if( pOleObjectInfo->mbDmlShape )
            return Reference< XShape >();

        PropertyMap aOleProps;
        awt::Size aOleSize( rShapeRect.Width, rShapeRect.Height );
        if( rFilter.getOleObjectHelper().importOleObject( aOleProps, *pOleObjectInfo, aOleSize ) )
        {
            Reference< XShape > xShape = mrDrawing.createAndInsertXShape( "com.sun.star.drawing.OLE2Shape", rxShapes, rShapeRect );
            if( xShape.is() )
            {
                
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

    
    const ControlInfo* pControlInfo = mrDrawing.getControlInfo( maTypeModel.maShapeId );
    if( pControlInfo && !pControlInfo->maFragmentPath.isEmpty() )
    {
        OSL_ENSURE( nShapeType == VML_SHAPETYPE_HOSTCONTROL, "ComplexShape::implConvertAndInsert - unexpected shape type" );
        OUString aShapeName = getShapeName();
        if( !aShapeName.isEmpty() )
        {
            OSL_ENSURE( aShapeName == pControlInfo->maName, "ComplexShape::implConvertAndInsert - control name mismatch" );
            
            ::oox::ole::EmbeddedControl aControl( aShapeName );
            if( rFilter.importFragment( new ::oox::ole::AxControlFragment( rFilter, pControlInfo->maFragmentPath, aControl ) ) )
            {
                
                sal_Int32 nCtrlIndex = -1;
                Reference< XShape > xShape = mrDrawing.createAndInsertXControlShape( aControl, rxShapes, rShapeRect, nCtrlIndex );
                
                if( xShape.is() )
                    return xShape;
            }
        }
    }

    
    if( (nShapeType == VML_SHAPETYPE_HOSTCONTROL) && !pControlInfo )
    {
        OSL_ENSURE( getClientData(), "ComplexShape::implConvertAndInsert - missing client data" );
        Reference< XShape > xShape = mrDrawing.createAndInsertClientXShape( *this, rxShapes, rShapeRect );
        if( xShape.is() )
            return xShape;
    }

    
    if( !aGraphicPath.isEmpty() )
        return SimpleShape::createPictureObject(rxShapes, rShapeRect, aGraphicPath);

    
    return CustomShape::implConvertAndInsert( rxShapes, rShapeRect );
}



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
    
    ShapeBase::finalizeFragmentImport();
    
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

Reference< XShape > GroupShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    Reference< XShape > xGroupShape;
    
    ShapeParentAnchor aParentAnchor;
    aParentAnchor.maShapeRect = rShapeRect;
    aParentAnchor.maCoordSys = getCoordSystem();
    if( !mxChildren->empty() && (aParentAnchor.maCoordSys.Width > 0) && (aParentAnchor.maCoordSys.Height > 0) ) try
    {
        xGroupShape = mrDrawing.createAndInsertXShape( "com.sun.star.drawing.GroupShape", rxShapes, rShapeRect );
        Reference< XShapes > xChildShapes( xGroupShape, UNO_QUERY_THROW );
        mxChildren->convertAndInsert( xChildShapes, &aParentAnchor );
        if( !xChildShapes->hasElements() )
        {
            SAL_WARN("oox", "no child shape has been created - deleting the group shape");
            rxShapes->remove( xGroupShape );
            xGroupShape.clear();
        }
    }
    catch( Exception& )
    {
    }

    if (!maTypeModel.maEditAs.isEmpty())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xGroupShape, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aGrabBag;
        xPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
        beans::PropertyValue aPair;
        aPair.Name = "mso-edit-as";
        aPair.Value = uno::makeAny(maTypeModel.maEditAs);
       if (aGrabBag.hasElements())
       {
            sal_Int32 nLength = aGrabBag.getLength();
            aGrabBag.realloc(nLength + 1);
            aGrabBag[nLength] = aPair;
       }
       else
       {
           aGrabBag.realloc(1);
           aGrabBag[0] = aPair;
       }
       xPropertySet->setPropertyValue("InteropGrabBag", uno::makeAny(aGrabBag));
    }
    
    PropertySet aPropertySet(xGroupShape);
    lcl_SetAnchorType(aPropertySet, maTypeModel);
    if (!maTypeModel.maRotation.isEmpty())
        lcl_SetRotation(aPropertySet, maTypeModel.maRotation.toInt32());
    return xGroupShape;
}



} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
