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

#include <BaseGFXHelper.hxx>
#include <DateHelper.hxx>
#include <VCoordinateSystem.hxx>
#include "VCartesianCoordinateSystem.hxx"
#include "VPolarCoordinateSystem.hxx"
#include <BaseCoordinateSystem.hxx>
#include <GridProperties.hxx>
#include <ChartModel.hxx>
#include <ScaleAutomatism.hxx>
#include <ShapeFactory.hxx>
#include <servicenames_coosystems.hxx>
#include <ObjectIdentifier.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <Axis.hxx>
#include "VAxisBase.hxx"
#include <defines.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <rtl/math.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>
#include <limits>
#include <utility>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

std::unique_ptr<VCoordinateSystem> VCoordinateSystem::createCoordinateSystem(
            const rtl::Reference< BaseCoordinateSystem >& xCooSysModel )
{
    if( !xCooSysModel.is() )
        return nullptr;

    OUString aViewServiceName = xCooSysModel->getViewServiceName();

    //@todo: in future the coordinatesystems should be instantiated via service factory
    std::unique_ptr<VCoordinateSystem> pRet;
    if( aViewServiceName == CHART2_COOSYSTEM_CARTESIAN_VIEW_SERVICE_NAME )
        pRet.reset( new VCartesianCoordinateSystem(xCooSysModel) );
    else if( aViewServiceName == CHART2_COOSYSTEM_POLAR_VIEW_SERVICE_NAME )
        pRet.reset( new VPolarCoordinateSystem(xCooSysModel) );
    if(!pRet)
        pRet.reset( new VCoordinateSystem(xCooSysModel) );
    return pRet;
}

VCoordinateSystem::VCoordinateSystem( rtl::Reference< BaseCoordinateSystem > xCooSys )
    : m_xCooSysModel(std::move(xCooSys))
    , m_eLeftWallPos(CuboidPlanePosition_Left)
    , m_eBackWallPos(CuboidPlanePosition_Back)
    , m_eBottomPos(CuboidPlanePosition_Bottom)
    , m_aExplicitScales(3)
    , m_aExplicitIncrements(3)
{
    if( !m_xCooSysModel.is() || m_xCooSysModel->getDimension()<3 )
    {
        m_aExplicitScales[2].Minimum = 1.0;
        m_aExplicitScales[2].Maximum = 2.0;
        m_aExplicitScales[2].Orientation = AxisOrientation_MATHEMATICAL;
    }
}
VCoordinateSystem::~VCoordinateSystem()
{
}

void VCoordinateSystem::initPlottingTargets(  const rtl::Reference< SvxShapeGroupAnyD >& xLogicTarget
       , const rtl::Reference< SvxShapeGroupAnyD >& xFinalTarget
       , rtl::Reference<SvxShapeGroupAnyD>& xLogicTargetForSeriesBehindAxis )
{
    OSL_PRECOND(xLogicTarget.is()&&xFinalTarget.is(),"no proper initialization parameters");
    //is only allowed to be called once

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    //create group shape for grids first thus axes are always painted above grids
    if(nDimensionCount==2)
    {
        //create and add to target
        m_xLogicTargetForGrids = ShapeFactory::createGroup2D( xLogicTarget );
        xLogicTargetForSeriesBehindAxis = ShapeFactory::createGroup2D( xLogicTarget );
        m_xLogicTargetForAxes = ShapeFactory::createGroup2D( xLogicTarget );
    }
    else
    {
        //create and added to target
        m_xLogicTargetForGrids = ShapeFactory::createGroup3D( xLogicTarget );
        xLogicTargetForSeriesBehindAxis = ShapeFactory::createGroup3D( xLogicTarget );
        m_xLogicTargetForAxes = ShapeFactory::createGroup3D( xLogicTarget );
    }
    m_xFinalTarget  = xFinalTarget;
}

void VCoordinateSystem::setParticle( const OUString& rCooSysParticle )
{
    m_aCooSysParticle = rCooSysParticle;
}

void VCoordinateSystem::setTransformationSceneToScreen(
    const drawing::HomogenMatrix& rMatrix )
{
    m_aMatrixSceneToScreen = rMatrix;

    //correct transformation for axis
    for (auto const& elem : m_aAxisMap)
    {
        VAxisBase* pVAxis = elem.second.get();
        if( pVAxis )
        {
            if(pVAxis->getDimensionCount()==2)
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
        }
    }
}

