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
#include "createtableobjectbar.hxx"

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
#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "ViewShellManager.hxx"
#include "FormShellManager.hxx"
#include <svx/dialogs.hrc>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/svdoutl.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/lok.hxx>

#include <svl/slstitm.hxx>
#include <sfx2/request.hxx>
#include "SpellDialogChildWindow.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsSelectionObserver.hxx"
#include "view/SlideSorterView.hxx"

#include <basegfx/tools/zoomtools.hxx>

#include "Window.hxx"
#include "fupoor.hxx"

#include <editeng/numitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <svl/poolitem.hxx>
#include <glob.hrc>
#include "AccessibleDocumentViewBase.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

namespace {

class ViewShellObjectBarFactory
    : public ::sd::ShellFactory<SfxShell>
{
public:
    explicit ViewShellObjectBarFactory (::sd::ViewShell& rViewShell);
    virtual ~ViewShellObjectBarFactory();
    virtual SfxShell* CreateShell (
        ::sd::ShellId nId,
        vcl::Window* pParentWindow,
        ::sd::FrameView* pFrameView = nullptr) override;
    virtual void ReleaseShell (SfxShell* pShell) override;
private:
    ::sd::ViewShell& mrViewShell;
    /** This cache holds the already created object bars.
    */
    typedef ::std::map< ::sd::ShellId,SfxShell*> ShellCache;
    ShellCache maShellCache;
};

} // end of anonymous namespace

namespace sd {

bool ViewShell::IsPageFlipMode() const
{
    return dynamic_cast< const DrawViewShell *>( this ) !=  nullptr && mpContentWindow.get() != nullptr &&
        mpContentWindow->GetVisibleHeight() >= 1.0;
}

SfxViewFrame* ViewShell::GetViewFrame() const
{
    const SfxViewShell* pViewShell = GetViewShell();
    if (pViewShell != nullptr)
    {
        return pViewShell->GetViewFrame();
    }
    else
    {
        OSL_ASSERT (GetViewShell()!=nullptr);
        return nullptr;
    }
}

/// declare SFX-Slotmap and standard interface

ViewShell::ViewShell( SfxViewFrame*, vcl::Window* pParentWindow, ViewShellBase& rViewShellBase)
:   SfxShell(&rViewShellBase)
,   mpParentWindow(pParentWindow)
{
    construct();
}

ViewShell::~ViewShell()
{
    // Keep the content window from accessing in its destructor the
    // WindowUpdater.
    if (mpContentWindow)
        mpContentWindow->SetViewShell(nullptr);

    delete mpZoomList;

    mpLayerTabBar.disposeAndClear();

    if (mpImpl->mpSubShellFactory.get() != nullptr)
        GetViewShellBase().GetViewShellManager()->RemoveSubShellFactory(
            this,mpImpl->mpSubShellFactory);

    if (mpContentWindow)
    {
        SAL_INFO(
            "sd.ui",
            "destroying mpContentWindow at " << mpContentWindow.get()
                << " with parent " << mpContentWindow->GetParent());
        mpContentWindow.disposeAndClear();
    }

    mpScrollBarBox.disposeAndClear();
    mpVerticalRuler.disposeAndClear();
    mpHorizontalRuler.disposeAndClear();
    mpVerticalScrollBar.disposeAndClear();
    mpHorizontalScrollBar.disposeAndClear();
}

/**
 * common initialization part of both constructors
 */
void ViewShell::construct()
{
    mbHasRulers = false;
    mpActiveWindow = nullptr;
    mpView = nullptr;
    mpFrameView = nullptr;
    mpZoomList = nullptr;
    mbStartShowWithDialog = false;
    mnPrintedHandoutPageNum = 1;
    mnPrintedHandoutPageCount = 0;
    mpWindowUpdater.reset( new ::sd::WindowUpdater() );
    mpImpl.reset(new Implementation(*this));
    meShellType = ST_NONE;

    OSL_ASSERT (GetViewShell()!=nullptr);

    if (IsMainViewShell())
        GetDocSh()->Connect (this);

    mpZoomList = new ZoomList( this );

    mpContentWindow.reset(VclPtr< ::sd::Window >::Create(GetParentWindow()));
    SetActiveWindow (mpContentWindow.get());

    GetParentWindow()->SetBackground (Wallpaper());
    mpContentWindow->SetBackground (Wallpaper());
    mpContentWindow->SetCenterAllowed(true);
    mpContentWindow->SetViewShell(this);
    mpContentWindow->SetPosSizePixel(
        GetParentWindow()->GetPosPixel(),GetParentWindow()->GetSizePixel());

    if ( ! GetDocSh()->IsPreview())
    {
        // Create scroll bars and the filler between the scroll bars.
        mpHorizontalScrollBar.reset (VclPtr<ScrollBar>::Create(GetParentWindow(), WinBits(WB_HSCROLL | WB_DRAG)));
        mpHorizontalScrollBar->EnableRTL (false);
        mpHorizontalScrollBar->SetRange(Range(0, 32000));
        mpHorizontalScrollBar->SetScrollHdl(LINK(this, ViewShell, HScrollHdl));

        mpVerticalScrollBar.reset (VclPtr<ScrollBar>::Create(GetParentWindow(), WinBits(WB_VSCROLL | WB_DRAG)));
        mpVerticalScrollBar->SetRange(Range(0, 32000));
        mpVerticalScrollBar->SetScrollHdl(LINK(this, ViewShell, VScrollHdl));

        mpScrollBarBox.reset(VclPtr<ScrollBarBox>::Create(GetParentWindow(), WB_SIZEABLE));
    }

    OUString aName( "ViewShell" );
    SetName (aName);

    GetDoc()->StartOnlineSpelling(false);

    mpWindowUpdater->SetViewShell (*this);
    mpWindowUpdater->SetDocument (GetDoc());

    // Re-initialize the spell dialog.
    ::sd::SpellDialogChildWindow* pSpellDialog =
          static_cast< ::sd::SpellDialogChildWindow*> (
              GetViewFrame()->GetChildWindow (
                  ::sd::SpellDialogChildWindow::GetChildWindowId()));
    if (pSpellDialog != nullptr)
        pSpellDialog->InvalidateSpellDialog();

    // Register the sub shell factory.
    mpImpl->mpSubShellFactory.reset(new ViewShellObjectBarFactory(*this));
    GetViewShellBase().GetViewShellManager()->AddSubShellFactory(this,mpImpl->mpSubShellFactory);
}

void ViewShell::doShow()
{
    mpContentWindow->Show();
    static_cast< vcl::Window*>(mpContentWindow.get())->Resize();
    SAL_INFO(
        "sd.view",
        "content window has size " << mpContentWindow->GetSizePixel().Width()
            << " " << mpContentWindow->GetSizePixel().Height());

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

void ViewShell::Exit()
{
    sd::View* pView = GetView();
    if (pView!=nullptr && pView->IsTextEdit())
    {
        pView->SdrEndTextEdit();
        pView->UnmarkAll();
    }

    Deactivate (true);

    if (IsMainViewShell())
        GetDocSh()->Disconnect(this);

    SetIsMainViewShell(false);
}

/**
 * set focus to working window
 */
void ViewShell::Activate(bool bIsMDIActivate)
{
    // Do not forward to SfxShell::Activate()

    /* According to MI, nobody is allowed to call GrabFocus, who does not
       exactly know from which window the focus is grabbed. Since Activate()
       is sent sometimes asynchronous,  it can happen, that the wrong window
       gets the focus. */

    if (mpHorizontalRuler.get() != nullptr)
        mpHorizontalRuler->SetActive();
    if (mpVerticalRuler.get() != nullptr)
        mpVerticalRuler->SetActive();

    if (bIsMDIActivate)
    {
        // thus, the Navigator will also get a current status
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, true );
        if (GetDispatcher() != nullptr)
            GetDispatcher()->ExecuteList(
                SID_NAVIGATOR_INIT,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                { &aItem });

        SfxViewShell* pViewShell = GetViewShell();
        OSL_ASSERT (pViewShell!=nullptr);
        SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_3D_STATE, true );

        rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
        if(xSlideShow.is() && xSlideShow->isRunning() )
            xSlideShow->activate(GetViewShellBase());

        if(HasCurrentFunction())
            GetCurrentFunction()->Activate();

        if(!GetDocSh()->IsUIActive())
            UpdatePreview( GetActualPage(), true );
    }

