/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/documenttabbar.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>

#include <vcl/event.hxx>
#include <vcl/help.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/commandevent.hxx>

#include <tools/gen.hxx>
#include <comphelper/processfactory.hxx>

#include <algorithm>
#include <vector>
#include <iostream>

constexpr tools::Long TAB_HEIGHT = 28;
constexpr tools::Long TAB_MIN_WIDTH = 80;
constexpr tools::Long TAB_MAX_WIDTH = 200;
constexpr tools::Long SCROLL_BUTTON_WIDTH = 20;
constexpr tools::Long TAB_SPACING = 2;
constexpr tools::Long CLOSE_BUTTON_SIZE = 12;

struct DocumentTabBarData
{
    std::vector<DocumentTabBar::TabInfo> aTabs;
    sal_uInt16 nLastActivatedTab = 0;
    sal_uInt16 nLastClosedTab = 0;
    sal_uInt16 nHoverTab = 0;
    bool bScrollForward = false;
    bool bScrollBack = false;
    tools::Rectangle aScrollForwardRect;
    tools::Rectangle aScrollBackRect;

    DocumentTabBarData() = default;
};

DocumentTabBar::DocumentTabBar(vcl::Window* pParent, SfxViewFrame* pViewFrame)
    : Control(pParent)
    , mpData(std::make_unique<DocumentTabBarData>())
    , mpViewFrame(pViewFrame)
    , mnNextId(1)
    , mnActiveTab(0)
    , mnScrollOffset(0)
    , mnTabHeight(TAB_HEIGHT)
    , mnTabMinWidth(TAB_MIN_WIDTH)
    , mnTabMaxWidth(TAB_MAX_WIDTH)
    , mnScrollButtonWidth(SCROLL_BUTTON_WIDTH)
    , mbScrollable(true)
    , mbShowCloseButtons(true)
    , mbShowIcons(true)
{
    ImplInit();
}

DocumentTabBar::~DocumentTabBar()
{
    disposeOnce();
}

void DocumentTabBar::dispose()
{
    mpData.reset();
    Control::dispose();
}

void DocumentTabBar::ImplInit()
{
    SetOutputSizePixel(Size(100, mnTabHeight));

    ImplInitSettings();

    // Enable mouse and keyboard events
    SetMouseTransparent(false);
    EnableChildTransparentMode(false);
    SetPaintTransparent(false);

    Show();
}

void DocumentTabBar::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    SetBackground(rStyleSettings.GetFaceColor());
    SetTextColor(rStyleSettings.GetButtonTextColor());
}

sal_uInt16 DocumentTabBar::AddTab(const OUString& rTitle, SfxObjectShell* pDoc)
{
    sal_uInt16 nNewId = mnNextId++;

    mpData->aTabs.emplace_back(nNewId, rTitle, pDoc);
    TabInfo& rNewTab = mpData->aTabs.back();

    // Set document path if available
    if (pDoc)
    {
        rNewTab.sDocumentPath = pDoc->GetMedium() ? pDoc->GetMedium()->GetName() : OUString();
        rNewTab.sTooltip = rNewTab.sDocumentPath.isEmpty() ? rTitle : rNewTab.sDocumentPath;
    }

    ImplUpdateLayout();
    Invalidate();

    return nNewId;
}

void DocumentTabBar::RemoveTab(sal_uInt16 nId)
{
    auto it = std::find_if(mpData->aTabs.begin(), mpData->aTabs.end(),
                          [nId](const TabInfo& tab) { return tab.nId == nId; });

    if (it != mpData->aTabs.end())
    {
        // Store for event handling
        mpData->nLastClosedTab = nId;

        // If removing active tab, activate another one
        if (it->bActive && mpData->aTabs.size() > 1)
        {
            // Try to activate next tab, or previous if this is the last
            auto nextIt = it + 1;
            if (nextIt != mpData->aTabs.end())
                SetActiveTab(nextIt->nId);
            else if (it != mpData->aTabs.begin())
                SetActiveTab((it - 1)->nId);
        }

        mpData->aTabs.erase(it);

        // Reset active tab if no tabs remain
        if (mpData->aTabs.empty())
            mnActiveTab = 0;

        ImplUpdateLayout();
        Invalidate();

        // Notify listeners
        maTabClosedHdl.Call(this);
    }
}

