/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/layout.hxx>

#include <sfx2/dllapi.h>
#include <sfx2/viewfrm.hxx>

#include <vcl/weld.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <functional>
#include <unordered_set>

struct CurrentEntry final
{
    OUString m_aCommandURL;
    OUString m_aTooltip;

    CurrentEntry(OUString const& rCommandURL, OUString const& rTooltip)
        : m_aCommandURL(rCommandURL)
        , m_aTooltip(rTooltip)
    {
    }
};

struct MenuContent final
{
    OUString m_aCommandURL;
    OUString m_aMenuLabel;
    OUString m_aSearchableMenuLabel;
    OUString m_aFullLabelWithPath;
    OUString m_aTooltip;
    std::vector<MenuContent> m_aSubMenuContent;
};

class MenuContentHandler final
{
private:
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    css::uno::Reference<css::i18n::XCharacterClassification> m_xCharacterClassification;
    css::uno::Reference<css::util::XURLTransformer> m_xURLTransformer;

    MenuContent m_aMenuContent;
    OUString m_sModuleLongName;
    OUString toLower(OUString const& rString);
    std::unordered_set<OUString> m_aAdded;

public:
    MenuContentHandler(css::uno::Reference<css::frame::XFrame> const& xFrame);

    void gatherMenuContent(css::uno::Reference<css::container::XIndexAccess> const& xIndexAccess,
                           MenuContent& rMenuContent);

    void findInMenu(OUString const& rText, std::unique_ptr<weld::TreeView>& rpCommandTreeView,
                    std::vector<CurrentEntry>& rCommandList);

private:
    void findInMenuRecursive(
        MenuContent const& rMenuContent, OUString const& rText,
        std::unique_ptr<weld::TreeView>& rpCommandTreeView, std::vector<CurrentEntry>& rCommandList,
        std::function<bool(MenuContent const&, OUString const&)> const& rSearchCriterium);

    void addCommandIfPossible(MenuContent const& rMenuContent,
                              std::unique_ptr<weld::TreeView>& rpCommandTreeView,
                              std::vector<CurrentEntry>& rCommandList);
};

class CommandListBox final
{
private:
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Popover> mxPopover;
    std::unique_ptr<weld::Entry> mpEntry;
    std::unique_ptr<weld::TreeView> mpCommandTreeView;

    std::vector<CurrentEntry> maCommandList;
    std::unique_ptr<MenuContentHandler> mpMenuContentHandler;

    DECL_LINK(QueryTooltip, const weld::TreeIter&, OUString);
    DECL_LINK(RowActivated, weld::TreeView&, bool);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(TreeViewKeyPress, const KeyEvent&, bool);

    void dispatchCommandAndClose(OUString const& rCommand);

public:
    CommandListBox(weld::Window* pParent, css::uno::Reference<css::frame::XFrame> const& xFrame);
    void connect_closed(const Link<weld::Popover&, void>& rLink)
    {
        mxPopover->connect_closed(rLink);
    }
};

class CommandPopupHandler final
{
private:
    std::unique_ptr<CommandListBox> mpListBox;

public:
    void showPopup(weld::Window* pParent, css::uno::Reference<css::frame::XFrame> const& xFrame);
    DECL_LINK(PopupModeEnd, weld::Popover&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