    ReadFrameViewData( mpFrameView );

    if (IsMainViewShell())
        GetDocSh()->Connect(this);
}

void ViewShell::UIActivating( SfxInPlaceClient*  )
{
    OSL_ASSERT (GetViewShell()!=nullptr);
    GetViewShellBase().GetToolBarManager()->ToolBarsDestroyed();
}

void ViewShell::UIDeactivated( SfxInPlaceClient*  )
{
    OSL_ASSERT (GetViewShell()!=nullptr);
    GetViewShellBase().GetToolBarManager()->ToolBarsDestroyed();
    if ( GetDrawView() )
        GetViewShellBase().GetToolBarManager()->SelectionHasChanged(*this, *GetDrawView());
}

void ViewShell::Deactivate(bool bIsMDIActivate)
{
    // remove view from a still active drag'n'drop session
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

    if (IsMainViewShell())
        GetDocSh()->Disconnect(this);

    if( pDragTransferable )
        pDragTransferable->SetView( nullptr );

    OSL_ASSERT (GetViewShell()!=nullptr);

    // remember view attributes of FrameView
    WriteFrameViewData();

    if (bIsMDIActivate)
    {
        rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
        if(xSlideShow.is() && xSlideShow->isRunning() )
            xSlideShow->deactivate(GetViewShellBase());

        if(HasCurrentFunction())
            GetCurrentFunction()->Deactivate();
    }

    if (mpHorizontalRuler.get() != nullptr)
        mpHorizontalRuler->SetActive(false);
    if (mpVerticalRuler.get() != nullptr)
        mpVerticalRuler->SetActive(false);

    SfxShell::Deactivate(bIsMDIActivate);
}

void ViewShell::Shutdown()
{
    Exit ();
}

