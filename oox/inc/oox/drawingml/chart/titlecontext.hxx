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

#ifndef OOX_DRAWINGML_CHART_TITLECONTEXT_HXX
#define OOX_DRAWINGML_CHART_TITLECONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

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

struct TextModel;

/** Handler for a chart text context (c:tx element).
 */
class TextContext : public ContextBase< TextModel >
{
public:
    explicit            TextContext(  ::oox::core::ContextHandler2Helper& rParent, TextModel& rModel );
    virtual             ~TextContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );
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

