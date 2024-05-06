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

#include "VCartesianGrid.hxx"
#include "Tickmarks.hxx"
#include <PlottingPositionHelper.hxx>
#include <ShapeFactory.hxx>
#include <ObjectIdentifier.hxx>
#include <CommonConverters.hxx>
#include <AxisHelper.hxx>
#include <VLineProperties.hxx>
#include <GridProperties.hxx>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <memory>
#include <vector>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace {

struct GridLinePoints
{
    Sequence< double > P0;
    Sequence< double > P1;
    Sequence< double > P2;

    GridLinePoints( const PlottingPositionHelper* pPosHelper, sal_Int32 nDimensionIndex
        , CuboidPlanePosition eLeftWallPos=CuboidPlanePosition_Left
        , CuboidPlanePosition eBackWallPos=CuboidPlanePosition_Back
        , CuboidPlanePosition eBottomPos=CuboidPlanePosition_Bottom );
    void update( double fScaledTickValue );

    sal_Int32 m_nDimensionIndex;
};

}

GridLinePoints::GridLinePoints( const PlottingPositionHelper* pPosHelper, sal_Int32 nDimensionIndex
                , CuboidPlanePosition eLeftWallPos
                , CuboidPlanePosition eBackWallPos
                , CuboidPlanePosition eBottomPos )
                : m_nDimensionIndex(nDimensionIndex)
{
    double MinX = pPosHelper->getLogicMinX();
    double MinY = pPosHelper->getLogicMinY();
    double MinZ = pPosHelper->getLogicMinZ();
    double MaxX = pPosHelper->getLogicMaxX();
    double MaxY = pPosHelper->getLogicMaxY();
    double MaxZ = pPosHelper->getLogicMaxZ();

    pPosHelper->doLogicScaling( &MinX,&MinY,&MinZ );
    pPosHelper->doLogicScaling( &MaxX,&MaxY,&MaxZ );

    if(!pPosHelper->isMathematicalOrientationX())
        std::swap( MinX, MaxX );
    if(!pPosHelper->isMathematicalOrientationY())
        std::swap( MinY, MaxY );
    if(pPosHelper->isMathematicalOrientationZ())//z axis in draw is reverse to mathematical
        std::swap( MinZ, MaxZ );
    bool bSwapXY = pPosHelper->isSwapXAndY();

    //P0: point on 'back' wall, not on 'left' wall
    //P1: point on both walls
    //P2: point on 'left' wall not on 'back' wall

    const double v0 = (eLeftWallPos == CuboidPlanePosition_Left || bSwapXY) ? MinX : MaxX;
    const double v1 = (eLeftWallPos == CuboidPlanePosition_Left || !bSwapXY) ? MinY : MaxY;
    const double v2 = (eBackWallPos == CuboidPlanePosition_Back) ? MinZ : MaxZ;
    P0 = P1 = P2 = { v0, v1, v2 };

    if(m_nDimensionIndex==0)
    {
        P0.getArray()[1] = (eLeftWallPos == CuboidPlanePosition_Left || !bSwapXY) ? MaxY : MinY;
        P2.getArray()[2]= (eBackWallPos == CuboidPlanePosition_Back) ? MaxZ : MinZ;
        if( eBottomPos != CuboidPlanePosition_Bottom && !bSwapXY )
            P2=P1;
    }
    else if(m_nDimensionIndex==1)
    {
        P0.getArray()[0]= (eLeftWallPos == CuboidPlanePosition_Left || bSwapXY) ? MaxX : MinX;
        P2.getArray()[2]= (eBackWallPos == CuboidPlanePosition_Back) ? MaxZ : MinZ;
        if( eBottomPos != CuboidPlanePosition_Bottom && bSwapXY )
            P2=P1;
    }
    else if(m_nDimensionIndex==2)
    {
        P0.getArray()[0]= (eLeftWallPos == CuboidPlanePosition_Left || bSwapXY) ? MaxX : MinX;
        P2.getArray()[1]= (eLeftWallPos == CuboidPlanePosition_Left || !bSwapXY) ? MaxY : MinY;
        if( eBottomPos != CuboidPlanePosition_Bottom )
        {
            if( !bSwapXY )
                P0=P1;
            else
                P2=P1;
        }
    }
}

