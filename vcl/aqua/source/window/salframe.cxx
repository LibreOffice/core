/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.48 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-06 09:56:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <string.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_VCLWINDOW_H
#include <VCLWindow.h>
#endif
#ifndef _SV_VCLGRAPHICS_H
#include <VCLGraphics.h>
#endif

// =======================================================================

static long ImplSalFrameCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
    return 0;
}

// =======================================================================

SalFrame::SalFrame()
{
    SalData* pSalData = GetSalData();

    maFrameData.mhWnd               = NULL;
    maFrameData.mpGraphics          = NULL;
    maFrameData.mpInst              = NULL;
    maFrameData.mpProc              = ImplSalFrameCallbackDummy;
    maFrameData.mnWidth             = 0;
    maFrameData.mnHeight            = 0;
    maFrameData.mbGraphics          = FALSE;

    // insert frame in framelist
    maFrameData.mpNextFrame = pSalData->mpFirstFrame;
    pSalData->mpFirstFrame = this;
}

// -----------------------------------------------------------------------

SalFrame::~SalFrame()
{
    SalData* pSalData = GetSalData();

    if ( maFrameData.mpGraphics )
        delete maFrameData.mpGraphics;

    if ( maFrameData.mhWnd )
        VCLWindow_Release( maFrameData.mhWnd );

    // remove frame from framelist
    if ( this == pSalData->mpFirstFrame )
        pSalData->mpFirstFrame = maFrameData.mpNextFrame;
    else
    {
        SalFrame* pTempFrame = pSalData->mpFirstFrame;
        while ( pTempFrame->maFrameData.mpNextFrame != this )
            pTempFrame = pTempFrame->maFrameData.mpNextFrame;

        pTempFrame->maFrameData.mpNextFrame = maFrameData.mpNextFrame;
    }
}

// -----------------------------------------------------------------------

SalGraphics* SalFrame::GetGraphics()
{
    if ( maFrameData.mbGraphics )
        return NULL;

    if ( !maFrameData.mpGraphics )
    {
        VCLVIEW hView = NULL;
        SalFrame *pFrame = this;

        // Search for the parent SalFrame that has a native window and
        // use that window to get an NSView
        while ( !pFrame->maFrameData.mhWnd ) {
            pFrame = pFrame->maFrameData.mpParent;
            if ( !pFrame )
                break;
        }
        hView = VCLWindow_ContentView( pFrame->maFrameData.mhWnd );

        if ( hView )
        {
            maFrameData.mpGraphics = new SalGraphics;
            maFrameData.mpGraphics->maGraphicsData.mhDC      = hView;
            maFrameData.mpGraphics->maGraphicsData.mbPrinter = FALSE;
            maFrameData.mpGraphics->maGraphicsData.mbVirDev  = FALSE;
            maFrameData.mpGraphics->maGraphicsData.mbWindow  = TRUE;
            maFrameData.mpGraphics->maGraphicsData.mbScreen  = TRUE;
        }
    }

    maFrameData.mbGraphics = TRUE;

    return maFrameData.mpGraphics;
}

// -----------------------------------------------------------------------

void SalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    maFrameData.mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL SalFrame::PostEvent( void *pData )
{
    VCLWINDOW hWindow = NULL;
    SalFrame *pFrame = this;

    // Search for the parent SalFrame that has a native window and
    // use that window to post the event to
    while ( !pFrame->maFrameData.mhWnd ) {
        pFrame = pFrame->maFrameData.mpParent;
        if ( !pFrame )
            break;
    }
    return VCLWindow_PostEvent( pFrame->maFrameData.mhWnd, pData );
}

// -----------------------------------------------------------------------

void SalFrame::SetTitle( const XubString& rTitle )
{
    ByteString aByteTitle( rTitle, gsl_getSystemTextEncoding() );
    char *pTitle = (char *)aByteTitle.GetBuffer();

    if ( maFrameData.mhWnd )
        VCLWindow_SetTitle( maFrameData.mhWnd, pTitle );
}

