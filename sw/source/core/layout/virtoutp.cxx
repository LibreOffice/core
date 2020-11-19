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
#include <osl/diagnose.h>

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
// a virtual output device is used.
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
    if( rNew.IsEmpty() )
        return false;
    if( rNew.Width() <= m_aSize.Width() )
        return true;
    if( !m_pVirDev )
    {
        m_pVirDev = VclPtr<VirtualDevice>::Create();
        m_pVirDev->SetLineColor();
        if( m_pOut )
        {
            if( m_pVirDev->GetFillColor() != m_pOut->GetFillColor() )
                m_pVirDev->SetFillColor( m_pOut->GetFillColor() );
        }
    }

    if( rNew.Width() > m_aSize.Width() )
    {
        m_aSize.setWidth( rNew.Width() );
        if( !m_pVirDev->SetOutputSizePixel( m_aSize ) )
        {
            m_pVirDev.disposeAndClear();
            m_aSize.setWidth( 0 );
            return false;
        }
    }
    return true;
}

///     change 2nd parameter <rRect> - no longer <const>
///     in order to return value of class member variable <aRect>, if virtual
///     output is used.
///    <aRect> contains the rectangle that represents the area the virtual
///     output device is used for and that is flushed at the end.
void SwLayVout::Enter(  SwViewShell *pShell, SwRect &rRect, bool bOn )
{
    Flush();

#ifdef DBG_UTIL
    if( pShell->GetViewOptions()->IsTest3() )
    {
        ++m_nCount;
        return;
    }
#endif

    bOn = bOn && !m_nCount && rRect.HasArea() && pShell->GetWin();
    ++m_nCount;
    if( !bOn )
        return;

    m_pShell = pShell;
    m_pOut = nullptr;
    OutputDevice *pO = m_pShell->GetOut();
// We don't cheat on printers or virtual output devices...
    if( OUTDEV_WINDOW != pO->GetOutDevType() )
        return;

    m_pOut = pO;
    Size aPixSz( m_pOut->PixelToLogic( Size( 1,1 )) );
    SwRect aTmp( rRect );
    aTmp.AddWidth(aPixSz.Width()/2 + 1 );
    aTmp.AddHeight(aPixSz.Height()/2 + 1 );
    tools::Rectangle aTmpRect( pO->LogicToPixel( aTmp.SVRect() ) );

    OSL_ENSURE( !m_pShell->GetWin()->IsReallyVisible() ||
            aTmpRect.GetWidth() <= m_pShell->GetWin()->GetOutputSizePixel().Width() + 2,
            "Paintwidth bigger than visarea?" );
    // Does the rectangle fit in our buffer?
    if( !DoesFit( aTmpRect.GetSize() ) )
    {
        m_pOut = nullptr;
        return;
    }

    m_aRect = SwRect( pO->PixelToLogic( aTmpRect ) );

    SetOutDev( m_pShell, m_pVirDev );

    if( m_pVirDev->GetFillColor() != m_pOut->GetFillColor() )
        m_pVirDev->SetFillColor( m_pOut->GetFillColor() );

    MapMode aMapMode( m_pOut->GetMapMode() );
    // use method to set mapping
    //aMapMode.SetOrigin( Point(0,0) - aRect.Pos() );
    ::SetMappingForVirtDev( m_aRect.Pos(), m_pOut, m_pVirDev );

    if( aMapMode != m_pVirDev->GetMapMode() )
        m_pVirDev->SetMapMode( aMapMode );

    // set value of parameter <rRect>
    rRect = m_aRect;

}

void SwLayVout::Flush_()
{
    OSL_ENSURE( m_pVirDev, "SwLayVout::DrawOut: nothing left Toulouse" );
    m_pOut->DrawOutDev( m_aRect.Pos(), m_aRect.SSize(),
                      m_aRect.Pos(), m_aRect.SSize(), *m_pVirDev );
    SetOutDev( m_pShell, m_pOut );
    m_pOut = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
