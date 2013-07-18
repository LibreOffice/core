/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "ViewShell.hxx"
#include "ViewShellImplementation.hxx"

#include <com/sun/star/embed/EmbedStates.hpp>
#include "ViewShellBase.hxx"
#include "ShellFactory.hxx"
#include "DrawController.hxx"
#include "LayerTabBar.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/scrbar.hxx>
#include <svl/eitem.hxx>
#include <svx/ruler.hxx>
#include <svx/svxids.hrc>
#include <svx/fmshell.hxx>
#include "WindowUpdater.hxx"
#include "GraphicViewShell.hxx"
#include <sfx2/childwin.hxx>
#include <sdxfer.hxx>

#include "app.hrc"
#include "helpids.h"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "OutlineView.hxx"
#include "Client.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "slideshow.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "zoomlist.hxx"
#include "FrameView.hxx"
#include "optsitem.hxx"
#include "BezierObjectBar.hxx"
#include "TextObjectBar.hxx"
#include "GraphicObjectBar.hxx"
#include "MediaObjectBar.hxx"
#include "ViewShellManager.hxx"
#include "FormShellManager.hxx"
#include <svx/dialogs.hrc>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/svdoutl.hxx>
#include <tools/diagnose_ex.h>

#include <svl/slstitm.hxx>
#include <sfx2/request.hxx>
#include "SpellDialogChildWindow.hxx"

#include <basegfx/tools/zoomtools.hxx>

#include "Window.hxx"
#include "fupoor.hxx"

#include <editeng/numitem.hxx>
#include <editeng/eeitem.hxx>
#include <svl/poolitem.hxx>
#include <glob.hrc>

namespace sd { namespace ui { namespace table {
    extern SfxShell* CreateTableObjectBar( ViewShell& rShell, ::sd::View* pView );
} } }

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

namespace {

class ViewShellObjectBarFactory
    : public ::sd::ShellFactory<SfxShell>
{
public:
    ViewShellObjectBarFactory (::sd::ViewShell& rViewShell);
    virtual ~ViewShellObjectBarFactory (void);
    virtual SfxShell* CreateShell (
        ::sd::ShellId nId,
        ::Window* pParentWindow,
        ::sd::FrameView* pFrameView);
    virtual void ReleaseShell (SfxShell* pShell);
private:
    ::sd::ViewShell& mrViewShell;
    /** This cache holds the already created object bars.
    */
    typedef ::std::map< ::sd::ShellId,SfxShell*> ShellCache;
    ShellCache maShellCache;
};


} // end of anonymous namespace


