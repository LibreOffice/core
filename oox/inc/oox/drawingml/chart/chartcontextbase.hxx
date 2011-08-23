/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
