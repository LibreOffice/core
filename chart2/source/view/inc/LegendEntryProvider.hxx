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


#ifndef CHART2_VIEW_LEGENDENTRYPROVIDER_HXX
#define CHART2_VIEW_LEGENDENTRYPROVIDER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vector>

namespace chart
{

enum LegendSymbolStyle
{
    /** A square box with border.
     */
    LegendSymbolStyle_BOX,

    /** A line like with a symbol.
     */
    LegendSymbolStyle_LINE,

    /** A bordered circle which has the same bounding-box as the
        <member>BOX</member>.
     */
    LegendSymbolStyle_CIRCLE
};

struct ViewLegendEntry
{
    /** The legend symbol that represents a data series or other
        information contained in the legend
     */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > aSymbol;

    /** The descriptive text for a legend entry.
     */
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XFormattedString > >  aLabel;
};

class LegendEntryProvider
{
public:
    virtual ::com::sun::star::awt::Size getPreferredLegendKeyAspectRatio()=0;

    virtual std::vector< ViewLegendEntry > createLegendEntries(
            const ::com::sun::star::awt::Size& rEntryKeyAspectRatio,
            ::com::sun::star::chart::ChartLegendExpansion eLegendExpansion,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xTextProperties,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& xShapeFactory,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext >& xContext
                ) = 0;
};

} //  namespace chart

// CHART2_VIEW_LEGENDENTRYPROVIDER_HXX
#endif