namespace sd {

sal_Bool ViewShell::IsPageFlipMode(void) const
{
    return this->ISA(DrawViewShell) && mpContentWindow.get() != NULL &&
        mpContentWindow->GetVisibleHeight() >= 1.0;
}

SfxViewFrame* ViewShell::GetViewFrame (void) const
{
    const SfxViewShell* pViewShell = GetViewShell();
    if (pViewShell != NULL)
    {
        return pViewShell->GetViewFrame();
    }
    else
    {
        OSL_ASSERT (GetViewShell()!=NULL);
        return NULL;
    }
}


/// declare SFX-Slotmap and standard interface
TYPEINIT1(ViewShell, SfxShell);


ViewShell::ViewShell( SfxViewFrame*, ::Window* pParentWindow, ViewShellBase& rViewShellBase, bool bAllowCenter)
:   SfxShell(&rViewShellBase)
,   mbCenterAllowed(bAllowCenter)
,   mpParentWindow(pParentWindow)
{
    construct();
}

ViewShell::~ViewShell()
{
    // Keep the content window from accessing in its destructor the
    // WindowUpdater.
    if (mpContentWindow)
        mpContentWindow->SetViewShell(NULL);

    delete mpZoomList;

    mpLayerTabBar.reset();

    if (mpImpl->mpSubShellFactory.get() != NULL)
        GetViewShellBase().GetViewShellManager()->RemoveSubShellFactory(
            this,mpImpl->mpSubShellFactory);

    if (mpContentWindow)
    {
        OSL_TRACE("destroying mpContentWindow at %x with parent %x", mpContentWindow.get(),
            mpContentWindow->GetParent());
        mpContentWindow.reset();
    }
}


/**
 * common initialization part of both constructors
 */
void ViewShell::construct(void)
{
    mbHasRulers = false;
    mpActiveWindow = 0;
    mpView = 0;
    mpFrameView = 0;
    mpZoomList = 0;
    mbStartShowWithDialog = sal_False;
    mnPrintedHandoutPageNum = 1;
    mnPrintedHandoutPageCount = 0;
    mpWindowUpdater.reset( new ::sd::WindowUpdater() );
    mpImpl.reset(new Implementation(*this));
    meShellType = ST_NONE;

    OSL_ASSERT (GetViewShell()!=NULL);

    if (IsMainViewShell())
        GetDocSh()->Connect (this);

    mpZoomList = new ZoomList( this );

    mpContentWindow.reset(new ::sd::Window(GetParentWindow()));
    SetActiveWindow (mpContentWindow.get());

    GetParentWindow()->SetBackground (Wallpaper());
    mpContentWindow->SetBackground (Wallpaper());
    mpContentWindow->SetCenterAllowed(mbCenterAllowed);
    mpContentWindow->SetViewShell(this);
    mpContentWindow->SetPosSizePixel(
        GetParentWindow()->GetPosPixel(),GetParentWindow()->GetSizePixel());

    if ( ! GetDocSh()->IsPreview())
    {
        // Create scroll bars and the filler between the scroll bars.
        mpHorizontalScrollBar.reset (new ScrollBar(GetParentWindow(), WinBits(WB_HSCROLL | WB_DRAG)));
        mpHorizontalScrollBar->EnableRTL (sal_False);
        mpHorizontalScrollBar->SetRange(Range(0, 32000));
        mpHorizontalScrollBar->SetScrollHdl(LINK(this, ViewShell, HScrollHdl));

        mpVerticalScrollBar.reset (new ScrollBar(GetParentWindow(), WinBits(WB_VSCROLL | WB_DRAG)));
        mpVerticalScrollBar->SetRange(Range(0, 32000));
        mpVerticalScrollBar->SetScrollHdl(LINK(this, ViewShell, VScrollHdl));

        mpScrollBarBox.reset(new ScrollBarBox(GetParentWindow(), WB_SIZEABLE));
    }

    OUString aName( "ViewShell" );
    SetName (aName);

    GetDoc()->StartOnlineSpelling(sal_False);

    mpWindowUpdater->SetViewShell (*this);
    mpWindowUpdater->SetDocument (GetDoc());

    // Re-initialize the spell dialog.
    ::sd::SpellDialogChildWindow* pSpellDialog =
          static_cast< ::sd::SpellDialogChildWindow*> (
              GetViewFrame()->GetChildWindow (
                  ::sd::SpellDialogChildWindow::GetChildWindowId()));
    if (pSpellDialog != NULL)
        pSpellDialog->InvalidateSpellDialog();

    // Register the sub shell factory.
    mpImpl->mpSubShellFactory.reset(new ViewShellObjectBarFactory(*this));
    GetViewShellBase().GetViewShellManager()->AddSubShellFactory(this,mpImpl->mpSubShellFactory);
}

void ViewShell::doShow(void)
{
    mpContentWindow->Show();
    static_cast< ::Window*>(mpContentWindow.get())->Resize();
    OSL_TRACE("content window has size %d %d",
        mpContentWindow->GetSizePixel().Width(),
        mpContentWindow->GetSizePixel().Height());

    if ( ! GetDocSh()->IsPreview())
    {
        // Show scroll bars
        mpHorizontalScrollBar->Show();

        mpVerticalScrollBar->Show();
        maScrBarWH = Size(
            mpVerticalScrollBar->GetSizePixel().Width(),
            mpHorizontalScrollBar->GetSizePixel().Height());

        mpScrollBarBox->Show();
    }

    GetParentWindow()->Show();
}

void ViewShell::Init (bool bIsMainViewShell)
{
    mpImpl->mbIsInitialized = true;
    SetIsMainViewShell(bIsMainViewShell);
    if (bIsMainViewShell)
        SetActiveWindow (mpContentWindow.get());
}




void ViewShell::Exit (void)
{
    sd::View* pView = GetView();
    if (pView!=NULL && pView->IsTextEdit())
    {
        pView->SdrEndTextEdit();
        pView->UnmarkAll();
    }

    Deactivate (sal_True);

    if (IsMainViewShell())
    {
        GetDocSh()->Disconnect(this);
    }

    SetIsMainViewShell(false);
}




/**
 * set focus to working window
 */
void ViewShell::Activate(sal_Bool bIsMDIActivate)
{
    // Do not forward to SfxShell::Activate()

    /* According to MI, nobody is allowed to call GrabFocus, who does not
       exactly know from which window the focus is grabbed. Since Activate()
       is sent sometimes asynchronous,  it can happen, that the wrong window
       gets the focus. */

    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetActive(sal_True);
    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->SetActive(sal_True);

    if (bIsMDIActivate)
    {
        // thus, the Navigator will also get a current status
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, sal_True );
        if (GetDispatcher() != NULL)
            GetDispatcher()->Execute(
                SID_NAVIGATOR_INIT,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                &aItem,
                0L);

        SfxViewShell* pViewShell = GetViewShell();
        OSL_ASSERT (pViewShell!=NULL);
        SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_3D_STATE, sal_True, sal_False );

        rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
        if(xSlideShow.is() && xSlideShow->isRunning() )
        {
            xSlideShow->activate(GetViewShellBase());
        }
        if(HasCurrentFunction())
        {
            GetCurrentFunction()->Activate();
        }

        if(!GetDocSh()->IsUIActive())
            UpdatePreview( GetActualPage(), sal_True );

    }

    ReadFrameViewData( mpFrameView );

    if (IsMainViewShell())
        GetDocSh()->Connect(this);
}

void ViewShell::UIActivating( SfxInPlaceClient*  )
{
    OSL_ASSERT (GetViewShell()!=NULL);
    GetViewShellBase().GetToolBarManager()->ToolBarsDestroyed();
}



void ViewShell::UIDeactivated( SfxInPlaceClient*  )
{
    OSL_ASSERT (GetViewShell()!=NULL);
    GetViewShellBase().GetToolBarManager()->ToolBarsDestroyed();
    if ( GetDrawView() )
        GetViewShellBase().GetToolBarManager()->SelectionHasChanged(*this, *GetDrawView());
}


void ViewShell::Deactivate(sal_Bool bIsMDIActivate)
{
    // remove view from a still active drag'n'drop session
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

    if (IsMainViewShell())
        GetDocSh()->Disconnect(this);

    if( pDragTransferable )
        pDragTransferable->SetView( NULL );

    OSL_ASSERT (GetViewShell()!=NULL);

    // remember view attributes of FrameView
    WriteFrameViewData();

    if (bIsMDIActivate)
    {
        rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
        if(xSlideShow.is() && xSlideShow->isRunning() )
        {
            xSlideShow->deactivate(GetViewShellBase());
        }
        if(HasCurrentFunction())
        {
            GetCurrentFunction()->Deactivate();
        }
    }

    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetActive(sal_False);
    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->SetActive(sal_False);
    // Do not forward to SfxShell::Deactivate()
}




