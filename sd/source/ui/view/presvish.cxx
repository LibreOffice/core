/*************************************************************************
 *
 *  $RCSfile: presvish.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 15:00:43 $
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
#include "PaneManager.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_FACTORY_IDS_HXX
#include "FactoryIds.hxx"
#endif

#define PresentationViewShell
using namespace sd;
#include "sdslots.hxx"

namespace {

/** Objects of this class wait for the execution of an asynchronous view
    shell change and finish the initialization of a new
    PresentationViewShell object.

    After it has done its job the object removes itself as listener from the
    view shell base and destroyes itself.
*/
class ViewShellChangeListener
{
public:
    ViewShellChangeListener (
        ViewShellBase& rBase,
        FrameView* pFrameView,
        USHORT nPageNumber,
        const SfxRequest& rRequest)
        : mrBase (rBase),
          mpFrameView(pFrameView),
          mnPageNumber(nPageNumber),
          maRequest(rRequest)
    {
        if (mpFrameView != NULL)
            mpFrameView->Connect();
        rBase.GetPaneManager().AddEventListener (
            LINK(this, ViewShellChangeListener, HandleViewShellChange));
    }
private:
    ViewShellBase& mrBase;
    FrameView* const mpFrameView;
    const USHORT mnPageNumber;
    SfxRequest maRequest;

    DECL_LINK(HandleViewShellChange, PaneManagerEvent*);
};

IMPL_LINK(ViewShellChangeListener, HandleViewShellChange,
    PaneManagerEvent*, pEvent)
{
    if (pEvent->meEventId == PaneManagerEvent::EID_VIEW_SHELL_ADDED
        && pEvent->mePane == PaneManager::PT_CENTER)
    {
        if (pEvent->mpShell->ISA(PresentationViewShell))
        {
            PresentationViewShell* pShell
                = PTR_CAST(PresentationViewShell, pEvent->mpShell);
            pShell->FinishInitialization (
                mpFrameView,
                maRequest,
                mnPageNumber);
            if (mpFrameView != NULL)
                mpFrameView->Disconnect();
            /*
            mpFrameView->Connect();
            pShell->GetFrameView()->Disconnect();
            pShell->SetFrameView (mpFrameView);
            pShell->SwitchPage (mnPageNumber);
            pShell->WriteFrameViewData();

            SfxBoolItem aShowItem (SID_SHOWPOPUPS, FALSE);
            SfxUInt16Item aId (SID_CONFIGITEMID, SID_NAVIGATOR);
            pShell->GetViewFrame()->GetDispatcher()->Execute(
                SID_SHOWPOPUPS, SFX_CALLMODE_SYNCHRON, &aShowItem, &aId, 0L );
            pShell->GetViewFrame()->Show();
            pShell->SetSlideShowFunction (new FuSlideShow(
                pShell,
                pShell->GetActiveWindow(),
                pShell->GetView(),
                pShell->GetDoc(),
                maRequest));
            pShell->GetActiveWindow()->GrabFocus();
            pShell->GetSlideShow()->Activate();
            pShell->GetSlideShow()->StartShow();
            */
        }

        // Remove this listener from the view shell base and destroy it.
        mrBase.GetPaneManager().RemoveEventListener (
            LINK(this,ViewShellChangeListener,HandleViewShellChange));
        delete this;
    }

    return 0;
}

} // end of anonymouse namespace

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