// -----------------------------------------------------------------------

void SalFrame::SetIcon( USHORT nIcon )
{
}

// -----------------------------------------------------------------------

void SalFrame::Show( BOOL bVisible )
{
    if ( bVisible )
    {
        if ( maFrameData.mhWnd )
            VCLWindow_Show( maFrameData.mhWnd );
    } // if
    else
    {
        if ( maFrameData.mhWnd )
        {
            VCLWindow_Close( maFrameData.mhWnd );
            if ( maFrameData.mpParent )
                maFrameData.mpParent->Show( TRUE );
        }
    } // else
} // SalFrame::Show

// -----------------------------------------------------------------------

void SalFrame::Enable( BOOL bEnable )
{
}

// -----------------------------------------------------------------------

void SalFrame::SetMinClientSize( long nWidth, long nHeight )
{
}

// -----------------------------------------------------------------------

void SalFrame::SetClientSize( long nWidth, long nHeight )
{
    maFrameData.mnWidth = nWidth;
    maFrameData.mnHeight = nHeight;

    // If this is a native window, resize it
    if ( maFrameData.mhWnd )
        VCLWindow_SetSize( maFrameData.mhWnd, nWidth, nHeight );
}

// -----------------------------------------------------------------------

void SalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    rWidth  = maFrameData.mnWidth;
    rHeight = maFrameData.mnHeight;
}

// -----------------------------------------------------------------------

void SalFrame::SetWindowState( const SalFrameState* pState )
{
}

// -----------------------------------------------------------------------

BOOL SalFrame::GetWindowState( SalFrameState* pState )
{
    return FALSE;
}

// -----------------------------------------------------------------------

void SalFrame::ShowFullScreen( BOOL /* bFullScreen */, sal_Int32 /* nDisplay */ )
{
}

// -----------------------------------------------------------------------

void SalFrame::StartPresentation( BOOL bStart )
{
}

// -----------------------------------------------------------------------

void SalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
}

// -----------------------------------------------------------------------

void SalFrame::ToTop( USHORT nFlags )
{
    if ( maFrameData.mhWnd )
        VCLWindow_Show( maFrameData.mhWnd );
}

// -----------------------------------------------------------------------

void SalFrame::SetPointer( PointerStyle ePointerStyle )
{
}

// -----------------------------------------------------------------------

void SalFrame::CaptureMouse( BOOL bCapture )
{
}

// -----------------------------------------------------------------------

void SalFrame::SetPointerPos( long nX, long nY )
{
}

// -----------------------------------------------------------------------

void SalFrame::Flush()
{
}

// -----------------------------------------------------------------------

void SalFrame::Sync()
{
}

// -----------------------------------------------------------------------

void SalFrame::SetInputContext( SalInputContext* pContext )
{
}

// -----------------------------------------------------------------------

void SalFrame::EndExtTextInput( USHORT nFlags )
{
}

// -----------------------------------------------------------------------

XubString SalFrame::GetKeyName( USHORT nKeyCode )
{
    return XubString();
}

// -----------------------------------------------------------------------

XubString SalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
    return GetKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

void SalFrame::UpdateSettings( AllSettings& rSettings )
{
}

// -----------------------------------------------------------------------

const SystemEnvData* SalFrame::GetSystemData() const
{
    return NULL;
}

// -----------------------------------------------------------------------

void SalFrame::Beep( SoundType eSoundType )
{
    VCLWindow_Beep();
}

// -----------------------------------------------------------------------

void SalFrame::SetCallback( void* pInst, SALFRAMEPROC pProc )
{
    maFrameData.mpInst = pInst;
    if ( pProc )
        maFrameData.mpProc = pProc;
    else
        maFrameData.mpProc = ImplSalFrameCallbackDummy;
}