bool ViewShell::KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    bool bReturn(false);

    if(pWin)
        SetActiveWindow(pWin);

    if(!bReturn)
    {
        // give key input first to SfxViewShell to give CTRL+Key
        // (e.g. CTRL+SHIFT+'+', to front) priority.
        OSL_ASSERT (GetViewShell()!=nullptr);
        bReturn = GetViewShell()->KeyInput(rKEvt);
    }

    const size_t OriCount = GetView()->GetMarkedObjectList().GetMarkCount();
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
                    bReturn = true;
                }
            }
        }
    }
    const size_t EndCount = GetView()->GetMarkedObjectList().GetMarkCount();
    // Here, oriCount or endCount must have one value=0, another value > 0, then to switch focus between Document and shape objects
    if(bReturn &&  (OriCount + EndCount > 0) && (OriCount * EndCount == 0))
        SwitchActiveViewFireFocus();

    if(!bReturn && GetActiveWindow())
    {
        vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

        if (aKeyCode.IsMod1() && aKeyCode.IsShift()
            && aKeyCode.GetCode() == KEY_R)
        {
            InvalidateWindows();
            bReturn = true;
        }
    }

    return bReturn;
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
    if (GetView() != nullptr)
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
                GetCurrentFunction()->MouseButtonDown(rMEvt);
        }
    }
}

void ViewShell::LogicMouseButtonDown(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    Point aPoint = mpActiveWindow->GetPointerPosPixel();
    mpActiveWindow->SetLastMousePos(rMouseEvent.GetPosPixel());

    MouseButtonDown(rMouseEvent, mpActiveWindow);

    mpActiveWindow->SetPointerPosPixel(aPoint);
}

void ViewShell::LogicMouseButtonUp(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    Point aPoint = mpActiveWindow->GetPointerPosPixel();
    mpActiveWindow->SetLastMousePos(rMouseEvent.GetPosPixel());

    MouseButtonUp(rMouseEvent, mpActiveWindow);

    mpActiveWindow->SetPointerPosPixel(aPoint);
}

void ViewShell::LogicMouseMove(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    Point aPoint = mpActiveWindow->GetPointerPosPixel();
    mpActiveWindow->SetLastMousePos(rMouseEvent.GetPosPixel());

    MouseMove(rMouseEvent, mpActiveWindow);

    mpActiveWindow->SetPointerPosPixel(aPoint);
}

void ViewShell::SetCursorMm100Position(const Point& rPosition, bool bPoint, bool bClearMark)
{
    if (SdrView* pSdrView = GetView())
    {
        rtl::Reference<sdr::SelectionController> xSelectionController(GetView()->getSelectionController());
        if (!xSelectionController.is() || !xSelectionController->setCursorLogicPosition(rPosition, bPoint))
        {
            if (pSdrView->GetTextEditObject())
            {
                EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
                rEditView.SetCursorLogicPosition(rPosition, bPoint, bClearMark);
            }
        }
    }
}

OString ViewShell::GetTextSelection(const OString& _aMimeType, OString& rUsedMimeType)
{
    SdrView* pSdrView = GetView();
    if (!pSdrView)
        return OString();

    if (!pSdrView->GetTextEditObject())
        return OString();

    EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
    uno::Reference<datatransfer::XTransferable> xTransferable = rEditView.GetEditEngine()->CreateTransferable(rEditView.GetSelection());

    // Take care of UTF-8 text here.
    bool bConvert = false;
    sal_Int32 nIndex = 0;
    OString aMimeType = _aMimeType;
    if (aMimeType.getToken(0, ';', nIndex) == "text/plain")
    {
        if (aMimeType.getToken(0, ';', nIndex) == "charset=utf-8")
        {
            aMimeType = "text/plain;charset=utf-16";
            bConvert = true;
        }
    }

    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = OUString::fromUtf8(aMimeType.getStr());
    if (aMimeType == "text/plain;charset=utf-16")
        aFlavor.DataType = cppu::UnoType<OUString>::get();
    else
        aFlavor.DataType = cppu::UnoType< uno::Sequence<sal_Int8> >::get();

    if (!xTransferable->isDataFlavorSupported(aFlavor))
        return OString();

    uno::Any aAny(xTransferable->getTransferData(aFlavor));

    OString aRet;
    if (aFlavor.DataType == cppu::UnoType<OUString>::get())
    {
        OUString aString;
        aAny >>= aString;
        if (bConvert)
            aRet = OUStringToOString(aString, RTL_TEXTENCODING_UTF8);
        else
            aRet = OString(reinterpret_cast<const sal_Char *>(aString.getStr()), aString.getLength() * sizeof(sal_Unicode));
    }
    else
    {
        uno::Sequence<sal_Int8> aSequence;
        aAny >>= aSequence;
        aRet = OString(reinterpret_cast<sal_Char*>(aSequence.getArray()), aSequence.getLength());
    }

    rUsedMimeType = _aMimeType;
    return aRet;
}

