/*************************************************************************
 *
 *  $RCSfile: brkpnts.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:08 $
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

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <sbmod.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <sbstar.hxx>
#endif
#ifndef _SB_SBMETH_HXX //autogen
#include <sbmeth.hxx>
#endif
#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _TEXTDATA_HXX //autogen
#include <svtools/textdata.hxx>
#endif
#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif

#ifndef _BASIC_TTRESHLP_HXX
#include "ttstrhlp.hxx"
#endif

#include <brkpnts.hxx>
#include "basic.hrc"
#include "resids.hrc"

struct Breakpoint
{
    USHORT nLine;

    Breakpoint( USHORT nL ) { nLine = nL; }
};


ImageList* BreakpointWindow::pImages = NULL;


BreakpointWindow::BreakpointWindow( Window *pParent )
: Window( pParent )
, nCurYOffset( 0 )
, pModule( NULL )
{
    if ( !pImages )
        pImages = new ImageList( ResId( RID_IMGLST_LAYOUT ) );

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
    USHORT i;
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
        pModule->SetBP( (USHORT)pBrk->nLine );
#ifdef DEBUG
        DBG_ASSERT( pModule->IsBP( (USHORT)pBrk->nLine ), "Brechpunkt wurde nicht gesetzt" )
#endif
        pBrk = Next();
    }
    for ( USHORT nMethod = 0; nMethod < pModule->GetMethods()->Count(); nMethod++ )
    {
        SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( nMethod );
        DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
        pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
    }
}


void BreakpointWindow::InsertBreakpoint( USHORT nLine )
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
    // Keine Einfuegeposition gefunden => LIST_APPEND
    if ( pNewBrk )
        Insert( pNewBrk, LIST_APPEND );

    // vielleicht mal etwas genauer...
    Invalidate();

    if ( pModule->SetBP( nLine ) )
    {
#ifdef DEBUG
        DBG_ASSERT( pModule->IsBP( nLine ), "Brechpunkt wurde nicht gesetzt" )
#endif
        if ( StarBASIC::IsRunning() )
        {
            for ( USHORT nMethod = 0; nMethod < pModule->GetMethods()->Count(); nMethod++ )
            {
                SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( nMethod );
                DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
                pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
            }
        }
    }
#ifdef DEBUG
    DBG_ASSERT( pModule->IsBP( nLine ), "Brechpunkt wurde nicht gesetzt" )
#endif
}


Breakpoint* BreakpointWindow::FindBreakpoint( ULONG nLine )
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


void BreakpointWindow::AdjustBreakpoints( ULONG nLine, BOOL bInserted )
{
    if ( nLine == TEXT_PARA_ALL+1 )
        return;
    Breakpoint* pBrk = First();
    while ( pBrk )
    {
        BOOL bDelBrk = FALSE;
        if ( pBrk->nLine == nLine )
        {
            if ( bInserted )
                pBrk->nLine++;
            else
                bDelBrk = TRUE;
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
            ULONG n = GetCurPos();
            delete Remove( pBrk );
            pBrk = Seek( n );
        }
        else
        {
            pBrk = Next();
        }
    }
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
        InsertBreakpoint( aBreakpoints.GetToken( i, ';' ).ToInt32() );
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
#ifdef DEBUG
        DBG_ASSERT( pModule->IsBP( pBrk->nLine ), "Brechpunkt wurde nicht gesetzt" )
#endif
        ULONG nLine = pBrk->nLine-1;
        ULONG nY = nLine*nLineHeight - nCurYOffset;
        DrawImage( Point( 0, nY ) + aBmpOff, aBrk );
        pBrk = Next();
    }
    ShowMarker( TRUE );
}


Breakpoint* BreakpointWindow::FindBreakpoint( const Point& rMousePos )
{
    long nLineHeight = GetTextHeight();
    long nYPos = rMousePos.Y() + nCurYOffset;

    Breakpoint* pBrk = First();
    while ( pBrk )
    {
        ULONG nLine = pBrk->nLine-1;
        long nY = nLine*nLineHeight;
        if ( ( nYPos > nY ) && ( nYPos < ( nY + nLineHeight ) ) )
            return pBrk;
        pBrk = Next();
    }
    return 0;
}


void BreakpointWindow::ToggleBreakpoint( USHORT nLine )
{
    BOOL bNewBreakPoint = FALSE;
    Breakpoint* pBrk = FindBreakpoint( nLine );
    if ( pBrk ) // entfernen
    {
        pModule->ClearBP( nLine );
        delete Remove( pBrk );
    }
    else // einen erzeugen
    {
        InsertBreakpoint( nLine );
    }
    // vielleicht mal etwas genauer...
    Invalidate();
}

void BreakpointWindow::ShowMarker( BOOL bShow )
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

    ULONG nY = nMarkerPos*nLineHeight - nCurYOffset;
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
        ToggleBreakpoint( nLine );
        // vielleicht mal etwas genauer...
        Invalidate();
    }
}


void BreakpointWindow::SetMarkerPos( USHORT nLine, BOOL bError )
{
    ShowMarker( FALSE );    // Alten wegzeichen...
    nMarkerPos = nLine;
    bErrorMarker = bError;
    ShowMarker( TRUE );     // Neuen zeichnen...
    Update();
}


void BreakpointWindow::Scroll( long nHorzScroll, long nVertScroll )
{
    nCurYOffset -= nVertScroll;
    Window::Scroll( nHorzScroll, nVertScroll );
}

