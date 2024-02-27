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

#include <sfx2/sidebar/ResourceManager.hxx>

#include <vcl/InterimItemWindow.hxx>

#include <functional>

namespace com::sun::star::frame { class XFrame; }

namespace svt { class AcceleratorExecute; }

namespace weld { class Toolbar; }

namespace sfx2::sidebar {

class FocusManager;
class SidebarController;

/** The tab bar is the container for the individual tabs.
*/
class TabBar final : public InterimItemWindow
{
    friend class TabBarUIObject;
public:
    /** DeckMenuData has entries for display name, and a flag:
         - isCurrentDeck for the deck selection data
         - isEnabled     for the show/hide menu
    */
    class DeckMenuData
    {
    public:
        OUString msDisplayName;
        bool mbIsCurrentDeck;
        bool mbIsActive;
        bool mbIsEnabled;
    };
    typedef ::std::function<void (
            weld::Menu& rMainMenu, weld::Menu& rSubMenu,
            const ::std::vector<DeckMenuData>& rMenuData)> PopupMenuProvider;
    TabBar (
        vcl::Window* pParentWindow,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ::std::function<void (const OUString& rsDeckId)> aDeckActivationFunctor,
        PopupMenuProvider aPopupMenuProvider,
        SidebarController* rParentSidebarController);

    weld::Container* GetContainer() { return m_xContainer.get(); }

    virtual ~TabBar() override;
    virtual void dispose() override;

    virtual void DataChanged (const DataChangedEvent& rDataChangedEvent) override;
    virtual bool EventNotify (NotifyEvent& rEvent) override;

    static sal_Int32 GetDefaultWidth();

    void SetDecks (
        const ResourceManager::DeckContextDescriptorContainer& rDecks);
    void HighlightDeck (std::u16string_view rsDeckId);
    void RemoveDeckHighlight ();
    OUString const & GetDeckIdForIndex (const sal_Int32 nIndex) const;
    void ToggleHideFlag (const sal_Int32 nIndex);
    void RestoreHideFlags();

    void UpdateFocusManager (FocusManager& rFocusManager);

    /// Enables/Disables the menu button. Used by LoKit.
    void EnableMenuButton(const bool bEnable);

    virtual FactoryFunction GetUITestFactory() const override;
private:
    css::uno::Reference<css::frame::XFrame> mxFrame;

    // This unusual auxiliary builder is because without a toplevel GtkWindow
    // gtk will warn on loading a .ui with an accelerator defined, so use a
    // temporary toplevel to suppress that and move the contents after load
    std::unique_ptr<weld::Builder> mxAuxBuilder;
    std::unique_ptr<weld::Box> mxTempToplevel;
    std::unique_ptr<weld::Widget> mxContents;

    std::unique_ptr<weld::MenuButton> mxMenuButton;
    std::unique_ptr<weld::Menu> mxMainMenu;
    std::unique_ptr<weld::Menu> mxSubMenu;
    std::unique_ptr<weld::Widget> mxMeasureBox;
    class Item
    {
    private:
        TabBar& mrTabBar;
        std::unique_ptr<weld::Builder> mxBuilder;
    public:
        Item(TabBar& rTabBar);
        ~Item();
        DECL_LINK(HandleClick, const OUString&, void);
        std::unique_ptr<weld::Toolbar> mxButton;
        OUString msDeckId;
        ::std::function<void (const OUString& rsDeckId)> maDeckActivationFunctor;
        bool mbIsHidden;
        bool mbIsHiddenByDefault;
    };
    typedef ::std::vector<std::unique_ptr<Item>> ItemContainer;
    ItemContainer maItems;
    const ::std::function<void (const OUString& rsDeckId)> maDeckActivationFunctor;
    PopupMenuProvider maPopupMenuProvider;

    void CreateTabItem(weld::Toolbar& rButton, const DeckDescriptor& rDeckDescriptor);
    css::uno::Reference<css::graphic::XGraphic> GetItemImage(const DeckDescriptor& rDeskDescriptor) const;
    void UpdateButtonIcons();

    DECL_LINK(OnToolboxClicked, weld::Toggleable&, void);

    SidebarController* pParentSidebarController;
};


} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
