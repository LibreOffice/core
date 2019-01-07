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

#include "VCartesianCoordinateSystem.hxx"
#include "VCartesianGrid.hxx"
#include "VCartesianAxis.hxx"
#include <AxisIndexDefines.hxx>
#include <AxisHelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <ChartModel.hxx>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

class TextualDataProvider : public ::cppu::WeakImplHelper<
        css::chart2::data::XTextualDataSequence
        >
{
public:
    explicit TextualDataProvider( const uno::Sequence< OUString >& rTextSequence )
        : m_aTextSequence( rTextSequence )
    {
    }

    //XTextualDataSequence
    virtual uno::Sequence< OUString > SAL_CALL getTextualData() override
    {
        return m_aTextSequence;
    }

private: //member
    uno::Sequence< OUString > m_aTextSequence;
};

VCartesianCoordinateSystem::VCartesianCoordinateSystem( const Reference< XCoordinateSystem >& xCooSys )
    : VCoordinateSystem(xCooSys)
{
}

VCartesianCoordinateSystem::~VCartesianCoordinateSystem()
{
}

void VCartesianCoordinateSystem::createGridShapes()
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

        VCartesianGrid aGrid(nDimensionIndex,nDimensionCount, getGridListFromAxis( xAxis ));
        aGrid.setExplicitScaleAndIncrement( getExplicitScale(nDimensionIndex,nAxisIndex)
                            , getExplicitIncrement(nDimensionIndex,nAxisIndex) );
        aGrid.set3DWallPositions( m_eLeftWallPos, m_eBackWallPos, m_eBottomPos );

        aGrid.initPlotter(m_xLogicTargetForGrids,m_xFinalTarget,m_xShapeFactory
            , createCIDForGrid( nDimensionIndex,nAxisIndex ) );
        if(nDimensionCount==2)
            aGrid.setTransformationSceneToScreen( m_aMatrixSceneToScreen );
        aGrid.setScales( getExplicitScales(nDimensionIndex,nAxisIndex), bSwapXAndY );
        aGrid.createShapes();
    }
}

void VCartesianCoordinateSystem::createVAxisList(
              const uno::Reference<chart2::XChartDocument> & xChartDoc
            , const awt::Size& rFontReferenceSize
            , const awt::Rectangle& rMaximumSpaceForLabels
            , bool bLimitSpaceForLabels
            )
{
    // note: using xChartDoc itself as XNumberFormatsSupplier would cause
    // a leak from VCartesianAxis due to cyclic reference
    uno::Reference<util::XNumberFormatsSupplier> const xNumberFormatsSupplier(
        dynamic_cast<ChartModel&>(*xChartDoc).getNumberFormatsSupplier());

    m_aAxisMap.clear();

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    bool bSwapXAndY = getPropertySwapXAndYAxis();

    if(nDimensionCount<=0)
        return;

    sal_Int32 nDimensionIndex = 0;

    // dimension index -> x, y or z axis.
    for( nDimensionIndex = 0; nDimensionIndex < nDimensionCount; nDimensionIndex++ )
    {
        // axis index -> primary or secondary axis.
        sal_Int32 nMaxAxisIndex = m_xCooSysModel->getMaximumAxisIndexByDimension(nDimensionIndex);
        for( sal_Int32 nAxisIndex = 0; nAxisIndex <= nMaxAxisIndex; nAxisIndex++ )
        {
            Reference< XAxis > xAxis = getAxisByDimension(nDimensionIndex,nAxisIndex);
            if(!xAxis.is() || !AxisHelper::shouldAxisBeDisplayed( xAxis, m_xCooSysModel ))
                continue;

            AxisProperties aAxisProperties(xAxis,getExplicitCategoriesProvider());
            aAxisProperties.m_nDimensionIndex = nDimensionIndex;
            aAxisProperties.m_bSwapXAndY = bSwapXAndY;
            aAxisProperties.m_bIsMainAxis = (nAxisIndex==0);
            aAxisProperties.m_bLimitSpaceForLabels = bLimitSpaceForLabels;
            Reference< XAxis > xCrossingMainAxis( AxisHelper::getCrossingMainAxis( xAxis, m_xCooSysModel ) );
            if( xCrossingMainAxis.is() )
            {
                ScaleData aCrossingScale( xCrossingMainAxis->getScaleData() );
                aAxisProperties.m_bCrossingAxisHasReverseDirection = (aCrossingScale.Orientation==AxisOrientation_REVERSE);

                if( aCrossingScale.AxisType == AxisType::CATEGORY )
                    aAxisProperties.m_bCrossingAxisIsCategoryAxes = true;
            }

            if( nDimensionIndex == 2 )
            {
                aAxisProperties.m_xAxisTextProvider = new TextualDataProvider( m_aSeriesNamesForZAxis );

                //for the z axis copy the positioning properties from the x axis (or from the y axis for swapped coordinate systems)
                Reference< XAxis > xSisterAxis( AxisHelper::getCrossingMainAxis( xCrossingMainAxis, m_xCooSysModel ) );
                aAxisProperties.initAxisPositioning( Reference< beans::XPropertySet >( xSisterAxis, uno::UNO_QUERY) );
            }
            aAxisProperties.init(true);
            if(aAxisProperties.m_bDisplayLabels)
                aAxisProperties.m_nNumberFormatKey = getNumberFormatKeyForAxis(xAxis, xChartDoc);

            std::shared_ptr< VAxisBase > apVAxis( new VCartesianAxis(aAxisProperties,xNumberFormatsSupplier,nDimensionIndex,nDimensionCount) );
            tFullAxisIndex aFullAxisIndex( nDimensionIndex, nAxisIndex );
            m_aAxisMap[aFullAxisIndex] = apVAxis;
            apVAxis->set3DWallPositions( m_eLeftWallPos, m_eBackWallPos, m_eBottomPos );

            apVAxis->initAxisLabelProperties(rFontReferenceSize,rMaximumSpaceForLabels);
        }
    }
}

void VCartesianCoordinateSystem::initVAxisInList()
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
            pVAxis->setExplicitScaleAndIncrement( getExplicitScale( nDimensionIndex, nAxisIndex ), getExplicitIncrement( nDimensionIndex, nAxisIndex ) );
            pVAxis->initPlotter(m_xLogicTargetForAxes,m_xFinalTarget,m_xShapeFactory
                , createCIDForAxis( nDimensionIndex, nAxisIndex ) );
            if(nDimensionCount==2)
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->setScales( getExplicitScales(nDimensionIndex,nAxisIndex), bSwapXAndY );
        }
    }
}

void VCartesianCoordinateSystem::updateScalesAndIncrementsOnAxes()
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
            pVAxis->setExplicitScaleAndIncrement( getExplicitScale( nDimensionIndex, nAxisIndex ), getExplicitIncrement( nDimensionIndex, nAxisIndex ) );
            if(nDimensionCount==2)
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->setScales( getExplicitScales(nDimensionIndex,nAxisIndex), bSwapXAndY );
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
