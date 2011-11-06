/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef CHART_DATAPOINTPROPERTIES_HXX
#define CHART_DATAPOINTPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include <com/sun/star/beans/Property.hpp>

#include <vector>

namespace chart
{

// implements service DataPointProperties
class DataPointProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // common
        PROP_DATAPOINT_COLOR = FAST_PROPERTY_ID_START_DATA_POINT,
        PROP_DATAPOINT_TRANSPARENCY,

        // fill
        PROP_DATAPOINT_FILL_STYLE,
//         PROP_DATAPOINT_TRANSPARENCY_STYLE,
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
        PROP_DATAPOINT_LABEL,
        PROP_DATAPOINT_LABEL_SEPARATOR,
        PROP_DATAPOINT_NUMBER_FORMAT,
        PROP_DATAPOINT_PERCENTAGE_NUMBER_FORMAT,
        PROP_DATAPOINT_LABEL_PLACEMENT,
        PROP_DATAPOINT_REFERENCE_DIAGRAM_SIZE,
        PROP_DATAPOINT_TEXT_ROTATION,

        // statistics
        PROP_DATAPOINT_ERROR_BAR_X,
        PROP_DATAPOINT_ERROR_BAR_Y,
        PROP_DATAPOINT_SHOW_ERROR_BOX,
        PROP_DATAPOINT_PERCENT_DIAGONAL

        // additionally some properites from ::chart::LineProperties
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    static void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

private:
    // not implemented
    DataPointProperties();
};

} //  namespace chart

// CHART_DATAPOINTPROPERTIES_HXX
#endif
