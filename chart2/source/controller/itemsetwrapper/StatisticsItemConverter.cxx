/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StatisticsItemConverter.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:30:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "StatisticsItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "RegressionCurveHelper.hxx"
#include "ErrorBar.hxx"
#include "PropertyHelper.hxx"
#include "ChartModelHelper.hxx"
#include "ChartTypeHelper.hxx"

#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"

#ifndef _SVX_CHRTITEM_HXX
#include "SchAllDefinesFor_svx_chrtitem.hxx"
#include <svx/chrtitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_DATAPOINTLABEL_HPP_
#include <com/sun/star/chart2/DataPointLabel.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XREGRESSIONCURVECONTAINER_HPP_
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_ERRORBARSTYLE_HPP_
#include <com/sun/star/chart2/ErrorBarStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

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

uno::Reference< chart2::XRegressionCurve > lcl_createRegressionCurve(
    SvxChartRegress eRegress,
    const uno::Reference< beans::XPropertySet > & xFormerProp,
    const uno::Reference< beans::XPropertySet > & xSeriesProp,
    const uno::Reference< frame::XModel > & xModel )
{
    uno::Reference< chart2::XRegressionCurve > xResult;
    ::rtl::OUString aServiceName;

    switch( eRegress )
    {
        case CHREGRESS_LINEAR:
            aServiceName = C2U( "com.sun.star.chart2.LinearRegressionCurve" );
            break;
        case CHREGRESS_LOG:
            aServiceName = C2U( "com.sun.star.chart2.LogarithmicRegressionCurve" );
            break;
        case CHREGRESS_EXP:
            aServiceName = C2U( "com.sun.star.chart2.ExponentialRegressionCurve" );
            break;
        case CHREGRESS_POWER:
            aServiceName = C2U( "com.sun.star.chart2.PotentialRegressionCurve" );
            break;

        case CHREGRESS_NONE:
            break;
    }

    if( aServiceName.getLength())
    {
        // todo: use a valid context
        xResult.set( ::chart::RegressionCurveHelper::createRegressionCurveByServiceName(
                         uno::Reference< uno::XComponentContext >(), aServiceName ));

        uno::Reference< beans::XPropertySet > xProp( xResult, uno::UNO_QUERY );
        if( xProp.is())
        {
            if( xFormerProp.is())
                ::chart::PropertyHelper::copyProperties( xFormerProp, xProp );
            else
            {
                if( xSeriesProp.is())
                {
                    xProp->setPropertyValue( C2U( "LineColor" ),
                                             xSeriesProp->getPropertyValue( C2U( "Color" )));
                }
                xProp->setPropertyValue( C2U( "LineWidth" ), uno::makeAny( sal_Int32( 100 )));
            }
        }
    }

    return xResult;
}

/** removes all regression curves that are of type linear, log, exp or pot. (not
    mean-value) and returns true, if anything was removed
 */
