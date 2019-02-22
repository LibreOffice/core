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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_TABBAR_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_TABBAR_HXX

#include <sfx2//dllapi.h>
#include <sfx2/sidebar/DeckDescriptor.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>


#include <vcl/menu.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/frame/XFrame.hpp>

#include <functional>

#include <svtools/acceleratorexecute.hxx>

class Button;
class CheckBox;
class RadioButton;

namespace sfx2 { namespace sidebar {

class FocusManager;
class TabBarConfiguration;
class TabItem;
class SidebarController;

/** The tab bar is the container for the individual tabs.
*/
class SFX2_DLLPUBLIC TabBar
    : public vcl::Window
{
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
            const tools::Rectangle&,
            const ::std::vector<DeckMenuData>& rMenuData)> PopupMenuProvider;
    TabBar (
        vcl::Window* pParentWindow,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        const ::std::function<void (const OUString& rsDeckId)>& rDeckActivationFunctor,
        const PopupMenuProvider& rPopupMenuProvider,
        SidebarController* rParentSidebarController);

    virtual ~TabBar() override;
    virtual void dispose() override;

    virtual void Paint (vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rUpdateArea) override;
    virtual void DataChanged (const DataChangedEvent& rDataChangedEvent) override;
    virtual bool EventNotify (NotifyEvent& rEvent) override;

    static sal_Int32 GetDefaultWidth();

    void SetDecks (
        const ResourceManager::DeckContextDescriptorContainer& rDecks);
    void HighlightDeck (const OUString& rsDeckId);
    void RemoveDeckHighlight ();
    OUString const & GetDeckIdForIndex (const sal_Int32 nIndex) const;
    void ToggleHideFlag (const sal_Int32 nIndex);
    void RestoreHideFlags();

    void UpdateFocusManager (FocusManager& rFocusManager);

private:
    css::uno::Reference<css::frame::XFrame> mxFrame;
    VclPtr<CheckBox> mpMenuButton;
    class Item
    {
    public:
        DECL_LINK(HandleClick, Button*, void);
        VclPtr<RadioButton> mpButton;
        OUString msDeckId;
        ::std::function<void (const OUString& rsDeckId)> maDeckActivationFunctor;
        bool mbIsHidden;
        bool mbIsHiddenByDefault;
    };
    typedef ::std::vector<Item> ItemContainer;
    ItemContainer maItems;
    const ::std::function<void (const OUString& rsDeckId)> maDeckActivationFunctor;
    sal_Int32 mnMenuSeparatorY;
    PopupMenuProvider const maPopupMenuProvider;

    VclPtr<RadioButton> CreateTabItem (const DeckDescriptor& rDeckDescriptor);
    Image GetItemImage (const DeckDescriptor& rDeskDescriptor) const;
    void Layout();
    void UpdateButtonIcons();

    DECL_LINK(OnToolboxClicked, Button*, void);

    SidebarController* pParentSidebarController;
    std::unique_ptr<svt::AcceleratorExecute> mpAccel;

};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
