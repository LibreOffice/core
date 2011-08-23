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
    virtual void        onEndElement( const ::rtl::OUString& rChars );

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
    virtual void        onEndElement( const ::rtl::OUString& rChars );

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

