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
#pragma once

#include "LegendEntryProvider.hxx"
#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star { namespace awt { struct Size; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShapes; } } } }
namespace com { namespace sun { namespace star { namespace lang { class XMultiServiceFactory; } } } }
namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }

namespace chart
{

namespace VLegendSymbolFactory
{
    enum class PropertyType
    {
        FilledSeries,
        LineSeries,
        Line,
    };

    css::uno::Reference< css::drawing::XShape >
        createSymbol(
            const css::awt::Size& rEntryKeyAspectRatio,
            const css::uno::Reference< css::drawing::XShapes >& rSymbolContainer,
            LegendSymbolStyle eStyle,
            const css::uno::Reference< css::lang::XMultiServiceFactory > & xShapeFactory,
            const css::uno::Reference< css::beans::XPropertySet > & xLegendEntryProperties,
            PropertyType ePropertyType,
            const css::uno::Any& rExplicitSymbol /*should contain a css::chart2::Symbol without automatic symbol if the charttype does support symbols else empty*/);
}

} //  namespace chart

// INCLUDED_CHART2_SOURCE_VIEW_INC_VLEGENDSYMBOLFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
