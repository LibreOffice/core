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

#include "StatisticsItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "RegressionCurveHelper.hxx"
#include "ItemPropertyMap.hxx"
#include "ErrorBar.hxx"
#include "PropertyHelper.hxx"
#include "ChartModelHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "StatisticsHelper.hxx"

#include "GraphicPropertyItemConverter.hxx"
#include <unonames.hxx>

#include <svl/stritem.hxx>
#include <svx/chrtitem.hxx>
#include <svl/intitem.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/lang/XServiceName.hpp>

#include <functional>
#include <algorithm>
#include <vector>

using namespace ::com::sun::star;

namespace
{

uno::Reference< beans::XPropertySet > lcl_GetErrorBar(
    const uno::Reference< beans::XPropertySet > & xProp, bool bYError )
{
    uno::Reference< beans::XPropertySet > xResult;

    if( xProp.is())
        try
        {
        ( xProp->getPropertyValue( bYError ? OUString(CHART_UNONAME_ERRORBAR_Y) : OUString(CHART_UNONAME_ERRORBAR_X) ) >>= xResult );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }

    return xResult;
}

uno::Reference< beans::XPropertySet > lcl_GetDefaultErrorBar()
{
    // todo: use a valid context
    return uno::Reference< beans::XPropertySet >(
        ::chart::createErrorBar( uno::Reference< uno::XComponentContext >()));
}

void lcl_getErrorValues( const uno::Reference< beans::XPropertySet > & xErrorBarProp,
                    double & rOutPosError, double & rOutNegError )
{
    if( ! xErrorBarProp.is())
        return;

    try
    {
        xErrorBarProp->getPropertyValue( "PositiveError" ) >>= rOutPosError;
        xErrorBarProp->getPropertyValue( "NegativeError" ) >>= rOutNegError;
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void lcl_getErrorIndicatorValues(
    const uno::Reference< beans::XPropertySet > & xErrorBarProp,
    bool & rOutShowPosError, bool & rOutShowNegError )
{
    if( ! xErrorBarProp.is())
        return;

    try
    {
        xErrorBarProp->getPropertyValue( "ShowPositiveError" ) >>= rOutShowPosError;
        xErrorBarProp->getPropertyValue( "ShowNegativeError" ) >>= rOutShowNegError;
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

uno::Reference< beans::XPropertySet > lcl_getEquationProperties(
    const uno::Reference< beans::XPropertySet > & xSeriesPropSet, const SfxItemSet * pItemSet )
{
    bool bEquationExists = true;

    // ensure that a trendline is on
    if( pItemSet )
    {
        SvxChartRegress eRegress = CHREGRESS_NONE;
        const SfxPoolItem *pPoolItem = nullptr;
        if( pItemSet->GetItemState( SCHATTR_REGRESSION_TYPE, true, &pPoolItem ) == SfxItemState::SET )
        {
            eRegress = static_cast< const SvxChartRegressItem * >( pPoolItem )->GetValue();
            bEquationExists = ( eRegress != CHREGRESS_NONE );
        }
    }

    if( bEquationExists )
    {
        uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropSet, uno::UNO_QUERY );
        uno::Reference< chart2::XRegressionCurve > xCurve(
            ::chart::RegressionCurveHelper::getFirstCurveNotMeanValueLine( xRegCnt ));
        if( xCurve.is())
        {
            return xCurve->getEquationProperties();
        }
    }

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< beans::XPropertySet > lcl_getCurveProperties(
    const uno::Reference< beans::XPropertySet > & xSeriesPropSet, const SfxItemSet * pItemSet )
{
    bool bExists = true;

    // ensure that a trendline is on
    if( pItemSet )
    {
        SvxChartRegress eRegress = CHREGRESS_NONE;
        const SfxPoolItem *pPoolItem = nullptr;
        if( pItemSet->GetItemState( SCHATTR_REGRESSION_TYPE, true, &pPoolItem ) == SfxItemState::SET )
        {
            eRegress = static_cast< const SvxChartRegressItem * >( pPoolItem )->GetValue();
            bExists = ( eRegress != CHREGRESS_NONE );
        }
    }

    if( bExists )
    {
        uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropSet, uno::UNO_QUERY );
        uno::Reference< chart2::XRegressionCurve > xCurve(
            ::chart::RegressionCurveHelper::getFirstCurveNotMeanValueLine( xRegCnt ));
        if( xCurve.is())
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            return xProperties;
        }
    }

    return uno::Reference< beans::XPropertySet >();
}

template <class T, class D>
bool lclConvertToPropertySet(const SfxItemSet& rItemSet, sal_uInt16 nWhichId, uno::Reference<beans::XPropertySet> xProperties, const OUString& aPropertyID)
{
    OSL_ASSERT(xProperties.is());
    if( xProperties.is() )
    {
        T aValue = static_cast<T>(static_cast<const D&>(rItemSet.Get( nWhichId )).GetValue());
        T aOldValue = aValue;
        bool aSuccess = xProperties->getPropertyValue( aPropertyID ) >>= aOldValue;
        if (!aSuccess || aOldValue != aValue)
        {
            xProperties->setPropertyValue( aPropertyID , uno::makeAny( aValue ));
            return true;
        }
    }
    return false;
}

template <class T, class D>
void lclConvertToItemSet(SfxItemSet& rItemSet, sal_uInt16 nWhichId, uno::Reference<beans::XPropertySet> xProperties, const OUString& aPropertyID)
{
    OSL_ASSERT(xProperties.is());
    if( xProperties.is() )
    {
        T aValue = static_cast<T>(static_cast<const D&>(rItemSet.Get( nWhichId )).GetValue());
        if(xProperties->getPropertyValue( aPropertyID ) >>= aValue)
        {
            rItemSet.Put(D( nWhichId, aValue ));
        }
    }
}

void lclConvertToItemSetDouble(SfxItemSet& rItemSet, sal_uInt16 nWhichId, uno::Reference<beans::XPropertySet> xProperties, const OUString& aPropertyID)
{
    OSL_ASSERT(xProperties.is());
    if( xProperties.is() )
    {
        double aValue = static_cast<double>(static_cast<const SvxDoubleItem&>(rItemSet.Get( nWhichId )).GetValue());
        if(xProperties->getPropertyValue( aPropertyID ) >>= aValue)
        {
            rItemSet.Put(SvxDoubleItem( aValue, nWhichId ));
        }
    }
}

} // anonymous namespace

namespace chart
{
namespace wrapper
{

StatisticsItemConverter::StatisticsItemConverter(
    const uno::Reference< frame::XModel > & xModel,
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_xModel( xModel )
{
}

StatisticsItemConverter::~StatisticsItemConverter()
{
}

const sal_uInt16 * StatisticsItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nStatWhichPairs;
}

bool StatisticsItemConverter::GetItemProperty(
    tWhichIdType /* nWhichId */,
    tPropertyNameWithMemberId & /* rOutProperty */ ) const
{
    return false;
}

bool StatisticsItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_STAT_AVERAGE:
        {
            uno::Reference< chart2::XRegressionCurveContainer > xRegCnt(
                GetPropertySet(), uno::UNO_QUERY );
            bool bOldHasMeanValueLine = RegressionCurveHelper::hasMeanValueLine( xRegCnt );

            bool bNewHasMeanValueLine =
                static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();

            if( bOldHasMeanValueLine != bNewHasMeanValueLine )
            {
                if( ! bNewHasMeanValueLine )
                    RegressionCurveHelper::removeMeanValueLine( xRegCnt );
                else
                    RegressionCurveHelper::addMeanValueLine(
                        xRegCnt, uno::Reference< uno::XComponentContext >(), GetPropertySet() );
                bChanged = true;
            }
        }
        break;

        // Attention !!! This case must be passed before SCHATTR_STAT_PERCENT,
        // SCHATTR_STAT_BIGERROR, SCHATTR_STAT_CONSTPLUS,
        // SCHATTR_STAT_CONSTMINUS and SCHATTR_STAT_INDICATE
        case SCHATTR_STAT_KIND_ERROR:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();

            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetErrorBar( GetPropertySet(), bYError ));

            SvxChartKindError eErrorKind =
                static_cast< const SvxChartKindErrorItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            if( !xErrorBarProp.is() && eErrorKind == CHERROR_NONE)
            {
                //nothing to do
            }
            else
            {
                sal_Int32 nStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;

                switch( eErrorKind )
                {
                    case CHERROR_NONE:
                        nStyle = ::com::sun::star::chart::ErrorBarStyle::NONE; break;
                    case CHERROR_VARIANT:
                        nStyle = ::com::sun::star::chart::ErrorBarStyle::VARIANCE; break;
                    case CHERROR_SIGMA:
                        nStyle = ::com::sun::star::chart::ErrorBarStyle::STANDARD_DEVIATION; break;
                    case CHERROR_PERCENT:
                        nStyle = ::com::sun::star::chart::ErrorBarStyle::RELATIVE; break;
                    case CHERROR_BIGERROR:
                        nStyle = ::com::sun::star::chart::ErrorBarStyle::ERROR_MARGIN; break;
                    case CHERROR_CONST:
                        nStyle = ::com::sun::star::chart::ErrorBarStyle::ABSOLUTE; break;
                    case CHERROR_STDERROR:
                        nStyle = ::com::sun::star::chart::ErrorBarStyle::STANDARD_ERROR; break;
                    case CHERROR_RANGE:
                        nStyle = ::com::sun::star::chart::ErrorBarStyle::FROM_DATA; break;
                }

                if( !xErrorBarProp.is() )
                {
                    xErrorBarProp = lcl_GetDefaultErrorBar();
                    GetPropertySet()->setPropertyValue( bYError ? OUString(CHART_UNONAME_ERRORBAR_Y) : OUString(CHART_UNONAME_ERRORBAR_X),
                                                        uno::makeAny( xErrorBarProp ));
                }

                xErrorBarProp->setPropertyValue( "ErrorBarStyle" , uno::makeAny( nStyle ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_PERCENT:
        case SCHATTR_STAT_BIGERROR:
        {
            OSL_FAIL( "Deprecated item" );
            bool bYError =
                static_cast<const SfxBoolItem&>(rItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();

            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetErrorBar( GetPropertySet(), bYError));
            bool bOldHasErrorBar = xErrorBarProp.is();

            double fValue =
                static_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( xErrorBarProp, fPos, fNeg );

            if( bOldHasErrorBar &&
                ! ( ::rtl::math::approxEqual( fPos, fValue ) &&
                    ::rtl::math::approxEqual( fNeg, fValue )))
            {
                xErrorBarProp->setPropertyValue( "PositiveError" , uno::makeAny( fValue ));
                xErrorBarProp->setPropertyValue( "NegativeError" , uno::makeAny( fValue ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTPLUS:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();

            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetErrorBar( GetPropertySet(),bYError));
            bool bOldHasErrorBar = xErrorBarProp.is();

            double fValue =
                static_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( xErrorBarProp, fPos, fNeg );

            if( bOldHasErrorBar &&
                ! ::rtl::math::approxEqual( fPos, fValue ))
            {
                xErrorBarProp->setPropertyValue( "PositiveError" , uno::makeAny( fValue ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTMINUS:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetErrorBar( GetPropertySet(),bYError));
            bool bOldHasErrorBar = xErrorBarProp.is();

            double fValue =
                static_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( xErrorBarProp, fPos, fNeg );

            if( bOldHasErrorBar &&
                ! ::rtl::math::approxEqual( fNeg, fValue ))
            {
                xErrorBarProp->setPropertyValue( "NegativeError" , uno::makeAny( fValue ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_REGRESSION_TYPE:
        {
            SvxChartRegress eRegress =
                static_cast< const SvxChartRegressItem& >(
                    rItemSet.Get( nWhichId )).GetValue();

            uno::Reference< chart2::XRegressionCurve > xCurve( GetPropertySet(), uno::UNO_QUERY );
            uno::Reference< chart2::XRegressionCurveContainer > xContainer( GetPropertySet(), uno::UNO_QUERY );

            if( eRegress == CHREGRESS_NONE )
            {
                if ( xContainer.is() )
                {
                    xContainer->removeRegressionCurve( xCurve );
                    bChanged = true;
                }
            }
            else
            {
                if ( xCurve.is() )
                {
                    SvxChartRegress eOldRegress(
                                RegressionCurveHelper::getRegressionType(xCurve));

                    if( eOldRegress != eRegress )
                    {
                        xCurve = RegressionCurveHelper::changeRegressionCurveType(
                                                eRegress,
                                                xContainer,
                                                xCurve,
                                                uno::Reference< uno::XComponentContext >());
                        uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
                        resetPropertySet( xProperties );
                        bChanged = true;
                    }
                }
            }
        }
        break;

        case SCHATTR_REGRESSION_DEGREE:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), &rItemSet ));
            bChanged = lclConvertToPropertySet<sal_Int32, SfxInt32Item>(rItemSet, nWhichId, xProperties, "PolynomialDegree");
        }
        break;

        case SCHATTR_REGRESSION_PERIOD:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), &rItemSet ));
            bChanged = lclConvertToPropertySet<sal_Int32, SfxInt32Item>(rItemSet, nWhichId, xProperties, "MovingAveragePeriod");
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), &rItemSet ));
            bChanged = lclConvertToPropertySet<double, SvxDoubleItem>(rItemSet, nWhichId, xProperties, "ExtrapolateForward");
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), &rItemSet ));
            bChanged = lclConvertToPropertySet<double, SvxDoubleItem>(rItemSet, nWhichId, xProperties, "ExtrapolateBackward");
        }
        break;

        case SCHATTR_REGRESSION_SET_INTERCEPT:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), &rItemSet ));
            bChanged = lclConvertToPropertySet<sal_Bool, SfxBoolItem>(rItemSet, nWhichId, xProperties, "ForceIntercept");
        }
        break;

        case SCHATTR_REGRESSION_INTERCEPT_VALUE:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), &rItemSet ));
            bChanged = lclConvertToPropertySet<double, SvxDoubleItem>(rItemSet, nWhichId, xProperties, "InterceptValue");
        }
        break;

        case SCHATTR_REGRESSION_CURVE_NAME:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), &rItemSet ));
            bChanged = lclConvertToPropertySet<OUString, SfxStringItem>(rItemSet, nWhichId, xProperties, "CurveName");
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            uno::Reference< beans::XPropertySet > xEqProp( lcl_getEquationProperties( GetPropertySet(), &rItemSet ));
            bChanged = lclConvertToPropertySet<sal_Bool, SfxBoolItem>(rItemSet, nWhichId, xEqProp, "ShowEquation");
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            uno::Reference< beans::XPropertySet > xEqProp( lcl_getEquationProperties( GetPropertySet(), &rItemSet ));
            bChanged = lclConvertToPropertySet<sal_Bool, SfxBoolItem>(rItemSet, nWhichId, xEqProp, "ShowCorrelationCoefficient");
        }
        break;

        case SCHATTR_STAT_INDICATE:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetErrorBar( GetPropertySet(),bYError));
            bool bOldHasErrorBar = xErrorBarProp.is();

            SvxChartIndicate eIndicate =
                static_cast< const SvxChartIndicateItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            bool bNewIndPos = (eIndicate == CHINDICATE_BOTH || eIndicate == CHINDICATE_UP );
            bool bNewIndNeg = (eIndicate == CHINDICATE_BOTH || eIndicate == CHINDICATE_DOWN );

            bool bShowPos(false), bShowNeg(false);
            lcl_getErrorIndicatorValues( xErrorBarProp, bShowPos, bShowNeg );

            if( bOldHasErrorBar &&
                ( bShowPos != bNewIndPos ||
                  bShowNeg != bNewIndNeg ))
            {
                xErrorBarProp->setPropertyValue( "ShowPositiveError" , uno::makeAny( bNewIndPos ));
                xErrorBarProp->setPropertyValue( "ShowNegativeError" , uno::makeAny( bNewIndNeg ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_RANGE_POS:
        case SCHATTR_STAT_RANGE_NEG:
        {
            const bool bYError =
                static_cast<const SfxBoolItem&>(rItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            uno::Reference< chart2::data::XDataSource > xErrorBarSource( lcl_GetErrorBar( GetPropertySet(), bYError),
                                                                         uno::UNO_QUERY );
            uno::Reference< chart2::XChartDocument > xChartDoc( m_xModel, uno::UNO_QUERY );
            uno::Reference< chart2::data::XDataProvider > xDataProvider;

            if( xChartDoc.is())
                xDataProvider.set( xChartDoc->getDataProvider());
            if( xErrorBarSource.is() && xDataProvider.is())
            {
                OUString aNewRange( static_cast< const SfxStringItem & >( rItemSet.Get( nWhichId )).GetValue());
                bool bApplyNewRange = false;

                bool bIsPositiveValue( nWhichId == SCHATTR_STAT_RANGE_POS );
                if( xChartDoc->hasInternalDataProvider())
                {
                    if( !aNewRange.isEmpty())
                    {
                        uno::Reference< chart2::data::XDataSequence > xSeq(
                            StatisticsHelper::getErrorDataSequenceFromDataSource(
                                xErrorBarSource, bIsPositiveValue, bYError ));
                        if( ! xSeq.is())
                        {
                            // no data range for error bars yet => create
                            uno::Reference< chart2::XInternalDataProvider > xIntDataProvider( xDataProvider, uno::UNO_QUERY );
                            OSL_ASSERT( xIntDataProvider.is());
                            if( xIntDataProvider.is())
                            {
                                xIntDataProvider->appendSequence();
                                aNewRange = "last";
                                bApplyNewRange = true;
                            }
                        }
                    }
                }
                else
                {
                    uno::Reference< chart2::data::XDataSequence > xSeq(
                        StatisticsHelper::getErrorDataSequenceFromDataSource(
                            xErrorBarSource, bIsPositiveValue, bYError ));
                    bApplyNewRange =
                        ! ( xSeq.is() && (aNewRange == xSeq->getSourceRangeRepresentation()));
                }

                if( bApplyNewRange )
                    StatisticsHelper::setErrorDataSequence(
                        xErrorBarSource, xDataProvider, aNewRange, bIsPositiveValue, bYError );
            }
        }
        break;
    }

    return bChanged;
}

void StatisticsItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        case SCHATTR_STAT_AVERAGE:
            rOutItemSet.Put(
                SfxBoolItem( nWhichId,
                             RegressionCurveHelper::hasMeanValueLine(
                                 uno::Reference< chart2::XRegressionCurveContainer >(
                                     GetPropertySet(), uno::UNO_QUERY ))));
            break;

        case SCHATTR_STAT_KIND_ERROR:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rOutItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            SvxChartKindError eErrorKind = CHERROR_NONE;
            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetErrorBar( GetPropertySet(), bYError));
            if( xErrorBarProp.is() )
            {
                sal_Int32 nStyle = 0;
                if( xErrorBarProp->getPropertyValue( "ErrorBarStyle" ) >>= nStyle )
                {
                    switch( nStyle )
                    {
                        case ::com::sun::star::chart::ErrorBarStyle::NONE:
                            break;
                        case ::com::sun::star::chart::ErrorBarStyle::VARIANCE:
                            eErrorKind = CHERROR_VARIANT; break;
                        case ::com::sun::star::chart::ErrorBarStyle::STANDARD_DEVIATION:
                            eErrorKind = CHERROR_SIGMA; break;
                        case ::com::sun::star::chart::ErrorBarStyle::ABSOLUTE:
                            eErrorKind = CHERROR_CONST; break;
                        case ::com::sun::star::chart::ErrorBarStyle::RELATIVE:
                            eErrorKind = CHERROR_PERCENT; break;
                        case ::com::sun::star::chart::ErrorBarStyle::ERROR_MARGIN:
                            eErrorKind = CHERROR_BIGERROR; break;
                        case ::com::sun::star::chart::ErrorBarStyle::STANDARD_ERROR:
                            eErrorKind = CHERROR_STDERROR; break;
                        case ::com::sun::star::chart::ErrorBarStyle::FROM_DATA:
                            eErrorKind = CHERROR_RANGE; break;
                    }
                }
            }
            rOutItemSet.Put( SvxChartKindErrorItem( eErrorKind, SCHATTR_STAT_KIND_ERROR ));
        }
        break;

        case SCHATTR_STAT_PERCENT:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rOutItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetErrorBar( GetPropertySet(),bYError));
            if( xErrorBarProp.is())
            {
                double fPos(0.0), fNeg(0.0);
                lcl_getErrorValues( xErrorBarProp, fPos, fNeg );
                rOutItemSet.Put( SvxDoubleItem( ( fPos + fNeg ) / 2.0, nWhichId ));
            }
        }
        break;

        case SCHATTR_STAT_BIGERROR:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rOutItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetErrorBar( GetPropertySet(),bYError));
            if( xErrorBarProp.is())
            {
                double fPos(0.0), fNeg(0.0);
                lcl_getErrorValues( xErrorBarProp, fPos, fNeg );
                rOutItemSet.Put( SvxDoubleItem( ( fPos + fNeg ) / 2.0, nWhichId ));
            }
        }
        break;

        case SCHATTR_STAT_CONSTPLUS:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rOutItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetErrorBar( GetPropertySet(),bYError));
            if( xErrorBarProp.is())
            {
                double fPos(0.0), fNeg(0.0);
                lcl_getErrorValues( xErrorBarProp, fPos, fNeg );
                rOutItemSet.Put( SvxDoubleItem( fPos, nWhichId ));
            }
        }
        break;

        case SCHATTR_STAT_CONSTMINUS:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rOutItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetErrorBar( GetPropertySet(),bYError));
            if( xErrorBarProp.is())
            {
                double fPos(0.0), fNeg(0.0);
                lcl_getErrorValues( xErrorBarProp, fPos, fNeg );
                rOutItemSet.Put( SvxDoubleItem( fNeg, nWhichId ));
            }
        }
        break;

        case SCHATTR_REGRESSION_TYPE:
        {
            SvxChartRegress eRegress =
                    RegressionCurveHelper::getFirstRegressTypeNotMeanValueLine(
                        uno::Reference< chart2::XRegressionCurveContainer >(
                            GetPropertySet(), uno::UNO_QUERY ) );
            rOutItemSet.Put( SvxChartRegressItem( eRegress, SCHATTR_REGRESSION_TYPE ));
        }
        break;

        case SCHATTR_REGRESSION_DEGREE:
        {

            uno::Reference<beans::XPropertySet> xProperties( lcl_getCurveProperties( GetPropertySet(), nullptr ));
            lclConvertToItemSet<sal_Int32, SfxInt32Item>(rOutItemSet, nWhichId, xProperties, "PolynomialDegree");
        }
        break;

        case SCHATTR_REGRESSION_PERIOD:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), nullptr ));
            lclConvertToItemSet<sal_Int32, SfxInt32Item>(rOutItemSet, nWhichId, xProperties, "MovingAveragePeriod");
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), nullptr ));
            lclConvertToItemSetDouble(rOutItemSet, nWhichId, xProperties, "ExtrapolateForward");
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), nullptr ));
            lclConvertToItemSetDouble(rOutItemSet, nWhichId, xProperties, "ExtrapolateBackward");
        }
        break;

        case SCHATTR_REGRESSION_SET_INTERCEPT:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), nullptr ));
            lclConvertToItemSet<sal_Bool, SfxBoolItem>(rOutItemSet, nWhichId, xProperties, "ForceIntercept");
        }
        break;

        case SCHATTR_REGRESSION_INTERCEPT_VALUE:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), nullptr ));
            lclConvertToItemSetDouble(rOutItemSet, nWhichId, xProperties, "InterceptValue");
        }
        break;

        case SCHATTR_REGRESSION_CURVE_NAME:
        {
            uno::Reference< beans::XPropertySet > xProperties( lcl_getCurveProperties( GetPropertySet(), nullptr ));
            lclConvertToItemSet<OUString, SfxStringItem>(rOutItemSet, nWhichId, xProperties, "CurveName");
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            uno::Reference< beans::XPropertySet > xEqProp( lcl_getEquationProperties( GetPropertySet(), nullptr ));
            lclConvertToItemSet<sal_Bool, SfxBoolItem>(rOutItemSet, nWhichId, xEqProp, "ShowEquation");
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            uno::Reference< beans::XPropertySet > xEqProp( lcl_getEquationProperties( GetPropertySet(), nullptr ));
            lclConvertToItemSet<sal_Bool, SfxBoolItem>(rOutItemSet, nWhichId, xEqProp, "ShowCorrelationCoefficient");
        }
        break;

        case SCHATTR_STAT_INDICATE:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rOutItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetErrorBar( GetPropertySet(),bYError));
            SvxChartIndicate eIndicate = CHINDICATE_BOTH;
            if( xErrorBarProp.is())
            {
                bool bShowPos(false), bShowNeg(false);
                lcl_getErrorIndicatorValues( xErrorBarProp, bShowPos, bShowNeg );

                if( bShowPos )
                {
                    if( bShowNeg )
                        eIndicate = CHINDICATE_BOTH;
                    else
                        eIndicate = CHINDICATE_UP;
                }
                else
                {
                    if( bShowNeg )
                        eIndicate = CHINDICATE_DOWN;
                    else
                        eIndicate = CHINDICATE_NONE;
                }
            }
            rOutItemSet.Put( SvxChartIndicateItem( eIndicate, SCHATTR_STAT_INDICATE ));
        }
        break;

        case SCHATTR_STAT_RANGE_POS:
        case SCHATTR_STAT_RANGE_NEG:
        {
            bool bYError =
                static_cast<const SfxBoolItem&>(rOutItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE)).GetValue();
            uno::Reference< chart2::data::XDataSource > xErrorBarSource( lcl_GetErrorBar( GetPropertySet(),bYError),
                                                                         uno::UNO_QUERY );
            if( xErrorBarSource.is())
            {
                uno::Reference< chart2::data::XDataSequence > xSeq(
                    StatisticsHelper::getErrorDataSequenceFromDataSource(
                        xErrorBarSource, (nWhichId == SCHATTR_STAT_RANGE_POS), bYError ));
                if( xSeq.is())
                    rOutItemSet.Put( SfxStringItem( nWhichId, xSeq->getSourceRangeRepresentation()));
            }
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
