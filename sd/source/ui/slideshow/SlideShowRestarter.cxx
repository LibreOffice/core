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

#include <DrawController.hxx>
#include <ViewShellBase.hxx>
#include <slideshow.hxx>
#include "SlideShowRestarter.hxx"
#include <framework/ConfigurationController.hxx>
#include <framework/FrameworkHelper.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include <svx/svxids.hrc>
#include <vcl/svapp.hxx>

#include <functional>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using ::sd::framework::FrameworkHelper;

namespace sd {

SlideShowRestarter::SlideShowRestarter (
    const ::rtl::Reference<SlideShow>& rpSlideShow,
    ViewShellBase* pViewShellBase)
    : mnEventId(nullptr),
      mpSlideShow(rpSlideShow),
      mpViewShellBase(pViewShellBase),
      mnDisplayCount(Application::GetScreenCount()),
      mpDispatcher(pViewShellBase->GetViewFrame()->GetDispatcher()),
      mnCurrentSlideNumber(0)
{
}

SlideShowRestarter::~SlideShowRestarter()
{
}

void SlideShowRestarter::Restart (bool bForce)
{
    // Prevent multiple and concurrently restarts.
    if (mnEventId != nullptr)
        return;

    if (bForce)
        mnDisplayCount = 0;

    // Remember the current slide in order to restore it after the slide
    // show has been restarted.
    if (mpSlideShow.is())
        mnCurrentSlideNumber = mpSlideShow->getCurrentPageNumber();

    // Remember a shared pointer to this object to prevent its destruction
    // before the whole restarting process has finished.
    mpSelf = shared_from_this();

    // We do not know in what situation this method was called.  So, in
    // order to be able to cleanly stop the presentation, we do that
    // asynchronously.
    mnEventId = Application::PostUserEvent(
        LINK(this, SlideShowRestarter, EndPresentation));
}

IMPL_LINK_NOARG(SlideShowRestarter, EndPresentation, void*, void)
{
    mnEventId = nullptr;
    if (!mpSlideShow.is())
        return;

    if (mnDisplayCount == static_cast<sal_Int32>(Application::GetScreenCount()))
        return;

    bool bIsExitAfterPresenting = mpSlideShow->IsExitAfterPresenting();
    mpSlideShow->SetExitAfterPresenting(false);
    mpSlideShow->end();
    mpSlideShow->SetExitAfterPresenting(bIsExitAfterPresenting);

    // The following piece of code should not be here because the
    // slide show should be aware of the existence of the presenter
    // console (which is displayed in the FullScreenPane).  But the
    // timing has to be right and I did not find a better place for
    // it.

    // Wait for the full screen pane, which displays the presenter
    // console, to disappear.  Only when it is gone, call
    // InitiatePresenterStart(), in order to begin the asynchronous
    // restart of the slide show.
    if (mpViewShellBase == nullptr)
        return;

    ::std::shared_ptr<FrameworkHelper> pHelper(
        FrameworkHelper::Instance(*mpViewShellBase));
    if (pHelper->GetConfigurationController()->getResource(
        FrameworkHelper::CreateResourceId(FrameworkHelper::msFullScreenPaneURL)).is())
    {
        ::sd::framework::ConfigurationController::Lock aLock (
            pHelper->GetConfigurationController());

        pHelper->RunOnConfigurationEvent(
            FrameworkHelper::msConfigurationUpdateEndEvent,
            ::std::bind(&SlideShowRestarter::StartPresentation, shared_from_this()));
        pHelper->UpdateConfiguration();
    }
    else
    {
        StartPresentation();
    }
}

void SlideShowRestarter::StartPresentation()
{
    //rhbz#1091117 crash because we're exiting the application, and this is
    //being called during the configuration update event on exit. At this point
    //newly created objects won't get disposed called on them, because the
    //disposer is doing its last execution of that now.
    if (mpViewShellBase && mpViewShellBase->GetDrawController().IsDisposing())
        return;

    if (mpDispatcher == nullptr && mpViewShellBase!=nullptr)
        mpDispatcher = mpViewShellBase->GetViewFrame()->GetDispatcher();

    // Start the slide show on the saved current slide.
    if (mpDispatcher != nullptr)
    {
        mpDispatcher->Execute(SID_PRESENTATION, SfxCallMode::ASYNCHRON);
        if (mpSlideShow.is())
        {
            Sequence<css::beans::PropertyValue> aProperties (1);
            aProperties[0].Name = "FirstPage";
            aProperties[0].Value <<= "page" + OUString::number(mnCurrentSlideNumber+1);
            mpSlideShow->startWithArguments(aProperties);
        }
        mpSelf.reset();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
