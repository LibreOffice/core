/*************************************************************************
 *
 *  $RCSfile: StatisticsItemConverter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: bm $ $Date: 2003-12-10 17:08:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "StatisticsItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "RegressionCurveHelper.hxx"

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

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_DATAPOINTLABEL_HPP_
#include <drafts/com/sun/star/chart2/DataPointLabel.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XREGRESSIONCURVECONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XRegressionCurveContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_ERRORBAR_HPP_
#include <drafts/com/sun/star/chart2/ErrorBar.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

#include <functional>
#include <algorithm>
#include <vector>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{
bool lcl_HasMeanValueLine( const uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    bool bResult = false;

    if( !xRegCnt.is())
        return bResult;

    try
    {
        uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(
            xRegCnt->getRegressionCurves());
        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            uno::Reference< lang::XServiceName > xServName( aCurves[i], uno::UNO_QUERY );
            if( xServName.is() &&
                xServName->getServiceName().equals(
                    C2U( "com.sun.star.comp.chart2.MeanValueRegressionCurve" )))
            {
                bResult = true;
                break;
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return bResult;
}

void lcl_RemoveMeanValueLine( uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    if( !xRegCnt.is())
        return;
    OSL_ASSERT( lcl_HasMeanValueLine( xRegCnt ));

    try
    {
        uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(
            xRegCnt->getRegressionCurves());
        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            uno::Reference< lang::XServiceName > xServName( aCurves[i], uno::UNO_QUERY );
            if( xServName.is() &&
                xServName->getServiceName().equals(
                    C2U( "com.sun.star.comp.chart2.MeanValueRegressionCurve" )))
            {
                // note: assume that there is only one mean-value curve
                xRegCnt->removeRegressionCurve( aCurves[i] );
                // attention: the iterator i has become invalid now
                break;
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void lcl_AddMeanValueLine( uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    if( !xRegCnt.is())
        return;
    OSL_ASSERT( ! lcl_HasMeanValueLine( xRegCnt ));

    // todo: use a valid context
    xRegCnt->addRegressionCurve( ::chart::RegressionCurveHelper::createMeanValueLine(
                                     uno::Reference< uno::XComponentContext >() ));
}

bool lcl_GetYErrorBar(
    const uno::Reference< beans::XPropertySet > & xProp,
    chart2::ErrorBar & rOutErrorBar )
{
    bool bResult = false;

    if( xProp.is())
        try
        {
            bResult = ( xProp->getPropertyValue( C2U( "ErrorBarY" )) >>= rOutErrorBar );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }

    return bResult;
}

bool lcl_getRegressType( const uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt,
                         SvxChartRegress & rOutRegress )
{
    bool bResult = false;

    if( xRegCnt.is())
    {
        try
        {
            uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(
                xRegCnt->getRegressionCurves());
            for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
            {
                uno::Reference< lang::XServiceName > xServName( aCurves[i], uno::UNO_QUERY );
                if( xServName.is())
                {
                    ::rtl::OUString aServiceName( xServName->getServiceName() );

                    // note: take first regression curve that matches any known
                    // type (except mean-value line)
                    if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                "com.sun.star.comp.chart2.LinearRegressionCurve" )))
                    {
                        rOutRegress = CHREGRESS_LINEAR;
                        bResult = true;
                        break;
                    }
                    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                "com.sun.star.comp.chart2.LogarithmicRegressionCurve" )))
                    {
                        rOutRegress = CHREGRESS_LOG;
                        bResult = true;
                        break;
                    }
                    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                "com.sun.star.comp.chart2.ExponentialRegressionCurve" )))
                    {
                        rOutRegress = CHREGRESS_EXP;
                        bResult = true;
                        break;
                    }
                    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                "com.sun.star.comp.chart2.PotentialRegressionCurve" )))
                    {
                        rOutRegress = CHREGRESS_POWER;
                        bResult = true;
                        break;
                    }
                }
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return bResult;
}

uno::Reference< chart2::XRegressionCurve > lcl_createRegressionCurve( SvxChartRegress eRegress )
{
    uno::Reference< chart2::XRegressionCurve > xResult;
    ::rtl::OUString aServiceName;

    switch( eRegress )
    {
        case CHREGRESS_LINEAR:
            aServiceName = C2U( "com.sun.star.comp.chart2.LinearRegressionCurve" );
            break;
        case CHREGRESS_LOG:
            aServiceName = C2U( "com.sun.star.comp.chart2.LogarithmicRegressionCurve" );
            break;
        case CHREGRESS_EXP:
            aServiceName = C2U( "com.sun.star.comp.chart2.ExponentialRegressionCurve" );
            break;
        case CHREGRESS_POWER:
            aServiceName = C2U( "com.sun.star.comp.chart2.PotentialRegressionCurve" );
            break;

        case CHREGRESS_NONE:
            break;
    }

    if( aServiceName.getLength())
    {
        // todo: use a valid context
        xResult.set( ::chart::RegressionCurveHelper::createRegressionCurveByServiceName(
                         uno::Reference< uno::XComponentContext >(), aServiceName ));
    }

    return xResult;
}

/** removes all regression curves that are of type linear, log, exp or pot. (not
    mean-value) and returns true, if anything was removed
 */
