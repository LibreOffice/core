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

#include <PositionAndSizeHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <tools/gen.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <ChartModel.hxx>
#include <Diagram.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

bool PositionAndSizeHelper::moveObject( ObjectType eObjectType
                , const uno::Reference< beans::XPropertySet >& xObjectProp
                , const awt::Rectangle& rNewPositionAndSize
                , const awt::Rectangle& rOldPositionAndSize
                , const awt::Rectangle& rPageRectangle
                )
{
    if(!xObjectProp.is())
        return false;
    tools::Rectangle aObjectRect( Point(rNewPositionAndSize.X,rNewPositionAndSize.Y), Size(rNewPositionAndSize.Width,rNewPositionAndSize.Height) );
    tools::Rectangle aPageRect( Point(rPageRectangle.X,rPageRectangle.Y), Size(rPageRectangle.Width,rPageRectangle.Height) );

    // every following branch divides by width and height
    if (aPageRect.getOpenWidth() == 0 || aPageRect.getOpenHeight() == 0)
        return false;

    if( eObjectType==OBJECTTYPE_TITLE )
    {
        //@todo decide whether x is primary or secondary
        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_CENTER;
        //the anchor point at the title object is top/middle
        Point aPos = aObjectRect.TopLeft();
        aRelativePosition.Primary = (double(aPos.X())+double(aObjectRect.getOpenWidth())/2.0)/double(aPageRect.getOpenWidth());
        aRelativePosition.Secondary = (double(aPos.Y())+double(aObjectRect.getOpenHeight())/2.0)/double(aPageRect.getOpenHeight());
        xObjectProp->setPropertyValue( u"RelativePosition"_ustr, uno::Any(aRelativePosition) );
    }
    else if( eObjectType == OBJECTTYPE_DATA_LABEL )
    {
        RelativePosition aAbsolutePosition;
        RelativePosition aCustomLabelPosition;
        aAbsolutePosition.Primary = double(rOldPositionAndSize.X) / double(aPageRect.getOpenWidth());
        aAbsolutePosition.Secondary = double(rOldPositionAndSize.Y) / double(aPageRect.getOpenHeight());

        if( xObjectProp->getPropertyValue(u"CustomLabelPosition"_ustr) >>= aCustomLabelPosition )
        {
            aAbsolutePosition.Primary -= aCustomLabelPosition.Primary;
            aAbsolutePosition.Secondary -= aCustomLabelPosition.Secondary;
        }

        //the anchor point at the data label object is top/left
        Point aPos = aObjectRect.TopLeft();
        double fRotation = 0.0;
        xObjectProp->getPropertyValue(u"TextRotation"_ustr) >>= fRotation;
        if( fRotation == 90.0 )
            aPos = aObjectRect.BottomLeft();
        else if( fRotation == 270.0 )
            aPos = aObjectRect.TopRight();

        aCustomLabelPosition.Primary = double(aPos.X()) / double(aPageRect.getOpenWidth()) - aAbsolutePosition.Primary;
        aCustomLabelPosition.Secondary = double(aPos.Y()) / double(aPageRect.getOpenHeight()) - aAbsolutePosition.Secondary;
        xObjectProp->setPropertyValue(u"CustomLabelPosition"_ustr, uno::Any(aCustomLabelPosition));
    }
    else if( eObjectType==OBJECTTYPE_DATA_CURVE_EQUATION )
    {
        //@todo decide whether x is primary or secondary
        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
        //the anchor point at the title object is top/middle
        Point aPos = aObjectRect.TopLeft();
        aRelativePosition.Primary = double(aPos.X())/double(aPageRect.getOpenWidth());
        aRelativePosition.Secondary = double(aPos.Y())/double(aPageRect.getOpenHeight());
        xObjectProp->setPropertyValue( u"RelativePosition"_ustr, uno::Any(aRelativePosition) );
    }
    else if(eObjectType==OBJECTTYPE_LEGEND)
    {
        xObjectProp->setPropertyValue( u"Expansion"_ustr, uno::Any(css::chart::ChartLegendExpansion_CUSTOM));
        chart2::RelativePosition aRelativePosition;
        chart2::RelativeSize aRelativeSize;
        Point aAnchor = aObjectRect.TopLeft();

        aRelativePosition.Primary =
            static_cast< double >( aAnchor.X()) /
            static_cast< double >( aPageRect.getOpenWidth() );
        aRelativePosition.Secondary =
            static_cast< double >( aAnchor.Y()) /
            static_cast< double >( aPageRect.getOpenHeight());

        xObjectProp->setPropertyValue( u"RelativePosition"_ustr, uno::Any(aRelativePosition) );

        aRelativeSize.Primary =
            static_cast< double >( aObjectRect.getOpenWidth()) /
            static_cast< double >( aPageRect.getOpenWidth() );
        if (aRelativeSize.Primary > 1.0)
            aRelativeSize.Primary = 1.0;
        aRelativeSize.Secondary =
            static_cast< double >( aObjectRect.getOpenHeight()) /
            static_cast< double >( aPageRect.getOpenHeight());
        if (aRelativeSize.Secondary > 1.0)
            aRelativeSize.Secondary = 1.0;

        xObjectProp->setPropertyValue( u"RelativeSize"_ustr, uno::Any(aRelativeSize) );
    }
    else if(eObjectType==OBJECTTYPE_DIAGRAM || eObjectType==OBJECTTYPE_DIAGRAM_WALL || eObjectType==OBJECTTYPE_DIAGRAM_FLOOR)
    {
        //@todo decide whether x is primary or secondary

        //set position:
        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_CENTER;

        Point aPos = aObjectRect.Center();
        aRelativePosition.Primary = double(aPos.X())/double(aPageRect.getOpenWidth());
        aRelativePosition.Secondary = double(aPos.Y())/double(aPageRect.getOpenHeight());
        xObjectProp->setPropertyValue( u"RelativePosition"_ustr, uno::Any(aRelativePosition) );

        //set size:
        RelativeSize aRelativeSize;
        //the anchor points for the diagram are in the middle of the diagram
        //and in the middle of the page
        aRelativeSize.Primary = double(aObjectRect.getOpenWidth())/double(aPageRect.getOpenWidth());
        aRelativeSize.Secondary = double(aObjectRect.getOpenHeight())/double(aPageRect.getOpenHeight());
        xObjectProp->setPropertyValue( u"RelativeSize"_ustr, uno::Any(aRelativeSize) );
    }
    else
        return false;
    return true;
}

bool PositionAndSizeHelper::moveObject( std::u16string_view rObjectCID
                , const rtl::Reference<::chart::ChartModel>& xChartModel
                , const awt::Rectangle& rNewPositionAndSize
                , const awt::Rectangle& rOldPositionAndSize
                , const awt::Rectangle& rPageRectangle
                )
{
    ControllerLockGuardUNO aLockedControllers( xChartModel );

    awt::Rectangle aNewPositionAndSize( rNewPositionAndSize );

    uno::Reference< beans::XPropertySet > xObjectProp = ObjectIdentifier::getObjectPropertySet( rObjectCID, xChartModel );
    ObjectType eObjectType( ObjectIdentifier::getObjectType( rObjectCID ) );
    if(eObjectType==OBJECTTYPE_DIAGRAM || eObjectType==OBJECTTYPE_DIAGRAM_WALL || eObjectType==OBJECTTYPE_DIAGRAM_FLOOR)
    {
        xObjectProp = ObjectIdentifier::getDiagramForCID( rObjectCID, xChartModel );
        if(!xObjectProp.is())
            return false;
    }
    return moveObject( eObjectType, xObjectProp, aNewPositionAndSize, rOldPositionAndSize, rPageRectangle );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
