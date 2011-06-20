/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

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
using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;

#define C2U(x) OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

namespace sd {

SlideShowRestarter::SlideShowRestarter (
    const ::rtl::Reference<SlideShow>& rpSlideShow,
    ViewShellBase* pViewShellBase)
    : mnEventId(0),
      mpSlideShow(rpSlideShow),
      mpViewShellBase(pViewShellBase),
      mnDisplayCount(GetDisplayCount()),
      mpDispatcher(pViewShellBase->GetViewFrame()->GetDispatcher()),
      mnCurrentSlideNumber(0)
{
}




SlideShowRestarter::~SlideShowRestarter (void)
{
}




void SlideShowRestarter::Restart (void)
{
    // Prevent multiple and concurrently restarts.
    if (mnEventId != 0)
        return;

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




sal_Int32 SlideShowRestarter::GetDisplayCount (void)
{
    const Reference<XComponentContext> xContext (
        ::comphelper::getProcessComponentContext() );
    Reference<XMultiComponentFactory> xFactory (
        xContext->getServiceManager(), UNO_QUERY);
    if ( ! xFactory.is())
        return 0;

    Reference<com::sun::star::container::XIndexAccess> xIndexAccess (
        xFactory->createInstanceWithContext(C2U("com.sun.star.awt.DisplayAccess"),xContext),
        UNO_QUERY);
    if ( ! xIndexAccess.is())
        return 0;

    return xIndexAccess->getCount();
}




IMPL_LINK(SlideShowRestarter, EndPresentation, void*, EMPTYARG)
{
    mnEventId = 0;
    if (mpSlideShow.is())
    {
        if (mnDisplayCount!=GetDisplayCount())
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
            aProperties[0].Name = C2U("FirstPage");
            aProperties[0].Value <<= C2U("page") + OUString::valueOf(mnCurrentSlideNumber+1);
            mpSlideShow->startWithArguments(aProperties);
        }
        mpSelf.reset();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
