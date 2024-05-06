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

#include "VPolarGrid.hxx"
#include "VCartesianGrid.hxx"
#include "Tickmarks.hxx"
#include <GridProperties.hxx>
#include <PlottingPositionHelper.hxx>
#include <ShapeFactory.hxx>
#include <ObjectIdentifier.hxx>
#include <CommonConverters.hxx>
#include <VLineProperties.hxx>
#include "Tickmarks_Equidistant.hxx"

#include <osl/diagnose.h>

#include <vector>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

VPolarGrid::VPolarGrid( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
                       , std::vector< rtl::Reference< ::chart::GridProperties > > aGridPropertiesList )
            : VAxisOrGridBase( nDimensionIndex, nDimensionCount )
            , m_aGridPropertiesList( std::move(aGridPropertiesList) )
{
    PlotterBase::m_pPosHelper = &m_aPosHelper;
}

VPolarGrid::~VPolarGrid()
{
}

void VPolarGrid::setIncrements( std::vector< ExplicitIncrementData >&& rIncrements )
{
    m_aIncrements = std::move(rIncrements);
}

void VPolarGrid::getAllTickInfos( sal_Int32 nDimensionIndex, TickInfoArraysType& rAllTickInfos ) const
{
    const std::vector<ExplicitScaleData>& rScales = m_pPosHelper->getScales();
    TickFactory aTickFactory(rScales[nDimensionIndex], m_aIncrements[nDimensionIndex]);
    aTickFactory.getAllTicks( rAllTickInfos );
}

void VPolarGrid::createLinePointSequence_ForAngleAxis(
        drawing::PointSequenceSequence& rPoints
        , TickInfoArraysType& rAllTickInfos
        , const ExplicitIncrementData& rIncrement
        , const ExplicitScaleData& rScale
        , PolarPlottingPositionHelper const * pPosHelper
        , double fLogicRadius, double fLogicZ )
{
    Reference< XScaling > xInverseScaling;
    if( rScale.Scaling.is() )
        xInverseScaling = rScale.Scaling->getInverseScaling();

    sal_Int32 nTick = 0;
    EquidistantTickIter aIter( rAllTickInfos, rIncrement, 0 );
    auto pPoints = rPoints.getArray();
    for( TickInfo* pTickInfo = aIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = aIter.nextInfo(), nTick++ )
    {
        if(nTick>=rPoints[0].getLength())
            pPoints[0].realloc(rPoints[0].getLength()+30);
        auto pPoints0 = pPoints[0].getArray();

        //xxxxx pTickInfo->updateUnscaledValue( xInverseScaling );
        double fLogicAngle = pTickInfo->getUnscaledTickValue();

        drawing::Position3D aScenePosition3D( pPosHelper->transformAngleRadiusToScene( fLogicAngle, fLogicRadius, fLogicZ ) );
        pPoints0[nTick].X = static_cast<sal_Int32>(aScenePosition3D.PositionX);
        pPoints0[nTick].Y = static_cast<sal_Int32>(aScenePosition3D.PositionY);
    }
    if(rPoints[0].getLength()>1)
    {
        pPoints[0].realloc(nTick+1);
        auto pPoints0 = pPoints[0].getArray();
        pPoints0[nTick].X = rPoints[0][0].X;
        pPoints0[nTick].Y = rPoints[0][0].Y;
    }
    else
        pPoints[0].realloc(0);
}
#ifdef NOTYET
void VPolarGrid::create2DAngleGrid( const Reference< drawing::XShapes >& xLogicTarget
        , TickInfoArraysType& /* rRadiusTickInfos */
        , TickInfoArraysType& rAngleTickInfos
        , const std::vector<VLineProperties>& rLinePropertiesList )
{
    Reference< drawing::XShapes > xMainTarget(
        createGroupShape( xLogicTarget, m_aCID ) );

    const std::vector<ExplicitScaleData>& rScales = m_pPosHelper->getScales();
    const ExplicitScaleData& rAngleScale = rScales[0];
    Reference< XScaling > xInverseScaling( NULL );
    if( rAngleScale.Scaling.is() )
        xInverseScaling = rAngleScale.Scaling->getInverseScaling();

    double fLogicInnerRadius = m_pPosHelper->getInnerLogicRadius();
    double fLogicOuterRadius = m_pPosHelper->getOuterLogicRadius();

    sal_Int32 nLinePropertiesCount = rLinePropertiesList.size();
    if(nLinePropertiesCount)
    {
        double fLogicZ      = 1.0;//as defined
        sal_Int32 nDepth=0;
        //create axis main lines
        drawing::PointSequenceSequence aAllPoints;
        for (auto const& tick : rAngleTickInfos[0])
        {
            if( !tick.bPaintIt )
                continue;

            //xxxxx rTickInfo.updateUnscaledValue( xInverseScaling );
            double fLogicAngle = tick.getUnscaledTickValue();

            drawing::PointSequenceSequence aPoints(1);
            aPoints[0].realloc(2);
            drawing::Position3D aScenePositionStart( m_pPosHelper->transformAngleRadiusToScene( fLogicAngle, fLogicInnerRadius, fLogicZ ) );
            drawing::Position3D aScenePositionEnd(   m_pPosHelper->transformAngleRadiusToScene( fLogicAngle, fLogicOuterRadius, fLogicZ ) );
            aPoints[0][0].X = static_cast<sal_Int32>(aScenePositionStart.PositionX);
            aPoints[0][0].Y = static_cast<sal_Int32>(aScenePositionStart.PositionY);
            aPoints[0][1].X = static_cast<sal_Int32>(aScenePositionEnd.PositionX);
            aPoints[0][1].Y = static_cast<sal_Int32>(aScenePositionEnd.PositionY);
            appendPointSequence( aAllPoints, aPoints );
        }

        rtl::Reference<SvxShapePolyPolygon> xShape = ShapeFactory::createLine2D(
                xMainTarget, aAllPoints, &rLinePropertiesList[nDepth] );
        //because of this name this line will be used for marking
        m_pShapeFactory->setShapeName( xShape, "MarkHandles" );
    }
}
#endif

