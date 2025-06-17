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

#include <com/sun/star/drawing/framework/TabBarButton.hpp>
#include <com/sun/star/drawing/framework/XToolBar.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <comphelper/compbase.hxx>
#include <vcl/InterimItemWindow.hxx>

#include <vector>

namespace com::sun::star::drawing::framework { class XResourceId; }
namespace vcl { class Window; }

namespace sd {
    class DrawController;
    class ViewShellBase;
    class ViewTabBar;
}
namespace sd::framework { class ConfigurationController; }

namespace sd {

class TabBarControl final : public InterimItemWindow
{
public:
    TabBarControl(vcl::Window* pParentWindow, ::rtl::Reference<ViewTabBar> pViewTabBar);
    virtual void dispose() override;
    virtual ~TabBarControl() override;
    weld::Notebook& GetNotebook() { return *mxTabControl; }
    int GetAllocatedWidth() const { return mnAllocatedWidth; }
private:
    std::unique_ptr<weld::Notebook> mxTabControl;
    ::rtl::Reference<ViewTabBar> mpViewTabBar;
    int mnAllocatedWidth;

    DECL_LINK(ActivatePageHdl, const OUString&, void);
    DECL_LINK(NotebookSizeAllocHdl, const Size&, void);
};

typedef comphelper::WeakComponentImplHelper <
    css::drawing::framework::XToolBar,
    css::drawing::framework::XConfigurationChangeListener
    > ViewTabBarInterfaceBase;

/** Tab control for switching between views in the center pane.

    UI control for the selection of views in a pane.
    <p>Every tab of a tab bar has, besides its localized title and help
    text, the URL of a view.  A possible alternative would be to use a
    command URL instead of the view URL.</p>
    <p>In the current Impress implementation a tab bar is only used for the
    center pane to switch between views in the center pane.  Tab bars can
    make sense for other panes as well, i.e. for showing either the slide
    sorter or the outline view in the left pane.</p>
    <p>Tab bar buttons are identified by their resource id.  Note that
    because the resource anchors are all the same (the tab bar), it is the
    resource URL that really identifies a button. There can not be two
    buttons with the same resource id.</p>
    </p>
    <p>A better place for this interface (in an extended version) would be
    <code>com::sun::star::awt</code></p>
    @see TabBarButton
*/
class ViewTabBar final
    : public ViewTabBarInterfaceBase
{
public:
    ViewTabBar (
        const css::uno::Reference< css::drawing::framework::XResourceId>& rxViewTabBarId,
        const rtl::Reference< ::sd::DrawController>& rxController);
    virtual ~ViewTabBar() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    const VclPtr<TabBarControl>& GetTabControl() const { return mpTabControl; }

    bool ActivatePage(size_t nIndex);

    //----- drawing::framework::XConfigurationChangeListener ------------------

    virtual void SAL_CALL
        notifyConfigurationChange (
            const css::drawing::framework::ConfigurationChangeEvent& rEvent) override;

    //----- XEventListener ----------------------------------------------------

    virtual void SAL_CALL disposing(
        const css::lang::EventObject& rEvent) override;

    /** Add a tab bar button to the right of another one.
        @param aButton
            The new tab bar button that is to be inserted.  If a button with
            the same resource id is already present than that is removed before the
            new button is inserted.
        @param aAnchor
            The new button is inserted to the right of this button.  When
            its ResourceId is empty then the new button is inserted at the left
            most position.
    */
    void
        addTabBarButtonAfter (
            const css::drawing::framework::TabBarButton& rButton,
            const css::drawing::framework::TabBarButton& rAnchor);

    /** Add a tab bar button at the right most position.
        @param aButton
            The new tab bar button that is to be inserted.
    */
    void
        appendTabBarButton (
            const css::drawing::framework::TabBarButton& rButton);

    /** Remove a tab bar button.
        @param aButton
            The tab bar button to remove.  When there is no button with the
            specified resource id then this call is silently ignored.
    */
    void
        removeTabBarButton (
            const css::drawing::framework::TabBarButton& rButton);

    /** Test whether the specified button exists in the tab bar.
        @param aButton
            The tab bar button whose existence is tested.
        @return
            Returns `TRUE` when the button exists.
    */
    bool
        hasTabBarButton (
            const css::drawing::framework::TabBarButton& rButton);

    /** Return a sequence of all the tab bar buttons.
        <p>Their order reflects the visible order in the tab bar.</p>
        <p>This method can be used when
        addTabBarButtonAfter() does not provide enough
        control as to where to insert a new button.</p>
    */
    css::uno::Sequence<css::drawing::framework::TabBarButton>
        getTabBarButtons();

    //----- XResource ---------------------------------------------------------

    virtual css::uno::Reference<
        css::drawing::framework::XResourceId> SAL_CALL getResourceId() override;

    virtual sal_Bool SAL_CALL isAnchorOnly() override;

    /** The returned value is calculated as the difference between the
        total height of the control and the height of its first tab page.
        This can be considered a hack.
        This procedure works only when the control is visible.  Calling this
        method when the control is not visible results in returning a
        default value.
        To be on the safe side wait for this control to become visible and
        the call this method again.
    */
    int GetHeight() const;

    void UpdateActiveButton();

    void AddTabBarButton (
        const css::drawing::framework::TabBarButton& rButton,
        const css::drawing::framework::TabBarButton& rAnchor);
    void AddTabBarButton (
        const css::drawing::framework::TabBarButton& rButton);
    void RemoveTabBarButton (
        const css::drawing::framework::TabBarButton& rButton);
    bool HasTabBarButton (
        const css::drawing::framework::TabBarButton& rButton);
    css::uno::Sequence<css::drawing::framework::TabBarButton>
        GetTabBarButtons();

private:
    VclPtr<TabBarControl> mpTabControl;
    rtl::Reference<::sd::DrawController> mxController;
    rtl::Reference<::sd::framework::ConfigurationController> mxConfigurationController;
    typedef ::std::vector<css::drawing::framework::TabBarButton> TabBarButtonList;
    TabBarButtonList maTabBarButtons;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewTabBarId;
    ViewShellBase* mpViewShellBase;
    int mnNoteBookWidthPadding;

    void AddTabBarButton (
        const css::drawing::framework::TabBarButton& rButton,
        sal_Int32 nPosition);
    void UpdateTabBarButtons();

    /** This method is called from the constructor to get the window for an
        anchor ResourceId and pass it to our base class.  It has to be
        static because it must not access any of the, not yet initialized
        members.
    */
    static vcl::Window* GetAnchorWindow(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewTabBarId,
        const rtl::Reference<::sd::DrawController>& rxController);
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
