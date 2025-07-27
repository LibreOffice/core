/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/documenttabbar.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/event.hxx>
#include <sfx2/app.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <iostream>

namespace
{
    /**
     * Global manager for document tabbing functionality.
     * Coordinates between SfxViewFrame instances and the DocumentTabBar widget.
     */
    class DocumentTabBarManager : public SfxListener
{
private:
    std::vector<VclPtr<DocumentTabBar>> m_aTabBars;  // All tab bar instances
    SfxViewFrame* m_pCurrentViewFrame;
    std::vector<SfxViewFrame*> m_aViewFrames;
    bool m_bEnabled;

public:
    DocumentTabBarManager();
    virtual ~DocumentTabBarManager() override;

    // Lifecycle management
    void Initialize();
    void Dispose();
    bool IsEnabled() const { return m_bEnabled; }

    // ViewFrame management
    void RegisterViewFrame(SfxViewFrame* pViewFrame);
    void UnregisterViewFrame(SfxViewFrame* pViewFrame);
    void SetCurrentViewFrame(SfxViewFrame* pViewFrame);

    // Tab operations
    void CreateTabForDocument(SfxObjectShell* pDoc, SfxViewFrame* pViewFrame);
    void RemoveTabForDocument(SfxObjectShell* pDoc);
    void UpdateTabTitle(SfxObjectShell* pDoc, const OUString& rTitle);
    void UpdateTabModified(SfxObjectShell* pDoc, bool bModified);
    void ActivateTab(SfxObjectShell* pDoc);

    // Event handling
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // Tab bar access
    void RegisterTabBar(DocumentTabBar* pTabBar);
    void UnregisterTabBar(DocumentTabBar* pTabBar);
    DocumentTabBar* GetTabBar() const { return m_aTabBars.empty() ? nullptr : m_aTabBars[0].get(); }

private:
    // Event handlers
    DECL_LINK(TabActivatedHdl, DocumentTabBar*, void);
    DECL_LINK(TabClosedHdl, DocumentTabBar*, void);
    DECL_LINK(TabMenuHdl, DocumentTabBar*, void);

    // Utilities
    SfxViewFrame* FindViewFrameForDocument(SfxObjectShell* pDoc) const;
    void SwitchToDocument(SfxObjectShell* pDoc);
    void CloseDocument(SfxObjectShell* pDoc);
    void ShowTabContextMenu();
    bool ShouldShowTabs() const;
    void UpdateTabBarVisibility();
    void UpdateAllTabBars();
    std::vector<DocumentTabBar::TabInfo> CollectAllDocuments();
};

    // Global document tab bar manager instance
    static DocumentTabBarManager* g_pTabBarManager = nullptr;

DocumentTabBarManager::DocumentTabBarManager()
    : m_pCurrentViewFrame(nullptr)
    , m_bEnabled(false)
{
}

DocumentTabBarManager::~DocumentTabBarManager()
{
    Dispose();
}

void DocumentTabBarManager::Initialize()
{
    // Check if document tabbing is enabled
    m_bEnabled = ShouldShowTabs();

    if (!m_bEnabled)
        return;

    // Listen to global SFX events
    StartListening(*SfxGetpApp());
}

void DocumentTabBarManager::Dispose()
{
    if (SfxGetpApp())
        EndListening(*SfxGetpApp());

    m_aViewFrames.clear();
    m_pCurrentViewFrame = nullptr;

    // Clear all tab bar references (they are disposed by their parent windows)
    m_aTabBars.clear();

    m_bEnabled = false;
}

void DocumentTabBarManager::RegisterTabBar(DocumentTabBar* pTabBar)
{
    if (!pTabBar || !m_bEnabled)
        return;

    // Add to our list if not already present
    auto it = std::find(m_aTabBars.begin(), m_aTabBars.end(), pTabBar);
    if (it == m_aTabBars.end())
    {
        m_aTabBars.emplace_back(pTabBar);
        // DEBUG: Registered DocumentTabBar, total count: m_aTabBars.size()

        // Set up event handlers for this tab bar
        pTabBar->SetTabActivatedHdl(LINK(this, DocumentTabBarManager, TabActivatedHdl));
        pTabBar->SetTabClosedHdl(LINK(this, DocumentTabBarManager, TabClosedHdl));
        pTabBar->SetTabMenuHdl(LINK(this, DocumentTabBarManager, TabMenuHdl));

        // Synchronize this tab bar with existing documents
        UpdateAllTabBars();
    }
}

void DocumentTabBarManager::UnregisterTabBar(DocumentTabBar* pTabBar)
{
    if (!pTabBar)
        return;

    // Remove from our list
    auto it = std::find(m_aTabBars.begin(), m_aTabBars.end(), pTabBar);
    if (it != m_aTabBars.end())
    {
        m_aTabBars.erase(it);
    }
}

void DocumentTabBarManager::RegisterViewFrame(SfxViewFrame* pViewFrame)
{
    if (!pViewFrame || !m_bEnabled)
        return;

    // Add to our list if not already present
    auto it = std::find(m_aViewFrames.begin(), m_aViewFrames.end(), pViewFrame);
    if (it == m_aViewFrames.end())
    {
        m_aViewFrames.push_back(pViewFrame);

        // Create tab for the document
        SfxObjectShell* pDoc = pViewFrame->GetObjectShell();
        if (pDoc)
            CreateTabForDocument(pDoc, pViewFrame);
    }
}

void DocumentTabBarManager::UnregisterViewFrame(SfxViewFrame* pViewFrame)
{
    if (!pViewFrame)
        return;

    // Remove from our list
    auto it = std::find(m_aViewFrames.begin(), m_aViewFrames.end(), pViewFrame);
    if (it != m_aViewFrames.end())
    {
        // Remove tab for the document
        SfxObjectShell* pDoc = pViewFrame->GetObjectShell();
        if (pDoc)
            RemoveTabForDocument(pDoc);

        m_aViewFrames.erase(it);

        if (m_pCurrentViewFrame == pViewFrame)
            m_pCurrentViewFrame = nullptr;
    }

    UpdateTabBarVisibility();
}

void DocumentTabBarManager::SetCurrentViewFrame(SfxViewFrame* pViewFrame)
{
    if (m_pCurrentViewFrame != pViewFrame)
    {
        m_pCurrentViewFrame = pViewFrame;

        // Update all tab bars to show the new active document
        UpdateAllTabBars();
    }
}

void DocumentTabBarManager::CreateTabForDocument(SfxObjectShell* pDoc, SfxViewFrame* /*pViewFrame*/)
{
    if (!pDoc || m_aTabBars.empty())
        return;

    // Update all tab bars with the new document
    UpdateAllTabBars();
}

void DocumentTabBarManager::RemoveTabForDocument(SfxObjectShell* pDoc)
{
    if (!pDoc || m_aTabBars.empty())
        return;

    // Update all tab bars to remove the document
    UpdateAllTabBars();
}

void DocumentTabBarManager::UpdateTabTitle(SfxObjectShell* pDoc, const OUString& /*rTitle*/)
{
    if (!pDoc || m_aTabBars.empty())
        return;

    // Update all tab bars with the new title
    UpdateAllTabBars();
}

void DocumentTabBarManager::UpdateTabModified(SfxObjectShell* pDoc, bool /*bModified*/)
{
    if (!pDoc || m_aTabBars.empty())
        return;

    // Update all tab bars with the modified state
    UpdateAllTabBars();
}

void DocumentTabBarManager::ActivateTab(SfxObjectShell* pDoc)
{
    if (!pDoc || m_aTabBars.empty())
        return;

    // Update all tab bars to show the active document
    UpdateAllTabBars();
}

void DocumentTabBarManager::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (!m_bEnabled)
        return;

    if (rHint.GetId() == SfxHintId::ThisIsAnSfxEventHint)
    {
        const SfxEventHint& rEventHint = static_cast<const SfxEventHint&>(rHint);
        SfxObjectShell* pDoc = rEventHint.GetObjShell().get();

        if (!pDoc)
            return;

        switch (rEventHint.GetEventId())
        {
            case SfxEventHintId::CreateDoc:
            case SfxEventHintId::OpenDoc:
                // Document opened - tab will be created when view frame is registered
                break;

            case SfxEventHintId::CloseDoc:
                RemoveTabForDocument(pDoc);
                break;

            case SfxEventHintId::SaveDoc:
            case SfxEventHintId::SaveAsDoc:
                UpdateTabModified(pDoc, false);
                break;

            case SfxEventHintId::ModifyChanged:
                UpdateTabModified(pDoc, pDoc->IsModified());
                break;

            default:
                break;
        }
    }
}

IMPL_LINK(DocumentTabBarManager, TabActivatedHdl, DocumentTabBar*, pTabBar, void)
{
    std::cout << "DEBUG: TabActivatedHdl called" << std::endl;
    if (!pTabBar)
    {
        std::cout << "DEBUG: pTabBar is null" << std::endl;
        return;
    }

    sal_uInt16 nActiveTab = pTabBar->GetActiveTab();
    std::cout << "DEBUG: Active tab ID: " << nActiveTab << std::endl;
    if (nActiveTab == 0)
        return;

    DocumentTabBar::TabInfo aTabInfo = pTabBar->GetTabInfo(nActiveTab);
    if (aTabInfo.pObjectShell)
    {
        std::cout << "DEBUG: Calling SwitchToDocument for: " << aTabInfo.sTitle.toUtf8().getStr() << std::endl;
        SwitchToDocument(aTabInfo.pObjectShell);
    }
    else
    {
        std::cout << "DEBUG: aTabInfo.pObjectShell is null" << std::endl;
    }
}

IMPL_LINK(DocumentTabBarManager, TabClosedHdl, DocumentTabBar*, pTabBar, void)
{
    if (!pTabBar)
        return;

    sal_uInt16 nClosedTab = pTabBar->GetLastClosedTab();
    if (nClosedTab == 0)
        return;

    // Find the document that was closed
    // Note: The tab is already removed at this point, so we need to track this differently
    // This is a simplified implementation
}

IMPL_LINK_NOARG(DocumentTabBarManager, TabMenuHdl, DocumentTabBar*, void)
{
    ShowTabContextMenu();
}

SfxViewFrame* DocumentTabBarManager::FindViewFrameForDocument(SfxObjectShell* pDoc) const
{
    for (SfxViewFrame* pFrame : m_aViewFrames)
    {
        if (pFrame && pFrame->GetObjectShell() == pDoc)
            return pFrame;
    }
    return nullptr;
}

void DocumentTabBarManager::SwitchToDocument(SfxObjectShell* pDoc)
{
    std::cout << "DEBUG: SwitchToDocument called" << std::endl;

    // Find the ViewFrame containing this document
    SfxViewFrame* pTargetFrame = FindViewFrameForDocument(pDoc);
    std::cout << "DEBUG: Found target frame: " << (pTargetFrame ? "yes" : "no") << std::endl;
    std::cout << "DEBUG: Current frame == target frame: " << (pTargetFrame == m_pCurrentViewFrame ? "yes" : "no") << std::endl;

    if (pTargetFrame && pTargetFrame != m_pCurrentViewFrame)
    {
        std::cout << "DEBUG: Switching to different frame - bringing to front" << std::endl;

        // Bring the target window to front
        pTargetFrame->ToTop();

        // Get the window and bring it to front with focus
        vcl::Window& rWindow = pTargetFrame->GetWindow();
        rWindow.ToTop(ToTopFlags::RestoreWhenMin | ToTopFlags::ForegroundTask);
        rWindow.GrabFocus();

        // Make it the active frame
        pTargetFrame->MakeActive_Impl(false);

        // Update current frame tracking
        SetCurrentViewFrame(pTargetFrame);

        // Update all tab bars to show new active document
        UpdateAllTabBars();
    }
    else
    {
        std::cout << "DEBUG: Not switching - same frame or target frame is null" << std::endl;
    }
}

void DocumentTabBarManager::CloseDocument(SfxObjectShell* pDoc)
{
    SfxViewFrame* pViewFrame = FindViewFrameForDocument(pDoc);
    if (pViewFrame)
    {
        pViewFrame->GetFrame().DoClose();
    }
}

void DocumentTabBarManager::ShowTabContextMenu()
{
    // Placeholder for context menu implementation
    // Would show menu with options like "Close Tab", "Close Others", etc.
}