void ViewShell::SetGraphicMm100Position(bool bStart, const Point& rPosition)
{
    if (bStart)
    {
        MouseEvent aClickEvent(rPosition, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
        MouseButtonDown(aClickEvent, mpActiveWindow);
        MouseEvent aMoveEvent(Point(rPosition.getX(), rPosition.getY()), 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
        MouseMove(aMoveEvent, mpActiveWindow);
    }
    else
    {
        MouseEvent aMoveEvent(Point(rPosition.getX(), rPosition.getY()), 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
        MouseMove(aMoveEvent, mpActiveWindow);
        MouseEvent aClickEvent(rPosition, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
        MouseButtonUp(aClickEvent, mpActiveWindow);
    }
}

void ViewShell::MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if (rMEvt.IsLeaveWindow())
    {
        if ( ! mpImpl->mpUpdateLockForMouse.expired())
        {
            std::shared_ptr<ViewShell::Implementation::ToolBarManagerLock> pLock(
                mpImpl->mpUpdateLockForMouse);
            if (pLock.get() != nullptr)
                pLock->Release();
        }
    }

    if ( pWin )
    {
        SetActiveWindow(pWin);
    }

    // insert MouseEvent into E3dView
    if (GetView() != nullptr)
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
        SetActiveWindow(pWin);

    // insert MouseEvent into E3dView
    if (GetView() != nullptr)
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
        std::shared_ptr<ViewShell::Implementation::ToolBarManagerLock> pLock(
            mpImpl->mpUpdateLockForMouse);
        if (pLock.get() != nullptr)
            pLock->Release();
    }
}

void ViewShell::Command(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    bool bDone = HandleScrollCommand (rCEvt, pWin);

    if( !bDone )
    {
        if( rCEvt.GetCommand() == CommandEventId::InputLanguageChange )
        {
            //#i42732# update state of fontname if input language changes
            GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONT );
            GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
        }
        else
        {
            bool bConsumed = false;
            if( GetView() )
                bConsumed = GetView()->getSmartTags().Command(rCEvt);

            if( !bConsumed && HasCurrentFunction())
                GetCurrentFunction()->Command(rCEvt);
        }
    }
}

bool ViewShell::Notify(NotifyEvent& rNEvt, ::sd::Window* pWin)
{
    // handle scroll commands when they arrived at child windows
    bool bRet = false;
    if( rNEvt.GetType() == MouseNotifyEvent::COMMAND )
    {
        // note: dynamic_cast is not possible as GetData() returns a void*
        CommandEvent* pCmdEvent = static_cast< CommandEvent* >(rNEvt.GetData());
        bRet = HandleScrollCommand(*pCmdEvent, pWin);
    }
    return bRet;
}

bool ViewShell::HandleScrollCommand(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    bool bDone = false;

    switch( rCEvt.GetCommand() )
    {
        case CommandEventId::Swipe:
            {
                rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
                if (xSlideShow.is())
                {
                    const CommandSwipeData* pSwipeData = rCEvt.GetSwipeData();
                    bDone = xSlideShow->swipe(*pSwipeData);
                }
            }
            break;
        case CommandEventId::LongPress:
            {
                rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
                if (xSlideShow.is())
                {
                    const CommandLongPressData* pLongPressData = rCEvt.GetLongPressData();
                    bDone = xSlideShow->longpress(*pLongPressData);
                }
            }
            break;

        case CommandEventId::Wheel:
            {
                Reference< XSlideShowController > xSlideShowController( SlideShow::GetSlideShowController(GetViewShellBase() ) );
                if( xSlideShowController.is() )
                {
                    // We ignore zooming with control+mouse wheel.
                    const CommandWheelData* pData = rCEvt.GetWheelData();
                    if( pData && !pData->GetModifier() && ( pData->GetMode() == CommandWheelMode::SCROLL ) && !pData->IsHorz() )
                    {
                        long nDelta = pData->GetDelta();
                        if( nDelta > 0 )
                            xSlideShowController->gotoPreviousSlide();
                        else if( nDelta < 0 )
                            xSlideShowController->gotoNextEffect();
                    }
                    break;
                }
            }
            SAL_FALLTHROUGH;
        case CommandEventId::StartAutoScroll:
        case CommandEventId::AutoScroll:
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();

            if (pData != nullptr)
            {
                if (pData->IsMod1())
                {
                    if( !GetDocSh()->IsUIActive() )
                    {
                        const long  nOldZoom = GetActiveWindow()->GetZoom();
                        long        nNewZoom;
                        Point aOldMousePos = GetActiveWindow()->PixelToLogic(rCEvt.GetMousePosPixel());

                        if( pData->GetDelta() < 0L )
                            nNewZoom = std::max( (long) pWin->GetMinZoom(), basegfx::zoomtools::zoomOut( nOldZoom ));
                        else
                            nNewZoom = std::min( (long) pWin->GetMaxZoom(), basegfx::zoomtools::zoomIn( nOldZoom ));

                        SetZoom( nNewZoom );
                        // Keep mouse at same doc point before zoom
                        Point aNewMousePos = GetActiveWindow()->PixelToLogic(rCEvt.GetMousePosPixel());
                        SetWinViewPos(GetWinViewPos() - (aNewMousePos - aOldMousePos));

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
                            rCEvt.IsMouseEvent(),static_cast<const void *>(&aWheelData) );
                        bDone = pWin->HandleScrollCommand( aReWrite,
                            mpHorizontalScrollBar.get(),
                            mpVerticalScrollBar.get());
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

void ViewShell::SetupRulers()
{
    if(mbHasRulers && (mpContentWindow.get() != nullptr) && !SlideShow::IsRunning(GetViewShellBase()))
    {
        long nHRulerOfs = 0;

        if ( mpVerticalRuler.get() == nullptr )
        {
            mpVerticalRuler.reset(CreateVRuler(GetActiveWindow()));
            if ( mpVerticalRuler.get() != nullptr )
            {
                nHRulerOfs = mpVerticalRuler->GetSizePixel().Width();
                mpVerticalRuler->SetActive();
                mpVerticalRuler->Show();
            }
        }
        if ( mpHorizontalRuler.get() == nullptr )
        {
            mpHorizontalRuler.reset(CreateHRuler(GetActiveWindow()));
            if ( mpHorizontalRuler.get() != nullptr )
            {
                mpHorizontalRuler->SetWinPos(nHRulerOfs);
                mpHorizontalRuler->SetActive();
                mpHorizontalRuler->Show();
            }
        }
    }
}

const SfxPoolItem* ViewShell::GetNumBulletItem(SfxItemSet& aNewAttr, sal_uInt16& nNumItemId)
{
    const SfxPoolItem* pTmpItem = nullptr;

    if(aNewAttr.GetItemState(nNumItemId, false, &pTmpItem) == SfxItemState::SET)
    {
        return pTmpItem;
    }
    else
    {
        nNumItemId = aNewAttr.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        SfxItemState eState = aNewAttr.GetItemState(nNumItemId, false, &pTmpItem);
        if (eState == SfxItemState::SET)
            return pTmpItem;
        else
        {
            bool bOutliner = false;
            bool bTitle = false;

            if( mpView )
            {
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                const size_t nCount = rMarkList.GetMarkCount();

                for(size_t nNum = 0; nNum < nCount; ++nNum)
                {
                    SdrObject* pObj = rMarkList.GetMark(nNum)->GetMarkedSdrObj();
                    if( pObj->GetObjInventor() == SdrInventor )
                    {
                        switch(pObj->GetObjIdentifier())
                        {
                        case OBJ_TITLETEXT:
                            bTitle = true;
                            break;
                        case OBJ_OUTLINETEXT:
                            bOutliner = true;
                            break;
                        }
                    }
                }
            }

            const SvxNumBulletItem *pItem = nullptr;
            if(bOutliner)
            {
                SfxStyleSheetBasePool* pSSPool = mpView->GetDocSh()->GetStyleSheetPool();
                OUString aStyleName(SD_RESSTR(STR_LAYOUT_OUTLINE) + " 1");
                SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);
                if( pFirstStyleSheet )
                    pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, false, reinterpret_cast<const SfxPoolItem**>(&pItem));
            }

            if( pItem == nullptr )
                pItem = static_cast<const SvxNumBulletItem*>( aNewAttr.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET) );

            std::unique_ptr<SfxPoolItem> pNewItem(pItem->CloneSetWhich(EE_PARA_NUMBULLET));
            aNewAttr.Put(*pNewItem);

            if(bTitle && aNewAttr.GetItemState(EE_PARA_NUMBULLET) == SfxItemState::SET )
            {
                const SvxNumBulletItem* pBulletItem = static_cast<const SvxNumBulletItem*>(aNewAttr.GetItem(EE_PARA_NUMBULLET));
                SvxNumRule* pRule = pBulletItem->GetNumRule();
                if(pRule)
                {
                    SvxNumRule aNewRule( *pRule );
                    aNewRule.SetFeatureFlag( SvxNumRuleFlags::NO_NUMBERS );

                    SvxNumBulletItem aNewItem( aNewRule, EE_PARA_NUMBULLET );
                    aNewAttr.Put(aNewItem);
                }
            }

            SfxItemState eNumState = aNewAttr.GetItemState(nNumItemId, false, &pTmpItem);
            if (eNumState == SfxItemState::SET)
                return pTmpItem;

        }
    }
    return pTmpItem;
}

void ViewShell::Resize()
{
    SetupRulers ();

    if (mpParentWindow == nullptr)
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
        pView->VisAreaChanged(GetActiveWindow());
}

SvBorder ViewShell::GetBorder (bool )
{
    SvBorder aBorder;

    // Horizontal scrollbar.
    if (mpHorizontalScrollBar.get()!=nullptr
        && mpHorizontalScrollBar->IsVisible())
    {
        aBorder.Bottom() = maScrBarWH.Height();
    }

    // Vertical scrollbar.
    if (mpVerticalScrollBar.get()!=nullptr
        && mpVerticalScrollBar->IsVisible())
    {
        aBorder.Right() = maScrBarWH.Width();
    }

    // Place horizontal ruler below tab bar.
    if (mbHasRulers && mpContentWindow.get() != nullptr)
    {
        SetupRulers();
        if (mpHorizontalRuler.get() != nullptr)
            aBorder.Top() = mpHorizontalRuler->GetSizePixel().Height();
        if (mpVerticalRuler.get() != nullptr)
            aBorder.Left() = mpVerticalRuler->GetSizePixel().Width();
    }

    return aBorder;
}

void ViewShell::ArrangeGUIElements()
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
    if (mpHorizontalScrollBar.get()!=nullptr
        && mpHorizontalScrollBar->IsVisible())
    {
        nBottom -= maScrBarWH.Height();
        if (mpLayerTabBar.get()!=nullptr && mpLayerTabBar->IsVisible())
            nBottom -= mpLayerTabBar->GetSizePixel().Height();
        mpHorizontalScrollBar->SetPosSizePixel (
            Point(nLeft, nBottom),
            Size(nRight - nLeft - maScrBarWH.Width(), maScrBarWH.Height()));
    }

    // Vertical scrollbar.
    if (mpVerticalScrollBar.get()!=nullptr
        && mpVerticalScrollBar->IsVisible())
    {
        nRight -= maScrBarWH.Width();
        mpVerticalScrollBar->SetPosSizePixel (
            Point(nRight,nTop),
            Size (maScrBarWH.Width(), nBottom-nTop));
    }

    // Filler in the lower right corner.
    if (mpScrollBarBox.get() != nullptr)
    {
        if (mpHorizontalScrollBar.get()!=nullptr
            && mpHorizontalScrollBar->IsVisible()
            && mpVerticalScrollBar.get()!=nullptr
            && mpVerticalScrollBar->IsVisible())
        {
            mpScrollBarBox->Show();
            mpScrollBarBox->SetPosSizePixel(Point(nRight, nBottom), maScrBarWH);
        }
        else
            mpScrollBarBox->Hide();
    }

    // Place horizontal ruler below tab bar.
    if (mbHasRulers && mpContentWindow.get() != nullptr)
    {
        if (mpHorizontalRuler.get() != nullptr)
        {
            Size aRulerSize = mpHorizontalRuler->GetSizePixel();
            aRulerSize.Width() = nRight - nLeft;
            mpHorizontalRuler->SetPosSizePixel (
                Point(nLeft,nTop), aRulerSize);
            if (mpVerticalRuler.get() != nullptr)
                mpHorizontalRuler->SetBorderPos(
                    mpVerticalRuler->GetSizePixel().Width()-1);
            nTop += aRulerSize.Height();
        }
        if (mpVerticalRuler.get() != nullptr)
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
        OSL_ASSERT (GetViewShell()!=nullptr);

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

    if (mpContentWindow.get() != nullptr)
        mpContentWindow->UpdateMapOrigin();

    UpdateScrollBars();

    mpImpl->mbArrangeActive = false;
}

void ViewShell::SetUIUnit(FieldUnit eUnit)
{
    // Set unit at horizontal and vertical rulers.
    if (mpHorizontalRuler.get() != nullptr)
        mpHorizontalRuler->SetUnit(eUnit);

    if (mpVerticalRuler.get() != nullptr)
        mpVerticalRuler->SetUnit(eUnit);
}

/**
 * set DefTab at horizontal rulers
 */
void ViewShell::SetDefTabHRuler( sal_uInt16 nDefTab )
{
    if (mpHorizontalRuler.get() != nullptr)
        mpHorizontalRuler->SetDefTabDist( nDefTab );
}

/** Tell the FmFormShell that the view shell is closing.  Give it the
    opportunity to prevent that.
*/
bool ViewShell::PrepareClose (bool bUI)
{
    bool bResult = true;

    FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager()->GetFormShell();
    if (pFormShell != nullptr)
        bResult = pFormShell->PrepareClose (bUI);

    return bResult;
}

void ViewShell::UpdatePreview (SdPage*, bool )
{
    // Do nothing.  After the actual preview has been removed,
    // OutlineViewShell::UpdatePreview() is the place where something
    // useful is still done.
}

::svl::IUndoManager* ViewShell::ImpGetUndoManager() const
{
    const ViewShell* pMainViewShell = GetViewShellBase().GetMainViewShell().get();

    if( pMainViewShell == nullptr )
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
                ::Outliner& rOutl = pOlView->GetOutliner();
                return &rOutl.GetUndoManager();
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

    return nullptr;
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
                // generate one String in list per undo step
                aStringList.push_back( pUndoManager->GetRedoActionComment(a) );

            // set item
            rSet.Put(SfxStringListItem(SID_GETREDOSTRINGS, &aStringList));
        }
        else
        {
            rSet.DisableItem(SID_GETREDOSTRINGS);
        }
    }
}

