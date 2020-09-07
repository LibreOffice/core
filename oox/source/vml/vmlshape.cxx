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

#include <algorithm>

#include <optional>

#include <o3tl/safeint.hxx>
#include <oox/vml/vmlshape.hxx>
#include <vcl/wmfexternal.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <sal/log.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/ole/axcontrol.hxx>
#include <oox/ole/axcontrolfragment.hxx>
#include <oox/ole/oleobjecthelper.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/vml/vmlshapecontainer.hxx>
#include <oox/vml/vmltextbox.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/containerhelper.hxx>
#include <svx/EnhancedCustomShapeTypeNames.hxx>
#include <svx/unoapi.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/svdobj.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/storagehelper.hxx>

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;

namespace oox::vml {

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

/// Count the crop value based on a crop fraction and a reference size.
sal_Int32 lclConvertCrop(const OUString& rCrop, sal_uInt32 nSize)
{
    if (rCrop.endsWith("f"))
    {
        // Numeric value is specified in 1/65536-ths.
        sal_uInt32 nCrop = rCrop.copy(0, rCrop.getLength() - 1).toUInt32();
        return (nCrop * nSize) / 65536;
    }

    return 0;
}

} // namespace

ShapeTypeModel::ShapeTypeModel():
    mbAutoHeight( false ),
    mbVisible( true )
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

    sal_Int32 nLeft;
    if (o3tl::checked_add<sal_Int32>(ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maTypeModel.maLeft, 0, true, true),
                                     ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maTypeModel.maMarginLeft, 0, true, true),
                                     nLeft))
    {
        SAL_WARN("oox", "overflow in addition");
        nLeft = 0;
    }
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
    : mbIsSignatureLine(false)
    , mbSignatureLineShowSignDate(true)
    , mbSignatureLineCanAddComment(false)
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
    if( maShapeModel.maType.getLength() <= 1 )
        return;

    OUString aType = maShapeModel.maType;
    if (aType[ 0 ] == '#')
        aType = aType.copy(1);
    if( const ShapeType* pShapeType = mrDrawing.getShapes().getShapeTypeById( aType ) )
    {
        // Make sure that the props from maTypeModel have priority over the props from
        // the shape type.
        StrokeModel aMergedStrokeModel;
        aMergedStrokeModel.assignUsed(pShapeType->getTypeModel().maStrokeModel);
        aMergedStrokeModel.assignUsed(maTypeModel.maStrokeModel);
        FillModel aMergedFillModel;
        aMergedFillModel.assignUsed(pShapeType->getTypeModel().maFillModel);
        aMergedFillModel.assignUsed(maTypeModel.maFillModel);

        maTypeModel.assignUsed( pShapeType->getTypeModel() );
        maTypeModel.maStrokeModel = aMergedStrokeModel;
        maTypeModel.maFillModel = aMergedFillModel;
    }
    else {
        // Temporary fix, shapetype not found if referenced from different substream
        // FIXME: extend scope of ShapeContainer to store all shapetypes from the document
        const OUString sShapeTypePrefix = "shapetype_";
        if (aType.startsWith(sShapeTypePrefix)) {
            maTypeModel.moShapeType = aType.copy(sShapeTypePrefix.getLength()).toInt32();
        }
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
            return aBaseName + OUStringChar(' ') + OUString::number( nShapeIdx );
    }

    return OUString();
}

const ShapeType* ShapeBase::getChildTypeById( const OUString& ) const
{
    return nullptr;
}

