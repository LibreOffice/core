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
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "ObjectIdentifier.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "AxisHelper.hxx"
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <vector>
#include <boost/scoped_ptr.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

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
    {
        double fHelp = MinX;
        MinX = MaxX;
        MaxX = fHelp;
    }
    if(!pPosHelper->isMathematicalOrientationY())
    {
        double fHelp = MinY;
        MinY = MaxY;
        MaxY = fHelp;
    }
    if(pPosHelper->isMathematicalOrientationZ())//z axis in draw is reverse to mathematical
    {
        double fHelp = MinZ;
        MinZ = MaxZ;
        MaxZ = fHelp;
    }
    bool bSwapXY = pPosHelper->isSwapXAndY();

    P0.realloc(3);
    P1.realloc(3);
    P2.realloc(3);

    //P0: point on 'back' wall, not on 'left' wall
    //P1: point on both walls
    //P2: point on 'left' wall not on 'back' wall

    P0[0]=P1[0]=P2[0]= (CuboidPlanePosition_Left == eLeftWallPos || bSwapXY) ? MinX : MaxX;
    P0[1]=P1[1]=P2[1]= (CuboidPlanePosition_Left == eLeftWallPos || !bSwapXY) ? MinY : MaxY;
    P0[2]=P1[2]=P2[2]= (CuboidPlanePosition_Back == eBackWallPos) ? MinZ : MaxZ;

    if(m_nDimensionIndex==0)
    {
        P0[1]= (CuboidPlanePosition_Left == eLeftWallPos || !bSwapXY) ? MaxY : MinY;
        P2[2]= (CuboidPlanePosition_Back == eBackWallPos) ? MaxZ : MinZ;
        if( CuboidPlanePosition_Bottom != eBottomPos && !bSwapXY )
            P2=P1;
    }
    else if(m_nDimensionIndex==1)
    {
        P0[0]= (CuboidPlanePosition_Left == eLeftWallPos || bSwapXY) ? MaxX : MinX;
        P2[2]= (CuboidPlanePosition_Back == eBackWallPos) ? MaxZ : MinZ;
        if( CuboidPlanePosition_Bottom != eBottomPos && bSwapXY )
            P2=P1;
    }
    else if(m_nDimensionIndex==2)
    {
        P0[0]= (CuboidPlanePosition_Left == eLeftWallPos || bSwapXY) ? MaxX : MinX;
        P2[1]= (CuboidPlanePosition_Left == eLeftWallPos || !bSwapXY) ? MaxY : MinY;
        if( CuboidPlanePosition_Bottom != eBottomPos )
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
    P0[m_nDimensionIndex] = P1[m_nDimensionIndex] = P2[m_nDimensionIndex] = fScaledTickValue;
}

void addLine2D( drawing::PointSequenceSequence& rPoints, sal_Int32 nIndex
             , const GridLinePoints& rScaledLogicPoints
             , const Reference< XTransformation > & xTransformation
              )
{
    drawing::Position3D aPA = SequenceToPosition3D( xTransformation->transform( rScaledLogicPoints.P0 ) );
    drawing::Position3D aPB = SequenceToPosition3D( xTransformation->transform( rScaledLogicPoints.P1 ) );

    rPoints[nIndex].realloc(2);
    rPoints[nIndex][0].X = static_cast<sal_Int32>(aPA.PositionX);
    rPoints[nIndex][0].Y = static_cast<sal_Int32>(aPA.PositionY);
    rPoints[nIndex][1].X = static_cast<sal_Int32>(aPB.PositionX);
    rPoints[nIndex][1].Y = static_cast<sal_Int32>(aPB.PositionY);
}

void addLine3D( drawing::PolyPolygonShape3D& rPoints, sal_Int32 nIndex
            , const GridLinePoints& rBasePoints
            , const Reference< XTransformation > & xTransformation )
{
    drawing::Position3D aPoint = SequenceToPosition3D( xTransformation->transform( rBasePoints.P0 ) );
    AddPointToPoly( rPoints, aPoint, nIndex );
    aPoint = SequenceToPosition3D( xTransformation->transform( rBasePoints.P1 ) );
    AddPointToPoly( rPoints, aPoint, nIndex );
    aPoint = SequenceToPosition3D( xTransformation->transform( rBasePoints.P2 ) );
    AddPointToPoly( rPoints, aPoint, nIndex );
}

VCartesianGrid::VCartesianGrid( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
                               , const Sequence< Reference< beans::XPropertySet > > & rGridPropertiesList )
            : VAxisOrGridBase( nDimensionIndex, nDimensionCount )
            , m_aGridPropertiesList( rGridPropertiesList )
{
    m_pPosHelper = new PlottingPositionHelper();
}

VCartesianGrid::~VCartesianGrid()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

void VCartesianGrid::fillLinePropertiesFromGridModel( ::std::vector<VLineProperties>& rLinePropertiesList
                                     , const Sequence< Reference< beans::XPropertySet > > & rGridPropertiesList )
{
    rLinePropertiesList.clear();
    if( !rGridPropertiesList.getLength() )
        return;

    VLineProperties aLineProperties;
    for( sal_Int32 nN=0; nN < rGridPropertiesList.getLength(); nN++ )
    {
        if(!AxisHelper::isGridVisible( rGridPropertiesList[nN] ))
            aLineProperties.LineStyle = uno::makeAny( drawing::LineStyle_NONE );
        else
            aLineProperties.initFromPropertySet( rGridPropertiesList[nN] );
        rLinePropertiesList.push_back(aLineProperties);
    }
};

