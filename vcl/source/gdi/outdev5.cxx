/*************************************************************************
 *
 *  $RCSfile: outdev5.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
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

#define _SV_OUTDEV_CXX
#include <tools/ref.hxx>
#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#else
#ifndef _SV_RMOUTDEV_HXX
#include <rmoutdev.hxx>
#endif
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_POLY_H
#include <poly.h>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_OUTDATA_HXX
#include <outdata.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif

// =======================================================================

DBG_NAMEEX( OutputDevice );

// =======================================================================

void OutputDevice::DrawRect( const Rectangle& rRect,
                             ULONG nHorzRound, ULONG nVertRound )
{
    DBG_TRACE( "OutputDevice::DrawRoundRect()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRoundRectAction( rRect, nHorzRound, nVertRound ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) )
        return;

    const Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;

    nHorzRound = ImplLogicWidthToDevicePixel( nHorzRound );
    nVertRound = ImplLogicHeightToDevicePixel( nVertRound );

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    if ( !nHorzRound && !nVertRound )
        mpGraphics->DrawRect( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight() );
    else
    {
        const Polygon aRoundRectPoly( aRect, nHorzRound, nVertRound );

        if ( aRoundRectPoly.GetSize() >= 2 )
        {
            const SalPoint* pPtAry = (const SalPoint*) aRoundRectPoly.ImplGetConstPointAry();

            if ( !mbFillColor )
                mpGraphics->DrawPolyLine( aRoundRectPoly.GetSize(), pPtAry );
            else
                mpGraphics->DrawPolygon( aRoundRectPoly.GetSize(), pPtAry );
        }
    }
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
        if ( !nHorzRound || !nVertRound )
            pGraphics->DrawRect( aRect );
        else
            pGraphics->DrawRect( aRect, nHorzRound, nVertRound );
    }
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawEllipse( const Rectangle& rRect )
{
    DBG_TRACE( "OutputDevice::DrawEllipse()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaEllipseAction( rRect ) );

    if  ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
    if ( aRect.IsEmpty() )
        return;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    Polygon aRectPoly( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );
    if ( aRectPoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = (const SalPoint*)aRectPoly.ImplGetConstPointAry();
        if ( !mbFillColor )
            mpGraphics->DrawPolyLine( aRectPoly.GetSize(), pPtAry );
        else
        {
            if ( mbInitFillColor )
                ImplInitFillColor();
            mpGraphics->DrawPolygon( aRectPoly.GetSize(), pPtAry );
        }
    }
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
        pGraphics->DrawEllipse( aRect );
    }
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawArc( const Rectangle& rRect,
                            const Point& rStartPt, const Point& rEndPt )
{
    DBG_TRACE( "OutputDevice::DrawArc()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaArcAction( rRect, rStartPt, rEndPt ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
    if ( aRect.IsEmpty() )
        return;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    const Point     aStart( ImplLogicToDevicePixel( rStartPt ) );
    const Point     aEnd( ImplLogicToDevicePixel( rEndPt ) );
    Polygon         aArcPoly( aRect, aStart, aEnd, POLY_ARC );

    if ( aArcPoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = (const SalPoint*)aArcPoly.ImplGetConstPointAry();
        mpGraphics->DrawPolyLine( aArcPoly.GetSize(), pPtAry );
    }
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
        pGraphics->DrawArc( aRect,
                            ImplLogicToDevicePixel( rStartPt ),
                            ImplLogicToDevicePixel( rEndPt ) );
    }
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawPie( const Rectangle& rRect,
                            const Point& rStartPt, const Point& rEndPt )
{
    DBG_TRACE( "OutputDevice::DrawPie()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPieAction( rRect, rStartPt, rEndPt ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
    if ( aRect.IsEmpty() )
        return;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    const Point     aStart( ImplLogicToDevicePixel( rStartPt ) );
    const Point     aEnd( ImplLogicToDevicePixel( rEndPt ) );
    Polygon         aPiePoly( aRect, aStart, aEnd, POLY_PIE );

    if ( aPiePoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = (const SalPoint*)aPiePoly.ImplGetConstPointAry();
        if ( !mbFillColor )
            mpGraphics->DrawPolyLine( aPiePoly.GetSize(), pPtAry );
        else
        {
            if ( mbInitFillColor )
                ImplInitFillColor();
            mpGraphics->DrawPolygon( aPiePoly.GetSize(), pPtAry );
        }
    }
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
        pGraphics->DrawPie( aRect,
                            ImplLogicToDevicePixel( rStartPt ),
                            ImplLogicToDevicePixel( rEndPt ) );
    }
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawChord( const Rectangle& rRect,
                              const Point& rStartPt, const Point& rEndPt )
{
    DBG_TRACE( "OutputDevice::DrawChord()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaChordAction( rRect, rStartPt, rEndPt ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
    if ( aRect.IsEmpty() )
        return;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    const Point     aStart( ImplLogicToDevicePixel( rStartPt ) );
    const Point     aEnd( ImplLogicToDevicePixel( rEndPt ) );
    Polygon         aChordPoly( aRect, aStart, aEnd, POLY_CHORD );

    if ( aChordPoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = (const SalPoint*)aChordPoly.ImplGetConstPointAry();
        if ( !mbFillColor )
            mpGraphics->DrawPolyLine( aChordPoly.GetSize(), pPtAry );
        else
        {
            if ( mbInitFillColor )
                ImplInitFillColor();
            mpGraphics->DrawPolygon( aChordPoly.GetSize(), pPtAry );
        }
    }
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
        pGraphics->DrawChord( aRect,
                              ImplLogicToDevicePixel( rStartPt ),
                              ImplLogicToDevicePixel( rEndPt ) );
    }
#endif
}
