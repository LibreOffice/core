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


#ifndef _CHART2_ACCSTATISTICSOBJECT_HXX
#define _CHART2_ACCSTATISTICSOBJECT_HXX

#ifndef _CHART2_ACCESSIBLE_CHART_ELEMENT_HXX_
#include "AccessibleChartElement.hxx"
#endif

#include "ObjectIdentifier.hxx"

namespace chart
{

class AccStatisticsObject : public AccessibleChartElement
{
public:
    /*
    enum StatisticsObjectType
    {
        MEAN_VAL_LINE = OBJECTTYPE_AVERAGEVALUE,
        ERROR_BARS    = OBJECTTYPE_ERROR,
        REGRESSION    = OBJECTTYPE_REGRESSION
    };
    */

    AccStatisticsObject( const AccessibleElementInfo& rAccInfo );

    virtual ~AccStatisticsObject();

    // ________ XAccessibleContext ________
    virtual ::rtl::OUString SAL_CALL getAccessibleName() throw (::com::sun::star::uno::RuntimeException);

    // ________ XServiceInfo ________
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);

private:
    //const StatisticsObjectType    m_eType;
    //const sal_uInt16              m_nSeriesIndex;
};

}  // accessibility

#endif
