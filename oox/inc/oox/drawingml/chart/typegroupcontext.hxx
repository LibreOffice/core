/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_CHART_TYPEGROUPCONTEXT_HXX
#define OOX_DRAWINGML_CHART_TYPEGROUPCONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct UpDownBarsModel;

/** Handler for an up/down bars context (c:upDownBars element).
 */
class UpDownBarsContext : public ContextBase< UpDownBarsModel >
{
public:
    explicit            UpDownBarsContext( ::oox::core::ContextHandler2Helper& rParent, UpDownBarsModel& rModel );
    virtual             ~UpDownBarsContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct TypeGroupModel;
typedef ContextBase< TypeGroupModel > TypeGroupContextBase;

// ============================================================================

/** Handler for area type group contexts (c:area3DChart, c:areaChart elements).
 */
class AreaTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            AreaTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~AreaTypeGroupContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for bar type group contexts (c:bar3DChart, c:barChart elements).
 */
class BarTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            BarTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~BarTypeGroupContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for bubble type group context (c:bubbleChart element).
 */
class BubbleTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            BubbleTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~BubbleTypeGroupContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for line type group contexts (c:line3DChart, c:lineChart,
    c:stockChart elements).
 */
class LineTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            LineTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~LineTypeGroupContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for pie type group contexts (c:doughnutChart, c:ofPieChart,
    c:pie3DChart, c:pieChart elements).
 */
class PieTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            PieTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~PieTypeGroupContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for radar type group context (c:radarChart element).
 */
class RadarTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            RadarTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~RadarTypeGroupContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for scatter type group context (c:scatterChart element).
 */
class ScatterTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            ScatterTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~ScatterTypeGroupContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for surface type group contexts (c:surface3DChart, c:surfaceChart
    elements).
 */
class SurfaceTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            SurfaceTypeGroupContext( ::oox::core::ContextHandler2Helper& rParent, TypeGroupModel& rModel );
    virtual             ~SurfaceTypeGroupContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

