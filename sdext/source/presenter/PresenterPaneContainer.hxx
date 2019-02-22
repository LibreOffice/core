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

#include "PresenterTheme.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Color.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>

#include <functional>
#include <memory>
#include <vector>

namespace sdext { namespace presenter {

class PresenterPaneBase;
class PresenterSprite;

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
    explicit PresenterPaneContainer (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterPaneContainer() override;
    PresenterPaneContainer(const PresenterPaneContainer&) = delete;
    PresenterPaneContainer& operator=(const PresenterPaneContainer&) = delete;

    virtual void SAL_CALL disposing() override;

    typedef ::std::function<void (const css::uno::Reference<css::drawing::framework::XView>&)>
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
        typedef ::std::function<std::shared_ptr<PresenterSprite> ()> SpriteProvider;
        css::uno::Reference<css::drawing::framework::XResourceId> mxPaneId;
        OUString msViewURL;
        ::rtl::Reference<PresenterPaneBase> mxPane;
        css::uno::Reference<css::drawing::framework::XView> mxView;
        css::uno::Reference<css::awt::XWindow> mxContentWindow;
        css::uno::Reference<css::awt::XWindow> mxBorderWindow;
        OUString msTitleTemplate;
        OUString msAccessibleTitleTemplate;
        OUString msTitle;
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
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const OUString& rsViewURL,
        const OUString& rsTitle,
        const OUString& rsAccessibleTitle,
        const bool bIsOpaque,
        const ViewInitializationFunction& rViewIntialization);

    SharedPaneDescriptor StorePane (
        const rtl::Reference<PresenterPaneBase>& rxPane);

    SharedPaneDescriptor StoreBorderWindow(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);

    SharedPaneDescriptor StoreView (
        const css::uno::Reference<css::drawing::framework::XView>& rxView);

    SharedPaneDescriptor RemovePane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId);

    SharedPaneDescriptor RemoveView (
        const css::uno::Reference<css::drawing::framework::XView>& rxView);

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
    SharedPaneDescriptor FindPaneId (const css::uno::Reference<
        css::drawing::framework::XResourceId>& rxPaneId);

    SharedPaneDescriptor FindViewURL (const OUString& rsViewURL);

    OUString GetPaneURLForViewURL (const OUString& rsViewURL);

    void ToTop (const SharedPaneDescriptor& rpDescriptor);

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent) override;

private:
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
