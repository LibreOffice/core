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



#ifndef OOX_DRAWINGML_CHART_CHARTCONTEXTBASE_HXX
#define OOX_DRAWINGML_CHART_CHARTCONTEXTBASE_HXX

#include "oox/core/fragmenthandler2.hxx"

namespace oox { namespace drawingml { class Shape; } }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

template< typename ModelType >
class ContextBase : public ::oox::core::ContextHandler2
{
public:
    inline explicit     ContextBase( ::oox::core::ContextHandler2Helper& rParent, ModelType& rModel ) :
                            ::oox::core::ContextHandler2( rParent ), mrModel( rModel ) {}
    virtual             ~ContextBase() {}

protected:
    ModelType&          mrModel;
};

// ============================================================================

template< typename ModelType >
class FragmentBase : public ::oox::core::FragmentHandler2
{
public:
    explicit            FragmentBase( ::oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, ModelType& rModel ) :
                            ::oox::core::FragmentHandler2( rFilter, rFragmentPath, false ), mrModel( rModel ) {}
    virtual             ~FragmentBase() {}

protected:
    ModelType&          mrModel;
};

// ============================================================================

/** Help class for all contexts that have only the c:spPr child element.
 */
class ShapePrWrapperContext : public ContextBase< Shape >
{
public:
    explicit            ShapePrWrapperContext( ::oox::core::ContextHandler2Helper& rParent, Shape& rModel );
    virtual             ~ShapePrWrapperContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct LayoutModel;

/** Handler for a chart layout context (c:layout element).
 */
class LayoutContext : public ContextBase< LayoutModel >
{
public:
    explicit            LayoutContext( ::oox::core::ContextHandler2Helper& rParent, LayoutModel& rModel );
    virtual             ~LayoutContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