void GridLinePoints::update( double fScaledTickValue )
{
    P0.getArray()[m_nDimensionIndex] = P1.getArray()[m_nDimensionIndex] = P2.getArray()[m_nDimensionIndex] = fScaledTickValue;
}

static void addLine2D( drawing::PointSequenceSequence& rPoints, sal_Int32 nIndex
             , const GridLinePoints& rScaledLogicPoints
             , const XTransformation2& rTransformation
              )
{
    drawing::Position3D aPA = rTransformation.transform( SequenceToPosition3D(rScaledLogicPoints.P0) );
    drawing::Position3D aPB = rTransformation.transform( SequenceToPosition3D(rScaledLogicPoints.P1) );

    rPoints.getArray()[nIndex]
        = { { static_cast<sal_Int32>(aPA.PositionX), static_cast<sal_Int32>(aPA.PositionY) },
            { static_cast<sal_Int32>(aPB.PositionX), static_cast<sal_Int32>(aPB.PositionY) } };
}

static void addLine3D( std::vector<std::vector<css::drawing::Position3D>>& rPoints, sal_Int32 nIndex
            , const GridLinePoints& rBasePoints
            , const XTransformation2 & rTransformation )
{
    drawing::Position3D aPoint =rTransformation.transform( rBasePoints.P0 );
    AddPointToPoly( rPoints, aPoint, nIndex );
    aPoint = rTransformation.transform( rBasePoints.P1 );
    AddPointToPoly( rPoints, aPoint, nIndex );
    aPoint = rTransformation.transform( rBasePoints.P2 );
    AddPointToPoly( rPoints, aPoint, nIndex );
}

VCartesianGrid::VCartesianGrid( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
                               , std::vector< rtl::Reference< ::chart::GridProperties > > aGridPropertiesList )
            : VAxisOrGridBase( nDimensionIndex, nDimensionCount )
            , m_aGridPropertiesList( std::move(aGridPropertiesList) )
{
    m_pPosHelper = new PlottingPositionHelper();
}

VCartesianGrid::~VCartesianGrid()
{
    delete m_pPosHelper;
    m_pPosHelper = nullptr;
}

void VCartesianGrid::fillLinePropertiesFromGridModel( std::vector<VLineProperties>& rLinePropertiesList
                                     , const std::vector< rtl::Reference< ::chart::GridProperties > > & rGridPropertiesList )
{
    rLinePropertiesList.clear();
    if( rGridPropertiesList.empty() )
        return;

    VLineProperties aLineProperties;
    for( const auto & rxPropSet : rGridPropertiesList )
    {
        if(!AxisHelper::isGridVisible( rxPropSet ))
            aLineProperties.LineStyle <<= drawing::LineStyle_NONE;
        else
            aLineProperties.initFromPropertySet( rxPropSet );
        rLinePropertiesList.push_back(aLineProperties);
    }
};

