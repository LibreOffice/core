/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    ContentWindow(::Window& rParent, SlideSorter& rSlideSorter);
    ~ContentWindow (void);
    void SetCurrentFunction (const rtl::Reference<FuPoor>& rpFunction);
    virtual void Paint(const Rectangle& rRect);
    virtual void KeyInput (const KeyEvent& rEvent);
    virtual void MouseMove (const MouseEvent& rEvent);
    virtual void MouseButtonUp (const MouseEvent& rEvent);
    virtual void MouseButtonDown (const MouseEvent& rEvent);
    virtual void Command (const CommandEvent& rEvent);
    virtual bool Notify (NotifyEvent& rEvent);

private:
    SlideSorter& mrSlideSorter;
    rtl::Reference<FuPoor> mpCurrentFunction;
};
}






::boost::shared_ptr<SlideSorter> SlideSorter::CreateSlideSorter(
    ViewShell& rViewShell,
    const ::boost::shared_ptr<sd::Window>& rpContentWindow,
    const ::boost::shared_ptr<ScrollBar>& rpHorizontalScrollBar,
    const ::boost::shared_ptr<ScrollBar>& rpVerticalScrollBar,
    const ::boost::shared_ptr<ScrollBarBox>& rpScrollBarBox)
{
    ::boost::shared_ptr<SlideSorter> pSlideSorter(
        new SlideSorter(
            rViewShell,
            rpContentWindow,
            rpHorizontalScrollBar,
            rpVerticalScrollBar,
            rpScrollBarBox));
    pSlideSorter->Init();
    return pSlideSorter;
}




::boost::shared_ptr<SlideSorter> SlideSorter::CreateSlideSorter (
    ViewShellBase& rBase,
    ViewShell* pViewShell,
    ::Window& rParentWindow)
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
    const ::boost::shared_ptr<sd::Window>& rpContentWindow,
    const ::boost::shared_ptr<ScrollBar>& rpHorizontalScrollBar,
    const ::boost::shared_ptr<ScrollBar>& rpVerticalScrollBar,
    const ::boost::shared_ptr<ScrollBarBox>& rpScrollBarBox)
    : mbIsValid(false),
      mpSlideSorterController(),
      mpSlideSorterModel(),
      mpSlideSorterView(),
      mxControllerWeak(),
      mpViewShell(&rViewShell),
      mpViewShellBase(&rViewShell.GetViewShellBase()),
      mpContentWindow(rpContentWindow),
      mbOwnesContentWindow(false),
      mpHorizontalScrollBar(rpHorizontalScrollBar),
      mpVerticalScrollBar(rpVerticalScrollBar),
      mpScrollBarBox(rpScrollBarBox),
      mbLayoutPending(true),
      mpProperties(new controller::Properties()),
      mpTheme(new view::Theme(mpProperties))
{
}




SlideSorter::SlideSorter (
    ViewShellBase& rBase,
    ViewShell* pViewShell,
    ::Window& rParentWindow)
    : mbIsValid(false),
      mpSlideSorterController(),
      mpSlideSorterModel(),
      mpSlideSorterView(),
      mxControllerWeak(),
      mpViewShell(pViewShell),
      mpViewShellBase(&rBase),
      mpContentWindow(new ContentWindow(rParentWindow,*this )),
      mbOwnesContentWindow(true),
      mpHorizontalScrollBar(new ScrollBar(&rParentWindow,WinBits(WB_HSCROLL | WB_DRAG))),
      mpVerticalScrollBar(new ScrollBar(&rParentWindow,WinBits(WB_VSCROLL | WB_DRAG))),
      mpScrollBarBox(new ScrollBarBox(&rParentWindow)),
      mbLayoutPending(true),
      mpProperties(new controller::Properties()),
      mpTheme(new view::Theme(mpProperties))
{
}




void SlideSorter::Init (void)
{
    if (mpViewShellBase != NULL)
        mxControllerWeak = mpViewShellBase->GetController();

    
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

    
    SharedSdWindow pContentWindow (GetContentWindow());
    if (pContentWindow)
    {
        ::Window* pParentWindow = pContentWindow->GetParent();
        if (pParentWindow != NULL)
            pParentWindow->SetBackground(Wallpaper());
        pContentWindow->SetBackground(Wallpaper());
        pContentWindow->SetViewOrigin (Point(0,0));
        
        pContentWindow->SetUseDropScroll (false);
        
        pContentWindow->SetStyle ((pContentWindow->GetStyle() & ~WB_DIALOGCONTROL) | WB_TABSTOP);
        pContentWindow->Hide();

        
        SetupControls(pParentWindow);

        mbIsValid = true;
    }
}




SlideSorter::~SlideSorter (void)
{
    mbIsValid = false;

    ReleaseListeners();

    
    
    mpSlideSorterController->Dispose();
    mpSlideSorterView->Dispose();
    mpSlideSorterModel->Dispose();

    
    mpSlideSorterController.reset();
    mpSlideSorterView.reset();
    mpSlideSorterModel.reset();

    mpHorizontalScrollBar.reset();
    mpVerticalScrollBar.reset();
    mpScrollBarBox.reset();

    if (mbOwnesContentWindow)
    {
        OSL_ASSERT(mpContentWindow.unique());
    }
    else
    {
        
        
        OSL_ASSERT(mpContentWindow.use_count()==2);
    }
    mpContentWindow.reset();
}




bool SlideSorter::IsValid (void) const
{
    return mbIsValid;
}




