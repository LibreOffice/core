/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <basic/sbx.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <vcl/image.hxx>
#include <svtools/textdata.hxx>
#include <tools/config.hxx>
#include <vcl/gradient.hxx>

#include <basic/ttstrhlp.hxx>

#include "brkpnts.hxx"
#include "basic.hrc"
#include "resids.hrc"
#include "basrid.hxx"

struct Breakpoint
{
    sal_uInt32 nLine;
    Breakpoint( sal_uInt32 nL ) { nLine = nL; }
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
    for ( size_t i = 0, n = BreakpointList.size(); i < n; ++i )
        delete BreakpointList[ i ];
    BreakpointList.clear();

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

    for ( size_t i = 0, n = BreakpointList.size(); i < n; ++i )
    {
        Breakpoint* pBrk = BreakpointList[ i ];
        pModule->SetBP( (sal_uInt16)pBrk->nLine );
#if OSL_DEBUG_LEVEL > 1
        DBG_ASSERT( !pModule->IsCompiled() || pModule->IsBP( (sal_uInt16)pBrk->nLine ), "Brechpunkt wurde nicht gesetzt" );
#endif
    }
    for ( sal_uInt16 nMethod = 0; nMethod < pModule->GetMethods()->Count(); nMethod++ )
    {
        SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( nMethod );
        DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
        pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
    }
}


void BreakpointWindow::InsertBreakpoint( sal_uInt32 nLine )
{
    Breakpoint* pNewBrk = new Breakpoint( nLine );

    for ( ::std::vector< Breakpoint* >::iterator i = BreakpointList.begin();
          i < BreakpointList.end();
          ++ i
        )
    {
        Breakpoint* pBrk = *i;
        if ( nLine <= pBrk->nLine )
        {
            if ( nLine != pBrk->nLine )
                BreakpointList.insert( i, pNewBrk );
            else
                delete pNewBrk;
            pNewBrk = NULL;
            break;
        }
    }

    // No insert position found => LIST_APPEND
    if ( pNewBrk )
        BreakpointList.push_back( pNewBrk );

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


Breakpoint* BreakpointWindow::FindBreakpoint( sal_uInt32 nLine )
{
    for ( size_t i = 0, n = BreakpointList.size(); i < n; ++i )
    {
        Breakpoint* pBP = BreakpointList[ i ];
        if ( pBP->nLine == nLine )
            return pBP;
    }
    return NULL;
}


void BreakpointWindow::AdjustBreakpoints( sal_uInt32 nLine, bool bInserted )
{
    if ( nLine == 0 ) //TODO: nLine == TEXT_PARA_ALL+1
        return;

    for ( size_t i = 0; i < BreakpointList.size(); )
    {
        Breakpoint* pBrk = BreakpointList[ i ];
        bool bDelBrk = false;

        if ( pBrk->nLine == nLine )
        {
            if ( bInserted )
                pBrk->nLine++;
            else
                bDelBrk = true;
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
            delete pBrk;
            ::std::vector< Breakpoint* >::iterator it = BreakpointList.begin();
            ::std::advance( it, i );
            BreakpointList.erase( it );
        }
        else
        {
            ++i;
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

    for ( size_t i = 0, n = BreakpointList.size(); i < n; ++i )
    {
        Breakpoint* pBrk = BreakpointList[ i ];
        if ( aBreakpoints.Len() )
            aBreakpoints += ';';
        aBreakpoints += ByteString::CreateFromInt32( pBrk->nLine );
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

    for ( size_t i = 0, n = BreakpointList.size(); i < n; ++i )
    {
        Breakpoint* pBrk = BreakpointList[ i ];
#if OSL_DEBUG_LEVEL > 1
        DBG_ASSERT( !pModule->IsCompiled() || pModule->IsBP( pBrk->nLine ), "Brechpunkt wurde nicht gesetzt" );
#endif
        sal_Int32 nLine = pBrk->nLine-1;
        sal_Int32 nY = nLine*nLineHeight - nCurYOffset;
        DrawImage( Point( 0, nY ) + aBmpOff, aBrk );
    }
    ShowMarker( sal_True );
}


Breakpoint* BreakpointWindow::FindBreakpoint( const Point& rMousePos )
{
    sal_Int32 nLineHeight = GetTextHeight();
    sal_Int32 nYPos = rMousePos.Y() + nCurYOffset;

    for ( size_t i = 0, n = BreakpointList.size(); i < n; ++i )
    {
        Breakpoint* pBrk = BreakpointList[ i ];
        sal_Int32 nLine = pBrk->nLine-1;
        sal_Int32 nY = nLine * nLineHeight;
        if ( ( nYPos > nY ) && ( nYPos < ( nY + nLineHeight ) ) )
            return pBrk;
    }
    return NULL;
}


void BreakpointWindow::ToggleBreakpoint( sal_uInt32 nLine )
{
    bool Removed = false;
    for ( size_t i = 0, n = BreakpointList.size(); i < n; ++i )
    {
        Breakpoint* pBP = BreakpointList[ i ];
        if ( pBP->nLine == nLine )              // remove
        {
            pModule->ClearBP( nLine );
            delete pBP;
            ::std::vector< Breakpoint* >::iterator it = BreakpointList.begin();
            ::std::advance( it, i );
            BreakpointList.erase( it );
            Removed = true;
            break;
        }
    }

    if ( !Removed )                             // create one
        InsertBreakpoint( nLine );

    Invalidate();
}

void BreakpointWindow::ShowMarker( bool bShow )
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


void BreakpointWindow::SetMarkerPos( sal_uInt32 nLine, bool bError )
{
    ShowMarker( false );   // Remove old one
    nMarkerPos = nLine;
    bErrorMarker = bError;
    ShowMarker( true );    // Draw new one
    Update();
}


void BreakpointWindow::Scroll( long nHorzScroll, long nVertScroll, sal_uInt16 nFlags )
{
    (void) nFlags; /* avoid warning about unused parameter */
    nCurYOffset -= nVertScroll;
    Window::Scroll( nHorzScroll, nVertScroll );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
