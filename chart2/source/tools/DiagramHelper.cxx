/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DiagramHelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:28:21 $
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
#include "DiagramHelper.hxx"
#include "LegendHelper.hxx"
#include "PropertyHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif

using namespace ::com::sun::star;

namespace chart
{
namespace helper
{

// static
void DiagramHelper::changeDiagram(
    const uno::Reference< chart2::XDiagram > & xSource,
    uno::Reference< chart2::XDiagram > & xDestination )
{
    OSL_ASSERT( xSource.is() && xDestination.is());
    if( ! ( xSource.is() && xDestination.is()))
        return;

    uno::Reference< chart2::XTitled > xSrcTitled( xSource, uno::UNO_QUERY );
    uno::Reference< chart2::XTitled > xDestTitled( xDestination, uno::UNO_QUERY );

    if( xSrcTitled.is() && xDestTitled.is())
    {
        xDestTitled->setTitle( xSrcTitled->getTitle() );
    }

    uno::Reference< chart2::XLegend > xLegend( xSource->getLegend());
    LegendHelper::flushLegend( xLegend );
    LegendHelper::defaultFillEmptyLegend( xLegend, xDestination );
    xDestination->setLegend( xLegend );

    uno::Reference< beans::XPropertySet > xSrcProp(  xSource->getWall());
    uno::Reference< beans::XPropertySet > xDestProp( xDestination->getWall());
    if( xSrcProp.is() && xDestProp.is())
        PropertyHelper::copyProperties( xSrcProp, xDestProp );

    xSrcProp.set(  xSource->getFloor());
    xDestProp.set( xDestination->getFloor());
    if( xSrcProp.is() && xDestProp.is())
        PropertyHelper::copyProperties( xSrcProp, xDestProp );
}

} //  namespace helper
} //  namespace chart
