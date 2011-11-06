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


#ifndef CHART_NAMEDLINEPROPERTIES_HXX
#define CHART_NAMEDLINEPROPERTIES_HXX

#error "Deprecated, do not include this file"

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include <com/sun/star/beans/Property.hpp>

#include <vector>

namespace chart
{


// @depreated !!
class NamedLineProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.drawing.LineProperties (only named properties)
        PROP_LINE_DASH_NAME = FAST_PROPERTY_ID_START_NAMED_LINE_PROP,
        PROP_LINE_END_NAME,
        PROP_LINE_START_NAME,

        FAST_PROPERTY_ID_END_NAMED_LINE_PROP
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties,
        bool bIncludeLineEnds = false );

    static void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap,
                                  bool bIncludeLineEnds = false );

    //will return e.g. "LineDashName" for PROP_LINE_DASH_NAME
    static ::rtl::OUString GetPropertyNameForHandle( sal_Int32 nHandle );

private:
    // not implemented
    NamedLineProperties();
};

} //  namespace chart

// CHART_NAMEDLINEPROPERTIES_HXX
#endif
