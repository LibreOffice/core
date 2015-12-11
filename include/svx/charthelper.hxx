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

#ifndef INCLUDED_SVX_CHARTHELPER_HXX
#define INCLUDED_SVX_CHARTHELPER_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <basegfx/range/b2drange.hxx>
#include <svx/svxdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

class SAL_WARN_UNUSED SVX_DLLPUBLIC ChartHelper
{
public:
    /// Check that the XDiagram is a real 3D chart.
    static bool isGL3DDiagram(const css::uno::Reference<css::chart2::XDiagram>& xDiagram);

    /// Use chart's XUpdatable::update() to update values.
    static void updateChart( const css::uno::Reference< css::frame::XModel >& rXModel, bool bHardUpdate );

    // try to access rXModel in case of a chart to get the chart content
    // as sequence of primitives. Return range of primitives (chart size) in rRange;
    // it will be used to embed the chart to the SdrObject transformation. This
    // allows to define possible distances between chart and SDrObject bounds here
    static drawinglayer::primitive2d::Primitive2DContainer tryToGetChartContentAsPrimitive2DSequence(
        const css::uno::Reference< css::frame::XModel >& rXModel,
        basegfx::B2DRange& rRange);

    // #i121334# Allow to switch off line and fill style by setting these as attributes
    // at the OLE chart object. This is needed to allow fill styles of the covering objects
    // to make their own fill/line settings work. This should not be done by changing
    // the defaults at the chart (see StaticPageBackgroundDefaults_Initializer::lcl_AddDefaultsToMap)
    // since this would not be saved/loaded, thus the compatibility will be better when setting it at
    // newly created charts using this method
    static void AdaptDefaultsForChart(
        const css::uno::Reference < css::embed::XEmbeddedObject > & xEmbObj,
        bool bNoFillStyle = true,
        bool bNoLineStyle = true);
};

#endif // INCLUDED_SVX_CHARTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
