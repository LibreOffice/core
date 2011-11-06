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



#ifndef OOX_DRAWINGML_CHART_TITLECONTEXT_HXX
#define OOX_DRAWINGML_CHART_TITLECONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct TextModel;

/** Handler for a chart text context (c:tx element).
 */
class TextContext : public ContextBase< TextModel >
{
public:
    explicit            TextContext(  ::oox::core::ContextHandler2Helper& rParent, TextModel& rModel );
    virtual             ~TextContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );
};

// ============================================================================

struct TitleModel;

/** Handler for a chart title context (c:title element).
 */
class TitleContext : public ContextBase< TitleModel >
{
public:
    explicit            TitleContext( ::oox::core::ContextHandler2Helper& rParent, TitleModel& rModel );
    virtual             ~TitleContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct LegendModel;

/** Handler for a chart legend context (c:legend element).
 */
class LegendContext : public ContextBase< LegendModel >
{
public:
    explicit            LegendContext( ::oox::core::ContextHandler2Helper& rParent, LegendModel& rModel );
    virtual             ~LegendContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
