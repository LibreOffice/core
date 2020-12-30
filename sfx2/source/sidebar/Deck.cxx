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

#include <sfx2/sidebar/Deck.hxx>
#include <sidebar/DeckDescriptor.hxx>
#include <sidebar/DeckLayouter.hxx>
#include <sidebar/DrawHelper.hxx>
#include <sidebar/DeckTitleBar.hxx>
#include <sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/viewsh.hxx>

#include <vcl/event.hxx>
#include <comphelper/lok.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/IDialogRenderable.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/svborder.hxx>
#include <tools/json_writer.hxx>
#include <sal/log.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

Deck::Deck(const DeckDescriptor& rDeckDescriptor, vcl::Window* pParentWindow,
           const std::function<void()>& rCloserAction)
    : Window(pParentWindow, 0)
    , msId(rDeckDescriptor.msId)
    , mnMinimalWidth(0)
    , mnMinimalHeight(0)
    , maPanels()
    , mpTitleBar(VclPtr<DeckTitleBar>::Create(rDeckDescriptor.msTitle, this, rCloserAction))
    , mpScrollClipWindow(VclPtr<vcl::Window>::Create(this))
    , mpScrollContainer(VclPtr<ScrollContainerWindow>::Create(mpScrollClipWindow.get()))
    , mpFiller(VclPtr<vcl::Window>::Create(this))
    , mpVerticalScrollBar(VclPtr<ScrollBar>::Create(this))
{
    mpScrollClipWindow->SetBackground(Wallpaper());
    mpScrollClipWindow->Show();

    mpScrollContainer->SetStyle(mpScrollContainer->GetStyle() | WB_DIALOGCONTROL);
    mpScrollContainer->SetBackground(Wallpaper());
    mpScrollContainer->Show();

    mpVerticalScrollBar->SetScrollHdl(LINK(this, Deck, HandleVerticalScrollBarChange));
    mpVerticalScrollBar->SetLineSize(10);
    mpVerticalScrollBar->SetPageSize(100);

#ifdef DEBUG
    SetText(OUString("Deck"));
    mpScrollClipWindow->SetText(OUString("ScrollClipWindow"));
    mpFiller->SetText(OUString("Filler"));
    mpVerticalScrollBar->SetText(OUString("VerticalScrollBar"));
#endif
}

Deck::~Deck()
{
    disposeOnce();
}

void Deck::dispose()
{
    SharedPanelContainer aPanels;
    aPanels.swap(maPanels);

    // We have to explicitly trigger the destruction of panels.
    // Otherwise that is done by one of our base class destructors
    // without updating maPanels.
    for (VclPtr<Panel> & rpPanel : aPanels)
        rpPanel.disposeAndClear();

    maPanels.clear(); // just to keep the loplugin:vclwidgets happy
    mpTitleBar.disposeAndClear();
    mpFiller.disposeAndClear();
    mpVerticalScrollBar.disposeAndClear();
    mpScrollContainer.disposeAndClear();
    mpScrollClipWindow.disposeAndClear();

    vcl::Window::dispose();
}

VclPtr<DeckTitleBar> const & Deck::GetTitleBar() const
{
    return mpTitleBar;
}

tools::Rectangle Deck::GetContentArea() const
{
    const Size aWindowSize (GetSizePixel());
    const int nBorderSize (Theme::GetInteger(Theme::Int_DeckBorderSize));
    if (aWindowSize.IsEmpty())
        return tools::Rectangle();

    return tools::Rectangle(
        Theme::GetInteger(Theme::Int_DeckLeftPadding) + nBorderSize,
        Theme::GetInteger(Theme::Int_DeckTopPadding) + nBorderSize,
        aWindowSize.Width() - 1 - Theme::GetInteger(Theme::Int_DeckRightPadding) - nBorderSize,
        aWindowSize.Height() - 1 - Theme::GetInteger(Theme::Int_DeckBottomPadding) - nBorderSize);
}

void Deck::ApplySettings(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetBackground(Wallpaper());
}

void Deck::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rUpdateArea*/)
{
    const Size aWindowSize (GetSizePixel());
    const SvBorder aPadding(Theme::GetInteger(Theme::Int_DeckLeftPadding),
                            Theme::GetInteger(Theme::Int_DeckTopPadding),
                            Theme::GetInteger(Theme::Int_DeckRightPadding),
                            Theme::GetInteger(Theme::Int_DeckBottomPadding));

    // Paint deck background outside the border.
    tools::Rectangle aBox(0, 0, aWindowSize.Width() - 1, aWindowSize.Height() - 1);
    DrawHelper::DrawBorder(rRenderContext, aBox, aPadding,
                           Theme::GetColor(Theme::Color_DeckBackground),
                           Theme::GetColor(Theme::Color_DeckBackground));

    // Paint the border.
    const int nBorderSize(Theme::GetInteger(Theme::Int_DeckBorderSize));
    aBox.AdjustLeft(aPadding.Left() );
    aBox.AdjustTop(aPadding.Top() );
    aBox.AdjustRight( -(aPadding.Right()) );
    aBox.AdjustBottom( -(aPadding.Bottom()) );
    const Color nHorizontalBorderPaint(Theme::GetColor(Theme::Color_HorizontalBorder));
    DrawHelper::DrawBorder(rRenderContext, aBox,
                           SvBorder(nBorderSize, nBorderSize, nBorderSize, nBorderSize),
                           nHorizontalBorderPaint,
                           Theme::GetColor(Theme::Color_VerticalBorder));
}

void Deck::DataChanged (const DataChangedEvent&)
{
    RequestLayoutInternal();
}

bool Deck::EventNotify(NotifyEvent& rEvent)
{
    if (rEvent.GetType() == MouseNotifyEvent::COMMAND)
    {
        CommandEvent* pCommandEvent = static_cast<CommandEvent*>(rEvent.GetData());
        if (pCommandEvent != nullptr)
            switch (pCommandEvent->GetCommand())
            {
                case CommandEventId::Wheel:
                    return ProcessWheelEvent(pCommandEvent);

                default:
                    break;
            }
    }

    return Window::EventNotify(rEvent);
}

void Deck::Resize()
{
    Window::Resize();

    if (const vcl::ILibreOfficeKitNotifier* pNotifier = GetLOKNotifier())
    {
        std::vector<vcl::LOKPayloadItem> aItems;
        aItems.emplace_back("type", "deck");
        aItems.emplace_back(std::make_pair("position", Point(GetOutOffXPixel(), GetOutOffYPixel()).toString()));
        aItems.emplace_back(std::make_pair("size", GetSizePixel().toString()));
        pNotifier->notifyWindow(GetLOKWindowId(), "size_changed", aItems);
    }
}

/*
 * Get the ordering as is shown in the layout, and our type as 'deck'
 * also elide nested panel windows.
 */
void Deck::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    rJsonWriter.put("id", get_id());  // TODO could be missing - sort out
    rJsonWriter.put("type", "deck");
    rJsonWriter.put("text", GetText());
    rJsonWriter.put("enabled", IsEnabled());

    auto childrenNode = rJsonWriter.startArray("children");
    for (auto &it : maPanels)
    {
        if (it->IsLurking())
            continue;

        // collapse the panel itself out
        auto xContent = it->GetElementWindow();
        if (!xContent.is())
            continue;
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xContent);
        if (!pWindow)
            continue;

        auto childNode = rJsonWriter.startStruct();
        rJsonWriter.put("id", it->GetId());
        rJsonWriter.put("type", "panel");
        rJsonWriter.put("text", it->GetText());
        rJsonWriter.put("enabled", it->IsEnabled());

        {
            auto children2Node = rJsonWriter.startArray("children");
            {
                auto child2Node = rJsonWriter.startStruct();
                pWindow->DumpAsPropertyTree(rJsonWriter);
            }
        }
    }
}

