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


#ifndef INCLUDED_DEVCHARTCONFIG_HXX
#define INCLUDED_DEVCHARTCONFIG_HXX

#include <unotools/configitem.hxx>

namespace ScDevChart
{

class DevChartConfigItem : public ::utl::ConfigItem
{
public:
    DevChartConfigItem() :
            ConfigItem( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Internal" )),
                        CONFIG_MODE_IMMEDIATE_UPDATE )
    {}

    bool UseDevelopmentChart();
};

bool DevChartConfigItem::UseDevelopmentChart()
{
    bool bResult = false;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames( 1 );
    aNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DevelopmentChart" ));

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aResult( GetProperties( aNames ));

    OSL_ASSERT( aResult.getLength());
    aResult[0] >>= bResult;

    return bResult;
}

/** States whether the new chart implementation or the old one should be used.
    If <TRUE/> is returned the newly developed chart (chart2) should be used.
    If <FALSE/> is returned, the old chart (sch) should be used.

    Config-Item: Office.Common/Internal:DevelopmentChart

    This function (the complete header) is only for a transitional period.  It
    will be deprecated after the new chart is definitely integrated into the
    product.
 */
bool UseDevChart()
{
//     static DevChartConfigItem aCfgItem;
//     return aCfgItem.UseDevelopmentChart();

    // ignore configuration
    //@todo: get rid of this class
    return true;
}

} // namespace ScDevChart

// INCLUDED_DEVCHARTCONFIG_HXX
#endif
