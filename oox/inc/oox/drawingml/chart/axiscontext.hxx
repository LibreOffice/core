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



#ifndef OOX_DRAWINGML_CHART_AXISCONTEXT_HXX
#define OOX_DRAWINGML_CHART_AXISCONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct AxisDispUnitsModel;

/** Handler for a value axis display units context (c:dispUnits element).
 */
class AxisDispUnitsContext : public ContextBase< AxisDispUnitsModel >
{
public:
    explicit            AxisDispUnitsContext( ::oox::core::ContextHandler2Helper& rParent, AxisDispUnitsModel& rModel );
    virtual             ~AxisDispUnitsContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct AxisModel;

/** Base class for axis context handlers (c:catAx, c:dateAx, c:serAx, c:valAx
    elements).
 */
class AxisContextBase : public ContextBase< AxisModel >
{
public:
    explicit            AxisContextBase( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~AxisContextBase();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a category axis context (c:catAx element).
 */
class CatAxisContext : public AxisContextBase
{
public:
    explicit            CatAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~CatAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a date axis context (c:dateAx element).
 */
class DateAxisContext : public AxisContextBase
{
public:
    explicit            DateAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~DateAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a series axis context (c:serAx element).
 */
class SerAxisContext : public AxisContextBase
{
public:
    explicit            SerAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~SerAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a value axis context (c:valAx element).
 */
class ValAxisContext : public AxisContextBase
{
public:
    explicit            ValAxisContext( ::oox::core::ContextHandler2Helper& rParent, AxisModel& rModel );
    virtual             ~ValAxisContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