class KeepSlideSorterInSyncWithPageChanges
{
    sd::slidesorter::view::SlideSorterView::DrawLock m_aDrawLock;
    sd::slidesorter::controller::SlideSorterController::ModelChangeLock m_aModelLock;
    sd::slidesorter::controller::PageSelector::UpdateLock m_aUpdateLock;
    sd::slidesorter::controller::SelectionObserver::Context m_aContext;

public:
    explicit KeepSlideSorterInSyncWithPageChanges(sd::slidesorter::SlideSorter& rSlideSorter)
        : m_aDrawLock(rSlideSorter)
        , m_aModelLock(rSlideSorter.GetController())
        , m_aUpdateLock(rSlideSorter)
        , m_aContext(rSlideSorter)
    {
    }
};

void ViewShell::ImpSidUndo(bool, SfxRequest& rReq)
{
    //The xWatcher keeps the SlideSorter selection in sync
    //with the page insertions/deletions that Undo may introduce
    std::unique_ptr<KeepSlideSorterInSyncWithPageChanges> xWatcher;
    slidesorter::SlideSorterViewShell* pSlideSorterViewShell
        = slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase());
    if (pSlideSorterViewShell)
        xWatcher.reset(new KeepSlideSorterInSyncWithPageChanges(pSlideSorterViewShell->GetSlideSorter()));

    ::svl::IUndoManager* pUndoManager = ImpGetUndoManager();
    sal_uInt16 nNumber(1);
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    if(pReqArgs)
    {
        const SfxUInt16Item* pUIntItem = static_cast<const SfxUInt16Item*>(&pReqArgs->Get(SID_UNDO));
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
                    pUndoManager->Undo();
            }
            catch( const Exception& )
            {
                // no need to handle. By definition, the UndoManager handled this by clearing the
                // Undo/Redo stacks
            }
        }

        // refresh rulers, maybe UNDO was move of TAB marker in ruler
        if (mbHasRulers)
            Invalidate(SID_ATTR_TABSTOP);
    }

    // This one is corresponding to the default handling
    // of SID_UNDO in sfx2
    GetViewFrame()->GetBindings().InvalidateAll(false);

    rReq.Done();
}

void ViewShell::ImpSidRedo(bool, SfxRequest& rReq)
{
    //The xWatcher keeps the SlideSorter selection in sync
    //with the page insertions/deletions that Undo may introduce
    std::unique_ptr<KeepSlideSorterInSyncWithPageChanges> xWatcher;
    slidesorter::SlideSorterViewShell* pSlideSorterViewShell
        = slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase());
    if (pSlideSorterViewShell)
        xWatcher.reset(new KeepSlideSorterInSyncWithPageChanges(pSlideSorterViewShell->GetSlideSorter()));

    ::svl::IUndoManager* pUndoManager = ImpGetUndoManager();
    sal_uInt16 nNumber(1);
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    if(pReqArgs)
    {
        const SfxUInt16Item* pUIntItem = static_cast<const SfxUInt16Item*>(&pReqArgs->Get(SID_REDO));
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
                    pUndoManager->Redo();
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
    GetViewFrame()->GetBindings().InvalidateAll(false);

    rReq.Done();
}

void ViewShell::ExecReq( SfxRequest& rReq )
{
    sal_uInt16 nSlot = rReq.GetSlot();
    switch( nSlot )
    {
        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            rtl::Reference<FuPoor> xFunc( GetCurrentFunction() );
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
            DrawModeFlags nMode = OUTPUT_DRAWMODE_COLOR;

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

/** This default implementation returns only an empty reference.  See derived
    classes for more interesting examples.
*/
css::uno::Reference<css::accessibility::XAccessible>
ViewShell::CreateAccessibleDocumentView (::sd::Window* )
{
    OSL_FAIL("ViewShell::CreateAccessibleDocumentView should not be called!, perhaps Meyers, 3rd edition, Item 9:\n");

    return css::uno::Reference<css::accessibility::XAccessible> ();
}

::sd::WindowUpdater* ViewShell::GetWindowUpdater() const
{
    return mpWindowUpdater.get();
}

ViewShellBase& ViewShell::GetViewShellBase() const
{
    return *static_cast<ViewShellBase*>(GetViewShell());
}

ViewShell::ShellType ViewShell::GetShellType() const
{
    return meShellType;
}

DrawDocShell* ViewShell::GetDocSh() const
{
    return GetViewShellBase().GetDocShell();
}

SdDrawDocument* ViewShell::GetDoc() const
{
    return GetViewShellBase().GetDocument();
}

ErrCode ViewShell::DoVerb (long )
{
    return ERRCODE_NONE;
}

void ViewShell::SetCurrentFunction( const rtl::Reference<FuPoor>& xFunction)
{
    if( mxCurrentFunction.is() && (mxOldFunction != mxCurrentFunction) )
        mxCurrentFunction->Dispose();
    rtl::Reference<FuPoor> xTemp( mxCurrentFunction );
    mxCurrentFunction = xFunction;
}

void ViewShell::SetOldFunction(const rtl::Reference<FuPoor>& xFunction)
{
    if( mxOldFunction.is() && (xFunction != mxOldFunction) && (mxCurrentFunction != mxOldFunction) )
        mxOldFunction->Dispose();

    rtl::Reference<FuPoor> xTemp( mxOldFunction );
    mxOldFunction = xFunction;
}

/** this method deactivates the current function. If an old function is
    saved, this will become activated and current function.
*/
void ViewShell::Cancel()
{
    if(mxCurrentFunction.is() && (mxCurrentFunction != mxOldFunction ))
    {
        rtl::Reference<FuPoor> xTemp( mxCurrentFunction );
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

        rtl::Reference<FuPoor> xTemp( mxCurrentFunction );
        mxCurrentFunction.clear();
    }
}

void ViewShell::DisposeFunctions()
{
    if(mxCurrentFunction.is())
    {
        rtl::Reference<FuPoor> xTemp( mxCurrentFunction );
        mxCurrentFunction.clear();
        xTemp->Deactivate();
        xTemp->Dispose();
    }

    if(mxOldFunction.is())
    {
        rtl::Reference<FuPoor> xTemp( mxOldFunction );
        mxOldFunction->Dispose();
        mxOldFunction.clear();
    }
}

bool ViewShell::IsMainViewShell() const
{
    return mpImpl->mbIsMainViewShell;
}

void ViewShell::SetIsMainViewShell (bool bIsMainViewShell)
{
    if (bIsMainViewShell != mpImpl->mbIsMainViewShell)
    {
        mpImpl->mbIsMainViewShell = bIsMainViewShell;
        if (bIsMainViewShell)
            GetDocSh()->Connect (this);
        else
            GetDocSh()->Disconnect (this);
    }
}

void ViewShell::PrePaint()
{
}

void ViewShell::Paint (const Rectangle&, ::sd::Window* )
{
}

void ViewShell::ShowUIControls (bool bVisible)
{
    mpImpl->mbIsShowingUIControls = bVisible;

    if (mbHasRulers)
    {
        if (mpHorizontalRuler.get() != nullptr)
            mpHorizontalRuler->Show( bVisible );

        if (mpVerticalRuler.get() != nullptr)
            mpVerticalRuler->Show( bVisible );
    }

    if (mpVerticalScrollBar.get() != nullptr)
        mpVerticalScrollBar->Show( bVisible );

    if (mpHorizontalScrollBar.get() != nullptr)
        mpHorizontalScrollBar->Show( bVisible );

    if (mpScrollBarBox.get() != nullptr)
        mpScrollBarBox->Show(bVisible);

    if (mpContentWindow.get() != nullptr)
        mpContentWindow->Show( bVisible );
}

bool ViewShell::RelocateToParentWindow (vcl::Window* pParentWindow)
{
    mpParentWindow = pParentWindow;

    mpParentWindow->SetBackground (Wallpaper());

    if (mpContentWindow.get() != nullptr)
        mpContentWindow->SetParent(pParentWindow);

    if (mpHorizontalScrollBar.get() != nullptr)
        mpHorizontalScrollBar->SetParent(mpParentWindow);
    if (mpVerticalScrollBar.get() != nullptr)
        mpVerticalScrollBar->SetParent(mpParentWindow);
    if (mpScrollBarBox.get() != nullptr)
        mpScrollBarBox->SetParent(mpParentWindow);

    return true;
}

void ViewShell::SwitchViewFireFocus(const css::uno::Reference< css::accessibility::XAccessible >& xAcc )
{
    if (xAcc.get())
    {
        ::accessibility::AccessibleDocumentViewBase* pBase = static_cast< ::accessibility::AccessibleDocumentViewBase* >(xAcc.get());
        if (pBase)
            pBase->SwitchViewActivated();
    }
}
void ViewShell::SwitchActiveViewFireFocus()
{
    if (mpContentWindow)
    {
        SwitchViewFireFocus(mpContentWindow->GetAccessible(false));
    }
}
// move these two methods from DrawViewShell.
void ViewShell::fireSwitchCurrentPage(sal_Int32 pageIndex)
{
    GetViewShellBase().GetDrawController().fireSwitchCurrentPage(pageIndex);
}
void ViewShell::NotifyAccUpdate( )
{
    GetViewShellBase().GetDrawController().NotifyAccUpdate();
}

sd::Window* ViewShell::GetContentWindow() const
{
    return mpContentWindow.get();
}

} // end of namespace sd

//===== ViewShellObjectBarFactory =============================================

namespace {

ViewShellObjectBarFactory::ViewShellObjectBarFactory (
    ::sd::ViewShell& rViewShell)
    : mrViewShell (rViewShell)
{
}

ViewShellObjectBarFactory::~ViewShellObjectBarFactory()
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
    vcl::Window*,
    ::sd::FrameView* )
{
    SfxShell* pShell = nullptr;

    ShellCache::iterator aI (maShellCache.find(nId));
    if (aI == maShellCache.end() || aI->second==nullptr)
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
                pShell = new svx::ExtrusionBar(
                    &mrViewShell.GetViewShellBase());
                break;

            case RID_SVX_FONTWORK_BAR:
                pShell = new svx::FontworkBar(
                    &mrViewShell.GetViewShellBase());
                break;

            default:
                pShell = nullptr;
                break;
        }
    }
    else
        pShell = aI->second;

    return pShell;
}

void ViewShellObjectBarFactory::ReleaseShell (SfxShell* pShell)
{
    if (pShell != nullptr)
        delete pShell;
}

} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
