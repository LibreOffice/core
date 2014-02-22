/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

#include <editeng/unoviwou.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>

SvxDrawOutlinerViewForwarder::SvxDrawOutlinerViewForwarder( OutlinerView& rOutl ) :
    mrOutlinerView ( rOutl ), maTextShapeTopLeft()
{
}

SvxDrawOutlinerViewForwarder::SvxDrawOutlinerViewForwarder( OutlinerView& rOutl, const Point& rShapePosTopLeft ) :
    mrOutlinerView ( rOutl ), maTextShapeTopLeft( rShapePosTopLeft )
{
}

SvxDrawOutlinerViewForwarder::~SvxDrawOutlinerViewForwarder()
{
}

Point SvxDrawOutlinerViewForwarder::GetTextOffset() const
{
    
    Rectangle aOutputRect( mrOutlinerView.GetOutputArea() );

    return aOutputRect.TopLeft() - maTextShapeTopLeft;
}

sal_Bool SvxDrawOutlinerViewForwarder::IsValid() const
{
    return sal_True;
}

Rectangle SvxDrawOutlinerViewForwarder::GetVisArea() const
{
    OutputDevice* pOutDev = mrOutlinerView.GetWindow();

    if( pOutDev )
    {
        Rectangle aVisArea = mrOutlinerView.GetVisArea();

        Point aTextOffset( GetTextOffset() );
        aVisArea.Move( aTextOffset.X(), aTextOffset.Y() );

        
        Outliner* pOutliner = mrOutlinerView.GetOutliner();

        if( pOutliner )
        {
            MapMode aMapMode(pOutDev->GetMapMode());
            aVisArea = OutputDevice::LogicToLogic( aVisArea,
                                                   pOutliner->GetRefMapMode(),
                                                   aMapMode.GetMapUnit() );
            aMapMode.SetOrigin(Point());
            return pOutDev->LogicToPixel( aVisArea, aMapMode );
        }
    }

    return Rectangle();
}

Point SvxDrawOutlinerViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    OutputDevice* pOutDev = mrOutlinerView.GetWindow();

    if( pOutDev )
    {
        Point aPoint1( rPoint );
        Point aTextOffset( GetTextOffset() );

        aPoint1.X() += aTextOffset.X();
        aPoint1.Y() += aTextOffset.Y();

        MapMode aMapMode(pOutDev->GetMapMode());
        Point aPoint2( OutputDevice::LogicToLogic( aPoint1, rMapMode,
                                                   aMapMode.GetMapUnit() ) );
        aMapMode.SetOrigin(Point());
        return pOutDev->LogicToPixel( aPoint2, aMapMode );
    }

    return Point();
}

Point SvxDrawOutlinerViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    OutputDevice* pOutDev = mrOutlinerView.GetWindow();

    if( pOutDev )
    {
        MapMode aMapMode(pOutDev->GetMapMode());
        aMapMode.SetOrigin(Point());
        Point aPoint1( pOutDev->PixelToLogic( rPoint, aMapMode ) );
        Point aPoint2( OutputDevice::LogicToLogic( aPoint1,
                                                   aMapMode.GetMapUnit(),
                                                   rMapMode ) );
        Point aTextOffset( GetTextOffset() );

        aPoint2.X() -= aTextOffset.X();
        aPoint2.Y() -= aTextOffset.Y();

        return aPoint2;
    }

    return Point();
}

sal_Bool SvxDrawOutlinerViewForwarder::GetSelection( ESelection& rSelection ) const
{
    rSelection = mrOutlinerView.GetSelection();
    return sal_True;
}

sal_Bool SvxDrawOutlinerViewForwarder::SetSelection( const ESelection& rSelection )
{
    mrOutlinerView.SetSelection( rSelection );
    return sal_True;
}

sal_Bool SvxDrawOutlinerViewForwarder::Copy()
{
    mrOutlinerView.Copy();
    return sal_True;
}

sal_Bool SvxDrawOutlinerViewForwarder::Cut()
{
    mrOutlinerView.Cut();
    return sal_True;
}

sal_Bool SvxDrawOutlinerViewForwarder::Paste()
{
    mrOutlinerView.Paste();
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
