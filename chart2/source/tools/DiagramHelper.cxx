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

#include <DiagramHelper.hxx>
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartTypeManager.hxx>
#include <ChartTypeTemplate.hxx>
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <LabeledDataSequence.hxx>
#include <servicenames_charttypes.hxx>
#include <RelativePositionHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <NumberFormatterWrapper.hxx>
#include <unonames.hxx>
#include <BaseCoordinateSystem.hxx>

#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>

#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <unotools/saveopt.hxx>
#include <rtl/math.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <comphelper/sequence.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include <limits>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::chart2::XAnyDescriptionAccess;

namespace chart
{

DiagramHelper::tTemplateWithServiceName
    DiagramHelper::getTemplateForDiagram(
        const rtl::Reference< Diagram > & xDiagram,
        const rtl::Reference< ::chart::ChartTypeManager > & xChartTypeManager )
{
    DiagramHelper::tTemplateWithServiceName aResult;

    if( ! (xChartTypeManager.is() && xDiagram.is()))
        return aResult;

    Sequence< OUString > aServiceNames( xChartTypeManager->getAvailableServiceNames());
    const sal_Int32 nLength = aServiceNames.getLength();

    bool bTemplateFound = false;

    for( sal_Int32 i = 0; ! bTemplateFound && i < nLength; ++i )
    {
        try
        {
            rtl::Reference< ::chart::ChartTypeTemplate > xTempl =
                xChartTypeManager->createTemplate( aServiceNames[ i ] );

            if (xTempl.is() && xTempl->matchesTemplate(xDiagram, true))
            {
                aResult.xChartTypeTemplate = xTempl;
                aResult.sServiceName = aServiceNames[ i ];
                bTemplateFound = true;
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return aResult;
}

void DiagramHelper::setVertical(
    const rtl::Reference< Diagram > & xDiagram,
    bool bVertical /* = true */ )
{
    try
    {
        if (!xDiagram.is())
            return;

        uno::Any aValue;
        aValue <<= bVertical;
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : xDiagram->getBaseCoordinateSystems() )
        {
            bool bChanged = false;
            bool bOldSwap = false;
            if( !(xCooSys->getPropertyValue("SwapXAndYAxis") >>= bOldSwap)
                || bVertical != bOldSwap )
                bChanged = true;

            if( bChanged )
                xCooSys->setPropertyValue("SwapXAndYAxis", aValue);

            const sal_Int32 nDimensionCount = xCooSys->getDimension();
            sal_Int32 nDimIndex = 0;
            for (nDimIndex=0; nDimIndex < nDimensionCount; ++nDimIndex)
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nDimIndex);
                for (sal_Int32 nI = 0; nI <= nMaximumScaleIndex; ++nI)
                {
                    rtl::Reference<Axis> xAxis = xCooSys->getAxisByDimension2(nDimIndex,nI);
                    if (!xAxis.is())
                        continue;

                    //adapt title rotation only when axis swapping has changed
                    if (!bChanged)
                        continue;

                    Reference< beans::XPropertySet > xTitleProps( xAxis->getTitleObject(), uno::UNO_QUERY );
                    if (!xTitleProps.is())
                        continue;

                    double fAngleDegree = 0.0;
                    xTitleProps->getPropertyValue("TextRotation") >>= fAngleDegree;
                    if (fAngleDegree != 0.0 &&
                        !rtl::math::approxEqual(fAngleDegree, 90.0))
                        continue;

                    double fNewAngleDegree = 0.0;
                    if( !bVertical && nDimIndex == 1 )
                        fNewAngleDegree = 90.0;
                    else if( bVertical && nDimIndex == 0 )
                        fNewAngleDegree = 90.0;

                    xTitleProps->setPropertyValue("TextRotation", uno::Any(fNewAngleDegree));
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

bool DiagramHelper::getVertical( const rtl::Reference< Diagram > & xDiagram,
                             bool& rbFound, bool& rbAmbiguous )
{
    bool bValue = false;
    rbFound = false;
    rbAmbiguous = false;

    if (!xDiagram.is())
        return false;

    for (rtl::Reference<BaseCoordinateSystem> const & coords : xDiagram->getBaseCoordinateSystems())
    {
        bool bCurrent = false;
        if (coords->getPropertyValue("SwapXAndYAxis") >>= bCurrent)
        {
            if (!rbFound)
            {
                bValue = bCurrent;
                rbFound = true;
            }
            else if (bCurrent != bValue)
            {
                // ambiguous -> choose always first found
                rbAmbiguous = true;
            }
        }
    }
    return bValue;
}

void DiagramHelper::setStackMode(
    const rtl::Reference< Diagram > & xDiagram,
    StackMode eStackMode
)
{
    try
    {
        bool bValueFound = false;
        bool bIsAmbiguous = false;
        StackMode eOldStackMode = DiagramHelper::getStackMode( xDiagram, bValueFound, bIsAmbiguous );

        if( eStackMode == eOldStackMode && !bIsAmbiguous )
            return;

        StackingDirection eNewDirection = StackingDirection_NO_STACKING;
        if( eStackMode == StackMode::YStacked || eStackMode == StackMode::YStackedPercent )
            eNewDirection = StackingDirection_Y_STACKING;
        else if( eStackMode == StackMode::ZStacked )
            eNewDirection = StackingDirection_Z_STACKING;

        uno::Any aNewDirection( eNewDirection );

        bool bPercent = false;
        if( eStackMode == StackMode::YStackedPercent )
            bPercent = true;

        //iterate through all coordinate systems
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : xDiagram->getBaseCoordinateSystems() )
        {
            //set correct percent stacking
            const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(1);
            for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
            {
                rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2( 1,nI );
                if( xAxis.is())
                {
                    chart2::ScaleData aScaleData = xAxis->getScaleData();
                    if( (aScaleData.AxisType==AxisType::PERCENT) != bPercent )
                    {
                        if( bPercent )
                            aScaleData.AxisType = AxisType::PERCENT;
                        else
                            aScaleData.AxisType = AxisType::REALNUMBER;
                        xAxis->setScaleData( aScaleData );
                    }
                }
            }
            //iterate through all chart types in the current coordinate system
            const std::vector< rtl::Reference< ChartType > > & aChartTypeList( xCooSys->getChartTypes2() );
            if (aChartTypeList.empty())
                continue;

            rtl::Reference< ChartType > xChartType( aChartTypeList[0] );

            //iterate through all series in this chart type
            const uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xChartType->getDataSeries() );
            for( uno::Reference< XDataSeries > const & dataSeries : aSeriesList )
            {
                Reference< beans::XPropertySet > xProp( dataSeries, uno::UNO_QUERY );
                if(xProp.is())
                    xProp->setPropertyValue( "StackingDirection", aNewDirection );
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

StackMode DiagramHelper::getStackMode( const rtl::Reference< Diagram > & xDiagram, bool& rbFound, bool& rbAmbiguous )
{
    rbFound=false;
    rbAmbiguous=false;

    StackMode eGlobalStackMode = StackMode::NONE;

    if( !xDiagram.is() )
        return eGlobalStackMode;

    //iterate through all coordinate systems
    for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : xDiagram->getBaseCoordinateSystems() )
    {
        //iterate through all chart types in the current coordinate system
        std::vector< rtl::Reference< ChartType > > aChartTypeList( xCooSys->getChartTypes2() );
        for( sal_Int32 nT = 0; nT < static_cast<sal_Int32>(aChartTypeList.size()); ++nT )
        {
            rtl::Reference< ChartType > xChartType( aChartTypeList[nT] );

            StackMode eLocalStackMode = DiagramHelper::getStackModeFromChartType(
                xChartType, rbFound, rbAmbiguous, xCooSys );

            if( rbFound && eLocalStackMode != eGlobalStackMode && nT>0 )
            {
                rbAmbiguous = true;
                return eGlobalStackMode;
            }

            eGlobalStackMode = eLocalStackMode;
        }
    }

    return eGlobalStackMode;
}

StackMode DiagramHelper::getStackModeFromChartType(
    const rtl::Reference< ChartType > & xChartType,
    bool& rbFound, bool& rbAmbiguous,
    const rtl::Reference< BaseCoordinateSystem > & xCorrespondingCoordinateSystem )
{
    StackMode eStackMode = StackMode::NONE;
    rbFound = false;
    rbAmbiguous = false;

    try
    {
        Sequence< Reference< chart2::XDataSeries > > aSeries( xChartType->getDataSeries());

        chart2::StackingDirection eCommonDirection = chart2::StackingDirection_NO_STACKING;
        bool bDirectionInitialized = false;

        // first series is irrelevant for stacking, start with second, unless
        // there is only one series
        const sal_Int32 nSeriesCount = aSeries.getLength();
        sal_Int32 i = (nSeriesCount == 1) ? 0: 1;
        for( ; i<nSeriesCount; ++i )
        {
            rbFound = true;
            Reference< beans::XPropertySet > xProp( aSeries[i], uno::UNO_QUERY_THROW );
            chart2::StackingDirection eCurrentDirection = eCommonDirection;
            // property is not MAYBEVOID
            bool bSuccess = ( xProp->getPropertyValue( "StackingDirection" ) >>= eCurrentDirection );
            OSL_ASSERT( bSuccess );
            if( ! bDirectionInitialized )
            {
                eCommonDirection = eCurrentDirection;
                bDirectionInitialized = true;
            }
            else
            {
                if( eCommonDirection != eCurrentDirection )
                {
                    rbAmbiguous = true;
                    break;
                }
            }
        }

        if( rbFound )
        {
            if( eCommonDirection == chart2::StackingDirection_Z_STACKING )
                eStackMode = StackMode::ZStacked;
            else if( eCommonDirection == chart2::StackingDirection_Y_STACKING )
            {
                eStackMode = StackMode::YStacked;

                // percent stacking
                if( xCorrespondingCoordinateSystem.is() )
                {
                    if( 1 < xCorrespondingCoordinateSystem->getDimension() )
                    {
                        sal_Int32 nAxisIndex = 0;
                        if( nSeriesCount )
                            nAxisIndex = DataSeriesHelper::getAttachedAxisIndex(aSeries[0]);

                        rtl::Reference< Axis > xAxis =
                            xCorrespondingCoordinateSystem->getAxisByDimension2( 1,nAxisIndex );
                        if( xAxis.is())
                        {
                            chart2::ScaleData aScaleData = xAxis->getScaleData();
                            if( aScaleData.AxisType==chart2::AxisType::PERCENT )
                                eStackMode = StackMode::YStackedPercent;
                        }
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return eStackMode;
}

sal_Int32 DiagramHelper::getDimension( const rtl::Reference< Diagram > & xDiagram )
{
    // -1: not yet set
    sal_Int32 nResult = -1;
    if (!xDiagram)
        return nResult;

    try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : xDiagram->getBaseCoordinateSystems() )
        {
            if(xCooSys.is())
            {
                nResult = xCooSys->getDimension();
                break;
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return nResult;
}

void DiagramHelper::setDimension(
    const rtl::Reference< Diagram > & xDiagram,
    sal_Int32 nNewDimensionCount )
{
    if( ! xDiagram.is())
        return;

    if( DiagramHelper::getDimension( xDiagram ) == nNewDimensionCount )
        return;

    try
    {
        bool rbFound = false;
        bool rbAmbiguous = true;
        StackMode eStackMode = DiagramHelper::getStackMode( xDiagram, rbFound, rbAmbiguous );
        bool bIsSupportingOnlyDeepStackingFor3D=false;

        //change all coordinate systems:
        auto aCoordSystems = xDiagram->getBaseCoordinateSystems();
        for( rtl::Reference<BaseCoordinateSystem> const & xOldCooSys : aCoordSystems )
        {
            rtl::Reference< BaseCoordinateSystem > xNewCooSys;

            const std::vector< rtl::Reference< ChartType > > aChartTypeList( xOldCooSys->getChartTypes2() );
            for( rtl::Reference< ChartType > const & xChartType : aChartTypeList )
            {
                bIsSupportingOnlyDeepStackingFor3D = ChartTypeHelper::isSupportingOnlyDeepStackingFor3D( xChartType );
                if(!xNewCooSys.is())
                {
                    xNewCooSys = dynamic_cast<BaseCoordinateSystem*>(xChartType->createCoordinateSystem( nNewDimensionCount ).get());
                    assert(xNewCooSys);
                    break;
                }
                //@todo make sure that all following charttypes are also capable of the new dimension
                //otherwise separate them in a different group
                //BM: might be done in replaceCoordinateSystem()
            }

            // replace the old coordinate system at all places where it was used
            DiagramHelper::replaceCoordinateSystem( xDiagram, xOldCooSys, xNewCooSys );
        }

        //correct stack mode if necessary
        if( nNewDimensionCount==3 && eStackMode != StackMode::ZStacked && bIsSupportingOnlyDeepStackingFor3D )
            DiagramHelper::setStackMode( xDiagram, StackMode::ZStacked );
        else if( nNewDimensionCount==2 && eStackMode == StackMode::ZStacked )
            DiagramHelper::setStackMode( xDiagram, StackMode::NONE );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void DiagramHelper::replaceCoordinateSystem(
    const rtl::Reference< Diagram > & xDiagram,
    const rtl::Reference< BaseCoordinateSystem > & xCooSysToReplace,
    const rtl::Reference< BaseCoordinateSystem > & xReplacement )
{
    OSL_ASSERT( xDiagram.is());
    if( ! xDiagram.is())
        return;

    // update the coordinate-system container

    try
    {
        uno::Reference< chart2::data::XLabeledDataSequence > xCategories = DiagramHelper::getCategoriesFromDiagram( xDiagram );

        // move chart types of xCooSysToReplace to xReplacement
        xReplacement->setChartTypes( xCooSysToReplace->getChartTypes());

        xDiagram->removeCoordinateSystem( xCooSysToReplace );
        xDiagram->addCoordinateSystem( xReplacement );

        if( xCategories.is() )
            DiagramHelper::setCategoriesToDiagram( xCategories, xDiagram );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

bool DiagramHelper::isSeriesAttachedToMainAxis(
                          const uno::Reference< chart2::XDataSeries >& xDataSeries )
{
    sal_Int32 nAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
    return (nAxisIndex==0);
}

bool DiagramHelper::attachSeriesToAxis( bool bAttachToMainAxis
                        , const uno::Reference< chart2::XDataSeries >& xDataSeries
                        , const rtl::Reference< Diagram >& xDiagram
                        , const uno::Reference< uno::XComponentContext > & xContext
                        , bool bAdaptAxes )
{
    bool bChanged = false;

    //set property at axis
    Reference< beans::XPropertySet > xProp( xDataSeries, uno::UNO_QUERY_THROW );

    sal_Int32 nNewAxisIndex = bAttachToMainAxis ? 0 : 1;
    sal_Int32 nOldAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
    uno::Reference< chart2::XAxis > xOldAxis( DiagramHelper::getAttachedAxis( xDataSeries, xDiagram ) );

    if( nOldAxisIndex != nNewAxisIndex )
    {
        try
        {
            xProp->setPropertyValue( "AttachedAxisIndex", uno::Any( nNewAxisIndex ) );
            bChanged = true;
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    if( bChanged && xDiagram.is() )
    {
        rtl::Reference< Axis > xAxis = AxisHelper::getAxis( 1, bAttachToMainAxis, xDiagram );
        if(!xAxis.is()) //create an axis if necessary
            xAxis = AxisHelper::createAxis( 1, bAttachToMainAxis, xDiagram, xContext );
        if( bAdaptAxes )
        {
            AxisHelper::makeAxisVisible( xAxis );
            AxisHelper::hideAxisIfNoDataIsAttached( xOldAxis, xDiagram );
        }
    }

    return bChanged;
}

rtl::Reference< Axis > DiagramHelper::getAttachedAxis(
        const uno::Reference< XDataSeries >& xSeries,
        const rtl::Reference< Diagram >& xDiagram )
{
    return AxisHelper::getAxis( 1, DiagramHelper::isSeriesAttachedToMainAxis( xSeries ), xDiagram );
}

rtl::Reference< Axis > DiagramHelper::getAttachedAxis(
        const rtl::Reference< DataSeries >& xSeries,
        const rtl::Reference< Diagram >& xDiagram )
{
    return AxisHelper::getAxis( 1, DiagramHelper::isSeriesAttachedToMainAxis( xSeries ), xDiagram );
}

rtl::Reference< ChartType > DiagramHelper::getChartTypeOfSeries(
                                const rtl::Reference< Diagram >&   xDiagram
                              , const uno::Reference< XDataSeries >&        xGivenDataSeries )
{
    if( !xGivenDataSeries.is() )
        return nullptr;
    if(!xDiagram.is())
        return nullptr;

    //iterate through the model to find the given xSeries
    //the found parent indicates the charttype

    //iterate through all coordinate systems

    for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : xDiagram->getBaseCoordinateSystems() )
    {
        //iterate through all chart types in the current coordinate system
        const std::vector< rtl::Reference< ChartType > > & aChartTypeList( xCooSys->getChartTypes2() );
        for( rtl::Reference< ChartType > const & xChartType : aChartTypeList )
        {
            //iterate through all series in this chart type
            const uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xChartType->getDataSeries() );
            for( uno::Reference< XDataSeries > const & dataSeries : aSeriesList )
            {
                if( xGivenDataSeries==dataSeries )
                    return xChartType;
            }
        }
    }
    return nullptr;
}

std::vector< rtl::Reference< ::chart::DataSeries > >
    DiagramHelper::getDataSeriesFromDiagram(
        const rtl::Reference< Diagram > & xDiagram )
{
    std::vector< rtl::Reference< DataSeries > > aResult;
    if (!xDiagram)
        return aResult;

    try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & coords : xDiagram->getBaseCoordinateSystems() )
        {
            for( rtl::Reference< ChartType> const & chartType : coords->getChartTypes2() )
            {
                const std::vector< rtl::Reference< DataSeries > > aSeriesSeq( chartType->getDataSeries2() );
                aResult.insert( aResult.end(), aSeriesSeq.begin(), aSeriesSeq.end() );
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return aResult;
}

std::vector< std::vector< rtl::Reference< DataSeries > > >
        DiagramHelper::getDataSeriesGroups( const rtl::Reference< Diagram > & xDiagram )
{
    if (!xDiagram)
        return {};

    vector< std::vector< rtl::Reference< DataSeries > > > aResult;

    //iterate through all coordinate systems
    for( rtl::Reference< BaseCoordinateSystem > const & coords : xDiagram->getBaseCoordinateSystems() )
    {
        //iterate through all chart types in the current coordinate system
        for( rtl::Reference< ChartType >  const & chartType : coords->getChartTypes2() )
        {
            aResult.push_back( chartType->getDataSeries2() );
        }
    }
    return aResult;
}

rtl::Reference< ChartType >
    DiagramHelper::getChartTypeByIndex( const rtl::Reference< Diagram >& xDiagram, sal_Int32 nIndex )
{
    if (!xDiagram)
        return nullptr;

    rtl::Reference< ChartType > xChartType;

    //iterate through all coordinate systems
    sal_Int32 nTypesSoFar = 0;
    for( rtl::Reference< BaseCoordinateSystem > const & coords : xDiagram->getBaseCoordinateSystems() )
    {
        const std::vector< rtl::Reference< ChartType > > & aChartTypeList( coords->getChartTypes2() );
        if( nIndex >= 0 && nIndex < static_cast<sal_Int32>(nTypesSoFar + aChartTypeList.size()) )
        {
            xChartType = aChartTypeList[nIndex - nTypesSoFar];
            break;
        }
        nTypesSoFar += aChartTypeList.size();
    }

    return xChartType;
}

namespace
{

std::vector< Reference< XAxis > > lcl_getAxisHoldingCategoriesFromDiagram(
    const rtl::Reference< Diagram > & xDiagram )
{
    std::vector< Reference< XAxis > > aRet;

    // return first x-axis as fall-back
    Reference< XAxis > xFallBack;
    if (xDiagram.is()) try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : xDiagram->getBaseCoordinateSystems() )
        {
            OSL_ASSERT( xCooSys.is());
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                {
                    rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2( nN,nI );
                    OSL_ASSERT( xAxis.is());
                    if( xAxis.is())
                    {
                        ScaleData aScaleData = xAxis->getScaleData();
                        if( aScaleData.Categories.is() || (aScaleData.AxisType == AxisType::CATEGORY) )
                        {
                            aRet.push_back(xAxis);
                        }
                        if( (nN == 0) && !xFallBack.is())
                            xFallBack.set( xAxis );
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2" );
    }

    if( aRet.empty() )
        aRet.push_back(xFallBack);

    return aRet;
}

} // anonymous namespace

bool DiagramHelper::isCategoryDiagram(
            const rtl::Reference< Diagram >& xDiagram )
{
    try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : xDiagram->getBaseCoordinateSystems() )
        {
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                {
                    rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2( nN,nI );
                    OSL_ASSERT( xAxis.is());
                    if( xAxis.is())
                    {
                        ScaleData aScaleData = xAxis->getScaleData();
                        if( aScaleData.AxisType == AxisType::CATEGORY || aScaleData.AxisType == AxisType::DATE )
                            return true;
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return false;
}

void DiagramHelper::setCategoriesToDiagram(
    const uno::Reference< chart2::data::XLabeledDataSequence >& xCategories,
    const rtl::Reference< Diagram >& xDiagram,
    bool bSetAxisType  /* = false */,
    bool bCategoryAxis /* = true */ )
{
    std::vector< Reference< chart2::XAxis > > aCatAxes(
        lcl_getAxisHoldingCategoriesFromDiagram( xDiagram ));

    for (const Reference< chart2::XAxis >& xCatAxis : aCatAxes)
    {
        if( xCatAxis.is())
        {
            ScaleData aScaleData( xCatAxis->getScaleData());
            aScaleData.Categories = xCategories;
            if( bSetAxisType )
            {
                if( bCategoryAxis )
                    aScaleData.AxisType = AxisType::CATEGORY;
                else if( aScaleData.AxisType == AxisType::CATEGORY || aScaleData.AxisType == AxisType::DATE )
                    aScaleData.AxisType = AxisType::REALNUMBER;
            }
            xCatAxis->setScaleData( aScaleData );
        }
    }
}

uno::Reference< chart2::data::XLabeledDataSequence >
    DiagramHelper::getCategoriesFromDiagram(
        const rtl::Reference< Diagram > & xDiagram )
{
    uno::Reference< chart2::data::XLabeledDataSequence > xResult;

    try
    {
        std::vector< Reference< chart2::XAxis > > aCatAxes(
            lcl_getAxisHoldingCategoriesFromDiagram( xDiagram ));
        //search for first categories
        if (!aCatAxes.empty())
        {
            Reference< chart2::XAxis > xCatAxis(aCatAxes[0]);
            if( xCatAxis.is())
            {
                ScaleData aScaleData( xCatAxis->getScaleData());
                if( aScaleData.Categories.is() )
                {
                    xResult = aScaleData.Categories;
                    uno::Reference<beans::XPropertySet> xProp(xResult->getValues(), uno::UNO_QUERY );
                    if( xProp.is() )
                    {
                        try
                        {
                            xProp->setPropertyValue( "Role", uno::Any( OUString("categories") ) );
                        }
                        catch( const uno::Exception & )
                        {
                            DBG_UNHANDLED_EXCEPTION("chart2");
                        }
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

static void lcl_generateAutomaticCategoriesFromChartType(
            Sequence< OUString >& rRet,
            const rtl::Reference< ChartType >& xChartType )
{
    if(!xChartType.is())
        return;
    OUString aMainSeq( xChartType->getRoleOfSequenceForSeriesLabel() );

    const Sequence< Reference< XDataSeries > > aSeriesSeq( xChartType->getDataSeries() );
    for( Reference< XDataSeries > const & dataSeries : aSeriesSeq )
    {
        Reference< data::XDataSource > xDataSource( dataSeries, uno::UNO_QUERY );
        if( !xDataSource.is() )
            continue;
        Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
            ::chart::DataSeriesHelper::getDataSequenceByRole( xDataSource, aMainSeq ));
        if( !xLabeledSeq.is() )
            continue;
        Reference< chart2::data::XDataSequence > xValueSeq( xLabeledSeq->getValues() );
        if( !xValueSeq.is() )
            continue;
        rRet = xValueSeq->generateLabel( chart2::data::LabelOrigin_LONG_SIDE );
        if( rRet.hasElements() )
            return;
    }
}

Sequence< OUString > DiagramHelper::generateAutomaticCategoriesFromCooSys( const rtl::Reference< BaseCoordinateSystem > & xCooSys )
{
    Sequence< OUString > aRet;

    if( xCooSys.is() )
    {
        const std::vector< rtl::Reference< ChartType > > & aChartTypes( xCooSys->getChartTypes2() );
        for( rtl::Reference< ChartType > const & chartType : aChartTypes )
        {
            lcl_generateAutomaticCategoriesFromChartType( aRet, chartType );
            if( aRet.hasElements() )
                return aRet;
        }
    }
    return aRet;
}

Sequence< OUString > DiagramHelper::getExplicitSimpleCategories(
            ChartModel& rModel )
{
    rtl::Reference< BaseCoordinateSystem > xCooSys( ChartModelHelper::getFirstCoordinateSystem( &rModel ) );
    ExplicitCategoriesProvider aExplicitCategoriesProvider( xCooSys, rModel );
    return aExplicitCategoriesProvider.getSimpleCategories();
}

namespace
{
void lcl_switchToDateCategories( const rtl::Reference< ChartModel >& xChartDoc, const Reference< XAxis >& xAxis )
{
    if( !xAxis.is() )
        return;
    if( !xChartDoc.is() )
        return;

    ScaleData aScale( xAxis->getScaleData() );
    if( xChartDoc->hasInternalDataProvider() )
    {
        //remove all content the is not of type double and remove multiple level
        Reference< XAnyDescriptionAccess > xDataAccess( xChartDoc->getDataProvider(), uno::UNO_QUERY );
        if( xDataAccess.is() )
        {
            Sequence< Sequence< Any > > aAnyCategories( xDataAccess->getAnyRowDescriptions() );
            auto aAnyCategoriesRange = asNonConstRange(aAnyCategories);
            double fTest = 0.0;
            sal_Int32 nN = aAnyCategories.getLength();
            for( ; nN--; )
            {
                Sequence< Any >& rCat = aAnyCategoriesRange[nN];
                if( rCat.getLength() > 1 )
                    rCat.realloc(1);
                if( rCat.getLength() == 1 )
                {
                    Any& rAny = rCat.getArray()[0];
                    if( !(rAny>>=fTest) )
                    {
                        rAny <<= std::numeric_limits<double>::quiet_NaN();
                    }
                }
            }
            xDataAccess->setAnyRowDescriptions( aAnyCategories );
        }
        //check the numberformat at the axis
        Reference< beans::XPropertySet > xAxisProps( xAxis, uno::UNO_QUERY );
        if( xAxisProps.is() )
        {
            sal_Int32 nNumberFormat = -1;
            xAxisProps->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;

            Reference< util::XNumberFormats > xNumberFormats( xChartDoc->getNumberFormats() );
            if( xNumberFormats.is() )
            {
                Reference< beans::XPropertySet > xKeyProps;
                try
                {
                    xKeyProps = xNumberFormats->getByKey( nNumberFormat );
                }
                catch( const uno::Exception & )
                {
                    DBG_UNHANDLED_EXCEPTION("chart2");
                }
                sal_Int32 nType = util::NumberFormat::UNDEFINED;
                if( xKeyProps.is() )
                    xKeyProps->getPropertyValue( "Type" ) >>= nType;
                if( !( nType & util::NumberFormat::DATE ) )
                {
                    //set a date format to the axis
                    const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
                    Sequence<sal_Int32> aKeySeq = xNumberFormats->queryKeys( util::NumberFormat::DATE,  rLocaleDataWrapper.getLanguageTag().getLocale(), true/*bCreate*/ );
                    if( aKeySeq.hasElements() )
                    {
                        xAxisProps->setPropertyValue(CHART_UNONAME_NUMFMT, uno::Any(aKeySeq[0]));
                    }
                }
            }
        }
    }
    if( aScale.AxisType != chart2::AxisType::DATE )
        AxisHelper::removeExplicitScaling( aScale );
    aScale.AxisType = chart2::AxisType::DATE;
    xAxis->setScaleData( aScale );
}

void lcl_switchToTextCategories( const rtl::Reference< ChartModel >& xChartDoc, const Reference< XAxis >& xAxis )
{
    if( !xAxis.is() )
        return;
    if( !xChartDoc.is() )
        return;
    ScaleData aScale( xAxis->getScaleData() );
    if( aScale.AxisType != chart2::AxisType::CATEGORY )
        AxisHelper::removeExplicitScaling( aScale );
    //todo migrate dates to text?
    aScale.AxisType = chart2::AxisType::CATEGORY;
    aScale.AutoDateAxis = false;
    xAxis->setScaleData( aScale );
}

}

void DiagramHelper::switchToDateCategories( const rtl::Reference<::chart::ChartModel>& xChartDoc )
{
    if(xChartDoc.is())
    {
        ControllerLockGuardUNO aCtrlLockGuard( xChartDoc );

        rtl::Reference< BaseCoordinateSystem > xCooSys = ChartModelHelper::getFirstCoordinateSystem( xChartDoc );
        if( xCooSys.is() )
        {
            rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2(0,0);
            lcl_switchToDateCategories( xChartDoc, xAxis );
        }
    }
}

void DiagramHelper::switchToTextCategories( const rtl::Reference<::chart::ChartModel>& xChartDoc )
{
    if(xChartDoc.is())
    {
        ControllerLockGuardUNO aCtrlLockGuard( xChartDoc );

        rtl::Reference< BaseCoordinateSystem > xCooSys = ChartModelHelper::getFirstCoordinateSystem( xChartDoc );
        if( xCooSys.is() )
        {
            rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2(0,0);
            lcl_switchToTextCategories( xChartDoc, xAxis );
        }
    }
}

bool DiagramHelper::isSupportingDateAxis( const rtl::Reference< Diagram >& xDiagram )
{
    return ::chart::ChartTypeHelper::isSupportingDateAxis(
            DiagramHelper::getChartTypeByIndex( xDiagram, 0 ), 0 );
}

bool DiagramHelper::isDateNumberFormat( sal_Int32 nNumberFormat, const Reference< util::XNumberFormats >& xNumberFormats )
{
    bool bIsDate = false;
    if( !xNumberFormats.is() )
        return bIsDate;

    Reference< beans::XPropertySet > xKeyProps = xNumberFormats->getByKey( nNumberFormat );
    if( xKeyProps.is() )
    {
        sal_Int32 nType = util::NumberFormat::UNDEFINED;
        xKeyProps->getPropertyValue( "Type" ) >>= nType;
        bIsDate = nType & util::NumberFormat::DATE;
    }
    return bIsDate;
}

sal_Int32 DiagramHelper::getDateNumberFormat( const Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
{
    sal_Int32 nRet=-1;

    //try to get a date format with full year display
    const LanguageTag& rLanguageTag = Application::GetSettings().GetLanguageTag();
    NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
    SvNumberFormatter* pNumFormatter = aNumberFormatterWrapper.getSvNumberFormatter();
    if( pNumFormatter )
    {
        nRet = pNumFormatter->GetFormatIndex( NF_DATE_SYS_DDMMYYYY, rLanguageTag.getLanguageType() );
    }
    else
    {
        Reference< util::XNumberFormats > xNumberFormats( xNumberFormatsSupplier->getNumberFormats() );
        if( xNumberFormats.is() )
        {
            Sequence<sal_Int32> aKeySeq = xNumberFormats->queryKeys( util::NumberFormat::DATE,
                    rLanguageTag.getLocale(), true/*bCreate */);
            if( aKeySeq.hasElements() )
            {
                nRet = aKeySeq[0];
            }
        }
    }
    return nRet;
}

sal_Int32 DiagramHelper::getDateTimeInputNumberFormat( const Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier, double fNumber )
{
    sal_Int32 nRet = 0;

    // Get the most detailed date/time format according to fNumber.
    NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
    SvNumberFormatter* pNumFormatter = aNumberFormatterWrapper.getSvNumberFormatter();
    if (!pNumFormatter)
        SAL_WARN("chart2", "DiagramHelper::getDateTimeInputNumberFormat - no SvNumberFormatter");
    else
    {
        SvNumFormatType nType;
        // Obtain best matching date, time or datetime format.
        nRet = pNumFormatter->GuessDateTimeFormat( nType, fNumber, LANGUAGE_SYSTEM);
        // Obtain the corresponding edit format.
        nRet = pNumFormatter->GetEditFormat( fNumber, nRet, nType, LANGUAGE_SYSTEM, nullptr);
    }
    return nRet;
}

sal_Int32 DiagramHelper::getPercentNumberFormat( const Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
{
    sal_Int32 nRet=-1;
    const LanguageTag& rLanguageTag = Application::GetSettings().GetLanguageTag();
    NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
    SvNumberFormatter* pNumFormatter = aNumberFormatterWrapper.getSvNumberFormatter();
    if( pNumFormatter )
    {
        nRet = pNumFormatter->GetFormatIndex( NF_PERCENT_INT, rLanguageTag.getLanguageType() );
    }
    else
    {
        Reference< util::XNumberFormats > xNumberFormats( xNumberFormatsSupplier->getNumberFormats() );
        if( xNumberFormats.is() )
        {
            Sequence<sal_Int32> aKeySeq = xNumberFormats->queryKeys( util::NumberFormat::PERCENT,
                    rLanguageTag.getLocale(), true/*bCreate*/ );
            if( aKeySeq.hasElements() )
            {
                // This *assumes* the sequence is sorted as in
                // NfIndexTableOffset and the first format is the integer 0%
                // format by chance... which usually is the case, but... anyway,
                // we usually also have a number formatter so don't reach here.
                nRet = aKeySeq[0];
            }
        }
    }
    return nRet;
}

std::vector< rtl::Reference< ChartType > >
    DiagramHelper::getChartTypesFromDiagram(
        const rtl::Reference< Diagram > & xDiagram )
{
    if(!xDiagram)
        return {};

    std::vector< rtl::Reference< ChartType > > aResult;
    try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & coords : xDiagram->getBaseCoordinateSystems() )
        {
            const std::vector< rtl::Reference< ChartType > > & aChartTypeSeq( coords->getChartTypes2());
            aResult.insert( aResult.end(), aChartTypeSeq.begin(), aChartTypeSeq.end() );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return aResult;
}

bool DiagramHelper::areChartTypesCompatible( const rtl::Reference< ChartType >& xFirstType,
                const rtl::Reference< ChartType >& xSecondType )
{
    if( !xFirstType.is() || !xSecondType.is() )
        return false;

    auto aFirstRoles( comphelper::sequenceToContainer<std::vector< OUString >>( xFirstType->getSupportedMandatoryRoles() ) );
    auto aSecondRoles( comphelper::sequenceToContainer<std::vector< OUString >>( xSecondType->getSupportedMandatoryRoles() ) );
    std::sort( aFirstRoles.begin(), aFirstRoles.end() );
    std::sort( aSecondRoles.begin(), aSecondRoles.end() );
    return ( aFirstRoles == aSecondRoles );
}

namespace
{
     /**
     * This method implements the logic of checking if a series can be moved
     * forward/backward. Depending on the "bDoMove" parameter the series will
     * be moved (bDoMove = true) or the function just will test if the
     * series can be moved without doing the move (bDoMove = false).
     *
     * @param xDiagram
     *  Reference to the diagram that contains the series.
     *
     * @param xGivenDataSeries
     *  Reference to the series that should moved or tested for moving.
     *
     * @param bForward
     *  Direction in which the series should be moved or tested for moving.
     *
     * @param bDoMove
     *  Should this function really move the series (true) or just test if it is
     *  possible (false).
     *
     *
     * @returns
     *  in case of bDoMove == true
     *      - True : if the move was done
     *      - False : the move failed
     *  in case of bDoMove == false
     *      - True : the series can be moved
     *      - False : the series can not be moved
     *
     */

bool lcl_moveSeriesOrCheckIfMoveIsAllowed(
    const rtl::Reference< Diagram >& xDiagram,
    const Reference< XDataSeries >& xGivenDataSeries,
    bool bForward,
    bool bDoMove )
{
    bool bMovedOrMoveAllowed = false;

    try
    {
        if( xGivenDataSeries.is() && xDiagram.is() )
        {
            //find position of series.
            bool bFound = false;
            const std::vector< rtl::Reference< BaseCoordinateSystem > > & aCooSysList( xDiagram->getBaseCoordinateSystems() );

            for( sal_Int32 nCS = 0; !bFound && nCS < static_cast<sal_Int32>(aCooSysList.size()); ++nCS )
            {
                const rtl::Reference< BaseCoordinateSystem > & xCooSys( aCooSysList[nCS] );

                //iterate through all chart types in the current coordinate system
                std::vector< rtl::Reference< ChartType > > aChartTypeList( xCooSys->getChartTypes2() );
                rtl::Reference< ChartType > xFormerChartType;

                for( sal_Int32 nT = 0; !bFound && nT < static_cast<sal_Int32>(aChartTypeList.size()); ++nT )
                {
                    rtl::Reference< ChartType > xCurrentChartType( aChartTypeList[nT] );

                    //iterate through all series in this chart type

                    uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xCurrentChartType->getDataSeries() );
                    auto aSeriesListRange = asNonConstRange(aSeriesList);

                    for( sal_Int32 nS = 0; !bFound && nS < aSeriesList.getLength(); ++nS )
                    {

                        // We found the series we are interested in!
                        if( xGivenDataSeries==aSeriesList[nS] )
                        {
                            sal_Int32 nOldSeriesIndex = nS;
                            bFound = true;

                            try
                            {
                                sal_Int32 nNewSeriesIndex = nS;

                                // tdf#34517 Bringing forward means increasing, backwards means decreasing series position
                                if( !bForward )
                                    nNewSeriesIndex--;
                                else
                                    nNewSeriesIndex++;

                                if( nNewSeriesIndex >= 0 && nNewSeriesIndex < aSeriesList.getLength() )
                                {
                                    //move series in the same charttype
                                    bMovedOrMoveAllowed = true;
                                    if( bDoMove )
                                    {
                                        aSeriesListRange[ nOldSeriesIndex ] = aSeriesList[ nNewSeriesIndex ];
                                        aSeriesListRange[ nNewSeriesIndex ] = xGivenDataSeries;
                                        xCurrentChartType->setDataSeries( aSeriesList );
                                    }
                                }
                                else if( nNewSeriesIndex<0 )
                                {
                                    //exchange series with former charttype
                                    if( xFormerChartType.is() && DiagramHelper::areChartTypesCompatible( xFormerChartType, xCurrentChartType ) )
                                    {
                                        bMovedOrMoveAllowed = true;
                                        if( bDoMove )
                                        {
                                            uno::Sequence< uno::Reference< XDataSeries > > aOtherSeriesList( xFormerChartType->getDataSeries() );
                                            sal_Int32 nOtherSeriesIndex = aOtherSeriesList.getLength()-1;
                                            if( nOtherSeriesIndex >= 0 && nOtherSeriesIndex < aOtherSeriesList.getLength() )
                                            {
                                                uno::Reference< XDataSeries > xExchangeSeries( aOtherSeriesList[nOtherSeriesIndex] );
                                                aOtherSeriesList.getArray()[nOtherSeriesIndex] = xGivenDataSeries;
                                                xFormerChartType->setDataSeries(aOtherSeriesList);

                                                aSeriesListRange[nOldSeriesIndex]=xExchangeSeries;
                                                xCurrentChartType->setDataSeries(aSeriesList);
                                            }
                                        }
                                    }
                                }
                                else if( nT+1 < static_cast<sal_Int32>(aChartTypeList.size()) )
                                {
                                    //exchange series with next charttype
                                    rtl::Reference< ChartType > xOtherChartType( aChartTypeList[nT+1] );
                                    if( xOtherChartType.is() && DiagramHelper::areChartTypesCompatible( xOtherChartType, xCurrentChartType ) )
                                    {
                                        bMovedOrMoveAllowed = true;
                                        if( bDoMove )
                                        {
                                            uno::Sequence< uno::Reference< XDataSeries > > aOtherSeriesList( xOtherChartType->getDataSeries() );
                                            if( aOtherSeriesList.hasElements() )
                                            {
                                                uno::Reference< XDataSeries > xExchangeSeries( aOtherSeriesList[0] );
                                                aOtherSeriesList.getArray()[0] = xGivenDataSeries;
                                                xOtherChartType->setDataSeries(aOtherSeriesList);

                                                aSeriesListRange[nOldSeriesIndex]=xExchangeSeries;
                                                xCurrentChartType->setDataSeries(aSeriesList);
                                            }
                                        }
                                    }
                                }
                            }
                            catch( const util::CloseVetoException& )
                            {
                            }
                            catch( const uno::RuntimeException& )
                            {
                            }
                        }
                    }
                    xFormerChartType = xCurrentChartType;
                }
            }
        }
    }
    catch( const util::CloseVetoException& )
    {
    }
    catch( const uno::RuntimeException& )
    {
    }
    return bMovedOrMoveAllowed;
}
} // anonymous namespace

bool DiagramHelper::isSeriesMoveable(
    const rtl::Reference< Diagram >& xDiagram,
    const Reference< XDataSeries >& xGivenDataSeries,
    bool bForward )
{
    const bool bDoMove = false;

    bool bIsMoveable = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        xDiagram, xGivenDataSeries, bForward, bDoMove );

    return bIsMoveable;
}

bool DiagramHelper::moveSeries( const rtl::Reference< Diagram >& xDiagram, const Reference< XDataSeries >& xGivenDataSeries, bool bForward )
{
    const bool bDoMove = true;

    bool bMoved = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        xDiagram, xGivenDataSeries, bForward, bDoMove );

    return bMoved;
}

bool DiagramHelper::isSupportingFloorAndWall( const rtl::Reference< Diagram >& xDiagram )
{
    //pies and donuts currently do not support this because of wrong files from older versions
    //todo: allow this in future again, if fileversion is available for OLE objects (metastream)
    //thus the wrong bottom can be removed on import

    const std::vector< rtl::Reference< ChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
    for( rtl::Reference< ChartType > const & xType : aTypes )
    {
        if( xType.is() && xType->getChartType().match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return false;
        if( xType.is() && xType->getChartType().match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return false;
        if( xType.is() && xType->getChartType().match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
            return false;
    }
    return true;
}

bool DiagramHelper::isPieOrDonutChart( const rtl::Reference< Diagram >& xDiagram )
{
    rtl::Reference< ChartType > xChartType( DiagramHelper::getChartTypeByIndex(
        xDiagram, 0 ) );

    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType == CHART2_SERVICE_NAME_CHARTTYPE_PIE )
            return true;
    }
    return false;
}

sal_Int32 DiagramHelper::getGeometry3D(
    const rtl::Reference< Diagram > & xDiagram,
    bool& rbFound, bool& rbAmbiguous )
{
    sal_Int32 nCommonGeom( DataPointGeometry3D::CUBOID );
    rbFound = false;
    rbAmbiguous = false;

    std::vector< rtl::Reference< DataSeries > > aSeriesVec =
        DiagramHelper::getDataSeriesFromDiagram( xDiagram );

    if( aSeriesVec.empty())
        rbAmbiguous = true;

    for (auto const& series : aSeriesVec)
    {
        try
        {
            sal_Int32 nGeom = 0;
            if( series->getPropertyValue( "Geometry3D") >>= nGeom )
            {
                if( ! rbFound )
                {
                    // first series
                    nCommonGeom = nGeom;
                    rbFound = true;
                }
                // further series: compare for uniqueness
                else if( nCommonGeom != nGeom )
                {
                    rbAmbiguous = true;
                    break;
                }
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return nCommonGeom;
}

void DiagramHelper::setGeometry3D(
    const rtl::Reference< Diagram > & xDiagram,
    sal_Int32 nNewGeometry )
{
    std::vector< rtl::Reference< DataSeries > > aSeriesVec =
        DiagramHelper::getDataSeriesFromDiagram( xDiagram );

    for (auto const& series : aSeriesVec)
    {
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(
            series, "Geometry3D", uno::Any( nNewGeometry ));
    }
}

sal_Int32 DiagramHelper::getCorrectedMissingValueTreatment(
            const rtl::Reference< Diagram > & xDiagram,
            const rtl::Reference< ChartType >& xChartType )
{
    sal_Int32 nResult = css::chart::MissingValueTreatment::LEAVE_GAP;
    const uno::Sequence < sal_Int32 > aAvailableMissingValueTreatments(
                ChartTypeHelper::getSupportedMissingValueTreatments( xChartType ) );

    if( xDiagram.is() && (xDiagram->getPropertyValue( "MissingValueTreatment" ) >>= nResult) )
    {
        //ensure that the set value is supported by this charttype
        for( sal_Int32 n : aAvailableMissingValueTreatments )
            if( n == nResult )
                return nResult; //ok
    }

    //otherwise use the first supported one
    if( aAvailableMissingValueTreatments.hasElements() )
    {
        nResult = aAvailableMissingValueTreatments[0];
        return nResult;
    }

    return nResult;
}

DiagramPositioningMode DiagramHelper::getDiagramPositioningMode( const rtl::Reference<
                Diagram > & xDiagram )
{
    DiagramPositioningMode eMode = DiagramPositioningMode_AUTO;
    if( xDiagram.is() )
    {
        RelativePosition aRelPos;
        RelativeSize aRelSize;
        if( (xDiagram->getPropertyValue("RelativePosition") >>= aRelPos ) &&
            (xDiagram->getPropertyValue("RelativeSize") >>= aRelSize ) )
        {
            bool bPosSizeExcludeAxes=false;
            xDiagram->getPropertyValue("PosSizeExcludeAxes") >>= bPosSizeExcludeAxes;
            if( bPosSizeExcludeAxes )
                eMode = DiagramPositioningMode_EXCLUDING;
            else
                eMode = DiagramPositioningMode_INCLUDING;
        }
    }
    return eMode;
}

static void lcl_ensureRange0to1( double& rValue )
{
    if(rValue<0.0)
        rValue=0.0;
    if(rValue>1.0)
        rValue=1.0;
}

bool DiagramHelper::setDiagramPositioning( const rtl::Reference<::chart::ChartModel>& xChartModel,
        const awt::Rectangle& rPosRect /*100th mm*/ )
{
    ControllerLockGuardUNO aCtrlLockGuard( xChartModel );

    bool bChanged = false;
    awt::Size aPageSize( ChartModelHelper::getPageSize(xChartModel) );
    rtl::Reference< Diagram > xDiagram = ChartModelHelper::findDiagram( xChartModel );
    if( !xDiagram.is() )
        return bChanged;

    RelativePosition aOldPos;
    RelativeSize aOldSize;
    xDiagram->getPropertyValue("RelativePosition" ) >>= aOldPos;
    xDiagram->getPropertyValue("RelativeSize" ) >>= aOldSize;

    RelativePosition aNewPos;
    aNewPos.Anchor = drawing::Alignment_TOP_LEFT;
    aNewPos.Primary = double(rPosRect.X)/double(aPageSize.Width);
    aNewPos.Secondary = double(rPosRect.Y)/double(aPageSize.Height);

    chart2::RelativeSize aNewSize;
    aNewSize.Primary = double(rPosRect.Width)/double(aPageSize.Width);
    aNewSize.Secondary = double(rPosRect.Height)/double(aPageSize.Height);

    lcl_ensureRange0to1( aNewPos.Primary );
    lcl_ensureRange0to1( aNewPos.Secondary );
    lcl_ensureRange0to1( aNewSize.Primary );
    lcl_ensureRange0to1( aNewSize.Secondary );
    if( (aNewPos.Primary + aNewSize.Primary) > 1.0 )
        aNewPos.Primary = 1.0 - aNewSize.Primary;
    if( (aNewPos.Secondary + aNewSize.Secondary) > 1.0 )
        aNewPos.Secondary = 1.0 - aNewSize.Secondary;

    xDiagram->setPropertyValue( "RelativePosition", uno::Any(aNewPos) );
    xDiagram->setPropertyValue( "RelativeSize", uno::Any(aNewSize) );

    bChanged = (aOldPos.Anchor!=aNewPos.Anchor) ||
        (aOldPos.Primary!=aNewPos.Primary) ||
        (aOldPos.Secondary!=aNewPos.Secondary) ||
        (aOldSize.Primary!=aNewSize.Primary) ||
        (aOldSize.Secondary!=aNewSize.Secondary);
    return bChanged;
}

awt::Rectangle DiagramHelper::getDiagramRectangleFromModel( const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    awt::Rectangle aRet(-1,-1,-1,-1);

    rtl::Reference< Diagram > xDiagram = ChartModelHelper::findDiagram( xChartModel );
    if( !xDiagram.is() )
        return aRet;

    awt::Size aPageSize( ChartModelHelper::getPageSize(xChartModel) );

    RelativePosition aRelPos;
    RelativeSize aRelSize;
    xDiagram->getPropertyValue("RelativePosition" ) >>= aRelPos;
    xDiagram->getPropertyValue("RelativeSize" ) >>= aRelSize;

    awt::Size aAbsSize(
        static_cast< sal_Int32 >( aRelSize.Primary * aPageSize.Width ),
        static_cast< sal_Int32 >( aRelSize.Secondary * aPageSize.Height ));

    awt::Point aAbsPos(
        static_cast< sal_Int32 >( aRelPos.Primary * aPageSize.Width ),
        static_cast< sal_Int32 >( aRelPos.Secondary * aPageSize.Height ));

    awt::Point aAbsPosLeftTop = RelativePositionHelper::getUpperLeftCornerOfAnchoredObject( aAbsPos, aAbsSize, aRelPos.Anchor );

    aRet = awt::Rectangle(aAbsPosLeftTop.X, aAbsPosLeftTop.Y, aAbsSize.Width, aAbsSize.Height );

    return aRet;
}

bool DiagramHelper::switchDiagramPositioningToExcludingPositioning(
    ChartModel& rModel, bool bResetModifiedState, bool bConvertAlsoFromAutoPositioning )
{
    //return true if something was changed
    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(GetODFSaneDefaultVersion());
    if (SvtSaveOptions::ODFSVER_012 < nCurrentODFVersion)
    {
        uno::Reference< css::chart::XDiagramPositioning > xDiagramPositioning( rModel.getFirstDiagram(), uno::UNO_QUERY );
        if( xDiagramPositioning.is() && ( bConvertAlsoFromAutoPositioning || !xDiagramPositioning->isAutomaticDiagramPositioning() )
                && !xDiagramPositioning->isExcludingDiagramPositioning() )
        {
            ControllerLockGuard aCtrlLockGuard( rModel );
            bool bModelWasModified = rModel.isModified();
            xDiagramPositioning->setDiagramPositionExcludingAxes( xDiagramPositioning->calculateDiagramPositionExcludingAxes() );
            if(bResetModifiedState && !bModelWasModified )
                rModel.setModified(false);
            return true;
        }
    }
    return false;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
