/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_LEGENDENTRYPROVIDER_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_LEGENDENTRYPROVIDER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/XFormattedString2.hpp>
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
            ::com::sun::star::chart2::XFormattedString2 > >  aLabel;
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

protected:
    ~LegendEntryProvider() {}
};

}

// INCLUDED_CHART2_SOURCE_VIEW_INC_LEGENDENTRYPROVIDER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
