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

#include "SlideSorter.hxx"

#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsAnimator.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsTheme.hxx"
#include "model/SlideSorterModel.hxx"

#include "glob.hrc"
#include "DrawController.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "Window.hxx"

#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <sfx2/dispatch.hxx>
#include "sdresid.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace sd { namespace slidesorter {

namespace {
class ContentWindow : public ::sd::Window
{
public:
    ContentWindow(vcl::Window& rParent, SlideSorter& rSlideSorter);
    virtual ~ContentWindow();
    void SetCurrentFunction (const rtl::Reference<FuPoor>& rpFunction);
    virtual void Paint(vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect) SAL_OVERRIDE;
    virtual void KeyInput (const KeyEvent& rEvent) SAL_OVERRIDE;
    virtual void MouseMove (const MouseEvent& rEvent) SAL_OVERRIDE;
    virtual void MouseButtonUp (const MouseEvent& rEvent) SAL_OVERRIDE;
    virtual void MouseButtonDown (const MouseEvent& rEvent) SAL_OVERRIDE;
    virtual void Command (const CommandEvent& rEvent) SAL_OVERRIDE;
    virtual bool Notify (NotifyEvent& rEvent) SAL_OVERRIDE;

private:
    SlideSorter& mrSlideSorter;
    rtl::Reference<FuPoor> mpCurrentFunction;
};
}

//===== SlideSorter ===========================================================

::boost::shared_ptr<SlideSorter> SlideSorter::CreateSlideSorter(
    ViewShell& rViewShell,
    sd::Window* pContentWindow,
    ScrollBar* pHorizontalScrollBar,
    ScrollBar* pVerticalScrollBar,
    ScrollBarBox* pScrollBarBox)
{
    ::boost::shared_ptr<SlideSorter> pSlideSorter(
        new SlideSorter(
            rViewShell,
            pContentWindow,
            pHorizontalScrollBar,
            pVerticalScrollBar,
            pScrollBarBox));
    pSlideSorter->Init();
    return pSlideSorter;
}

::boost::shared_ptr<SlideSorter> SlideSorter::CreateSlideSorter (
    ViewShellBase& rBase,
    ViewShell* pViewShell,
    vcl::Window& rParentWindow)
{
    ::boost::shared_ptr<SlideSorter> pSlideSorter(
        new SlideSorter(
            rBase,
            pViewShell,
            rParentWindow));
    pSlideSorter->Init();
    return pSlideSorter;
}

SlideSorter::SlideSorter (
    ViewShell& rViewShell,
    sd::Window* pContentWindow,
    ScrollBar* pHorizontalScrollBar,
    ScrollBar* pVerticalScrollBar,
    ScrollBarBox* pScrollBarBox)
    : mbIsValid(false),
      mpSlideSorterController(),
      mpSlideSorterModel(),
      mpSlideSorterView(),
      mxControllerWeak(),
      mpViewShell(&rViewShell),
      mpViewShellBase(&rViewShell.GetViewShellBase()),
      mpContentWindow(pContentWindow),
      mbOwnesContentWindow(false),
      mpHorizontalScrollBar(pHorizontalScrollBar),
      mpVerticalScrollBar(pVerticalScrollBar),
      mpScrollBarBox(pScrollBarBox),
      mbLayoutPending(true),
      mpProperties(new controller::Properties()),
      mpTheme(new view::Theme(mpProperties))
{
}

SlideSorter::SlideSorter (
    ViewShellBase& rBase,
    ViewShell* pViewShell,
    vcl::Window& rParentWindow)
    : mbIsValid(false),
      mpSlideSorterController(),
      mpSlideSorterModel(),
      mpSlideSorterView(),
      mxControllerWeak(),
      mpViewShell(pViewShell),
      mpViewShellBase(&rBase),
      mpContentWindow(VclPtr<ContentWindow>::Create(rParentWindow,*this )),
      mbOwnesContentWindow(true),
      mpHorizontalScrollBar(VclPtr<ScrollBar>::Create(&rParentWindow,WinBits(WB_HSCROLL | WB_DRAG))),
      mpVerticalScrollBar(VclPtr<ScrollBar>::Create(&rParentWindow,WinBits(WB_VSCROLL | WB_DRAG))),
      mpScrollBarBox(VclPtr<ScrollBarBox>::Create(&rParentWindow)),
      mbLayoutPending(true),
      mpProperties(new controller::Properties()),
      mpTheme(new view::Theme(mpProperties))
{
}

