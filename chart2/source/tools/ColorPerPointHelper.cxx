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

#include <ColorPerPointHelper.hxx>
#include <DataSeries.hxx>
#include <DataSeriesProperties.hxx>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::chart::DataSeriesProperties;

namespace chart
{

bool ColorPerPointHelper::hasPointOwnColor(
        const rtl::Reference< DataSeries >& xDataSeries
        , sal_Int32 nPointIndex
        , const css::uno::Reference< css::beans::XPropertySet >& xDataPointProperties //may be NULL this is just for performance
         )
{
    if( !xDataSeries.is() )
        return false;

    if( hasPointOwnProperties( xDataSeries, nPointIndex ))
    {
        uno::Reference< beans::XPropertyState > xPointState( xDataPointProperties, uno::UNO_QUERY );
        if( !xPointState.is() )
        {
            xPointState.set( xDataSeries->getDataPointByIndex( nPointIndex ), uno::UNO_QUERY );
        }
        if( !xPointState.is() )
            return false;

        return (xPointState->getPropertyState( u"Color"_ustr) != beans::PropertyState_DEFAULT_VALUE );
    }

    return false;
}

bool ColorPerPointHelper::hasPointOwnProperties(
    const rtl::Reference< ::chart::DataSeries >& xDataSeries
    , sal_Int32 nPointIndex )
{
    if( xDataSeries.is() )
    {
        uno::Sequence< sal_Int32 > aIndexList;
        if( xDataSeries->getFastPropertyValue( PROP_DATASERIES_ATTRIBUTED_DATA_POINTS ) >>= aIndexList ) // "AttributedDataPoints"
        {
            return (std::find(aIndexList.begin(), aIndexList.end(), nPointIndex) != aIndexList.end());
        }
    }

    return false;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
