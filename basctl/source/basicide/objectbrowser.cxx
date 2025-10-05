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

#include <bitmaps.hlst>
#include <iderid.hxx>
#include <strings.hrc>

#include <sal/log.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/event.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/weld.hxx>

namespace basctl
{
namespace
{
// Helper to get an icon resource ID for a given symbol type.
OUString GetIconForSymbol(IdeSymbolKind eKind)
{
    switch (eKind)
    {
        case IdeSymbolKind::ROOT_APPLICATION_LIBS:
            return RID_BMP_INSTALLATION;
        case IdeSymbolKind::ROOT_DOCUMENT_LIBS:
            return RID_BMP_DOCUMENT;
        case IdeSymbolKind::LIBRARY:
            return RID_BMP_MODLIB;
        case IdeSymbolKind::MODULE:
            return RID_BMP_MODULE;
        case IdeSymbolKind::FUNCTION:
        case IdeSymbolKind::SUB:
            return RID_BMP_MACRO;
        case IdeSymbolKind::ROOT_UNO_APIS:
            return u"cmd/sc_configuredialog.png"_ustr;
        case IdeSymbolKind::CLASS_MODULE:
            return u"cmd/sc_insertobject.png"_ustr;
        case IdeSymbolKind::UNO_NAMESPACE:
            return u"cmd/sc_navigator.png"_ustr;
        case IdeSymbolKind::UNO_INTERFACE:
            return u"cmd/sc_insertplugin.png"_ustr;
        case IdeSymbolKind::UNO_SERVICE:
            return u"cmd/sc_insertobjectstarmath.png"_ustr;
        case IdeSymbolKind::UNO_STRUCT:
            return u"cmd/sc_insertframe.png"_ustr;
        case IdeSymbolKind::UNO_ENUM:
            return u"cmd/sc_numberformatmenu.png"_ustr;
        case IdeSymbolKind::UNO_PROPERTY:
            return u"cmd/sc_controlproperties.png"_ustr;
        case IdeSymbolKind::UNO_METHOD:
            return u"cmd/sc_insertformula.png"_ustr;
        case IdeSymbolKind::ENUM_MEMBER:
            return u"cmd/sc_bullet.png"_ustr;
        case IdeSymbolKind::PLACEHOLDER:
            return u"cmd/sc_more.png"_ustr;
        default:
            return u"cmd/sc_insertobject.png"_ustr;
    }
}

// Helper to determine if a symbol is expandable in the tree view.
bool IsExpandable(const IdeSymbolInfo& rSymbol)
{
    switch (rSymbol.eKind)
    {
        case IdeSymbolKind::ROOT_UNO_APIS:
        case IdeSymbolKind::ROOT_APPLICATION_LIBS:
        case IdeSymbolKind::ROOT_DOCUMENT_LIBS:
        case IdeSymbolKind::LIBRARY:
        case IdeSymbolKind::MODULE:
        case IdeSymbolKind::CLASS_MODULE:
        case IdeSymbolKind::UNO_NAMESPACE:
            return true;
        default:
            // This case is for future use when members are nested.
            return !rSymbol.mapMembers.empty();
    }
}

// Helper to add a symbol entry to a tree view and its corresponding data stores.
void AddEntry(weld::TreeView& rTargetTree, std::vector<std::shared_ptr<IdeSymbolInfo>>& rStore,
              std::map<OUString, std::shared_ptr<IdeSymbolInfo>>& rIndex,
              const weld::TreeIter* pParent, const std::shared_ptr<IdeSymbolInfo>& pSymbol,
              bool bChildrenOnDemand, weld::TreeIter* pRetIter = nullptr)
{
    if (!pSymbol)
        return;

    OUString sId = pSymbol->sIdentifier;
    if (pSymbol->eKind == IdeSymbolKind::PLACEHOLDER)
    {
        sId = u"placeholder_for:"_ustr + (pParent ? rTargetTree.get_id(*pParent) : u"root"_ustr);
        pSymbol->sIdentifier = sId;
    }

    if (sId.isEmpty())
    {
        SAL_WARN("basctl", "AddEntry - Symbol with empty ID. Name: " << pSymbol->sName);
        return;
    }

    rStore.push_back(pSymbol);
    rIndex[sId] = pSymbol;

    std::unique_ptr<weld::TreeIter> xLocalIter;
    if (!pRetIter)
    {
        xLocalIter = rTargetTree.make_iterator();
        pRetIter = xLocalIter.get();
    }

    rTargetTree.insert(pParent, -1, &pSymbol->sName, &sId, nullptr, nullptr, bChildrenOnDemand,
                       pRetIter);
    OUString sIconName = GetIconForSymbol(pSymbol->eKind);
    rTargetTree.set_image(*pRetIter, sIconName, -1);
}

} // anonymous namespace

ObjectBrowser::ObjectBrowser(Shell& rShell, vcl::Window* pParent)
    : basctl::DockingWindow(pParent, u"modules/BasicIDE/ui/objectbrowser.ui"_ustr,
                            u"ObjectBrowser"_ustr)
    , m_pShell(&rShell)
    , m_pDataProvider(std::make_unique<IdeDataProvider>())
    , m_bDisposed(false)
    , m_eInitState(ObjectBrowserInitState::NotInitialized)
    , m_bDataMayBeStale(true)
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
        m_xScopeSelector->append(u"ALL_LIBRARIES"_ustr, IDEResId(RID_STR_OB_SCOPE_ALL));
        m_xScopeSelector->append(u"CURRENT_DOCUMENT"_ustr, IDEResId(RID_STR_OB_SCOPE_CURRENT));
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

