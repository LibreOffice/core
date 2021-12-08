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

#include "DataSeriesProperties.hxx"
#include "DataPointProperties.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void DataSeriesProperties::AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "AttributedDataPoints",
                  PROP_DATASERIES_ATTRIBUTED_DATA_POINTS,
                  cppu::UnoType<uno::Sequence< sal_Int32 >>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "StackingDirection",
                  PROP_DATASERIES_STACKING_DIRECTION,
                  cppu::UnoType<chart2::StackingDirection>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "VaryColorsByPoint",
                  PROP_DATASERIES_VARY_COLORS_BY_POINT,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "AttachedAxisIndex",
                  PROP_DATASERIES_ATTACHED_AXIS_INDEX,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ShowLegendEntry",
                  PROP_DATASERIES_SHOW_LEGEND_ENTRY,
                  cppu::UnoType<sal_Bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "DeletedLegendEntries",
                  PROP_DATASERIES_DELETED_LEGEND_ENTRIES,
                  cppu::UnoType<uno::Sequence<sal_Int32>>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "ShowCustomLeaderLines",
                  PROP_DATASERIES_SHOW_CUSTOM_LEADERLINES,
                  cppu::UnoType<sal_Bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // add properties of service DataPointProperties
    DataPointProperties::AddPropertiesToVector( rOutProperties );
}

void DataSeriesProperties::AddDefaultsToMap(
    tPropertyValueMap & rOutMap )
{
    rOutMap.setPropertyValueDefault( PROP_DATASERIES_STACKING_DIRECTION, chart2::StackingDirection_NO_STACKING );
    rOutMap.setPropertyValueDefault( PROP_DATASERIES_VARY_COLORS_BY_POINT, false );
    rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_DATASERIES_ATTACHED_AXIS_INDEX, 0 );
    rOutMap.setPropertyValueDefault( PROP_DATASERIES_SHOW_LEGEND_ENTRY, true );
    rOutMap.setPropertyValueDefault( PROP_DATASERIES_SHOW_CUSTOM_LEADERLINES, true );

    // PROP_DATASERIES_ATTRIBUTED_DATA_POINTS has no default

    // add properties of service DataPointProperties
    DataPointProperties::AddDefaultsToMap( rOutMap );
}

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
