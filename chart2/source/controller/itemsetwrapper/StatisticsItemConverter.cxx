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
        ( xProp->getPropertyValue( bYError ? OUString( "ErrorBarY" ) : OUString("ErrorBarX") ) >>= xResult );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }

    return xResult;
}

::chart::RegressionCurveHelper::tRegressionType lcl_convertRegressionType( SvxChartRegress eRegress )
{
    ::chart::RegressionCurveHelper::tRegressionType eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_NONE;
    switch( eRegress )
    {
        case CHREGRESS_LINEAR:
            eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_LINEAR;
            break;
        case CHREGRESS_LOG:
            eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_LOG;
            break;
        case CHREGRESS_EXP:
            eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_EXP;
            break;
        case CHREGRESS_POWER:
            eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_POWER;
            break;
        case CHREGRESS_NONE:
            break;
    }
    return eType;
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
        const SfxPoolItem *pPoolItem = NULL;
        if( pItemSet->GetItemState( SCHATTR_REGRESSION_TYPE, sal_True, &pPoolItem ) == SFX_ITEM_SET )
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
    OSL_ASSERT( static_cast< int >( RegressionCurveHelper::REGRESSION_TYPE_NONE ) ==
                static_cast< int >( CHREGRESS_NONE ));
    OSL_ASSERT( static_cast< int >( RegressionCurveHelper::REGRESSION_TYPE_LINEAR ) ==
                static_cast< int >( CHREGRESS_LINEAR ));
    OSL_ASSERT( static_cast< int >( RegressionCurveHelper::REGRESSION_TYPE_LOG ) ==
                static_cast< int >( CHREGRESS_LOG ));
    OSL_ASSERT( static_cast< int >( RegressionCurveHelper::REGRESSION_TYPE_EXP ) ==
                static_cast< int >( CHREGRESS_EXP ));
    OSL_ASSERT( static_cast< int >( RegressionCurveHelper::REGRESSION_TYPE_POWER ) ==
                static_cast< int >( CHREGRESS_POWER ));
}

StatisticsItemConverter::~StatisticsItemConverter()
{}

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
    uno::Any aValue;

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
                    GetPropertySet()->setPropertyValue( bYError ? OUString( "ErrorBarY" ) : OUString("ErrorBarX"),
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
            double fPos, fNeg;
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
            double fPos, fNeg;
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
            double fPos, fNeg;
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
                static_cast< const SvxChartRegressItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            uno::Reference< chart2::XRegressionCurveContainer > xRegCnt(
                GetPropertySet(), uno::UNO_QUERY );

            if( eRegress == CHREGRESS_NONE )
            {
                bChanged = RegressionCurveHelper::removeAllExceptMeanValueLine( xRegCnt );
            }
            else
            {
                SvxChartRegress eOldRegress(
                    static_cast< SvxChartRegress >(
                        static_cast< sal_Int32 >(
                            RegressionCurveHelper::getFirstRegressTypeNotMeanValueLine( xRegCnt ))));
                if( eOldRegress != eRegress )
                {
                    RegressionCurveHelper::replaceOrAddCurveAndReduceToOne(
                        lcl_convertRegressionType( eRegress ), xRegCnt,
                        uno::Reference< uno::XComponentContext >());
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            uno::Reference< beans::XPropertySet > xEqProp( lcl_getEquationProperties( GetPropertySet(), &rItemSet ));
            if( xEqProp.is())
            {
                bool bShowEq = false;
                xEqProp->getPropertyValue( "ShowEquation" ) >>= bShowEq;
                bool bNewShowEq =
                    static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();
                if( bShowEq != bNewShowEq )
                {
                    xEqProp->setPropertyValue( "ShowEquation" , uno::makeAny( bNewShowEq ));
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            uno::Reference< beans::XPropertySet > xEqProp( lcl_getEquationProperties( GetPropertySet(), &rItemSet ));
            if( xEqProp.is())
            {
                bool bShowCoeff = false;
                xEqProp->getPropertyValue( "ShowCorrelationCoefficient" ) >>= bShowCoeff;
                bool bNewShowCoeff =
                    static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();
                if( bShowCoeff != bNewShowCoeff )
                {
                    xEqProp->setPropertyValue( "ShowCorrelationCoefficient" , uno::makeAny( bNewShowCoeff ));
                    bChanged = true;
                }
            }
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

            bool bShowPos, bShowNeg;
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
                double fPos, fNeg;
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
                double fPos, fNeg;
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
                double fPos, fNeg;
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
                double fPos, fNeg;
                lcl_getErrorValues( xErrorBarProp, fPos, fNeg );
                rOutItemSet.Put( SvxDoubleItem( fNeg, nWhichId ));
            }
        }
        break;

        case SCHATTR_REGRESSION_TYPE:
        {
            SvxChartRegress eRegress = static_cast< SvxChartRegress >(
                static_cast< sal_Int32 >(
                    RegressionCurveHelper::getFirstRegressTypeNotMeanValueLine(
                        uno::Reference< chart2::XRegressionCurveContainer >(
                            GetPropertySet(), uno::UNO_QUERY ) )));
            rOutItemSet.Put( SvxChartRegressItem( eRegress, SCHATTR_REGRESSION_TYPE ));
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            bool bShowEq = false;
            uno::Reference< beans::XPropertySet > xEqProp( lcl_getEquationProperties( GetPropertySet(), 0 ));
            if( xEqProp.is())
                xEqProp->getPropertyValue( "ShowEquation" ) >>= bShowEq;
            rOutItemSet.Put( SfxBoolItem( nWhichId, bShowEq ));
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            bool bShowCoeff = false;
            uno::Reference< beans::XPropertySet > xEqProp( lcl_getEquationProperties( GetPropertySet(), 0 ));
            if( xEqProp.is())
                xEqProp->getPropertyValue( "ShowCorrelationCoefficient" ) >>= bShowCoeff;
            rOutItemSet.Put( SfxBoolItem( nWhichId, bShowCoeff ));
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
                bool bShowPos, bShowNeg;
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
                    rOutItemSet.Put( SfxStringItem( nWhichId, String( xSeq->getSourceRangeRepresentation())));
            }
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
