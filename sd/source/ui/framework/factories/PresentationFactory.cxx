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

#include <framework/PresentationFactory.hxx>
#include <framework/ConfigurationController.hxx>

#include <DrawController.hxx>
#include <framework/AbstractView.hxx>
#include <ResourceId.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <slideshow.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;


namespace sd::framework {

namespace {

/** The PresentationView is not an actual view, it is a marker whose
    existence in a configuration indicates that a slideshow is running
    (in another application window).
*/
class PresentationView : public AbstractView
{
public:
    explicit PresentationView (const rtl::Reference<ResourceId>& rxViewId)
        : mxResourceId(rxViewId) {};

    // XView

    virtual rtl::Reference<ResourceId> getResourceId() override
    { return mxResourceId; };

    virtual bool isAnchorOnly() override
    { return false; }

private:
    rtl::Reference<ResourceId> mxResourceId;
};

} // end of anonymous namespace.

//===== PresentationFactory ===================================================

constexpr OUString gsPresentationViewURL = u"private:resource/view/Presentation"_ustr;

PresentationFactory::PresentationFactory (
    const rtl::Reference<::sd::DrawController>& rxController)
    : mxController(rxController),
      mxListener(new Listener)
{
}

PresentationFactory::~PresentationFactory()
{
}

//----- XViewFactory ----------------------------------------------------------

rtl::Reference<AbstractResource> PresentationFactory::createResource (
    const rtl::Reference<ResourceId>& rxViewId)
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }

    if (rxViewId.is())
        if ( ! rxViewId->hasAnchor() && rxViewId->getResourceURL() == gsPresentationViewURL)
            return new PresentationView(rxViewId);

    return rtl::Reference<AbstractResource>();
}

void PresentationFactory::releaseResource (
    const rtl::Reference<AbstractResource>&)
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }

    if (mxController)
    {
        ViewShellBase* pBase = mxController->GetViewShellBase();
        if (pBase != nullptr)
            SlideShow::Stop( *pBase );
    }
}

//===== ConfigurationChangeListener ==========================================

void PresentationFactory::Listener::notifyConfigurationChange (
    const ConfigurationChangeEvent&)
{}

//===== lang::XEventListener ==================================================

void SAL_CALL PresentationFactory::Listener::disposing (
    const lang::EventObject&)
{}

void PresentationFactory::install(const rtl::Reference<::sd::DrawController>& rxController)
{
    try
    {
        rtl::Reference<ConfigurationController> xCC (rxController->getConfigurationController());
        if (xCC.is())
            xCC->addResourceFactory(
                gsPresentationViewURL,
                new PresentationFactory(rxController));
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION("sd");
    }
}

} // end of namespace sd::framework


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
