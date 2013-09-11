/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    explicit            ContextBase( ::oox::core::ContextHandler2Helper& rParent, ModelType& rModel ) :
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
    explicit            FragmentBase( ::oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath, ModelType& rModel ) :
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