void SlideSorter::Init()
{
    if (mpViewShellBase != NULL)
        mxControllerWeak = mpViewShellBase->GetController();

    // Reinitialize colors in Properties with window specific values.
    if (mpContentWindow)
    {
        mpProperties->SetBackgroundColor(
            mpContentWindow->GetSettings().GetStyleSettings().GetWindowColor());
        mpProperties->SetTextColor(
            mpContentWindow->GetSettings().GetStyleSettings().GetWindowTextColor());
        mpProperties->SetSelectionColor(
            mpContentWindow->GetSettings().GetStyleSettings().GetMenuHighlightColor());
        mpProperties->SetHighlightColor(
            mpContentWindow->GetSettings().GetStyleSettings().GetMenuHighlightColor());
    }

    CreateModelViewController ();

    SetupListeners ();

    // Initialize the window.
    sd::Window *pContentWindow (GetContentWindow());
    if (pContentWindow)
    {
        vcl::Window* pParentWindow = pContentWindow->GetParent();
        if (pParentWindow != NULL)
            pParentWindow->SetBackground(Wallpaper());
        pContentWindow->SetBackground(Wallpaper());
        pContentWindow->SetViewOrigin (Point(0,0));
        // We do our own scrolling while dragging a page selection.
        pContentWindow->SetUseDropScroll (false);
        // Change the winbits so that the active window accepts the focus.
        pContentWindow->SetStyle ((pContentWindow->GetStyle() & ~WB_DIALOGCONTROL) | WB_TABSTOP);
        pContentWindow->Hide();

        // Set view pointer of base class.
        SetupControls(pParentWindow);

        mbIsValid = true;
    }
}

SlideSorter::~SlideSorter()
{
    mbIsValid = false;

    ReleaseListeners();

    // Dispose model, view and controller to avoid calls between them when
    // they are being destructed and one or two of them are already gone.
    mpSlideSorterController->Dispose();
    mpSlideSorterView->Dispose();
    mpSlideSorterModel->Dispose();

    // Reset the auto pointers explicitly to control the order of destruction.
    mpSlideSorterController.reset();
    mpSlideSorterView.reset();
    mpSlideSorterModel.reset();

    mpHorizontalScrollBar.reset();
    mpVerticalScrollBar.reset();
    mpScrollBarBox.reset();
}

model::SlideSorterModel& SlideSorter::GetModel() const
{
    OSL_ASSERT(mpSlideSorterModel.get()!=NULL);
    return *mpSlideSorterModel;
}

view::SlideSorterView& SlideSorter::GetView() const
{
    OSL_ASSERT(mpSlideSorterView.get()!=NULL);
    return *mpSlideSorterView;
}

controller::SlideSorterController& SlideSorter::GetController() const
{
    OSL_ASSERT(mpSlideSorterController.get()!=NULL);
    return *mpSlideSorterController;
}

Reference<frame::XController> SlideSorter::GetXController() const
{
    Reference<frame::XController> xController(mxControllerWeak);
    return xController;
}

void SlideSorter::Paint (const Rectangle& rRepaintArea)
{
    GetController().Paint(
        rRepaintArea,
        GetContentWindow());
}

void SlideSorter::SetupControls (vcl::Window* )
{
    GetVerticalScrollBar()->Show();
}

void SlideSorter::SetupListeners()
{
    sd::Window *pWindow (GetContentWindow());
    if (pWindow)
    {
        vcl::Window* pParentWindow = pWindow->GetParent();
        if (pParentWindow != NULL)
            pParentWindow->AddEventListener(
                LINK(
                    mpSlideSorterController.get(),
                    controller::SlideSorterController,
                    WindowEventHandler));
        pWindow->AddEventListener(
            LINK(
                mpSlideSorterController.get(),
                controller::SlideSorterController,
                WindowEventHandler));
    }
    Application::AddEventListener(
        LINK(
            mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));

    mpSlideSorterController->GetScrollBarManager().Connect();
}

void SlideSorter::ReleaseListeners()
{
    mpSlideSorterController->GetScrollBarManager().Disconnect();

    sd::Window *pWindow (GetContentWindow());
    if (pWindow)
    {
        pWindow->RemoveEventListener(
            LINK(mpSlideSorterController.get(),
                controller::SlideSorterController,
                WindowEventHandler));

        vcl::Window* pParentWindow = pWindow->GetParent();
        if (pParentWindow != NULL)
            pParentWindow->RemoveEventListener(
                LINK(mpSlideSorterController.get(),
                    controller::SlideSorterController,
                    WindowEventHandler));
    }
    Application::RemoveEventListener(
        LINK(mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));
}

void SlideSorter::CreateModelViewController()
{
    mpSlideSorterModel.reset(CreateModel());
    DBG_ASSERT (mpSlideSorterModel.get()!=NULL,
        "Can not create model for slide browser");

    mpSlideSorterView.reset(CreateView());
    DBG_ASSERT (mpSlideSorterView.get()!=NULL,
        "Can not create view for slide browser");

    mpSlideSorterController.reset(CreateController());
    DBG_ASSERT (mpSlideSorterController.get()!=NULL,
        "Can not create controller for slide browser");

    // Now that model, view, and controller are constructed, do the
    // initialization that relies on all three being in place.
    mpSlideSorterController->Init();
    mpSlideSorterView->Init();
}

