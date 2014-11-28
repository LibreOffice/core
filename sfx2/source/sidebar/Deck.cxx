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

#include "Deck.hxx"
#include "DeckDescriptor.hxx"
#include "DeckLayouter.hxx"
#include "DrawHelper.hxx"
#include "DeckTitleBar.hxx"
#include "PanelTitleBar.hxx"
#include "Paint.hxx"
#include "Panel.hxx"
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/Theme.hxx>

#include <vcl/dockwin.hxx>
#include <vcl/scrbar.hxx>
#include <tools/svborder.hxx>

#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;


namespace sfx2 { namespace sidebar {

Deck::Deck (
    const DeckDescriptor& rDeckDescriptor,
    vcl::Window* pParentWindow,
    const ::boost::function<void(void)>& rCloserAction)
    : Window(pParentWindow, 0),
      msId(rDeckDescriptor.msId),
      maIcon(),
      mnMinimalWidth(0),
      maPanels(),
      mpTitleBar(new DeckTitleBar(rDeckDescriptor.msTitle, this, rCloserAction)),
      mpScrollClipWindow(new vcl::Window(this)),
      mpScrollContainer(new ScrollContainerWindow(mpScrollClipWindow.get())),
      mpFiller(new vcl::Window(this)),
      mpVerticalScrollBar(new ScrollBar(this))
{
    SetBackground(Wallpaper());

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




Deck::~Deck (void)
{
    Dispose();

    // We have to explicitly trigger the destruction of panels.
    // Otherwise that is done by one of our base class destructors
    // without updating maPanels.
    maPanels.clear();
}




void Deck::Dispose (void)
{
    SharedPanelContainer aPanels;
    aPanels.swap(maPanels);
    for (SharedPanelContainer::iterator
             iPanel(aPanels.begin()),
             iEnd(aPanels.end());
         iPanel!=iEnd;
         ++iPanel)
    {
        if (*iPanel)
        {
            (*iPanel)->Dispose();
            OSL_ASSERT(iPanel->unique());
            iPanel->reset();
        }
    }

    mpTitleBar.reset();
    mpFiller.reset();
    mpVerticalScrollBar.reset();
}








DeckTitleBar* Deck::GetTitleBar (void) const
{
    return mpTitleBar.get();
}




Rectangle Deck::GetContentArea (void) const
{
    const Size aWindowSize (GetSizePixel());
    const int nBorderSize (Theme::GetInteger(Theme::Int_DeckBorderSize));

    return Rectangle(
        Theme::GetInteger(Theme::Int_DeckLeftPadding) + nBorderSize,
        Theme::GetInteger(Theme::Int_DeckTopPadding) + nBorderSize,
        aWindowSize.Width() - 1 - Theme::GetInteger(Theme::Int_DeckRightPadding) - nBorderSize,
        aWindowSize.Height() - 1 - Theme::GetInteger(Theme::Int_DeckBottomPadding) - nBorderSize);
}

void Deck::Paint (const Rectangle& rUpdateArea)
{
    (void) rUpdateArea;

    const Size aWindowSize (GetSizePixel());
    const SvBorder aPadding (
            Theme::GetInteger(Theme::Int_DeckLeftPadding),
            Theme::GetInteger(Theme::Int_DeckTopPadding),
            Theme::GetInteger(Theme::Int_DeckRightPadding),
            Theme::GetInteger(Theme::Int_DeckBottomPadding));

    // Paint deck background outside the border.
    Rectangle aBox(
        0,
        0,
        aWindowSize.Width() - 1,
        aWindowSize.Height() - 1);
    DrawHelper::DrawBorder(
        *this,
        aBox,
        aPadding,
        Theme::GetPaint(Theme::Paint_DeckBackground),
        Theme::GetPaint(Theme::Paint_DeckBackground));

    // Paint the border.
    const int nBorderSize (Theme::GetInteger(Theme::Int_DeckBorderSize));
    aBox.Left() += aPadding.Left();
    aBox.Top() += aPadding.Top();
    aBox.Right() -= aPadding.Right();
    aBox.Bottom() -= aPadding.Bottom();
    const sfx2::sidebar::Paint& rHorizontalBorderPaint (Theme::GetPaint(Theme::Paint_HorizontalBorder));
    DrawHelper::DrawBorder(
        *this,
        aBox,
        SvBorder(nBorderSize, nBorderSize, nBorderSize, nBorderSize),
        rHorizontalBorderPaint,
        Theme::GetPaint(Theme::Paint_VerticalBorder));
}




void Deck::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;
    RequestLayout();
}

bool Deck::Notify (NotifyEvent& rEvent)
{
    if (rEvent.GetType() == MouseNotifyEvent::COMMAND)
    {
        CommandEvent* pCommandEvent = reinterpret_cast<CommandEvent*>(rEvent.GetData());
        if (pCommandEvent != NULL)
            switch (pCommandEvent->GetCommand())
            {
                case COMMAND_WHEEL:
                    return ProcessWheelEvent(pCommandEvent);

                default:
                    break;
            }
    }

    return Window::Notify(rEvent);
}

bool Deck::ProcessWheelEvent(CommandEvent* pCommandEvent)
{
    if ( ! mpVerticalScrollBar)
        return false;
    if ( ! mpVerticalScrollBar->IsVisible())
        return false;

    // Get the wheel data and check that it describes a valid vertical
    // scroll.
    const CommandWheelData* pData = pCommandEvent->GetWheelData();
    if (pData==NULL
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

void Deck::SetPanels (const SharedPanelContainer& rPanels)
{
    maPanels = rPanels;

    RequestLayout();
}

void Deck::RequestLayout (void)
{
    mnMinimalWidth = 0;

    DeckLayouter::LayoutDeck(
        GetContentArea(),
        mnMinimalWidth,
        maPanels,
        *GetTitleBar(),
        *mpScrollClipWindow,
        *mpScrollContainer,
        *mpFiller,
        *mpVerticalScrollBar);
}

vcl::Window* Deck::GetPanelParentWindow (void)
{
    return mpScrollContainer.get();
}

void Deck::ShowPanel (const Panel& rPanel)
{
    if (mpVerticalScrollBar && mpVerticalScrollBar->IsVisible())
    {
        // Get vertical extent of the panel.
        sal_Int32 nPanelTop (rPanel.GetPosPixel().Y());
        const sal_Int32 nPanelBottom (nPanelTop + rPanel.GetSizePixel().Height() - 1);
        // Add the title bar into the extent.
        if (rPanel.GetTitleBar() != NULL && rPanel.GetTitleBar()->IsVisible())
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
}




const OUString GetWindowClassification (const vcl::Window* pWindow)
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


void Deck::PrintWindowSubTree (vcl::Window* pRoot, int nIndentation)
{
    static const char* sIndentation = "                                                                  ";
    const Point aLocation (pRoot->GetPosPixel());
    const Size aSize (pRoot->GetSizePixel());
    OUString sClassification = GetWindowClassification(pRoot);
    const char* sVisible = pRoot->IsVisible() ? "visible" : "hidden";
    OSL_TRACE("%s%x %s %s +%d+%d x%dx%d",
        sIndentation+strlen(sIndentation)-nIndentation*4,
        pRoot,
        OUStringToOString(sClassification, RTL_TEXTENCODING_ASCII_US).getStr(),
        sVisible,
        aLocation.X(),aLocation.Y(),
        aSize.Width(),aSize.Height());

    const sal_uInt16 nChildCount (pRoot->GetChildCount());
    for (sal_uInt16 nIndex=0; nIndex<nChildCount; ++nIndex)
        PrintWindowSubTree(pRoot->GetChild(nIndex), nIndentation+1);
}

void Deck::PrintWindowTree (void)
{
    PrintWindowSubTree(this, 0);
}

IMPL_LINK(Deck, HandleVerticalScrollBarChange,void*, EMPTYARG)
{
    const sal_Int32 nYOffset (-mpVerticalScrollBar->GetThumbPos());
    mpScrollContainer->SetPosPixel(
        Point(
            mpScrollContainer->GetPosPixel().X(),
            nYOffset));
    return sal_True;
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




Deck::ScrollContainerWindow::~ScrollContainerWindow (void)
{
}




void Deck::ScrollContainerWindow::Paint (const Rectangle& rUpdateArea)
{
    (void)rUpdateArea;

    // Paint the separators.
    const sal_Int32 nSeparatorHeight (Theme::GetInteger(Theme::Int_DeckSeparatorHeight));
    const sal_Int32 nLeft  (0);
    const sal_Int32 nRight (GetSizePixel().Width()-1);
    const sfx2::sidebar::Paint& rHorizontalBorderPaint (Theme::GetPaint(Theme::Paint_HorizontalBorder));
    for (::std::vector<sal_Int32>::const_iterator iY(maSeparators.begin()), iEnd(maSeparators.end());
         iY!=iEnd;
         ++iY)
    {
        DrawHelper::DrawHorizontalLine(
            *this,
            nLeft,
            nRight,
            *iY,
            nSeparatorHeight,
            rHorizontalBorderPaint);
    }
}




void Deck::ScrollContainerWindow::SetSeparators (const ::std::vector<sal_Int32>& rSeparators)
{
    maSeparators = rSeparators;
}


} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
