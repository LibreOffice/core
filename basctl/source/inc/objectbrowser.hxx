/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "bastypes.hxx"
#include "doceventnotifier.hxx"
#include "idedataprovider.hxx"

#include <basctl/idecodecompletiontypes.hxx>

#include <memory>
#include <vector>

#include <vcl/weld.hxx>

namespace basctl
{
class Shell;
class IdeDataProviderInterface;
class ObjectBrowserSearch;

enum class ObjectBrowserInitState
{
    NotInitialized,
    Initializing,
    Initialized,
    Failed,
    Disposed
};

class ObjectBrowser : public basctl::DockingWindow, public basctl::DocumentEventListener
{
public:
    ObjectBrowser(Shell& rShell, vcl::Window* pParent);
    ~ObjectBrowser() override;

    void dispose() override;
    virtual bool Close() override;

    void Show(bool bVisible = true);
    void RefreshUI(bool bForceKeepUno = false);

    // DocumentEventListener stubs
    void onDocumentCreated(const ScriptDocument& _rDocument) override;
    void onDocumentOpened(const ScriptDocument& _rDocument) override;
    void onDocumentSave(const ScriptDocument& _rDocument) override;
    void onDocumentSaveDone(const ScriptDocument& _rDocument) override;
    void onDocumentSaveAs(const ScriptDocument& _rDocument) override;
    void onDocumentSaveAsDone(const ScriptDocument& _rDocument) override;
    void onDocumentClosed(const ScriptDocument& _rDocument) override;
    void onDocumentTitleChanged(const ScriptDocument& _rDocument) override;
    void onDocumentModeChanged(const ScriptDocument& _rDocument) override;

    // Accessors for the Search Handler
    weld::Entry* GetFilterBox() { return m_pFilterBox.get(); }
    weld::Button* GetClearSearchButton() { return m_xClearSearchButton.get(); }

private:
    void Initialize();

    // Core tree management methods
    static void ClearTreeView(weld::TreeView& rTree,
                              std::vector<std::shared_ptr<basctl::IdeSymbolInfo>>& rStore);
    void PopulateLeftTree();
    void ShowLoadingState();

    // Core References
    Shell* m_pShell;

    // Data Provider
    std::unique_ptr<IdeDataProviderInterface> m_pDataProvider;

    // State Management
    bool m_bDisposed = false;
    ObjectBrowserInitState m_eInitState = ObjectBrowserInitState::NotInitialized;
    bool m_bUIInitialized = false;

    // UI Widgets
    std::unique_ptr<weld::ComboBox> m_xScopeSelector;
    std::unique_ptr<weld::Entry> m_pFilterBox;
    std::unique_ptr<weld::TreeView> m_xLeftTreeView;
    std::unique_ptr<weld::TreeView> m_xRightMembersView;
    std::unique_ptr<weld::TextView> m_xDetailPane;
    std::unique_ptr<weld::Label> m_xStatusLabel;
    std::unique_ptr<weld::Label> m_xRightPaneHeaderLabel;
    std::unique_ptr<weld::Button> m_xBackButton;
    std::unique_ptr<weld::Button> m_xForwardButton;
    std::unique_ptr<weld::Button> m_xClearSearchButton;

    // Data Storage for TreeViews
    std::vector<std::shared_ptr<basctl::IdeSymbolInfo>> m_aLeftTreeSymbolStore;
    std::vector<std::shared_ptr<basctl::IdeSymbolInfo>> m_aRightTreeSymbolStore;
    std::map<OUString, size_t> m_aNextChunk; // For lazy-loading nodes

    // Search Handler
    std::unique_ptr<ObjectBrowserSearch> m_pSearchHandler;

    // Event Handling
    std::unique_ptr<basctl::DocumentEventNotifier> m_pDocNotifier;

    // UI Event Handlers
    DECL_STATIC_LINK(ObjectBrowser, OnLeftTreeSelect, weld::TreeView&, void);
    DECL_STATIC_LINK(ObjectBrowser, OnRightTreeSelect, weld::TreeView&, void);
    DECL_STATIC_LINK(ObjectBrowser, OnNodeExpand, const weld::TreeIter&, bool);
    DECL_STATIC_LINK(ObjectBrowser, OnScopeChanged, weld::ComboBox&, void);
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
