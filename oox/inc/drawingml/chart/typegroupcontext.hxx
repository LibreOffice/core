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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_TYPEGROUPCONTEXT_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_TYPEGROUPCONTEXT_HXX

#include <drawingml/chart/chartcontextbase.hxx>

namespace oox::drawingml::chart {


struct UpDownBarsModel;

/** Handler for an up/down bars context (c:upDownBars element).
 */
class UpDownBarsContext final : public ContextBase< UpDownBarsModel >
{
public:
    explicit            UpDownBarsContext( ::oox::core::ContextHandler2Helper& rParent, UpDownBarsModel& rModel );
    virtual             ~UpDownBarsContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


struct TypeGroupModel;
typedef ContextBase< TypeGroupModel > TypeGroupContextBase;


/** Handler for area type group contexts (c:area3DChart, c:areaChart elements).
 */
class AreaTypeGroupContext final : public TypeGroupContextBase
{
public:
    explicit            AreaTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~AreaTypeGroupContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for bar type group contexts (c:bar3DChart, c:barChart elements).
 */
class BarTypeGroupContext final : public TypeGroupContextBase
{
public:
    explicit            BarTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~BarTypeGroupContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for bubble type group context (c:bubbleChart element).
 */
class BubbleTypeGroupContext final : public TypeGroupContextBase
{
public:
    explicit            BubbleTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~BubbleTypeGroupContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for line type group contexts (c:line3DChart, c:lineChart,
    c:stockChart elements).
 */
class LineTypeGroupContext final : public TypeGroupContextBase
{
public:
    explicit            LineTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~LineTypeGroupContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for pie type group contexts (c:doughnutChart, c:ofPieChart,
    c:pie3DChart, c:pieChart elements).
 */
class PieTypeGroupContext final : public TypeGroupContextBase
{
public:
    explicit            PieTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~PieTypeGroupContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for radar type group context (c:radarChart element).
 */
class RadarTypeGroupContext final : public TypeGroupContextBase
{
public:
    explicit            RadarTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~RadarTypeGroupContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for scatter type group context (c:scatterChart element).
 */
class ScatterTypeGroupContext final : public TypeGroupContextBase
{
public:
    explicit            ScatterTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~ScatterTypeGroupContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for surface type group contexts (c:surface3DChart, c:surfaceChart
    elements).
 */
class SurfaceTypeGroupContext final : public TypeGroupContextBase
{
public:
    explicit            SurfaceTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~SurfaceTypeGroupContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


} // namespace oox::drawingml::chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
