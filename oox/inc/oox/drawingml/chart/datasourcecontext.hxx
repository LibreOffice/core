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



#ifndef OOX_DRAWINGML_CHART_DATASOURCECONTEXT_HXX
#define OOX_DRAWINGML_CHART_DATASOURCECONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct DataSequenceModel;

typedef ContextBase< DataSequenceModel > DataSequenceContextBase;

// ============================================================================

/** Handler for a double sequence context (c:numLit, c:numRef elements).
 */
class DoubleSequenceContext : public DataSequenceContextBase
{
public:
    explicit            DoubleSequenceContext( ::oox::core::ContextHandler2Helper& rParent, DataSequenceModel& rModel );
    virtual             ~DoubleSequenceContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );

private:
    sal_Int32           mnPtIndex;          /// Current data point index.
};

// ============================================================================

/** Handler for a string sequence context (c:multiLvlStrRef, c:strLit,
    c:strRef elements).
 */
class StringSequenceContext : public DataSequenceContextBase
{
public:
    explicit            StringSequenceContext( ::oox::core::ContextHandler2Helper& rParent, DataSequenceModel& rModel );
    virtual             ~StringSequenceContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );

private:
    sal_Int32           mnPtIndex;          /// Current data point index.
};

// ============================================================================

struct DataSourceModel;

/** Handler for a data source context (c:bubbleSize, c:cat, c:minus, c:plus,
    c:val, c:xVal, c:yVal elements).
 */
class DataSourceContext : public ContextBase< DataSourceModel >
{
public:
    explicit            DataSourceContext( ::oox::core::ContextHandler2Helper& rParent, DataSourceModel& rModel );
    virtual             ~DataSourceContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
