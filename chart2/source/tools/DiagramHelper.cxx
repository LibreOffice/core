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

#include "DiagramHelper.hxx"
#include "LegendHelper.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "AxisHelper.hxx"
#include "ContainerHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "ChartModelHelper.hxx"
#include "CommonConverters.hxx"
#include "ExplicitCategoriesProvider.hxx"
#include "servicenames_charttypes.hxx"
#include "RelativePositionHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "NumberFormatterWrapper.hxx"

#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/InterpretedData.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>

#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <unotools/saveopt.hxx>
#include <rtl/math.hxx>
#include <svl/zformat.hxx>
// header for class Application
#include <vcl/svapp.hxx>

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
        const Reference< lang::XMultiServiceFactory > & xChartTypeManager,
        const OUString & rPreferredTemplateName )
{
    DiagramHelper::tTemplateWithServiceName aResult;

    if( ! (xChartTypeManager.is() && xDiagram.is()))
        return aResult;

    Sequence< OUString > aServiceNames( xChartTypeManager->getAvailableServiceNames());
    const sal_Int32 nLength = aServiceNames.getLength();

    bool bHasPreferredTemplate = !rPreferredTemplateName.isEmpty();
    bool bTemplateFound = false;

    if( bHasPreferredTemplate )
    {
        Reference< XChartTypeTemplate > xTempl(
            xChartTypeManager->createInstance( rPreferredTemplateName ), uno::UNO_QUERY );

        if( xTempl.is() &&
            xTempl->matchesTemplate( xDiagram, sal_True ))
        {
            aResult.first = xTempl;
            aResult.second = rPreferredTemplateName;
            bTemplateFound = true;
        }
    }

    for( sal_Int32 i = 0; ! bTemplateFound && i < nLength; ++i )
    {
        try
        {
            if( ! bHasPreferredTemplate ||
                ! rPreferredTemplateName.equals( aServiceNames[ i ] ))
            {
                Reference< XChartTypeTemplate > xTempl(
                    xChartTypeManager->createInstance( aServiceNames[ i ] ), uno::UNO_QUERY_THROW );

                if( xTempl->matchesTemplate( xDiagram, sal_True ))
                {
                    aResult.first = xTempl;
                    aResult.second = aServiceNames[ i ];
                    bTemplateFound = true;
                }
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
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
        if( xCnt.is())
        {
            Sequence< Reference< XCoordinateSystem > > aCooSys(
                xCnt->getCoordinateSystems());
            uno::Any aValue;
            aValue <<= bVertical;
            for( sal_Int32 i=0; i<aCooSys.getLength(); ++i )
            {
                uno::Reference< XCoordinateSystem > xCooSys( aCooSys[i] );
                Reference< beans::XPropertySet > xProp( xCooSys, uno::UNO_QUERY );
                bool bChanged = false;
                if( xProp.is() )
                {
                    bool bOldSwap = sal_False;
                    if( !(xProp->getPropertyValue( "SwapXAndYAxis" ) >>= bOldSwap)
                        || bVertical != bOldSwap )
                        bChanged = true;

                    if( bChanged )
                        xProp->setPropertyValue( "SwapXAndYAxis", aValue );
                }
                if( xCooSys.is() )
                {
                    const sal_Int32 nDimensionCount( xCooSys->getDimension() );
                    sal_Int32 nDimIndex = 0;
                    for(nDimIndex=0; nDimIndex<nDimensionCount; ++nDimIndex)
                    {
                        const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nDimIndex);
                        for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                        {
                            Reference< chart2::XAxis > xAxis( xCooSys->getAxisByDimension( nDimIndex,nI ));
                            if( xAxis.is() )
                            {
                                //adapt title rotation only when axis swapping has changed
                                if( bChanged )
                                {
                                    Reference< XTitled > xTitled( xAxis, uno::UNO_QUERY );
                                    if( xTitled.is())
                                    {
                                        Reference< beans::XPropertySet > xTitleProps( xTitled->getTitleObject(), uno::UNO_QUERY );
                                        if( !xTitleProps.is() )
                                            continue;
                                        double fAngleDegree = 0.0;
                                        xTitleProps->getPropertyValue( "TextRotation" ) >>= fAngleDegree;
                                        if( !::rtl::math::approxEqual( fAngleDegree, 0.0 )
                                            && !::rtl::math::approxEqual( fAngleDegree, 90.0 ) )
                                            continue;

                                        double fNewAngleDegree = 0.0;
                                        if( !bVertical && nDimIndex == 1 )
                                            fNewAngleDegree = 90.0;
                                        else if( bVertical && nDimIndex == 0 )
                                            fNewAngleDegree = 90.0;

                                        xTitleProps->setPropertyValue( "TextRotation", uno::makeAny( fNewAngleDegree ));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

bool DiagramHelper::getVertical( const uno::Reference< chart2::XDiagram > & xDiagram,
                             bool& rbFound, bool& rbAmbiguous )
{
    bool bValue = false;
    rbFound = false;
    rbAmbiguous = false;

    Reference< XCoordinateSystemContainer > xCnt( xDiagram, uno::UNO_QUERY );
    if( xCnt.is())
    {
        Sequence< Reference< XCoordinateSystem > > aCooSys(
            xCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSys.getLength(); ++i )
        {
            Reference< beans::XPropertySet > xProp( aCooSys[i], uno::UNO_QUERY );
            if( xProp.is())
            {
                bool bCurrent = false;
                if( xProp->getPropertyValue( "SwapXAndYAxis" ) >>= bCurrent )
                {
                    if( !rbFound )
                    {
                        bValue = bCurrent;
                        rbFound = true;
                    }
                    else if( bCurrent != bValue )
                    {
                        // ambiguous -> choose always first found
                        rbAmbiguous = true;
                    }
                }
            }
        }
    }
    return bValue;
}

void DiagramHelper::setStackMode(
    const Reference< XDiagram > & xDiagram,
    StackMode eStackMode,
    bool bOnlyAtFirstChartType /* = true */
)
{
    try
    {
        if( eStackMode == StackMode_AMBIGUOUS )
            return;

        bool bValueFound = false;
        bool bIsAmbiguous = false;
        StackMode eOldStackMode = DiagramHelper::getStackMode( xDiagram, bValueFound, bIsAmbiguous );

        if( eStackMode == eOldStackMode && !bIsAmbiguous )
            return;

        StackingDirection eNewDirection = StackingDirection_NO_STACKING;
        if( eStackMode == StackMode_Y_STACKED || eStackMode == StackMode_Y_STACKED_PERCENT )
            eNewDirection = StackingDirection_Y_STACKING;
        else if( eStackMode == StackMode_Z_STACKED )
            eNewDirection = StackingDirection_Z_STACKING;

        uno::Any aNewDirection( uno::makeAny(eNewDirection) );

        sal_Bool bPercent = sal_False;
        if( eStackMode == StackMode_Y_STACKED_PERCENT )
            bPercent = sal_True;

        //iterate through all coordinate systems
        uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
        if( !xCooSysContainer.is() )
            return;
        uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
        {
            uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );
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
            sal_Int32 nMax = aChartTypeList.getLength();
            if( bOnlyAtFirstChartType
                && nMax >= 1 )
                nMax = 1;
            for( sal_Int32 nT = 0; nT < nMax; ++nT )
            {
                uno::Reference< XChartType > xChartType( aChartTypeList[nT] );

                //iterate through all series in this chart type
                uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
                OSL_ASSERT( xDataSeriesContainer.is());
                if( !xDataSeriesContainer.is() )
                    continue;

                uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
                for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
                {
                    Reference< beans::XPropertySet > xProp( aSeriesList[nS], uno::UNO_QUERY );
                    if(xProp.is())
                        xProp->setPropertyValue( "StackingDirection", aNewDirection );
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

StackMode DiagramHelper::getStackMode( const Reference< XDiagram > & xDiagram, bool& rbFound, bool& rbAmbiguous )
{
    rbFound=false;
    rbAmbiguous=false;

    StackMode eGlobalStackMode = StackMode_NONE;

    //iterate through all coordinate systems
    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is() )
        return eGlobalStackMode;
    uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );

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
    StackMode eStackMode = StackMode_NONE;
    rbFound = false;
    rbAmbiguous = false;

    try
    {
        Reference< XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XDataSeries > > aSeries( xDSCnt->getDataSeries());

        chart2::StackingDirection eCommonDirection = chart2::StackingDirection_NO_STACKING;
        bool bDirectionInitialized = false;

        // first series is irrelvant for stacking, start with second, unless
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
            (void)(bSuccess);  // avoid warning in non-debug builds
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
                eStackMode = StackMode_Z_STACKED;
            else if( eCommonDirection == chart2::StackingDirection_Y_STACKING )
            {
                eStackMode = StackMode_Y_STACKED;

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
                                eStackMode = StackMode_Y_STACKED_PERCENT;
                        }
                    }
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
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
            Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
                xCooSysCnt->getCoordinateSystems());

            for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
            {
                Reference< XCoordinateSystem > xCooSys( aCooSysSeq[i] );
                if(xCooSys.is())
                {
                    nResult = xCooSys->getDimension();
                    break;
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
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
        Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
        {
            Reference< XCoordinateSystem > xOldCooSys( aCooSysList[nCS], uno::UNO_QUERY );
            Reference< XCoordinateSystem > xNewCooSys;

            Reference< XChartTypeContainer > xChartTypeContainer( xOldCooSys, uno::UNO_QUERY );
            if( !xChartTypeContainer.is() )
                continue;

            Sequence< Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
            for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
            {
                Reference< XChartType > xChartType( aChartTypeList[nT], uno::UNO_QUERY );
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
        if( nNewDimensionCount==3 && eStackMode != StackMode_Z_STACKED && bIsSupportingOnlyDeepStackingFor3D )
            DiagramHelper::setStackMode( xDiagram, StackMode_Z_STACKED );
        else if( nNewDimensionCount==2 && eStackMode == StackMode_Z_STACKED )
            DiagramHelper::setStackMode( xDiagram, StackMode_NONE );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
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
    if( xCont.is())
    {
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
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
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
    if( !xProp.is() )
        return bChanged;

    sal_Int32 nNewAxisIndex = bAttachToMainAxis ? 0 : 1;
    sal_Int32 nOldAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
    uno::Reference< chart2::XAxis > xOldAxis( DiagramHelper::getAttachedAxis( xDataSeries, xDiagram ) );

    if( nOldAxisIndex != nNewAxisIndex )
    {
        try
        {
            xProp->setPropertyValue( "AttachedAxisIndex", uno::makeAny( nNewAxisIndex ) );
            bChanged = true;
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
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
        return 0;
    if(!xDiagram.is())
        return 0;

    //iterate through the model to find the given xSeries
    //the found parent indicates the charttype

    //iterate through all coordinate systems
    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return 0;

    uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );

        //iterate through all chart types in the current coordinate system
        uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        OSL_ASSERT( xChartTypeContainer.is());
        if( !xChartTypeContainer.is() )
            continue;
        uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference< XChartType > xChartType( aChartTypeList[nT] );

            //iterate through all series in this chart type
            uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
            OSL_ASSERT( xDataSeriesContainer.is());
            if( !xDataSeriesContainer.is() )
                continue;

            uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
            for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
            {
                if( xGivenDataSeries==aSeriesList[nS] )
                    return xChartType;
            }
        }
    }
    return 0;
}

::std::vector< Reference< XDataSeries > >
    DiagramHelper::getDataSeriesFromDiagram(
        const Reference< XDiagram > & xDiagram )
{
    ::std::vector< Reference< XDataSeries > > aResult;

    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[i], uno::UNO_QUERY_THROW );
            Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( sal_Int32 j=0; j<aChartTypeSeq.getLength(); ++j )
            {
                Reference< XDataSeriesContainer > xDSCnt( aChartTypeSeq[j], uno::UNO_QUERY_THROW );
                Sequence< Reference< XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries() );
                ::std::copy( aSeriesSeq.getConstArray(), aSeriesSeq.getConstArray() + aSeriesSeq.getLength(),
                             ::std::back_inserter( aResult ));
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
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
        Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
        {
            //iterate through all chart types in the current coordinate system
            Reference< XChartTypeContainer > xChartTypeContainer( aCooSysList[nCS], uno::UNO_QUERY );
            if( !xChartTypeContainer.is() )
                continue;
            Sequence< Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
            for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
            {
                Reference< XDataSeriesContainer > xDataSeriesContainer( aChartTypeList[nT], uno::UNO_QUERY );
                if( !xDataSeriesContainer.is() )
                    continue;
                aResult.push_back( xDataSeriesContainer->getDataSeries() );
            }
        }
    }
    return ContainerHelper::ContainerToSequence( aResult );
}

Reference< XChartType >
    DiagramHelper::getChartTypeByIndex( const Reference< XDiagram >& xDiagram, sal_Int32 nIndex )
{
    Reference< XChartType > xChartType;

    //iterate through all coordinate systems
    Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( ! xCooSysContainer.is())
        return xChartType;

    Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    sal_Int32 nTypesSoFar = 0;
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        Reference< XChartTypeContainer > xChartTypeContainer( aCooSysList[nCS], uno::UNO_QUERY );
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

    Reference< XAxis > xResult;
    // return first x-axis as fall-back
    Reference< XAxis > xFallBack;
    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< XCoordinateSystem > xCooSys( aCooSysSeq[i] );
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
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
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
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< XCoordinateSystem > xCooSys( aCooSysSeq[i] );
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
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
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

    std::vector< Reference< chart2::XAxis > >::iterator aIt( aCatAxes.begin() );
    std::vector< Reference< chart2::XAxis > >::const_iterator aEnd( aCatAxes.end() );

    for( aIt = aCatAxes.begin(); aIt != aEnd; ++aIt )
    {
        Reference< chart2::XAxis > xCatAxis(*aIt);
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
        std::vector< Reference< chart2::XAxis > >::iterator aIt( aCatAxes.begin() );
        std::vector< Reference< chart2::XAxis > >::const_iterator aEnd( aCatAxes.end() );
        //search for first categories
        if( aIt != aEnd )
        {
            Reference< chart2::XAxis > xCatAxis(*aIt);
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
                            xProp->setPropertyValue( "Role", uno::makeAny( OUString("categories") ) );
                        }
                        catch( const uno::Exception & ex )
                        {
                            ASSERT_EXCEPTION( ex );
                        }
                    }
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

void lcl_generateAutomaticCategoriesFromChartType(
            Sequence< OUString >& rRet,
            const Reference< XChartType >& xChartType )
{
    if(!xChartType.is())
        return;
    OUString aMainSeq( xChartType->getRoleOfSequenceForSeriesLabel() );
        Reference< XDataSeriesContainer > xSeriesCnt( xChartType, uno::UNO_QUERY );
    if( xSeriesCnt.is() )
    {
        Sequence< Reference< XDataSeries > > aSeriesSeq( xSeriesCnt->getDataSeries() );
        for( sal_Int32 nS = 0; nS < aSeriesSeq.getLength(); nS++ )
        {
            Reference< data::XDataSource > xDataSource( aSeriesSeq[nS], uno::UNO_QUERY );
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
            if( rRet.getLength() )
                return;
        }
    }
}

Sequence< OUString > DiagramHelper::generateAutomaticCategoriesFromCooSys( const Reference< XCoordinateSystem > & xCooSys )
{
    Sequence< OUString > aRet;

    Reference< XChartTypeContainer > xTypeCntr( xCooSys, uno::UNO_QUERY );
    if( xTypeCntr.is() )
    {
        Sequence< Reference< XChartType > > aChartTypes( xTypeCntr->getChartTypes() );
        for( sal_Int32 nN=0; nN<aChartTypes.getLength(); nN++ )
        {
            lcl_generateAutomaticCategoriesFromChartType( aRet, aChartTypes[nN] );
            if( aRet.getLength() )
                return aRet;
        }
    }
    return aRet;
}

Sequence< OUString > DiagramHelper::getExplicitSimpleCategories(
            const Reference< XChartDocument >& xChartDoc )
{
    Sequence< OUString > aRet;
    uno::Reference< frame::XModel > xChartModel( xChartDoc, uno::UNO_QUERY );
    if(xChartModel.is())
    {
        uno::Reference< chart2::XCoordinateSystem > xCooSys( ChartModelHelper::getFirstCoordinateSystem( xChartModel ) );
        ExplicitCategoriesProvider aExplicitCategoriesProvider( xCooSys, xChartModel );
        aRet = aExplicitCategoriesProvider.getSimpleCategories();
    }
    return aRet;
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
                        rAny = uno::makeAny(fNan);
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
            xAxisProps->getPropertyValue( "NumberFormat" ) >>= nNumberFormat;

            Reference< util::XNumberFormats > xNumberFormats = Reference< util::XNumberFormats >( xNumberFormatsSupplier->getNumberFormats() );
            if( xNumberFormats.is() )
            {
                Reference< beans::XPropertySet > xKeyProps;
                try
                {
                    xKeyProps = xNumberFormats->getByKey( nNumberFormat );
                }
                catch( const uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
                sal_Int32 nType = util::NumberFormat::UNDEFINED;
                if( xKeyProps.is() )
                    xKeyProps->getPropertyValue( "Type" ) >>= nType;
                if( !( nType & util::NumberFormat::DATE ) )
                {
                    //set a date format to the axis
                    sal_Bool bCreate = sal_True;
                    const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
                    Sequence<sal_Int32> aKeySeq = xNumberFormats->queryKeys( util::NumberFormat::DATE,  rLocaleDataWrapper.getLanguageTag().getLocale(), bCreate );
                    if( aKeySeq.getLength() )
                    {
                        xAxisProps->setPropertyValue( "NumberFormat", uno::makeAny(aKeySeq[0]) );
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
    Reference< frame::XModel > xChartModel( xChartDoc, uno::UNO_QUERY );
    if(xChartModel.is())
    {
        ControllerLockGuard aCtrlLockGuard( xChartModel );

        Reference< chart2::XCoordinateSystem > xCooSys( ChartModelHelper::getFirstCoordinateSystem( xChartModel ) );
        if( xCooSys.is() )
        {
            Reference< XAxis > xAxis( xCooSys->getAxisByDimension(0,0) );
            lcl_switchToDateCategories( xChartDoc, xAxis );
        }
    }
}

void DiagramHelper::switchToTextCategories( const Reference< XChartDocument >& xChartDoc )
{
    Reference< frame::XModel > xChartModel( xChartDoc, uno::UNO_QUERY );
    if(xChartModel.is())
    {
        ControllerLockGuard aCtrlLockGuard( xChartModel );

        Reference< chart2::XCoordinateSystem > xCooSys( ChartModelHelper::getFirstCoordinateSystem( xChartModel ) );
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
            DiagramHelper::getChartTypeByIndex( xDiagram, 0 ), DiagramHelper::getDimension( xDiagram ), 0 );
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
    Reference< util::XNumberFormats > xNumberFormats( xNumberFormatsSupplier->getNumberFormats() );
    if( xNumberFormats.is() )
    {
        sal_Bool bCreate = sal_True;
        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
        Sequence<sal_Int32> aKeySeq = xNumberFormats->queryKeys( util::NumberFormat::DATE,
            rLocaleDataWrapper.getLanguageTag().getLocale(), bCreate );
        if( aKeySeq.getLength() )
        {
            nRet = aKeySeq[0];
        }
    }

    //try to get a date format with full year display
    NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
    SvNumberFormatter* pNumFormatter = aNumberFormatterWrapper.getSvNumberFormatter();
    if( pNumFormatter )
    {
        const SvNumberformat* pFormat = pNumFormatter->GetEntry( nRet );
        if( pFormat )
            nRet = pNumFormatter->GetFormatIndex( NF_DATE_SYS_DDMMYYYY, pFormat->GetLanguage() );
    }
    return nRet;
}

sal_Int32 DiagramHelper::getPercentNumberFormat( const Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
{
    sal_Int32 nRet=-1;
    Reference< util::XNumberFormats > xNumberFormats( xNumberFormatsSupplier->getNumberFormats() );
    if( xNumberFormats.is() )
    {
        sal_Bool bCreate = sal_True;
        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
        Sequence<sal_Int32> aKeySeq = xNumberFormats->queryKeys( util::NumberFormat::PERCENT,
            rLocaleDataWrapper.getLanguageTag().getLocale(), bCreate );
        if( aKeySeq.getLength() )
        {
            nRet = aKeySeq[0];
        }
    }
    return nRet;
}

Sequence< Reference< XChartType > >
    DiagramHelper::getChartTypesFromDiagram(
        const Reference< XDiagram > & xDiagram )
{
    ::std::vector< Reference< XChartType > > aResult;

    if(xDiagram.is())
    {
        try
        {
            Reference< XCoordinateSystemContainer > xCooSysCnt(
                xDiagram, uno::UNO_QUERY_THROW );
            Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
                xCooSysCnt->getCoordinateSystems());
            for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
            {
                Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[i], uno::UNO_QUERY_THROW );
                Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
                ::std::copy( aChartTypeSeq.getConstArray(),
                                aChartTypeSeq.getConstArray() + aChartTypeSeq.getLength(),
                                ::std::back_inserter( aResult ));
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return ContainerHelper::ContainerToSequence( aResult );
}

bool DiagramHelper::areChartTypesCompatible( const Reference< ::chart2::XChartType >& xFirstType,
                const Reference< ::chart2::XChartType >& xSecondType )
{
    if( !xFirstType.is() || !xSecondType.is() )
        return false;

    ::std::vector< OUString > aFirstRoles( ContainerHelper::SequenceToVector( xFirstType->getSupportedMandatoryRoles() ) );
    ::std::vector< OUString > aSecondRoles( ContainerHelper::SequenceToVector( xSecondType->getSupportedMandatoryRoles() ) );
    ::std::sort( aFirstRoles.begin(), aFirstRoles.end() );
    ::std::sort( aSecondRoles.begin(), aSecondRoles.end() );
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

        //find position of series.
        bool bFound = false;

        if( xGivenDataSeries.is() && xCooSysContainer.is() )
        {
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

                        // We found the series we are interrested in !
                        if( xGivenDataSeries==aSeriesList[nS] )
                        {
                            sal_Int32 nOldSeriesIndex = nS;
                            bFound = true;

                            try
                            {
                                sal_Int32 nNewSeriesIndex = nS;

                                if( bForward )
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
                                                sal_Int32 nOtherSeriesIndex = 0;
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
    bool bIsMoveable = false;
    const bool bDoMove = false;

    bIsMoveable = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        xDiagram, xGivenDataSeries, bForward, bDoMove );

    return bIsMoveable;
}

bool DiagramHelper::moveSeries( const Reference< XDiagram >& xDiagram, const Reference< XDataSeries >& xGivenDataSeries, bool bForward )
{
    bool bMoved = false;
    const bool bDoMove = true;

    bMoved = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        xDiagram, xGivenDataSeries, bForward, bDoMove );

    return bMoved;
}

bool DiagramHelper::isSupportingFloorAndWall( const Reference<
                chart2::XDiagram >& xDiagram )
{
    //pies and donuts currently do not support this because of wrong files from older versions
    //todo: allow this in future again, if fileversion are available for ole objects (metastream)
    //thus the wrong bottom can be removed on import

    Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
    for( sal_Int32 nN = 0; nN < aTypes.getLength(); nN++ )
    {
        Reference< chart2::XChartType > xType( aTypes[nN] );
        if( xType.is() && xType->getChartType().match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return false;
        if( xType.is() && xType->getChartType().match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return false;
        if( xType.is() && xType->getChartType().match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
            return false;
    }
    return true;
}

bool DiagramHelper::isPieOrDonutChart( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram >& xDiagram )
{
    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex(
        xDiagram, 0 ) );

    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
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

    ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

    if( aSeriesVec.empty())
        rbAmbiguous = true;

    for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aIt =
             aSeriesVec.begin(); aIt != aSeriesVec.end(); ++aIt )
    {
        try
        {
            sal_Int32 nGeom = 0;
            Reference< beans::XPropertySet > xProp( *aIt, uno::UNO_QUERY_THROW );
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
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return nCommonGeom;
}

void DiagramHelper::setGeometry3D(
    const Reference< chart2::XDiagram > & xDiagram,
    sal_Int32 nNewGeometry )
{
    ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

    for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aIt =
             aSeriesVec.begin(); aIt != aSeriesVec.end(); ++aIt )
    {
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(
            *aIt, "Geometry3D", uno::makeAny( nNewGeometry ));
    }
}

sal_Int32 DiagramHelper::getCorrectedMissingValueTreatment(
            const Reference< chart2::XDiagram > & xDiagram,
            const Reference< chart2::XChartType >& xChartType )
{
    sal_Int32 nResult = ::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP;
    uno::Sequence < sal_Int32 > aAvailableMissingValueTreatments(
                ChartTypeHelper::getSupportedMissingValueTreatments( xChartType ) );

    uno::Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );
    if( xDiaProp.is() && (xDiaProp->getPropertyValue( "MissingValueTreatment" ) >>= nResult) )
    {
        //ensure that the set value is supported by this charttype
        for( sal_Int32 nN = 0; nN < aAvailableMissingValueTreatments.getLength(); nN++ )
            if( aAvailableMissingValueTreatments[nN] == nResult )
                return nResult; //ok
    }

    //otherwise use the first supported one
    if( aAvailableMissingValueTreatments.getLength() )
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

void lcl_ensureRange0to1( double& rValue )
{
    if(rValue<0.0)
        rValue=0.0;
    if(rValue>1.0)
        rValue=1.0;
}

bool DiagramHelper::setDiagramPositioning( const uno::Reference< frame::XModel >& xChartModel,
        const awt::Rectangle& rPosRect /*100th mm*/ )
{
    ControllerLockGuard aCtrlLockGuard( xChartModel );

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

    xDiaProps->setPropertyValue( "RelativePosition", uno::makeAny(aNewPos) );
    xDiaProps->setPropertyValue( "RelativeSize", uno::makeAny(aNewSize) );

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
    const uno::Reference< frame::XModel >& xChartModel
    , bool bResetModifiedState, bool bConvertAlsoFromAutoPositioning )
{
    //return true if something was changed
    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );
    if( nCurrentODFVersion > SvtSaveOptions::ODFVER_012 )
    {
        uno::Reference< ::com::sun::star::chart::XChartDocument > xOldDoc( xChartModel, uno::UNO_QUERY ) ;
        if( xOldDoc.is() )
        {
            uno::Reference< ::com::sun::star::chart::XDiagramPositioning > xDiagramPositioning( xOldDoc->getDiagram(), uno::UNO_QUERY );
            if( xDiagramPositioning.is() && ( bConvertAlsoFromAutoPositioning || !xDiagramPositioning->isAutomaticDiagramPositioning() )
                && !xDiagramPositioning->isExcludingDiagramPositioning() )
            {
                ControllerLockGuard aCtrlLockGuard( xChartModel );
                uno::Reference< util::XModifiable > xModifiable( xChartModel, uno::UNO_QUERY );
                bool bModelWasModified = xModifiable.is() && xModifiable->isModified();
                xDiagramPositioning->setDiagramPositionExcludingAxes( xDiagramPositioning->calculateDiagramPositionExcludingAxes() );
                if(bResetModifiedState && !bModelWasModified && xModifiable.is() )
                    xModifiable->setModified(sal_False);
                return true;
            }
        }
    }
    return false;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
