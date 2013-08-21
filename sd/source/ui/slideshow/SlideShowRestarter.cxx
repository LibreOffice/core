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


#include "SlideShowRestarter.hxx"
#include "framework/ConfigurationController.hxx"
#include "framework/FrameworkHelper.hxx"
#include <comphelper/processfactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>
#include <vcl/svapp.hxx>
#include <boost/bind.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using ::sd::framework::FrameworkHelper;


namespace sd {

SlideShowRestarter::SlideShowRestarter (
    const ::rtl::Reference<SlideShow>& rpSlideShow,
    ViewShellBase* pViewShellBase)
    : mnEventId(0),
      mpSlideShow(rpSlideShow),
      mpViewShellBase(pViewShellBase),
      mnDisplayCount(Application::GetScreenCount()),
      mpDispatcher(pViewShellBase->GetViewFrame()->GetDispatcher()),
      mnCurrentSlideNumber(0)
{
}

SlideShowRestarter::~SlideShowRestarter (void)
{
}

void SlideShowRestarter::Restart (bool bForce)
{
    // Prevent multiple and concurrently restarts.
    if (mnEventId != 0)
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

IMPL_LINK_NOARG(SlideShowRestarter, EndPresentation)
{
    mnEventId = 0;
    if (mpSlideShow.is())
    {
        if (mnDisplayCount != (sal_Int32)Application::GetScreenCount())
        {
            mpSlideShow->end();

            // The following piece of code should not be here because the
            // slide show should be aware of the existence of the presenter
            // console (which is displayed in the FullScreenPane).  But the
            // timing has to be right and I did not find a better place for
            // it.

            // Wait for the full screen pane, which displays the presenter
            // console, to disappear.  Only when it is gone, call
            // InitiatePresenterStart(), in order to begin the asynchronous
            // restart of the slide show.
            if (mpViewShellBase != NULL)
            {
                ::boost::shared_ptr<FrameworkHelper> pHelper(
                    FrameworkHelper::Instance(*mpViewShellBase));
                if (pHelper->GetConfigurationController()->getResource(
                    pHelper->CreateResourceId(FrameworkHelper::msFullScreenPaneURL)).is())
                {
                    ::sd::framework::ConfigurationController::Lock aLock (
                        pHelper->GetConfigurationController());

                    pHelper->RunOnConfigurationEvent(
                        FrameworkHelper::msConfigurationUpdateEndEvent,
                        ::boost::bind(&SlideShowRestarter::StartPresentation, shared_from_this()));
                    pHelper->UpdateConfiguration();
                }
                else
                {
                    StartPresentation();
                }
            }
        }
    }
    return 0;
}

void SlideShowRestarter::StartPresentation (void)
{
    if (mpDispatcher == NULL && mpViewShellBase!=NULL)
        mpDispatcher = mpViewShellBase->GetViewFrame()->GetDispatcher();

    // Start the slide show on the saved current slide.
    if (mpDispatcher != NULL)
    {
        mpDispatcher->Execute(SID_PRESENTATION, SFX_CALLMODE_ASYNCHRON);
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
