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



#ifndef _CHART2_COLORPERPOINTHELPER_HXX
#define _CHART2_COLORPERPOINTHELPER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include "charttoolsdllapi.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class OOO_DLLPUBLIC_CHARTTOOLS ColorPerPointHelper
{
public:
    static bool hasPointOwnColor(
        const ::com::sun::star::uno::Reference<
              ::com::sun::star::beans::XPropertySet >& xDataSeriesProperties
        , sal_Int32 nPointIndex
        , const ::com::sun::star::uno::Reference<
              ::com::sun::star::beans::XPropertySet >& xDataPointProperties //may be NULL this is just for performance
         );

    // returns true if AttributedDataPoints contains nPointIndex and the
    // property Color is DEFAULT
    SAL_DLLPRIVATE static bool hasPointOwnProperties(
        const ::com::sun::star::uno::Reference<
              ::com::sun::star::beans::XPropertySet >& xSeriesProperties
        , sal_Int32 nPointIndex );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
