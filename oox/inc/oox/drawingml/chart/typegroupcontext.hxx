/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typegroupcontext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:51:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_CHART_TYPEGROUPCONTEXT_HXX
#define OOX_DRAWINGML_CHART_TYPEGROUPCONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

class TypeGroupModel;
typedef ChartContextBase< TypeGroupModel > TypeGroupContextBase;

// ============================================================================

/** Handler for area type group contexts (c:area3DChart, c:areaChart elements).
 */
class AreaTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            AreaTypeGroupContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            TypeGroupModel& rModel );
    virtual             ~AreaTypeGroupContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for bar type group contexts (c:bar3DChart, c:barChart elements).
 */
class BarTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            BarTypeGroupContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            TypeGroupModel& rModel );
    virtual             ~BarTypeGroupContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for bubble type group context (c:bubbleChart element).
 */
class BubbleTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            BubbleTypeGroupContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            TypeGroupModel& rModel );
    virtual             ~BubbleTypeGroupContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for line type group contexts (c:line3DChart, c:lineChart,
    c:stockChart elements).
 */
class LineTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            LineTypeGroupContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            TypeGroupModel& rModel );
    virtual             ~LineTypeGroupContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for pie type group contexts (c:doughnutChart, c:ofPieChart,
    c:pie3DChart, c:pieChart elements).
 */
class PieTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            PieTypeGroupContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            TypeGroupModel& rModel );
    virtual             ~PieTypeGroupContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for radar type group context (c:radarChart element).
 */
class RadarTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            RadarTypeGroupContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            TypeGroupModel& rModel );
    virtual             ~RadarTypeGroupContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for scatter type group context (c:scatterChart element).
 */
class ScatterTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            ScatterTypeGroupContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            TypeGroupModel& rModel );
    virtual             ~ScatterTypeGroupContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for surface type group contexts (c:surface3DChart, c:surfaceChart
    elements).
 */
class SurfaceTypeGroupContext : public TypeGroupContextBase
{
public:
    explicit            SurfaceTypeGroupContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            TypeGroupModel& rModel );
    virtual             ~SurfaceTypeGroupContext();

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