bool Deck::ProcessWheelEvent(CommandEvent const * pCommandEvent)
{
    if ( ! mpVerticalScrollBar)
        return false;
    if ( ! mpVerticalScrollBar->IsVisible())
        return false;

    // Get the wheel data and check that it describes a valid vertical
    // scroll.
    const CommandWheelData* pData = pCommandEvent->GetWheelData();
    if (pData==nullptr
        || pData->GetModifier()
        || pData->GetMode() != CommandWheelMode::SCROLL
        || pData->IsHorz())
        return false;

    // Execute the actual scroll action.
    tools::Long nDelta = pData->GetDelta();
    mpVerticalScrollBar->DoScroll(
        mpVerticalScrollBar->GetThumbPos() - nDelta);
    return true;
}

/**
 * This container may contain existing panels that are
 * being re-used, and new ones too.
 */
void Deck::ResetPanels(const SharedPanelContainer& rPanelContainer)
{
    SharedPanelContainer aHiddens;

    // First hide old panels we don't need just now.
    for (VclPtr<Panel> & rpPanel : maPanels)
    {
        bool bFound = false;
        for (const auto & i : rPanelContainer)
            bFound = bFound || (rpPanel.get() == i.get());
        if (!bFound) // this one didn't survive.
        {
            rpPanel->SetLurkMode(true);
            aHiddens.push_back(rpPanel);
        }
    }
    maPanels = rPanelContainer;

    // Hidden ones always at the end
    maPanels.insert(std::end(maPanels), std::begin(aHiddens), std::end(aHiddens));

    RequestLayoutInternal();
}

void Deck::RequestLayoutInternal()
{
    mnMinimalWidth = 0;
    mnMinimalHeight = 0;

    DeckLayouter::LayoutDeck(GetContentArea(), mnMinimalWidth, mnMinimalHeight, maPanels,
                             *GetTitleBar(), *mpScrollClipWindow, *mpScrollContainer,
                             *mpFiller, *mpVerticalScrollBar);
}

void Deck::RequestLayout()
{
    RequestLayoutInternal();

    if (!comphelper::LibreOfficeKit::isActive())
        return;

    bool bChangeNeeded = false;
    Size aParentSize = GetParent()->GetSizePixel();

    if (mnMinimalHeight > 0 && (mnMinimalHeight != aParentSize.Height() || GetSizePixel().Height() != mnMinimalHeight))
    {
        aParentSize.setHeight(mnMinimalHeight);
        bChangeNeeded = true;
    }
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    if (mnMinimalWidth > 0 && (mnMinimalWidth != aParentSize.Width() || GetSizePixel().Width() != mnMinimalWidth)
            && pViewShell && pViewShell->isLOKMobilePhone())
    {
        aParentSize.setWidth(mnMinimalWidth);
        bChangeNeeded = true;
    }

    if (bChangeNeeded)
    {
        GetParent()->SetSizePixel(aParentSize);
        setPosSizePixel(0, 0, aParentSize.Width(), aParentSize.Height());
    }
    else if (aParentSize != GetSizePixel()) //Sync parent & child sizes
        setPosSizePixel(0, 0, aParentSize.Width(), aParentSize.Height());
}

vcl::Window* Deck::GetPanelParentWindow()
{
    return mpScrollContainer.get();
}

Panel* Deck::GetPanel(std::u16string_view panelId)
{
    for (const VclPtr<Panel> & pPanel : maPanels)
    {
        if(pPanel->GetId() == panelId)
        {
            return pPanel.get();
        }
    }
    return nullptr;

}

void Deck::ShowPanel(const Panel& rPanel)
{
    if (!mpVerticalScrollBar || !mpVerticalScrollBar->IsVisible())
        return;

    // Get vertical extent of the panel.
    sal_Int32 nPanelTop (rPanel.GetPosPixel().Y());
    const sal_Int32 nPanelBottom (nPanelTop + rPanel.GetSizePixel().Height() - 1);
    // Add the title bar into the extent.
    if (rPanel.GetTitleBar() && rPanel.GetTitleBar()->IsVisible())
        nPanelTop = rPanel.GetTitleBar()->GetPosPixel().Y();

    // Determine what the new thumb position should be like.
    // When the whole panel does not fit then make its top visible
    // and it off at the bottom.
    sal_Int32 nNewThumbPos (mpVerticalScrollBar->GetThumbPos());
    if (nPanelBottom >= nNewThumbPos+mpVerticalScrollBar->GetVisibleSize())
        nNewThumbPos = nPanelBottom - mpVerticalScrollBar->GetVisibleSize();
    if (nPanelTop < nNewThumbPos)
        nNewThumbPos = nPanelTop;

    mpVerticalScrollBar->SetThumbPos(nNewThumbPos);
    mpScrollContainer->SetPosPixel(
        Point(
            mpScrollContainer->GetPosPixel().X(),
            -nNewThumbPos));
}

static OUString GetWindowClassification(const vcl::Window* pWindow)
{
    const OUString& rsName (pWindow->GetText());
    if (!rsName.isEmpty())
    {
        return rsName;
    }
    else
    {
        return "window";
    }
}

void Deck::PrintWindowSubTree(vcl::Window* pRoot, int nIndentation)
{
    static const char* const sIndentation = "                                                                  ";
    const Point aLocation (pRoot->GetPosPixel());
    const Size aSize (pRoot->GetSizePixel());
    SAL_INFO(
        "sfx.sidebar",
        sIndentation + strlen(sIndentation) - nIndentation * 4 << pRoot << " "
            << GetWindowClassification(pRoot) << " "
            << (pRoot->IsVisible() ? "visible" : "hidden") << " +"
            << aLocation.X() << "+" << aLocation.Y() << " x" << aSize.Width()
            << "x" << aSize.Height());

    const sal_uInt16 nChildCount(pRoot->GetChildCount());
    for (sal_uInt16 nIndex = 0; nIndex < nChildCount; ++nIndex)
        PrintWindowSubTree(pRoot->GetChild(nIndex), nIndentation + 1);
}

IMPL_LINK_NOARG(Deck, HandleVerticalScrollBarChange, ScrollBar*, void)
{
    const sal_Int32 nYOffset (-mpVerticalScrollBar->GetThumbPos());
    mpScrollContainer->SetPosPixel(Point(mpScrollContainer->GetPosPixel().X(),
                                         nYOffset));
    mpScrollContainer->Invalidate();
}

//----- Deck::ScrollContainerWindow -------------------------------------------

Deck::ScrollContainerWindow::ScrollContainerWindow (vcl::Window* pParentWindow)
    : Window(pParentWindow),
      maSeparators()
{
#ifdef DEBUG
    SetText(OUString("ScrollContainerWindow"));
#endif
}

void Deck::ScrollContainerWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rUpdateArea*/)
{
    // Paint the separators.
    const sal_Int32 nSeparatorHeight(Theme::GetInteger(Theme::Int_DeckSeparatorHeight));
    const sal_Int32 nLeft(0);
    const sal_Int32 nRight(GetSizePixel().Width() - 1);
    const Color nHorizontalBorderPaint(Theme::GetColor(Theme::Color_HorizontalBorder));
    for (auto const& separator : maSeparators)
    {
        DrawHelper::DrawHorizontalLine(rRenderContext, nLeft, nRight, separator,
                                       nSeparatorHeight, nHorizontalBorderPaint);
    }
}

void Deck::ScrollContainerWindow::SetSeparators (const ::std::vector<sal_Int32>& rSeparators)
{
    maSeparators = rSeparators;
    Invalidate();
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
