/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

#include <editeng/unoviwed.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>

SvxEditEngineViewForwarder::SvxEditEngineViewForwarder( EditView& rView ) :
    mrView( rView )
{
}

SvxEditEngineViewForwarder::~SvxEditEngineViewForwarder()
{
}

sal_Bool SvxEditEngineViewForwarder::IsValid() const
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
