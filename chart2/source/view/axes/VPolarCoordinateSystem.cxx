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

#include "VPolarCoordinateSystem.hxx"
#include "VPolarGrid.hxx"
#include "VPolarAxis.hxx"
#include <AxisIndexDefines.hxx>
#include <AxisHelper.hxx>
#include <ChartModel.hxx>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

VPolarCoordinateSystem::VPolarCoordinateSystem( const Reference< XCoordinateSystem >& xCooSys )
    : VCoordinateSystem(xCooSys)
{
}

VPolarCoordinateSystem::~VPolarCoordinateSystem()
{
}

//better performance for big data
uno::Sequence< sal_Int32 > VPolarCoordinateSystem::getCoordinateSystemResolution(
            const awt::Size& rPageSize, const awt::Size& rPageResolution )
{
    uno::Sequence< sal_Int32 > aResolution( VCoordinateSystem::getCoordinateSystemResolution( rPageSize, rPageResolution) );

    if( aResolution.getLength() >= 2 )
    {
        if( getPropertySwapXAndYAxis() )
        {
            aResolution[0]/=2;//radius
            aResolution[1]*=4;//outer circle resolution
        }
        else
        {
            aResolution[0]*=4;//outer circle resolution
            aResolution[1]/=2;//radius
        }
    }

    return aResolution;
}

void VPolarCoordinateSystem::createVAxisList(
              const uno::Reference<chart2::XChartDocument> & xChartDoc
            , const awt::Size& rFontReferenceSize
            , const awt::Rectangle& rMaximumSpaceForLabels
            , bool //bLimitSpaceForLabels
            )
{
    // note: using xChartDoc itself as XNumberFormatsSupplier would cause
    // a leak from VPolarAxis due to cyclic reference
    uno::Reference<util::XNumberFormatsSupplier> const xNumberFormatsSupplier(
        dynamic_cast<ChartModel&>(*xChartDoc).getNumberFormatsSupplier());

    m_aAxisMap.clear();
    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    sal_Int32 nDimensionIndex = 0;

    //create angle axis (dimension index 0)
    for( nDimensionIndex = 0; nDimensionIndex < nDimensionCount; nDimensionIndex++ )
    {
        sal_Int32 nMaxAxisIndex = m_xCooSysModel->getMaximumAxisIndexByDimension(nDimensionIndex);
        for( sal_Int32 nAxisIndex = 0; nAxisIndex <= nMaxAxisIndex; nAxisIndex++ )
        {
            Reference< XAxis > xAxis( getAxisByDimension(nDimensionIndex,nAxisIndex) );
            if(!xAxis.is() || !AxisHelper::shouldAxisBeDisplayed( xAxis, m_xCooSysModel ))
                continue;
            AxisProperties aAxisProperties(xAxis,getExplicitCategoriesProvider());
            aAxisProperties.init();
            if(aAxisProperties.m_bDisplayLabels)
                aAxisProperties.m_nNumberFormatKey = getNumberFormatKeyForAxis(xAxis, xChartDoc);

            std::shared_ptr< VAxisBase > apVAxis( VPolarAxis::createAxis( aAxisProperties,xNumberFormatsSupplier,nDimensionIndex,nDimensionCount) );
            tFullAxisIndex aFullAxisIndex( nDimensionIndex, nAxisIndex );
            m_aAxisMap[aFullAxisIndex] = apVAxis;

            apVAxis->initAxisLabelProperties(rFontReferenceSize,rMaximumSpaceForLabels);
        }
    }
}

void VPolarCoordinateSystem::initVAxisInList()
{
    if(!m_xLogicTargetForAxes.is() || !m_xFinalTarget.is() || !m_xCooSysModel.is() )
        return;

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    bool bSwapXAndY = getPropertySwapXAndYAxis();

    for (auto const& elem : m_aAxisMap)
    {
        VAxisBase* pVAxis = elem.second.get();
        if( pVAxis )
        {
            sal_Int32 nDimensionIndex = elem.first.first;
            sal_Int32 nAxisIndex = elem.first.second;
            pVAxis->setExplicitScaleAndIncrement( getExplicitScale( nDimensionIndex, nAxisIndex ), getExplicitIncrement(nDimensionIndex, nAxisIndex) );
            pVAxis->initPlotter(m_xLogicTargetForAxes,m_xFinalTarget,m_xShapeFactory
                , createCIDForAxis( nDimensionIndex, nAxisIndex ) );
            VPolarAxis* pVPolarAxis = dynamic_cast< VPolarAxis* >( pVAxis );
            if( pVPolarAxis )
                pVPolarAxis->setIncrements( getExplicitIncrements( nDimensionIndex, nAxisIndex ) );
            if(nDimensionCount==2)
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->setScales( getExplicitScales( nDimensionIndex, nAxisIndex ), bSwapXAndY );
        }
    }
}

void VPolarCoordinateSystem::updateScalesAndIncrementsOnAxes()
{
    if(!m_xLogicTargetForAxes.is() || !m_xFinalTarget.is() || !m_xCooSysModel.is() )
        return;

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    bool bSwapXAndY = getPropertySwapXAndYAxis();

    for (auto const& elem : m_aAxisMap)
    {
        VAxisBase* pVAxis = elem.second.get();
        if( pVAxis )
        {
            sal_Int32 nDimensionIndex = elem.first.first;
            sal_Int32 nAxisIndex = elem.first.second;
            pVAxis->setExplicitScaleAndIncrement( getExplicitScale( nDimensionIndex, nAxisIndex ), getExplicitIncrement(nDimensionIndex, nAxisIndex) );
            VPolarAxis* pVPolarAxis = dynamic_cast< VPolarAxis* >( pVAxis );
            if( pVPolarAxis )
                pVPolarAxis->setIncrements( getExplicitIncrements( nDimensionIndex, nAxisIndex ) );
            if(nDimensionCount==2)
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->setScales( getExplicitScales( nDimensionIndex, nAxisIndex ), bSwapXAndY );
        }
    }
}

void VPolarCoordinateSystem::createGridShapes()
{
    if(!m_xLogicTargetForGrids.is() || !m_xFinalTarget.is() )
        return;

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    bool bSwapXAndY = getPropertySwapXAndYAxis();

    for( sal_Int32 nDimensionIndex=0; nDimensionIndex<3; nDimensionIndex++)
    {
        sal_Int32 nAxisIndex = MAIN_AXIS_INDEX;

        Reference< XAxis > xAxis( AxisHelper::getAxis( nDimensionIndex, nAxisIndex, m_xCooSysModel ) );
        if(!xAxis.is() || !AxisHelper::shouldAxisBeDisplayed( xAxis, m_xCooSysModel ))
            continue;

        VPolarGrid aGrid(nDimensionIndex,nDimensionCount,getGridListFromAxis( xAxis ));
        aGrid.setIncrements( getExplicitIncrements( nDimensionIndex, nAxisIndex ) );
        aGrid.initPlotter(m_xLogicTargetForGrids,m_xFinalTarget,m_xShapeFactory
            , createCIDForGrid( nDimensionIndex, nAxisIndex ) );
        if(nDimensionCount==2)
            aGrid.setTransformationSceneToScreen( m_aMatrixSceneToScreen );
        aGrid.setScales( getExplicitScales( nDimensionIndex, nAxisIndex), bSwapXAndY );
        aGrid.createShapes();
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