void VCartesianGrid::createShapes()
{
    if(!m_aGridPropertiesList.getLength())
        return;
    //somehow equal to axis tickmarks

    //create named group shape
    Reference< drawing::XShapes > xGroupShape_Shapes(
        this->createGroupShape( m_xLogicTarget, m_aCID ) );

    if(!xGroupShape_Shapes.is())
        return;

    ::std::vector<VLineProperties> aLinePropertiesList;
    fillLinePropertiesFromGridModel( aLinePropertiesList, m_aGridPropertiesList );

    //create all scaled tickmark values
    boost::scoped_ptr< TickFactory > apTickFactory( this->createTickFactory() );
    TickFactory& aTickFactory = *apTickFactory.get();
    ::std::vector< ::std::vector< TickInfo > > aAllTickInfos;
    aTickFactory.getAllTicks( aAllTickInfos );

    //create tick mark line shapes
    ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter             = aAllTickInfos.begin();
    const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = aAllTickInfos.end();

    if(aDepthIter == aDepthEnd)//no tickmarks at all
        return;

    sal_Int32 nLinePropertiesCount = aLinePropertiesList.size();
    for( sal_Int32 nDepth=0
        ; aDepthIter != aDepthEnd && nDepth < nLinePropertiesCount
        ; ++aDepthIter, nDepth++ )
    {
        if( !aLinePropertiesList[nDepth].isLineVisible() )
            continue;

        Reference< drawing::XShapes > xTarget( xGroupShape_Shapes );
        if( nDepth > 0 )
        {
            xTarget.set( this->createGroupShape( m_xLogicTarget
                , ObjectIdentifier::addChildParticle( m_aCID, ObjectIdentifier::createChildParticleWithIndex( OBJECTTYPE_SUBGRID, nDepth-1 ) )
                ) );
            if(!xTarget.is())
                xTarget.set( xGroupShape_Shapes );
        }

        if(2==m_nDimension)
        {

            GridLinePoints aGridLinePoints( m_pPosHelper, m_nDimensionIndex );

            sal_Int32 nPointCount = (*aDepthIter).size();
            drawing::PointSequenceSequence aPoints(nPointCount);

            ::std::vector< TickInfo >::const_iterator       aTickIter = (*aDepthIter).begin();
            const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
            sal_Int32 nRealPointCount = 0;
            for( ; aTickIter != aTickEnd; ++aTickIter )
            {
                if( !(*aTickIter).bPaintIt )
                    continue;
                aGridLinePoints.update( (*aTickIter).fScaledTickValue );
                addLine2D( aPoints, nRealPointCount, aGridLinePoints, m_pPosHelper->getTransformationScaledLogicToScene() );
                nRealPointCount++;
            }
            aPoints.realloc(nRealPointCount);
            m_pShapeFactory->createLine2D( xTarget, aPoints, &aLinePropertiesList[nDepth] );

            //prepare polygon for handle shape:
            drawing::PointSequenceSequence aHandlesPoints(1);
            sal_Int32 nOldHandleCount = aHandlesPoints[0].getLength();
            aHandlesPoints[0].realloc(nOldHandleCount+nRealPointCount);
            for( sal_Int32 nN = 0; nN<nRealPointCount; nN++)
                aHandlesPoints[0][nOldHandleCount+nN] = aPoints[nN][1];

            //create handle shape:
            VLineProperties aHandleLineProperties;
            aHandleLineProperties.LineStyle    = uno::makeAny( drawing::LineStyle_NONE );
            Reference< drawing::XShape > xHandleShape =
                m_pShapeFactory->createLine2D( xTarget, aHandlesPoints, &aHandleLineProperties );
            m_pShapeFactory->setShapeName( xHandleShape, "HandlesOnly" );
        }
        else //if(2!=m_nDimension)
        {
            GridLinePoints aGridLinePoints( m_pPosHelper, m_nDimensionIndex, m_eLeftWallPos, m_eBackWallPos, m_eBottomPos );

            sal_Int32 nPointCount = (*aDepthIter).size();
            drawing::PolyPolygonShape3D aPoints;
            aPoints.SequenceX.realloc(nPointCount);
            aPoints.SequenceY.realloc(nPointCount);
            aPoints.SequenceZ.realloc(nPointCount);

            ::std::vector< TickInfo >::const_iterator       aTickIter = (*aDepthIter).begin();
            const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
            sal_Int32 nRealPointCount = 0;
            sal_Int32 nPolyIndex = 0;
            for( ; aTickIter != aTickEnd; ++aTickIter, ++nPolyIndex )
            {
                if( !(*aTickIter).bPaintIt )
                    continue;

                aGridLinePoints.update( (*aTickIter).fScaledTickValue );
                addLine3D( aPoints, nPolyIndex, aGridLinePoints, m_pPosHelper->getTransformationScaledLogicToScene() );
                nRealPointCount+=3;
            }
            aPoints.SequenceX.realloc(nRealPointCount);
            aPoints.SequenceY.realloc(nRealPointCount);
            aPoints.SequenceZ.realloc(nRealPointCount);
            m_pShapeFactory->createLine3D( xTarget, aPoints, aLinePropertiesList[nDepth] );
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
