/*************************************************************************
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ganaya $ $Date: 2000-11-13 21:12:25 $
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

#include <string.h>

#define _SV_SALFRAME_CXX

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

SalFrame::SalFrame()
{
    SalData* pSalData = GetSalData();

    maFrameData.mhWnd               = 0;
    maFrameData.mpGraphics          = NULL;
    maFrameData.mpInst              = NULL;
    maFrameData.mpProc              = NULL;
    maFrameData.mnInputLang         = 0;
    maFrameData.mnInputCodePage     = 0;
    maFrameData.mbGraphics          = FALSE;
    maFrameData.mbCaption           = FALSE;
    maFrameData.mbBorder            = FALSE;
    maFrameData.mbSizeBorder        = FALSE;
    maFrameData.mbFullScreen        = FALSE;
    maFrameData.mbPresentation      = FALSE;
    maFrameData.mbInShow            = FALSE;
    maFrameData.mbRestoreMaximize   = FALSE;
    maFrameData.mbInMoveMsg         = FALSE;
    maFrameData.mbInSizeMsg         = FALSE;
    maFrameData.mbFullScreenToolWin = FALSE;
    maFrameData.mbDefPos            = TRUE;
    maFrameData.mbOverwriteState    = TRUE;
    maFrameData.mbIME               = FALSE;
    maFrameData.mbHandleIME         = FALSE;
    maFrameData.mbSpezIME           = FALSE;
    maFrameData.mbAtCursorIME       = FALSE;
    maFrameData.mbCompositionMode   = FALSE;
    maFrameData.mbCandidateMode     = FALSE;
    memset( &maFrameData.maState, 0, sizeof( SalFrameState ) );
    maFrameData.maSysData.nSize     = sizeof( SystemEnvData );
}

// -----------------------------------------------------------------------

SalFrame::~SalFrame()
{
}

// -----------------------------------------------------------------------

SalGraphics* SalFrame::GetGraphics()
{
    if ( maFrameData.mbGraphics )
        return NULL;

    if ( !maFrameData.mpGraphics )
    {
        VCLVIEW hView = VCLWindow_contentView( maFrameData.mhWnd );
        if ( hView )
        {
            SalData* pSalData = GetSalData();
            maFrameData.mpGraphics = new SalGraphics;
            maFrameData.mpGraphics->maGraphicsData.mhDC      = hView;
            maFrameData.mpGraphics->maGraphicsData.mhWnd     = maFrameData.mhWnd;
            maFrameData.mpGraphics->maGraphicsData.mbPrinter = FALSE;
            maFrameData.mpGraphics->maGraphicsData.mbVirDev  = FALSE;
            maFrameData.mpGraphics->maGraphicsData.mbWindow  = TRUE;
            maFrameData.mpGraphics->maGraphicsData.mbScreen  = TRUE;
            maFrameData.mbGraphics = TRUE;
        }
    }
    else
        maFrameData.mbGraphics = TRUE;

    return maFrameData.mpGraphics;
}

// -----------------------------------------------------------------------

void SalFrame::ReleaseGraphics( SalGraphics* )
{
}

// -----------------------------------------------------------------------

BOOL SalFrame::PostEvent( void* pData )
{
    return FALSE;
}

// -----------------------------------------------------------------------

void SalFrame::SetTitle( const XubString& rTitle )
{
  ByteString aByteTitle( rTitle, gsl_getSystemTextEncoding() );
  char *pTitle = (char*)aByteTitle.GetBuffer();
  VCLWindow_SetTitle(maFrameData.mhWnd, pTitle);
}

// -----------------------------------------------------------------------

void SalFrame::SetIcon( USHORT nIcon )
{
}

// -----------------------------------------------------------------------

void SalFrame::Show( BOOL bVisible )
{
    if ( bVisible )
        VCLWindow_makeKeyAndOrderFront( maFrameData.mhWnd );
    else
        VCLWindow_close( maFrameData.mhWnd );

    // This is temporary code for testing only and should be removed when
    // development of the SalObject class is complete. This code allows
    // us to test our SalGraphics drawing methods.

    // Get this window's cached handle to its native content view
    VCLVIEW hView = VCLWindow_contentView( maFrameData.mhWnd );

    // Draw a line on the native content view
    VCLGraphics_drawLine( hView, 0L, 0L, 1000L, 1000L );
}

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
}

// -----------------------------------------------------------------------

void SalFrame::GetClientSize( long& rWidth, long& rHeight )
{
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

void SalFrame::ShowFullScreen( BOOL bFullScreen )
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

void SalFrame::UpdateExtTextInputArea()
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
}

// -----------------------------------------------------------------------

void SalFrame::SetCallback( void* pInst, SALFRAMEPROC pProc )
{
}
