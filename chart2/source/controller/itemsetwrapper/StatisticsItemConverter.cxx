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

uno::Reference< beans::XPropertySet > lcl_GetYErrorBar(
    const uno::Reference< beans::XPropertySet > & xProp )
{
    uno::Reference< beans::XPropertySet > xResult;

    if( xProp.is())
        try
        {
            ( xProp->getPropertyValue( C2U( "ErrorBarY" )) >>= xResult );
        }
        catch( uno::Exception & ex )
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
        xErrorBarProp->getPropertyValue( C2U( "PositiveError" )) >>= rOutPosError;
        xErrorBarProp->getPropertyValue( C2U( "NegativeError" )) >>= rOutNegError;
    }
    catch( uno::Exception & ex )
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
        xErrorBarProp->getPropertyValue( C2U( "ShowPositiveError" )) >>= rOutShowPosError;
        xErrorBarProp->getPropertyValue( C2U( "ShowNegativeError" )) >>= rOutShowNegError;
    }
    catch( uno::Exception & ex )
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
            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet() ));

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
                    GetPropertySet()->setPropertyValue(
                        C2U( "ErrorBarY" ), uno::makeAny( xErrorBarProp ));
                }

                xErrorBarProp->setPropertyValue( C2U( "ErrorBarStyle" ),
                                                    uno::makeAny( nStyle ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_PERCENT:
        case SCHATTR_STAT_BIGERROR:
        {
            OSL_ENSURE( false, "Deprectaed item" );
            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet()));
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
                xErrorBarProp->setPropertyValue( C2U( "PositiveError" ),
                                                    uno::makeAny( fValue ));
                xErrorBarProp->setPropertyValue( C2U( "NegativeError" ),
                                                    uno::makeAny( fValue ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTPLUS:
        {
            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet()));
            bool bOldHasErrorBar = xErrorBarProp.is();

            double fValue =
                static_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos, fNeg;
            lcl_getErrorValues( xErrorBarProp, fPos, fNeg );

            if( bOldHasErrorBar &&
                ! ::rtl::math::approxEqual( fPos, fValue ))
            {
                xErrorBarProp->setPropertyValue( C2U( "PositiveError" ), uno::makeAny( fValue ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTMINUS:
        {
            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet()));
            bool bOldHasErrorBar = xErrorBarProp.is();

            double fValue =
                static_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos, fNeg;
            lcl_getErrorValues( xErrorBarProp, fPos, fNeg );

            if( bOldHasErrorBar &&
                ! ::rtl::math::approxEqual( fNeg, fValue ))
            {
                xErrorBarProp->setPropertyValue( C2U( "NegativeError" ), uno::makeAny( fValue ));
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
                xEqProp->getPropertyValue( C2U("ShowEquation")) >>= bShowEq;
                bool bNewShowEq =
                    static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();
                if( bShowEq != bNewShowEq )
                {
                    xEqProp->setPropertyValue( C2U("ShowEquation"), uno::makeAny( bNewShowEq ));
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
                xEqProp->getPropertyValue( C2U("ShowCorrelationCoefficient")) >>= bShowCoeff;
                bool bNewShowCoeff =
                    static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();
                if( bShowCoeff != bNewShowCoeff )
                {
                    xEqProp->setPropertyValue( C2U("ShowCorrelationCoefficient"), uno::makeAny( bNewShowCoeff ));
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_STAT_INDICATE:
        {
            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet()));
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
                xErrorBarProp->setPropertyValue( C2U( "ShowPositiveError" ), uno::makeAny( bNewIndPos ));
                xErrorBarProp->setPropertyValue( C2U( "ShowNegativeError" ), uno::makeAny( bNewIndNeg ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_RANGE_POS:
        case SCHATTR_STAT_RANGE_NEG:
        {
            // @todo: also be able to deal with x-error bars
            const bool bYError = true;
            uno::Reference< chart2::data::XDataSource > xErrorBarSource( lcl_GetYErrorBar( GetPropertySet()), uno::UNO_QUERY );
            uno::Reference< chart2::XChartDocument > xChartDoc( m_xModel, uno::UNO_QUERY );
            uno::Reference< chart2::data::XDataProvider > xDataProvider;

            if( xChartDoc.is())
                xDataProvider.set( xChartDoc->getDataProvider());
            if( xErrorBarSource.is() && xDataProvider.is())
            {
                ::rtl::OUString aNewRange( static_cast< const SfxStringItem & >( rItemSet.Get( nWhichId )).GetValue());
                bool bApplyNewRange = false;

                bool bIsPositiveValue( nWhichId == SCHATTR_STAT_RANGE_POS );
                if( xChartDoc->hasInternalDataProvider())
                {
                    if( aNewRange.getLength())
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
                                aNewRange = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("last"));
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
                        ! ( xSeq.is() && aNewRange.equals( xSeq->getSourceRangeRepresentation()));
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
            SvxChartKindError eErrorKind = CHERROR_NONE;
            uno::Reference< beans::XPropertySet > xErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet()));
            if( xErrorBarProp.is() )
            {
                sal_Int32 nStyle = 0;
                if( xErrorBarProp->getPropertyValue( C2U( "ErrorBarStyle" )) >>= nStyle )
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
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetYErrorBar( GetPropertySet()));
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
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetYErrorBar( GetPropertySet()));
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
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetYErrorBar( GetPropertySet()));
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
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetYErrorBar( GetPropertySet()));
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
                xEqProp->getPropertyValue( C2U("ShowEquation")) >>= bShowEq;
            rOutItemSet.Put( SfxBoolItem( nWhichId, bShowEq ));
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            bool bShowCoeff = false;
            uno::Reference< beans::XPropertySet > xEqProp( lcl_getEquationProperties( GetPropertySet(), 0 ));
            if( xEqProp.is())
                xEqProp->getPropertyValue( C2U("ShowCorrelationCoefficient")) >>= bShowCoeff;
            rOutItemSet.Put( SfxBoolItem( nWhichId, bShowCoeff ));
        }
        break;

        case SCHATTR_STAT_INDICATE:
        {
            uno::Reference< beans::XPropertySet > xErrorBarProp( lcl_GetYErrorBar( GetPropertySet()));
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
            uno::Reference< chart2::data::XDataSource > xErrorBarSource( lcl_GetYErrorBar( GetPropertySet()), uno::UNO_QUERY );
            if( xErrorBarSource.is())
            {
                uno::Reference< chart2::data::XDataSequence > xSeq(
                    StatisticsHelper::getErrorDataSequenceFromDataSource(
                        xErrorBarSource, (nWhichId == SCHATTR_STAT_RANGE_POS) /*, true */ /* y */ ));
                if( xSeq.is())
                    rOutItemSet.Put( SfxStringItem( nWhichId, String( xSeq->getSourceRangeRepresentation())));
            }
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart
