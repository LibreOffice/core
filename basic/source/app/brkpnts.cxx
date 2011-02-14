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
#include "precompiled_basic.hxx"
#include <tools/list.hxx>
#include <basic/sbx.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <vcl/image.hxx>
#include <svtools/textdata.hxx>
#include <tools/config.hxx>
#include <vcl/gradient.hxx>

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

#include "brkpnts.hxx"
#include "basic.hrc"
#include "resids.hrc"
#include "basrid.hxx"

struct Breakpoint
{
    sal_uInt16 nLine;

    Breakpoint( sal_uInt16 nL ) { nLine = nL; }
};


ImageList* BreakpointWindow::pImages = NULL;


BreakpointWindow::BreakpointWindow( Window *pParent )
: Window( pParent )
, nCurYOffset( 0 )
, nMarkerPos( MARKER_NOMARKER )
, pModule( NULL )
, bErrorMarker( sal_False )
{
    if ( !pImages )
        pImages = new ImageList( SttResId( RID_IMGLST_LAYOUT ) );

    Gradient aGradient( GRADIENT_AXIAL, Color( 185, 182, 215 ), Color( 250, 245, 255 ) );
    aGradient.SetAngle(900);
    SetBackground( aGradient );
    Show();
}


void BreakpointWindow::Reset()
{
    Breakpoint* pBrk = First();
    while ( pBrk )
    {
        delete pBrk;
        pBrk = Next();
    }
    Clear();

    pModule->ClearAllBP();
}

void BreakpointWindow::SetModule( SbModule *pMod )
{
    pModule = pMod;
    sal_uInt16 i;
    for ( i=0 ; i < pModule->GetBPCount() ; i++ )
    {
        InsertBreakpoint( pModule->GetBP( i ) );
    }
    SetBPsInModule();
}


void BreakpointWindow::SetBPsInModule()
{
    pModule->ClearAllBP();

    Breakpoint* pBrk = First();
    while ( pBrk )
    {
        pModule->SetBP( (sal_uInt16)pBrk->nLine );
#if OSL_DEBUG_LEVEL > 1
        DBG_ASSERT( !pModule->IsCompiled() || pModule->IsBP( (USHORT)pBrk->nLine ), "Brechpunkt wurde nicht gesetzt" );
#endif
        pBrk = Next();
    }
    for ( sal_uInt16 nMethod = 0; nMethod < pModule->GetMethods()->Count(); nMethod++ )
    {
        SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( nMethod );
        DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
        pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
    }
}


void BreakpointWindow::InsertBreakpoint( sal_uInt16 nLine )
{
    Breakpoint* pNewBrk = new Breakpoint( nLine );
    Breakpoint* pBrk = First();
    while ( pBrk )
    {
        if ( nLine <= pBrk->nLine )
        {
            if ( pBrk->nLine != nLine )
                Insert( pNewBrk );
            else
                delete pNewBrk;
            pNewBrk = NULL;
            pBrk = NULL;
        }
        else
            pBrk = Next();
    }
    // No insert position found => LIST_APPEND
    if ( pNewBrk )
        Insert( pNewBrk, LIST_APPEND );

    Invalidate();

    if ( pModule->SetBP( nLine ) )
    {
#if OSL_DEBUG_LEVEL > 1
        DBG_ASSERT( !pModule->IsCompiled() || pModule->IsBP( nLine ), "Brechpunkt wurde nicht gesetzt" );
#endif
        if ( StarBASIC::IsRunning() )
        {
            for ( sal_uInt16 nMethod = 0; nMethod < pModule->GetMethods()->Count(); nMethod++ )
            {
                SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( nMethod );
                DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
                pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
            }
        }
    }
#if OSL_DEBUG_LEVEL > 1
    DBG_ASSERT( !pModule->IsCompiled() || pModule->IsBP( nLine ), "Brechpunkt wurde nicht gesetzt" );
#endif
}


Breakpoint* BreakpointWindow::FindBreakpoint( sal_uIntPtr nLine )
{
    Breakpoint* pBrk = First();
    while ( pBrk )
    {
        if ( pBrk->nLine == nLine )
            return pBrk;

        pBrk = Next();
    }

    return (Breakpoint*)0;
}


void BreakpointWindow::AdjustBreakpoints( sal_uIntPtr nLine, sal_Bool bInserted )
{
    if ( nLine == 0 ) //TODO: nLine == TEXT_PARA_ALL+1
        return;
    Breakpoint* pBrk = First();
    while ( pBrk )
    {
        sal_Bool bDelBrk = sal_False;
        if ( pBrk->nLine == nLine )
        {
            if ( bInserted )
                pBrk->nLine++;
            else
                bDelBrk = sal_True;
        }
        else if ( pBrk->nLine > nLine )
        {
            if ( bInserted )
                pBrk->nLine++;
            else
                pBrk->nLine--;
        }

        if ( bDelBrk )
        {
            sal_uIntPtr n = GetCurPos();
            delete Remove( pBrk );
            pBrk = Seek( n );
        }
        else
        {
            pBrk = Next();
        }
    }
    Invalidate();
}


