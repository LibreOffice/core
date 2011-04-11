/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "ColorPerPointHelper.hxx"
#include "macros.hxx"
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

#include <algorithm>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