model::SlideSorterModel* SlideSorter::CreateModel()
{
    // Get pointers to the document.
    ViewShellBase* pViewShellBase = GetViewShellBase();
    if (pViewShellBase != NULL)
    {
        OSL_ASSERT (pViewShellBase->GetDocument() != NULL);

        return new model::SlideSorterModel(*this);
    }
    else
        return NULL;
}

view::SlideSorterView* SlideSorter::CreateView()
{
    return new view::SlideSorterView (*this);
}

controller::SlideSorterController* SlideSorter::CreateController()
{
    controller::SlideSorterController* pController
        = new controller::SlideSorterController (*this);
    return pController;
}

void SlideSorter::ArrangeGUIElements (
    const Point& rOffset,
    const Size& rSize)
{
    Point aOrigin (rOffset);

    if (rSize.Width()>0
        && rSize.Height()>0
        && GetContentWindow()
        && GetContentWindow()->IsVisible())
    {
        // Prevent untimely redraws while the view is not yet correctly
        // resized.
        view::SlideSorterView::DrawLock aLock (*this);
        GetContentWindow()->EnablePaint (false);

        mpSlideSorterController->Resize (Rectangle(aOrigin, rSize));

        GetContentWindow()->EnablePaint (true);

        mbLayoutPending = false;
    }
}

bool SlideSorter::RelocateToWindow (vcl::Window* pParentWindow)
{
   // Stop all animations for they have been started for the old window.
    mpSlideSorterController->GetAnimator()->RemoveAllAnimations();

    ReleaseListeners();

    vcl::Window *pNewWindow = NULL;
    if (mpViewShell)
    {
        mpViewShell->ViewShell::RelocateToParentWindow(pParentWindow);
        pNewWindow = mpViewShell->GetParentWindow();
    }
    else
        pNewWindow = NULL;

    SetupControls(pNewWindow);
    SetupListeners();

    // For accessibility we have to shortly hide the content window.  This
    // triggers the construction of a new accessibility object for the new
    // view shell.  (One is created earlier while the construtor of the base
    // class is executed.  But because at that time the correct
    // accessibility object can not be constructed we do that now.)
    if (mpContentWindow.get() !=NULL)
    {
        mpContentWindow->Hide();
        mpContentWindow->Show();
    }

    return true;
}

void SlideSorter::SetCurrentFunction (const rtl::Reference<FuPoor>& rpFunction)
{
    if (GetViewShell() != NULL)
    {
        GetViewShell()->SetCurrentFunction(rpFunction);
        GetViewShell()->SetOldFunction(rpFunction);
    }
    else
    {
        ContentWindow* pWindow = dynamic_cast<ContentWindow*>(GetContentWindow().get());
        if (pWindow != NULL)
            pWindow->SetCurrentFunction(rpFunction);
    }
}

::boost::shared_ptr<controller::Properties> SlideSorter::GetProperties() const
{
    OSL_ASSERT(mpProperties);
    return mpProperties;
}

::boost::shared_ptr<view::Theme> SlideSorter::GetTheme() const
{
    OSL_ASSERT(mpTheme);
    return mpTheme;
}

//===== ContentWindow =========================================================

namespace {

ContentWindow::ContentWindow(
    vcl::Window& rParent,
    SlideSorter& rSlideSorter)
    : ::sd::Window(&rParent),
    mrSlideSorter(rSlideSorter),
    mpCurrentFunction()
{
    SetDialogControlFlags(GetDialogControlFlags() & ~DialogControlFlags::WantFocus);
    SetStyle(GetStyle() | WB_NOPOINTERFOCUS);
}

ContentWindow::~ContentWindow()
{
}

void ContentWindow::SetCurrentFunction (const rtl::Reference<FuPoor>& rpFunction)
{
    mpCurrentFunction = rpFunction;
}

void ContentWindow::Paint (vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect)
{
    mrSlideSorter.Paint(rRect);
}

void ContentWindow::KeyInput (const KeyEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->KeyInput(rEvent);
}

void ContentWindow::MouseMove (const MouseEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->MouseMove(rEvent);
}

void ContentWindow::MouseButtonUp(const MouseEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->MouseButtonUp(rEvent);
}

void ContentWindow::MouseButtonDown(const MouseEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->MouseButtonDown(rEvent);
}

void ContentWindow::Command(const CommandEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->Command(rEvent);
}

bool ContentWindow::Notify (NotifyEvent&)
{
    return false;
}

} // end of anonymous namespace

} } // end of namespace ::sd::slidesorter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