::boost::shared_ptr<ScrollBar> SlideSorter::GetVerticalScrollBar (void) const
{
    return mpVerticalScrollBar;
}





::boost::shared_ptr<ScrollBar> SlideSorter::GetHorizontalScrollBar (void) const
{
    return mpHorizontalScrollBar;
}




::boost::shared_ptr<ScrollBarBox> SlideSorter::GetScrollBarFiller (void) const
{
    return mpScrollBarBox;
}




model::SlideSorterModel& SlideSorter::GetModel (void) const
{
    OSL_ASSERT(mpSlideSorterModel.get()!=NULL);
    return *mpSlideSorterModel;
}




view::SlideSorterView& SlideSorter::GetView (void) const
{
    OSL_ASSERT(mpSlideSorterView.get()!=NULL);
    return *mpSlideSorterView;
}




controller::SlideSorterController& SlideSorter::GetController (void) const
{
    OSL_ASSERT(mpSlideSorterController.get()!=NULL);
    return *mpSlideSorterController;
}




Reference<frame::XController> SlideSorter::GetXController (void) const
{
    Reference<frame::XController> xController(mxControllerWeak);
    return xController;
}




void SlideSorter::Paint (const Rectangle& rRepaintArea)
{
    GetController().Paint(
        rRepaintArea,
        GetContentWindow().get());
}




::SharedSdWindow SlideSorter::GetContentWindow (void) const
{
    return mpContentWindow;
}




ViewShellBase* SlideSorter::GetViewShellBase (void) const
{
    return mpViewShellBase;
}




ViewShell* SlideSorter::GetViewShell (void) const
{
    return mpViewShell;
}




void SlideSorter::SetupControls (::Window* )
{
    GetVerticalScrollBar()->Show();
    mpSlideSorterController->GetScrollBarManager().LateInitialization();
}




void SlideSorter::SetupListeners (void)
{
    SharedSdWindow pWindow (GetContentWindow());
    if (pWindow)
    {
        ::Window* pParentWindow = pWindow->GetParent();
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




void SlideSorter::ReleaseListeners (void)
{
    mpSlideSorterController->GetScrollBarManager().Disconnect();

    SharedSdWindow pWindow (GetContentWindow());
    if (pWindow)
    {
        pWindow->RemoveEventListener(
            LINK(mpSlideSorterController.get(),
                controller::SlideSorterController,
                WindowEventHandler));

        ::Window* pParentWindow = pWindow->GetParent();
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




void SlideSorter::CreateModelViewController (void)
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

    
    
    mpSlideSorterModel->Init();
    mpSlideSorterController->Init();
    mpSlideSorterView->Init();
}




model::SlideSorterModel* SlideSorter::CreateModel (void)
{
    
    ViewShellBase* pViewShellBase = GetViewShellBase();
    if (pViewShellBase != NULL)
    {
        OSL_ASSERT (pViewShellBase->GetDocument() != NULL);

        return new model::SlideSorterModel(*this);
    }
    else
        return NULL;
}




view::SlideSorterView* SlideSorter::CreateView (void)
{
    return new view::SlideSorterView (*this);
}




controller::SlideSorterController* SlideSorter::CreateController (void)
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
        
        
        view::SlideSorterView::DrawLock aLock (*this);
        GetContentWindow()->EnablePaint (false);

        mpSlideSorterController->Resize (Rectangle(aOrigin, rSize));

        GetContentWindow()->EnablePaint (true);

        mbLayoutPending = false;
    }
}




SvBorder SlideSorter::GetBorder (void)
{
    SvBorder aBorder;

    ::boost::shared_ptr<ScrollBar> pScrollBar = GetVerticalScrollBar();
    if (pScrollBar.get() != NULL && pScrollBar->IsVisible())
        aBorder.Right() = pScrollBar->GetOutputSizePixel().Width();

    pScrollBar = GetHorizontalScrollBar();
    if (pScrollBar.get() != NULL && pScrollBar->IsVisible())
        aBorder.Bottom() = pScrollBar->GetOutputSizePixel().Height();

    return aBorder;
}




bool SlideSorter::RelocateToWindow (::Window* pParentWindow)
{
   
    mpSlideSorterController->GetAnimator()->RemoveAllAnimations();

    ReleaseListeners();

    if (mpViewShell != NULL)
        mpViewShell->ViewShell::RelocateToParentWindow(pParentWindow);

    SetupControls(mpViewShell->GetParentWindow());
    SetupListeners();

    
    
    
    
    
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




::boost::shared_ptr<controller::Properties> SlideSorter::GetProperties (void) const
{
    OSL_ASSERT(mpProperties);
    return mpProperties;
}




::boost::shared_ptr<view::Theme> SlideSorter::GetTheme (void) const
{
    OSL_ASSERT(mpTheme);
    return mpTheme;
}






namespace {

ContentWindow::ContentWindow(
    ::Window& rParent,
    SlideSorter& rSlideSorter)
    : ::sd::Window(&rParent),
    mrSlideSorter(rSlideSorter),
    mpCurrentFunction()
{
    SetDialogControlFlags(GetDialogControlFlags() & ~WINDOW_DLGCTRL_WANTFOCUS);
    SetStyle(GetStyle() | WB_NOPOINTERFOCUS);
}




ContentWindow::~ContentWindow (void)
{
}




void ContentWindow::SetCurrentFunction (const rtl::Reference<FuPoor>& rpFunction)
{
    mpCurrentFunction = rpFunction;
}




void ContentWindow::Paint (const Rectangle& rRect)
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



} 





} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
