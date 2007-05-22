/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColorPerPointHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:56:34 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ColorPerPointHelper.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#include <algorithm>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//static
bool ColorPerPointHelper::hasPointOwnColor(
        const ::com::sun::star::uno::Reference<
              ::com::sun::star::beans::XPropertySet >& xDataSeriesProperties
        , sal_Int32 nPointIndex
        , const ::com::sun::star::uno::Reference<
              ::com::sun::star::beans::XPropertySet >& xDataPointProperties //may be NULL this is just for performance
         )
{
    if( !xDataSeriesProperties.is() )
        return false;

    if( hasPointOwnProperties( xDataSeriesProperties, nPointIndex ))
    {
        uno::Reference< beans::XPropertyState > xPointState( xDataPointProperties, uno::UNO_QUERY );
        if( !xPointState.is() )
        {
            uno::Reference< XDataSeries > xSeries( xDataSeriesProperties, uno::UNO_QUERY );
            if(xSeries.is())
                xPointState.set( xSeries->getDataPointByIndex( nPointIndex ), uno::UNO_QUERY );
        }
        if( !xPointState.is() )
            return false;

        return (xPointState->getPropertyState( C2U("Color")) != beans::PropertyState_DEFAULT_VALUE );
    }

    return false;
}

// static
bool ColorPerPointHelper::hasPointOwnProperties(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >& xSeriesProperties
    , sal_Int32 nPointIndex )
{
    if( xSeriesProperties.is() )
    {
        uno::Sequence< sal_Int32 > aIndexList;
        if( xSeriesProperties->getPropertyValue( C2U( "AttributedDataPoints" ) ) >>= aIndexList )
        {
            const sal_Int32 * pBegIt = aIndexList.getConstArray();
            const sal_Int32 * pEndIt = pBegIt + aIndexList.getLength();
            return ( ::std::find( pBegIt, pEndIt, nPointIndex ) != pEndIt );
        }
    }

    return false;
}

//.............................................................................
} //namespace chart
//.............................................................................