bool lcl_removeAllKnownRegressionCurves( const uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    bool bResult = false;
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
                          C2U( "com.sun.star.comp.chart2.LinearRegressionCurve" )) ||
                      xServName->getServiceName().equals(
                          C2U( "com.sun.star.comp.chart2.LogarithmicRegressionCurve" )) ||
                      xServName->getServiceName().equals(
                          C2U( "com.sun.star.comp.chart2.ExponentialRegressionCurve" )) ||
                      xServName->getServiceName().equals(
                          C2U( "com.sun.star.comp.chart2.PotentialRegressionCurve" ))
                        ))
                {
                    aDeleteIndexes.push_back( i );
                }
            }

            if( aDeleteIndexes.size() > 0 )
            {
                for( ::std::vector< sal_Int32 >::const_iterator aIt = aDeleteIndexes.begin();
                     aIt != aDeleteIndexes.end(); ++aIt )
                {
                    xRegCnt->removeRegressionCurve( aCurves[ *aIt ] );
                }
                bResult = true;
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return bResult;
}

chart2::ErrorBar lcl_GetDefaultErrorBar()
{
    chart2::ErrorBar aErrorBar;

    aErrorBar.aStyle = chart2::ErrorBarStyle_ABSOLUTE;
    aErrorBar.fPositiveError = 0.0;
    aErrorBar.fNegativeError = 0.0;
    aErrorBar.fWeight = 0.0;
    aErrorBar.bShowPositiveError = false;
    aErrorBar.bShowNegativeError = false;

    return aErrorBar;
}

} // anonymous namespace

namespace chart
{
namespace wrapper
{

StatisticsItemConverter::StatisticsItemConverter(
    const uno::Reference<
    beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool ) :
        ItemConverter( rPropertySet, rItemPool )
{
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
            bool bOldHasMeanValueLine = lcl_HasMeanValueLine( xRegCnt );

            bool bNewHasMeanValueLine =
                reinterpret_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();

            if( bOldHasMeanValueLine != bNewHasMeanValueLine )
            {
                if( ! bNewHasMeanValueLine )
                    lcl_RemoveMeanValueLine( xRegCnt );
                else
                    lcl_AddMeanValueLine( xRegCnt );
                bChanged = true;
            }
        }
        break;

