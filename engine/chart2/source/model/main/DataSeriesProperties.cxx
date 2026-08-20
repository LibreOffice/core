/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <DataSeriesProperties.hxx>
#include "DataPointProperties.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void DataSeriesProperties::AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( u"AttributedDataPoints"_ustr,
                  PROP_DATASERIES_ATTRIBUTED_DATA_POINTS,
                  cppu::UnoType<cpo::uno::Sequence< sal_Int32 >>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"StackingDirection"_ustr,
                  PROP_DATASERIES_STACKING_DIRECTION,
                  cppu::UnoType<chart2::StackingDirection>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"VaryColorsByPoint"_ustr,
                  PROP_DATASERIES_VARY_COLORS_BY_POINT,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"AttachedAxisIndex"_ustr,
                  PROP_DATASERIES_ATTACHED_AXIS_INDEX,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"ShowLegendEntry"_ustr,
                  PROP_DATASERIES_SHOW_LEGEND_ENTRY,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"DeletedLegendEntries"_ustr,
                  PROP_DATASERIES_DELETED_LEGEND_ENTRIES,
                  cppu::UnoType<cpo::uno::Sequence<sal_Int32>>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"ShowCustomLeaderLines"_ustr,
                  PROP_DATASERIES_SHOW_CUSTOM_LEADERLINES,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"InvertNegative"_ustr,
                  PROP_DATASERIES_INVERT_NEGATIVE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"IntervalClosed"_ustr,
                  PROP_DATASERIES_INTERVAL_CLOSED,
                  cppu::UnoType<sal_uInt32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"ConnectorLines"_ustr,
                  PROP_DATASERIES_CONNECTOR_LINES,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"HasExplicitFill"_ustr,
                  PROP_DATASERIES_HAS_EXPLICIT_FILL,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"HasDataLabels"_ustr,
                  PROP_DATASERIES_HAS_DATA_LABELS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"MeanLine"_ustr,
                  PROP_DATASERIES_MEAN_LINE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"MeanMarker"_ustr,
                  PROP_DATASERIES_MEAN_MARKER,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"Nonoutliers"_ustr,
                  PROP_DATASERIES_NONOUTLIERS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"Outliers"_ustr,
                  PROP_DATASERIES_OUTLIERS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"ParentLabelLayout"_ustr,
                  PROP_DATASERIES_PARENT_LABEL_LAYOUT,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"RegionLabelLayout"_ustr,
                  PROP_DATASERIES_REGION_LABEL_LAYOUT,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"QuartileMethod"_ustr,
                  PROP_DATASERIES_QUARTILE_METHOD,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"SubtotalIndices"_ustr,
                  PROP_DATASERIES_SUBTOTAL_INDICES,
                  cppu::UnoType<cpo::uno::Sequence<sal_Int32>>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"HasGeography"_ustr,
                  PROP_DATASERIES_HAS_GEOGRAPHY,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"GeographyProjectionType"_ustr,
                  PROP_DATASERIES_GEOGRAPHY_PROJECTION_TYPE,
                  cppu::UnoType<sal_Int32>::get(), // Actually enum GeoProjectionType
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"GeographyViewedRegionType"_ustr,
                  PROP_DATASERIES_GEOGRAPHY_VIEWED_REGION_TYPE,
                  cppu::UnoType<sal_Int32>::get(), // Actually enum GeoMappingLevel
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"GeographyCultureLanguage"_ustr,
                  PROP_DATASERIES_GEOGRAPHY_CULTURE_LANGUAGE,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"GeographyCultureRegion"_ustr,
                  PROP_DATASERIES_GEOGRAPHY_CULTURE_REGION,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"GeographyAttribution"_ustr,
                  PROP_DATASERIES_GEOGRAPHY_ATTRIBUTION,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"GeoCacheProvider"_ustr,
                  PROP_DATASERIES_GEOCACHE_PROVIDER,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"GeoCacheBinary"_ustr,
                  PROP_DATASERIES_GEOCACHE_BINARY,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"GeoCacheClearData"_ustr,
                  PROP_DATASERIES_GEOCACHE_CLEAR_DATA,
                  cppu::UnoType<cpo::uno::Sequence<beans::PropertyValue>>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    // Chartex only: the cx:axisId values listed on the cx:series element at
    // import. Empty for axis-less chartex types (regionMap, sunburst, treemap).
    rOutProperties.emplace_back( u"ChartexAxisIds"_ustr,
                  PROP_DATASERIES_CHARTEX_AXIS_IDS,
                  cppu::UnoType<cpo::uno::Sequence<sal_Int32>>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    // Chartex only: cx:series/@ownerIdx. When present, this series shares
    // data with the series at this zero-based index in the same
    // plotAreaRegion, and no cx:dataId is emitted.
    rOutProperties.emplace_back( u"ChartexOwnerIdx"_ustr,
                  PROP_DATASERIES_CHARTEX_OWNER_IDX,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    // add properties of service DataPointProperties
    DataPointProperties::AddPropertiesToVector( rOutProperties );
}

void DataSeriesProperties::AddDefaultsToMap(
    tPropertyValueMap & rOutMap )
{
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATASERIES_STACKING_DIRECTION, chart2::StackingDirection_NO_STACKING );
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATASERIES_VARY_COLORS_BY_POINT, false );
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATASERIES_ATTACHED_AXIS_INDEX, sal_Int32(0) );
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATASERIES_SHOW_LEGEND_ENTRY, true );
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATASERIES_SHOW_CUSTOM_LEADERLINES, true );
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATASERIES_INVERT_NEGATIVE, false );
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATASERIES_HAS_EXPLICIT_FILL, false );

    // PROP_DATASERIES_ATTRIBUTED_DATA_POINTS has no default

    // add properties of service DataPointProperties
    DataPointProperties::AddDefaultsToMap( rOutMap );
}

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