void VPolarGrid::create2DRadiusGrid( const rtl::Reference<SvxShapeGroupAnyD>& xLogicTarget
        , TickInfoArraysType& rRadiusTickInfos
        , TickInfoArraysType& rAngleTickInfos
        , const std::vector<VLineProperties>& rLinePropertiesList )
{
    rtl::Reference<SvxShapeGroupAnyD> xMainTarget =
        createGroupShape( xLogicTarget, m_aCID );

    const std::vector<ExplicitScaleData>& rScales = m_pPosHelper->getScales();
    const ExplicitScaleData&     rRadiusScale = rScales[1];
    const ExplicitScaleData&     rAngleScale = rScales[0];
    const ExplicitIncrementData& rAngleIncrement = m_aIncrements[0];
    Reference< XScaling > xInverseRadiusScaling;
    if( rRadiusScale.Scaling.is() )
        xInverseRadiusScaling = rRadiusScale.Scaling->getInverseScaling();

    sal_Int32 nLinePropertiesCount = rLinePropertiesList.size();
    TickInfoArraysType::iterator aDepthIter             = rRadiusTickInfos.begin();
    const TickInfoArraysType::const_iterator aDepthEnd  = rRadiusTickInfos.end();
    for( sal_Int32 nDepth=0
        ; aDepthIter != aDepthEnd && nDepth < nLinePropertiesCount
        ; ++aDepthIter, nDepth++ )
    {
        if( !rLinePropertiesList[nDepth].isLineVisible() )
            continue;

        rtl::Reference<SvxShapeGroupAnyD> xTarget( xMainTarget );
        if( nDepth > 0 )
        {
            xTarget = createGroupShape( xLogicTarget
                , ObjectIdentifier::addChildParticle( m_aCID, ObjectIdentifier::createChildParticleWithIndex( OBJECTTYPE_SUBGRID, nDepth-1 ) )
                );
            if(!xTarget.is())
                xTarget = xMainTarget;
        }

        //create axis main lines
        drawing::PointSequenceSequence aAllPoints;
        for (auto const& tick : *aDepthIter)
        {
            if( !tick.bPaintIt )
                continue;

            //xxxxx rTickInfo.updateUnscaledValue( xInverseRadiusScaling );
            double fLogicRadius = tick.getUnscaledTickValue();
            double const fLogicZ = 1.0;//as defined

            drawing::PointSequenceSequence aPoints(1);
            VPolarGrid::createLinePointSequence_ForAngleAxis( aPoints, rAngleTickInfos
                , rAngleIncrement, rAngleScale, &m_aPosHelper, fLogicRadius, fLogicZ );
            if(aPoints[0].getLength())
                appendPointSequence( aAllPoints, aPoints );
        }

        rtl::Reference<SvxShapePolyPolygon> xShape = ShapeFactory::createLine2D(
                xTarget, aAllPoints, &rLinePropertiesList[nDepth] );
        //because of this name this line will be used for marking
        ::chart::ShapeFactory::setShapeName( xShape, u"MarkHandles"_ustr );
    }
}

void VPolarGrid::createShapes()
{
    OSL_PRECOND(m_xLogicTarget.is()&&m_xFinalTarget.is(),"Axis is not proper initialized");
    if(!(m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;
    if(m_aGridPropertiesList.empty())
        return;

    //create all scaled tickmark values
    TickInfoArraysType aAngleTickInfos;
    TickInfoArraysType aRadiusTickInfos;
    getAllTickInfos( 0, aAngleTickInfos );
    getAllTickInfos( 1, aRadiusTickInfos );

    std::vector<VLineProperties> aLinePropertiesList;
    VCartesianGrid::fillLinePropertiesFromGridModel( aLinePropertiesList, m_aGridPropertiesList );

    //create tick mark line shapes
    if(m_nDimension==2)
    {
        if(m_nDimensionIndex==1)
            create2DRadiusGrid( m_xLogicTarget, aRadiusTickInfos, aAngleTickInfos, aLinePropertiesList );
        //else //no Angle Grid so far as this equals exactly the y axis positions
        //    create2DAngleGrid( m_xLogicTarget, aRadiusTickInfos, aAngleTickInfos, aLinePropertiesList );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
