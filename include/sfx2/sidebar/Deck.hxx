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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_DECK_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_DECK_HXX

#include <sfx2/sidebar/Panel.hxx>

#include <vcl/window.hxx>
#include <vcl/image.hxx>
#include <com/sun/star/ui/LayoutSize.hpp>

class ScrollBar;

namespace sfx2 { namespace sidebar {

class DeckDescriptor;
class DeckTitleBar;

/** This is the parent window of the panels.
    It displays the deck title.
*/
class Deck : public vcl::Window
{
public:
    Deck(const DeckDescriptor& rDeckDescriptor,
         vcl::Window* pParentWindow,
         const std::function<void()>& rCloserAction);
    virtual ~Deck() override;
    virtual void dispose() override;

    const OUString& GetId() const { return msId; }

    VclPtr<DeckTitleBar> GetTitleBar() const;
    Rectangle GetContentArea() const;
    void ResetPanels (const SharedPanelContainer& rPanels);
    const SharedPanelContainer& GetPanels() const  { return maPanels; }

    Panel* GetPanel(OUString const & panelId);

    void RequestLayout();
    vcl::Window* GetPanelParentWindow();

    /** Try to make the panel completely visible.
        When the whole panel does not fit then make its top visible
        and it off at the bottom.
    */
    void ShowPanel (const Panel& rPanel);

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rUpdateArea) override;
    virtual void DataChanged (const DataChangedEvent& rEvent) override;
    virtual bool EventNotify(NotifyEvent& rEvent) override;

    static void PrintWindowSubTree (vcl::Window* pRoot, int nIndentation);

    sal_Int32 GetMinimalWidth() const { return mnMinimalWidth; }

    class ScrollContainerWindow : public vcl::Window
    {
    public:
        ScrollContainerWindow(vcl::Window* pParentWindow);
        virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rUpdateArea) override;
        void SetSeparators(const ::std::vector<sal_Int32>& rSeparators);
    private:
        std::vector<sal_Int32> maSeparators;
    };

private:
    const OUString msId;
    sal_Int32 mnMinimalWidth;
    SharedPanelContainer maPanels;
    VclPtr<DeckTitleBar> mpTitleBar;
    VclPtr<vcl::Window> mpScrollClipWindow;
    VclPtr<ScrollContainerWindow> mpScrollContainer;
    VclPtr<vcl::Window> mpFiller;
    VclPtr<ScrollBar> mpVerticalScrollBar;

    DECL_LINK(HandleVerticalScrollBarChange, ScrollBar*, void);
    bool ProcessWheelEvent(CommandEvent* pCommandEvent);

};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
