/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basidesh.hxx>
#include <objectbrowser.hxx>
#include <objectbrowsersearch.hxx>
#include <idedataprovider.hxx>
#include "idetimer.hxx"
#include <iderid.hxx>
#include <strings.hrc>

#include <vcl/taskpanelist.hxx>
#include <sal/log.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <vcl/weld.hxx>

namespace basctl
{
ObjectBrowser::ObjectBrowser(Shell& rShell, vcl::Window* pParent)
    : basctl::DockingWindow(pParent, u"modules/BasicIDE/ui/objectbrowser.ui"_ustr,
                            u"ObjectBrowser"_ustr)
    , m_pShell(&rShell)
    , m_pDataProvider(std::make_unique<IdeDataProvider>())
    , m_bDisposed(false)
    , m_eInitState(ObjectBrowserInitState::NotInitialized)
    , m_bUIInitialized(false)
    , m_pDocNotifier(std::make_unique<DocumentEventNotifier>(*this))
{
    SetText(IDEResId(RID_STR_OBJECT_BROWSER));
    SetBackground(GetSettings().GetStyleSettings().GetWindowColor());
    EnableInput(true, true);
}

ObjectBrowser::~ObjectBrowser() { disposeOnce(); }

void ObjectBrowser::Initialize()
{
    if (m_eInitState != ObjectBrowserInitState::NotInitialized)
        return;

    // Set state to initializing
    m_eInitState = ObjectBrowserInitState::Initializing;

    // Handles to all our widgets
    m_xScopeSelector = m_xBuilder->weld_combo_box(u"ScopeSelector"_ustr);
    m_pFilterBox = m_xBuilder->weld_entry(u"FilterBox"_ustr);
    m_xLeftTreeView = m_xBuilder->weld_tree_view(u"LeftTreeView"_ustr);
    m_xRightMembersView = m_xBuilder->weld_tree_view(u"RightMembersView"_ustr);
    m_xDetailPane = m_xBuilder->weld_text_view(u"DetailPane"_ustr);
    m_xStatusLabel = m_xBuilder->weld_label(u"StatusLabel"_ustr);
    m_xRightPaneHeaderLabel = m_xBuilder->weld_label(u"RightPaneHeaderLabel"_ustr);
    m_xBackButton = m_xBuilder->weld_button(u"BackButton"_ustr);
    m_xForwardButton = m_xBuilder->weld_button(u"ForwardButton"_ustr);
    m_xClearSearchButton = m_xBuilder->weld_button(u"ClearSearchButton"_ustr);

    m_pSearchHandler = std::make_unique<ObjectBrowserSearch>(*this);
    m_pSearchHandler->Initialize();

    if (m_xScopeSelector)
    {
        m_xScopeSelector->append(IDEResId(RID_STR_OB_SCOPE_ALL), u"ALL_LIBRARIES"_ustr);
        m_xScopeSelector->append(IDEResId(RID_STR_OB_SCOPE_CURRENT), u"CURRENT_DOCUMENT"_ustr);
        m_xScopeSelector->set_active(0);
        m_xScopeSelector->connect_changed(LINK(this, ObjectBrowser, OnScopeChanged));
    }

    if (m_xLeftTreeView)
    {
        m_xLeftTreeView->connect_selection_changed(LINK(this, ObjectBrowser, OnLeftTreeSelect));
        m_xLeftTreeView->connect_expanding(LINK(this, ObjectBrowser, OnNodeExpand));
    }
    if (m_xRightMembersView)
    {
        m_xRightMembersView->connect_selection_changed(
            LINK(this, ObjectBrowser, OnRightTreeSelect));
    }

    if (m_xBackButton)
        m_xBackButton->set_sensitive(false);
    if (m_xForwardButton)
        m_xForwardButton->set_sensitive(false);

    if (GetParent() && GetParent()->GetSystemWindow())
    {
        GetParent()->GetSystemWindow()->GetTaskPaneList()->AddWindow(this);
    }

    m_bUIInitialized = true;
    m_eInitState = ObjectBrowserInitState::Initialized;
}

void ObjectBrowser::dispose()
{
    if (m_bDisposed)
    {
        return;
    }
    m_bDisposed = true;

    m_eInitState = ObjectBrowserInitState::Disposed;

    if (GetParent() && GetParent()->GetSystemWindow())
    {
        if (auto* pTaskPaneList = GetParent()->GetSystemWindow()->GetTaskPaneList())
            pTaskPaneList->RemoveWindow(this);
    }

    // Disconnect all signals
    if (m_xScopeSelector)
        m_xScopeSelector->connect_changed(Link<weld::ComboBox&, void>());
    if (m_xLeftTreeView)
    {
        m_xLeftTreeView->connect_selection_changed(Link<weld::TreeView&, void>());
        m_xLeftTreeView->connect_expanding(Link<const weld::TreeIter&, bool>());
    }
    if (m_xRightMembersView)
        m_xRightMembersView->connect_selection_changed(Link<weld::TreeView&, void>());

    if (m_pDocNotifier)
    {
        m_pDocNotifier->dispose();
        m_pDocNotifier.reset();
    }

    if (m_pSearchHandler)
    {
        m_pSearchHandler.reset();
    }

    m_pDataProvider.reset();

    // Destroy widgets
    m_xScopeSelector.reset();
    m_pFilterBox.reset();
    m_xLeftTreeView.reset();
    m_xRightMembersView.reset();
    m_xDetailPane.reset();
    m_xStatusLabel.reset();
    m_xRightPaneHeaderLabel.reset();
    m_xBackButton.reset();
    m_xForwardButton.reset();
    m_xClearSearchButton.reset();

    DockingWindow::dispose();
}

bool ObjectBrowser::Close()
{
    Show(false);
    m_pShell->GetViewFrame().GetBindings().Invalidate(SID_BASICIDE_OBJECT_BROWSER);
    return false;
}

void ObjectBrowser::Show(bool bVisible)
{
    DockingWindow::Show(bVisible);
    if (!bVisible)
        return;

    if (m_eInitState == ObjectBrowserInitState::NotInitialized)
    {
        Initialize();
    }

    if (m_pDataProvider && !m_pDataProvider->IsInitialized())
    {
        ShowLoadingState();
        {
            weld::WaitObject aWait(GetFrameWeld());
            m_pDataProvider->Initialize();
        }
        RefreshUI();
    }
}

void ObjectBrowser::RefreshUI(bool /*bForceKeepUno*/)
{
    if (m_pDataProvider && m_pDataProvider->IsInitialized())
    {
        PopulateLeftTree();
        if (m_xStatusLabel)
            m_xStatusLabel->set_label(u"Ready"_ustr);
    }
    else
    {
        ShowLoadingState();
    }
}

void ObjectBrowser::PopulateLeftTree()
{
    if (!m_xLeftTreeView)
        return;

    m_xLeftTreeView->freeze();
    ClearTreeView(*m_xLeftTreeView, m_aLeftTreeSymbolStore);

    SymbolInfoList aTopLevelNodes = m_pDataProvider->GetTopLevelNodes();
    for (const auto& pNode : aTopLevelNodes)
    {
        m_aLeftTreeSymbolStore.push_back(pNode);
        m_xLeftTreeView->insert(nullptr, -1, &pNode->sName, nullptr, nullptr, nullptr, true,
                                nullptr);
    }

    m_xLeftTreeView->thaw();
}

void ObjectBrowser::ShowLoadingState()
{
    if (m_xLeftTreeView)
    {
        m_xLeftTreeView->freeze();
        ClearTreeView(*m_xLeftTreeView, m_aLeftTreeSymbolStore);

        auto pLoadingNode = std::make_shared<IdeSymbolInfo>(u"[Initializing...]",
                                                            IdeSymbolKind::PLACEHOLDER, u"");

        m_aLeftTreeSymbolStore.push_back(pLoadingNode);
        m_xLeftTreeView->insert(nullptr, -1, &pLoadingNode->sName, nullptr, nullptr, nullptr, false,
                                nullptr);
        m_xLeftTreeView->thaw();
    }
    if (m_xStatusLabel)
        m_xStatusLabel->set_label(u"Initializing Object Browser..."_ustr);
}

void ObjectBrowser::ClearTreeView(weld::TreeView& rTree,
                                  std::vector<std::shared_ptr<basctl::IdeSymbolInfo>>& rStore)
{
    rTree.clear();
    rStore.clear();
}

void ObjectBrowser::onDocumentCreated(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentOpened(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentSave(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentSaveDone(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentSaveAs(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentSaveAsDone(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentClosed(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentTitleChanged(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentModeChanged(const ScriptDocument&) { /* STUB */}

IMPL_STATIC_LINK(ObjectBrowser, OnLeftTreeSelect, weld::TreeView&, /*rTree*/, void) { /* STUB */}
IMPL_STATIC_LINK(ObjectBrowser, OnRightTreeSelect, weld::TreeView&, /*rTree*/, void) { /* STUB */}
IMPL_STATIC_LINK(ObjectBrowser, OnNodeExpand, const weld::TreeIter&, /*rParentIter*/, bool)
{
    return false;
}
IMPL_STATIC_LINK(ObjectBrowser, OnScopeChanged, weld::ComboBox&, /*rComboBox*/, void) { /* STUB */}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
