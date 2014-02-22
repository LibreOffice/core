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

#include <vcl/window.hxx>

#include "hintids.hxx"
#include "viewsh.hxx"
#include "virtoutp.hxx"
#include "viewopt.hxx"
#include "rootfrm.hxx"

#include "setmapvirtdev.hxx"

#if OSL_DEBUG_LEVEL > 1

class DbgRect
{
        OutputDevice *pOut;
public:
        DbgRect( OutputDevice *pOut, const Rectangle &rRect,
                 const ColorData eColor = COL_LIGHTBLUE );
};

inline DbgRect::DbgRect( OutputDevice *pOutDev, const Rectangle &rRect,
                         const ColorData eColor )
   :pOut( pOutDev )
{
    if( pOut )
    {
        pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
        pOut->SetLineColor( eColor );
        pOut->SetFillColor();
        pOut->DrawRect( rRect );
        pOut->Pop();
    }
}

#endif

/* The SWLayVout class manages the virtual output devices.
 * RootFrm has a static member of this class which is created in _FrmInit
 * and destroyed in _FrmFinit.
 * */

sal_Bool SwRootFrm::FlushVout()
{
    if( SwRootFrm::pVout->IsFlushable() )
    {
        SwRootFrm::pVout->_Flush();
        return sal_True;
    }
    return sal_False;
}

sal_Bool SwRootFrm::HasSameRect( const SwRect& rRect )
{
    if( SwRootFrm::pVout->IsFlushable() )
        return ( rRect == SwRootFrm::pVout->GetOrgRect() );
    return sal_False;
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

    @param _pMapMode
    input/output parameter - instance of the mapping mode, which will be set
    at the virtual output device.

    @param _rNewOrigin
    input parameter - constant instance of the origin, which will be used in
    the virtual output device
*/


void SetMappingForVirtDev(  const Point&    _rNewOrigin,
                            MapMode*        ,
                            const OutputDevice* _pOrgOutDev,
                            VirtualDevice*  _pVirDev )
{
        
        Point aPixelOffset = _pOrgOutDev->LogicToPixel( _rNewOrigin );
        _pVirDev->SetPixelOffset( Size( -aPixelOffset.X(), -aPixelOffset.Y() ) );
}


sal_Bool SwLayVout::DoesFit( const Size &rNew )
{
    if( rNew.Height() > VIRTUALHEIGHT )
        return sal_False;
    if( rNew.Width() <= 0 || rNew.Height() <= 0 )
        return sal_False;
    if( rNew.Width() <= aSize.Width() )
        return sal_True;
    if( !pVirDev )
    {
        pVirDev = new VirtualDevice();
        pVirDev->SetLineColor();
        if( pOut )
        {
            if( pVirDev->GetFillColor() != pOut->GetFillColor() )
                pVirDev->SetFillColor( pOut->GetFillColor() );
        }
    }

    if( rNew.Width() > aSize.Width() )
    {
        aSize.Width() = rNew.Width();
        if( !pVirDev->SetOutputSizePixel( aSize ) )
        {
            delete pVirDev;
            pVirDev = NULL;
            aSize.Width() = 0;
            return sal_False;
        }
    }
    return sal_True;
}






void SwLayVout::Enter(  SwViewShell *pShell, SwRect &rRect, sal_Bool bOn )
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
    if( bOn )
    {
        pSh = pShell;
        pOut = NULL;
        OutputDevice *pO = pSh->GetOut();

        if( OUTDEV_WINDOW != pO->GetOutDevType() )
            return;

        pOut = pO;
        Size aPixSz( pOut->PixelToLogic( Size( 1,1 )) );
        SwRect aTmp( rRect );
        aTmp.SSize().Width() += aPixSz.Width()/2 + 1;
        aTmp.SSize().Height()+= aPixSz.Height()/2 + 1;
        Rectangle aTmpRect( pO->LogicToPixel( aTmp.SVRect() ) );

        OSL_ENSURE( !pSh->GetWin()->IsReallyVisible() ||
                aTmpRect.GetWidth() <= pSh->GetWin()->GetOutputSizePixel().Width() + 2,
                "Paintwidth bigger than visarea?" );
        
        if( !DoesFit( aTmpRect.GetSize() ) )
        {
            pOut = NULL;
            return;
        }

        aRect = SwRect( pO->PixelToLogic( aTmpRect ) );

        SetOutDev( pSh, pVirDev );

        if( pVirDev->GetFillColor() != pOut->GetFillColor() )
            pVirDev->SetFillColor( pOut->GetFillColor() );

        MapMode aMapMode( pOut->GetMapMode() );
        
        
        ::SetMappingForVirtDev( aRect.Pos(), &aMapMode, pOut, pVirDev );

        if( aMapMode != pVirDev->GetMapMode() )
            pVirDev->SetMapMode( aMapMode );

        
        rRect = aRect;
    }
}

void SwLayVout::_Flush()
{
    OSL_ENSURE( pVirDev, "SwLayVout::DrawOut: nothing left Toulouse" );
    pOut->DrawOutDev( aRect.Pos(), aRect.SSize(),
                      aRect.Pos(), aRect.SSize(), *pVirDev );
    SetOutDev( pSh, pOut );
    pOut = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
