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
#ifndef SFX_SIDEBAR_DECK_HXX
#define SFX_SIDEBAR_DECK_HXX

#include "Panel.hxx"

#include <vcl/window.hxx>
#include <vcl/image.hxx>
#include <com/sun/star/ui/LayoutSize.hpp>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

class ScrollBar;

namespace css = ::com::sun::star;

namespace sfx2 { namespace sidebar {

class DeckDescriptor;
class DeckTitleBar;


/** This is the parent window of the panels.
    It displays the deck title.
*/
class Deck
    : public Window
{
public:
    Deck (
        const DeckDescriptor& rDeckDescriptor,
        Window* pParentWindow,
        const ::boost::function<void(void)>& rCloserAction);
    virtual ~Deck (void);

    void Dispose (void);

    const ::rtl::OUString& GetId (void) const;
    DeckTitleBar* GetTitleBar (void) const;
    Rectangle GetContentArea (void) const;
    void SetPanels (const SharedPanelContainer& rPanels);
    const SharedPanelContainer& GetPanels (void) const;
    void RequestLayout (void);
    ::Window* GetPanelParentWindow (void);

    /** Try to make the panel completely visible.
        When the whole panel does not fit then make its top visible
        and it off at the bottom.
    */
    void ShowPanel (const Panel& rPanel);

    virtual void Paint (const Rectangle& rUpdateArea);
    virtual void DataChanged (const DataChangedEvent& rEvent);
    virtual long Notify (NotifyEvent& rEvent);

    void PrintWindowTree (void);
    void PrintWindowTree (const ::std::vector<Panel*>& rPanels);
    static void PrintWindowSubTree (Window* pRoot, int nIndentation);

    sal_Int32 GetMinimalWidth() const { return mnMinimalWidth; }

    class ScrollContainerWindow : public Window
    {
    public:
        ScrollContainerWindow (Window* pParentWindow);
        virtual ~ScrollContainerWindow (void);
        virtual void Paint (const Rectangle& rUpdateArea);
        void SetSeparators (const ::std::vector<sal_Int32>& rSeparators);
    private:
        ::std::vector<sal_Int32> maSeparators;
    };

private:
    const ::rtl::OUString msTitle;
    const ::rtl::OUString msId;
    Image maIcon;
    const ::rtl::OUString msIconURL;
    const ::rtl::OUString msHighContrastIconURL;
    sal_Int32 mnMinimalWidth;
    SharedPanelContainer maPanels;
    ::boost::scoped_ptr<DeckTitleBar> mpTitleBar;
    ::boost::scoped_ptr<Window> mpScrollClipWindow;
    ::boost::scoped_ptr<ScrollContainerWindow> mpScrollContainer;
    ::boost::scoped_ptr<Window> mpFiller;
    ::boost::scoped_ptr<ScrollBar> mpVerticalScrollBar;

    DECL_LINK(HandleVerticalScrollBarChange,void*);
    bool ProcessWheelEvent (
        CommandEvent* pCommandEvent,
        NotifyEvent& rEvent);
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
