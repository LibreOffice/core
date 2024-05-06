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
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <RelativePositionHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <NumberFormatterWrapper.hxx>
#include <unonames.hxx>
#include <BaseCoordinateSystem.hxx>

#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>

#include <com/sun/star/util/NumberFormat.hpp>

#include <unotools/saveopt.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

#include <limits>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::chart2::XAnyDescriptionAccess;

namespace chart
{

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
        const std::vector< rtl::Reference< DataSeries > > & aSeries = xChartType->getDataSeries2();

        chart2::StackingDirection eCommonDirection = chart2::StackingDirection_NO_STACKING;
        bool bDirectionInitialized = false;

        // first series is irrelevant for stacking, start with second, unless
        // there is only one series
        const sal_Int32 nSeriesCount = aSeries.size();
        sal_Int32 i = (nSeriesCount == 1) ? 0: 1;
        for( ; i<nSeriesCount; ++i )
        {
            rbFound = true;
            chart2::StackingDirection eCurrentDirection = eCommonDirection;
            // property is not MAYBEVOID
            bool bSuccess = ( aSeries[i]->getPropertyValue( u"StackingDirection"_ustr ) >>= eCurrentDirection );
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

bool DiagramHelper::isSeriesAttachedToMainAxis(
                          const rtl::Reference< ::chart::DataSeries >& xDataSeries )
{
    sal_Int32 nAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
    return (nAxisIndex==0);
}

static void lcl_generateAutomaticCategoriesFromChartType(
            Sequence< OUString >& rRet,
            const rtl::Reference< ChartType >& xChartType )
{
    if(!xChartType.is())
        return;
    OUString aMainSeq( xChartType->getRoleOfSequenceForSeriesLabel() );

    const std::vector< rtl::Reference< DataSeries > > & aSeriesSeq = xChartType->getDataSeries2();
    for( rtl::Reference< DataSeries > const & dataSeries : aSeriesSeq )
    {
        uno::Reference< data::XLabeledDataSequence > xLabeledSeq =
            ::chart::DataSeriesHelper::getDataSequenceByRole( dataSeries, aMainSeq );
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
                    xKeyProps->getPropertyValue( u"Type"_ustr ) >>= nType;
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

bool DiagramHelper::isDateNumberFormat( sal_Int32 nNumberFormat, const Reference< util::XNumberFormats >& xNumberFormats )
{
    bool bIsDate = false;
    if( !xNumberFormats.is() )
        return bIsDate;

    Reference< beans::XPropertySet > xKeyProps = xNumberFormats->getByKey( nNumberFormat );
    if( xKeyProps.is() )
    {
        sal_Int32 nType = util::NumberFormat::UNDEFINED;
        xKeyProps->getPropertyValue( u"Type"_ustr ) >>= nType;
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
        nRet = pNumFormatter->GetEditFormat( fNumber, nRet, nType, nullptr);
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
    rtl::Reference< Diagram > xDiagram = xChartModel->getFirstChartDiagram();
    if( !xDiagram.is() )
        return bChanged;

    RelativePosition aOldPos;
    RelativeSize aOldSize;
    xDiagram->getPropertyValue(u"RelativePosition"_ustr ) >>= aOldPos;
    xDiagram->getPropertyValue(u"RelativeSize"_ustr ) >>= aOldSize;

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

    xDiagram->setPropertyValue( u"RelativePosition"_ustr, uno::Any(aNewPos) );
    xDiagram->setPropertyValue( u"RelativeSize"_ustr, uno::Any(aNewSize) );

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

    rtl::Reference< Diagram > xDiagram = xChartModel->getFirstChartDiagram();
    if( !xDiagram.is() )
        return aRet;

    awt::Size aPageSize( ChartModelHelper::getPageSize(xChartModel) );

    RelativePosition aRelPos;
    RelativeSize aRelSize;
    xDiagram->getPropertyValue(u"RelativePosition"_ustr ) >>= aRelPos;
    xDiagram->getPropertyValue(u"RelativeSize"_ustr ) >>= aRelSize;

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
