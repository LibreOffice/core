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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <vcl/window.hxx>

#include "hintids.hxx"
#include "viewsh.hxx"
#include "virtoutp.hxx"
#include "viewopt.hxx"
#include "rootfrm.hxx"
// OD 12.11.2002 #96272# - include declaration for <SetMappingForVirtDev>
#include "setmapvirtdev.hxx"

#ifdef DBG_UTIL

/*************************************************************************
 *                          class DbgRect
 *************************************************************************/

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

/* class SwLayVout verwaltet das virtuelle Outputdevice
 * Es gibt von dieser Klasse einen statischen Member am RootFrm,
 * dieser wird in _FrmInit angelegt und in _FrmFinit zerstoert.
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

    @author OD

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
// define to control, if old or new solution for setting the mapping for
// an virtual output device is used.
void SetMappingForVirtDev(  const Point&    _rNewOrigin,
                            MapMode*        ,
                            const OutputDevice* _pOrgOutDev,
                            VirtualDevice*  _pVirDev )
{
        // new solution: set pixel offset at virtual output device
        Point aPixelOffset = _pOrgOutDev->LogicToPixel( _rNewOrigin );
        _pVirDev->SetPixelOffset( Size( -aPixelOffset.X(), -aPixelOffset.Y() ) );
}


/*************************************************************************
 *                          SwVOut::DoesFit()
 *************************************************************************/

// rSize muss in Pixel-Koordinaten vorliegen!
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

/*************************************************************************
 *                         SwLayVout::Enter
 *************************************************************************/
/// OD 27.09.2002 #103636# - change 2nd parameter <rRect> - no longer <const>
///     in order to return value of class member variable <aRect>, if virtual
///     output is used.
///     <aRect> contains the rectangle that represents the area the virtual
///     output device is used for and that is flushed at the end.
void SwLayVout::Enter(  ViewShell *pShell, SwRect &rRect, sal_Bool bOn )
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
// Auf dem Drucker oder einem virt. Outputdevice wird nicht getrickst...
        if( OUTDEV_WINDOW != pO->GetOutDevType() )
            return;

        pOut = pO;
        Size aPixSz( pOut->PixelToLogic( Size( 1,1 )) );
        SwRect aTmp( rRect );
        aTmp.SSize().Width() += aPixSz.Width()/2 + 1;
        aTmp.SSize().Height()+= aPixSz.Height()/2 + 1;
        Rectangle aTmpRect( pO->LogicToPixel( aTmp.SVRect() ) );

        ASSERT( !pSh->GetWin()->IsReallyVisible() ||
                aTmpRect.GetWidth() <= pSh->GetWin()->GetOutputSizePixel().Width() + 2,
                "Paintwidth bigger than visarea?" );
        // Passt das Rechteck in unseren Buffer ?
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
        // OD 12.11.2002 #96272# - use method to set mapping
        //aMapMode.SetOrigin( Point(0,0) - aRect.Pos() );
        ::SetMappingForVirtDev( aRect.Pos(), &aMapMode, pOut, pVirDev );

        if( aMapMode != pVirDev->GetMapMode() )
            pVirDev->SetMapMode( aMapMode );

        /// OD 27.09.2002 #103636# - set value of parameter <rRect>
        rRect = aRect;
    }
}

/*************************************************************************
 *                         SwLayVout::Flush()
 *************************************************************************/

void SwLayVout::_Flush()
{
    ASSERT( pVirDev, "SwLayVout::DrawOut: nothing left Toulouse" );
    Rectangle aTmp( aRect.SVRect() );
    pOut->DrawOutDev( aRect.Pos(), aRect.SSize(),
                      aRect.Pos(), aRect.SSize(), *pVirDev );
    SetOutDev( pSh, pOut );
    pOut = NULL;
}


