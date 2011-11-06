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


#ifndef CHART_USERDEFINEDPROPERTIES_HXX
#define CHART_USERDEFINEDPROPERTIES_HXX

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/beans/Property.hpp>

#include <vector>

namespace chart
{

// implements service com.sun.star.xml.UserDefinedAttributeSupplier
class OOO_DLLPUBLIC_CHARTTOOLS UserDefinedProperties
{
public:
    // FastProperty Ids for properties
    enum
    {
        // UserDefined attributes in the chart-properties
        PROP_XML_USERDEF_CHART = FAST_PROPERTY_ID_START_USERDEF_PROP,
        PROP_XML_USERDEF_TEXT,
        PROP_XML_USERDEF_PARA,
        // com.sun.star.xml.UserDefinedAttributeSupplier
        // UserDefined attributes in the other properties-sections. (as long as
        // there is no reliable mechanism, properties may move between the
        // sections)
        PROP_XML_USERDEF
    };

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

private:
    // not implemented
    UserDefinedProperties();
};

} //  namespace chart

// CHART_USERDEFINEDPROPERTIES_HXX
#endif
