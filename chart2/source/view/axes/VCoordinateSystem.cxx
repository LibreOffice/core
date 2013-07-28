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

#include "VCoordinateSystem.hxx"
#include "VCartesianCoordinateSystem.hxx"
#include "VPolarCoordinateSystem.hxx"
#include "ScaleAutomatism.hxx"
#include "VSeriesPlotter.hxx"
#include "ShapeFactory.hxx"
#include "servicenames_coosystems.hxx"
#include "macros.hxx"
#include "AxisIndexDefines.hxx"
#include "ObjectIdentifier.hxx"
#include "ExplicitCategoriesProvider.hxx"
#include "AxisHelper.hxx"
#include "ContainerHelper.hxx"
#include "VAxisBase.hxx"
#include "ViewDefines.hxx"
#include "DataSeriesHelper.hxx"
#include "defines.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>

#include <rtl/math.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

VCoordinateSystem* VCoordinateSystem::createCoordinateSystem(
            const Reference< XCoordinateSystem >& xCooSysModel )
{
    if( !xCooSysModel.is() )
        return 0;

    OUString aViewServiceName = xCooSysModel->getViewServiceName();

    //@todo: in future the coordinatesystems should be instanciated via service factory
    VCoordinateSystem* pRet=NULL;
    if( aViewServiceName.equals( CHART2_COOSYSTEM_CARTESIAN_VIEW_SERVICE_NAME ) )
        pRet = new VCartesianCoordinateSystem(xCooSysModel);
    else if( aViewServiceName.equals( CHART2_COOSYSTEM_POLAR_VIEW_SERVICE_NAME ) )
        pRet = new VPolarCoordinateSystem(xCooSysModel);
    if(!pRet)
        pRet = new VCoordinateSystem(xCooSysModel);
    return pRet;
}

VCoordinateSystem::VCoordinateSystem( const Reference< XCoordinateSystem >& xCooSys )
    : m_xCooSysModel(xCooSys)
    , m_xLogicTargetForGrids(0)
    , m_xLogicTargetForAxes(0)
    , m_xFinalTarget(0)
    , m_xShapeFactory(0)
    , m_aMatrixSceneToScreen()
    , m_eLeftWallPos(CuboidPlanePosition_Left)
    , m_eBackWallPos(CuboidPlanePosition_Back)
    , m_eBottomPos(CuboidPlanePosition_Bottom)
    , m_aMergedMinimumAndMaximumSupplier()
    , m_aExplicitScales(3)
    , m_aExplicitIncrements(3)
    , m_apExplicitCategoriesProvider(NULL)
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

void VCoordinateSystem::initPlottingTargets(  const Reference< drawing::XShapes >& xLogicTarget
       , const Reference< drawing::XShapes >& xFinalTarget
       , const Reference< lang::XMultiServiceFactory >& xShapeFactory
       , Reference< drawing::XShapes >& xLogicTargetForSeriesBehindAxis )
            throw (uno::RuntimeException)
{
    OSL_PRECOND(xLogicTarget.is()&&xFinalTarget.is()&&xShapeFactory.is(),"no proper initialization parameters");
    //is only allowed to be called once

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    //create group shape for grids first thus axes are always painted above grids
    ShapeFactory aShapeFactory(xShapeFactory);
    if(nDimensionCount==2)
    {
        //create and add to target
        m_xLogicTargetForGrids = aShapeFactory.createGroup2D( xLogicTarget );
        xLogicTargetForSeriesBehindAxis = aShapeFactory.createGroup2D( xLogicTarget );
        m_xLogicTargetForAxes = aShapeFactory.createGroup2D( xLogicTarget );
    }
    else
    {
        //create and added to target
        m_xLogicTargetForGrids = aShapeFactory.createGroup3D( xLogicTarget );
        xLogicTargetForSeriesBehindAxis = aShapeFactory.createGroup3D( xLogicTarget );
        m_xLogicTargetForAxes = aShapeFactory.createGroup3D( xLogicTarget );
    }
    m_xFinalTarget  = xFinalTarget;
    m_xShapeFactory = xShapeFactory;
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
    tVAxisMap::iterator aIt( m_aAxisMap.begin() );
    tVAxisMap::const_iterator aEnd( m_aAxisMap.end() );
    for( ; aIt != aEnd; ++aIt )
    {
        VAxisBase* pVAxis = aIt->second.get();
        if( pVAxis )
        {
            if(2==pVAxis->getDimensionCount())
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
        }
    }
}