void ViewShell::Shutdown (void)
{
    Exit ();
}




sal_Bool ViewShell::KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    sal_Bool bReturn(sal_False);

    if(pWin)
    {
        SetActiveWindow(pWin);
    }

    if(!bReturn)
    {
        // give key input first to SfxViewShell to give CTRL+Key
        // (e.g. CTRL+SHIFT+'+', to front) priority.
        OSL_ASSERT (GetViewShell()!=NULL);
        bReturn = (sal_Bool)GetViewShell()->KeyInput(rKEvt);
    }

    if(!bReturn)
    {
        rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
        if(xSlideShow.is() && xSlideShow->isRunning())
        {
            bReturn = xSlideShow->keyInput(rKEvt);
        }
        else
        {
            bool bConsumed = false;
            if( GetView() )
                bConsumed = GetView()->getSmartTags().KeyInput(rKEvt);


            if( !bConsumed )
            {
                rtl::Reference< sdr::SelectionController > xSelectionController( GetView()->getSelectionController() );
                if( !xSelectionController.is() || !xSelectionController->onKeyInput( rKEvt, pWin ) )
                {
                    if(HasCurrentFunction())
                        bReturn = GetCurrentFunction()->KeyInput(rKEvt);
                }
                else
                {
                    bReturn = sal_True;
                }
            }
        }
    }

    if(!bReturn && GetActiveWindow())
    {
        KeyCode aKeyCode = rKEvt.GetKeyCode();

        if (aKeyCode.IsMod1() && aKeyCode.IsShift()
            && aKeyCode.GetCode() == KEY_R)
        {
            InvalidateWindows();
            bReturn = sal_True;
        }
    }

    return(bReturn);
}


void ViewShell::MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    // We have to lock tool bar updates while the mouse button is pressed in
    // order to prevent the shape under the mouse to be moved (this happens
    // when the number of docked tool bars changes as result of a changed
    // selection;  this changes the window size and thus the mouse position
    // in model coordinates: with respect to model coordinates the mouse
    // moves.)
    OSL_ASSERT(mpImpl->mpUpdateLockForMouse.expired());
    mpImpl->mpUpdateLockForMouse = ViewShell::Implementation::ToolBarManagerLock::Create(
        GetViewShellBase().GetToolBarManager());

    if ( pWin && !pWin->HasFocus() )
    {
        pWin->GrabFocus();
        SetActiveWindow(pWin);
    }

    // insert MouseEvent into E3dView
    if (GetView() != NULL)
        GetView()->SetMouseEvent(rMEvt);

    bool bConsumed = false;
    if( GetView() )
        bConsumed = GetView()->getSmartTags().MouseButtonDown( rMEvt );

    if( !bConsumed )
    {
        rtl::Reference< sdr::SelectionController > xSelectionController( GetView()->getSelectionController() );
        if( !xSelectionController.is() || !xSelectionController->onMouseButtonDown( rMEvt, pWin ) )
        {
            if(HasCurrentFunction())
            {
                GetCurrentFunction()->MouseButtonDown(rMEvt);
            }
        }
    }
}


void ViewShell::MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if (rMEvt.IsLeaveWindow())
    {
        if ( ! mpImpl->mpUpdateLockForMouse.expired())
        {
            ::boost::shared_ptr<ViewShell::Implementation::ToolBarManagerLock> pLock(
                mpImpl->mpUpdateLockForMouse);
            if (pLock.get() != NULL)
                pLock->Release();
        }
    }

    if ( pWin )
    {
        SetActiveWindow(pWin);
    }

    // insert MouseEvent into E3dView
    if (GetView() != NULL)
        GetView()->SetMouseEvent(rMEvt);

    if(HasCurrentFunction())
    {
        rtl::Reference< sdr::SelectionController > xSelectionController( GetView()->getSelectionController() );
        if( !xSelectionController.is() || !xSelectionController->onMouseMove( rMEvt, pWin ) )
        {
            if(HasCurrentFunction())
                GetCurrentFunction()->MouseMove(rMEvt);
        }
    }
}


void ViewShell::MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( pWin )
    {
        SetActiveWindow(pWin);
    }

    // insert MouseEvent into E3dView
    if (GetView() != NULL)
        GetView()->SetMouseEvent(rMEvt);

    if( HasCurrentFunction())
    {
        rtl::Reference< sdr::SelectionController > xSelectionController( GetView()->getSelectionController() );
        if( !xSelectionController.is() || !xSelectionController->onMouseButtonUp( rMEvt, pWin ) )
        {
            if(HasCurrentFunction())
                GetCurrentFunction()->MouseButtonUp(rMEvt);
        }
    }

    if ( ! mpImpl->mpUpdateLockForMouse.expired())
    {
        ::boost::shared_ptr<ViewShell::Implementation::ToolBarManagerLock> pLock(
            mpImpl->mpUpdateLockForMouse);
        if (pLock.get() != NULL)
            pLock->Release();
    }
}



void ViewShell::Command(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    sal_Bool bDone = HandleScrollCommand (rCEvt, pWin);

    if( !bDone )
    {
        if( rCEvt.GetCommand() == COMMAND_INPUTLANGUAGECHANGE )
        {
            //#i42732# update state of fontname if input language changes
            GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONT );
            GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
        }
        else
        {
            bool bConsumed = false;
               if( GetView() )
               {
                bConsumed = GetView()->getSmartTags().Command(rCEvt);
            }

            if( !bConsumed && HasCurrentFunction())
            {
                GetCurrentFunction()->Command(rCEvt);
            }
        }
    }
}

long ViewShell::Notify(NotifyEvent& rNEvt, ::sd::Window* pWin)
{
    // handle scroll commands when they arrived at child windows
    long nRet = sal_False;
    if( rNEvt.GetType() == EVENT_COMMAND )
    {
        // note: dynamic_cast is not possible as GetData() returns a void*
        CommandEvent* pCmdEvent = reinterpret_cast< CommandEvent* >(rNEvt.GetData());
        nRet = HandleScrollCommand(*pCmdEvent, pWin);
    }
    return nRet;
}


bool ViewShell::HandleScrollCommand(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    bool bDone = false;

    switch( rCEvt.GetCommand() )
    {
        case COMMAND_WHEEL:
            {
                Reference< XSlideShowController > xSlideShowController( SlideShow::GetSlideShowController(GetViewShellBase() ) );
                if( xSlideShowController.is() )
                {
                    // We ignore zooming with control+mouse wheel.
                    const CommandWheelData* pData = rCEvt.GetWheelData();
                    if( pData && !pData->GetModifier() && ( pData->GetMode() == COMMAND_WHEEL_SCROLL ) && !pData->IsHorz() )
                    {
                        long nDelta = pData->GetDelta();
                        if( nDelta > 0 )
                        {
                            xSlideShowController->gotoPreviousSlide();
                        }
                        else if( nDelta < 0 )
                        {
                            xSlideShowController->gotoNextEffect();
                        }
                    }
                    break;
                }
            }
            // fall through when not running slideshow
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();

            if (pData != NULL)
            {
                if (pData->IsMod1())
                {
                    if( !GetDocSh()->IsUIActive() )
                    {
                        const long  nOldZoom = GetActiveWindow()->GetZoom();
                        long        nNewZoom;

                        if( pData->GetDelta() < 0L )
                            nNewZoom = std::max( (long) pWin->GetMinZoom(), basegfx::zoomtools::zoomOut( nOldZoom ));
                        else
                            nNewZoom = std::min( (long) pWin->GetMaxZoom(), basegfx::zoomtools::zoomIn( nOldZoom ));

                        SetZoom( nNewZoom );
                        Invalidate( SID_ATTR_ZOOM );
                        Invalidate( SID_ATTR_ZOOMSLIDER );

                        bDone = true;
                    }
                }
                else
                {
                    if( mpContentWindow.get() == pWin )
                    {
                        sal_uLong nScrollLines = pData->GetScrollLines();
                        if(IsPageFlipMode())
                            nScrollLines = COMMAND_WHEEL_PAGESCROLL;
                        CommandWheelData aWheelData( pData->GetDelta(),pData->GetNotchDelta(),
                            nScrollLines,pData->GetMode(),pData->GetModifier(),pData->IsHorz() );
                        CommandEvent aReWrite( rCEvt.GetMousePosPixel(),rCEvt.GetCommand(),
                            rCEvt.IsMouseEvent(),(const void *) &aWheelData );
                        bDone = pWin->HandleScrollCommand( aReWrite,
                            mpHorizontalScrollBar.get(),
                            mpVerticalScrollBar.get()) == sal_True;
                    }
                }
            }
        }
        break;

        default:
        break;
    }

    return bDone;
}



void ViewShell::SetupRulers (void)
{
    if(mbHasRulers && (mpContentWindow.get() != NULL) && !SlideShow::IsRunning(GetViewShellBase()))
    {
        long nHRulerOfs = 0;

        if ( mpVerticalRuler.get() == NULL )
        {
            mpVerticalRuler.reset(CreateVRuler(GetActiveWindow()));
            if ( mpVerticalRuler.get() != NULL )
            {
                nHRulerOfs = mpVerticalRuler->GetSizePixel().Width();
                mpVerticalRuler->SetActive(sal_True);
                mpVerticalRuler->Show();
            }
        }
        if ( mpHorizontalRuler.get() == NULL )
        {
            mpHorizontalRuler.reset(CreateHRuler(GetActiveWindow(), sal_True));
            if ( mpHorizontalRuler.get() != NULL )
            {
                mpHorizontalRuler->SetWinPos(nHRulerOfs);
                mpHorizontalRuler->SetActive(sal_True);
                mpHorizontalRuler->Show();
            }
        }
    }
}

const SfxPoolItem* ViewShell::GetNumBulletItem(SfxItemSet& aNewAttr, sal_uInt16& nNumItemId)
{
    const SfxPoolItem* pTmpItem = NULL;

    if(aNewAttr.GetItemState(nNumItemId, sal_False, &pTmpItem) == SFX_ITEM_SET)
    {
        return pTmpItem;
    }
    else
    {
        nNumItemId = aNewAttr.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        SfxItemState eState = aNewAttr.GetItemState(nNumItemId, sal_False, &pTmpItem);
        if (eState == SFX_ITEM_SET)
            return pTmpItem;
        else
        {
            sal_Bool bOutliner = sal_False;
            sal_Bool bTitle = sal_False;

            if( mpView )
                 {
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                const sal_uInt32 nCount = rMarkList.GetMarkCount();

                for(sal_uInt32 nNum = 0; nNum < nCount; nNum++)
                {
                    SdrObject* pObj = rMarkList.GetMark(nNum)->GetMarkedSdrObj();
                    if( pObj->GetObjInventor() == SdrInventor )
                    {
                        switch(pObj->GetObjIdentifier())
                        {
                        case OBJ_TITLETEXT:
                            bTitle = sal_True;
                            break;
                        case OBJ_OUTLINETEXT:
                            bOutliner = sal_True;
                            break;
                        }
                    }
                }
            }

            const SvxNumBulletItem *pItem = NULL;
            if(bOutliner)
            {
                SfxStyleSheetBasePool* pSSPool = mpView->GetDocSh()->GetStyleSheetPool();
                String aStyleName((SdResId(STR_LAYOUT_OUTLINE)));
                aStyleName.AppendAscii( " 1" );
                SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);
                if( pFirstStyleSheet )
                    pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, sal_False, (const SfxPoolItem**)&pItem);
            }

            if( pItem == NULL )
                pItem = (SvxNumBulletItem*) aNewAttr.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET);

            aNewAttr.Put(*pItem, EE_PARA_NUMBULLET);

            if(bTitle && aNewAttr.GetItemState(EE_PARA_NUMBULLET,sal_True) == SFX_ITEM_ON )
            {
                SvxNumBulletItem* pBulletItem = (SvxNumBulletItem*)aNewAttr.GetItem(EE_PARA_NUMBULLET,sal_True);
                SvxNumRule* pRule = pBulletItem->GetNumRule();
                if(pRule)
                {
                    SvxNumRule aNewRule( *pRule );
                    aNewRule.SetFeatureFlag( NUM_NO_NUMBERS, sal_True );

                    SvxNumBulletItem aNewItem( aNewRule, EE_PARA_NUMBULLET );
                    aNewAttr.Put(aNewItem);
                }
            }

            SfxItemState eNumState = aNewAttr.GetItemState(nNumItemId, sal_False, &pTmpItem);
            if (eNumState == SFX_ITEM_SET)
                return pTmpItem;

        }
    }
    return pTmpItem;
}


sal_Bool ViewShell::HasRuler (void)
{
    return mbHasRulers;
}




void ViewShell::Resize (void)
{
    SetupRulers ();

    if (mpParentWindow == NULL)
        return;

    // Make sure that the new size is not degenerate.
    const Size aSize (mpParentWindow->GetSizePixel());
    if (aSize.Width()==0 || aSize.Height()==0)
        return;

    // Remember the new position and size.
    maViewPos = Point(0,0);
    maViewSize = aSize;

    // Rearrange the UI elements to take care of the new position and size.
    ArrangeGUIElements ();
    // end of included AdjustPosSizePixel.

    ::sd::View* pView = GetView();

    if (pView)
    {
        pView->VisAreaChanged(GetActiveWindow());
    }
}

SvBorder ViewShell::GetBorder (bool )
{
    SvBorder aBorder;

    // Horizontal scrollbar.
    if (mpHorizontalScrollBar.get()!=NULL
        && mpHorizontalScrollBar->IsVisible())
    {
        aBorder.Bottom() = maScrBarWH.Height();
    }

    // Vertical scrollbar.
    if (mpVerticalScrollBar.get()!=NULL
        && mpVerticalScrollBar->IsVisible())
    {
        aBorder.Right() = maScrBarWH.Width();
    }

    // Place horizontal ruler below tab bar.
    if (mbHasRulers && mpContentWindow.get() != NULL)
    {
        SetupRulers();
        if (mpHorizontalRuler.get() != NULL)
            aBorder.Top() = mpHorizontalRuler->GetSizePixel().Height();
        if (mpVerticalRuler.get() != NULL)
            aBorder.Left() = mpVerticalRuler->GetSizePixel().Width();
    }

    return aBorder;
}




void ViewShell::ArrangeGUIElements (void)
{
    if (mpImpl->mbArrangeActive)
        return;
    mpImpl->mbArrangeActive = true;

    // Calculate border for in-place editing.
    long nLeft = maViewPos.X();
    long nTop  = maViewPos.Y();
    long nRight = maViewPos.X() + maViewSize.Width();
    long nBottom = maViewPos.Y() + maViewSize.Height();

    // Horizontal scrollbar.
    if (mpHorizontalScrollBar.get()!=NULL
        && mpHorizontalScrollBar->IsVisible())
    {
        int nLocalLeft = nLeft;
        if (mpLayerTabBar.get()!=NULL && mpLayerTabBar->IsVisible())
            nLocalLeft += mpLayerTabBar->GetSizePixel().Width();
        nBottom -= maScrBarWH.Height();
        mpHorizontalScrollBar->SetPosSizePixel (
            Point(nLocalLeft,nBottom),
            Size(nRight-nLocalLeft-maScrBarWH.Width(),maScrBarWH.Height()));
    }

    // Vertical scrollbar.
    if (mpVerticalScrollBar.get()!=NULL
        && mpVerticalScrollBar->IsVisible())
    {
        nRight -= maScrBarWH.Width();
        mpVerticalScrollBar->SetPosSizePixel (
            Point(nRight,nTop),
            Size (maScrBarWH.Width(),nBottom-nTop));
    }

    // Filler in the lower right corner.
    if (mpScrollBarBox.get() != NULL)
    {
        if (mpHorizontalScrollBar.get()!=NULL
            && mpHorizontalScrollBar->IsVisible()
            && mpVerticalScrollBar.get()!=NULL
            && mpVerticalScrollBar->IsVisible())
        {
            mpScrollBarBox->Show();
            mpScrollBarBox->SetPosSizePixel(Point(nRight, nBottom), maScrBarWH);
        }
        else
            mpScrollBarBox->Hide();
    }

    // Place horizontal ruler below tab bar.
    if (mbHasRulers && mpContentWindow.get() != NULL)
    {
        if (mpHorizontalRuler.get() != NULL)
        {
            Size aRulerSize = mpHorizontalRuler->GetSizePixel();
            aRulerSize.Width() = nRight - nLeft;
            mpHorizontalRuler->SetPosSizePixel (
                Point(nLeft,nTop), aRulerSize);
            if (mpVerticalRuler.get() != NULL)
                mpHorizontalRuler->SetBorderPos(
                    mpVerticalRuler->GetSizePixel().Width()-1);
            nTop += aRulerSize.Height();
        }
        if (mpVerticalRuler.get() != NULL)
        {
            Size aRulerSize = mpVerticalRuler->GetSizePixel();
            aRulerSize.Height() = nBottom  - nTop;
            mpVerticalRuler->SetPosSizePixel (
                Point (nLeft,nTop), aRulerSize);
            nLeft += aRulerSize.Width();
        }
    }

    rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );

    // The size of the window of the center pane is set differently from
    // that of the windows in the docking windows.
    bool bSlideShowActive = (xSlideShow.is() && xSlideShow->isRunning()) && !xSlideShow->isFullScreen() && xSlideShow->getAnimationMode() == ANIMATIONMODE_SHOW;
    if ( !bSlideShowActive)
    {
        OSL_ASSERT (GetViewShell()!=NULL);

        if (mpContentWindow)
            mpContentWindow->SetPosSizePixel(
                Point(nLeft,nTop),
                Size(nRight-nLeft,nBottom-nTop));
    }

    // Windows in the center and rulers at the left and top side.
    maAllWindowRectangle = Rectangle(
        maViewPos,
        Size(maViewSize.Width()-maScrBarWH.Width(),
            maViewSize.Height()-maScrBarWH.Height()));

    if (mpContentWindow.get() != NULL)
    {
        mpContentWindow->UpdateMapOrigin();
    }

    UpdateScrollBars();

    mpImpl->mbArrangeActive = false;
}




