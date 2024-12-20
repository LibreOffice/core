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
#include <rtl/ref.hxx>
#include <svx/unoshape.hxx>

namespace com::sun::star::awt { struct Size; }
namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::uno { class Any; }

namespace chart::VLegendSymbolFactory
{
    enum class PropertyType
    {
        FilledSeries,
        LineSeries,
        Line,
    };

    rtl::Reference< SvxShapeGroup >
        createSymbol(
            const css::awt::Size& rEntryKeyAspectRatio,
            const rtl::Reference<SvxShapeGroupAnyD>& rSymbolContainer,
            LegendSymbolStyle eStyle,
            const css::uno::Reference< css::beans::XPropertySet > & xLegendEntryProperties,
            PropertyType ePropertyType,
            const css::uno::Any& rExplicitSymbol /*should contain a css::chart2::Symbol without automatic symbol if the charttype does support symbols else empty*/);

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
