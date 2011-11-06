/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef OOX_DRAWINGML_CHART_PLOTAREACONTEXT_HXX
#define OOX_DRAWINGML_CHART_PLOTAREACONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct View3DModel;

/** Handler for a chart plot area context (c:plotArea element).
 */
class View3DContext : public ContextBase< View3DModel >
{
public:
    explicit            View3DContext( ::oox::core::ContextHandler2Helper& rParent, View3DModel& rModel );
    virtual             ~View3DContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct WallFloorModel;

/** Handler for a chart wall/floor context (c:backWall, c:floor, c:sideWall
    elements).
 */
class WallFloorContext : public ContextBase< WallFloorModel >
{
public:
    explicit            WallFloorContext( ::oox::core::ContextHandler2Helper& rParent, WallFloorModel& rModel );
    virtual             ~WallFloorContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct PlotAreaModel;

/** Handler for a chart plot area context (c:plotArea element).
 */
class PlotAreaContext : public ContextBase< PlotAreaModel >
{
public:
    explicit            PlotAreaContext( ::oox::core::ContextHandler2Helper& rParent, PlotAreaModel& rModel );
    virtual             ~PlotAreaContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
