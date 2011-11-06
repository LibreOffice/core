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


#ifndef CHART_WRAPPED_STATISTIC_PROPERTIES_HXX
#define CHART_WRAPPED_STATISTIC_PROPERTIES_HXX

#include "WrappedProperty.hxx"
#include "Chart2ModelContact.hxx"

#include <boost/shared_ptr.hpp>
#include <vector>

//.............................................................................
namespace chart
{
namespace wrapper
{

class WrappedStatisticProperties
{
public:
    static void addProperties( ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );
    static void addWrappedPropertiesForSeries( std::vector< WrappedProperty* >& rList
                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    static void addWrappedPropertiesForDiagram( std::vector< WrappedProperty* >& rList
                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
};

} //namespace wrapper
} //namespace chart
//.............................................................................

// CHART_WRAPPED_STATISTIC_PROPERTIES_HXX
#endif
