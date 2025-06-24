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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANECONTAINER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANECONTAINER_HXX

#include "PresenterPaneBase.hxx"
#include <PresenterHelper.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <ResourceId.hxx>
#include <framework/AbstractView.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>

#include <functional>
#include <memory>
#include <vector>

namespace sdext::presenter {

typedef ::cppu::WeakComponentImplHelper <
    css::lang::XEventListener
> PresenterPaneContainerInterfaceBase;

/** This class could also be called PresenterPaneAndViewContainer because it
    stores not only references to all panes that belong to the presenter
    screen but stores the views displayed in these panes as well.
*/
class PresenterPaneContainer
    : private ::cppu::BaseMutex,
      public PresenterPaneContainerInterfaceBase
{
public:
    explicit PresenterPaneContainer();
    virtual ~PresenterPaneContainer() override;
    PresenterPaneContainer(const PresenterPaneContainer&) = delete;
    PresenterPaneContainer& operator=(const PresenterPaneContainer&) = delete;

    virtual void SAL_CALL disposing() override;

    typedef ::std::function<void (const rtl::Reference<sd::framework::AbstractView>&)>
        ViewInitializationFunction;

    /** Each pane descriptor holds references to one pane and the view
        displayed in this pane as well as the other information that is used
        to manage the pane window like an XWindow reference, the title, and
        the coordinates.

        A initialization function for the view is stored as well.  This
        function is executed as soon as a view is created.
    */
    class PaneDescriptor
    {
    public:
        rtl::Reference<sd::framework::ResourceId> mxPaneId;
        OUString msViewURL;
        ::rtl::Reference<PresenterPaneBase> mxPane;
        rtl::Reference<sd::framework::AbstractView> mxView;
        css::uno::Reference<css::awt::XWindow> mxContentWindow;
        css::uno::Reference<css::awt::XWindow> mxBorderWindow;
        OUString msTitleTemplate;
        OUString msAccessibleNameTemplate;
        OUString msTitle;
        OUString msAccessibleName;
        ViewInitializationFunction maViewInitialization;
        bool mbIsActive;
        bool mbIsOpaque;
        bool mbIsSprite;

        void SetActivationState (const bool bIsActive);
    };
    typedef std::shared_ptr<PaneDescriptor> SharedPaneDescriptor;
    typedef ::std::vector<SharedPaneDescriptor> PaneList;
    PaneList maPanes;

    void PreparePane (
        const rtl::Reference<sd::framework::ResourceId>& rxPaneId,
        const OUString& rsViewURL,
        const OUString& rsTitle,
        const OUString& rsAccessibleTitle,
        const bool bIsOpaque,
        const ViewInitializationFunction& rViewInitialization);

    SharedPaneDescriptor StorePane (
        const rtl::Reference<PresenterPaneBase>& rxPane);

    SharedPaneDescriptor StoreBorderWindow(
        const rtl::Reference<sd::framework::ResourceId>& rxPaneId,
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);

    SharedPaneDescriptor StoreView (
        const rtl::Reference<sd::framework::AbstractView>& rxView);

    SharedPaneDescriptor RemovePane (
        const rtl::Reference<sd::framework::ResourceId>& rxPaneId);

    SharedPaneDescriptor RemoveView (
        const rtl::Reference<sd::framework::AbstractView>& rxView);

    /** Find the pane whose border window is identical to the given border
        window.
    */
    SharedPaneDescriptor FindBorderWindow (
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);

    /** Find the pane whose border window is identical to the given content
        window.
    */
    SharedPaneDescriptor FindContentWindow (
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);

    /** Find the pane whose pane URL is identical to the given URL string.
    */
    SharedPaneDescriptor FindPaneURL (const OUString& rsPaneURL);

    /** Find the pane whose resource id is identical to the given one.
    */
    SharedPaneDescriptor FindPaneId (const rtl::Reference<sd::framework::ResourceId>& rxPaneId);

    SharedPaneDescriptor FindViewURL (const OUString& rsViewURL);

    OUString GetPaneURLForViewURL (const OUString& rsViewURL);

    void ToTop (const SharedPaneDescriptor& rpDescriptor);

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent) override;
};

} // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