        // Attention !!! This case must be passed before SCHATTR_STAT_PERCENT,
        // SCHATTR_STAT_BIGERROR, SCHATTR_STAT_CONSTPLUS,
        // SCHATTR_STAT_CONSTMINUS and SCHATTR_STAT_INDICATE
        case SCHATTR_STAT_KIND_ERROR:
        {
            chart2::ErrorBar aOldErrorBar;
            bool bOldHasErrorBar = ( lcl_GetYErrorBar( GetPropertySet(), aOldErrorBar ));

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

                if( eErrorKind != CHERROR_NONE &&
                    ( ! bOldHasErrorBar ||
                      aOldErrorBar.aStyle != eStyle ))
                {
                    if( ! bOldHasErrorBar )
                        aOldErrorBar = lcl_GetDefaultErrorBar();
                    aOldErrorBar.aStyle = eStyle;
                    GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                        uno::makeAny( aOldErrorBar ));
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_STAT_PERCENT:
        case SCHATTR_STAT_BIGERROR:
        {
            chart2::ErrorBar aOldErrorBar;
            bool bOldHasErrorBar = ( lcl_GetYErrorBar( GetPropertySet(), aOldErrorBar ));

            double fValue =
                reinterpret_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            if( bOldHasErrorBar &&
                ! ( ::rtl::math::approxEqual( aOldErrorBar.fPositiveError, fValue ) &&
                    ::rtl::math::approxEqual( aOldErrorBar.fNegativeError, fValue )))
            {
                aOldErrorBar.fPositiveError = fValue;
                aOldErrorBar.fNegativeError = fValue;
                GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                    uno::makeAny( aOldErrorBar ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTPLUS:
        {
            chart2::ErrorBar aOldErrorBar;
            bool bOldHasErrorBar = ( lcl_GetYErrorBar( GetPropertySet(), aOldErrorBar ));

            double fValue =
                reinterpret_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            if( bOldHasErrorBar &&
                ! ::rtl::math::approxEqual( aOldErrorBar.fPositiveError, fValue ))
            {
                aOldErrorBar.fPositiveError = fValue;
                GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                    uno::makeAny( aOldErrorBar ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTMINUS:
        {
            chart2::ErrorBar aOldErrorBar;
            bool bOldHasErrorBar = ( lcl_GetYErrorBar( GetPropertySet(), aOldErrorBar ));

            double fValue =
                reinterpret_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            if( bOldHasErrorBar &&
                ! ::rtl::math::approxEqual( aOldErrorBar.fNegativeError, fValue ))
            {
                aOldErrorBar.fNegativeError = fValue;
                GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                    uno::makeAny( aOldErrorBar ));
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
                bChanged = lcl_removeAllKnownRegressionCurves( xRegCnt );
            }
            else
            {
                SvxChartRegress eOldRegress;;
                if( ! lcl_getRegressType( xRegCnt, eOldRegress ) ||
                    eOldRegress != eRegress )
                {
                    lcl_removeAllKnownRegressionCurves( xRegCnt );
                    xRegCnt->addRegressionCurve(
                        lcl_createRegressionCurve( eRegress ));
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_STAT_INDICATE:
        {
            chart2::ErrorBar aOldErrorBar;
            bool bOldHasErrorBar = ( lcl_GetYErrorBar( GetPropertySet(), aOldErrorBar ));

            SvxChartIndicate eIndicate =
                reinterpret_cast< const SvxChartIndicateItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            bool bNewIndPos = (eIndicate == CHINDICATE_BOTH || eIndicate == CHINDICATE_UP );
            bool bNewIndNeg = (eIndicate == CHINDICATE_BOTH || eIndicate == CHINDICATE_DOWN );

            if( bOldHasErrorBar &&
                ( aOldErrorBar.bShowPositiveError != bNewIndPos ||
                  aOldErrorBar.bShowNegativeError != bNewIndNeg ))
            {
                aOldErrorBar.bShowPositiveError = bNewIndPos;
                aOldErrorBar.bShowNegativeError = bNewIndNeg;

                GetPropertySet()->setPropertyValue( C2U( "ErrorBarY" ),
                                                    uno::makeAny( aOldErrorBar ));
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
                             lcl_HasMeanValueLine(
                                 uno::Reference< chart2::XRegressionCurveContainer >(
                                     GetPropertySet(), uno::UNO_QUERY ))));
            break;

        case SCHATTR_STAT_KIND_ERROR:
        {
            SvxChartKindError eErrorKind = CHERROR_NONE;
            chart2::ErrorBar aErrorBar;
            if( lcl_GetYErrorBar( GetPropertySet(), aErrorBar ))
            {
                switch( aErrorBar.aStyle )
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

                    case chart2::ErrorBarStyle_FROM_DATA:
                        // suppress warning
                    case chart2::ErrorBarStyle_MAKE_FIXED_SIZE:
                        break;
                }
            }
            rOutItemSet.Put( SvxChartKindErrorItem( eErrorKind ));
        }
        break;

        case SCHATTR_STAT_PERCENT:
        {
            chart2::ErrorBar aErrorBar;
            if( lcl_GetYErrorBar( GetPropertySet(), aErrorBar ))
            {
                double fValue = ( aErrorBar.fPositiveError + aErrorBar.fNegativeError ) / 2.0;
                rOutItemSet.Put( SvxDoubleItem( fValue, nWhichId ));
            }
        }
        break;

        case SCHATTR_STAT_BIGERROR:
        {
            chart2::ErrorBar aErrorBar;
            if( lcl_GetYErrorBar( GetPropertySet(), aErrorBar ))
            {
                double fValue = ( aErrorBar.fPositiveError + aErrorBar.fNegativeError ) / 2.0;
                rOutItemSet.Put( SvxDoubleItem( fValue, nWhichId ));
            }
        }
        break;

        case SCHATTR_STAT_CONSTPLUS:
        {
            chart2::ErrorBar aErrorBar;
            if( lcl_GetYErrorBar( GetPropertySet(), aErrorBar ))
                rOutItemSet.Put( SvxDoubleItem( aErrorBar.fPositiveError, nWhichId ));
        }
        break;

        case SCHATTR_STAT_CONSTMINUS:
        {
            chart2::ErrorBar aErrorBar;
            if( lcl_GetYErrorBar( GetPropertySet(), aErrorBar ))
                rOutItemSet.Put( SvxDoubleItem( aErrorBar.fNegativeError, nWhichId ));
        }
        break;

        case SCHATTR_STAT_REGRESSTYPE:
        {
            SvxChartRegress eRegress = CHREGRESS_NONE;
            lcl_getRegressType( uno::Reference< chart2::XRegressionCurveContainer >(
                                    GetPropertySet(), uno::UNO_QUERY ), eRegress );
            rOutItemSet.Put( SvxChartRegressItem( eRegress ));
        }
        break;

        case SCHATTR_STAT_INDICATE:
        {
            chart2::ErrorBar aErrorBar;
            if( lcl_GetYErrorBar( GetPropertySet(), aErrorBar ))
            {
                SvxChartIndicate eIndicate = CHINDICATE_NONE;

                if( aErrorBar.bShowPositiveError )
                {
                    if( aErrorBar.bShowNegativeError )
                        eIndicate = CHINDICATE_BOTH;
                    else
                        eIndicate = CHINDICATE_UP;
                }
                else
                {
                    if( aErrorBar.bShowNegativeError )
                        eIndicate = CHINDICATE_DOWN;
                    else
                        eIndicate = CHINDICATE_NONE;
                }

                rOutItemSet.Put( SvxChartIndicateItem( eIndicate ));
            }
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart
