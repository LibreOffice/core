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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANEFACTORY_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANEFACTORY_HXX

#include <com/sun/star/frame/XController.hpp>
#include <framework/AbstractPane.hxx>
#include <framework/ResourceFactory.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ref.hxx>
#include <unotools/weakref.hxx>
#include <map>
#include <memory>

namespace sd { class DrawController; }
namespace sd::framework { class ConfigurationController; }

namespace sdext::presenter {

class PresenterController;

/** The PresenterPaneFactory provides a fixed set of panes.

    In order to make the presenter screen more easily extendable in the
    future the set of supported panes could be made extendable on demand.
*/
class PresenterPaneFactory : public sd::framework::ResourceFactory
{
public:
    static constexpr OUString msCurrentSlidePreviewPaneURL
        = u"private:resource/pane/Presenter/Pane1"_ustr;
    static constexpr OUString msNextSlidePreviewPaneURL
        = u"private:resource/pane/Presenter/Pane2"_ustr;
    static constexpr OUString msNotesPaneURL = u"private:resource/pane/Presenter/Pane3"_ustr;
    static constexpr OUString msToolBarPaneURL = u"private:resource/pane/Presenter/Pane4"_ustr;
    static constexpr OUString msSlideSorterPaneURL
        = u"private:resource/pane/Presenter/Pane5"_ustr;

    /** Create a new instance of this class and register it as resource
        factory in the drawing framework of the given controller.
        This registration keeps it alive.  When the drawing framework is
        shut down and releases its reference to the factory then the factory
        is destroyed.
    */
    static rtl::Reference<sd::framework::ResourceFactory> Create (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const rtl::Reference<::sd::DrawController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterPaneFactory() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // ResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxPaneId) override;

    virtual void
        releaseResource (
            const css::uno::Reference<css::drawing::framework::XResource>&
                rxPane) override;

private:
    css::uno::WeakReference<css::uno::XComponentContext> mxComponentContextWeak;
    unotools::WeakReference<sd::framework::ConfigurationController>
        mxConfigurationControllerWeak;
    ::rtl::Reference<PresenterController> mpPresenterController;
    typedef ::std::map<OUString, css::uno::Reference<css::drawing::framework::XResource> >
        ResourceContainer;
    std::unique_ptr<ResourceContainer> mpResourceCache;

    PresenterPaneFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        ::rtl::Reference<PresenterController> xPresenterController);

    void Register (const rtl::Reference<::sd::DrawController>& rxController);

    css::uno::Reference<css::drawing::framework::XResource> CreatePane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId);
    css::uno::Reference<css::drawing::framework::XResource> CreatePane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const rtl::Reference<sd::framework::AbstractPane>& rxParentPane,
        const bool bIsSpritePane);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
