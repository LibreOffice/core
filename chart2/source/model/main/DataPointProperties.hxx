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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_MAIN_DATAPOINTPROPERTIES_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_MAIN_DATAPOINTPROPERTIES_HXX

#include <PropertyHelper.hxx>
#include <FastPropertyIdRanges.hxx>
#include <com/sun/star/beans/Property.hpp>

#include <vector>

namespace chart
{

// implements service DataPointProperties
namespace DataPointProperties
{
    // FastProperty Ids for properties
    enum
    {
        // common
        PROP_DATAPOINT_COLOR = FAST_PROPERTY_ID_START_DATA_POINT,
        PROP_DATAPOINT_TRANSPARENCY,

        // fill
        PROP_DATAPOINT_FILL_STYLE,
        PROP_DATAPOINT_TRANSPARENCY_GRADIENT_NAME,
        PROP_DATAPOINT_GRADIENT_NAME,
        PROP_DATAPOINT_GRADIENT_STEPCOUNT,
        PROP_DATAPOINT_HATCH_NAME,
        PROP_DATAPOINT_FILL_BITMAP_NAME,
        PROP_DATAPOINT_FILL_BACKGROUND,

        // border (of filled objects)
        PROP_DATAPOINT_BORDER_COLOR,
        PROP_DATAPOINT_BORDER_STYLE,
        PROP_DATAPOINT_BORDER_WIDTH,
        PROP_DATAPOINT_BORDER_DASH_NAME,
        PROP_DATAPOINT_BORDER_TRANSPARENCY,

        // others
        PROP_DATAPOINT_SYMBOL_PROP,
        PROP_DATAPOINT_OFFSET,
        PROP_DATAPOINT_GEOMETRY3D,
        PROP_DATAPOINT_NUMBER_FORMAT,
        PROP_DATAPOINT_LINK_NUMBERFORMAT_TO_SOURCE,
        PROP_DATAPOINT_PERCENTAGE_NUMBER_FORMAT,
        PROP_DATAPOINT_LABEL_PLACEMENT,
        PROP_DATAPOINT_REFERENCE_DIAGRAM_SIZE,
        PROP_DATAPOINT_TEXT_ROTATION,

        // statistics
        PROP_DATAPOINT_ERROR_BAR_X,
        PROP_DATAPOINT_ERROR_BAR_Y,
        PROP_DATAPOINT_SHOW_ERROR_BOX,
        PROP_DATAPOINT_PERCENT_DIAGONAL,

        // label
        PROP_DATAPOINT_LABEL,
        PROP_DATAPOINT_LABEL_SEPARATOR,
        PROP_DATAPOINT_TEXT_WORD_WRAP,
        PROP_DATAPOINT_LABEL_BORDER_STYLE,
        PROP_DATAPOINT_LABEL_BORDER_COLOR,
        PROP_DATAPOINT_LABEL_BORDER_WIDTH,
        PROP_DATAPOINT_LABEL_BORDER_DASH,
        PROP_DATAPOINT_LABEL_BORDER_DASH_NAME,
        PROP_DATAPOINT_LABEL_BORDER_TRANS,
        PROP_DATAPOINT_CUSTOM_LABEL_FIELDS

        // additionally some properites from ::chart::LineProperties
    };

    void AddPropertiesToVector(
        std::vector< css::beans::Property > & rOutProperties );

    void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );
}

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_MAIN_DATAPOINTPROPERTIES_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