void DocumentTabBar::RemoveTab(SfxObjectShell* pDoc)
{
    sal_uInt16 nId = FindTab(pDoc);
    if (nId != 0)
        RemoveTab(nId);
}

void DocumentTabBar::RemoveAllTabs()
{
    mpData->aTabs.clear();
    mnActiveTab = 0;
    mnScrollOffset = 0;
    mpData->nLastActivatedTab = 0;
    mpData->nLastClosedTab = 0;
    mpData->nHoverTab = 0;

    ImplUpdateLayout();
    Invalidate();
}

void DocumentTabBar::SetActiveTab(sal_uInt16 nId)
{
    if (mnActiveTab == nId)
        return;

    // Deactivate current tab
    if (mnActiveTab != 0)
    {
        TabInfo* pCurrentTab = ImplFindTab(mnActiveTab);
        if (pCurrentTab)
            pCurrentTab->bActive = false;
    }

    // Activate new tab
    TabInfo* pNewTab = ImplFindTab(nId);
    if (pNewTab)
    {
        mpData->nLastActivatedTab = mnActiveTab;
        mnActiveTab = nId;
        pNewTab->bActive = true;

        // Ensure tab is visible
        ImplEnsureTabVisible(nId);

        Invalidate();

        // Notify listeners
        maTabActivatedHdl.Call(this);
    }
}

void DocumentTabBar::SetActiveTab(SfxObjectShell* pDoc)
{
    sal_uInt16 nId = FindTab(pDoc);
    if (nId != 0)
        SetActiveTab(nId);
}

sal_uInt16 DocumentTabBar::GetTabCount() const
{
    return static_cast<sal_uInt16>(mpData->aTabs.size());
}

sal_uInt16 DocumentTabBar::FindTab(SfxObjectShell* pDoc) const
{
    auto it = std::find_if(mpData->aTabs.begin(), mpData->aTabs.end(),
                          [pDoc](const TabInfo& tab) { return tab.pObjectShell == pDoc; });

    return (it != mpData->aTabs.end()) ? it->nId : 0;
}

DocumentTabBar::TabInfo DocumentTabBar::GetTabInfo(sal_uInt16 nId) const
{
    const TabInfo* pTab = ImplFindTab(nId);
    return pTab ? *pTab : TabInfo(0, OUString(), nullptr);
}

std::vector<DocumentTabBar::TabInfo> DocumentTabBar::GetAllTabs() const
{
    return mpData->aTabs;
}

void DocumentTabBar::SetTabTitle(sal_uInt16 nId, const OUString& rTitle)
{
    TabInfo* pTab = ImplFindTab(nId);
    if (pTab && pTab->sTitle != rTitle)
    {
        pTab->sTitle = rTitle;
        Invalidate();
    }
}

void DocumentTabBar::SetTabModified(sal_uInt16 nId, bool bModified)
{
    TabInfo* pTab = ImplFindTab(nId);
    if (pTab && pTab->bModified != bModified)
    {
        pTab->bModified = bModified;
        Invalidate();
    }
}

void DocumentTabBar::SetTabIcon(sal_uInt16 nId, const Image& rIcon)
{
    TabInfo* pTab = ImplFindTab(nId);
    if (pTab)
    {
        pTab->aIcon = rIcon;
        Invalidate();
    }
}

void DocumentTabBar::SetTabTooltip(sal_uInt16 nId, const OUString& rTooltip)
{
    TabInfo* pTab = ImplFindTab(nId);
    if (pTab)
        pTab->sTooltip = rTooltip;
}

