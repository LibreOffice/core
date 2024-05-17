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

#include <DrawController.hxx>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <comphelper/servicehelper.hxx>
#include <comphelper/compbase.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <slideshow.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;


namespace sd::framework {

namespace {

typedef comphelper::WeakComponentImplHelper<XView> PresentationViewInterfaceBase;

/** The PresentationView is not an actual view, it is a marker whose
    existence in a configuration indicates that a slideshow is running
    (in another application window).
*/
class PresentationView
    : public PresentationViewInterfaceBase
{
public:
    explicit PresentationView (const Reference<XResourceId>& rxViewId)
        : mxResourceId(rxViewId) {};

    // XView

    virtual Reference<XResourceId> SAL_CALL getResourceId() override
    { return mxResourceId; };

    virtual sal_Bool SAL_CALL isAnchorOnly() override
    { return false; }

private:
    Reference<XResourceId> mxResourceId;
};

} // end of anonymous namespace.

//===== PresentationFactory ===================================================

constexpr OUString gsPresentationViewURL = u"private:resource/view/Presentation"_ustr;

PresentationFactory::PresentationFactory (
    const rtl::Reference<::sd::DrawController>& rxController)
    : mxController(rxController)
{
}

PresentationFactory::~PresentationFactory()
{
}

//----- XViewFactory ----------------------------------------------------------

Reference<XResource> SAL_CALL PresentationFactory::createResource (
    const Reference<XResourceId>& rxViewId)
{
    ThrowIfDisposed();

    if (rxViewId.is())
        if ( ! rxViewId->hasAnchor() && rxViewId->getResourceURL() == gsPresentationViewURL)
            return new PresentationView(rxViewId);

    return Reference<XResource>();
}

void SAL_CALL PresentationFactory::releaseResource (
    const Reference<XResource>&)
{
    ThrowIfDisposed();

    if (mxController)
    {
        ViewShellBase* pBase = mxController->GetViewShellBase();
        if (pBase != nullptr)
            SlideShow::Stop( *pBase );
    }
}

//===== XConfigurationChangeListener ==========================================

void SAL_CALL PresentationFactory::notifyConfigurationChange (
    const ConfigurationChangeEvent&)
{}

//===== lang::XEventListener ==================================================

void SAL_CALL PresentationFactory::disposing (
    const lang::EventObject&)
{}

void PresentationFactory::ThrowIfDisposed() const
{
    if (m_bDisposed)
    {
        throw lang::DisposedException (u"PresentationFactory object has already been disposed"_ustr,
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

void PresentationFactory::install(const rtl::Reference<::sd::DrawController>& rxController)
{
    try
    {
        Reference<XConfigurationController> xCC (rxController->getConfigurationController());
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