void BreakpointWindow::LoadBreakpoints( String aFilename )
{
    Config aConfig(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));

    aConfig.SetGroup("Breakpoints");

    ByteString aBreakpoints;
    aBreakpoints = aConfig.ReadKey( ByteString( aFilename, RTL_TEXTENCODING_UTF8 ) );

    xub_StrLen i;

    for ( i = 0 ; i < aBreakpoints.GetTokenCount( ';' ) ; i++ )
    {
        InsertBreakpoint( (sal_uInt16)aBreakpoints.GetToken( i, ';' ).ToInt32() );
    }
}


void BreakpointWindow::SaveBreakpoints( String aFilename )
{
    ByteString aBreakpoints;

    Breakpoint* pBrk = First();
    while ( pBrk )
    {
        if ( aBreakpoints.Len() )
            aBreakpoints += ';';

        aBreakpoints += ByteString::CreateFromInt32( pBrk->nLine );
        pBrk = Next();
    }

    Config aConfig(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));

    aConfig.SetGroup("Breakpoints");

    if ( aBreakpoints.Len() )
        aConfig.WriteKey( ByteString( aFilename, RTL_TEXTENCODING_UTF8 ), aBreakpoints );
    else
        aConfig.DeleteKey( ByteString( aFilename, RTL_TEXTENCODING_UTF8 ) );
}


void BreakpointWindow::Paint( const Rectangle& )
{
    Size aOutSz( GetOutputSize() );
    long nLineHeight = GetTextHeight();

    Image aBrk( pImages->GetImage( IMGID_BRKENABLED ) );
    Size aBmpSz( aBrk.GetSizePixel() );
    aBmpSz = PixelToLogic( aBmpSz );
    Point aBmpOff( 0, 0 );
    aBmpOff.X() = ( aOutSz.Width() - aBmpSz.Width() ) / 2;
    aBmpOff.Y() = ( nLineHeight - aBmpSz.Height() ) / 2;

    Breakpoint* pBrk = First();
    while ( pBrk )
    {
#if OSL_DEBUG_LEVEL > 1
        DBG_ASSERT( !pModule->IsCompiled() || pModule->IsBP( pBrk->nLine ), "Brechpunkt wurde nicht gesetzt" );
#endif
        sal_uIntPtr nLine = pBrk->nLine-1;
        sal_uIntPtr nY = nLine*nLineHeight - nCurYOffset;
        DrawImage( Point( 0, nY ) + aBmpOff, aBrk );
        pBrk = Next();
    }
    ShowMarker( sal_True );
}


Breakpoint* BreakpointWindow::FindBreakpoint( const Point& rMousePos )
{
    long nLineHeight = GetTextHeight();
    long nYPos = rMousePos.Y() + nCurYOffset;

    Breakpoint* pBrk = First();
    while ( pBrk )
    {
        sal_uIntPtr nLine = pBrk->nLine-1;
        long nY = nLine*nLineHeight;
        if ( ( nYPos > nY ) && ( nYPos < ( nY + nLineHeight ) ) )
            return pBrk;
        pBrk = Next();
    }
    return 0;
}


void BreakpointWindow::ToggleBreakpoint( sal_uInt16 nLine )
{
    Breakpoint* pBrk = FindBreakpoint( nLine );
    if ( pBrk ) // remove
    {
        pModule->ClearBP( nLine );
        delete Remove( pBrk );
    }
    else // create one
    {
        InsertBreakpoint( nLine );
    }

    Invalidate();
}

void BreakpointWindow::ShowMarker( sal_Bool bShow )
{
    if ( nMarkerPos == MARKER_NOMARKER )
        return;

    Size aOutSz( GetOutputSize() );
    long nLineHeight = GetTextHeight();

    Image aMarker;
    if ( bErrorMarker )
        aMarker = pImages->GetImage( IMGID_ERRORMARKER );
    else
        aMarker = pImages->GetImage( IMGID_STEPMARKER );

    Size aMarkerSz( aMarker.GetSizePixel() );
    aMarkerSz = PixelToLogic( aMarkerSz );
    Point aMarkerOff( 0, 0 );
    aMarkerOff.X() = ( aOutSz.Width() - aMarkerSz.Width() ) / 2;
    aMarkerOff.Y() = ( nLineHeight - aMarkerSz.Height() ) / 2;

    sal_uIntPtr nY = nMarkerPos*nLineHeight - nCurYOffset;
    Point aPos( 0, nY );
    aPos += aMarkerOff;
    if ( bShow )
        DrawImage( aPos, aMarker );
    else
        Invalidate( Rectangle( aPos, aMarkerSz ) );
}


void BreakpointWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() == 2 )
    {
        Point aMousePos( PixelToLogic( rMEvt.GetPosPixel() ) );
        long nLineHeight = GetTextHeight();
        long nYPos = aMousePos.Y() + nCurYOffset;
        long nLine = nYPos / nLineHeight + 1;
        ToggleBreakpoint( sal::static_int_cast< sal_uInt16 >(nLine) );
        Invalidate();
    }
}


void BreakpointWindow::SetMarkerPos( sal_uInt16 nLine, sal_Bool bError )
{
    ShowMarker( sal_False );   // Remove old one
    nMarkerPos = nLine;
    bErrorMarker = bError;
    ShowMarker( sal_True );    // Draw new one
    Update();
}


void BreakpointWindow::Scroll( long nHorzScroll, long nVertScroll, sal_uInt16 nFlags )
{
    (void) nFlags; /* avoid warning about unused parameter */
    nCurYOffset -= nVertScroll;
    Window::Scroll( nHorzScroll, nVertScroll );
}