PresentationViewShell::PresentationViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    ::Window* pParentWindow,
    FrameView* pFrameView)
    : DrawViewShell (
        pFrame,
        rViewShellBase,
        pParentWindow,
        PK_STANDARD,
        pFrameView),
    mbShowStarted( sal_False )
{
    if( GetDocSh() && GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        maOldVisArea = GetDocSh()->GetVisArea( ASPECT_CONTENT );
    meShellType = ST_PRESENTATION;
}




PresentationViewShell::PresentationViewShell (
    SfxViewFrame* pFrame,
    ::Window* pParentWindow,
    const DrawViewShell& rShell)
    : DrawViewShell (pFrame, pParentWindow, rShell),
      mbShowStarted( sal_False )
{
    if( GetDocSh() && GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        maOldVisArea = GetDocSh()->GetVisArea( ASPECT_CONTENT );
    meShellType = ST_PRESENTATION;
}




PresentationViewShell::~PresentationViewShell (void)
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




void PresentationViewShell::FinishInitialization (
    FrameView* pFrameView,
    SfxRequest& rRequest,
    USHORT nPageNumber)
{
    DrawViewShell::Init();

    // Use the frame view that comes form the view shell that initiated our
    // creation.
    if (pFrameView != NULL)
    {
        GetFrameView()->Disconnect();
        SetFrameView (pFrameView);
        pFrameView->Connect();
    }
    SwitchPage (nPageNumber);
    WriteFrameViewData();

    SfxBoolItem aShowItem (SID_SHOWPOPUPS, FALSE);
    SfxUInt16Item aId (SID_CONFIGITEMID, SID_NAVIGATOR);
    GetViewFrame()->GetDispatcher()->Execute(
        SID_SHOWPOPUPS, SFX_CALLMODE_SYNCHRON, &aShowItem, &aId, 0L );
    GetViewFrame()->Show();
    SetSlideShowFunction (new FuSlideShow(
        this,
        GetActiveWindow(),
        GetView(),
        GetDoc(),
        rRequest));
    GetActiveWindow()->GrabFocus();

    // Start the show.
    GetSlideShow()->Activate();
    GetSlideShow()->StartShow();
}




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

void PresentationViewShell::CreateFullScreenShow (
    ViewShell* pOriginShell,
    SfxRequest& rRequest)
{
    SFX_REQUEST_ARG (rRequest, pAlwaysOnTop, SfxBoolItem,
        ATTR_PRESENT_ALWAYS_ON_TOP, FALSE);

    WorkWindow* pWorkWindow = new WorkWindow (
        NULL,
        WB_HIDE | WB_CLIPCHILDREN);
    SdDrawDocument* pDoc = pOriginShell->GetDoc();
    SdPage* pCurrentPage = pOriginShell->GetActualPage();
    bool bAlwaysOnTop =
        ((rRequest.GetSlot() !=  SID_REHEARSE_TIMINGS) && pAlwaysOnTop )
        ? pAlwaysOnTop->GetValue()
        : pDoc->GetPresAlwaysOnTop();

    pWorkWindow->StartPresentationMode (
        TRUE,
        bAlwaysOnTop ? PRESENTATION_HIDEALLAPPS : 0);
    if (pWorkWindow->IsVisible())
    {
        //AF The bHidden paramter (the fourth one) was previously set to
        // TRUE.  This does not work anymore for some unknown reason.  The
        // ViewShellBase does then not get activated and the whole
        // initialization process is not started: the screen becomes blank.
        SfxTopFrame* pNewFrame = SfxTopFrame::Create (
            pDoc->GetDocSh(),
            pWorkWindow,
            PRESENTATION_FACTORY_ID,
            FALSE/*TRUE*/);
        pNewFrame->SetPresentationMode (TRUE);

        ViewShellBase* pBase = static_cast<ViewShellBase*>(
            pNewFrame->GetCurrentViewFrame()->GetViewShell());
        if (pBase != NULL)
        {
            // Get the page where the show is to be started.  This normally
            // is the current page of the shell from which the show has been
            // started.  This, however, may be NULL, e.g. when started from
            // the slide sorter and that has an empty selection.
            USHORT nStartPage = 0;
            if (pCurrentPage != NULL)
                nStartPage = (pCurrentPage->GetPageNum() - 1) / 2;

            // The rest of the initialization is done by an object that
            // waits until the PresentationViewShell object has been
            // created.  This is necessary because the creation is done
            // asynchronously.
            new ViewShellChangeListener(
                *pBase,
                pOriginShell->GetFrameView(),
                nStartPage,
                rRequest);
            pBase->LateInit();
            pBase->GetViewFrame()->Show();
        }
    }
}

} // end of namespace sd
