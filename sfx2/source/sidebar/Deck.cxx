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
#include <sfx2/sidebar/DeckDescriptor.hxx>
#include <sfx2/sidebar/DeckLayouter.hxx>
#include <sfx2/sidebar/DrawHelper.hxx>
#include <sfx2/sidebar/DeckTitleBar.hxx>
#include <sfx2/sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/Paint.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/Theme.hxx>

#include <vcl/event.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/commandevent.hxx>
#include <tools/svborder.hxx>
#include <sal/log.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

Deck::Deck(const DeckDescriptor& rDeckDescriptor, vcl::Window* pParentWindow,
           const std::function<void()>& rCloserAction)
    : Window(pParentWindow, 0)
    , msId(rDeckDescriptor.msId)
    , mnMinimalWidth(0)
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
                           Theme::GetPaint(Theme::Paint_DeckBackground),
                           Theme::GetPaint(Theme::Paint_DeckBackground));

    // Paint the border.
    const int nBorderSize(Theme::GetInteger(Theme::Int_DeckBorderSize));
    aBox.AdjustLeft(aPadding.Left() );
    aBox.AdjustTop(aPadding.Top() );
    aBox.AdjustRight( -(aPadding.Right()) );
    aBox.AdjustBottom( -(aPadding.Bottom()) );
    const sfx2::sidebar::Paint& rHorizontalBorderPaint(Theme::GetPaint(Theme::Paint_HorizontalBorder));
    DrawHelper::DrawBorder(rRenderContext, aBox,
                           SvBorder(nBorderSize, nBorderSize, nBorderSize, nBorderSize),
                           rHorizontalBorderPaint,
                           Theme::GetPaint(Theme::Paint_VerticalBorder));
}

void Deck::DataChanged (const DataChangedEvent&)
{
    RequestLayout();
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
    long nDelta = pData->GetDelta();
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
    // First dispose old panels we no longer need.
    for (VclPtr<Panel> & rpPanel : maPanels)
    {
        bool bFound = false;
        for (const auto & i : rPanelContainer)
            bFound = bFound || (rpPanel.get() == i.get());
        if (!bFound) // this one didn't survive.
            rpPanel.disposeAndClear();
    }
    maPanels = rPanelContainer;

    RequestLayout();
}

void Deck::RequestLayout()
{
    mnMinimalWidth = 0;

    DeckLayouter::LayoutDeck(GetContentArea(), mnMinimalWidth, maPanels,
                             *GetTitleBar(), *mpScrollClipWindow, *mpScrollContainer,
                             *mpFiller, *mpVerticalScrollBar);
}

vcl::Window* Deck::GetPanelParentWindow()
{
    return mpScrollContainer.get();
}

Panel* Deck::GetPanel(const OUString & panelId)
{
    for (VclPtr<Panel> & pPanel : maPanels)
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

static const OUString GetWindowClassification(const vcl::Window* pWindow)
{
    const OUString& rsName (pWindow->GetText());
    if (!rsName.isEmpty())
    {
        return rsName;
    }
    else
    {
        return OUString("window");
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
    const sfx2::sidebar::Paint& rHorizontalBorderPaint(Theme::GetPaint(Theme::Paint_HorizontalBorder));
    for (auto const& separator : maSeparators)
    {
        DrawHelper::DrawHorizontalLine(rRenderContext, nLeft, nRight, separator,
                                       nSeparatorHeight, rHorizontalBorderPaint);
    }
}

void Deck::ScrollContainerWindow::SetSeparators (const ::std::vector<sal_Int32>& rSeparators)
{
    maSeparators = rSeparators;
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
