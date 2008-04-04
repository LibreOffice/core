/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PositionAndSizeHelper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:58:43 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "PositionAndSizeHelper.hxx"
#include "macros.hxx"
#include "ChartModelHelper.hxx"
#include "ControllerLockGuard.hxx"

#ifndef _COM_SUN_STAR_CHART2_LEGENDPOSITION_HPP_
#include <com/sun/star/chart2/LegendPosition.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_RELATIVEPOSITION_HPP_
#include <com/sun/star/chart2/RelativePosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_RELATIVESIZE_HPP_
#include <com/sun/star/chart2/RelativeSize.hpp>
#endif
#include "chartview/ExplicitValueProvider.hxx"

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

bool PositionAndSizeHelper::moveObject( ObjectType eObjectType
                , const uno::Reference< beans::XPropertySet >& xObjectProp
                , const awt::Rectangle& rNewPositionAndSize
                , const awt::Rectangle& rPageRectangle
                )
{
    if(!xObjectProp.is())
        return false;
    Rectangle aObjectRect( Point(rNewPositionAndSize.X,rNewPositionAndSize.Y), Size(rNewPositionAndSize.Width,rNewPositionAndSize.Height) );
    Rectangle aPageRect( Point(rPageRectangle.X,rPageRectangle.Y), Size(rPageRectangle.Width,rPageRectangle.Height) );

    if( OBJECTTYPE_TITLE==eObjectType )
    {
        //@todo decide wether x is primary or secondary
        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_CENTER;
        //the anchor point at the title object is top/middle
        Point aPos = aObjectRect.TopLeft();
        aRelativePosition.Primary = (double(aPos.X())+double(aObjectRect.getWidth())/2.0)/double(aPageRect.getWidth());
        aRelativePosition.Secondary = (double(aPos.Y())+double(aObjectRect.getHeight())/2.0)/double(aPageRect.getHeight());
        xObjectProp->setPropertyValue( C2U( "RelativePosition" ), uno::makeAny(aRelativePosition) );
    }
    else if( OBJECTTYPE_DATA_CURVE_EQUATION==eObjectType )
    {
        //@todo decide wether x is primary or secondary
        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
        //the anchor point at the title object is top/middle
        Point aPos = aObjectRect.TopLeft();
        aRelativePosition.Primary = double(aPos.X())/double(aPageRect.getWidth());
        aRelativePosition.Secondary = double(aPos.Y())/double(aPageRect.getHeight());
        xObjectProp->setPropertyValue( C2U( "RelativePosition" ), uno::makeAny(aRelativePosition) );
    }
    else if(OBJECTTYPE_LEGEND==eObjectType)
    {
        LegendPosition ePos = LegendPosition_LINE_END;
        xObjectProp->getPropertyValue( C2U( "AnchorPosition" )) >>= ePos;
        chart2::RelativePosition aRelativePosition;
        Point aAnchor = aObjectRect.TopLeft();

        switch( ePos )
        {
            case LegendPosition_LINE_START:
                {
                    //@todo language dependent positions ...
                    aRelativePosition.Anchor = drawing::Alignment_LEFT;
                    aAnchor = aObjectRect.LeftCenter();
                }
                break;
            case LegendPosition_LINE_END:
                {
                    //@todo language dependent positions ...
                    aRelativePosition.Anchor = drawing::Alignment_RIGHT;
                    aAnchor = aObjectRect.RightCenter();
                }
                break;
            case LegendPosition_PAGE_START:
                {
                    //@todo language dependent positions ...
                    aRelativePosition.Anchor = drawing::Alignment_TOP;
                    aAnchor = aObjectRect.TopCenter();
                }
                break;
            case LegendPosition_PAGE_END:
                //@todo language dependent positions ...
                {
                    aRelativePosition.Anchor = drawing::Alignment_BOTTOM;
                    aAnchor = aObjectRect.BottomCenter();
                }
                break;
            case LegendPosition_CUSTOM:
                {
                    //@todo language dependent positions ...
                    aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
                }
                break;
            case LegendPosition_MAKE_FIXED_SIZE:
                OSL_ASSERT( false );
                break;
        }
        aRelativePosition.Primary =
            static_cast< double >( aAnchor.X()) /
            static_cast< double >( aPageRect.getWidth() );
        aRelativePosition.Secondary =
            static_cast< double >( aAnchor.Y()) /
            static_cast< double >( aPageRect.getHeight());

        xObjectProp->setPropertyValue( C2U( "RelativePosition" ), uno::makeAny(aRelativePosition) );
    }
    else if(OBJECTTYPE_DIAGRAM==eObjectType || OBJECTTYPE_DIAGRAM_WALL==eObjectType || OBJECTTYPE_DIAGRAM_FLOOR==eObjectType)
    {
        //@todo decide wether x is primary or secondary

        //xChartView

        //set position:
        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_CENTER;

        Point aPos = aObjectRect.Center();
        aRelativePosition.Primary = double(aPos.X())/double(aPageRect.getWidth());
        aRelativePosition.Secondary = double(aPos.Y())/double(aPageRect.getHeight());
        xObjectProp->setPropertyValue( C2U( "RelativePosition" ), uno::makeAny(aRelativePosition) );

        //set size:
        RelativeSize aRelativeSize;
        //the anchor points for the diagram are in the middle of the diagram
        //and in the middle of the page
        aRelativeSize.Primary = double(aObjectRect.getWidth())/double(aPageRect.getWidth());
        aRelativeSize.Secondary = double(aObjectRect.getHeight())/double(aPageRect.getHeight());
        xObjectProp->setPropertyValue( C2U( "RelativeSize" ), uno::makeAny(aRelativeSize) );
    }
    else
        return false;
    return true;
}

bool PositionAndSizeHelper::moveObject( const rtl::OUString& rObjectCID
                , const uno::Reference< frame::XModel >& xChartModel
                , const awt::Rectangle& rNewPositionAndSize
                , const awt::Rectangle& rPageRectangle
                , uno::Reference< uno::XInterface > xChartView
                )
{
    ControllerLockGuard aLockedControllers( xChartModel );

    awt::Rectangle aNewPositionAndSize( rNewPositionAndSize );

    uno::Reference< beans::XPropertySet > xObjectProp = ObjectIdentifier::getObjectPropertySet( rObjectCID, xChartModel );
    ObjectType eObjectType( ObjectIdentifier::getObjectType( rObjectCID ) );
    if(OBJECTTYPE_DIAGRAM==eObjectType || OBJECTTYPE_DIAGRAM_WALL==eObjectType || OBJECTTYPE_DIAGRAM_FLOOR==eObjectType)
    {
        xObjectProp = uno::Reference< beans::XPropertySet >( ObjectIdentifier::getDiagramForCID( rObjectCID, xChartModel ), uno::UNO_QUERY );
        if(!xObjectProp.is())
            return false;

        //add axis title sizes to the diagram size
        aNewPositionAndSize = ExplicitValueProvider::calculateDiagramPositionAndSizeInclusiveTitle(
            xChartModel, xChartView, rNewPositionAndSize );
    }
    return moveObject( eObjectType, xObjectProp, aNewPositionAndSize, rPageRectangle );
}

//.............................................................................
} //namespace chart
//.............................................................................