//better performance for big data
uno::Sequence< sal_Int32 > VCoordinateSystem::getCoordinateSystemResolution(
            const awt::Size& rPageSize, const awt::Size& rPageResolution )
{
    uno::Sequence<sal_Int32> aResolution(
        std::max<sal_Int32>(m_xCooSysModel->getDimension(), 2));
    auto aResolutionRange = asNonConstRange(aResolution);
    for( auto& i : aResolutionRange )
        i = 1000;

    ::basegfx::B3DTuple aScale( BaseGFXHelper::GetScaleFromMatrix(
        BaseGFXHelper::HomogenMatrixToB3DHomMatrix(
            m_aMatrixSceneToScreen ) ) );

    double fCoosysWidth = fabs(aScale.getX()*FIXED_SIZE_FOR_3D_CHART_VOLUME);
    double fCoosysHeight = fabs(aScale.getY()*FIXED_SIZE_FOR_3D_CHART_VOLUME);

    double fPageWidth = rPageSize.Width;
    double fPageHeight = rPageSize.Height;

    //factor 2 to avoid rounding problems
    sal_Int32 nXResolution = static_cast<sal_Int32>(2.0*static_cast<double>(rPageResolution.Width)*fCoosysWidth/fPageWidth);
    sal_Int32 nYResolution = static_cast<sal_Int32>(2.0*static_cast<double>(rPageResolution.Height)*fCoosysHeight/fPageHeight);

    if( nXResolution < 10 )
        nXResolution = 10;
    if( nYResolution < 10 )
        nYResolution = 10;

    if( getPropertySwapXAndYAxis() )
        std::swap(nXResolution,nYResolution);

    //2D
    if( aResolution.getLength() == 2 )
    {
        aResolutionRange[0]=nXResolution;
        aResolutionRange[1]=nYResolution;
    }
    else
    {
        //this maybe can be optimized further ...
        sal_Int32 nMaxResolution = std::max( nXResolution, nYResolution );
        nMaxResolution*=2;
        for( auto& i : asNonConstRange(aResolution) )
            i = nMaxResolution;
    }

    return aResolution;
}

