/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: seriescontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:35:52 $
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

#include "oox/drawingml/chart/seriescontext.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"
#include "oox/drawingml/chart/seriessourcecontext.hxx"

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextWrapper;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

SeriesContext::SeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel,
        sal_Int32 nTitleSrcId, sal_Int32 nCategSrcId, sal_Int32 nValueSrcId, sal_Int32 nPointSrcId ) :
    ChartContextBase< SeriesModel >( rParent, rModel ),
    mnTitleSrcId( nTitleSrcId ),
    mnCategSrcId( nCategSrcId ),
    mnValueSrcId( nValueSrcId ),
    mnPointSrcId( nPointSrcId )
{
}

SeriesContext::~SeriesContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper SeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            if( nElement == mnTitleSrcId )
                return new SeriesSourceContext( *this, getModel().createTitleSource() );
            if( nElement == mnCategSrcId )
                return new SeriesSourceContext( *this, getModel().createCategorySource() );
            if( nElement == mnValueSrcId )
                return new SeriesSourceContext( *this, getModel().createValueSource() );
            if( nElement == mnPointSrcId )
                return new SeriesSourceContext( *this, getModel().createPointSource() );
            switch( nElement )
            {
                case C_TOKEN( idx ):
                    getModel().getData().mnIndex = rAttribs.getInteger( XML_val, -1 );
                    return false;
                case C_TOKEN( order ):
                    getModel().getData().mnOrder = rAttribs.getInteger( XML_val, -1 );
                    return false;
            }
        break;
    }
    return false;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

