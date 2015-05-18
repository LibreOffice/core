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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_VLEGENDSYMBOLFACTORY_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_VLEGENDSYMBOLFACTORY_HXX

#include "LegendEntryProvider.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

namespace chart
{

namespace VLegendSymbolFactory
{
    enum tPropertyType
    {
        PROP_TYPE_FILLED_SERIES,
        PROP_TYPE_LINE_SERIES,
        PROP_TYPE_FILL,
        PROP_TYPE_LINE,
        PROP_TYPE_FILL_AND_LINE
    };

    ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape >
        createSymbol(
            const ::com::sun::star::awt::Size& rEntryKeyAspectRatio,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& rSymbolContainer,
            LegendSymbolStyle eStyle,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory > & xShapeFactory,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & xLegendEntryProperties,
            tPropertyType ePropertyType,
            const ::com::sun::star::uno::Any& rExplicitSymbol /*should contain a ::com::sun::star::chart2::Symbol without automatic symbol if the charttype does support symbols else empty*/);
}

}

// INCLUDED_CHART2_SOURCE_VIEW_INC_VLEGENDSYMBOLFACTORY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
