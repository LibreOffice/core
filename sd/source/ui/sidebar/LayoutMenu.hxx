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

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_LAYOUTMENU_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_LAYOUTMENU_HXX

#include <sfx2/sidebar/ILayoutableWindow.hxx>

#include <svtools/valueset.hxx>
#include <vcl/transfer.hxx>
#include <sfx2/shell.hxx>
#include <xmloff/autolayout.hxx>

namespace com { namespace sun { namespace star { namespace frame { class XStatusListener; } } } }
namespace com { namespace sun { namespace star { namespace ui { class XSidebar; } } } }


namespace sd {
class DrawDocShell;
class ViewShellBase;
}

namespace sd { namespace tools {
class EventMultiplexerEvent;
} }

namespace sd { namespace sidebar {

class LayoutMenu
    : public ValueSet,
      public DragSourceHelper,
      public DropTargetHelper,
      public sfx2::sidebar::ILayoutableWindow
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
    LayoutMenu (
        vcl::Window* pParent,
        ViewShellBase& rViewShellBase,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);
    virtual ~LayoutMenu() override;
    virtual void dispose() override;

    void Dispose();

    /** Return a numerical value representing the currently selected
        layout.
    */
    AutoLayout GetSelectedAutoLayout();

    // From ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth) override;

    // From vcl::Window
    virtual void Resize() override;

    /** Show a context menu when the right mouse button is pressed.
    */
    virtual void MouseButtonDown (const MouseEvent& rEvent) override;

    /** Call this method when the set of displayed layouts is not up-to-date
        anymore.  It will re-assemble this set according to the current
        settings.
    */
    void InvalidateContent();

    // DragSourceHelper
    virtual void StartDrag (sal_Int8 nAction, const Point& rPosPixel) override;

    // DropTargetHelper
    virtual sal_Int8 AcceptDrop (const AcceptDropEvent& rEvent) override;
    virtual sal_Int8 ExecuteDrop (const ExecuteDropEvent& rEvent) override;

    /** The context menu is requested over this Command() method.
    */
    virtual void Command (const CommandEvent& rEvent) override;

    /** Call Fill() when switching to or from high contrast mode so that the
        correct set of icons is displayed.
    */
    virtual void DataChanged (const DataChangedEvent& rEvent) override;

    using Window::GetWindow;
    using ValueSet::StartDrag;

private:
    ViewShellBase& mrBase;

    /** If we are asked for the preferred window size, then use this
        many columns for the calculation.
    */
    css::uno::Reference<css::frame::XStatusListener> mxListener;
    bool mbIsMainViewChangePending;
    css::uno::Reference<css::ui::XSidebar> mxSidebar;
    bool mbIsDisposed;

    /** Calculate the number of displayed rows.  This depends on the given
        item size, the given number of columns, and the size of the
        control.  Note that this is not the number of rows managed by the
        valueset.  This number may be larger.  In that case a vertical
        scroll bar is displayed.
    */
    int CalculateRowCount (const Size& rItemSize, int nColumnCount);

    /** Fill the value set with the layouts that are applicable to the
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
    void AssignLayoutToSelectedSlides (AutoLayout aLayout);

    /** Insert a new page with the given layout.  The page is inserted via
        the main view shell, i.e. its SID_INSERTPAGE slot is called. If it
        does not support this slot then inserting a new page does not take
        place.  The new page is inserted after the currently active one (the
        one returned by ViewShell::GetActualPage().)
    */
    void InsertPageWithLayout (AutoLayout aLayout);

    /** Create a request structure that can be used with the SID_INSERTPAGE
        and SID_MODIFYPAGE slots.  The parameters are set so that the given
        layout is assigned to the current page of the main view shell.
        @param nSlotId
            Supported slots are SID_INSERTPAGE and SID_MODIFYPAGE.
        @param aLayout
            Layout of the page to insert or to assign.
    */
    SfxRequest CreateRequest (
        sal_uInt16 nSlotId,
        AutoLayout aLayout);

    /** Select the layout that is used by the current page.
    */
    void UpdateSelection();

    // internal ctor
    void    implConstruct( DrawDocShell& rDocumentShell );

    /** When clicked then set the current page of the view in the center pane.
    */
    DECL_LINK(ClickHandler, ValueSet*, void);
    DECL_LINK(StateChangeHandler, const OUString&, void);
    DECL_LINK(EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent&, void);
    DECL_LINK(WindowEventHandler, VclWindowEvent&, void);
    DECL_LINK(OnMenuItemSelected, Menu*, bool);
};

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