::std::pair< bool, uno::Reference< beans::XPropertySet > >
lcl_removeAllKnownRegressionCurves(
    const uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    ::std::pair< bool, uno::Reference< beans::XPropertySet > > aResult;
    aResult.first = false;
    ::std::vector< sal_Int32 > aDeleteIndexes;

    if( xRegCnt.is())
    {
        try
        {
            uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(
                xRegCnt->getRegressionCurves());
            for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
            {
                uno::Reference< lang::XServiceName > xServName( aCurves[i], uno::UNO_QUERY );
                if( xServName.is() &&
                    ( xServName->getServiceName().equals(
                          C2U( "com.sun.star.chart2.LinearRegressionCurve" )) ||
                      xServName->getServiceName().equals(
                          C2U( "com.sun.star.chart2.LogarithmicRegressionCurve" )) ||
                      xServName->getServiceName().equals(
                          C2U( "com.sun.star.chart2.ExponentialRegressionCurve" )) ||
                      xServName->getServiceName().equals(
                          C2U( "com.sun.star.chart2.PotentialRegressionCurve" ))
                        ))
                {
                    aDeleteIndexes.push_back( i );
                }
            }

            if( aDeleteIndexes.size() > 0 )
            {
                bool bGotFormerProp = false;
                for( ::std::vector< sal_Int32 >::const_iterator aIt = aDeleteIndexes.begin();
                     aIt != aDeleteIndexes.end(); ++aIt )
                {
                    if( ! bGotFormerProp )
                    {
                        aResult.second.set( aCurves[*aIt], uno::UNO_QUERY );
                        bGotFormerProp = true;
                    }
                    xRegCnt->removeRegressionCurve( aCurves[ *aIt ] );
                }
                aResult.first = true;
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return aResult;
}

uno::Reference< beans::XPropertySet > lcl_GetDefaultErrorBar()
{
    // todo: use a valid context
    return uno::Reference< beans::XPropertySet >(
        new ::chart::ErrorBar( uno::Reference< uno::XComponentContext >()));
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

const USHORT * StatisticsItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nStatWhichPairs;
}

bool StatisticsItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    return false;
}

bool StatisticsItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet )
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
                reinterpret_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();

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
            uno::Reference< beans::XPropertySet > xOldErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet() ));
            bool bOldHasErrorBar = xOldErrorBarProp.is();

            SvxChartKindError eErrorKind =
                reinterpret_cast< const SvxChartKindErrorItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            bool bNewHasErrorKind = (eErrorKind != CHERROR_NONE);

            if( bOldHasErrorBar && ! bNewHasErrorKind )
            {
                GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ), uno::Any());
                bChanged = true;
            }
            else
            {
                chart2::ErrorBarStyle eStyle;

                switch( eErrorKind )
                {
                    case CHERROR_VARIANT:
                        eStyle = chart2::ErrorBarStyle_VARIANCE; break;
                    case CHERROR_SIGMA:
                        eStyle = chart2::ErrorBarStyle_STANDARD_DEVIATION; break;
                    case CHERROR_PERCENT:
                        eStyle = chart2::ErrorBarStyle_RELATIVE; break;
                    case CHERROR_BIGERROR:
                        eStyle = chart2::ErrorBarStyle_ERROR_MARGIN; break;
                    case CHERROR_CONST:
                        eStyle = chart2::ErrorBarStyle_ABSOLUTE; break;

                    case CHERROR_NONE:
                        break;
                }

                if( eErrorKind != CHERROR_NONE )
                {
                    chart2::ErrorBarStyle eOldErrorBarStyle = chart2::ErrorBarStyle_VARIANCE;
                    if( bOldHasErrorBar )
                        xOldErrorBarProp->getPropertyValue( C2U( "ErrorBarStyle" )) >>= eOldErrorBarStyle;

                    if( ! bOldHasErrorBar ||
                        eOldErrorBarStyle != eStyle )
                    {
                        if( ! bOldHasErrorBar )
                            xOldErrorBarProp = lcl_GetDefaultErrorBar();
                        xOldErrorBarProp->setPropertyValue( C2U( "ErrorBarStyle" ),
                                                            uno::makeAny( eStyle ));
                        GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                            uno::makeAny( xOldErrorBarProp ));
                        bChanged = true;
                    }
                }
            }
        }
        break;

        case SCHATTR_STAT_PERCENT:
        case SCHATTR_STAT_BIGERROR:
        {
            uno::Reference< beans::XPropertySet > xOldErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet()));
            bool bOldHasErrorBar = xOldErrorBarProp.is();

            double fValue =
                reinterpret_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos, fNeg;
            lcl_getErrorValues( xOldErrorBarProp, fPos, fNeg );

            if( bOldHasErrorBar &&
                ! ( ::rtl::math::approxEqual( fPos, fValue ) &&
                    ::rtl::math::approxEqual( fNeg, fValue )))
            {
                xOldErrorBarProp->setPropertyValue( C2U( "PositiveError" ),
                                                    uno::makeAny( fValue ));
                xOldErrorBarProp->setPropertyValue( C2U( "NegativeError" ),
                                                    uno::makeAny( fValue ));
                GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                    uno::makeAny( xOldErrorBarProp ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTPLUS:
        {
            uno::Reference< beans::XPropertySet > xOldErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet()));
            bool bOldHasErrorBar = xOldErrorBarProp.is();

            double fValue =
                reinterpret_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos, fNeg;
            lcl_getErrorValues( xOldErrorBarProp, fPos, fNeg );

            if( bOldHasErrorBar &&
                ! ::rtl::math::approxEqual( fPos, fValue ))
            {
                xOldErrorBarProp->setPropertyValue( C2U( "PositiveError" ), uno::makeAny( fValue ));
                GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                    uno::makeAny( xOldErrorBarProp ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTMINUS:
        {
            uno::Reference< beans::XPropertySet > xOldErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet()));
            bool bOldHasErrorBar = xOldErrorBarProp.is();

            double fValue =
                reinterpret_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos, fNeg;
            lcl_getErrorValues( xOldErrorBarProp, fPos, fNeg );

            if( bOldHasErrorBar &&
                ! ::rtl::math::approxEqual( fNeg, fValue ))
            {
                xOldErrorBarProp->setPropertyValue( C2U( "NegativeError" ), uno::makeAny( fValue ));
                GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                    uno::makeAny( xOldErrorBarProp ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_REGRESSTYPE:
        {
            SvxChartRegress eRegress =
                reinterpret_cast< const SvxChartRegressItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            uno::Reference< chart2::XRegressionCurveContainer > xRegCnt(
                GetPropertySet(), uno::UNO_QUERY );

            if( eRegress == CHREGRESS_NONE )
            {
                bChanged = lcl_removeAllKnownRegressionCurves( xRegCnt ).first;
            }
            else
            {
                SvxChartRegress eOldRegress(
                    static_cast< SvxChartRegress >(
                        static_cast< sal_Int32 >(
                            RegressionCurveHelper::getRegressType( xRegCnt ))));
                if( eOldRegress != eRegress )
                {
                    uno::Reference< beans::XPropertySet > xFormerProp(
                        lcl_removeAllKnownRegressionCurves( xRegCnt ).second );
                    xRegCnt->addRegressionCurve(
                        lcl_createRegressionCurve( eRegress, xFormerProp, GetPropertySet(), m_xModel ));
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_STAT_INDICATE:
        {
            uno::Reference< beans::XPropertySet > xOldErrorBarProp(
                lcl_GetYErrorBar( GetPropertySet()));
            bool bOldHasErrorBar = xOldErrorBarProp.is();

            SvxChartIndicate eIndicate =
                reinterpret_cast< const SvxChartIndicateItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            bool bNewIndPos = (eIndicate == CHINDICATE_BOTH || eIndicate == CHINDICATE_UP );
            bool bNewIndNeg = (eIndicate == CHINDICATE_BOTH || eIndicate == CHINDICATE_DOWN );

            bool bShowPos, bShowNeg;
            lcl_getErrorIndicatorValues( xOldErrorBarProp, bShowPos, bShowNeg );

            if( bOldHasErrorBar &&
                ( bShowPos != bNewIndPos ||
                  bShowNeg != bNewIndNeg ))
            {
                xOldErrorBarProp->setPropertyValue( C2U( "ShowPositiveError" ), uno::makeAny( bNewIndPos ));
                xOldErrorBarProp->setPropertyValue( C2U( "ShowNegativeError" ), uno::makeAny( bNewIndNeg ));
                GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                    uno::makeAny( xOldErrorBarProp ));
                bChanged = true;
            }
        }
        break;
    }

    return bChanged;
}

void StatisticsItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
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
                chart2::ErrorBarStyle eStyle;
                if( xErrorBarProp->getPropertyValue( C2U( "ErrorBarStyle" )) >>= eStyle )
                {
                    switch( eStyle )
                    {
                        case chart2::ErrorBarStyle_VARIANCE:
                            eErrorKind = CHERROR_VARIANT; break;
                        case chart2::ErrorBarStyle_STANDARD_DEVIATION:
                            eErrorKind = CHERROR_SIGMA; break;
                        case chart2::ErrorBarStyle_ABSOLUTE:
                            eErrorKind = CHERROR_CONST; break;
                        case chart2::ErrorBarStyle_RELATIVE:
                            eErrorKind = CHERROR_PERCENT; break;
                        case chart2::ErrorBarStyle_ERROR_MARGIN:
                            eErrorKind = CHERROR_BIGERROR; break;
                        case chart2::ErrorBarStyle_STANDARD_ERROR:
                            // not yet available in UI
                            break;

                        case chart2::ErrorBarStyle_FROM_DATA:
                            // suppress warning
                        case chart2::ErrorBarStyle_MAKE_FIXED_SIZE:
                            break;
                    }
                }
            }
            rOutItemSet.Put( SvxChartKindErrorItem( eErrorKind ));
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

        case SCHATTR_STAT_REGRESSTYPE:
        {
            SvxChartRegress eRegress = static_cast< SvxChartRegress >(
                static_cast< sal_Int32 >(
                    RegressionCurveHelper::getRegressType(
                        uno::Reference< chart2::XRegressionCurveContainer >(
                            GetPropertySet(), uno::UNO_QUERY ) )));
            rOutItemSet.Put( SvxChartRegressItem( eRegress ));
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
            rOutItemSet.Put( SvxChartIndicateItem( eIndicate ));
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart
