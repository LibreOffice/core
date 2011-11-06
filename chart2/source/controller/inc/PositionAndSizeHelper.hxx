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



#ifndef _CHART2_POSITIONANDSIZEHELPER_HXX
#define _CHART2_POSITIONANDSIZEHELPER_HXX

#include "ObjectIdentifier.hxx"
#include <com/sun/star/frame/XModel.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class PositionAndSizeHelper
{
public:
    static bool moveObject( ObjectType eObjectType
            , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xObjectProp
            , const ::com::sun::star::awt::Rectangle& rNewPositionAndSize
            , const ::com::sun::star::awt::Rectangle& rPageRectangle );

    static bool moveObject( const rtl::OUString& rObjectCID
            , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
            , const ::com::sun::star::awt::Rectangle& rNewPositionAndSize
            , const ::com::sun::star::awt::Rectangle& rPageRectangle );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