void ViewShell::SetUIUnit(FieldUnit eUnit)
{
    // Set unit at horizontal and vertical rulers.
    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetUnit(eUnit);


    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->SetUnit(eUnit);
}

/**
 * set DefTab at horizontal rulers
 */
void ViewShell::SetDefTabHRuler( sal_uInt16 nDefTab )
{
    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->SetDefTabDist( nDefTab );
}




/** Tell the FmFormShell that the view shell is closing.  Give it the
    oportunity to prevent that.
*/
sal_uInt16 ViewShell::PrepareClose (sal_Bool bUI, sal_Bool bForBrowsing)
{
    sal_uInt16 nResult = sal_True;

    FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager()->GetFormShell();
    if (pFormShell != NULL)
        nResult = pFormShell->PrepareClose (bUI, bForBrowsing);

    return nResult;
}




void ViewShell::UpdatePreview (SdPage*, sal_Bool )
{
    // Do nothing.  After the actual preview has been removed,
    // OutlineViewShell::UpdatePreview() is the place where something
    // useful is still done.
}

::svl::IUndoManager* ViewShell::ImpGetUndoManager (void) const
{
    const ViewShell* pMainViewShell = GetViewShellBase().GetMainViewShell().get();

    if( pMainViewShell == 0 )
        pMainViewShell = this;

    ::sd::View* pView = pMainViewShell->GetView();

    // check for text edit our outline view
    if( pView )
    {
        if( pMainViewShell->GetShellType() == ViewShell::ST_OUTLINE )
        {
            OutlineView* pOlView = dynamic_cast< OutlineView* >( pView );
            if( pOlView )
            {
                ::Outliner* pOutl = pOlView->GetOutliner();
                if( pOutl )
                    return &pOutl->GetUndoManager();
            }
        }
        else if( pView->IsTextEdit() )
        {
            SdrOutliner* pOL = pView->GetTextEditOutliner();
            if( pOL )
                return &pOL->GetUndoManager();
        }
    }

    if( GetDocSh() )
        return GetDocSh()->GetUndoManager();

    return NULL;
}




void ViewShell::ImpGetUndoStrings(SfxItemSet &rSet) const
{
    ::svl::IUndoManager* pUndoManager = ImpGetUndoManager();
    if(pUndoManager)
    {
        sal_uInt16 nCount(pUndoManager->GetUndoActionCount());
        if(nCount)
        {
            // prepare list
            std::vector<OUString> aStringList;

            for (sal_uInt16 a = 0; a < nCount; ++a)
            {
                // generate one String in list per undo step
                aStringList.push_back( pUndoManager->GetUndoActionComment(a) );
            }

            // set item
            rSet.Put(SfxStringListItem(SID_GETUNDOSTRINGS, &aStringList));
        }
        else
        {
            rSet.DisableItem(SID_GETUNDOSTRINGS);
        }
    }
}

// -----------------------------------------------------------------------------

void ViewShell::ImpGetRedoStrings(SfxItemSet &rSet) const
{
    ::svl::IUndoManager* pUndoManager = ImpGetUndoManager();
    if(pUndoManager)
    {
        sal_uInt16 nCount(pUndoManager->GetRedoActionCount());
        if(nCount)
        {
            // prepare list
            ::std::vector< OUString > aStringList;
            sal_uInt16 a;

            for( a = 0; a < nCount; a++)
            {
                // generate one String in list per undo step
                aStringList.push_back( pUndoManager->GetRedoActionComment(a) );
            }

            // set item
            rSet.Put(SfxStringListItem(SID_GETREDOSTRINGS, &aStringList));
        }
        else
        {
            rSet.DisableItem(SID_GETREDOSTRINGS);
        }
    }
}

// -----------------------------------------------------------------------------

void ViewShell::ImpSidUndo(sal_Bool, SfxRequest& rReq)
{
    ::svl::IUndoManager* pUndoManager = ImpGetUndoManager();
    sal_uInt16 nNumber(1);
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    if(pReqArgs)
    {
        SfxUInt16Item* pUIntItem = (SfxUInt16Item*)&pReqArgs->Get(SID_UNDO);
        nNumber = pUIntItem->GetValue();
    }

    if(nNumber && pUndoManager)
    {
        sal_uInt16 nCount(pUndoManager->GetUndoActionCount());
        if(nCount >= nNumber)
        {
            try
            {
                // when UndoStack is cleared by ModifyPageUndoAction
                // the nCount may have changed, so test GetUndoActionCount()
                while(nNumber-- && pUndoManager->GetUndoActionCount())
                {
                    pUndoManager->Undo();
                }
            }
            catch( const Exception& )
            {
                // no need to handle. By definition, the UndoManager handled this by clearing the
                // Undo/Redo stacks
            }
        }

        // refresh rulers, maybe UNDO was move of TAB marker in ruler
        if (mbHasRulers)
        {
            Invalidate(SID_ATTR_TABSTOP);
        }
    }

    // This one is corresponding to the default handling
    // of SID_UNDO in sfx2
    GetViewFrame()->GetBindings().InvalidateAll(sal_False);

    rReq.Done();
}

