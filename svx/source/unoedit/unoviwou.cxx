/*************************************************************************
 *
 *  $RCSfile: unoviwou.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: thb $ $Date: 2002-05-27 16:43:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#include "unoviwou.hxx"
#include "outliner.hxx"
#include "svdotext.hxx"


SvxDrawOutlinerViewForwarder::SvxDrawOutlinerViewForwarder( OutlinerView& rOutl ) :
    mrOutlinerView ( rOutl )
{
}

SvxDrawOutlinerViewForwarder::~SvxDrawOutlinerViewForwarder()
{
}

BOOL SvxDrawOutlinerViewForwarder::IsValid() const
{
    return sal_True;
}

Rectangle SvxDrawOutlinerViewForwarder::GetVisArea() const
{
    OutputDevice* pOutDev = mrOutlinerView.GetWindow();

    if( pOutDev )
    {
        Rectangle aVisArea = mrOutlinerView.GetVisArea();

        // figure out map mode from edit engine
        Outliner* pOutliner = mrOutlinerView.GetOutliner();

        if( pOutliner )
        {
            aVisArea = pOutDev->LogicToLogic( aVisArea, pOutliner->GetRefMapMode(), pOutDev->GetMapMode() );
            return pOutDev->LogicToPixel( aVisArea );
        }
    }

    return Rectangle();
}

Point SvxDrawOutlinerViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    OutputDevice* pOutDev = mrOutlinerView.GetWindow();

    if( pOutDev )
    {
        Point aPoint( pOutDev->LogicToLogic( rPoint, rMapMode, pOutDev->GetMapMode() ) );
        return pOutDev->LogicToPixel( aPoint );
    }

    return Point();
}

Point SvxDrawOutlinerViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    OutputDevice* pOutDev = mrOutlinerView.GetWindow();

    if( pOutDev )
    {
        Point aPoint( pOutDev->PixelToLogic( rPoint ) );
        return pOutDev->LogicToLogic( aPoint, pOutDev->GetMapMode(), rMapMode );
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