rtl::Reference< Axis > VCoordinateSystem::getAxisByDimension( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const
{
    if( m_xCooSysModel.is() )
        return m_xCooSysModel->getAxisByDimension2( nDimensionIndex, nAxisIndex );
    return nullptr;
}

std::vector< rtl::Reference< ::chart::GridProperties > > VCoordinateSystem::getGridListFromAxis( const rtl::Reference< Axis >& xAxis )
{
    std::vector< rtl::Reference< ::chart::GridProperties > > aRet;

    if( xAxis.is() )
    {
        aRet.push_back( xAxis->getGridProperties2() );
        std::vector<rtl::Reference<::chart::GridProperties>> aSubGrids = xAxis->getSubGridProperties2();
        aRet.insert( aRet.end(), aSubGrids.begin(), aSubGrids.end() );
    }

    return aRet;
}

void VCoordinateSystem::impl_adjustDimension( sal_Int32& rDimensionIndex )
{
    rDimensionIndex = std::clamp<sal_Int32>(rDimensionIndex, 0, 2);
}

void VCoordinateSystem::impl_adjustDimensionAndIndex( sal_Int32& rDimensionIndex, sal_Int32& rAxisIndex ) const
{
    impl_adjustDimension( rDimensionIndex );

    if( rAxisIndex < 0 || rAxisIndex > getMaximumAxisIndexByDimension(rDimensionIndex) )
        rAxisIndex = 0;
}

void VCoordinateSystem::setExplicitCategoriesProvider( ExplicitCategoriesProvider* pExplicitCategoriesProvider /*takes ownership*/ )
{
    m_apExplicitCategoriesProvider.reset(pExplicitCategoriesProvider);
}

ExplicitCategoriesProvider* VCoordinateSystem::getExplicitCategoriesProvider()
{
    return m_apExplicitCategoriesProvider.get();
}

std::vector< ExplicitScaleData > VCoordinateSystem::getExplicitScales( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const
{
    std::vector< ExplicitScaleData > aRet(m_aExplicitScales);

    impl_adjustDimensionAndIndex( nDimensionIndex, nAxisIndex );
    aRet[nDimensionIndex]=getExplicitScale( nDimensionIndex, nAxisIndex );

    return aRet;
}

std::vector< ExplicitIncrementData > VCoordinateSystem::getExplicitIncrements( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const
{
    std::vector< ExplicitIncrementData > aRet(m_aExplicitIncrements);

    impl_adjustDimensionAndIndex( nDimensionIndex, nAxisIndex );
    aRet[nDimensionIndex]=getExplicitIncrement( nDimensionIndex, nAxisIndex );

    return aRet;
}

ExplicitScaleData VCoordinateSystem::getExplicitScale( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const
{
    ExplicitScaleData aRet;

    impl_adjustDimensionAndIndex( nDimensionIndex, nAxisIndex );

    if( nAxisIndex == 0)
    {
        aRet = m_aExplicitScales[nDimensionIndex];
    }
    else
    {
        tFullAxisIndex aFullAxisIndex( nDimensionIndex, nAxisIndex );
        tFullExplicitScaleMap::const_iterator aIt = m_aSecondaryExplicitScales.find( aFullAxisIndex );
        if( aIt != m_aSecondaryExplicitScales.end() )
            aRet = aIt->second;
        else
            aRet = m_aExplicitScales[nDimensionIndex];
    }

    return aRet;
}

ExplicitIncrementData VCoordinateSystem::getExplicitIncrement( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const
{
    ExplicitIncrementData aRet;

    impl_adjustDimensionAndIndex( nDimensionIndex, nAxisIndex );

    if( nAxisIndex == 0)
    {
        aRet = m_aExplicitIncrements[nDimensionIndex];
    }
    else
    {
        tFullAxisIndex aFullAxisIndex( nDimensionIndex, nAxisIndex );
        tFullExplicitIncrementMap::const_iterator aIt = m_aSecondaryExplicitIncrements.find( aFullAxisIndex );
        if( aIt != m_aSecondaryExplicitIncrements.end() )
            aRet = aIt->second;
        else
            aRet = m_aExplicitIncrements[nDimensionIndex];
    }

    return aRet;
}

OUString VCoordinateSystem::createCIDForAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    OUString aAxisParticle( ObjectIdentifier::createParticleForAxis( nDimensionIndex, nAxisIndex ) );
    return ObjectIdentifier::createClassifiedIdentifierForParticles( m_aCooSysParticle, aAxisParticle );
}
OUString VCoordinateSystem::createCIDForGrid( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    OUString aGridParticle( ObjectIdentifier::createParticleForGrid( nDimensionIndex, nAxisIndex ) );
    return ObjectIdentifier::createClassifiedIdentifierForParticles( m_aCooSysParticle, aGridParticle );
}

sal_Int32 VCoordinateSystem::getMaximumAxisIndexByDimension( sal_Int32 nDimensionIndex ) const
{
    sal_Int32 nRet = 0;
    for (auto const& elem : m_aSecondaryExplicitScales)
    {
        if(elem.first.first==nDimensionIndex)
        {
            sal_Int32 nLocalIdx = elem.first.second;
            if( nRet < nLocalIdx )
                nRet = nLocalIdx;
        }
    }
    return nRet;
}

void VCoordinateSystem::createVAxisList(
            const rtl::Reference<::chart::ChartModel> & /* xChartDoc */,
            const awt::Size& /* rFontReferenceSize */,
            const awt::Rectangle& /* rMaximumSpaceForLabels */,
            bool /* bLimitSpaceForLabels */,
            std::vector<std::unique_ptr<VSeriesPlotter>>& /*rSeriesPlotterList*/,
            uno::Reference<uno::XComponentContext> const& /*rComponentContext*/)
{
}

void VCoordinateSystem::initVAxisInList()
{
}
void VCoordinateSystem::updateScalesAndIncrementsOnAxes()
{
}

void VCoordinateSystem::prepareAutomaticAxisScaling( ScaleAutomatism& rScaleAutomatism, sal_Int32 nDimIndex, sal_Int32 nAxisIndex )
{
    bool bDateAxisX = (rScaleAutomatism.getScale().AxisType == AxisType::DATE) && (nDimIndex == 0);
    if( bDateAxisX )
    {
        // This is a date X dimension.  Determine proper time resolution.
        sal_Int32 nTimeResolution = css::chart::TimeUnit::MONTH;
        if( !(rScaleAutomatism.getScale().TimeIncrement.TimeResolution >>= nTimeResolution) )
        {
            nTimeResolution = m_aMergedMinMaxSupplier.calculateTimeResolutionOnXAxis();
            rScaleAutomatism.setAutomaticTimeResolution( nTimeResolution );
        }
        m_aMergedMinMaxSupplier.setTimeResolutionOnXAxis( nTimeResolution, rScaleAutomatism.getNullDate() );
    }

    double fMin = std::numeric_limits<double>::infinity();
    double fMax = -std::numeric_limits<double>::infinity();
    if( nDimIndex == 0 )
    {
        // x dimension
        fMin = m_aMergedMinMaxSupplier.getMinimumX();
        fMax = m_aMergedMinMaxSupplier.getMaximumX();
    }
    else if( nDimIndex == 1 )
    {
        // y dimension
        ExplicitScaleData aScale = getExplicitScale( 0, 0 );
        double fMaximum = aScale.Maximum;
        if (!aScale.m_bShiftedCategoryPosition && aScale.AxisType == AxisType::DATE)
        {
            // tdf#146066 Increase maximum date value by one month/year,
            //            because the automatic scaling of the Y axis was incorrect when the last Y value was the highest value.
            Date aMaxDate(aScale.NullDate);
            aMaxDate.AddDays(::rtl::math::approxFloor(fMaximum));
            switch (aScale.TimeResolution)
            {
                case css::chart::TimeUnit::MONTH:
                    aMaxDate = DateHelper::GetDateSomeMonthsAway(aMaxDate, 1);
                    break;
                case css::chart::TimeUnit::YEAR:
                    aMaxDate = DateHelper::GetDateSomeYearsAway(aMaxDate, 1);
                    break;
            }
            fMaximum = aMaxDate - aScale.NullDate;
        }
        fMin = m_aMergedMinMaxSupplier.getMinimumYInRange(aScale.Minimum,aScale.Maximum, nAxisIndex);
        fMax = m_aMergedMinMaxSupplier.getMaximumYInRange(aScale.Minimum, fMaximum, nAxisIndex);
    }
    else if( nDimIndex == 2 )
    {
        // z dimension
        fMin = m_aMergedMinMaxSupplier.getMinimumZ();
        fMax = m_aMergedMinMaxSupplier.getMaximumZ();
    }

    //merge our values with those already contained in rScaleAutomatism
    rScaleAutomatism.expandValueRange( fMin, fMax );

    rScaleAutomatism.setAutoScalingOptions(
        m_aMergedMinMaxSupplier.isExpandBorderToIncrementRhythm( nDimIndex ),
        m_aMergedMinMaxSupplier.isExpandIfValuesCloseToBorder( nDimIndex ),
        m_aMergedMinMaxSupplier.isExpandWideValuesToZero( nDimIndex ),
        m_aMergedMinMaxSupplier.isExpandNarrowValuesTowardZero( nDimIndex ) );

    if (bDateAxisX)
        return;

    VAxisBase* pVAxis = getVAxis(nDimIndex, nAxisIndex);
    if( pVAxis )
        rScaleAutomatism.setMaximumAutoMainIncrementCount( pVAxis->estimateMaximumAutoMainIncrementCount() );
}

VAxisBase* VCoordinateSystem::getVAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    VAxisBase* pRet = nullptr;

    tFullAxisIndex aFullAxisIndex( nDimensionIndex, nAxisIndex );

    tVAxisMap::const_iterator aIt = m_aAxisMap.find( aFullAxisIndex );
    if (aIt != m_aAxisMap.cend())
        pRet = aIt->second.get();

    return pRet;
}

void VCoordinateSystem::setExplicitScaleAndIncrement(
          sal_Int32 nDimensionIndex
        , sal_Int32 nAxisIndex
        , const ExplicitScaleData& rExplicitScale
        , const ExplicitIncrementData& rExplicitIncrement )
{
    impl_adjustDimension( nDimensionIndex );

    if( nAxisIndex==0 )
    {
        m_aExplicitScales[nDimensionIndex]=rExplicitScale;
        m_aExplicitIncrements[nDimensionIndex]=rExplicitIncrement;
    }
    else
    {
        tFullAxisIndex aFullAxisIndex( nDimensionIndex, nAxisIndex );
        m_aSecondaryExplicitScales[aFullAxisIndex] = rExplicitScale;
        m_aSecondaryExplicitIncrements[aFullAxisIndex] = rExplicitIncrement;
    }
}

void VCoordinateSystem::set3DWallPositions( CuboidPlanePosition eLeftWallPos, CuboidPlanePosition eBackWallPos, CuboidPlanePosition eBottomPos )
{
    m_eLeftWallPos = eLeftWallPos;
    m_eBackWallPos = eBackWallPos;
    m_eBottomPos = eBottomPos;
}

void VCoordinateSystem::createMaximumAxesLabels()
{
    for (auto const&[unused, pVAxis] : m_aAxisMap)
    {
        (void)unused;
        if (pVAxis)
        {
            if (pVAxis->getDimensionCount() == 2)
                pVAxis->setTransformationSceneToScreen(m_aMatrixSceneToScreen);
            pVAxis->createMaximumLabels();
        }
    }
}
void VCoordinateSystem::createAxesLabels()
{
    for (auto const&[unused, pVAxis] : m_aAxisMap)
    {
        (void)unused;
        if (pVAxis)
        {
            if (pVAxis->getDimensionCount() == 2)
                pVAxis->setTransformationSceneToScreen(m_aMatrixSceneToScreen);
            pVAxis->createLabels();
        }
    }
}

void VCoordinateSystem::updatePositions()
{
    for (auto const&[unused, pVAxis] : m_aAxisMap)
    {
        (void)unused;
        if (pVAxis)
        {
            if (pVAxis->getDimensionCount() == 2)
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->updatePositions();
        }
    }
}

void VCoordinateSystem::createAxesShapes()
{
    for (auto const&[aFullAxisIndex, pVAxis] : m_aAxisMap)
    {
        if (pVAxis)
        {
            auto const&[nDimensionIndex, nAxisIndex] = aFullAxisIndex;

            if (pVAxis->getDimensionCount() == 2)
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );

            if (nAxisIndex == 0)
            {
                if (nDimensionIndex == 0)
                {
                    if( m_aExplicitScales[1].AxisType!=AxisType::CATEGORY )
                        pVAxis->setExtraLinePositionAtOtherAxis(
                            m_aExplicitScales[1].Origin );
                }
                else if (nDimensionIndex == 1)
                {
                    if( m_aExplicitScales[0].AxisType!=AxisType::CATEGORY )
                        pVAxis->setExtraLinePositionAtOtherAxis(
                            m_aExplicitScales[0].Origin );
                }
            }

            pVAxis->createShapes();
        }
    }
}
void VCoordinateSystem::createGridShapes()
{
}
void VCoordinateSystem::addMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier )
{
    m_aMergedMinMaxSupplier.addMinimumAndMaximumSupplier(pMinimumAndMaximumSupplier);
}

bool VCoordinateSystem::hasMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier )
{
    return m_aMergedMinMaxSupplier.hasMinimumAndMaximumSupplier(pMinimumAndMaximumSupplier);
}

void VCoordinateSystem::clearMinimumAndMaximumSupplierList()
{
    m_aMergedMinMaxSupplier.clearMinimumAndMaximumSupplierList();
}

bool VCoordinateSystem::getPropertySwapXAndYAxis() const
{
    bool bSwapXAndY = false;
    if( m_xCooSysModel.is()) try
    {
        m_xCooSysModel->getPropertyValue( "SwapXAndYAxis" ) >>= bSwapXAndY;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return bSwapXAndY;
}

bool VCoordinateSystem::needSeriesNamesForAxis() const
{
    return ( m_xCooSysModel.is() && m_xCooSysModel->getDimension() == 3 );
}
void VCoordinateSystem::setSeriesNamesForAxis( const Sequence< OUString >& rSeriesNames )
{
    m_aSeriesNamesForZAxis = rSeriesNames;
}

sal_Int32 VCoordinateSystem::getNumberFormatKeyForAxis(
        const rtl::Reference< Axis >& xAxis
        , const rtl::Reference<::chart::ChartModel>& xChartDoc)
{
    return ExplicitValueProvider::getExplicitNumberFormatKeyForAxis(
                xAxis, m_xCooSysModel, xChartDoc);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