    // Start listening for application-wide events like document activation
    StartListening(*SfxGetpApp());
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (pViewFrame)
    {
        if (SfxObjectShell* pObjShell = pViewFrame->GetObjectShell())
        {
            m_sLastActiveDocumentIdentifier = pObjShell->GetTitle();
            SAL_INFO("basctl", "ObjectBrowser::Initialize: Active document -> '"
                                   << m_sLastActiveDocumentIdentifier << "'");
        }
    }

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

    // Stop listening to SFX events
    EndListening(*SfxGetpApp());

    // Disconnect all signals
    if (m_xScopeSelector)
        m_xScopeSelector->connect_changed(Link<weld::ComboBox&, void>());
    if (m_xLeftTreeView)
    {
        m_xLeftTreeView->connect_selection_changed(Link<weld::TreeView&, void>());
        m_xLeftTreeView->connect_expanding(Link<const weld::TreeIter&, bool>());
    }

    m_pDocNotifier->dispose();
    m_pDocNotifier.reset();
    m_pSearchHandler.reset();
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
    {
        return;
    }

    if (m_eInitState == ObjectBrowserInitState::NotInitialized)
    {
        Initialize();
    }

    if (m_pDataProvider)
    {
        if (!m_pDataProvider->IsInitialized())
        {
            ShowLoadingState();
            weld::WaitObject aWait(GetFrameWeld());
            m_pDataProvider->Initialize();
            m_bDataMayBeStale = true; // Data is fresh, but force a refresh
        }

        if (m_bDataMayBeStale)
        {
            RefreshUI();
            m_bDataMayBeStale = false;
        }
    }
}

void ObjectBrowser::RefreshUI(bool /*bForceKeepUno*/)
{
    IdeTimer aTimer(u"ObjectBrowser::RefreshUI"_ustr);
    if (!m_pDataProvider || !m_pDataProvider->IsInitialized())
    {
        ShowLoadingState();
        return;
    }

    m_xLeftTreeView->freeze();
    m_xRightMembersView->freeze();
    ClearLeftTreeView();
    ClearRightTreeView();

    static_cast<IdeDataProvider*>(m_pDataProvider.get())->RefreshDocumentNodes();

    if (m_xRightPaneHeaderLabel)
    {
        m_xRightPaneHeaderLabel->set_label(IDEResId(RID_STR_OB_GROUP_MEMBERS));
    }

    if (m_xDetailPane)
    {
        m_xDetailPane->set_text(u""_ustr);
    }

    // Get the filtered list of nodes based on the current scope
    SymbolInfoList aTopLevelNodes = m_pDataProvider->GetTopLevelNodes();
    for (const auto& pSymbol : aTopLevelNodes)
    {
        if (pSymbol)
        {
            AddEntry(*m_xLeftTreeView, m_aLeftTreeSymbolStore, m_aLeftTreeSymbolIndex, nullptr,
                     pSymbol, IsExpandable(*pSymbol));
        }
    }

    m_xLeftTreeView->thaw();

    if (m_xStatusLabel)
        m_xStatusLabel->set_label(u"Ready"_ustr);
}

void ObjectBrowser::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (m_bDisposed)
    {
        return;
    }

    const SfxEventHint* pEventHint = dynamic_cast<const SfxEventHint*>(&rHint);
    if (!pEventHint || pEventHint->GetEventId() != SfxEventHintId::ActivateDoc)
    {
        return;
    }

    rtl::Reference<SfxObjectShell> pObjShell = pEventHint->GetObjShell();
    if (!pObjShell)
    {
        return;
    }

    // Filter out activations of the BASIC IDE
    if (pObjShell->GetFactory().GetDocumentServiceName().endsWith(u"BasicIDE"))
    {
        return;
    }

    OUString sNewDocTitle = pObjShell->GetTitle();
    if (sNewDocTitle != m_sLastActiveDocumentIdentifier)
    {
        m_sLastActiveDocumentIdentifier = sNewDocTitle;
        SAL_INFO("basctl", "ObjectBrowser::Notify: Document activated -> '" << sNewDocTitle << "'");

        // If the user is in "Current Document" mode, a focus change
        // should trigger an immediate refresh to reflect the new context.
        if (IsVisible() && m_xScopeSelector
            && m_xScopeSelector->get_active_id() == "CURRENT_DOCUMENT")
        {
            SAL_INFO("basctl",
                     "ObjectBrowser::Notify: In CURRENT_DOCUMENT scope, updating scope to '"
                         << sNewDocTitle << "' and refreshing UI.");

            // Update scope to newly activated document
            m_pDataProvider->SetScope(IdeBrowserScope::CURRENT_DOCUMENT, sNewDocTitle);
            ScheduleRefresh();
        }
    }
}

void ObjectBrowser::ShowLoadingState()
{
    if (m_xLeftTreeView)
    {
        m_xLeftTreeView->freeze();
        ClearLeftTreeView();
        auto pLoadingNode = std::make_shared<IdeSymbolInfo>(u"[Initializing...]",
                                                            IdeSymbolKind::PLACEHOLDER, u"");
        AddEntry(*m_xLeftTreeView, m_aLeftTreeSymbolStore, m_aLeftTreeSymbolIndex, nullptr,
                 pLoadingNode, false);
        m_xLeftTreeView->thaw();
    }
    if (m_xStatusLabel)
    {
        m_xStatusLabel->set_label(u"Initializing Object Browser..."_ustr);
    }
}

void ObjectBrowser::ClearLeftTreeView()
{
    if (m_xLeftTreeView)
        m_xLeftTreeView->clear();
    m_aLeftTreeSymbolStore.clear();
    m_aLeftTreeSymbolIndex.clear();
}

void ObjectBrowser::ClearRightTreeView()
{
    if (m_xRightMembersView)
        m_xRightMembersView->clear();
    m_aRightTreeSymbolStore.clear();
    m_aRightTreeSymbolIndex.clear();
}

void ObjectBrowser::ScheduleRefresh()
{
    if (IsVisible())
        RefreshUI();
    else
        m_bDataMayBeStale = true;
}

// Document Event Handlers
void ObjectBrowser::onDocumentCreated(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentOpened(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentSave(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentSaveDone(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentSaveAs(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentSaveAsDone(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentClosed(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentTitleChanged(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentModeChanged(const ScriptDocument&) { /* STUB */}

IMPL_STATIC_LINK(ObjectBrowser, OnLeftTreeSelect, weld::TreeView&, /*rTree*/, void) { /* STUB */}
IMPL_STATIC_LINK(ObjectBrowser, OnRightTreeSelect, weld::TreeView&, /*rTree*/, void) { /* STUB */}
IMPL_STATIC_LINK(ObjectBrowser, OnNodeExpand, const weld::TreeIter&, /*rParentIter*/, bool)
{
    return false;
}

IMPL_LINK(ObjectBrowser, OnScopeChanged, weld::ComboBox&, rComboBox, void)
{
    if (m_bDisposed || !m_pDataProvider)
        return;

    OUString sSelectedId = rComboBox.get_active_id();
    IdeBrowserScope eScope = (sSelectedId == "ALL_LIBRARIES") ? IdeBrowserScope::ALL_LIBRARIES
                                                              : IdeBrowserScope::CURRENT_DOCUMENT;

    m_pDataProvider->SetScope(eScope, m_sLastActiveDocumentIdentifier);
    RefreshUI();
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