sal_uInt16 DocumentTabBar::GetLastActivatedTab() const
{
    return mpData->nLastActivatedTab;
}

sal_uInt16 DocumentTabBar::GetLastClosedTab() const
{
    return mpData->nLastClosedTab;
}

void DocumentTabBar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    ImplDrawBackground(rRenderContext, rRect);

    if (mpData->aTabs.empty())
        return;

    // Draw scroll buttons if needed
    if (ImplNeedsScrolling())
        ImplDrawScrollButtons(rRenderContext);

    // Calculate visible tabs
    tools::Long nStartX = ImplNeedsScrolling() ? mnScrollButtonWidth : 0;
    tools::Long nAvailableWidth = ImplGetAvailableWidth();
    tools::Long nCurrentX = nStartX;

    // Draw tabs
    for (size_t i = mnScrollOffset; i < mpData->aTabs.size() && nCurrentX < nAvailableWidth; ++i)
    {
        const TabInfo& rTab = mpData->aTabs[i];
        tools::Long nTabWidth = ImplCalculateTabWidth(rTab);

        if (nCurrentX + nTabWidth > nAvailableWidth)
            break;

        tools::Rectangle aTabRect(nCurrentX, 0, nCurrentX + nTabWidth, mnTabHeight);

        bool bHover = (mpData->nHoverTab == rTab.nId);
        ImplDrawTab(rRenderContext, rTab, aTabRect, rTab.bActive, bHover);

        nCurrentX += nTabWidth + TAB_SPACING;
    }
}

void DocumentTabBar::ImplDrawBackground(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
    rRenderContext.SetLineColor();
    rRenderContext.DrawRect(rRect);

    // Draw bottom border
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
    rRenderContext.DrawLine(Point(rRect.Left(), rRect.Bottom()),
                           Point(rRect.Right(), rRect.Bottom()));
}

void DocumentTabBar::ImplDrawTab(vcl::RenderContext& rRenderContext, const TabInfo& rTab,
                                const tools::Rectangle& rRect, bool bSelected, bool bHover)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // Tab background
    Color aTabColor = bSelected ? rStyleSettings.GetHighlightColor() :
                     bHover ? rStyleSettings.GetFaceColor() :
                     rStyleSettings.GetFaceColor();

    rRenderContext.SetFillColor(aTabColor);
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
    rRenderContext.DrawRect(rRect);

    // Tab text
    Color aTextColor = bSelected ? rStyleSettings.GetHighlightTextColor() :
                      rStyleSettings.GetButtonTextColor();
    rRenderContext.SetTextColor(aTextColor);

    OUString sDisplayText = rTab.sTitle;
    if (rTab.bModified)
        sDisplayText = "*" + sDisplayText;

    tools::Rectangle aTextRect = rRect;
    aTextRect.AdjustLeft(8);
    aTextRect.AdjustRight(-8);

    // Reserve space for close button
    if (mbShowCloseButtons)
        aTextRect.AdjustRight(-CLOSE_BUTTON_SIZE + 4);

    // Reserve space for icon
    if (mbShowIcons && !rTab.aIcon.operator!())
    {
        Size aIconSize = rTab.aIcon.GetSizePixel();
        Point aIconPos(aTextRect.Left(),
                      aTextRect.Top() + (aTextRect.GetHeight() - aIconSize.Height()) / 2);
        rRenderContext.DrawImage(aIconPos, rTab.aIcon);
        aTextRect.AdjustLeft(aIconSize.Width() + 4);
    }

    DrawTextFlags nTextStyle = DrawTextFlags::Left | DrawTextFlags::VCenter | DrawTextFlags::EndEllipsis;
    rRenderContext.DrawText(aTextRect, sDisplayText, nTextStyle);

    // Close button
    if (mbShowCloseButtons)
    {
        tools::Rectangle aCloseRect = ImplGetTabCloseRect(rTab.nId);
        if (!aCloseRect.IsEmpty())
        {
            rRenderContext.SetLineColor(aTextColor);
            // Draw simple X
            rRenderContext.DrawLine(aCloseRect.TopLeft(), aCloseRect.BottomRight());
            rRenderContext.DrawLine(aCloseRect.TopRight(), aCloseRect.BottomLeft());
        }
    }
}