bool DocumentTabBarManager::ShouldShowTabs() const
{
    // Check configuration setting for document tabbing
    // For now, always return true for the prototype
    try
    {
        return true; // Default to enabled for prototype
    }
    catch (...)
    {
        return true; // Default to enabled for prototype
    }
}

void DocumentTabBarManager::UpdateTabBarVisibility()
{
    // Show tab bars only if we have multiple documents or always show setting is enabled
    bool bShow = (m_aViewFrames.size() > 0) && m_bEnabled;

    for (VclPtr<DocumentTabBar>& pTabBar : m_aTabBars)
    {
        if (pTabBar)
            pTabBar->Show(bShow);
    }
}

void DocumentTabBarManager::UpdateAllTabBars()
{
    // Get list of all open documents
    std::vector<DocumentTabBar::TabInfo> allTabs = CollectAllDocuments();

    // DEBUG: UpdateAllTabBars called, allTabs.size() documents, m_aTabBars.size() tab bars

    // Update each tab bar with the same content
    for (VclPtr<DocumentTabBar>& pTabBar : m_aTabBars)
    {
        if (pTabBar)
        {
            // Clear existing tabs
            pTabBar->RemoveAllTabs();

            // Add all documents
            for (const auto& tabInfo : allTabs)
            {
                sal_uInt16 nTabId = pTabBar->AddTab(tabInfo.sTitle, tabInfo.pObjectShell);
                if (tabInfo.bActive)
                    pTabBar->SetActiveTab(nTabId);
            }
        }
    }
}

std::vector<DocumentTabBar::TabInfo> DocumentTabBarManager::CollectAllDocuments()
{
    std::vector<DocumentTabBar::TabInfo> allTabs;

    // Iterate through all view frames to collect documents
    for (SfxViewFrame* pFrame : m_aViewFrames)
    {
        if (pFrame && pFrame->GetObjectShell())
        {
            SfxObjectShell* pDoc = pFrame->GetObjectShell();
            DocumentTabBar::TabInfo tabInfo(0, pDoc->GetTitle(), pDoc);
            tabInfo.bModified = pDoc->IsModified();
            tabInfo.bActive = (pFrame == m_pCurrentViewFrame);

            // Check if already added (avoid duplicates)
            auto it = std::find_if(allTabs.begin(), allTabs.end(),
                [pDoc](const DocumentTabBar::TabInfo& tab) {
                    return tab.pObjectShell == pDoc;
                });

            if (it == allTabs.end())
                allTabs.push_back(tabInfo);
        }
    }

    return allTabs;
}

} // anonymous namespace

// Global interface functions

namespace sfx2 {

void InitializeDocumentTabBar()
{
    if (!g_pTabBarManager)
    {
        g_pTabBarManager = new DocumentTabBarManager();
        g_pTabBarManager->Initialize();
    }
}

void DisposeDocumentTabBar()
{
    if (g_pTabBarManager)
    {
        delete g_pTabBarManager;
        g_pTabBarManager = nullptr;
    }
}

void RegisterViewFrameForTabs(SfxViewFrame* pViewFrame)
{
    if (g_pTabBarManager)
        g_pTabBarManager->RegisterViewFrame(pViewFrame);
}

void UnregisterViewFrameFromTabs(SfxViewFrame* pViewFrame)
{
    if (g_pTabBarManager)
        g_pTabBarManager->UnregisterViewFrame(pViewFrame);
}

void SetCurrentTabViewFrame(SfxViewFrame* pViewFrame)
{
    if (g_pTabBarManager)
        g_pTabBarManager->SetCurrentViewFrame(pViewFrame);
}

DocumentTabBar* GetDocumentTabBar()
{
    return g_pTabBarManager ? g_pTabBarManager->GetTabBar() : nullptr;
}

bool IsDocumentTabbingEnabled()
{
    return g_pTabBarManager ? g_pTabBarManager->IsEnabled() : false;
}

void RegisterDocumentTabBar(DocumentTabBar* pTabBar)
{
    if (g_pTabBarManager)
        g_pTabBarManager->RegisterTabBar(pTabBar);
}

void UnregisterDocumentTabBar(DocumentTabBar* pTabBar)
{
    if (g_pTabBarManager)
        g_pTabBarManager->UnregisterTabBar(pTabBar);
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */