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


#ifndef CHART_DATASERIESPROPERTIES_HXX
#define CHART_DATASERIESPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include <com/sun/star/beans/Property.hpp>

#include <vector>

namespace chart
{

class DataSeriesProperties
{
public:
    enum
    {
        PROP_DATASERIES_ATTRIBUTED_DATA_POINTS = FAST_PROPERTY_ID_START_DATA_SERIES,
        PROP_DATASERIES_STACKING_DIRECTION,
        PROP_DATASERIES_VARY_COLORS_BY_POINT,
        PROP_DATASERIES_ATTACHED_AXIS_INDEX
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    static void AddDefaultsToMap( tPropertyValueMap & rOutMap );

private:
    // not implemented
    DataSeriesProperties();
};

} //  namespace chart

// CHART_DATASERIESPROPERTIES_HXX
#endif
