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

#pragma once

#include <framework/ResourceFactory.hxx>
#include <rtl/ref.hxx>
#include <vcl/vclptr.hxx>
#include <memory>

namespace sd {
class DrawController;
class ViewShell;
class ViewShellBase;
class FrameView;
}
class SfxViewFrame;
namespace vcl { class Window; }

namespace sd::framework {
class ConfigurationController;
class AbstractPane;

/** Factory for the frequently used standard views of the drawing framework:
        private:resource/view/
        private:resource/view/ImpressView
        private:resource/view/GraphicView
        private:resource/view/OutlineView
        private:resource/view/NotesView
        private:resource/view/HandoutView
        private:resource/view/SlideSorter
        private:resource/view/PresentationView
        private:resource/view/TaskPane
    For some views in some panes this class also acts as a cache.
*/
class BasicViewFactory final
    : public sd::framework::ResourceFactory
{
public:
    BasicViewFactory(const rtl::Reference<::sd::DrawController>& rxController);
    virtual ~BasicViewFactory() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // XViewFactory

    virtual rtl::Reference<sd::framework::AbstractResource>
        createResource (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId) override;

    virtual void releaseResource (
        const rtl::Reference<sd::framework::AbstractResource>& xView) override;

private:
    rtl::Reference<ConfigurationController> mxConfigurationController;
    class ViewDescriptor;
    using ViewShellContainer = std::vector<std::shared_ptr<ViewDescriptor>>;
    ViewShellContainer maViewShellContainer;
    ViewShellBase* mpBase;
    FrameView* mpFrameView;

    using ViewCache = std::vector<std::shared_ptr<ViewDescriptor>>;
    ScopedVclPtr<vcl::Window> mpWindow;
    std::shared_ptr<ViewCache> mpViewCache;

    rtl::Reference<framework::AbstractPane> mxLocalPane;

    std::shared_ptr<ViewDescriptor> CreateView (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        vcl::Window& rWindow,
        const rtl::Reference<framework::AbstractPane>& rxPane,
        FrameView* pFrameView,
        const bool bIsCenterView);

    std::shared_ptr<ViewShell> CreateViewShell (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        vcl::Window& rWindow,
        FrameView* pFrameView);

    void ActivateCenterView (
        const std::shared_ptr<ViewDescriptor>& rpDescriptor);

    void ReleaseView (
        const std::shared_ptr<ViewDescriptor>& rpDescriptor,
        bool bDoNotCache);

    bool IsCacheable (
        const std::shared_ptr<ViewDescriptor>& rpDescriptor);

    std::shared_ptr<ViewDescriptor> GetViewFromCache (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const rtl::Reference<AbstractPane>& rxPane);
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