void DocumentTabBar::ImplDrawScrollButtons(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // Left scroll button
    mpData->aScrollBackRect = tools::Rectangle(0, 0, mnScrollButtonWidth, mnTabHeight);
    rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
    rRenderContext.DrawRect(mpData->aScrollBackRect);

    // Draw left arrow
    Point aCenter = mpData->aScrollBackRect.Center();
    rRenderContext.SetLineColor(rStyleSettings.GetButtonTextColor());
    rRenderContext.DrawLine(Point(aCenter.X() - 3, aCenter.Y()),
                           Point(aCenter.X() + 3, aCenter.Y() - 3));
    rRenderContext.DrawLine(Point(aCenter.X() - 3, aCenter.Y()),
                           Point(aCenter.X() + 3, aCenter.Y() + 3));

    // Right scroll button
    tools::Long nRightX = GetSizePixel().Width() - mnScrollButtonWidth;
    mpData->aScrollForwardRect = tools::Rectangle(nRightX, 0, GetSizePixel().Width(), mnTabHeight);
    rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
    rRenderContext.DrawRect(mpData->aScrollForwardRect);

    // Draw right arrow
    aCenter = mpData->aScrollForwardRect.Center();
    rRenderContext.DrawLine(Point(aCenter.X() + 3, aCenter.Y()),
                           Point(aCenter.X() - 3, aCenter.Y() - 3));
    rRenderContext.DrawLine(Point(aCenter.X() + 3, aCenter.Y()),
                           Point(aCenter.X() - 3, aCenter.Y() + 3));
}

tools::Long DocumentTabBar::ImplCalculateTabWidth(const TabInfo& rTab) const
{
    // Base calculation on text width
    tools::Long nTextWidth = GetTextWidth(rTab.sTitle);
    if (rTab.bModified)
        nTextWidth += GetTextWidth("*");

    tools::Long nTabWidth = nTextWidth + 16; // padding

    // Add icon width
    if (mbShowIcons && !rTab.aIcon.operator!())
        nTabWidth += rTab.aIcon.GetSizePixel().Width() + 4;

    // Add close button width
    if (mbShowCloseButtons)
        nTabWidth += CLOSE_BUTTON_SIZE + 4;

    // Apply constraints
    nTabWidth = std::max(nTabWidth, mnTabMinWidth);
    nTabWidth = std::min(nTabWidth, mnTabMaxWidth);

    return nTabWidth;
}

tools::Long DocumentTabBar::ImplGetAvailableWidth() const
{
    tools::Long nWidth = GetSizePixel().Width();
    if (ImplNeedsScrolling())
        nWidth -= 2 * mnScrollButtonWidth;
    return nWidth;
}

bool DocumentTabBar::ImplNeedsScrolling() const
{
    if (!mbScrollable || mpData->aTabs.empty())
        return false;

    tools::Long nTotalWidth = 0;
    for (const auto& rTab : mpData->aTabs)
    {
        nTotalWidth += ImplCalculateTabWidth(rTab) + TAB_SPACING;
    }

    return nTotalWidth > GetSizePixel().Width();
}

DocumentTabBar::TabInfo* DocumentTabBar::ImplFindTab(sal_uInt16 nId)
{
    auto it = std::find_if(mpData->aTabs.begin(), mpData->aTabs.end(),
                          [nId](const TabInfo& tab) { return tab.nId == nId; });
    return (it != mpData->aTabs.end()) ? &(*it) : nullptr;
}

const DocumentTabBar::TabInfo* DocumentTabBar::ImplFindTab(sal_uInt16 nId) const
{
    auto it = std::find_if(mpData->aTabs.begin(), mpData->aTabs.end(),
                          [nId](const TabInfo& tab) { return tab.nId == nId; });
    return (it != mpData->aTabs.end()) ? &(*it) : nullptr;
}

