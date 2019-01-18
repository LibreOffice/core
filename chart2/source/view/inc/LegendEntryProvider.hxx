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

#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <vector>

namespace chart { class ChartModel; }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XFormattedString2; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShapes; } } } }
namespace com { namespace sun { namespace star { namespace lang { class XMultiServiceFactory; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }

namespace chart
{

enum class LegendSymbolStyle
{
    /** A square box with border.
     */
    Box,

    /** A line like with a symbol.
     */
    Line,

    /** A bordered circle which has the same bounding-box as the
        <member>BOX</member>.
     */
    Circle
};

struct ViewLegendEntry
{
    /** The legend symbol that represents a data series or other
        information contained in the legend
     */
    css::uno::Reference< css::drawing::XShape > aSymbol;

    /** The descriptive text for a legend entry.
     */
    css::uno::Sequence<
        css::uno::Reference< css::chart2::XFormattedString2 > >  aLabel;
};

class LegendEntryProvider
{
public:
    virtual css::awt::Size getPreferredLegendKeyAspectRatio()=0;

    virtual std::vector< ViewLegendEntry > createLegendEntries(
            const css::awt::Size& rEntryKeyAspectRatio,
            css::chart::ChartLegendExpansion eLegendExpansion,
            const css::uno::Reference< css::beans::XPropertySet >& xTextProperties,
            const css::uno::Reference< css::drawing::XShapes >& xTarget,
            const css::uno::Reference< css::lang::XMultiServiceFactory >& xShapeFactory,
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            ChartModel& rModel
                ) = 0;

protected:
    ~LegendEntryProvider() {}
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_VIEW_INC_LEGENDENTRYPROVIDER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