// -----------------------------------------------------------------------------

void ViewShell::ImpSidRedo(sal_Bool, SfxRequest& rReq)
{
    ::svl::IUndoManager* pUndoManager = ImpGetUndoManager();
    sal_uInt16 nNumber(1);
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    if(pReqArgs)
    {
        SfxUInt16Item* pUIntItem = (SfxUInt16Item*)&pReqArgs->Get(SID_REDO);
        nNumber = pUIntItem->GetValue();
    }

    if(nNumber && pUndoManager)
    {
        sal_uInt16 nCount(pUndoManager->GetRedoActionCount());
        if(nCount >= nNumber)
        {
            try
            {
                // when UndoStack is cleared by ModifyPageRedoAction
                // the nCount may have changed, so test GetRedoActionCount()
                while(nNumber-- && pUndoManager->GetRedoActionCount())
                {
                    pUndoManager->Redo();
                }
            }
            catch( const Exception& )
            {
                // no need to handle. By definition, the UndoManager handled this by clearing the
                // Undo/Redo stacks
            }
        }

        // refresh rulers, maybe REDO was move of TAB marker in ruler
        if (mbHasRulers)
        {
            Invalidate(SID_ATTR_TABSTOP);
        }
    }

    // This one is corresponding to the default handling
    // of SID_UNDO in sfx2
    GetViewFrame()->GetBindings().InvalidateAll(sal_False);

    rReq.Done();
}

// -----------------------------------------------------------------------------

void ViewShell::ExecReq( SfxRequest& rReq )
{
    sal_uInt16 nSlot = rReq.GetSlot();
    switch( nSlot )
    {
        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            FunctionReference xFunc( GetCurrentFunction() );
            if( xFunc.is() )
                ScrollLines( 0, -1 );

            rReq.Done();
        }
        break;

        case SID_OUTPUT_QUALITY_COLOR:
        case SID_OUTPUT_QUALITY_GRAYSCALE:
        case SID_OUTPUT_QUALITY_BLACKWHITE:
        case SID_OUTPUT_QUALITY_CONTRAST:
        {
            sal_uLong nMode = OUTPUT_DRAWMODE_COLOR;

            switch( nSlot )
            {
                case SID_OUTPUT_QUALITY_COLOR: nMode = OUTPUT_DRAWMODE_COLOR; break;
                case SID_OUTPUT_QUALITY_GRAYSCALE: nMode = OUTPUT_DRAWMODE_GRAYSCALE; break;
                case SID_OUTPUT_QUALITY_BLACKWHITE: nMode = OUTPUT_DRAWMODE_BLACKWHITE; break;
                case SID_OUTPUT_QUALITY_CONTRAST: nMode = OUTPUT_DRAWMODE_CONTRAST; break;
            }

            GetActiveWindow()->SetDrawMode( nMode );
            mpFrameView->SetDrawMode( nMode );

            GetActiveWindow()->Invalidate();

            Invalidate();
            rReq.Done();
            break;
        }
    }
}




/** This default implemenation returns only an empty reference.  See derived
    classes for more interesting examples.
*/
::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible>
ViewShell::CreateAccessibleDocumentView (::sd::Window* )
{
    OSL_FAIL("ViewShell::CreateAccessibleDocumentView should not be called!, perhaps Meyers, 3rd edition, Item 9:\n");

    return ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> ();
}



::sd::WindowUpdater* ViewShell::GetWindowUpdater (void) const
{
    return mpWindowUpdater.get();
}




ViewShellBase& ViewShell::GetViewShellBase (void) const
{
    return *static_cast<ViewShellBase*>(GetViewShell());
}




ViewShell::ShellType ViewShell::GetShellType (void) const
{
    return meShellType;
}




DrawDocShell* ViewShell::GetDocSh (void) const
{
    return GetViewShellBase().GetDocShell();
}




SdDrawDocument* ViewShell::GetDoc (void) const
{
    return GetViewShellBase().GetDocument();
}

ErrCode ViewShell::DoVerb (long )
{
    return ERRCODE_NONE;
}

void ViewShell::SetCurrentFunction( const FunctionReference& xFunction)
{
    if( mxCurrentFunction.is() && (mxOldFunction != mxCurrentFunction) )
        mxCurrentFunction->Dispose();
    FunctionReference xTemp( mxCurrentFunction );
    mxCurrentFunction = xFunction;
}

void ViewShell::SetOldFunction(const FunctionReference& xFunction)
{
    if( mxOldFunction.is() && (xFunction != mxOldFunction) && (mxCurrentFunction != mxOldFunction) )
        mxOldFunction->Dispose();

    FunctionReference xTemp( mxOldFunction );
    mxOldFunction = xFunction;
}

/** this method deactivates the current function. If an old function is
    saved, this will become activated and current function.
*/
void ViewShell::Cancel()
{
    if(mxCurrentFunction.is() && (mxCurrentFunction != mxOldFunction ))
    {
        FunctionReference xTemp( mxCurrentFunction );
        mxCurrentFunction.clear();
        xTemp->Deactivate();
        xTemp->Dispose();
    }

    if(mxOldFunction.is())
    {
        mxCurrentFunction = mxOldFunction;
        mxCurrentFunction->Activate();
    }
}

