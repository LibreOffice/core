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
#include <DataSeriesHelper.hxx>
#include <AxisHelper.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <servicenames_charttypes.hxx>
#include <RelativePositionHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <NumberFormatterWrapper.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
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
#include <svl/zforlist.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <comphelper/sequence.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

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
        const Reference< XDiagram > & xDiagram,
        const Reference< lang::XMultiServiceFactory > & xChartTypeManager )
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
            Reference< XChartTypeTemplate > xTempl(
                xChartTypeManager->createInstance( aServiceNames[ i ] ), uno::UNO_QUERY_THROW );

            if (xTempl.is() && xTempl->matchesTemplate(xDiagram, true))
            {
                aResult.first = xTempl;
                aResult.second = aServiceNames[ i ];
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
    const Reference< XDiagram > & xDiagram,
    bool bVertical /* = true */ )
{
    try
    {
        Reference< XCoordinateSystemContainer > xCnt( xDiagram, uno::UNO_QUERY );
        if (!xCnt.is())
            return;

        const Sequence< Reference<XCoordinateSystem> > aCooSys = xCnt->getCoordinateSystems();
        uno::Any aValue;
        aValue <<= bVertical;
        for( uno::Reference< XCoordinateSystem > const & xCooSys : aCooSys )
        {
            Reference< beans::XPropertySet > xProp( xCooSys, uno::UNO_QUERY );
            bool bChanged = false;
            if (xProp.is())
            {
                bool bOldSwap = false;
                if( !(xProp->getPropertyValue("SwapXAndYAxis") >>= bOldSwap)
                    || bVertical != bOldSwap )
                    bChanged = true;

                if( bChanged )
                    xProp->setPropertyValue("SwapXAndYAxis", aValue);
            }

            if (!xCooSys.is())
                continue;

            const sal_Int32 nDimensionCount = xCooSys->getDimension();
            sal_Int32 nDimIndex = 0;
            for (nDimIndex=0; nDimIndex < nDimensionCount; ++nDimIndex)
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nDimIndex);
                for (sal_Int32 nI = 0; nI <= nMaximumScaleIndex; ++nI)
                {
                    Reference<chart2::XAxis> xAxis = xCooSys->getAxisByDimension(nDimIndex,nI);
                    if (!xAxis.is())
                        continue;

                    //adapt title rotation only when axis swapping has changed
                    if (!bChanged)
                        continue;

                    Reference< XTitled > xTitled( xAxis, uno::UNO_QUERY );
                    if (!xTitled.is())
                        continue;

                    Reference< beans::XPropertySet > xTitleProps( xTitled->getTitleObject(), uno::UNO_QUERY );
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

bool DiagramHelper::getVertical( const uno::Reference< chart2::XDiagram > & xDiagram,
                             bool& rbFound, bool& rbAmbiguous )
{
    bool bValue = false;
    rbFound = false;
    rbAmbiguous = false;

    Reference< XCoordinateSystemContainer > xCnt( xDiagram, uno::UNO_QUERY );
    if (!xCnt.is())
        return false;

    const Sequence< Reference<XCoordinateSystem> > aCooSys = xCnt->getCoordinateSystems();

    for (Reference<XCoordinateSystem> const & coords : aCooSys)
    {
        Reference<beans::XPropertySet> xProp(coords, uno::UNO_QUERY);
        if (!xProp.is())
            continue;

        bool bCurrent = false;
        if (xProp->getPropertyValue("SwapXAndYAxis") >>= bCurrent)
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
    const Reference< XDiagram > & xDiagram,
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
        uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
        if( !xCooSysContainer.is() )
            return;
        const uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( uno::Reference< XCoordinateSystem > const & xCooSys : aCooSysList )
        {
            //set correct percent stacking
            const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(1);
            for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
            {
                Reference< chart2::XAxis > xAxis( xCooSys->getAxisByDimension( 1,nI ));
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
            uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
            if( !xChartTypeContainer.is() )
                continue;
            uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
            if (!aChartTypeList.hasElements())
                continue;

            uno::Reference< XChartType > xChartType( aChartTypeList[0] );

            //iterate through all series in this chart type
            uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
            OSL_ASSERT( xDataSeriesContainer.is());
            if( !xDataSeriesContainer.is() )
                continue;

            const uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
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

StackMode DiagramHelper::getStackMode( const Reference< XDiagram > & xDiagram, bool& rbFound, bool& rbAmbiguous )
{
    rbFound=false;
    rbAmbiguous=false;

    StackMode eGlobalStackMode = StackMode::NONE;

    //iterate through all coordinate systems
    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is() )
        return eGlobalStackMode;
    const uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( uno::Reference< XCoordinateSystem > const & xCooSys : aCooSysList )
    {
        //iterate through all chart types in the current coordinate system
        uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        if( !xChartTypeContainer.is() )
            continue;
        uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference< XChartType > xChartType( aChartTypeList[nT] );

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
    const Reference< XChartType > & xChartType,
    bool& rbFound, bool& rbAmbiguous,
    const Reference< XCoordinateSystem > & xCorrespondingCoordinateSystem )
{
    StackMode eStackMode = StackMode::NONE;
    rbFound = false;
    rbAmbiguous = false;

    try
    {
        Reference< XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XDataSeries > > aSeries( xDSCnt->getDataSeries());

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

                        Reference< chart2::XAxis > xAxis(
                            xCorrespondingCoordinateSystem->getAxisByDimension( 1,nAxisIndex ));
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

sal_Int32 DiagramHelper::getDimension( const Reference< XDiagram > & xDiagram )
{
    // -1: not yet set
    sal_Int32 nResult = -1;

    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY );
        if( xCooSysCnt.is() )
        {
            const Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
                xCooSysCnt->getCoordinateSystems());

            for( Reference< XCoordinateSystem > const & xCooSys : aCooSysSeq )
            {
                if(xCooSys.is())
                {
                    nResult = xCooSys->getDimension();
                    break;
                }
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
    const Reference< XDiagram > & xDiagram,
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
        Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY_THROW );
        const Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( Reference<XCoordinateSystem> const & xOldCooSys : aCooSysList )
        {
            Reference< XCoordinateSystem > xNewCooSys;

            Reference< XChartTypeContainer > xChartTypeContainer( xOldCooSys, uno::UNO_QUERY );
            if( !xChartTypeContainer.is() )
                continue;

            const Sequence< Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
            for( Reference< XChartType > const & xChartType : aChartTypeList )
            {
                bIsSupportingOnlyDeepStackingFor3D = ChartTypeHelper::isSupportingOnlyDeepStackingFor3D( xChartType );
                if(!xNewCooSys.is())
                {
                    xNewCooSys = xChartType->createCoordinateSystem( nNewDimensionCount );
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
    const Reference< XDiagram > & xDiagram,
    const Reference< XCoordinateSystem > & xCooSysToReplace,
    const Reference< XCoordinateSystem > & xReplacement )
{
    OSL_ASSERT( xDiagram.is());
    if( ! xDiagram.is())
        return;

    // update the coordinate-system container
    Reference< XCoordinateSystemContainer > xCont( xDiagram, uno::UNO_QUERY );
    if( !xCont.is())
        return;

    try
    {
        Reference< chart2::data::XLabeledDataSequence > xCategories = DiagramHelper::getCategoriesFromDiagram( xDiagram );

        // move chart types of xCooSysToReplace to xReplacement
        Reference< XChartTypeContainer > xCTCntCooSys( xCooSysToReplace, uno::UNO_QUERY_THROW );
        Reference< XChartTypeContainer > xCTCntReplacement( xReplacement, uno::UNO_QUERY_THROW );
        xCTCntReplacement->setChartTypes( xCTCntCooSys->getChartTypes());

        xCont->removeCoordinateSystem( xCooSysToReplace );
        xCont->addCoordinateSystem( xReplacement );

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
                        , const uno::Reference< chart2::XDiagram >& xDiagram
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
        uno::Reference< XAxis > xAxis( AxisHelper::getAxis( 1, bAttachToMainAxis, xDiagram ) );
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

uno::Reference< XAxis > DiagramHelper::getAttachedAxis(
        const uno::Reference< XDataSeries >& xSeries,
        const uno::Reference< XDiagram >& xDiagram )
{
    return AxisHelper::getAxis( 1, DiagramHelper::isSeriesAttachedToMainAxis( xSeries ), xDiagram );
}

uno::Reference< XChartType > DiagramHelper::getChartTypeOfSeries(
                                const uno::Reference< chart2::XDiagram >&   xDiagram
                              , const uno::Reference< XDataSeries >&        xGivenDataSeries )
{
    if( !xGivenDataSeries.is() )
        return nullptr;
    if(!xDiagram.is())
        return nullptr;

    //iterate through the model to find the given xSeries
    //the found parent indicates the charttype

    //iterate through all coordinate systems
    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return nullptr;

    const uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( uno::Reference< XCoordinateSystem > const & xCooSys : aCooSysList )
    {
        //iterate through all chart types in the current coordinate system
        uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        OSL_ASSERT( xChartTypeContainer.is());
        if( !xChartTypeContainer.is() )
            continue;
        const uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( uno::Reference< XChartType > const & xChartType : aChartTypeList )
        {
            //iterate through all series in this chart type
            uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
            OSL_ASSERT( xDataSeriesContainer.is());
            if( !xDataSeriesContainer.is() )
                continue;

            const uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
            for( uno::Reference< XDataSeries > const & dataSeries : aSeriesList )
            {
                if( xGivenDataSeries==dataSeries )
                    return xChartType;
            }
        }
    }
    return nullptr;
}

std::vector< Reference< XDataSeries > >
    DiagramHelper::getDataSeriesFromDiagram(
        const Reference< XDiagram > & xDiagram )
{
    std::vector< Reference< XDataSeries > > aResult;

    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        const Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( Reference< XCoordinateSystem > const & coords : aCooSysSeq )
        {
            Reference< XChartTypeContainer > xCTCnt( coords, uno::UNO_QUERY_THROW );
            const Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( Reference< XChartType> const & chartType : aChartTypeSeq )
            {
                Reference< XDataSeriesContainer > xDSCnt( chartType, uno::UNO_QUERY_THROW );
                Sequence< Reference< XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries() );
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

Sequence< Sequence< Reference< XDataSeries > > >
        DiagramHelper::getDataSeriesGroups( const Reference< XDiagram > & xDiagram )
{
    vector< Sequence< Reference< XDataSeries > > > aResult;

    //iterate through all coordinate systems
    Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( xCooSysContainer.is() )
    {
        const Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( Reference< XCoordinateSystem > const & coords : aCooSysList )
        {
            //iterate through all chart types in the current coordinate system
            Reference< XChartTypeContainer > xChartTypeContainer( coords, uno::UNO_QUERY );
            if( !xChartTypeContainer.is() )
                continue;
            const Sequence< Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
            for( Reference< XChartType >  const & chartType : aChartTypeList )
            {
                Reference< XDataSeriesContainer > xDataSeriesContainer( chartType, uno::UNO_QUERY );
                if( !xDataSeriesContainer.is() )
                    continue;
                aResult.push_back( xDataSeriesContainer->getDataSeries() );
            }
        }
    }
    return comphelper::containerToSequence( aResult );
}

Reference< XChartType >
    DiagramHelper::getChartTypeByIndex( const Reference< XDiagram >& xDiagram, sal_Int32 nIndex )
{
    Reference< XChartType > xChartType;

    //iterate through all coordinate systems
    Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( ! xCooSysContainer.is())
        return xChartType;

    const Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    sal_Int32 nTypesSoFar = 0;
    for( Reference< XCoordinateSystem > const & coords : aCooSysList )
    {
        Reference< XChartTypeContainer > xChartTypeContainer( coords, uno::UNO_QUERY );
        if( !xChartTypeContainer.is() )
            continue;
        Sequence< Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        if( nIndex >= 0 && nIndex < (nTypesSoFar + aChartTypeList.getLength()) )
        {
            xChartType.set( aChartTypeList[nIndex - nTypesSoFar] );
            break;
        }
        nTypesSoFar += aChartTypeList.getLength();
    }

    return xChartType;
}

namespace
{

std::vector< Reference< XAxis > > lcl_getAxisHoldingCategoriesFromDiagram(
    const Reference< XDiagram > & xDiagram )
{
    std::vector< Reference< XAxis > > aRet;

    // return first x-axis as fall-back
    Reference< XAxis > xFallBack;
    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        const Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( Reference< XCoordinateSystem > const & xCooSys : aCooSysSeq )
        {
            OSL_ASSERT( xCooSys.is());
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                {
                    Reference< XAxis > xAxis = xCooSys->getAxisByDimension( nN,nI );
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
            const Reference< XDiagram >& xDiagram )
{
    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        const Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( Reference< XCoordinateSystem > const & xCooSys : aCooSysSeq )
        {
            OSL_ASSERT( xCooSys.is());
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                {
                    Reference< XAxis > xAxis = xCooSys->getAxisByDimension( nN,nI );
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
    const Reference< chart2::data::XLabeledDataSequence >& xCategories,
    const Reference< XDiagram >& xDiagram,
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

Reference< data::XLabeledDataSequence >
    DiagramHelper::getCategoriesFromDiagram(
        const Reference< XDiagram > & xDiagram )
{
    Reference< data::XLabeledDataSequence > xResult;

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
                    xResult.set( aScaleData.Categories );
                    uno::Reference<beans::XPropertySet> xProp(aScaleData.Categories->getValues(), uno::UNO_QUERY );
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
            const Reference< XChartType >& xChartType )
{
    if(!xChartType.is())
        return;
    OUString aMainSeq( xChartType->getRoleOfSequenceForSeriesLabel() );
    Reference< XDataSeriesContainer > xSeriesCnt( xChartType, uno::UNO_QUERY );
    if( !xSeriesCnt.is() )
        return;

    const Sequence< Reference< XDataSeries > > aSeriesSeq( xSeriesCnt->getDataSeries() );
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

Sequence< OUString > DiagramHelper::generateAutomaticCategoriesFromCooSys( const Reference< XCoordinateSystem > & xCooSys )
{
    Sequence< OUString > aRet;

    Reference< XChartTypeContainer > xTypeCntr( xCooSys, uno::UNO_QUERY );
    if( xTypeCntr.is() )
    {
        const Sequence< Reference< XChartType > > aChartTypes( xTypeCntr->getChartTypes() );
        for( Reference< XChartType > const & chartType : aChartTypes )
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
    uno::Reference< chart2::XCoordinateSystem > xCooSys( ChartModelHelper::getFirstCoordinateSystem( rModel ) );
    ExplicitCategoriesProvider aExplicitCategoriesProvider( xCooSys, rModel );
    return aExplicitCategoriesProvider.getSimpleCategories();
}

namespace
{
void lcl_switchToDateCategories( const Reference< XChartDocument >& xChartDoc, const Reference< XAxis >& xAxis )
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
            double fTest = 0.0;
            double fNan = 0.0;
            ::rtl::math::setNan( & fNan );
            sal_Int32 nN = aAnyCategories.getLength();
            for( ; nN--; )
            {
                Sequence< Any >& rCat = aAnyCategories[nN];
                if( rCat.getLength() > 1 )
                    rCat.realloc(1);
                if( rCat.getLength() == 1 )
                {
                    Any& rAny = rCat[0];
                    if( !(rAny>>=fTest) )
                    {
                        rAny <<= fNan;
                    }
                }
            }
            xDataAccess->setAnyRowDescriptions( aAnyCategories );
        }
        //check the numberformat at the axis
        Reference< beans::XPropertySet > xAxisProps( xAxis, uno::UNO_QUERY );
        Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( xChartDoc, uno::UNO_QUERY );
        if( xAxisProps.is() && xNumberFormatsSupplier.is() )
        {
            sal_Int32 nNumberFormat = -1;
            xAxisProps->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;

            Reference< util::XNumberFormats > xNumberFormats( xNumberFormatsSupplier->getNumberFormats() );
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

void lcl_switchToTextCategories( const Reference< XChartDocument >& xChartDoc, const Reference< XAxis >& xAxis )
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

void DiagramHelper::switchToDateCategories( const Reference< XChartDocument >& xChartDoc )
{
    if(xChartDoc.is())
    {
        ControllerLockGuardUNO aCtrlLockGuard( xChartDoc );

        Reference< chart2::XCoordinateSystem > xCooSys( ChartModelHelper::getFirstCoordinateSystem( xChartDoc ) );
        if( xCooSys.is() )
        {
            Reference< XAxis > xAxis( xCooSys->getAxisByDimension(0,0) );
            lcl_switchToDateCategories( xChartDoc, xAxis );
        }
    }
}

void DiagramHelper::switchToTextCategories( const Reference< XChartDocument >& xChartDoc )
{
    if(xChartDoc.is())
    {
        ControllerLockGuardUNO aCtrlLockGuard( xChartDoc );

        Reference< chart2::XCoordinateSystem > xCooSys( ChartModelHelper::getFirstCoordinateSystem( xChartDoc ) );
        if( xCooSys.is() )
        {
            Reference< XAxis > xAxis( xCooSys->getAxisByDimension(0,0) );
            lcl_switchToTextCategories( xChartDoc, xAxis );
        }
    }
}

bool DiagramHelper::isSupportingDateAxis( const Reference< chart2::XDiagram >& xDiagram )
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

Sequence< Reference< XChartType > >
    DiagramHelper::getChartTypesFromDiagram(
        const Reference< XDiagram > & xDiagram )
{
    std::vector< Reference< XChartType > > aResult;

    if(xDiagram.is())
    {
        try
        {
            Reference< XCoordinateSystemContainer > xCooSysCnt(
                xDiagram, uno::UNO_QUERY_THROW );
            const Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
                xCooSysCnt->getCoordinateSystems());
            for( Reference< XCoordinateSystem > const & coords : aCooSysSeq )
            {
                Reference< XChartTypeContainer > xCTCnt( coords, uno::UNO_QUERY_THROW );
                Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
                aResult.insert( aResult.end(), aChartTypeSeq.begin(), aChartTypeSeq.end() );
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return comphelper::containerToSequence( aResult );
}

bool DiagramHelper::areChartTypesCompatible( const Reference< ::chart2::XChartType >& xFirstType,
                const Reference< ::chart2::XChartType >& xSecondType )
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
    const Reference< XDiagram >& xDiagram,
    const Reference< XDataSeries >& xGivenDataSeries,
    bool bForward,
    bool bDoMove )
{
    bool bMovedOrMoveAllowed = false;

    try
    {
        uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );

        if( xGivenDataSeries.is() && xCooSysContainer.is() )
        {
            //find position of series.
            bool bFound = false;
            uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );

            for( sal_Int32 nCS = 0; !bFound && nCS < aCooSysList.getLength(); ++nCS )
            {
                uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );

                //iterate through all chart types in the current coordinate system
                uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
                OSL_ASSERT( xChartTypeContainer.is());
                if( !xChartTypeContainer.is() )
                    continue;
                uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
                uno::Reference< XChartType > xFormerChartType;

                for( sal_Int32 nT = 0; !bFound && nT < aChartTypeList.getLength(); ++nT )
                {
                    uno::Reference< XChartType > xCurrentChartType( aChartTypeList[nT] );

                    //iterate through all series in this chart type
                    uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xCurrentChartType, uno::UNO_QUERY );
                    OSL_ASSERT( xDataSeriesContainer.is());
                    if( !xDataSeriesContainer.is() )
                        continue;

                    uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );

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
                                        aSeriesList[ nOldSeriesIndex ] = aSeriesList[ nNewSeriesIndex ];
                                        aSeriesList[ nNewSeriesIndex ] = xGivenDataSeries;
                                        xDataSeriesContainer->setDataSeries( aSeriesList );
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
                                            uno::Reference< XDataSeriesContainer > xOtherDataSeriesContainer( xFormerChartType, uno::UNO_QUERY );
                                            if( xOtherDataSeriesContainer.is() )
                                            {
                                                uno::Sequence< uno::Reference< XDataSeries > > aOtherSeriesList( xOtherDataSeriesContainer->getDataSeries() );
                                                sal_Int32 nOtherSeriesIndex = aOtherSeriesList.getLength()-1;
                                                if( nOtherSeriesIndex >= 0 && nOtherSeriesIndex < aOtherSeriesList.getLength() )
                                                {
                                                    uno::Reference< XDataSeries > xExchangeSeries( aOtherSeriesList[nOtherSeriesIndex] );
                                                    aOtherSeriesList[nOtherSeriesIndex] = xGivenDataSeries;
                                                    xOtherDataSeriesContainer->setDataSeries(aOtherSeriesList);

                                                    aSeriesList[nOldSeriesIndex]=xExchangeSeries;
                                                    xDataSeriesContainer->setDataSeries(aSeriesList);
                                                }
                                            }
                                        }
                                    }
                                }
                                else if( nT+1 < aChartTypeList.getLength() )
                                {
                                    //exchange series with next charttype
                                    uno::Reference< XChartType > xOtherChartType( aChartTypeList[nT+1] );
                                    if( xOtherChartType.is() && DiagramHelper::areChartTypesCompatible( xOtherChartType, xCurrentChartType ) )
                                    {
                                        bMovedOrMoveAllowed = true;
                                        if( bDoMove )
                                        {
                                            uno::Reference< XDataSeriesContainer > xOtherDataSeriesContainer( xOtherChartType, uno::UNO_QUERY );
                                            if( xOtherDataSeriesContainer.is() )
                                            {
                                                uno::Sequence< uno::Reference< XDataSeries > > aOtherSeriesList( xOtherDataSeriesContainer->getDataSeries() );
                                                if( aOtherSeriesList.hasElements() )
                                                {
                                                    uno::Reference< XDataSeries > xExchangeSeries( aOtherSeriesList[0] );
                                                    aOtherSeriesList[0] = xGivenDataSeries;
                                                    xOtherDataSeriesContainer->setDataSeries(aOtherSeriesList);

                                                    aSeriesList[nOldSeriesIndex]=xExchangeSeries;
                                                    xDataSeriesContainer->setDataSeries(aSeriesList);
                                                }
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
    const Reference< XDiagram >& xDiagram,
    const Reference< XDataSeries >& xGivenDataSeries,
    bool bForward )
{
    const bool bDoMove = false;

    bool bIsMoveable = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        xDiagram, xGivenDataSeries, bForward, bDoMove );

    return bIsMoveable;
}

bool DiagramHelper::moveSeries( const Reference< XDiagram >& xDiagram, const Reference< XDataSeries >& xGivenDataSeries, bool bForward )
{
    const bool bDoMove = true;

    bool bMoved = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        xDiagram, xGivenDataSeries, bForward, bDoMove );

    return bMoved;
}

bool DiagramHelper::isSupportingFloorAndWall( const Reference<
                chart2::XDiagram >& xDiagram )
{
    //pies and donuts currently do not support this because of wrong files from older versions
    //todo: allow this in future again, if fileversion is available for OLE objects (metastream)
    //thus the wrong bottom can be removed on import

    const Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
    for( Reference< chart2::XChartType > const & xType : aTypes )
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

bool DiagramHelper::isPieOrDonutChart( const css::uno::Reference< css::chart2::XDiagram >& xDiagram )
{
    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex(
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
    const uno::Reference< chart2::XDiagram > & xDiagram,
    bool& rbFound, bool& rbAmbiguous )
{
    sal_Int32 nCommonGeom( DataPointGeometry3D::CUBOID );
    rbFound = false;
    rbAmbiguous = false;

    std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

    if( aSeriesVec.empty())
        rbAmbiguous = true;

    for (auto const& series : aSeriesVec)
    {
        try
        {
            sal_Int32 nGeom = 0;
            Reference< beans::XPropertySet > xProp(series, uno::UNO_QUERY_THROW);
            if( xProp->getPropertyValue( "Geometry3D") >>= nGeom )
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
    const Reference< chart2::XDiagram > & xDiagram,
    sal_Int32 nNewGeometry )
{
    std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

    for (auto const& series : aSeriesVec)
    {
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(
            series, "Geometry3D", uno::Any( nNewGeometry ));
    }
}

sal_Int32 DiagramHelper::getCorrectedMissingValueTreatment(
            const Reference< chart2::XDiagram > & xDiagram,
            const Reference< chart2::XChartType >& xChartType )
{
    sal_Int32 nResult = css::chart::MissingValueTreatment::LEAVE_GAP;
    const uno::Sequence < sal_Int32 > aAvailableMissingValueTreatments(
                ChartTypeHelper::getSupportedMissingValueTreatments( xChartType ) );

    uno::Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );
    if( xDiaProp.is() && (xDiaProp->getPropertyValue( "MissingValueTreatment" ) >>= nResult) )
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

DiagramPositioningMode DiagramHelper::getDiagramPositioningMode( const uno::Reference<
                chart2::XDiagram > & xDiagram )
{
    DiagramPositioningMode eMode = DiagramPositioningMode_AUTO;
    uno::Reference< beans::XPropertySet > xDiaProps( xDiagram, uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        RelativePosition aRelPos;
        RelativeSize aRelSize;
        if( (xDiaProps->getPropertyValue("RelativePosition") >>= aRelPos ) &&
            (xDiaProps->getPropertyValue("RelativeSize") >>= aRelSize ) )
        {
            bool bPosSizeExcludeAxes=false;
            xDiaProps->getPropertyValue("PosSizeExcludeAxes") >>= bPosSizeExcludeAxes;
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

bool DiagramHelper::setDiagramPositioning( const uno::Reference< frame::XModel >& xChartModel,
        const awt::Rectangle& rPosRect /*100th mm*/ )
{
    ControllerLockGuardUNO aCtrlLockGuard( xChartModel );

    bool bChanged = false;
    awt::Size aPageSize( ChartModelHelper::getPageSize(xChartModel) );
    uno::Reference< beans::XPropertySet > xDiaProps( ChartModelHelper::findDiagram( xChartModel ), uno::UNO_QUERY );
    if( !xDiaProps.is() )
        return bChanged;

    RelativePosition aOldPos;
    RelativeSize aOldSize;
    xDiaProps->getPropertyValue("RelativePosition" ) >>= aOldPos;
    xDiaProps->getPropertyValue("RelativeSize" ) >>= aOldSize;

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

    xDiaProps->setPropertyValue( "RelativePosition", uno::Any(aNewPos) );
    xDiaProps->setPropertyValue( "RelativeSize", uno::Any(aNewSize) );

    bChanged = (aOldPos.Anchor!=aNewPos.Anchor) ||
        (aOldPos.Primary!=aNewPos.Primary) ||
        (aOldPos.Secondary!=aNewPos.Secondary) ||
        (aOldSize.Primary!=aNewSize.Primary) ||
        (aOldSize.Secondary!=aNewSize.Secondary);
    return bChanged;
}

awt::Rectangle DiagramHelper::getDiagramRectangleFromModel( const uno::Reference< frame::XModel >& xChartModel )
{
    awt::Rectangle aRet(-1,-1,-1,-1);

    uno::Reference< beans::XPropertySet > xDiaProps( ChartModelHelper::findDiagram( xChartModel ), uno::UNO_QUERY );
    if( !xDiaProps.is() )
        return aRet;

    awt::Size aPageSize( ChartModelHelper::getPageSize(xChartModel) );

    RelativePosition aRelPos;
    RelativeSize aRelSize;
    xDiaProps->getPropertyValue("RelativePosition" ) >>= aRelPos;
    xDiaProps->getPropertyValue("RelativeSize" ) >>= aRelSize;

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
    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(SvtSaveOptions().GetODFSaneDefaultVersion());
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
