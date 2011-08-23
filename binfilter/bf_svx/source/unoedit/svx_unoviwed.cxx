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



#include "unoviwed.hxx"
#include "editview.hxx"
#include "editeng.hxx"
#include "svdotext.hxx"
namespace binfilter {

SvxEditEngineViewForwarder::~SvxEditEngineViewForwarder()
{
}

BOOL SvxEditEngineViewForwarder::IsValid() const
{
    return sal_True;
}

Rectangle SvxEditEngineViewForwarder::GetVisArea() const
{
    OutputDevice* pOutDev = mrView.GetWindow();

    if( pOutDev )
    {
        Rectangle aVisArea = mrView.GetVisArea();
        
        // figure out map mode from edit engine
        EditEngine* pEditEngine = mrView.GetEditEngine();

        if( pEditEngine )
        {
            MapMode aMapMode(pOutDev->GetMapMode());
            aVisArea = OutputDevice::LogicToLogic( aVisArea, 
                                                   pEditEngine->GetRefMapMode(), 
                                                   aMapMode.GetMapUnit() );
            aMapMode.SetOrigin(Point());
            return pOutDev->LogicToPixel( aVisArea, aMapMode );
        }
    }

    return Rectangle();
}

Point SvxEditEngineViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    OutputDevice* pOutDev = mrView.GetWindow();

    if( pOutDev )
    {
        MapMode aMapMode(pOutDev->GetMapMode());
        Point aPoint( OutputDevice::LogicToLogic( rPoint, rMapMode, 
                                                  aMapMode.GetMapUnit() ) );
        aMapMode.SetOrigin(Point());
        return pOutDev->LogicToPixel( aPoint, aMapMode );
    }

    return Point();
}

Point SvxEditEngineViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    OutputDevice* pOutDev = mrView.GetWindow();

    if( pOutDev )
    {
        MapMode aMapMode(pOutDev->GetMapMode());
        aMapMode.SetOrigin(Point());
        Point aPoint( pOutDev->PixelToLogic( rPoint, aMapMode ) );
        return OutputDevice::LogicToLogic( aPoint, 
                                           aMapMode.GetMapUnit(), 
                                           rMapMode );
    }

    return Point();
}

sal_Bool SvxEditEngineViewForwarder::GetSelection( ESelection& rSelection ) const
{
    rSelection = mrView.GetSelection();
    return sal_True;
}

sal_Bool SvxEditEngineViewForwarder::SetSelection( const ESelection& rSelection )
{
    mrView.SetSelection( rSelection );
    return sal_True;
}

sal_Bool SvxEditEngineViewForwarder::Copy()
{
    mrView.Copy();
    return sal_True;
}

sal_Bool SvxEditEngineViewForwarder::Cut()
{
    mrView.Cut();
    return sal_True;
}

sal_Bool SvxEditEngineViewForwarder::Paste()
{
    mrView.Paste();
    return sal_True;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