drawing::HomogenMatrix VCoordinateSystem::getTransformationSceneToScreen()
{
    return m_aMatrixSceneToScreen;
}

//better performance for big data
uno::Sequence< sal_Int32 > VCoordinateSystem::getCoordinateSystemResolution(
            const awt::Size& rPageSize, const awt::Size& rPageResolution )
{
    uno::Sequence< sal_Int32 > aResolution(2);

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    if(nDimensionCount>2)
        aResolution.realloc(nDimensionCount);
    sal_Int32 nN = 0;
    for( nN = 0 ;nN<aResolution.getLength(); nN++ )
        aResolution[nN]=1000;

    ::basegfx::B3DTuple aScale( BaseGFXHelper::GetScaleFromMatrix(
        BaseGFXHelper::HomogenMatrixToB3DHomMatrix(
            m_aMatrixSceneToScreen ) ) );

    double fCoosysWidth = static_cast< double >( fabs(aScale.getX()*FIXED_SIZE_FOR_3D_CHART_VOLUME));
    double fCoosysHeight = static_cast< double >( fabs(aScale.getY()*FIXED_SIZE_FOR_3D_CHART_VOLUME));

    double fPageWidth = rPageSize.Width;
    double fPageHeight = rPageSize.Height;

    //factor 2 to avoid rounding problems
    sal_Int32 nXResolution = static_cast<sal_Int32>(2.0*static_cast<double>(rPageResolution.Width)*fCoosysWidth/fPageWidth);
    sal_Int32 nYResolution = static_cast<sal_Int32>(2.0*static_cast<double>(rPageResolution.Height)*fCoosysHeight/fPageHeight);

    if( nXResolution < 10 )
        nXResolution = 10;
    if( nYResolution < 10 )
        nYResolution = 10;

    if( this->getPropertySwapXAndYAxis() )
        std::swap(nXResolution,nYResolution);

    //2D
    if( 2 == aResolution.getLength() )
    {
        aResolution[0]=nXResolution;
        aResolution[1]=nYResolution;
    }
    else
    {
        //this maybe can be optimized further ...
        sal_Int32 nMaxResolution = std::max( nXResolution, nYResolution );
        nMaxResolution*=2;
        for( nN = 0 ;nN<aResolution.getLength(); nN++ )
            aResolution[nN]=nMaxResolution;
    }

    return aResolution;
}

Reference< XCoordinateSystem > VCoordinateSystem::getModel() const
{
    return m_xCooSysModel;
}

