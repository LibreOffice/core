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


#ifndef CHART2_VLEGENDSYMBOLFACTORY_HXX
#define CHART2_VLEGENDSYMBOLFACTORY_HXX

#include "LegendEntryProvider.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

namespace chart
{

class VLegendSymbolFactory
{
public:
    enum tPropertyType
    {
        PROP_TYPE_FILLED_SERIES,
        PROP_TYPE_LINE_SERIES,
        PROP_TYPE_FILL,
        PROP_TYPE_LINE,
        PROP_TYPE_FILL_AND_LINE
    };

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape >
        createSymbol(
            const ::com::sun::star::awt::Size& rEntryKeyAspectRatio,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes > xSymbolContainer,
            LegendSymbolStyle eStyle,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory > & xShapeFactory,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & xLegendEntryProperties,
            tPropertyType ePropertyType,
            const ::com::sun::star::uno::Any& rExplicitSymbol /*should contain a ::com::sun::star::chart2::Symbol without automatic symbol if the charttype does support symbols else empty*/);

private:
     VLegendSymbolFactory();
};

} //  namespace chart

// CHART2_VLEGENDSYMBOLFACTORY_HXX
#endif
