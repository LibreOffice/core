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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <oox/ppt/timenode.hxx>
#include <oox/ppt/pptshape.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <drawingml/fillproperties.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <drawingml/textliststyle.hxx>
#include <drawingml/textparagraphproperties.hxx>
#include <drawingml/connectorhelper.hxx>

#include <osl/diagnose.h>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/container/XIdentifierContainer.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeGluePointType.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <utility>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>

using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::animations;


namespace oox::ppt {

SlidePersist::SlidePersist( XmlFilterBase& rFilter, bool bMaster, bool bNotes,
    const css::uno::Reference< css::drawing::XDrawPage >& rxPage,
        oox::drawingml::ShapePtr pShapesPtr, drawingml::TextListStylePtr pDefaultTextStyle )
: mpDrawingPtr( std::make_shared<oox::vml::Drawing>( rFilter, rxPage, oox::vml::VMLDRAWING_POWERPOINT ) )
, mxPage( rxPage )
, maShapesPtr(std::move( pShapesPtr ))
, mnLayoutValueToken( 0 )
, mbMaster( bMaster )
, mbNotes ( bNotes )
, maDefaultTextStylePtr(std::move( pDefaultTextStyle ))
, maTitleTextStylePtr( std::make_shared<oox::drawingml::TextListStyle>() )
, maBodyTextStylePtr( std::make_shared<oox::drawingml::TextListStyle>() )
, maNotesTextStylePtr( std::make_shared<oox::drawingml::TextListStyle>() )
, maOtherTextStylePtr( std::make_shared<oox::drawingml::TextListStyle>() )
{
#if OSL_DEBUG_LEVEL > 0
    mxDebugPage = mxPage;
#endif
}

#if OSL_DEBUG_LEVEL > 0
css::uno::WeakReference< css::drawing::XDrawPage > SlidePersist::mxDebugPage;
#endif

SlidePersist::~SlidePersist()
{
}

sal_Int16 SlidePersist::getLayoutFromValueToken() const
{
    sal_Int16 nLayout = 20;     // 20 == blank (so many magic numbers :-( the description at com.sun.star.presentation.DrawPage.Layout does not help)
    switch( mnLayoutValueToken )
    {
        case XML_blank:             nLayout = 20; break;
        case XML_chart:             nLayout =  2; break;
        case XML_chartAndTx:        nLayout =  7; break;
        case XML_clipArtAndTx:      nLayout =  9; break;
        case XML_clipArtAndVertTx:  nLayout = 24; break;
        case XML_fourObj:           nLayout = 18; break;
        case XML_obj:               nLayout = 11; break;
        case XML_objAndTx:          nLayout = 13; break;
        case XML_objOverTx:         nLayout = 14; break;
        case XML_tbl:               nLayout =  8; break;
        case XML_title:             nLayout =  0; break;
        case XML_titleOnly:         nLayout = 19; break;
        case XML_twoObj:
        case XML_twoColTx:          nLayout =  3; break;
        case XML_twoObjAndObj:
        case XML_twoObjAndTx:       nLayout = 15; break;
        case XML_twoObjOverTx:      nLayout = 16; break;
        case XML_tx:                nLayout =  1; break;
        case XML_txAndChart:        nLayout =  4; break;
        case XML_txAndClipArt:      nLayout =  6; break;
        case XML_txAndMedia:        nLayout =  6; break;
        case XML_txAndObj:          nLayout = 10; break;
        case XML_objAndTwoObj:
        case XML_txAndTwoObj:       nLayout = 12; break;
        case XML_txOverObj:         nLayout = 17; break;
        case XML_vertTitleAndTx:    nLayout = 22; break;
        case XML_vertTitleAndTxOverChart: nLayout = 21; break;
        case XML_vertTx:            nLayout = 23; break;
        case XML_objOnly:           nLayout = 32; break;

        case XML_twoTxTwoObj:
        case XML_objTx:
        case XML_picTx:
        case XML_secHead:
        case XML_mediaAndTx:
        case XML_dgm:
        case XML_cust:
        default:
            nLayout = 20;
    }
    return nLayout;
}

static void lcl_createShapeMap(oox::drawingml::ShapePtr rShapePtr,
                               oox::drawingml::ShapeIdMap& rShapeMap)
{
    std::vector<oox::drawingml::ShapePtr>& rChildren = rShapePtr->getChildren();
    if (!rChildren.empty())
    {
        for (const auto& pIt : rChildren)
        {
            if (pIt->isConnectorShape())
            {
                rShapeMap[pIt->getId()] = pIt; // add child itself
                lcl_createShapeMap(pIt, rShapeMap); // and all its descendants
            }
        }
    }
    else
    {
        if(rShapePtr->isConnectorShape())
            rShapeMap[rShapePtr->getId()] = rShapePtr;
    }
}

void SlidePersist::createXShapes( XmlFilterBase& rFilterBase )
{
    applyTextStyles( rFilterBase );

    Reference< XShapes > xShapes( getPage() );
    std::vector< oox::drawingml::ShapePtr >& rShapes( maShapesPtr->getChildren() );
    oox::drawingml::ShapeIdMap aConnectorShapeMap;

    for (auto const& shape : rShapes)
    {
        std::vector< oox::drawingml::ShapePtr >& rChildren( shape->getChildren() );
        for (auto const& child : rChildren)
        {
            PPTShape* pPPTShape = dynamic_cast< PPTShape* >( child.get() );
            basegfx::B2DHomMatrix aTransformation;
            if ( pPPTShape )
            {
                pPPTShape->addShape( rFilterBase, *this, getTheme().get(), xShapes, aTransformation, &getShapeMap() );

                const auto pIter = maShapeMap.find(pPPTShape->getId());
                if (pIter != maShapeMap.end())
                    lcl_createShapeMap(pIter->second, aConnectorShapeMap);
            }
            else
                child->addShape( rFilterBase, getTheme().get(), xShapes, aTransformation, maShapesPtr->getFillProperties(), &getShapeMap() );
        }
    }

    if (!aConnectorShapeMap.empty())
    {
        for (auto& pIt : aConnectorShapeMap)
        {
            ConnectorHelper::applyConnections(pIt.second, getShapeMap());

            SdrObject* pObj = SdrObject::getSdrObjectFromXShape(pIt.second->getXShape());
            SdrModel& rModel(pObj->getSdrModelFromSdrObject());
            rModel.setLock(false);

            if (pIt.second->getConnectorName() == u"bentConnector3"_ustr
                || pIt.second->getConnectorName() == u"bentConnector4"_ustr
                || pIt.second->getConnectorName() == u"bentConnector5"_ustr)
            {
                ConnectorHelper::applyBentHandleAdjustments(pIt.second);
            }
            else if (pIt.second->getConnectorName() == u"curvedConnector3"_ustr
                     || pIt.second->getConnectorName() == u"curvedConnector4"_ustr
                     || pIt.second->getConnectorName() == u"curvedConnector5"_ustr)
            {
                ConnectorHelper::applyCurvedHandleAdjustments(pIt.second);
            }
            else // bentConnector2
                createConnectorShapeConnection(pIt.second);
        }
    }

    Reference< XAnimationNodeSupplier > xNodeSupplier( getPage(), UNO_QUERY);
    if( !xNodeSupplier.is() )
        return;

    Reference< XAnimationNode > xNode( xNodeSupplier->getAnimationNode() );
    if( xNode.is() && !maTimeNodeList.empty() )
    {
        SlidePersistPtr pSlidePtr( shared_from_this() );
        TimeNodePtr pNode(maTimeNodeList.front());
        OSL_ENSURE( pNode, "pNode" );

        Reference<XAnimationNode> xDummy;
        pNode->setNode(rFilterBase, xNode, pSlidePtr, xDummy);
    }
}

void SlidePersist::createBackground( const XmlFilterBase& rFilterBase )
{
    if ( mpBackgroundPropertiesPtr )
    {
        ::Color nPhClr = maBackgroundColor.isUsed() ?
            maBackgroundColor.getColor( rFilterBase.getGraphicHelper() ) : API_RGB_TRANSPARENT;

        css::awt::Size aSize;
        Reference< css::beans::XPropertySet > xSet(mxPage, UNO_QUERY);
        xSet->getPropertyValue(u"Width"_ustr) >>= aSize.Width;
        xSet->getPropertyValue(u"Height"_ustr) >>= aSize.Height;

        oox::drawingml::ShapePropertyIds aPropertyIds = oox::drawingml::ShapePropertyInfo::DEFAULT.mrPropertyIds;
        aPropertyIds[oox::drawingml::ShapeProperty::FillGradient] = PROP_FillGradientName;
        oox::drawingml::ShapePropertyInfo aPropInfo( aPropertyIds, true, false, true, false, false );
        oox::drawingml::ShapePropertyMap aPropMap( rFilterBase.getModelObjectHelper(), aPropInfo );
        mpBackgroundPropertiesPtr->pushToPropMap( aPropMap, rFilterBase.getGraphicHelper(), 0, nPhClr, aSize);
        PropertySet( mxPage ).setProperty( PROP_Background, aPropMap.makePropertySet() );
    }
}

static void setTextStyle( Reference< beans::XPropertySet > const & rxPropSet, const XmlFilterBase& rFilter,
    oox::drawingml::TextListStylePtr const & pTextListStylePtr, int nLevel )
{
    ::oox::drawingml::TextParagraphProperties* pTextParagraphPropertiesPtr( &pTextListStylePtr->getListStyle()[ nLevel ] );
    if( pTextParagraphPropertiesPtr == nullptr )
    {
        // no properties. return
        return;
    }

    PropertyMap& rTextParagraphPropertyMap( pTextParagraphPropertiesPtr->getTextParagraphPropertyMap() );

    PropertySet aPropSet( rxPropSet );
    aPropSet.setProperties( rTextParagraphPropertyMap );
    pTextParagraphPropertiesPtr->getTextCharacterProperties().pushToPropSet( aPropSet, rFilter );
}

void SlidePersist::applyTextStyles( const XmlFilterBase& rFilterBase )
{
    if ( !mbMaster )
        return;

    try
    {
        Reference< style::XStyleFamiliesSupplier > aXStyleFamiliesSupplier( rFilterBase.getModel(), UNO_QUERY_THROW );
        Reference< container::XNameAccess > aXNameAccess( aXStyleFamiliesSupplier->getStyleFamilies() );
        Reference< container::XNamed > aXNamed( mxPage, UNO_QUERY_THROW );

        if ( aXNameAccess.is() )
        {
            oox::drawingml::TextListStylePtr pTextListStylePtr;
            OUString aStyle;
            OUString aFamily;

            static constexpr OUStringLiteral sOutline( u"outline1" );
            static constexpr OUString sTitle( u"title"_ustr );
            static constexpr OUStringLiteral sStandard( u"standard" );
            static constexpr OUStringLiteral sSubtitle( u"subtitle" );

            for( int i = 0; i < 4; i++ )    // todo: aggregation of bodystyle (subtitle)
            {
                switch( i )
                {
                    case 0 :    // title style
                    {
                        pTextListStylePtr = maTitleTextStylePtr;
                        aStyle = sTitle;
                        aFamily= aXNamed->getName();
                        break;
                    }
                    case 1 :    // body style
                    {
                        pTextListStylePtr = maBodyTextStylePtr;
                        aStyle = sOutline;
                        aFamily= aXNamed->getName();
                        break;
                    }
                    case 3 :    // notes style
                    {
                        pTextListStylePtr = maNotesTextStylePtr;
                        aStyle = sTitle;
                        aFamily= aXNamed->getName();
                        break;
                    }
                    case 4 :    // standard style
                    {
                        pTextListStylePtr = maOtherTextStylePtr;
                        aStyle = sStandard;
                        aFamily = "graphics";
                        break;
                    }
                    case 5 :    // subtitle
                    {
                        pTextListStylePtr = maBodyTextStylePtr;
                        aStyle = sSubtitle;
                        aFamily = aXNamed->getName();
                        break;
                    }
                }
                Reference< container::XNameAccess > xFamilies;
                if ( aXNameAccess->hasByName( aFamily ) )
                {
                    if( aXNameAccess->getByName( aFamily ) >>= xFamilies )
                    {
                        if ( xFamilies->hasByName( aStyle ) )
                        {
                            Reference< style::XStyle > aXStyle;
                            if ( xFamilies->getByName( aStyle ) >>= aXStyle )
                            {
                                Reference< beans::XPropertySet > xPropSet( aXStyle, UNO_QUERY_THROW );
                                setTextStyle( xPropSet, rFilterBase, maDefaultTextStylePtr, 0 );
                                setTextStyle( xPropSet, rFilterBase, pTextListStylePtr, 0 );
                                if ( i == 1 /* BodyStyle */ )
                                {
                                    for ( int nLevel = 1; nLevel < 5; nLevel++ )
                                    {
                                        {
                                            char pOutline[ 9 ] = "outline1";
                                            pOutline[ 7 ] = static_cast< char >( '0' + nLevel );
                                            OUString sOutlineStyle( OUString::createFromAscii( pOutline ) );
                                            if ( xFamilies->hasByName( sOutlineStyle ) )
                                            {
                                                xFamilies->getByName( sOutlineStyle ) >>= aXStyle;
                                                if( aXStyle.is() )
                                                    xPropSet.set( aXStyle, UNO_QUERY_THROW );
                                            }
                                        }
                                        setTextStyle( xPropSet, rFilterBase, maDefaultTextStylePtr, nLevel );
                                        setTextStyle( xPropSet, rFilterBase, pTextListStylePtr, nLevel );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch( const Exception& )
    {
    }
}

void SlidePersist::hideShapesAsMasterShapes()
{
    std::vector< oox::drawingml::ShapePtr >& rShapes( maShapesPtr->getChildren() );
    for (auto const& shape : rShapes)
    {
        std::vector< oox::drawingml::ShapePtr >& rChildren( shape->getChildren() );
        for (auto const& child : rChildren)
        {
            PPTShape* pPPTShape = dynamic_cast< PPTShape* >( child.get() );
            if (!pPPTShape)
                continue;
            pPPTShape->setHiddenMasterShape( true );
        }
    }
}

// This angle determines in the direction of the line
static sal_Int32 lcl_GetAngle(const uno::Reference<drawing::XShape>& rXShape, const awt::Point& rPt)
{
    SdrObject* pObj = SdrObject::getSdrObjectFromXShape(rXShape);
    tools::Rectangle aR(pObj->GetSnapRect());
    sal_Int32 nLeftX = rPt.X - aR.Left();
    sal_Int32 nTopY = rPt.Y - aR.Top();
    sal_Int32 nRightX = aR.Right() - rPt.X;
    sal_Int32 nBottomY = aR.Bottom() - rPt.Y;
    sal_Int32 nX = std::min(nLeftX, nRightX);
    sal_Int32 nY = std::min(nTopY, nBottomY);

    sal_Int32 nAngle;
    if (nX < nY)
    {
        if (nLeftX < nRightX)
            nAngle = 180; // Left
        else
            nAngle = 0; // Right
    }
    else
    {
        if (nTopY < nBottomY)
            nAngle = 270; // Top
        else
            nAngle = 90; // Bottom
    }
    return nAngle;
}

Reference<XAnimationNode> SlidePersist::getAnimationNode(const OUString& sId) const
{
    const auto pIter = maAnimNodesMap.find(sId);
    if (pIter != maAnimNodesMap.end())
        return pIter->second;

    Reference<XAnimationNode> aResult;
    return aResult;
}

static void lcl_SetEdgeLineValue(const uno::Reference<drawing::XShape>& rXConnector,
                                 const oox::drawingml::ShapePtr& rShapePtr)
{
    sal_Int32 nEdge = 0;
    awt::Point aStartPt, aEndPt;
    tools::Rectangle aS, aE; // Start, End rectangle
    uno::Reference<drawing::XShape> xStartSp, xEndSp;
    uno::Reference<beans::XPropertySet> xPropSet(rXConnector, uno::UNO_QUERY);
    xPropSet->getPropertyValue(u"EdgeStartPoint"_ustr) >>= aStartPt;
    xPropSet->getPropertyValue(u"EdgeEndPoint"_ustr) >>= aEndPt;
    xPropSet->getPropertyValue(u"StartShape"_ustr) >>= xStartSp;
    xPropSet->getPropertyValue(u"EndShape"_ustr) >>= xEndSp;
    xPropSet->setPropertyValue(u"EdgeNode1HorzDist"_ustr, Any(sal_Int32(0)));
    xPropSet->setPropertyValue(u"EdgeNode1VertDist"_ustr, Any(sal_Int32(0)));
    xPropSet->setPropertyValue(u"EdgeNode2HorzDist"_ustr, Any(sal_Int32(0)));
    xPropSet->setPropertyValue(u"EdgeNode2VertDist"_ustr, Any(sal_Int32(0)));

    SdrObject* pStartObj = xStartSp.is() ? SdrObject::getSdrObjectFromXShape(xStartSp) : nullptr;
    SdrObject* pEndObj = xEndSp.is() ? SdrObject::getSdrObjectFromXShape(xEndSp) : nullptr;

    sal_Int32 nStartA = -1;
    sal_Int32 nEndA = -1;
    if (pStartObj)
    {
        aS = pStartObj->GetSnapRect();
        nStartA = lcl_GetAngle(xStartSp, aStartPt);
    }
    if (pEndObj)
    {
        aE = pEndObj->GetSnapRect();
        nEndA = lcl_GetAngle(xEndSp, aEndPt);
    }

    const OUString sConnectorName = rShapePtr->getConnectorName();
    if (sConnectorName == "bentConnector2")
    {
        awt::Size aConnSize = rXConnector->getSize();
        if (xStartSp.is() || xEndSp.is())
        {
            if (nStartA >= 0)
            {
                switch (nStartA)
                {
                    case 0:   nEdge = aEndPt.X - aS.Right();  break;
                    case 180: nEdge = aEndPt.X - aS.Left();   break;
                    case 90:  nEdge = aEndPt.Y - aS.Bottom(); break;
                    case 270: nEdge = aEndPt.Y - aS.Top();    break;
                }
            }
            else
            {
                switch (nEndA)
                {
                    case 0:   nEdge = aStartPt.X - aE.Right();  break;
                    case 180: nEdge = aStartPt.X - aE.Left();   break;
                    case 90:  nEdge = aStartPt.Y - aE.Bottom(); break;
                    case 270: nEdge = aStartPt.Y - aE.Top();    break;
                }
            }
        }
        else
        {
            bool bFlipH = rShapePtr->getFlipH();
            bool bFlipV = rShapePtr->getFlipV();
            sal_Int32 nConnectorAngle = rShapePtr->getRotation() / 60000;
            if (aConnSize.Height < aConnSize.Width)
            {
                if ((nConnectorAngle == 90 && bFlipH && bFlipV) || (nConnectorAngle == 180)
                    || (nConnectorAngle == 270 && bFlipH))
                    nEdge -= aConnSize.Width;
                else
                    nEdge += aConnSize.Width;
            }
            else
            {
                if ((nConnectorAngle == 180 && bFlipV) || (nConnectorAngle == 270 && bFlipV)
                    || (nConnectorAngle == 90 && bFlipH && bFlipV)
                    || (nConnectorAngle == 0 && !bFlipV))
                    nEdge -= aConnSize.Height;
                else
                    nEdge += aConnSize.Height;
            }
        }
        xPropSet->setPropertyValue(u"EdgeLine1Delta"_ustr, Any(nEdge / 2));
    }
}

// create connection between two shape with a connector shape.
void SlidePersist::createConnectorShapeConnection(const oox::drawingml::ShapePtr& pConnector)
{
    oox::drawingml::ConnectorShapePropertiesList aConnectorShapeProperties
        = pConnector->getConnectorShapeProperties();
    uno::Reference<drawing::XShape> xConnector(pConnector->getXShape(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xConnector, uno::UNO_QUERY);

    if (xConnector.is())
    {
        sal_Int32 nCount = aConnectorShapeProperties.size();
        for (sal_Int32 j = 0; j < nCount; j++)
        {
            OUString aDestShapeId = aConnectorShapeProperties[j].maDestShapeId;
            const auto pShape = maShapeMap.find(aDestShapeId);
            if (pShape == maShapeMap.end())
                continue;
            uno::Reference<drawing::XShape> xShape(pShape->second->getXShape(), uno::UNO_QUERY);
            if (xShape.is())
            {
                uno::Reference<drawing::XGluePointsSupplier> xSupplier(xShape, uno::UNO_QUERY);
                css::uno::Reference<css::container::XIdentifierContainer> xGluePoints(
                    xSupplier->getGluePoints(), uno::UNO_QUERY);

                sal_Int32 nCountGluePoints = xGluePoints->getIdentifiers().getLength();
                sal_Int32 nGlueId = aConnectorShapeProperties[j].mnDestGlueId;

                // The first 4 glue points belong to the bounding box.
                if (nCountGluePoints > 4)
                    nGlueId += 4;
                else
                {
                    bool bFlipH = pShape->second->getFlipH();
                    bool bFlipV = pShape->second->getFlipV();
                    if ((!bFlipH && !bFlipV) || (bFlipH && bFlipV))
                    {
                        // change id of the left and right glue points of the bounding box (1 <-> 3)
                        if (nGlueId == 1)
                            nGlueId = 3; // Right
                        else if (nGlueId == 3)
                            nGlueId = 1; // Left
                    }
                }

                bool bStart = aConnectorShapeProperties[j].mbStartShape;
                if (bStart)
                {
                    xPropertySet->setPropertyValue(u"StartShape"_ustr, uno::Any(xShape));
                    xPropertySet->setPropertyValue(u"StartGluePointIndex"_ustr, uno::Any(nGlueId));
                }
                else
                {
                    xPropertySet->setPropertyValue(u"EndShape"_ustr, uno::Any(xShape));
                    xPropertySet->setPropertyValue(u"EndGluePointIndex"_ustr, uno::Any(nGlueId));
                }
            }
        }
        uno::Reference<beans::XPropertySetInfo> xPropInfo = xPropertySet->getPropertySetInfo();
        if (xPropInfo->hasPropertyByName(u"EdgeKind"_ustr))
        {
            ConnectorType aConnectorType;
            xPropertySet->getPropertyValue(u"EdgeKind"_ustr) >>= aConnectorType;
            if (aConnectorType == ConnectorType_STANDARD)
                lcl_SetEdgeLineValue(xConnector, pConnector);
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
