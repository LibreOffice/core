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



#ifndef _CHART2_VLINEPROPERTIES_HXX
#define _CHART2_VLINEPROPERTIES_HXX

#include <com/sun/star/beans/XPropertySet.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

struct VLineProperties
{
    com::sun::star::uno::Any Color; //type sal_Int32 UNO_NAME_LINECOLOR
    com::sun::star::uno::Any LineStyle; //type drawing::LineStyle for property UNO_NAME_LINESTYLE
    com::sun::star::uno::Any Transparence;//type sal_Int16 for property UNO_NAME_LINETRANSPARENCE
    com::sun::star::uno::Any Width;//type sal_Int32 for property UNO_NAME_LINEWIDTH
    com::sun::star::uno::Any DashName;//type OUString for property "LineDashName"

    VLineProperties();
    void initFromPropertySet( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet >& xProp
                              , bool bUseSeriesPropertyNames=false );

    bool isLineVisible() const;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
