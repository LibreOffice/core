/*************************************************************************
 *
 *  $RCSfile: virtoutp.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:16:27 $
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


#pragma hdrstop

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#include "hintids.hxx"
#include "viewsh.hxx"
#include "virtoutp.hxx"
#include "viewopt.hxx"
#include "rootfrm.hxx"
// OD 12.11.2002 #96272# - include declaration for <SetMappingForVirtDev>
#include "setmapvirtdev.hxx"

static const ViewShell *pVsh = 0;

#ifndef PRODUCT

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

BOOL SwRootFrm::FlushVout()
{
    if( SwRootFrm::pVout->IsFlushable() )
    {
        SwRootFrm::pVout->_Flush();
        return TRUE;
    }
    return FALSE;
}

BOOL SwRootFrm::HasSameRect( const SwRect& rRect )
{
    if( SwRootFrm::pVout->IsFlushable() )
        return ( rRect == SwRootFrm::pVout->GetOrgRect() );
    return FALSE;
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
#define USE_NEW_MAPPING
void SetMappingForVirtDev(  const Point&    _rNewOrigin,
                            MapMode*        _pMapMode,
                            const OutputDevice* _pOrgOutDev,
                            VirtualDevice*  _pVirDev )
{
#ifndef USE_NEW_MAPPING
        // old solution: set origin at the mapping mode
        _pMapMode->SetOrigin( Point(0,0) - _rNewOrigin );
#else
        // new solution: set pixel offset at virtual output device
        Point aPixelOffset = _pOrgOutDev->LogicToPixel( _rNewOrigin );
        _pVirDev->SetPixelOffset( Size( -aPixelOffset.X(), -aPixelOffset.Y() ) );
#endif
}


/*************************************************************************
 *                          SwVOut::DoesFit()
 *************************************************************************/

// rSize muss in Pixel-Koordinaten vorliegen!
BOOL SwLayVout::DoesFit( const Size &rNew )
{
    if( rNew.Height() > VIRTUALHEIGHT )
        return FALSE;
    if( rNew.Width() <= 0 || rNew.Height() <= 0 )
        return FALSE;
    if( rNew.Width() <= aSize.Width() )
        return TRUE;
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
            return FALSE;
        }
    }
    return TRUE;
}

/*************************************************************************
 *                         SwLayVout::Enter
 *************************************************************************/
/// OD 27.09.2002 #103636# - change 2nd parameter <rRect> - no longer <const>
///     in order to return value of class member variable <aRect>, if virtual
///     output is used.
///     <aRect> contains the rectangle that represents the area the virtual
///     output device is used for and that is flushed at the end.
void SwLayVout::Enter(  ViewShell *pShell, SwRect &rRect, BOOL bOn )
{
    Flush();

#ifndef PRODUCT
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

        ASSERT( aTmpRect.GetWidth() <=
                pSh->GetWin()->GetOutputSizePixel().Width() + 2,
                "Paintwidth bigger than visarea?" );
        // Passt das Rechteck in unseren Buffer ?
        if( !DoesFit( aTmpRect.GetSize() ) )
        {
            pOut = NULL;
            return;
        }

        aRect = SwRect( pO->PixelToLogic( aTmpRect ) );

#ifdef USED
        Rectangle aBla( rRect.SVRect() );
        DbgRect aDbg1( pO, aBla, COL_LIGHTRED );
#endif
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
#ifdef USED
    DbgRect aDbg1( pOut, aTmp );
    DbgRect aDbg2( pVirDev, aTmp, COL_YELLOW );
#endif
    pOut->DrawOutDev( aRect.Pos(), aRect.SSize(),
                      aRect.Pos(), aRect.SSize(), *pVirDev );
    SetOutDev( pSh, pOut );
    pOut = NULL;
}


