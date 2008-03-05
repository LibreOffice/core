/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartcontextbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:47:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

