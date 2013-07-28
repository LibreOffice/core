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

#include "ColorPerPointHelper.hxx"
#include "macros.hxx"
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

#include <algorithm>

namespace chart
{
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

        return (xPointState->getPropertyState( "Color") != beans::PropertyState_DEFAULT_VALUE );
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
        if( xSeriesProperties->getPropertyValue( "AttributedDataPoints" ) >>= aIndexList )
        {
            const sal_Int32 * pBegIt = aIndexList.getConstArray();
            const sal_Int32 * pEndIt = pBegIt + aIndexList.getLength();
            return ( ::std::find( pBegIt, pEndIt, nPointIndex ) != pEndIt );
        }
    }

    return false;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