void DocumentTabBar::Resize()
{
    Control::Resize();
    ImplUpdateLayout();
}

void DocumentTabBar::ImplUpdateLayout()
{
    // Implementation for layout updates
    Invalidate();
}

void DocumentTabBar::MouseButtonDown(const MouseEvent& rMEvt)
{
    std::cout << "DEBUG: MouseButtonDown called at (" << rMEvt.GetPosPixel().X() << "," << rMEvt.GetPosPixel().Y() << ")" << std::endl;

    if (rMEvt.IsLeft())
    {
        sal_uInt16 nTabId;
        if (ImplIsCloseButtonAt(rMEvt.GetPosPixel(), nTabId))
        {
            std::cout << "DEBUG: Close button clicked for tab " << nTabId << std::endl;
            ImplCloseTab(nTabId);
            return;
        }

        sal_uInt16 nClickedTab = ImplGetTabAt(rMEvt.GetPosPixel());
        std::cout << "DEBUG: ImplGetTabAt returned tab ID: " << nClickedTab << std::endl;
        if (nClickedTab != 0)
        {
            std::cout << "DEBUG: Calling ImplActivateTab for tab " << nClickedTab << std::endl;
            ImplActivateTab(nClickedTab);
            return;
        }

        // Check scroll buttons
        if (mpData->aScrollBackRect.Contains(rMEvt.GetPosPixel()))
        {
            std::cout << "DEBUG: Scroll back clicked" << std::endl;
            ImplScrollTabs(false);
            return;
        }

        if (mpData->aScrollForwardRect.Contains(rMEvt.GetPosPixel()))
        {
            std::cout << "DEBUG: Scroll forward clicked" << std::endl;
            ImplScrollTabs(true);
            return;
        }

        std::cout << "DEBUG: No tab or button clicked" << std::endl;
    }

    Control::MouseButtonDown(rMEvt);
}

void DocumentTabBar::MouseButtonUp(const MouseEvent& rMEvt)
{
    Control::MouseButtonUp(rMEvt);
}

void DocumentTabBar::MouseMove(const MouseEvent& rMEvt)
{
    sal_uInt16 nHoverTab = ImplGetTabAt(rMEvt.GetPosPixel());
    if (nHoverTab != mpData->nHoverTab)
    {
        mpData->nHoverTab = nHoverTab;
        Invalidate();
    }

    Control::MouseMove(rMEvt);
}

void DocumentTabBar::KeyInput(const KeyEvent& rKEvt)
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = rKeyCode.GetCode();

    if (rKeyCode.IsMod1()) // Ctrl key
    {
        switch (nCode)
        {
            case KEY_TAB:
                // Ctrl+Tab: Next tab
                if (rKeyCode.IsShift())
                    ImplActivatePreviousTab();
                else
                    ImplActivateNextTab();
                return;

            case KEY_W:
                // Ctrl+W: Close current tab
                if (mnActiveTab != 0)
                    ImplCloseTab(mnActiveTab);
                return;
        }
    }

    Control::KeyInput(rKEvt);
}

void DocumentTabBar::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        Point aPos = rCEvt.GetMousePosPixel();
        sal_uInt16 nTabId = ImplGetTabAt(aPos);
        if (nTabId != 0)
        {
            ImplShowTabMenu(aPos);
            return;
        }
    }

    Control::Command(rCEvt);
}

Size DocumentTabBar::GetOptimalSize() const
{
    return Size(100, mnTabHeight);
}

void DocumentTabBar::ImplActivateTab(sal_uInt16 nId)
{
    SetActiveTab(nId);
}

void DocumentTabBar::ImplCloseTab(sal_uInt16 nId)
{
    RemoveTab(nId);
}

void DocumentTabBar::ImplShowTabMenu(const Point& /*rPos*/)
{
    maTabMenuHdl.Call(this);
}

void DocumentTabBar::ImplScrollTabs(bool bForward)
{
    if (bForward && mnScrollOffset < mpData->aTabs.size() - 1)
        mnScrollOffset++;
    else if (!bForward && mnScrollOffset > 0)
        mnScrollOffset--;

    Invalidate();
}

void DocumentTabBar::ImplActivateNextTab()
{
    if (mpData->aTabs.empty())
        return;

    auto it = std::find_if(mpData->aTabs.begin(), mpData->aTabs.end(),
                          [this](const TabInfo& tab) { return tab.nId == mnActiveTab; });

    if (it != mpData->aTabs.end())
    {
        ++it;
        if (it == mpData->aTabs.end())
            it = mpData->aTabs.begin();
        ImplActivateTab(it->nId);
    }
}

void DocumentTabBar::ImplActivatePreviousTab()
{
    if (mpData->aTabs.empty())
        return;

    auto it = std::find_if(mpData->aTabs.begin(), mpData->aTabs.end(),
                          [this](const TabInfo& tab) { return tab.nId == mnActiveTab; });

    if (it != mpData->aTabs.end())
    {
        if (it == mpData->aTabs.begin())
            it = mpData->aTabs.end() - 1;
        else
            --it;
        ImplActivateTab(it->nId);
    }
}

sal_uInt16 DocumentTabBar::ImplGetTabAt(const Point& rPos) const
{
    if (mpData->aTabs.empty() || rPos.Y() < 0 || rPos.Y() >= mnTabHeight)
        return 0;

    // Calculate visible tabs and their positions
    tools::Long nStartX = ImplNeedsScrolling() ? mnScrollButtonWidth : 0;
    tools::Long nAvailableWidth = ImplGetAvailableWidth();
    tools::Long nCurrentX = nStartX;

    // Check each visible tab
    for (size_t i = mnScrollOffset; i < mpData->aTabs.size() && nCurrentX < nAvailableWidth; ++i)
    {
        const TabInfo& rTab = mpData->aTabs[i];
        tools::Long nTabWidth = ImplCalculateTabWidth(rTab);

        if (nCurrentX + nTabWidth > nAvailableWidth)
            break;

        // Check if click is within this tab's bounds
        if (rPos.X() >= nCurrentX && rPos.X() < nCurrentX + nTabWidth)
            return rTab.nId;

        nCurrentX += nTabWidth + TAB_SPACING;
    }

    return 0;
}

tools::Rectangle DocumentTabBar::ImplGetTabRect(sal_uInt16 /*nId*/) const
{
    // Placeholder implementation
    return tools::Rectangle();
}

tools::Rectangle DocumentTabBar::ImplGetTabCloseRect(sal_uInt16 /*nId*/) const
{
    // Placeholder implementation
    return tools::Rectangle();
}

bool DocumentTabBar::ImplIsCloseButtonAt(const Point& /*rPos*/, sal_uInt16& rTabId) const
{
    // Placeholder implementation
    rTabId = 0;
    return false;
}

void DocumentTabBar::ImplEnsureTabVisible(sal_uInt16 /*nId*/)
{
    // Placeholder implementation for scrolling to make tab visible
}

void DocumentTabBar::StateChanged(StateChangedType nType)
{
    Control::StateChanged(nType);

    if (nType == StateChangedType::InitShow ||
        nType == StateChangedType::Zoom ||
        nType == StateChangedType::ControlFont ||
        nType == StateChangedType::Style)
    {
        ImplInitSettings();
        Invalidate();
    }
}

void DocumentTabBar::DataChanged(const DataChangedEvent& rDCEvt)
{
    Control::DataChanged(rDCEvt);

    if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        ImplInitSettings();
        Invalidate();
    }
}

void DocumentTabBar::GetFocus()
{
    Control::GetFocus();
    Invalidate();
}

void DocumentTabBar::LoseFocus()
{
    Control::LoseFocus();
    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */