/*************************************************************************
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-28 06:41:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *           - GNU Lesser General Public License Version 2.1
 *           - Sun Industry Standards Source License Version 1.1
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

#include <premac.h>
#include <QD/QD.h>
#include <postmac.h>

// =======================================================================

static long ImplSalFrameCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
    return 0;
}

// =======================================================================

SalFrame::SalFrame()
{
    SalData* pSalData = GetSalData();

    maFrameData.mhWnd               = 0;
    maFrameData.mpGraphics          = NULL;
    maFrameData.mpInst              = NULL;
    maFrameData.mpProc              = ImplSalFrameCallbackDummy;
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
    if ( maFrameData.mpGraphics )
        delete maFrameData.mpGraphics;

    if ( maFrameData.mhWnd )
        VCLWindow_Release( maFrameData.mhWnd );
}

// -----------------------------------------------------------------------

SalGraphics* SalFrame::GetGraphics()
{
    if ( maFrameData.mbGraphics )
        return NULL;

    if ( !maFrameData.mpGraphics )
    {
        VCLVIEW hView;
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
            SalData* pSalData = GetSalData();
            maFrameData.mpGraphics = new SalGraphics;
            maFrameData.mpGraphics->maGraphicsData.mhDC      = hView;
            maFrameData.mpGraphics->maGraphicsData.mhWnd     = pFrame->maFrameData.mhWnd;
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
    return VCLWindow_PostEvent( maFrameData.mhWnd, pData );
}

// -----------------------------------------------------------------------

void SalFrame::SetTitle( const XubString& rTitle )
{

    ByteString aByteTitle( rTitle, gsl_getSystemTextEncoding() );
    char *pTitle = (char *)aByteTitle.GetBuffer();
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
        VCLWindow_Show( maFrameData.mhWnd );
    } // if
    else
    {
        VCLWindow_Close( maFrameData.mhWnd );
    } // else

    // This is temporary code for testing only and should be removed
    // when development of the SalObject class is complete. This code
    // allows us to test our SalGraphics drawing methods.

    RGBColor  rectFillColor = { 65535, 25000, 25000 }; // shade of red
    RGBColor  polyFillColor = { 25000, 25000, 65535 }; // shade of blue
    RGBColor  linePenColor  = { 25000, 65535, 25000 }; // shade of green
    RGBColor  pixelColor    = { 60000, 40000, 20000 }; // shade of orange

    ULONG     polyVertexCount = 7;
    long      polyVertexXCoors[polyVertexCount];
    long      polyVertexYCoors[polyVertexCount];
    long      pixelXCoor;
    long      pixelYCoor;
    long      pixelXCnt;
    long      pixelYCnt;

    // Get this window's cached handle to its native content view

    VCLVIEW hView = VCLWindow_ContentView ( maFrameData.mhWnd );

    // Draw a line on the native content view (no color)

    VCLGraphics_DrawLine ( hView, 15L, 15L, 150L, 150L );

    // Draw a line on the native content view (color)

    VCLGraphics_DrawColorLine ( hView, 55L, 15L, 190L, 150L, &linePenColor );

    // Draw a rectangle on the native content view (no color)

    VCLGraphics_DrawRect ( hView, 200L, 15L, 100L, 150L );

    // Draw a rectangle on the native content view (color)

    VCLGraphics_DrawColorRect ( hView, 325L, 15L, 100L, 150L, &rectFillColor );

    // Draw a polygon on the native content view (no color)

    polyVertexXCoors[0] = 350;
    polyVertexYCoors[0] = 250;

    polyVertexXCoors[1] = 450;
    polyVertexYCoors[1] = 250;

    polyVertexXCoors[2] = 375;
    polyVertexYCoors[2] = 350;

    polyVertexXCoors[3] = 400;
    polyVertexYCoors[3] = 200;

    polyVertexXCoors[4] = 425;
    polyVertexYCoors[4] = 350;

    polyVertexCount = 5;

    VCLGraphics_DrawPolygon ( hView,
                              polyVertexCount,
                              polyVertexXCoors,
                              polyVertexYCoors
                            );

    // Draw a polygon on the native content view (color)

    polyVertexXCoors[0] = 15;
    polyVertexYCoors[0] = 250;

    polyVertexXCoors[1] = 145;
    polyVertexYCoors[1] = 250;

    polyVertexXCoors[2] = 165;
    polyVertexYCoors[2] = 200;

    polyVertexXCoors[3] = 185;
    polyVertexYCoors[3] = 250;

    polyVertexXCoors[4] = 325;
    polyVertexYCoors[4] = 250;

    polyVertexXCoors[5] = 215;
    polyVertexYCoors[5] = 320;

    polyVertexXCoors[6] = 125;
    polyVertexYCoors[6] = 320;

    polyVertexCount = 7;

    VCLGraphics_DrawColorPolygon ( hView,
                                   polyVertexCount,
                                   polyVertexXCoors,
                                   polyVertexYCoors,
                                   &polyFillColor
                                 );

    // Draw a polygon on the native content view (no color)

    pixelXCnt = 235;
    pixelYCnt = 390;

    for ( pixelXCoor = 135; pixelXCoor < pixelXCnt; pixelXCoor++ )
    {
        for ( pixelYCoor = 355; pixelYCoor < pixelYCnt; pixelYCoor++ )
        {
            VCLGraphics_DrawPixel ( hView, pixelXCoor, pixelYCoor );
        } // for
    } // for

    // Draw a polygon on the native content view (color)

    pixelXCnt = 115;

    for ( pixelXCoor = 15; pixelXCoor < pixelXCnt; pixelXCoor++ )
    {
        for ( pixelYCoor = 355; pixelYCoor < pixelYCnt; pixelYCoor++ )
        {
            pixelColor.red   = pixelYCoor + pixelXCoor + pixelColor.red;
            pixelColor.green = pixelYCoor + pixelXCoor + pixelColor.green;
            pixelColor.blue  = pixelYCoor + pixelXCoor + pixelColor.blue;

            VCLGraphics_DrawColorPixel ( hView,
                                           pixelXCoor,
                                           pixelYCoor,
                                           &pixelColor
                                         );
        } // for
    } // for
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
    // VCLWindow_SetSize( maFrameData.mhWnd, nWidth, nHeight );
}

// -----------------------------------------------------------------------

void SalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    // Stub code until we get real sizing working
    rWidth  = 500;
    rHeight = 400;
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
    VCLWindow_Beep( maFrameData.mhWnd );
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
