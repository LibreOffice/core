/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chartcontextbase.hxx,v $
 * $Revision: 1.3 $
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

#define C_TOKEN( token )    (::oox::NMSP_CHART | XML_##token)

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

template< typename ModelType >
class ModelAccess
{
public:
    inline explicit     ModelAccess( ModelType& rModel ) : mrModel( rModel ) {}
    virtual             ~ModelAccess() {}

    inline ModelType&   getModel() { return mrModel; }
    inline const ModelType& getModel() const { return mrModel; }

private:
    ModelType&          mrModel;
};

// ============================================================================

template< typename ModelType >
class ChartContextBase : public ::oox::core::ContextHandler2, public ModelAccess< ModelType >
{
public:
    inline explicit     ChartContextBase( ::oox::core::ContextHandler2Helper& rParent, ModelType& rModel ) :
                            ::oox::core::ContextHandler2( rParent ), ModelAccess< ModelType >( rModel ) {}
    virtual             ~ChartContextBase() {}
};

// ============================================================================

template< typename ModelType >
class ChartFragmentBase : public ::oox::core::FragmentHandler2, public ModelAccess< ModelType >
{
public:
    explicit            ChartFragmentBase( ::oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, ModelType& rModel ) :
                            ::oox::core::FragmentHandler2( rFilter, rFragmentPath ), ModelAccess< ModelType >( rModel ) {}
    virtual             ~ChartFragmentBase() {}
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