void ViewShell::DeactivateCurrentFunction( bool bPermanent /* == false */ )
{
    if( mxCurrentFunction.is() )
    {
        if(bPermanent && (mxOldFunction == mxCurrentFunction))
            mxOldFunction.clear();

        mxCurrentFunction->Deactivate();
        if( mxCurrentFunction != mxOldFunction )
            mxCurrentFunction->Dispose();

        FunctionReference xTemp( mxCurrentFunction );
        mxCurrentFunction.clear();
    }
}

void ViewShell::DisposeFunctions()
{
    if(mxCurrentFunction.is())
    {
        FunctionReference xTemp( mxCurrentFunction );
        mxCurrentFunction.clear();
        xTemp->Deactivate();
        xTemp->Dispose();
    }

    if(mxOldFunction.is())
    {
        FunctionReference xTemp( mxOldFunction );
        mxOldFunction->Dispose();
        mxOldFunction.clear();
    }
}

bool ViewShell::IsMainViewShell (void) const
{
    return mpImpl->mbIsMainViewShell;
}

void ViewShell::SetIsMainViewShell (bool bIsMainViewShell)
{
    if (bIsMainViewShell != mpImpl->mbIsMainViewShell)
    {
        mpImpl->mbIsMainViewShell = bIsMainViewShell;
        if (bIsMainViewShell)
        {
            GetDocSh()->Connect (this);
        }
        else
        {
            GetDocSh()->Disconnect (this);
        }
    }
}




::sd::Window* ViewShell::GetActiveWindow (void) const
{
    return mpActiveWindow;
}




void ViewShell::PrePaint()
{
}




void ViewShell::Paint (const Rectangle&, ::sd::Window* )
{
}




void ViewShell::Draw(OutputDevice &, const Region &)
{
}




ZoomList* ViewShell::GetZoomList (void)
{
    return mpZoomList;
}




void ViewShell::ShowUIControls (bool bVisible)
{
    mpImpl->mbIsShowingUIControls = bVisible;

    if (mbHasRulers)
    {
        if (mpHorizontalRuler.get() != NULL)
            mpHorizontalRuler->Show( bVisible );

        if (mpVerticalRuler.get() != NULL)
            mpVerticalRuler->Show( bVisible );
    }

    if (mpVerticalScrollBar.get() != NULL)
        mpVerticalScrollBar->Show( bVisible );

    if (mpHorizontalScrollBar.get() != NULL)
        mpHorizontalScrollBar->Show( bVisible );

    if (mpScrollBarBox.get() != NULL)
        mpScrollBarBox->Show(bVisible);

    if (mpContentWindow.get() != NULL)
        mpContentWindow->Show( bVisible );
}





bool ViewShell::RelocateToParentWindow (::Window* pParentWindow)
{
    mpParentWindow = pParentWindow;

    mpParentWindow->SetBackground (Wallpaper());

    if (mpContentWindow.get() != NULL)
        mpContentWindow->SetParent(pParentWindow);

    if (mpHorizontalScrollBar.get() != NULL)
        mpHorizontalScrollBar->SetParent(mpParentWindow);
    if (mpVerticalScrollBar.get() != NULL)
        mpVerticalScrollBar->SetParent(mpParentWindow);
    if (mpScrollBarBox.get() != NULL)
        mpScrollBarBox->SetParent(mpParentWindow);

    return true;
}



} // end of namespace sd





//===== ViewShellObjectBarFactory =============================================

namespace {

ViewShellObjectBarFactory::ViewShellObjectBarFactory (
    ::sd::ViewShell& rViewShell)
    : mrViewShell (rViewShell)
{
}




ViewShellObjectBarFactory::~ViewShellObjectBarFactory (void)
{
    for (ShellCache::iterator aI(maShellCache.begin());
         aI!=maShellCache.end();
         ++aI)
    {
        delete aI->second;
    }
}




SfxShell* ViewShellObjectBarFactory::CreateShell (
    ::sd::ShellId nId,
    ::Window*,
    ::sd::FrameView* )
{
    SfxShell* pShell = NULL;

    ShellCache::iterator aI (maShellCache.find(nId));
    if (aI == maShellCache.end() || aI->second==NULL)
    {
        ::sd::View* pView = mrViewShell.GetView();
        switch (nId)
        {
            case RID_BEZIER_TOOLBOX:
                pShell = new ::sd::BezierObjectBar(&mrViewShell, pView);
                break;

            case RID_DRAW_TEXT_TOOLBOX:
                pShell = new ::sd::TextObjectBar(
                    &mrViewShell, mrViewShell.GetDoc()->GetPool(), pView);
                break;

            case RID_DRAW_GRAF_TOOLBOX:
                pShell = new ::sd::GraphicObjectBar(&mrViewShell, pView);
                break;

            case RID_DRAW_MEDIA_TOOLBOX:
                pShell = new ::sd::MediaObjectBar(&mrViewShell, pView);
                break;

            case RID_DRAW_TABLE_TOOLBOX:
                pShell = ::sd::ui::table::CreateTableObjectBar( mrViewShell, pView );
                break;

            case RID_SVX_EXTRUSION_BAR:
                pShell = new ::svx::ExtrusionBar(
                    &mrViewShell.GetViewShellBase());
                break;

            case RID_SVX_FONTWORK_BAR:
                pShell = new ::svx::FontworkBar(
                    &mrViewShell.GetViewShellBase());
                break;

            default:
                pShell = NULL;
                break;
        }
    }
    else
        pShell = aI->second;

    return pShell;
}




void ViewShellObjectBarFactory::ReleaseShell (SfxShell* pShell)
{
    if (pShell != NULL)
        delete pShell;
}

} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