const ShapeBase* ShapeBase::getChildById( const OUString& ) const
{
    return nullptr;
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
                // set imported or generated shape name (not supported by form controls)
                PropertySet aShapeProp( xShape );
                if( aShapeProp.hasProperty( PROP_Name ) )
                    aShapeProp.setProperty( PROP_Name, getShapeName() );
                uno::Reference< lang::XServiceInfo > xSInfo( xShape, uno::UNO_QUERY_THROW );

                OUString sLinkChainName = getTypeModel().maLegacyId;
                sal_Int32 id = 0;
                sal_Int32 idPos = sLinkChainName.indexOf("_x");
                sal_Int32 seq = 0;
                if (idPos >= 0)
                {
                    sal_Int32 seqPos = sLinkChainName.indexOf("_s",idPos);
                    if (idPos < seqPos)
                    {
                        auto idPosEnd = idPos+2;
                        id = sLinkChainName.copy(idPosEnd, seqPos - idPosEnd).toInt32();
                        seq = sLinkChainName.copy(seqPos+2).toInt32();
                    }
                }

                OUString s_mso_next_textbox;
                if( getTextBox() )
                    s_mso_next_textbox = getTextBox()->msNextTextbox;
                if( s_mso_next_textbox.startsWith("#") )
                    s_mso_next_textbox = s_mso_next_textbox.copy(1);

                if (xSInfo->supportsService("com.sun.star.text.TextFrame"))
                {
                    uno::Reference<beans::XPropertySet> propertySet (xShape, uno::UNO_QUERY);
                    uno::Any aAny = propertySet->getPropertyValue("FrameInteropGrabBag");
                    auto aGrabBag = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aAny.get< uno::Sequence<beans::PropertyValue> >());

                    aGrabBag.push_back(comphelper::makePropertyValue("VML-Z-ORDER", maTypeModel.maZIndex.toInt32()));

                    if( !s_mso_next_textbox.isEmpty() )
                        aGrabBag.push_back(comphelper::makePropertyValue("mso-next-textbox", s_mso_next_textbox));

                    if( !sLinkChainName.isEmpty() )
                    {
                        aGrabBag.push_back(comphelper::makePropertyValue("TxbxHasLink", true));
                        aGrabBag.push_back(comphelper::makePropertyValue("Txbx-Id", id));
                        aGrabBag.push_back(comphelper::makePropertyValue("Txbx-Seq", seq));
                        aGrabBag.push_back(comphelper::makePropertyValue("LinkChainName", sLinkChainName));
                    }

                    if(!maTypeModel.maRotation.isEmpty())
                        aGrabBag.push_back(comphelper::makePropertyValue("mso-rotation-angle", ConversionHelper::decodeRotation(maTypeModel.maRotation)));
                    propertySet->setPropertyValue("FrameInteropGrabBag", uno::makeAny(comphelper::containerToSequence(aGrabBag)));
                    sal_Int32 backColorTransparency = 0;
                    propertySet->getPropertyValue("BackColorTransparency")
                        >>= backColorTransparency;
                    if (propertySet->getPropertyValue("FillStyle") == FillStyle_NONE &&
                        backColorTransparency == 100)
                    {
                        // If there is no fill, the Word default is 100% transparency.
                        propertySet->setPropertyValue("FillTransparence", makeAny(sal_Int16(100)));
                    }
                }
                else
                {
                    if( maTypeModel.maZIndex.toInt32() )
                    {
                        uno::Sequence<beans::PropertyValue> aGrabBag;
                        uno::Reference<beans::XPropertySet> propertySet (xShape, uno::UNO_QUERY);
                        propertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                        sal_Int32 length;

                        length = aGrabBag.getLength();
                        aGrabBag.realloc( length+1 );
                        aGrabBag[length].Name = "VML-Z-ORDER";
                        aGrabBag[length].Value <<= maTypeModel.maZIndex.toInt32();

                        if( !s_mso_next_textbox.isEmpty() )
                        {
                            length = aGrabBag.getLength();
                            aGrabBag.realloc( length+1 );
                            aGrabBag[length].Name = "mso-next-textbox";
                            aGrabBag[length].Value <<= s_mso_next_textbox;
                        }

                        if( !sLinkChainName.isEmpty() )
                        {
                            length = aGrabBag.getLength();
                            aGrabBag.realloc( length+4 );
                            aGrabBag[length].Name   = "TxbxHasLink";
                            aGrabBag[length].Value   <<= true;
                            aGrabBag[length+1].Name = "Txbx-Id";
                            aGrabBag[length+1].Value <<= id;
                            aGrabBag[length+2].Name = "Txbx-Seq";
                            aGrabBag[length+2].Value <<= seq;
                            aGrabBag[length+3].Name = "LinkChainName";
                            aGrabBag[length+3].Value <<= sLinkChainName;
                        }
                        propertySet->setPropertyValue( "InteropGrabBag", uno::makeAny(aGrabBag) );
                    }
                }
                Reference< XControlShape > xControlShape( xShape, uno::UNO_QUERY );
                if ( xControlShape.is() && !getTypeModel().mbVisible )
                {
                    PropertySet aControlShapeProp( xControlShape->getControl() );
                    aControlShapeProp.setProperty( PROP_EnableVisible, uno::makeAny( false ) );
                }
                /*  Notify the drawing that a new shape has been inserted. For
                    convenience, pass the rectangle that contains position and
                    size of the shape. */
                bool bGroupChild = pParentAnchor != nullptr;
                mrDrawing.notifyXShapeInserted( xShape, aShapeRect, *this, bGroupChild );
            }
        }
        else
            SAL_WARN("oox", "not converting shape, as calculated rectangle is empty");
    }
    return xShape;
}

void ShapeBase::convertFormatting( const Reference< XShape >& rxShape ) const
{
    if( !rxShape.is() )
        return;

    /*  Calculate shape rectangle. Applications may do something special
        according to some imported shape client data (e.g. Excel cell anchor). */
    awt::Rectangle aShapeRect = calcShapeRectangle( nullptr );

    // convert the shape, if the calculated rectangle is not empty
    if( (aShapeRect.Width > 0) || (aShapeRect.Height > 0) )
    {
        rxShape->setPosition( awt::Point( aShapeRect.X, aShapeRect.Y ) );
        rxShape->setSize( awt::Size( aShapeRect.Width, aShapeRect.Height ) );
        convertShapeProperties( rxShape );
    }
}

void ShapeBase::setContainer(ShapeContainer* pContainer) { mpContainer = pContainer; }

ShapeContainer* ShapeBase::getContainer() const { return mpContainer; }

// protected ------------------------------------------------------------------

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
        // Any other service supporting the ShadowFormat property?
        maTypeModel.maShadowModel.pushToPropMap(aPropMap, rGraphicHelper);
        // TextFrames have BackColor, not FillColor
        if (aPropMap.hasProperty(PROP_FillColor))
        {
            aPropMap.setAnyProperty(PROP_BackColor, aPropMap.getProperty(PROP_FillColor));
            aPropMap.erase(PROP_FillColor);
        }
        // TextFrames have BackColorTransparency, not FillTransparence
        if (aPropMap.hasProperty(PROP_FillTransparence))
        {
            aPropMap.setAnyProperty(PROP_BackColorTransparency, aPropMap.getProperty(PROP_FillTransparence));
            aPropMap.erase(PROP_FillTransparence);
        }
        // And no LineColor property; individual borders can have colors and widths
        std::optional<sal_Int32> oLineWidth;
        if (maTypeModel.maStrokeModel.moWeight.has())
            oLineWidth = ConversionHelper::decodeMeasureToHmm(
                rGraphicHelper, maTypeModel.maStrokeModel.moWeight.get(), 0, false, false);
        if (aPropMap.hasProperty(PROP_LineColor))
        {
            uno::Reference<beans::XPropertySet> xPropertySet(rxShape, uno::UNO_QUERY);
            static const sal_Int32 aBorders[] = {
                PROP_TopBorder, PROP_LeftBorder, PROP_BottomBorder, PROP_RightBorder
            };
            for (sal_Int32 nBorder : aBorders)
            {
                table::BorderLine2 aBorderLine = xPropertySet->getPropertyValue(PropertyMap::getPropertyName(nBorder)).get<table::BorderLine2>();
                aBorderLine.Color = aPropMap.getProperty(PROP_LineColor).get<sal_Int32>();
                if (oLineWidth)
                    aBorderLine.LineWidth = *oLineWidth;
                aPropMap.setProperty(nBorder, aBorderLine);
            }
            aPropMap.erase(PROP_LineColor);
        }
    }
    else if (xSInfo->supportsService("com.sun.star.drawing.CustomShape"))
        maTypeModel.maTextpathModel.pushToPropMap(aPropMap, rxShape, rGraphicHelper);

    PropertySet( rxShape ).setProperties( aPropMap );
}