Reference< XAxis > VCoordinateSystem::getAxisByDimension( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const
{
    if( m_xCooSysModel.is() )
        return m_xCooSysModel->getAxisByDimension( nDimensionIndex, nAxisIndex );
    return 0;
}

Sequence< Reference< beans::XPropertySet > > VCoordinateSystem::getGridListFromAxis( const Reference< XAxis >& xAxis )
{
    std::vector< Reference< beans::XPropertySet > > aRet;

    if( xAxis.is() )
    {
        aRet.push_back( xAxis->getGridProperties() );
        std::vector< Reference< beans::XPropertySet > > aSubGrids( ContainerHelper::SequenceToVector( xAxis->getSubGridProperties() ) );
        aRet.insert( aRet.end(), aSubGrids.begin(), aSubGrids.end() );
    }

    return ContainerHelper::ContainerToSequence( aRet );
}

void VCoordinateSystem::impl_adjustDimension( sal_Int32& rDimensionIndex ) const
{
    if( rDimensionIndex<0 )
        rDimensionIndex=0;
    if( rDimensionIndex>2 )
        rDimensionIndex=2;
}

void VCoordinateSystem::impl_adjustDimensionAndIndex( sal_Int32& rDimensionIndex, sal_Int32& rAxisIndex ) const
{
    impl_adjustDimension( rDimensionIndex );

    if( rAxisIndex < 0 || rAxisIndex > this->getMaximumAxisIndexByDimension(rDimensionIndex) )
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
    aRet[nDimensionIndex]=this->getExplicitScale( nDimensionIndex, nAxisIndex );

    return aRet;
}

std::vector< ExplicitIncrementData > VCoordinateSystem::getExplicitIncrements( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const
{
    std::vector< ExplicitIncrementData > aRet(m_aExplicitIncrements);

    impl_adjustDimensionAndIndex( nDimensionIndex, nAxisIndex );
    aRet[nDimensionIndex]=this->getExplicitIncrement( nDimensionIndex, nAxisIndex );

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

OUString VCoordinateSystem::createCIDForAxis( const Reference< chart2::XAxis >& /* xAxis */, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    OUString aAxisParticle( ObjectIdentifier::createParticleForAxis( nDimensionIndex, nAxisIndex ) );
    return ObjectIdentifier::createClassifiedIdentifierForParticles( m_aCooSysParticle, aAxisParticle );
}
OUString VCoordinateSystem::createCIDForGrid( const Reference< chart2::XAxis >& /* xAxis */, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    OUString aGridParticle( ObjectIdentifier::createParticleForGrid( nDimensionIndex, nAxisIndex ) );
    return ObjectIdentifier::createClassifiedIdentifierForParticles( m_aCooSysParticle, aGridParticle );
}

sal_Int32 VCoordinateSystem::getMaximumAxisIndexByDimension( sal_Int32 nDimensionIndex ) const
{
    sal_Int32 nRet = 0;
    tFullExplicitScaleMap::const_iterator aIt = m_aSecondaryExplicitScales.begin();
    tFullExplicitScaleMap::const_iterator aEnd = m_aSecondaryExplicitScales.end();
    for(; aIt!=aEnd; ++aIt)
    {
        if(aIt->first.first==nDimensionIndex)
        {
            sal_Int32 nLocalIdx = aIt->first.second;
            if( nRet < nLocalIdx )
                nRet = nLocalIdx;
        }
    }
    return nRet;
}

void VCoordinateSystem::createVAxisList(
              const uno::Reference< util::XNumberFormatsSupplier > & /* xNumberFormatsSupplier */
            , const awt::Size& /* rFontReferenceSize */
            , const awt::Rectangle& /* rMaximumSpaceForLabels */
            )
{
}

void VCoordinateSystem::initVAxisInList()
{
}
void VCoordinateSystem::updateScalesAndIncrementsOnAxes()
{
}

void VCoordinateSystem::prepareScaleAutomatismForDimensionAndIndex( ScaleAutomatism& rScaleAutomatism, sal_Int32 nDimIndex, sal_Int32 nAxisIndex )
{
    if( rScaleAutomatism.getScale().AxisType==AxisType::DATE && nDimIndex==0 )
    {
        sal_Int32 nTimeResolution = ::com::sun::star::chart::TimeUnit::MONTH;
        if( !(rScaleAutomatism.getScale().TimeIncrement.TimeResolution >>= nTimeResolution) )
        {
            nTimeResolution = m_aMergedMinimumAndMaximumSupplier.calculateTimeResolutionOnXAxis();
            rScaleAutomatism.setAutomaticTimeResolution( nTimeResolution );
        }
        m_aMergedMinimumAndMaximumSupplier.setTimeResolutionOnXAxis( nTimeResolution, rScaleAutomatism.getNullDate() );
    }

    double fMin = 0.0;
    double fMax = 0.0;
    ::rtl::math::setInf(&fMin, false);
    ::rtl::math::setInf(&fMax, true);
    if( 0 == nDimIndex )
    {
        fMin = m_aMergedMinimumAndMaximumSupplier.getMinimumX();
        fMax = m_aMergedMinimumAndMaximumSupplier.getMaximumX();
    }
    else if( 1 == nDimIndex )
    {
        ExplicitScaleData aScale = getExplicitScale( 0, 0 );
        fMin = m_aMergedMinimumAndMaximumSupplier.getMinimumYInRange(aScale.Minimum,aScale.Maximum, nAxisIndex);
        fMax = m_aMergedMinimumAndMaximumSupplier.getMaximumYInRange(aScale.Minimum,aScale.Maximum, nAxisIndex);
    }
    else if( 2 == nDimIndex )
    {
        fMin = m_aMergedMinimumAndMaximumSupplier.getMinimumZ();
        fMax = m_aMergedMinimumAndMaximumSupplier.getMaximumZ();
    }

    //merge our values with those already contained in rScaleAutomatism
    rScaleAutomatism.expandValueRange( fMin, fMax );

    rScaleAutomatism.setAutoScalingOptions(
        m_aMergedMinimumAndMaximumSupplier.isExpandBorderToIncrementRhythm( nDimIndex ),
        m_aMergedMinimumAndMaximumSupplier.isExpandIfValuesCloseToBorder( nDimIndex ),
        m_aMergedMinimumAndMaximumSupplier.isExpandWideValuesToZero( nDimIndex ),
        m_aMergedMinimumAndMaximumSupplier.isExpandNarrowValuesTowardZero( nDimIndex ) );

    VAxisBase* pVAxis( this->getVAxis( nDimIndex, nAxisIndex ) );
    if( pVAxis )
        rScaleAutomatism.setMaximumAutoMainIncrementCount( pVAxis->estimateMaximumAutoMainIncrementCount() );
}

VAxisBase* VCoordinateSystem::getVAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    VAxisBase* pRet = 0;

    tFullAxisIndex aFullAxisIndex( nDimensionIndex, nAxisIndex );

    tVAxisMap::const_iterator aIt = m_aAxisMap.find( aFullAxisIndex );
    if( aIt != m_aAxisMap.end() )
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
    tVAxisMap::iterator aIt( m_aAxisMap.begin() );
    tVAxisMap::const_iterator aEnd( m_aAxisMap.end() );
    for( ; aIt != aEnd; ++aIt )
    {
        VAxisBase* pVAxis = aIt->second.get();
        if( pVAxis )
        {
            if(2==pVAxis->getDimensionCount())
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->createMaximumLabels();
        }
    }
}
void VCoordinateSystem::createAxesLabels()
{
    tVAxisMap::iterator aIt( m_aAxisMap.begin() );
    tVAxisMap::const_iterator aEnd( m_aAxisMap.end() );
    for( ; aIt != aEnd; ++aIt )
    {
        VAxisBase* pVAxis = aIt->second.get();
        if( pVAxis )
        {
            if(2==pVAxis->getDimensionCount())
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->createLabels();
        }
    }
}

void VCoordinateSystem::updatePositions()
{
    tVAxisMap::iterator aIt( m_aAxisMap.begin() );
    tVAxisMap::const_iterator aEnd( m_aAxisMap.end() );
    for( ; aIt != aEnd; ++aIt )
    {
        VAxisBase* pVAxis = aIt->second.get();
        if( pVAxis )
        {
            if(2==pVAxis->getDimensionCount())
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            pVAxis->updatePositions();
        }
    }
}

void VCoordinateSystem::createAxesShapes()
{
    tVAxisMap::iterator aIt( m_aAxisMap.begin() );
    tVAxisMap::const_iterator aEnd( m_aAxisMap.end() );
    for( ; aIt != aEnd; ++aIt )
    {
        VAxisBase* pVAxis = aIt->second.get();
        if( pVAxis )
        {
            if(2==pVAxis->getDimensionCount())
                pVAxis->setTransformationSceneToScreen( m_aMatrixSceneToScreen );

            tFullAxisIndex aFullAxisIndex = aIt->first;
            if( aFullAxisIndex.second == 0 )
            {
                if( aFullAxisIndex.first == 0 )
                {
                    if( AxisType::CATEGORY!=m_aExplicitScales[1].AxisType )
                        pVAxis->setExrtaLinePositionAtOtherAxis(
                            m_aExplicitScales[1].Origin );
                }
                else if( aFullAxisIndex.first == 1 )
                {
                    if( AxisType::CATEGORY!=m_aExplicitScales[0].AxisType )
                        pVAxis->setExrtaLinePositionAtOtherAxis(
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
    m_aMergedMinimumAndMaximumSupplier.addMinimumAndMaximumSupplier(pMinimumAndMaximumSupplier);
}

bool VCoordinateSystem::hasMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier )
{
    return m_aMergedMinimumAndMaximumSupplier.hasMinimumAndMaximumSupplier(pMinimumAndMaximumSupplier);
}

void VCoordinateSystem::clearMinimumAndMaximumSupplierList()
{
    m_aMergedMinimumAndMaximumSupplier.clearMinimumAndMaximumSupplierList();
}

bool VCoordinateSystem::getPropertySwapXAndYAxis() const
{
    Reference<beans::XPropertySet> xProp(m_xCooSysModel, uno::UNO_QUERY );
    sal_Bool bSwapXAndY = false;
    if( xProp.is()) try
    {
        xProp->getPropertyValue( "SwapXAndYAxis" ) >>= bSwapXAndY;
    }
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
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
        const Reference< chart2::XAxis >& xAxis
        , const Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
{
    return ExplicitValueProvider::getExplicitNumberFormatKeyForAxis(
                xAxis, m_xCooSysModel, xNumberFormatsSupplier );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
