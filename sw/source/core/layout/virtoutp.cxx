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

#include "virtoutp.hxx"
#include <viewopt.hxx>
#include <rootfrm.hxx>

/* The SWLayVout class manages the virtual output devices.
 * RootFrame has a static member of this class which is created in FrameInit
 * and destroyed in FrameFinit.
 * */

bool SwRootFrame::FlushVout()
{
    if (SwRootFrame::s_pVout->IsFlushable())
    {
        SwRootFrame::s_pVout->Flush_();
        return true;
    }
    return false;
}

bool SwRootFrame::HasSameRect( const SwRect& rRect )
{
    if (SwRootFrame::s_pVout->IsFlushable())
        return ( rRect == SwRootFrame::s_pVout->GetOrgRect() );
    return false;
}

/** method to set mapping/pixel offset for virtual output device

    OD 12.11.2002 #96272# - method implements two solutions for the mapping of
    the virtual output device:
    The old solution set the origin of the mapping mode, which will be used in
    the virtual output device. This causes several paint errors, because of the
    different roundings in the virtual output device and the original output device.
    The new solution avoids the rounding differences between virtual and original
    output device by setting a pixel offset at the virtual output device.
    A define controls, which solution is used, in order to switch in escalation
    back to old solution.

    @param _pOrgOutDev
    input parameter - constant instance of the original output device, for which
    the virtual output device is created.

    @param _pVirDev
    input/output parameter - instance of the virtual output device.

    @param _rNewOrigin
    input parameter - constant instance of the origin, which will be used in
    the virtual output device
*/
// define to control, if old or new solution for setting the mapping for
// an virtual output device is used.
static void SetMappingForVirtDev(  const Point&    _rNewOrigin,
                            const vcl::RenderContext* _pOrgOutDev,
                            vcl::RenderContext*  _pVirDev )
{
        // new solution: set pixel offset at virtual output device
        Point aPixelOffset = _pOrgOutDev->LogicToPixel( _rNewOrigin );
        _pVirDev->SetPixelOffset( Size( -aPixelOffset.X(), -aPixelOffset.Y() ) );
}

// rSize must be pixel coordinates!
bool SwLayVout::DoesFit( const Size &rNew )
{
    if( rNew.Height() > VIRTUALHEIGHT )
        return false;
    if( rNew.Width() <= 0 || rNew.Height() <= 0 )
        return false;
    if( rNew.Width() <= aSize.Width() )
        return true;
    if( !pVirDev )
    {
        pVirDev = VclPtr<VirtualDevice>::Create();
        pVirDev->SetLineColor();
        if( pOut )
        {
            if( pVirDev->GetFillColor() != pOut->GetFillColor() )
                pVirDev->SetFillColor( pOut->GetFillColor() );
        }
    }

    if( rNew.Width() > aSize.Width() )
    {
        aSize.setWidth( rNew.Width() );
        if( !pVirDev->SetOutputSizePixel( aSize ) )
        {
            pVirDev.disposeAndClear();
            aSize.setWidth( 0 );
            return false;
        }
    }
    return true;
}

/// OD 27.09.2002 #103636# - change 2nd parameter <rRect> - no longer <const>
///     in order to return value of class member variable <aRect>, if virtual
///     output is used.
///     <aRect> contains the rectangle that represents the area the virtual
///     output device is used for and that is flushed at the end.
void SwLayVout::Enter(  SwViewShell *pShell, SwRect &rRect, bool bOn )
{
    Flush();

#ifdef DBG_UTIL
    if( pShell->GetViewOptions()->IsTest3() )
    {
        ++nCount;
        return;
    }
#endif

    bOn = bOn && !nCount && rRect.HasArea() && pShell->GetWin();
    ++nCount;
    if( !bOn )
        return;

    pSh = pShell;
    pOut = nullptr;
    OutputDevice *pO = pSh->GetOut();
// We don't cheat on printers or virtual output devices...
    if( OUTDEV_WINDOW != pO->GetOutDevType() )
        return;

    pOut = pO;
    Size aPixSz( pOut->PixelToLogic( Size( 1,1 )) );
    SwRect aTmp( rRect );
    aTmp.SSize().AdjustWidth(aPixSz.Width()/2 + 1 );
    aTmp.SSize().AdjustHeight(aPixSz.Height()/2 + 1 );
    tools::Rectangle aTmpRect( pO->LogicToPixel( aTmp.SVRect() ) );

    OSL_ENSURE( !pSh->GetWin()->IsReallyVisible() ||
            aTmpRect.GetWidth() <= pSh->GetWin()->GetOutputSizePixel().Width() + 2,
            "Paintwidth bigger than visarea?" );
    // Does the rectangle fit in our buffer?
    if( !DoesFit( aTmpRect.GetSize() ) )
    {
        pOut = nullptr;
        return;
    }

    aRect = SwRect( pO->PixelToLogic( aTmpRect ) );

    SetOutDev( pSh, pVirDev );

    if( pVirDev->GetFillColor() != pOut->GetFillColor() )
        pVirDev->SetFillColor( pOut->GetFillColor() );

    MapMode aMapMode( pOut->GetMapMode() );
    // OD 12.11.2002 #96272# - use method to set mapping
    //aMapMode.SetOrigin( Point(0,0) - aRect.Pos() );
    ::SetMappingForVirtDev( aRect.Pos(), pOut, pVirDev );

    if( aMapMode != pVirDev->GetMapMode() )
        pVirDev->SetMapMode( aMapMode );

    /// OD 27.09.2002 #103636# - set value of parameter <rRect>
    rRect = aRect;

}

void SwLayVout::Flush_()
{
    OSL_ENSURE( pVirDev, "SwLayVout::DrawOut: nothing left Toulouse" );
    pOut->DrawOutDev( aRect.Pos(), aRect.SSize(),
                      aRect.Pos(), aRect.SSize(), *pVirDev );
    SetOutDev( pSh, pOut );
    pOut = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
