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


#ifndef CHART_NAMEDPROPERTIES_HXX
#define CHART_NAMEDPROPERTIES_HXX

#include "NamedLineProperties.hxx"
#include "NamedFillProperties.hxx"

#include "PropertyHelper.hxx"
#include <com/sun/star/beans/Property.hpp>

#include <vector>

namespace chart
{

class NamedProperties
{
    /** this class combines the classes NamedFillAttributes and NamedLineAttributes
    thus you can handle all named properties with one call if you like
    */
public:

    static void AddPropertiesToVector(
        ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );

    //will return e.g. "FillGradientName" for nHandle == PROP_FILL_GRADIENT_NAME
    static ::rtl::OUString GetPropertyNameForHandle( sal_Int32 nHandle );

private:
    // not implemented
    NamedProperties();
};

} //  namespace chart

// CHART_NAMEDPROPERTIES_HXX
#endif
