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

#include <config_options.h>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>

namespace chart { class ChartModel; }
namespace com::sun::star::uno { class XComponentContext; }

namespace chart
{
class Diagram;
class Legend;

class LegendHelper
{
public:
    static rtl::Reference< ::chart::Legend >
        showLegend( ChartModel& rModel
                  , const css::uno::Reference< css::uno::XComponentContext >& xContext );

    static  void hideLegend( ChartModel& rModel );

    static rtl::Reference< ::chart::Legend >
        getLegend( ChartModel& rModel
                 , const css::uno::Reference< css::uno::XComponentContext >& xContext = nullptr
                 , bool bCreate = false );

    /** returns <FALSE/>, if either there is no legend at the diagram, or there
        is a legend which has a "Show" property of value <FALSE/>. Otherwise,
        <TRUE/> is returned.
     */
    static bool hasLegend( const rtl::Reference< ::chart::Diagram > & xDiagram );
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
