/*************************************************************************
 *
 *  $RCSfile: presvish.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 17:43:17 $
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

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX
#include <sfx2/topfrm.hxx>
#endif
#ifndef _SFX_DISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif

#include <svx/svxids.hrc>
#include <sfx2/app.hxx>
#include "frmview.hxx"
#include "sdresid.hxx"
#include "docshell.hxx"
#include "presvish.hxx"
#include "fuslshow.hxx"
#include "sdattr.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"

#define SdPresViewShell
#include "sdslots.hxx"

// -------------------
// - SdPresViewShell -
// -------------------

SFX_IMPL_INTERFACE( SdPresViewShell, SdDrawViewShell, SdResId( STR_PRESVIEWSHELL ) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER | SFX_VISIBILITY_READONLYDOC,
                                SdResId(RID_DRAW_VIEWER_TOOLBOX) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OPTIONS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_OPTIONS_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_COMMONTASK | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_COMMONTASK_TOOLBOX));
}

// -----------------------------------------------------------------------------

SFX_IMPL_VIEWFACTORY( SdPresViewShell, SdResId( STR_DEFAULTVIEW ) )
{
    SFX_VIEW_REGISTRATION( SdDrawDocShell );
}

// -----------------------------------------------------------------------------

TYPEINIT1( SdPresViewShell, SdDrawViewShell );

// -----------------------------------------------------------------------------

SdPresViewShell::SdPresViewShell( SfxViewFrame* pFrame, SfxViewShell *pOldShell ) :
    SdDrawViewShell( pFrame, pOldShell ),
    mbShowStarted( sal_False )
{
    if( pDocSh && pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        maOldVisArea = pDocSh->GetVisArea( ASPECT_CONTENT );
}

// -----------------------------------------------------------------------------

SdPresViewShell::SdPresViewShell( SfxViewFrame* pFrame, const SdDrawViewShell& rShell ) :
    SdDrawViewShell( pFrame, rShell ),
    mbShowStarted( sal_False )
{
    if( pDocSh && pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        maOldVisArea = pDocSh->GetVisArea( ASPECT_CONTENT );
}

// -----------------------------------------------------------------------------

SdPresViewShell::~SdPresViewShell()
{
    if( pDocSh && pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED && !maOldVisArea.IsEmpty() )
        pDocSh->SetVisArea( maOldVisArea );

    if( GetViewFrame() && GetViewFrame()->GetTopFrame() )
    {
        WorkWindow* pWorkWindow = (WorkWindow*) GetViewFrame()->GetTopFrame()->GetWindow().GetParent();

        if( pWorkWindow )
            pWorkWindow->StartPresentationMode( FALSE, pFuSlideShow ? pFuSlideShow->IsAlwaysOnTop() : 0 );
    }

    if( pFuSlideShow )
    {
        pFuSlideShow->Deactivate();
        pFuSlideShow->Terminate();
        pFuSlideShow->Destroy();
        pFuSlideShow = NULL;
    }
}

// -----------------------------------------------------------------------------

void SdPresViewShell::Activate( BOOL bIsMDIActivate )
{
    SfxViewShell::Activate( bIsMDIActivate );

    if( bIsMDIActivate )
    {
        SdView*     pView = GetView();
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, TRUE );

        GetViewFrame()->GetDispatcher()->Execute( SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

        if( pFuSlideShow && !pFuSlideShow->IsTerminated() )
            pFuSlideShow->Activate();

        if( pFuActual )
            pFuActual->Activate();

        if( pView )
            pView->ShowMarkHdl( NULL );
    }

    if( bIsMDIActivate )
        ReadFrameViewData( pFrameView );
    pDocSh->Connect( this );

    if( pFuSlideShow && !mbShowStarted )
    {
        pFuSlideShow->StartShow();
        mbShowStarted = sal_True;
    }
}

// -----------------------------------------------------------------------------

void SdPresViewShell::Paint( const Rectangle& rRect, SdWindow* pWin )
{
    // allow paints only if show is already started
    if( mbShowStarted )
        SdDrawViewShell::Paint( rRect, pWin );
}

// -----------------------------------------------------------------------------

void SdPresViewShell::CreateFullScreenShow( SdViewShell* pOriginShell, SfxRequest& rReq )
{
    SFX_REQUEST_ARG( rReq, pAlwaysOnTop, SfxBoolItem, ATTR_PRESENT_ALWAYS_ON_TOP, FALSE );

    WorkWindow*     pWorkWindow = new WorkWindow( NULL, WB_HIDE | WB_CLIPCHILDREN );
    SdDrawDocument* pDoc = pOriginShell->GetDoc();
    SdPage*         pActualPage = pOriginShell->GetActualPage();
    BOOL            bAlwaysOnTop = ( ( SID_REHEARSE_TIMINGS != rReq.GetSlot() ) && pAlwaysOnTop ) ? pAlwaysOnTop->GetValue() : pDoc->GetPresAlwaysOnTop();

    pWorkWindow->StartPresentationMode( TRUE, bAlwaysOnTop ? PRESENTATION_HIDEALLAPPS : 0 );
    SfxTopFrame* pNewFrame = SfxTopFrame::Create( pDoc->GetDocSh(), pWorkWindow, 4, TRUE );
    pNewFrame->SetPresentationMode( TRUE );

    SdPresViewShell*    pShell = (SdPresViewShell*) pNewFrame->GetCurrentViewFrame()->GetViewShell();
    SfxUInt16Item       aId( SID_CONFIGITEMID, SID_NAVIGATOR );
    SfxBoolItem         aShowItem( SID_SHOWPOPUPS, FALSE );
    const USHORT        nCurSdPageNum = ( pActualPage->GetPageNum() - 1 ) / 2;
    FrameView*          pNewFrameView = pOriginShell->GetFrameView();

    pNewFrameView->Connect();
    pShell->GetFrameView()->Disconnect();
    pShell->pFrameView = pNewFrameView;
    pShell->ReadFrameViewData( pNewFrameView );
    pShell->SwitchPage( nCurSdPageNum );
    pShell->WriteFrameViewData();

    pShell->GetViewFrame()->GetDispatcher()->Execute( SID_SHOWPOPUPS, SFX_CALLMODE_SYNCHRON, &aShowItem, &aId, 0L );
    pShell->GetViewFrame()->Show();
    pShell->pFuSlideShow = new FuSlideShow( pShell, pShell->pWindow, pShell->pDrView, pShell->pDoc, rReq );
    pShell->GetActiveWindow()->GrabFocus();
}
