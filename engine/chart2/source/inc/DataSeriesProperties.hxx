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
#pragma once

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"

#include <vector>

namespace com::sun::star::beans { struct Property; }

namespace chart::DataSeriesProperties
{
    enum
    {
        PROP_DATASERIES_ATTRIBUTED_DATA_POINTS = FAST_PROPERTY_ID_START_DATA_SERIES,
        PROP_DATASERIES_STACKING_DIRECTION,
        PROP_DATASERIES_VARY_COLORS_BY_POINT,
        PROP_DATASERIES_ATTACHED_AXIS_INDEX,
        PROP_DATASERIES_SHOW_LEGEND_ENTRY,
        PROP_DATASERIES_DELETED_LEGEND_ENTRIES,
        PROP_DATASERIES_SHOW_CUSTOM_LEADERLINES,
        PROP_DATASERIES_INVERT_NEGATIVE,
        PROP_DATASERIES_INTERVAL_CLOSED,
        PROP_DATASERIES_CONNECTOR_LINES,
        PROP_DATASERIES_MEAN_LINE,
        PROP_DATASERIES_MEAN_MARKER,
        PROP_DATASERIES_NONOUTLIERS,
        PROP_DATASERIES_OUTLIERS,
        PROP_DATASERIES_PARENT_LABEL_LAYOUT,
        PROP_DATASERIES_REGION_LABEL_LAYOUT,
        PROP_DATASERIES_QUARTILE_METHOD,
        PROP_DATASERIES_SUBTOTAL_INDICES,
        PROP_DATASERIES_HAS_GEOGRAPHY,
        PROP_DATASERIES_GEOGRAPHY_PROJECTION_TYPE,
        PROP_DATASERIES_GEOGRAPHY_VIEWED_REGION_TYPE,
        PROP_DATASERIES_GEOGRAPHY_CULTURE_LANGUAGE,
        PROP_DATASERIES_GEOGRAPHY_CULTURE_REGION,
        PROP_DATASERIES_GEOGRAPHY_ATTRIBUTION,
        PROP_DATASERIES_GEOCACHE_PROVIDER,
        PROP_DATASERIES_GEOCACHE_BINARY,
        PROP_DATASERIES_GEOCACHE_CLEAR_DATA,
    };

    void AddPropertiesToVector(
        std::vector< css::beans::Property > & rOutProperties );

    void AddDefaultsToMap( tPropertyValueMap & rOutMap );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
