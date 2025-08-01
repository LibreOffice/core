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

#pragma once

#include <sfx2/sidebar/ILayoutableWindow.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>

#include <sfx2/request.hxx>
#include <xmloff/autolayout.hxx>
#include <vcl/image.hxx>
#include <map>

#include <unotools/resmgr.hxx>

namespace com::sun::star::ui
{
class XSidebar;
}

namespace sd
{
class DrawDocShell;
class ViewShellBase;
}

namespace sd::tools
{
class EventMultiplexerEvent;
class SlotStateListener;
}

namespace sd::sidebar
{
class LayoutMenu : public PanelLayout, public sfx2::sidebar::ILayoutableWindow
{
public:
    /** Create a new layout menu.  Depending on the given flag it
        displays its own scroll bar or lets a surrounding window
        handle that.
        @param i_pParent
            the parent node in the control tree
        @param i_rPanelViewShell
            the view shell of the task pane.
    */
    LayoutMenu(weld::Widget* pParent, ViewShellBase& rViewShellBase,
               css::uno::Reference<css::ui::XSidebar> xSidebar);
    virtual ~LayoutMenu() override;

    void Dispose();

    /** Return a numerical value representing the currently selected
        layout.
    */
    AutoLayout GetSelectedAutoLayout() const;

    // From ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth(const sal_Int32 nWidth) override;

    /** Call this method when the set of displayed layouts is not up-to-date
        anymore.  It will re-assemble this set according to the current
        settings.
    */
    void InvalidateContent();

    /** The context menu is requested over this ShowContextMenu() method.
    */
    void ShowContextMenu(const Point& pPos);

    /** Call Fill() when switching to or from high contrast mode so that the
        correct set of icons is displayed.
    */
    virtual void DataChanged(const DataChangedEvent& rEvent) override;

private:
    ViewShellBase& mrBase;

    std::unique_ptr<weld::IconView> mxLayoutIconView;
    /** If we are asked for the preferred window size, then use this
        many columns for the calculation.
    */
    rtl::Reference<::sd::tools::SlotStateListener> mxListener;
    bool mbIsMainViewChangePending;
    css::uno::Reference<css::ui::XSidebar> mxSidebar;
    bool mbIsDisposed;
    std::map<AutoLayout, TranslateId> maLayoutToStringMap;

    std::unique_ptr<weld::Builder> mxMenuBuilder;
    std::unique_ptr<weld::Menu> mxMenu;

    // Store the size of preview image
    Size maPreviewSize;

    /**
     * Prevents StateChangeHandler from rebuilding layouts during context menu operations.
     * Without this flag, the first context menu operation would trigger a layout rebuild that
     * disrupts the selection state, causing the selected item to not appear highlighted.
     * Subsequent operations work correctly.
     */
    bool bInContextMenuOperation;

    OUString sLastItemIdent;

    /** Fill the icon view with the layouts that are applicable to the
        current main view shell.
    */
    void Fill();

    /** Remove all items from the value set.
    */
    void Clear();

    /** Assign the given layout to all selected slides of a slide sorter.
        If no slide sorter is active then this call is ignored.  The slide
        sorter in the center pane is preferred if the choice exists.
    */
    void AssignLayoutToSelectedSlides(AutoLayout aLayout);

    /** Insert a new page with the given layout.  The page is inserted via
        the main view shell, i.e. its SID_INSERTPAGE slot is called. If it
        does not support this slot then inserting a new page does not take
        place.  The new page is inserted after the currently active one (the
        one returned by ViewShell::GetActualPage().)
    */
    void InsertPageWithLayout(AutoLayout aLayout);

    /** Create a request structure that can be used with the SID_INSERTPAGE
        and SID_MODIFYPAGE slots.  The parameters are set so that the given
        layout is assigned to the current page of the main view shell.
        @param nSlotId
            Supported slots are SID_INSERTPAGE and SID_MODIFYPAGE.
        @param aLayout
            Layout of the page to insert or to assign.
    */
    SfxRequest CreateRequest(sal_uInt16 nSlotId, AutoLayout aLayout);

    /** Select the layout that is used by the current page.
    */
    void UpdateSelection();

    // internal ctor
    void implConstruct(DrawDocShell& rDocumentShell);

    void MenuSelect(const OUString& rIdent);

    /** When clicked then set the current page of the view in the center pane.
    */
    DECL_LINK(LayoutSelected, weld::IconView&, bool);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);
    DECL_LINK(StateChangeHandler, const OUString&, void);
    DECL_LINK(EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent&, void);
    DECL_LINK(MenuSelectAsyncHdl, void*, void);
    DECL_LINK(OnPopupEnd, const OUString&, void);

    static VclPtr<VirtualDevice> GetVirtualDevice(Image pPreview);
    static Bitmap GetPreviewAsBitmap(const Image& rImage);
    void HandleMenuSelect(std::u16string_view rIdent);

    TranslateId GetStringResourceIdForLayout(AutoLayout aLayout) const;
};

} // end of namespace ::sd::toolpanel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