void VCartesianGrid::createShapes()
{
    if(m_aGridPropertiesList.empty())
        return;
    //somehow equal to axis tickmarks

    //create named group shape
    rtl::Reference< SvxShapeGroupAnyD > xGroupShape_Shapes(
        createGroupShape( m_xLogicTarget, m_aCID ) );

    if(!xGroupShape_Shapes.is())
        return;

    std::vector<VLineProperties> aLinePropertiesList;
    fillLinePropertiesFromGridModel( aLinePropertiesList, m_aGridPropertiesList );

    //create all scaled tickmark values
    std::unique_ptr< TickFactory > apTickFactory( createTickFactory() );
    TickFactory& aTickFactory = *apTickFactory;
    TickInfoArraysType aAllTickInfos;
    aTickFactory.getAllTicks( aAllTickInfos );

    //create tick mark line shapes

    if(aAllTickInfos.empty())//no tickmarks at all
        return;

    TickInfoArraysType::iterator aDepthIter             = aAllTickInfos.begin();
    const TickInfoArraysType::const_iterator aDepthEnd  = aAllTickInfos.end();

    sal_Int32 nLinePropertiesCount = aLinePropertiesList.size();
    for( sal_Int32 nDepth=0
        ; aDepthIter != aDepthEnd && nDepth < nLinePropertiesCount
        ; ++aDepthIter, nDepth++ )
    {
        if( !aLinePropertiesList[nDepth].isLineVisible() )
            continue;

        rtl::Reference< SvxShapeGroupAnyD > xTarget( xGroupShape_Shapes );
        if( nDepth > 0 )
        {
            xTarget = createGroupShape( m_xLogicTarget
                , ObjectIdentifier::addChildParticle( m_aCID, ObjectIdentifier::createChildParticleWithIndex( OBJECTTYPE_SUBGRID, nDepth-1 ) )
                );
            if(!xTarget.is())
                xTarget = xGroupShape_Shapes;
        }

        if(m_nDimension==2)
        {

            GridLinePoints aGridLinePoints( m_pPosHelper, m_nDimensionIndex );

            sal_Int32 nPointCount = (*aDepthIter).size();
            drawing::PointSequenceSequence aPoints(nPointCount);

            sal_Int32 nRealPointCount = 0;
            for (auto const& tick : *aDepthIter)
            {
                if( !tick.bPaintIt )
                    continue;
                aGridLinePoints.update( tick.fScaledTickValue );
                addLine2D( aPoints, nRealPointCount, aGridLinePoints, *m_pPosHelper->getTransformationScaledLogicToScene() );
                nRealPointCount++;
            }
            aPoints.realloc(nRealPointCount);
            ShapeFactory::createLine2D( xTarget, aPoints, &aLinePropertiesList[nDepth] );

            //prepare polygon for handle shape:
            drawing::PointSequenceSequence aHandlesPoints(1);
            auto pHandlesPoints = aHandlesPoints.getArray();
            pHandlesPoints[0].realloc(nRealPointCount);
            auto pHandlesPoints0 = pHandlesPoints[0].getArray();
            for( sal_Int32 nN = 0; nN<nRealPointCount; nN++)
                pHandlesPoints0[nN] = aPoints[nN][1];

            //create handle shape:
            VLineProperties aHandleLineProperties;
            aHandleLineProperties.LineStyle    <<= drawing::LineStyle_NONE;
            rtl::Reference<SvxShapePolyPolygon> xHandleShape =
                ShapeFactory::createLine2D( xTarget, aHandlesPoints, &aHandleLineProperties );
            ::chart::ShapeFactory::setShapeName( xHandleShape, u"HandlesOnly"_ustr );
        }
        else //if(2!=m_nDimension)
        {
            GridLinePoints aGridLinePoints( m_pPosHelper, m_nDimensionIndex, m_eLeftWallPos, m_eBackWallPos, m_eBottomPos );

            sal_Int32 nPointCount = (*aDepthIter).size();
            std::vector<std::vector<css::drawing::Position3D>> aPoints;
            aPoints.resize(nPointCount);

            sal_Int32 nRealPointCount = 0;
            sal_Int32 nPolyIndex = 0;
            for (auto const& tick : *aDepthIter)
            {
                if( !tick.bPaintIt )
                {
                    ++nPolyIndex;
                    continue;
                }

                aGridLinePoints.update( tick.fScaledTickValue );
                addLine3D( aPoints, nPolyIndex, aGridLinePoints, *m_pPosHelper->getTransformationScaledLogicToScene() );
                nRealPointCount+=3;
                ++nPolyIndex;
            }
            aPoints.resize(nRealPointCount);
            ShapeFactory::createLine3D( xTarget, aPoints, aLinePropertiesList[nDepth] );
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