SimpleShape::SimpleShape( Drawing& rDrawing, const OUString& rService ) :
    ShapeBase( rDrawing ),
    maService( rService )
{
}

static void lcl_setSurround(PropertySet& rPropSet, const ShapeTypeModel& rTypeModel, const GraphicHelper& rGraphicHelper)
{
    OUString aWrapType = rTypeModel.moWrapType.get();

    // Extreme negative top margin? Then the shape will end up at the top of the page, it's pointless to perform any kind of wrapping.
    sal_Int32 nMarginTop = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, rTypeModel.maMarginTop, 0, false, true);
    if (nMarginTop < -35277) // Less than 1000 points.
        aWrapType.clear();

    css::text::WrapTextMode nSurround = css::text::WrapTextMode_THROUGH;
    if ( aWrapType == "square" || aWrapType == "tight" ||
         aWrapType == "through" )
    {
        nSurround = css::text::WrapTextMode_PARALLEL;
        if ( rTypeModel.moWrapSide.get() == "left" )
            nSurround = css::text::WrapTextMode_LEFT;
        else if ( rTypeModel.moWrapSide.get() == "right" )
            nSurround = css::text::WrapTextMode_RIGHT;
    }
    else if ( aWrapType == "topAndBottom" )
        nSurround = css::text::WrapTextMode_NONE;

    rPropSet.setProperty(PROP_Surround, static_cast<sal_Int32>(nSurround));
    rPropSet.setProperty(PROP_SurroundContour, aWrapType == "tight");
}

static void lcl_SetAnchorType(PropertySet& rPropSet, const ShapeTypeModel& rTypeModel, const GraphicHelper& rGraphicHelper)
{
    if ( rTypeModel.maPosition == "absolute" )
    {
        // Word supports as-character (inline) and at-character only, absolute can't be inline.
        rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AT_CHARACTER);
        // anchor is set after insertion, so reset to NONE
        rPropSet.setAnyProperty(PROP_VertOrient, makeAny(text::VertOrientation::NONE));

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
    {   // I'm not very sure this is correct either.
        rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AT_PARAGRAPH);
        // anchor is set after insertion, so reset to NONE
        rPropSet.setAnyProperty(PROP_VertOrient, makeAny(text::VertOrientation::NONE));
    }
    else // static (is the default) means anchored inline
    {
        rPropSet.setProperty(PROP_AnchorType, text::TextContentAnchorType_AS_CHARACTER);
        // Use top orientation, this one seems similar to what MSO uses as inline
        rPropSet.setAnyProperty(PROP_VertOrient, makeAny(text::VertOrientation::TOP));
    }

    if ( rTypeModel.maPositionHorizontal == "center" )
        rPropSet.setAnyProperty(PROP_HoriOrient, makeAny(text::HoriOrientation::CENTER));
    else if ( rTypeModel.maPositionHorizontal == "left" )
        rPropSet.setAnyProperty(PROP_HoriOrient, makeAny(text::HoriOrientation::LEFT));
    else if ( rTypeModel.maPositionHorizontal == "right" )
        rPropSet.setAnyProperty(PROP_HoriOrient, makeAny(text::HoriOrientation::RIGHT));
    else if ( rTypeModel.maPositionHorizontal == "inside" )
    {
        rPropSet.setAnyProperty(PROP_HoriOrient, makeAny(text::HoriOrientation::LEFT));
        rPropSet.setAnyProperty(PROP_PageToggle, makeAny(true));
    }
    else if ( rTypeModel.maPositionHorizontal == "outside" )
    {
        rPropSet.setAnyProperty(PROP_HoriOrient, makeAny(text::HoriOrientation::RIGHT));
        rPropSet.setAnyProperty(PROP_PageToggle, makeAny(true));
    }

    if ( rTypeModel.maPositionHorizontalRelative == "page" )
        rPropSet.setAnyProperty(PROP_HoriOrientRelation, makeAny(text::RelOrientation::PAGE_FRAME));
    else if ( rTypeModel.maPositionHorizontalRelative == "margin" )
        rPropSet.setProperty(PROP_HoriOrientRelation, text::RelOrientation::PAGE_PRINT_AREA);
    else if (rTypeModel.maPositionHorizontalRelative == "right-margin-area" ||
             rTypeModel.maPositionHorizontalRelative == "inner-margin-area")
        rPropSet.setProperty(PROP_HoriOrientRelation, text::RelOrientation::PAGE_RIGHT);
    else if (rTypeModel.maPositionHorizontalRelative == "left-margin-area" ||
             rTypeModel.maPositionHorizontalRelative == "outer-margin-area")
        rPropSet.setProperty(PROP_HoriOrientRelation, text::RelOrientation::PAGE_LEFT);
    else if ( rTypeModel.maPositionHorizontalRelative == "text" )
        rPropSet.setProperty(PROP_HoriOrientRelation, text::RelOrientation::FRAME);

    if ( rTypeModel.maPositionVertical == "center" )
        rPropSet.setAnyProperty(PROP_VertOrient, makeAny(text::VertOrientation::CENTER));
    else if ( rTypeModel.maPositionVertical == "top" )
        rPropSet.setAnyProperty(PROP_VertOrient, makeAny(text::VertOrientation::TOP));
    else if ( rTypeModel.maPositionVertical == "bottom" )
        rPropSet.setAnyProperty(PROP_VertOrient, makeAny(text::VertOrientation::BOTTOM));
    else if ( rTypeModel.maPositionVertical == "inside" )
        rPropSet.setAnyProperty(PROP_VertOrient, makeAny(text::VertOrientation::LINE_TOP));
    else if ( rTypeModel.maPositionVertical == "outside" )
        rPropSet.setAnyProperty(PROP_VertOrient, makeAny(text::VertOrientation::LINE_BOTTOM));

    lcl_setSurround( rPropSet, rTypeModel, rGraphicHelper );
}

Reference< XShape > SimpleShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    awt::Rectangle aShapeRect(rShapeRect);
    std::optional<sal_Int32> oRotation;
    bool bFlipX = false, bFlipY = false;
    if (!maTypeModel.maRotation.isEmpty())
        oRotation = ConversionHelper::decodeRotation(maTypeModel.maRotation);
    if (!maTypeModel.maFlip.isEmpty())
    {
        if (maTypeModel.maFlip.startsWith("x"))
        {
            bFlipX = true;
        }
        if (maTypeModel.maFlip.endsWith("y"))
        {
            bFlipY = true;
        }
    }

    Reference< XShape > xShape = mrDrawing.createAndInsertXShape( maService, rxShapes, aShapeRect );
    SdrObject* pShape = GetSdrObjectFromXShape( xShape );
    if( pShape && getShapeType() >= 0 )
    {
        OUString aShapeType = EnhancedCustomShapeTypeNames::Get( static_cast< MSO_SPT >(getShapeType()) );
        //The resize autoshape to fit text attr of FontWork/Word-Art should always be false
        //for the fallback geometry.
        if(aShapeType.startsWith("fontwork"))
        {
            pShape->SetMergedItem(makeSdrTextAutoGrowHeightItem(false));
            pShape->SetMergedItem(makeSdrTextAutoGrowWidthItem(false));
        }
    }
    convertShapeProperties( xShape );

    // Handle left/right/top/bottom wrap distance.
    // Default value of mso-wrap-distance-left/right is supposed to be 0 (see
    // 19.1.2.19 of the VML spec), but Word implements a non-zero value.
    // [MS-ODRAW] says the below default value in 2.3.4.9.
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

    if ( maService == "com.sun.star.text.TextFrame" )
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

        sal_Int16 nWritingMode = text::WritingMode2::LR_TB;
        if (getTextBox()->maLayoutFlow == "vertical" && maTypeModel.maLayoutFlowAlt.isEmpty())
        {
            nWritingMode = text::WritingMode2::TB_RL;
        }
        else if (maTypeModel.maLayoutFlowAlt == "bottom-to-top")
        {
            nWritingMode = text::WritingMode2::BT_LR;
        }
        if (nWritingMode != text::WritingMode2::LR_TB)
        {
            PropertySet(xShape).setAnyProperty(PROP_WritingMode, uno::makeAny(nWritingMode));
        }
    }
    else
    {
        // FIXME Setting the relative width/height only for everything but text frames as
        // TextFrames already have relative width/height feature... but currently not working
        // in the way we need.

        // Set the relative width / height if any
        if ( !maTypeModel.maWidthPercent.isEmpty( ) )
        {
            // Only page-relative width is supported ATM
            if ( maTypeModel.maWidthRelative.isEmpty() || maTypeModel.maWidthRelative == "page" )
            {
                sal_Int16 nWidth = maTypeModel.maWidthPercent.toInt32() / 10;
                // Only apply if nWidth != 0
                if ( nWidth )
                    PropertySet( xShape ).setAnyProperty(PROP_RelativeWidth, makeAny( nWidth ) );
            }
        }
        if ( !maTypeModel.maHeightPercent.isEmpty( ) )
        {
            // Only page-relative height is supported ATM
            if ( maTypeModel.maHeightRelative.isEmpty() || maTypeModel.maHeightRelative == "page" )
            {
                sal_Int16 nHeight = maTypeModel.maHeightPercent.toInt32() / 10;
                // Only apply if nHeight != 0
                if ( nHeight )
                    PropertySet( xShape ).setAnyProperty(PROP_RelativeHeight, makeAny( nHeight ) );
            }
        }

        // drawinglayer default is center, MSO default is top.
        drawing::TextVerticalAdjust eTextVerticalAdjust = drawing::TextVerticalAdjust_TOP;
        if (maTypeModel.maVTextAnchor == "middle")
            eTextVerticalAdjust = drawing::TextVerticalAdjust_CENTER;
        else if (maTypeModel.maVTextAnchor == "bottom")
            eTextVerticalAdjust = drawing::TextVerticalAdjust_BOTTOM;
        PropertySet(xShape).setAnyProperty(PROP_TextVerticalAdjust, makeAny(eTextVerticalAdjust));

        // tdf#97618
        if(!maTypeModel.maWrapStyle.isEmpty())
            PropertySet(xShape).setAnyProperty(PROP_TextWordWrap, makeAny(maTypeModel.maWrapStyle == "square"));

        PropertySet(xShape).setAnyProperty(PROP_TextAutoGrowHeight,
                                           makeAny(maTypeModel.mbAutoHeight));

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

    // Import Legacy Fragments (if any)
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
            aPropertySet.setAnyProperty(PROP_RotateAngle, makeAny(*oRotation));
            uno::Reference<lang::XServiceInfo> xServiceInfo(rxShapes, uno::UNO_QUERY);
            if (!xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
            {
                // If rotation is used, simple setPosition() is not enough.
                aPropertySet.setAnyProperty(PROP_HoriOrientPosition, makeAny(aShapeRect.X));
                aPropertySet.setAnyProperty(PROP_VertOrientPosition, makeAny(aShapeRect.Y));
            }
        }

        // custom shape geometry attributes
        std::vector<css::beans::PropertyValue> aPropVec;

        // When flip has 'x' or 'y', the associated ShapeRect will be changed but direction change doesn't occur.
        // It might occur internally in SdrObject of "sw" module, not here.
        // The associated properties "PROP_MirroredX" and "PROP_MirroredY" have to be set here so that direction change will occur internally.
        if (bFlipX)
            aPropVec.push_back(comphelper::makePropertyValue("MirroredX", true));
        if (bFlipY)
            aPropVec.push_back(comphelper::makePropertyValue("MirroredY", true));

        if (!maTypeModel.maAdjustments.isEmpty())
        {
            std::vector<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues;
            sal_Int32 nIndex = 0;
            do
            {
                OUString aToken = maTypeModel.maAdjustments.getToken(0, ',', nIndex);
                drawing::EnhancedCustomShapeAdjustmentValue aAdjustmentValue;
                if (aToken.isEmpty())
                    aAdjustmentValue.State = css::beans::PropertyState::PropertyState_DEFAULT_VALUE;
                else
                    aAdjustmentValue.Value <<= aToken.toInt32();
                aAdjustmentValues.push_back(aAdjustmentValue);
            } while (nIndex >= 0);

            css::beans::PropertyValue aProp;
            aProp.Name = "AdjustmentValues";
            aProp.Value <<= comphelper::containerToSequence(aAdjustmentValues);
            aPropVec.push_back(aProp);
        }

        if (!aPropVec.empty())
            aPropertySet.setAnyProperty(PROP_CustomShapeGeometry, makeAny(comphelper::containerToSequence(aPropVec)));
    }

    lcl_SetAnchorType(aPropertySet, maTypeModel, rGraphicHelper );

    return xShape;
}

Reference< XShape > SimpleShape::createEmbeddedPictureObject( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect, OUString const & rGraphicPath ) const
{
    Reference<XGraphic> xGraphic = mrDrawing.getFilter().getGraphicHelper().importEmbeddedGraphic(rGraphicPath);
    return SimpleShape::createPictureObject(rxShapes, rShapeRect, xGraphic);
}

Reference< XShape > SimpleShape::createPictureObject(const Reference< XShapes >& rxShapes,
                                                     const awt::Rectangle& rShapeRect,
                                                     uno::Reference<graphic::XGraphic> const & rxGraphic) const
{
    Reference< XShape > xShape = mrDrawing.createAndInsertXShape( "com.sun.star.drawing.GraphicObjectShape", rxShapes, rShapeRect );
    if( xShape.is() )
    {
        PropertySet aPropSet(xShape);
        if (rxGraphic.is())
        {
            aPropSet.setProperty(PROP_Graphic, rxGraphic);
        }
        uno::Reference< lang::XServiceInfo > xServiceInfo(rxShapes, uno::UNO_QUERY);
        // If the shape has an absolute position, set the properties accordingly, unless we're inside a group shape.
        if ( maTypeModel.maPosition == "absolute" && !xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
        {
            aPropSet.setProperty(PROP_HoriOrientPosition, rShapeRect.X);
            aPropSet.setProperty(PROP_VertOrientPosition, rShapeRect.Y);
            aPropSet.setProperty(PROP_Opaque, false);
        }
        // fdo#70457: preserve rotation information
        if ( !maTypeModel.maRotation.isEmpty() )
            aPropSet.setAnyProperty(PROP_RotateAngle, makeAny(ConversionHelper::decodeRotation(maTypeModel.maRotation)));

        const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();
        lcl_SetAnchorType(aPropSet, maTypeModel, rGraphicHelper);

        const sal_Int32 nWrapDistanceLeft = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maTypeModel.maWrapDistanceLeft, 0, true, true);
        const sal_Int32 nWrapDistanceRight = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maTypeModel.maWrapDistanceRight, 0, true, true);
        const sal_Int32 nWrapDistanceTop = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maTypeModel.maWrapDistanceTop, 0, false, true);
        const sal_Int32 nWrapDistanceBottom = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, maTypeModel.maWrapDistanceBottom, 0, false, true);
        aPropSet.setProperty(PROP_LeftMargin, uno::makeAny(nWrapDistanceLeft));
        aPropSet.setProperty(PROP_RightMargin, uno::makeAny(nWrapDistanceRight));
        aPropSet.setProperty(PROP_TopMargin, uno::makeAny(nWrapDistanceTop));
        aPropSet.setProperty(PROP_BottomMargin, uno::makeAny(nWrapDistanceBottom));

        if (maTypeModel.moCropBottom.has() || maTypeModel.moCropLeft.has() || maTypeModel.moCropRight.has() || maTypeModel.moCropTop.has())
        {
            text::GraphicCrop aGraphicCrop;
            awt::Size aOriginalSize = rGraphicHelper.getOriginalSize(rxGraphic);

            if (maTypeModel.moCropBottom.has())
                aGraphicCrop.Bottom = lclConvertCrop(maTypeModel.moCropBottom.get(), aOriginalSize.Height);
            if (maTypeModel.moCropLeft.has())
                aGraphicCrop.Left = lclConvertCrop(maTypeModel.moCropLeft.get(), aOriginalSize.Width);
            if (maTypeModel.moCropRight.has())
                aGraphicCrop.Right = lclConvertCrop(maTypeModel.moCropRight.get(), aOriginalSize.Width);
            if (maTypeModel.moCropTop.has())
                aGraphicCrop.Top = lclConvertCrop(maTypeModel.moCropTop.get(), aOriginalSize.Height);

            aPropSet.setProperty(PROP_GraphicCrop, aGraphicCrop);
        }
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

    // try to create a picture object
    if(!aGraphicPath.isEmpty())
        return SimpleShape::createEmbeddedPictureObject(rxShapes, rShapeRect, aGraphicPath);

    // default: try to create a rectangle shape
    Reference<XShape> xShape = SimpleShape::implConvertAndInsert(rxShapes, rShapeRect);
    OUString sArcsize = maTypeModel.maArcsize;
    if ( !sArcsize.isEmpty( ) )
    {
        sal_Unicode cLastChar = sArcsize[sArcsize.getLength() - 1];
        sal_Int32 nValue = sArcsize.copy( 0, sArcsize.getLength() - 1 ).toInt32( );
        // Get the smallest half-side
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
    // polygon path
    awt::Rectangle aCoordSys = getCoordSystem();
    if( !maShapeModel.maPoints.empty() && (aCoordSys.Width > 0) && (aCoordSys.Height > 0) )
    {
        ::std::vector< awt::Point > aAbsPoints;
        for (auto const& point : maShapeModel.maPoints)
            aAbsPoints.push_back( lclGetAbsPoint( point, rShapeRect, aCoordSys ) );
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
    // If we have an 'x' in the last part of the path it means it is closed...
    sal_Int32 nPos = maShapeModel.maVmlPath.lastIndexOf(',');
    if ( nPos != -1 && maShapeModel.maVmlPath.indexOf('x', nPos) != -1 )
    {
        const_cast<BezierShape*>( this )->setService( "com.sun.star.drawing.ClosedBezierShape" );
    }

    awt::Rectangle aCoordSys = getCoordSystem();
    PolyPolygonBezierCoords aBezierCoords;

    if( (aCoordSys.Width > 0) && (aCoordSys.Height > 0) )
    {
        const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();

        // Bezier paths may consist of one or more sub-paths
        typedef ::std::vector< ::std::vector< PolygonFlags > > FlagsList;
        std::vector< ::std::vector< awt::Point > > aCoordLists;
        FlagsList aFlagLists;
        sal_Int32 nIndex = 0;

        // Curve defined by to, from, control1 and control2 attributes
        if ( maShapeModel.maVmlPath.isEmpty() )
        {
            aCoordLists.emplace_back( );
            aFlagLists.emplace_back( );

            // Start point
            aCoordLists[ 0 ].emplace_back(
                  ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maFrom.getToken( 0, ',', nIndex ), 0, true, true ),
                  ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maFrom.getToken( 0, ',', nIndex ), 0, false, true ) );
            // Control point 1
            aCoordLists[ 0 ].emplace_back(
                      ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maControl1.getToken( 0, ',', nIndex ), 0, true, true ),
                      ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maControl1.getToken( 0, ',', nIndex ), 0, false, true ) );
            // Control point 2
            aCoordLists[ 0 ].emplace_back(
                      ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maControl2.getToken( 0, ',', nIndex ), 0, true, true ),
                      ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maControl2.getToken( 0, ',', nIndex ), 0, false, true ) );
            // End point
            aCoordLists[ 0 ].emplace_back(
                      ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maTo.getToken( 0, ',', nIndex ), 0, true, true ),
                      ConversionHelper::decodeMeasureToHmm( rGraphicHelper, maShapeModel.maTo.getToken( 0, ',', nIndex ), 0, false, true ) );

            // First and last points are normals, points 2 and 4 are controls
            aFlagLists[ 0 ].resize( aCoordLists[ 0 ].size(), PolygonFlags_CONTROL );
            aFlagLists[ 0 ][ 0 ] = PolygonFlags_NORMAL;
            aFlagLists[ 0 ].back() = PolygonFlags_NORMAL;
        }
        // Curve defined by path attribute
        else
        {
            // Parse VML path string and convert to absolute coordinates
            ConversionHelper::decodeVmlPath( aCoordLists, aFlagLists, maShapeModel.maVmlPath );

            for (auto & coordList : aCoordLists)
                for (auto & point : coordList)
                {
                    point = lclGetAbsPoint( point, rShapeRect, aCoordSys );
                }
        }

        aBezierCoords.Coordinates.realloc( aCoordLists.size() );
        for ( size_t i = 0; i < aCoordLists.size(); i++ )
            aBezierCoords.Coordinates[i] = ContainerHelper::vectorToSequence( aCoordLists[i] );

        aBezierCoords.Flags.realloc( aFlagLists.size() );
        for ( size_t i = 0; i < aFlagLists.size(); i++ )
            aBezierCoords.Flags[i] = ContainerHelper::vectorToSequence( aFlagLists[i] );

        if( !aCoordLists.front().empty() && !aCoordLists.back().empty()
            && aCoordLists.front().front().X == aCoordLists.back().back().X
            && aCoordLists.front().front().Y == aCoordLists.back().back().Y )
        { // HACK: If the shape is in fact closed, which can be found out only when the path is known,
          // force to closed bezier shape (otherwise e.g. fill won't work).
            const_cast< BezierShape* >( this )->setService( "com.sun.star.drawing.ClosedBezierShape" );
        }
    }

    Reference< XShape > xShape = SimpleShape::implConvertAndInsert( rxShapes, rShapeRect );

    if( aBezierCoords.Coordinates.hasElements())
    {
        PropertySet aPropSet( xShape );
        aPropSet.setProperty( PROP_PolyPolygonBezier, aBezierCoords );
    }

    // Handle horizontal and vertical flip.
    if (!maTypeModel.maFlip.isEmpty())
    {
        if (SdrObject* pShape = GetSdrObjectFromXShape(xShape))
        {
            if (maTypeModel.maFlip.startsWith("x"))
            {
                Point aCenter(pShape->GetSnapRect().Center());
                Point aPoint2(aCenter);
                aPoint2.setY(aPoint2.getY() + 1);
                pShape->NbcMirror(aCenter, aPoint2);
            }
            if (maTypeModel.maFlip.endsWith("y"))
            {
                Point aCenter(pShape->GetSnapRect().Center());
                Point aPoint2(aCenter);
                aPoint2.setX(aPoint2.getX() + 1);
                pShape->NbcMirror(aCenter, aPoint2);
            }
        }
    }

    // Hacky way of ensuring the shape is correctly sized/positioned
    try
    {
        // E.g. SwXFrame::setPosition() unconditionally throws
        xShape->setSize( awt::Size( rShapeRect.Width, rShapeRect.Height ) );
        xShape->setPosition( awt::Point( rShapeRect.X, rShapeRect.Y ) );
    }
    catch (const ::css::uno::Exception&)
    {
        // TODO: try some other way to ensure size/position
    }
    return xShape;
}

CustomShape::CustomShape( Drawing& rDrawing ) :
    SimpleShape( rDrawing, "com.sun.star.drawing.CustomShape" )
{
}

Reference< XShape > CustomShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    // try to create a custom shape
    Reference< XShape > xShape = SimpleShape::implConvertAndInsert( rxShapes, rShapeRect );
    if( xShape.is() ) try
    {
        // create the custom shape geometry
        Reference< XEnhancedCustomShapeDefaulter > xDefaulter( xShape, UNO_QUERY_THROW );
        xDefaulter->createCustomShapeDefaults( OUString::number( getShapeType() ) );
        // convert common properties
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

    // try to find registered OLE object info
    if( const OleObjectInfo* pOleObjectInfo = mrDrawing.getOleObjectInfo( maTypeModel.maShapeId ) )
    {
        SAL_WARN_IF(
            nShapeType != VML_SHAPETYPE_PICTUREFRAME, "oox",
            "ComplexShape::implConvertAndInsert - unexpected shape type");

        // if OLE object is embedded into a DrawingML shape (PPTX), do not create it here
        if( pOleObjectInfo->mbDmlShape )
            return Reference< XShape >();

        PropertyMap aOleProps;
        awt::Size aOleSize( rShapeRect.Width, rShapeRect.Height );
        if( rFilter.getOleObjectHelper().importOleObject( aOleProps, *pOleObjectInfo, aOleSize ) )
        {
            Reference< XShape > xShape = mrDrawing.createAndInsertXShape( "com.sun.star.drawing.OLE2Shape", rxShapes, rShapeRect );
            if( xShape.is() )
            {
                // set the replacement graphic
                if( !aGraphicPath.isEmpty() )
                {
                    WmfExternal aExtHeader;
                    aExtHeader.mapMode = 8;
                    aExtHeader.xExt = rShapeRect.Width;
                    aExtHeader.yExt = rShapeRect.Height;

                    Reference< XGraphic > xGraphic = rFilter.getGraphicHelper().importEmbeddedGraphic(aGraphicPath, &aExtHeader);
                    if (xGraphic.is())
                        aOleProps.setProperty( PROP_Graphic, xGraphic);
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
        if( !pControlInfo->maName.isEmpty() )
        {
            // load the control properties from fragment
            ::oox::ole::EmbeddedControl aControl(pControlInfo->maName);
            if( rFilter.importFragment( new ::oox::ole::AxControlFragment( rFilter, pControlInfo->maFragmentPath, aControl ) ) )
            {
                // create and return the control shape (including control model)
                sal_Int32 nCtrlIndex = -1;
                Reference< XShape > xShape = mrDrawing.createAndInsertXControlShape( aControl, rxShapes, rShapeRect, nCtrlIndex );

                if (pControlInfo->mbTextContentShape)
                {
                    PropertySet aPropertySet(xShape);
                    lcl_SetAnchorType(aPropertySet, maTypeModel, mrDrawing.getFilter().getGraphicHelper());
                }
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


    if( getShapeModel().mbIsSignatureLine )
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        bool bIsSigned(false);
        try
        {
            // Get the document signatures
            Reference<security::XDocumentDigitalSignatures> xSignatures(
                security::DocumentDigitalSignatures::createDefault(
                    comphelper::getProcessComponentContext()));

            uno::Reference<embed::XStorage> xStorage
                = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
                    ZIP_STORAGE_FORMAT_STRING, mrDrawing.getFilter().getFileUrl(),
                    embed::ElementModes::READ);
            SAL_WARN_IF(!xStorage.is(), "oox.vml", "No xStorage!");

            const uno::Sequence<security::DocumentSignatureInformation> xSignatureInfo
                = xSignatures->verifyScriptingContentSignatures(xStorage,
                                                                uno::Reference<io::XInputStream>());

            // Try to find matching signature line image - if none exists that is fine,
            // then the signature line is not digitally signed.
            auto pSignInfo = std::find_if(xSignatureInfo.begin(), xSignatureInfo.end(),
                [this](const security::DocumentSignatureInformation& rSigInfo) {
                    return rSigInfo.SignatureLineId == getShapeModel().maSignatureId; });
            if (pSignInfo != xSignatureInfo.end())
            {
                bIsSigned = true;
                if (pSignInfo->SignatureIsValid)
                {
                    // Signature is valid, use the 'valid' image
                    SAL_WARN_IF(!pSignInfo->ValidSignatureLineImage.is(), "oox.vml",
                                "No ValidSignatureLineImage!");
                    xGraphic = pSignInfo->ValidSignatureLineImage;
                }
                else
                {
                    // Signature is invalid, use the 'invalid' image
                    SAL_WARN_IF(!pSignInfo->InvalidSignatureLineImage.is(), "oox.vml",
                                "No InvalidSignatureLineImage!");
                    xGraphic = pSignInfo->InvalidSignatureLineImage;
                }
            }
        }
        catch (css::uno::Exception&)
        {
            // DocumentDigitalSignatures service not available.
            // We continue by rendering the "unsigned" shape instead.
        }

        Reference< XShape > xShape;
        if (xGraphic.is())
        {
            // If available, use the signed image from the signature
            xShape = SimpleShape::createPictureObject(rxShapes, rShapeRect, xGraphic);
        }
        else
        {
            // Create shape with the fallback "unsigned" image
            xShape = SimpleShape::createEmbeddedPictureObject(rxShapes, rShapeRect, aGraphicPath);
        }

        // Store signature line properties
        uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
        xPropertySet->setPropertyValue("IsSignatureLine", uno::makeAny(true));
        xPropertySet->setPropertyValue("SignatureLineId",
                                        uno::makeAny(getShapeModel().maSignatureId));
        xPropertySet->setPropertyValue(
            "SignatureLineSuggestedSignerName",
            uno::makeAny(getShapeModel().maSignatureLineSuggestedSignerName));
        xPropertySet->setPropertyValue(
            "SignatureLineSuggestedSignerTitle",
            uno::makeAny(getShapeModel().maSignatureLineSuggestedSignerTitle));
        xPropertySet->setPropertyValue(
            "SignatureLineSuggestedSignerEmail",
            uno::makeAny(getShapeModel().maSignatureLineSuggestedSignerEmail));
        xPropertySet->setPropertyValue(
            "SignatureLineSigningInstructions",
            uno::makeAny(getShapeModel().maSignatureLineSigningInstructions));
        xPropertySet->setPropertyValue(
            "SignatureLineShowSignDate",
            uno::makeAny(getShapeModel().mbSignatureLineShowSignDate));
        xPropertySet->setPropertyValue(
            "SignatureLineCanAddComment",
            uno::makeAny(getShapeModel().mbSignatureLineCanAddComment));
        xPropertySet->setPropertyValue("SignatureLineIsSigned", uno::makeAny(bIsSigned));

        if (!aGraphicPath.isEmpty())
        {
            xGraphic = rFilter.getGraphicHelper().importEmbeddedGraphic(aGraphicPath);
            xPropertySet->setPropertyValue("SignatureLineUnsignedImage", uno::makeAny(xGraphic));
        }
        return xShape;
    }

    // try to create a picture object
    if( !aGraphicPath.isEmpty() )
    {
        Reference<XShape> xShape = SimpleShape::createEmbeddedPictureObject(rxShapes, rShapeRect, aGraphicPath);
        // AS_CHARACTER shape: vertical orientation default is bottom, MSO default is top.
        if ( maTypeModel.maPosition != "absolute" && maTypeModel.maPosition != "relative" )
            PropertySet( xShape ).setAnyProperty( PROP_VertOrient, makeAny(text::VertOrientation::TOP));

        // Apply stroke props from the type model.
        oox::drawingml::ShapePropertyMap aPropMap(mrDrawing.getFilter().getModelObjectHelper());
        const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();
        maTypeModel.maStrokeModel.pushToPropMap(aPropMap, rGraphicHelper);
        //And, fill-color properties as well...
        maTypeModel.maFillModel.pushToPropMap(aPropMap, rGraphicHelper);
        PropertySet(xShape).setProperties(aPropMap);

        return xShape;
    }

    // default: try to create a custom shape
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
    // basic shape processing
    ShapeBase::finalizeFragmentImport();
    // finalize all child shapes
    mxChildren->finalizeFragmentImport();
}

const ShapeType* GroupShape::getChildTypeById( const OUString& rShapeId ) const
{
    return mxChildren->getShapeTypeById( rShapeId );
}

const ShapeBase* GroupShape::getChildById( const OUString& rShapeId ) const
{
    return mxChildren->getShapeById( rShapeId );
}

Reference< XShape > GroupShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    Reference< XShape > xGroupShape;
    // check that this shape contains children and a valid coordinate system
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

    uno::Reference<beans::XPropertySet> xPropertySet;
    if (!maTypeModel.maEditAs.isEmpty())
        xPropertySet = uno::Reference<beans::XPropertySet>(xGroupShape, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        uno::Sequence<beans::PropertyValue> aGrabBag;
        xPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
        beans::PropertyValue aPair;
        aPair.Name = "mso-edit-as";
        aPair.Value <<= maTypeModel.maEditAs;
        sal_Int32 nLength = aGrabBag.getLength();
        aGrabBag.realloc(nLength + 1);
        aGrabBag[nLength] = aPair;
        xPropertySet->setPropertyValue("InteropGrabBag", uno::makeAny(aGrabBag));
    }
    // Make sure group shapes are inline as well, unless there is an explicit different style.
    PropertySet aPropertySet(xGroupShape);
    const GraphicHelper& rGraphicHelper = mrDrawing.getFilter().getGraphicHelper();
    lcl_SetAnchorType(aPropertySet, maTypeModel, rGraphicHelper);
    if (!maTypeModel.maRotation.isEmpty())
        aPropertySet.setAnyProperty(PROP_RotateAngle, makeAny(ConversionHelper::decodeRotation(maTypeModel.maRotation)));
    return xGroupShape;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
