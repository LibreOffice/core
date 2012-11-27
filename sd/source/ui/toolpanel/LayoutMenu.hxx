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

#ifndef SD_TASKPANE_LAYOUT_MENU_HXX
#define SD_TASKPANE_LAYOUT_MENU_HXX

#include "taskpane/TaskPaneTreeNode.hxx"

#include <com/sun/star/frame/XStatusListener.hpp>

#include "glob.hxx"
#include "pres.hxx"
#include <vcl/ctrl.hxx>
#include <svtools/valueset.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/shell.hxx>


class SfxModule;


namespace sd {
class DrawDocShell;
class ViewShellBase;
}


namespace sd { namespace tools {
class EventMultiplexerEvent;
} }


namespace sd { namespace toolpanel {

class ControlFactory;
class ToolPanelViewShell;


class LayoutMenu
    : public ValueSet,
      public TreeNode,
      public SfxShell,
      public DragSourceHelper,
      public DropTargetHelper
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDLAYOUTMENU)

    /** Create a new layout menu.  Depending on the given flag it
        displays its own scroll bar or lets a surrounding window
        handle that.
        @param i_pParent
            the parent node in the control tree
        @param i_rPanelViewShell
            the view shell of the task pane.
    */
    LayoutMenu (
        TreeNode* i_pParent,
        ToolPanelViewShell& i_rPanelViewShell);
    virtual ~LayoutMenu (void);

    static std::auto_ptr<ControlFactory> CreateControlFactory (
        ToolPanelViewShell& i_rPanelViewShell );

    /** Return a numerical value representing the currently selected
        layout.
    */
    AutoLayout GetSelectedAutoLayout (void);


    // From ILayoutableWindow
    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual sal_Int32 GetMinimumWidth (void);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);

    // From ::Window
    virtual void Paint (const Rectangle& rRect);
    virtual void Resize (void);

    /** Show a context menu when the right mouse button is pressed.
    */
    virtual void MouseButtonDown (const MouseEvent& rEvent);

    void Execute (SfxRequest& rRequest);
    void GetState (SfxItemSet& rItemSet);

    /** The LayoutMenu does not support some main views.  In this case the
        LayoutMenu is disabled.  This state is updated in this method.
        @param eMode
            On some occasions the edit mode is being switched when this
            method is called can not (yet) be reliably detected.  Luckily,
            in these cases the new value is provided by some broadcaster.
            On other occasions the edit mode is not modified and is also not
            provided.  Therefore the Unknown value.
    */
    enum MasterMode { MM_NORMAL, MM_MASTER, MM_UNKNOWN };
    void UpdateEnabledState (const MasterMode eMode);

    // TreeNode overridables
    virtual TaskPaneShellManager* GetShellManager (void);

    /** Call this method when the set of displayed layouts is not up-to-date
        anymore.  It will re-assemple this set according to the current
        settings.
    */
    void InvalidateContent (void);

    // DragSourceHelper
    virtual void StartDrag (sal_Int8 nAction, const Point& rPosPixel);

    // DropTargetHelper
    virtual sal_Int8 AcceptDrop (const AcceptDropEvent& rEvent);
    virtual sal_Int8 ExecuteDrop (const ExecuteDropEvent& rEvent);

    /** The context menu is requested over this Command() method.
    */
    virtual void Command (const CommandEvent& rEvent);

    /** Call Fill() when switching to or from high contrast mode so that the
        correct set of icons is displayed.
    */
    virtual void DataChanged (const DataChangedEvent& rEvent);

    using Window::GetWindow;
    using ValueSet::StartDrag;

private:
    ViewShellBase& mrBase;

    TaskPaneShellManager*   mpShellManager;

    /** Do we use our own scroll bar or is viewport handling done by
        our parent?
    */
    bool mbUseOwnScrollBar;

    /** If we are asked for the preferred window size, then use this
        many columns for the calculation.
    */
    const int mnPreferredColumnCount;

    ::com::sun::star::uno::Reference<com::sun::star::frame::XStatusListener> mxListener;

    bool mbSelectionUpdatePending;

    bool mbIsMainViewChangePending;

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
    void Fill (void);

    /** Remove all items from the value set.
    */
    void Clear (void);

    /** Assign the given layout to all selected slides of a slide sorter.
        If no slide sorter is active then this call is ignored.  The slide
        sorter in the center pane is preferred if the choice exists.
    */
    void AssignLayoutToSelectedSlides (AutoLayout aLayout);

    /** Insert a new page with the given layout.  The page is inserted via
        the main view shell, i.e. its SID_INSERTPAGE slot is called.  It it
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
    void UpdateSelection (void);

    // internal ctor
    void    implConstruct( DrawDocShell& rDocumentShell );

    /** When clicked then set the current page of the view in the center pane.
    */
    DECL_LINK(ClickHandler, void *);
    DECL_LINK(RightClickHandler, MouseEvent*);
    DECL_LINK(StateChangeHandler, void *);
    DECL_LINK(EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent*);
};

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
