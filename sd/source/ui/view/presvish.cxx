/*************************************************************************
 *
 *  $RCSfile: presvish.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2004-01-20 12:52:21 $
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

#include "PresentationViewShell.hxx"

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
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#include "sdattr.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_SUB_SHELL_MANAGER_HXX
#include "SubShellManager.hxx"
#endif
#ifndef SD_FACTORY_IDS_HXX
#include "FactoryIds.hxx"
#endif

#define PresentationViewShell
using namespace sd;
#include "sdslots.hxx"

namespace sd {

// -------------------
// - PresentationViewShell -
// -------------------

SFX_IMPL_INTERFACE( PresentationViewShell, DrawViewShell, SdResId( STR_PRESVIEWSHELL ) )
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


TYPEINIT1( PresentationViewShell, DrawViewShell );

// -----------------------------------------------------------------------------

PresentationViewShell::PresentationViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    FrameView* pFrameViewArgument)
    : DrawViewShell (pFrame, rViewShellBase, PK_STANDARD, pFrameViewArgument),
    mbShowStarted( sal_False )
{
    if( GetDocSh() && GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        maOldVisArea = GetDocSh()->GetVisArea( ASPECT_CONTENT );
    meShellType = ST_PRESENTATION;
}

// -----------------------------------------------------------------------------

PresentationViewShell::PresentationViewShell( SfxViewFrame* pFrame, const DrawViewShell& rShell ) :
    DrawViewShell( pFrame, rShell ),
    mbShowStarted( sal_False )
{
    if( GetDocSh() && GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        maOldVisArea = GetDocSh()->GetVisArea( ASPECT_CONTENT );
    meShellType = ST_PRESENTATION;
}

// -----------------------------------------------------------------------------

PresentationViewShell::~PresentationViewShell()
{
    if( GetDocSh() && GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED && !maOldVisArea.IsEmpty() )
        GetDocSh()->SetVisArea( maOldVisArea );

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

void PresentationViewShell::Activate( BOOL bIsMDIActivate )
{
    DrawViewShell::Activate( bIsMDIActivate );

    if( bIsMDIActivate )
    {
        ::sd::View*     pView = GetView();
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
    GetDocSh()->Connect( this );

    if( pFuSlideShow && !mbShowStarted )
    {
        pFuSlideShow->StartShow();
        mbShowStarted = sal_True;
    }
}

// -----------------------------------------------------------------------------

void PresentationViewShell::Paint( const Rectangle& rRect, ::sd::Window* pWin )
{
    // allow paints only if show is already started
    if( mbShowStarted )
        DrawViewShell::Paint( rRect, pWin );
}

// -----------------------------------------------------------------------------

void PresentationViewShell::CreateFullScreenShow( ViewShell* pOriginShell, SfxRequest& rReq )
{
    SFX_REQUEST_ARG( rReq, pAlwaysOnTop, SfxBoolItem, ATTR_PRESENT_ALWAYS_ON_TOP, FALSE );

    WorkWindow*     pWorkWindow = new WorkWindow( NULL, WB_HIDE | WB_CLIPCHILDREN );
    SdDrawDocument* pDoc = pOriginShell->GetDoc();
    SdPage*         pActualPage = pOriginShell->GetActualPage();
    BOOL            bAlwaysOnTop = ( ( SID_REHEARSE_TIMINGS != rReq.GetSlot() ) && pAlwaysOnTop ) ? pAlwaysOnTop->GetValue() : pDoc->GetPresAlwaysOnTop();

    pWorkWindow->StartPresentationMode( TRUE, bAlwaysOnTop ? PRESENTATION_HIDEALLAPPS : 0 );
    SfxTopFrame* pNewFrame = SfxTopFrame::Create (
        pDoc->GetDocSh(), pWorkWindow, PRESENTATION_FACTORY_ID, TRUE );
    pNewFrame->SetPresentationMode( TRUE );

    ViewShellBase* pBase = static_cast<ViewShellBase*>(
        pNewFrame->GetCurrentViewFrame()->GetViewShell());
    PresentationViewShell *pShell = NULL;
    if (pBase != NULL)
        pShell = static_cast<PresentationViewShell*>(
            pBase->GetSubShellManager().GetMainSubShell());
    DBG_ASSERT(pShell!=NULL, "can not create presenation view shell");

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
    pShell->pFuSlideShow = new FuSlideShow( pShell, pShell->pWindow, pShell->pDrView, pShell->GetDoc(), rReq );
    pShell->GetActiveWindow()->GrabFocus();
}

} // end of namespace sd
