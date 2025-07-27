/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_DOCUMENTTABBAR_HXX
#define INCLUDED_SFX2_DOCUMENTTABBAR_HXX

#include <sfx2/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/image.hxx>
#include <tools/link.hxx>
#include <rtl/ustring.hxx>
#include <memory>

class SfxObjectShell;
class SfxViewFrame;
struct DocumentTabBarData;


/**
 * Document tabbing widget for LibreOffice applications.
 *
 * This widget displays tabs for multiple open documents within a single
 * application window, allowing users to switch between documents efficiently.
 * It integrates with the SFX2 framework for document lifecycle management.
 */
class SFX2_DLLPUBLIC DocumentTabBar final : public Control
{
public:
    /**
     * Information about a single document tab
     */
    struct TabInfo
    {
        sal_uInt16 nId;                 ///< Unique tab identifier
        OUString sTitle;                ///< Display title
        OUString sTooltip;              ///< Tooltip text
        OUString sDocumentPath;         ///< Full document path
        Image aIcon;                    ///< Document type icon
        bool bModified;                 ///< Document has unsaved changes
        bool bActive;                   ///< Currently active tab
        SfxObjectShell* pObjectShell;   ///< Associated document

        TabInfo(sal_uInt16 id, const OUString& title, SfxObjectShell* doc)
            : nId(id), sTitle(title), bModified(false), bActive(false), pObjectShell(doc)
        {}
    };

private:
    std::unique_ptr<DocumentTabBarData> mpData;
    SfxViewFrame* mpViewFrame;

    sal_uInt16 mnNextId;
    sal_uInt16 mnActiveTab;
    sal_uInt16 mnScrollOffset;

    // Event handlers
    Link<DocumentTabBar*, void> maTabActivatedHdl;
    Link<DocumentTabBar*, void> maTabClosedHdl;
    Link<DocumentTabBar*, void> maTabMenuHdl;

    // Layout
    tools::Long mnTabHeight;
    tools::Long mnTabMinWidth;
    tools::Long mnTabMaxWidth;
    tools::Long mnScrollButtonWidth;

    bool mbScrollable;
    bool mbShowCloseButtons;
    bool mbShowIcons;

public:
    DocumentTabBar(vcl::Window* pParent, SfxViewFrame* pViewFrame);
    virtual ~DocumentTabBar() override;
    virtual void dispose() override;

    // Tab management
    sal_uInt16 AddTab(const OUString& rTitle, SfxObjectShell* pDoc);
    void RemoveTab(sal_uInt16 nId);
    void RemoveTab(SfxObjectShell* pDoc);
    void RemoveAllTabs();
    void SetActiveTab(sal_uInt16 nId);
    void SetActiveTab(SfxObjectShell* pDoc);
    sal_uInt16 GetActiveTab() const { return mnActiveTab; }

    // Tab properties
    void SetTabTitle(sal_uInt16 nId, const OUString& rTitle);
    void SetTabModified(sal_uInt16 nId, bool bModified);
    void SetTabIcon(sal_uInt16 nId, const Image& rIcon);
    void SetTabTooltip(sal_uInt16 nId, const OUString& rTooltip);

    // Tab queries
    sal_uInt16 GetTabCount() const;
    sal_uInt16 FindTab(SfxObjectShell* pDoc) const;
    TabInfo GetTabInfo(sal_uInt16 nId) const;
    std::vector<TabInfo> GetAllTabs() const;

    // Configuration
    void SetScrollable(bool bScrollable);
    void SetShowCloseButtons(bool bShow);
    void SetShowIcons(bool bShow);
    void SetTabConstraints(tools::Long nMinWidth, tools::Long nMaxWidth);

    // Event handlers
    void SetTabActivatedHdl(const Link<DocumentTabBar*, void>& rLink) { maTabActivatedHdl = rLink; }
    void SetTabClosedHdl(const Link<DocumentTabBar*, void>& rLink) { maTabClosedHdl = rLink; }
    void SetTabMenuHdl(const Link<DocumentTabBar*, void>& rLink) { maTabMenuHdl = rLink; }

    // Get last activated/closed tab for event handling
    sal_uInt16 GetLastActivatedTab() const;
    sal_uInt16 GetLastClosedTab() const;

protected:
    // VCL overrides
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;
    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void MouseMove(const MouseEvent& rMEvt) override;
    virtual void KeyInput(const KeyEvent& rKEvt) override;
    virtual void Command(const CommandEvent& rCEvt) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;
    virtual void StateChanged(StateChangedType nType) override;
    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;

    // Size calculation
    virtual Size GetOptimalSize() const override;

private:
    // Internal implementation
    void ImplInit();
    void ImplInitSettings();
    void ImplUpdateScrollButtons();
    void ImplUpdateLayout();

    // Tab calculations
    tools::Rectangle ImplGetTabRect(sal_uInt16 nId) const;
    tools::Rectangle ImplGetTabCloseRect(sal_uInt16 nId) const;
    sal_uInt16 ImplGetTabAt(const Point& rPos) const;
    bool ImplIsCloseButtonAt(const Point& rPos, sal_uInt16& rTabId) const;

    // Rendering
    void ImplDrawTab(vcl::RenderContext& rRenderContext, const TabInfo& rTab,
                     const tools::Rectangle& rRect, bool bSelected, bool bHover);
    void ImplDrawScrollButtons(vcl::RenderContext& rRenderContext);
    void ImplDrawBackground(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);

    // Event handling
    void ImplActivateTab(sal_uInt16 nId);
    void ImplCloseTab(sal_uInt16 nId);
    void ImplShowTabMenu(const Point& rPos);
    void ImplScrollTabs(bool bForward);

    // Utilities
    TabInfo* ImplFindTab(sal_uInt16 nId);
    const TabInfo* ImplFindTab(sal_uInt16 nId) const;
    void ImplRemoveTabData(sal_uInt16 nId);
    void ImplEnsureTabVisible(sal_uInt16 nId);
    void ImplActivateNextTab();
    void ImplActivatePreviousTab();

    // Layout calculations
    tools::Long ImplCalculateTabWidth(const TabInfo& rTab) const;
    tools::Long ImplGetAvailableWidth() const;
    bool ImplNeedsScrolling() const;
};

#endif // INCLUDED_SFX2_DOCUMENTTABBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */