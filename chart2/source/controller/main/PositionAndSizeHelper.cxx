/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PositionAndSizeHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:34:34 $
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

#include "PositionAndSizeHelper.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "macros.hxx"
#include "ChartModelHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_LEGENDPOSITION_HPP_
#include <com/sun/star/chart2/LegendPosition.hpp>
#endif

#ifndef _COM_SUN_STAR_LAYOUT_RELATIVEPOSITION_HPP_
#include <com/sun/star/layout/RelativePosition.hpp>
#endif
#ifndef _COM_SUN_STAR_LAYOUT_RELATIVESIZE_HPP_
#include <com/sun/star/layout/RelativeSize.hpp>
#endif

// header for class Rectangle
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

bool PositionAndSizeHelper::moveObject( const rtl::OUString& rObjectCID
                , const uno::Reference< frame::XModel >& xChartModel
                , const awt::Rectangle& rNewPositionAndSize
                , const awt::Rectangle& rPageRectangle
                )
{
    uno::Reference< beans::XPropertySet > xProp = ObjectIdentifier::getObjectPropertySet( rObjectCID, xChartModel );
    if(!xProp.is())
        return false;
    Rectangle aObjectRect( Point(rNewPositionAndSize.X,rNewPositionAndSize.Y), Size(rNewPositionAndSize.Width,rNewPositionAndSize.Height) );
    Rectangle aPageRect( Point(rPageRectangle.X,rPageRectangle.Y), Size(rPageRectangle.Width,rPageRectangle.Height) );

    if(rObjectCID.indexOf(C2U("Title"))!=-1)
    {
        //@todo decide wether x is primary or secondary
        ::com::sun::star::layout::RelativePosition aRelativePosition;
        //the anchor point at the title object is top/middle
        Point aPos = aObjectRect.TopLeft();
        aRelativePosition.Primary = (double(aPos.X())+double(aObjectRect.getWidth())/2.0)/double(aPageRect.getWidth());
        aRelativePosition.Secondary = double(aPos.Y())/double(aPageRect.getHeight());
        xProp->setPropertyValue( C2U( "RelativePosition" ), uno::makeAny(aRelativePosition) );
    }
    else if(rObjectCID.indexOf(C2U("Legend"))!=-1)
    {
        LegendPosition ePos = LegendPosition_LINE_END;
        xProp->getPropertyValue( C2U( "AnchorPosition" )) >>= ePos;
        Point aLegendAnchor(0,0);//point at legend
        Point aPageAnchor(0,0);//point at page
        ::com::sun::star::layout::RelativePosition aRelativePosition;
        switch( ePos )
        {
            case LegendPosition_LINE_START:
                //@todo language dependent positions ...
                aLegendAnchor = aObjectRect.LeftCenter();
                aPageAnchor = aPageRect.LeftCenter();
                aRelativePosition.Primary = aLegendAnchor.X()-aPageAnchor.X();
                aRelativePosition.Secondary = aPageAnchor.Y()-aLegendAnchor.Y();
                aRelativePosition.Primary /= double(aPageRect.getWidth());
                aRelativePosition.Secondary /= double(aPageRect.getHeight());
                break;
            case LegendPosition_LINE_END:
                //@todo language dependent positions ...
                aLegendAnchor = aObjectRect.RightCenter();
                aPageAnchor = aPageRect.RightCenter();
                aRelativePosition.Primary = aPageAnchor.X()-aLegendAnchor.X();
                aRelativePosition.Secondary = aLegendAnchor.Y()-aPageAnchor.Y();
                aRelativePosition.Primary /= double(aPageRect.getWidth());
                aRelativePosition.Secondary /= double(aPageRect.getHeight());
                break;
            case LegendPosition_PAGE_START:
                //@todo language dependent positions ...
                aLegendAnchor = aObjectRect.TopCenter();
                aPageAnchor = aPageRect.TopCenter();
                aRelativePosition.Primary = aLegendAnchor.Y()-aPageAnchor.Y();
                aRelativePosition.Secondary = aLegendAnchor.X()-aPageAnchor.X();
                aRelativePosition.Primary /= double(aPageRect.getHeight());
                aRelativePosition.Secondary /= double(aPageRect.getWidth());
                break;
            case LegendPosition_PAGE_END:
            case LegendPosition_CUSTOM:
            case LegendPosition_MAKE_FIXED_SIZE:
                //@todo language dependent positions ...
                aLegendAnchor = aObjectRect.BottomCenter();
                aPageAnchor = aPageRect.BottomCenter();
                aRelativePosition.Primary = aPageAnchor.Y()-aLegendAnchor.Y();
                aRelativePosition.Secondary = aPageAnchor.X()-aLegendAnchor.X();
                aRelativePosition.Primary /= double(aPageRect.getHeight());
                aRelativePosition.Secondary /= double(aPageRect.getWidth());
                break;
        }
        xProp->setPropertyValue( C2U( "RelativePosition" ), uno::makeAny(aRelativePosition) );
    }
    else if(rObjectCID.indexOf(C2U("Diagram"))!=-1)
    {
        xProp = uno::Reference< beans::XPropertySet >( ChartModelHelper::findDiagram( xChartModel ), uno::UNO_QUERY );
        if(!xProp.is())
            return false;

        //@todo decide wether x is primary or secondary

        //set position:
        Point aPos = aObjectRect.TopLeft();
        ::com::sun::star::layout::RelativePosition aRelativePosition;
        //the anchor points for the diagram are in the middle of the diagram
        //and in the middle of the page
        aRelativePosition.Primary = (double(aPos.X())+double(aObjectRect.getWidth())/2.0-double(aPageRect.getWidth())/2.0)/double(aPageRect.getWidth());
        aRelativePosition.Secondary = (double(aPos.Y())+double(aObjectRect.getHeight())/2.0-double(aPageRect.getHeight())/2.0)/double(aPageRect.getHeight());
        xProp->setPropertyValue( C2U( "RelativePosition" ), uno::makeAny(aRelativePosition) );

        //set size:
        ::com::sun::star::layout::RelativeSize aRelativeSize;
        //the anchor points for the diagram are in the middle of the diagram
        //and in the middle of the page
        aRelativeSize.Primary = double(aObjectRect.getWidth())/double(aPageRect.getWidth());
        aRelativeSize.Secondary = double(aObjectRect.getHeight())/double(aPageRect.getHeight());
        xProp->setPropertyValue( C2U( "RelativeSize" ), uno::makeAny(aRelativeSize) );
    }
    else
        return false;
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................
