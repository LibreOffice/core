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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "VCartesianCoordinateSystem.hxx"
#include "VCartesianGrid.hxx"
#include "VCartesianAxis.hxx"
#include "macros.hxx"
#include "AxisIndexDefines.hxx"
#include "AxisHelper.hxx"
#include "ChartTypeHelper.hxx"

//for auto_ptr
#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

//.............................................................................

class TextualDataProvider : public ::cppu::WeakImplHelper1<
        ::com::sun::star::chart2::data::XTextualDataSequence
        >
{
public:
    TextualDataProvider( const uno::Sequence< ::rtl::OUString >& rTextSequence )
        : m_aTextSequence( rTextSequence )
    {
    }
    virtual ~TextualDataProvider()
    {
    }

    //XTextualDataSequence
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getTextualData()
        throw ( uno::RuntimeException)
    {
        return m_aTextSequence;
    }

private: //member
    uno::Sequence< ::rtl::OUString > m_aTextSequence;
};

//.............................................................................

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
    bool bSwapXAndY = this->getPropertySwapXAndYAxis();

    for( sal_Int32 nDimensionIndex=0; nDimensionIndex<3; nDimensionIndex++)
    {
        sal_Int32 nAxisIndex = MAIN_AXIS_INDEX;
        Reference< XAxis > xAxis( AxisHelper::getAxis( nDimensionIndex, nAxisIndex, m_xCooSysModel ) );
        if(!xAxis.is() || !AxisHelper::shouldAxisBeDisplayed( xAxis, m_xCooSysModel ))
            continue;

        VCartesianGrid aGrid(nDimensionIndex,nDimensionCount,this->getGridListFromAxis( xAxis ));
        aGrid.setExplicitScaleAndIncrement( this->getExplicitScale(nDimensionIndex,nAxisIndex)
                            , this->getExplicitIncrement(nDimensionIndex,nAxisIndex) );
        aGrid.set3DWallPositions( m_eLeftWallPos, m_eBackWallPos, m_eBottomPos );

        aGrid.initPlotter(m_xLogicTargetForGrids,m_xFinalTarget,m_xShapeFactory
            , this->createCIDForGrid( xAxis,nDimensionIndex,nAxisIndex ) );
        if(2==nDimensionCount)
            aGrid.setTransformationSceneToScreen( m_aMatrixSceneToScreen );
        aGrid.setScales( this->getExplicitScales(nDimensionIndex,nAxisIndex), bSwapXAndY );
        aGrid.createShapes();
    }
}

void VCartesianCoordinateSystem::createVAxisList(
              const uno::Reference< util::XNumberFormatsSupplier > & xNumberFormatsSupplier
            , const awt::Size& rFontReferenceSize
            , const awt::Rectangle& rMaximumSpaceForLabels
            )
{
    m_aAxisMap.clear();

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    bool bSwapXAndY = this->getPropertySwapXAndYAxis();

    if(nDimensionCount<=0)
        return;

    sal_Int32 nDimensionIndex = 0;

    for( nDimensionIndex = 0; nDimensionIndex < nDimensionCount; nDimensionIndex++ )
    {
        sal_Int32 nMaxAxisIndex = m_xCooSysModel->getMaximumAxisIndexByDimension(nDimensionIndex);
        for( sal_Int32 nAxisIndex = 0; nAxisIndex <= nMaxAxisIndex; nAxisIndex++ )
        {
            Reference< XAxis > xAxis = this->getAxisByDimension(nDimensionIndex,nAxisIndex);
            if(!xAxis.is() || !AxisHelper::shouldAxisBeDisplayed( xAxis, m_xCooSysModel ))
                continue;

            AxisProperties aAxisProperties(xAxis,this->getExplicitCategoriesProvider());
            aAxisProperties.m_nDimensionIndex = nDimensionIndex;
            aAxisProperties.m_bSwapXAndY = bSwapXAndY;
            aAxisProperties.m_bIsMainAxis = (nAxisIndex==0);
            Reference< XAxis > xCrossingMainAxis( AxisHelper::getCrossingMainAxis( xAxis, m_xCooSysModel ) );
            if( xCrossingMainAxis.is() )
            {
                ScaleData aCrossingScale( xCrossingMainAxis->getScaleData() );
                aAxisProperties.m_bCrossingAxisHasReverseDirection = (AxisOrientation_REVERSE==aCrossingScale.Orientation);

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
                aAxisProperties.m_nNumberFormatKey = this->getNumberFormatKeyForAxis( xAxis, xNumberFormatsSupplier );

            ::boost::shared_ptr< VAxisBase > apVAxis( new VCartesianAxis(aAxisProperties,xNumberFormatsSupplier,nDimensionIndex,nDimensionCount) );
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
    bool bSwapXAndY = this->getPropertySwapXAndYAxis();

    tVAxisMap::iterator aIt( m_aAxisMap.begin() );
    tVAxisMap::const_iterator aEnd( m_aAxisMap.end() );
    for( ; aIt != aEnd; ++aIt )
    {
        VAxisBase* pVAxis = aIt->second.get();
        if( pVAxis )
        {
            sal_Int32 nDimensionIndex = aIt->first.first;
            sal_Int32 nAxisIndex = aIt->first.second;
            pVAxis->setExplicitScaleAndIncrement( this->getExplicitScale( nDimensionIndex, nAxisIndex ), this->getExplicitIncrement( nDimensionIndex, nAxisIndex ) );
            pVAxis->initPlotter(m_xLogicTargetForAxes,m_xFinalTarget,m_xShapeFactory
                , this->createCIDForAxis( getAxisByDimension( nDimensionIndex, nAxisIndex ), nDimensionIndex, nAxisIndex ) );
            if(2==nDimensionCount)
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->setScales( this->getExplicitScales(nDimensionIndex,nAxisIndex), bSwapXAndY );
        }
    }
}

void VCartesianCoordinateSystem::updateScalesAndIncrementsOnAxes()
{
    if(!m_xLogicTargetForAxes.is() || !m_xFinalTarget.is() || !m_xCooSysModel.is() )
        return;

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    bool bSwapXAndY = this->getPropertySwapXAndYAxis();

    tVAxisMap::iterator aIt( m_aAxisMap.begin() );
    tVAxisMap::const_iterator aEnd( m_aAxisMap.end() );
    for( ; aIt != aEnd; ++aIt )
    {
        VAxisBase* pVAxis = aIt->second.get();
        if( pVAxis )
        {
            sal_Int32 nDimensionIndex = aIt->first.first;
            sal_Int32 nAxisIndex = aIt->first.second;
            pVAxis->setExplicitScaleAndIncrement( this->getExplicitScale( nDimensionIndex, nAxisIndex ), this->getExplicitIncrement( nDimensionIndex, nAxisIndex ) );
            if(2==nDimensionCount)
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->setScales( this->getExplicitScales(nDimensionIndex,nAxisIndex), bSwapXAndY );
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
