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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TASKPANE_SUBTOOLPANEL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TASKPANE_SUBTOOLPANEL_HXX

#include "taskpane/TaskPaneTreeNode.hxx"

#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>
#include <vector>


class Window;

namespace sd { namespace toolpanel {

/** The sub tool panel is in function similar to the tool panel.  It
    differes in two points.  First, it is a control that can be used
    as element in a tool panel and thus is actually a nested tool
    panel.

    <p>Secondly, it formats its elements differently.  The actual controls
    are placed one below the other with a title bar above each control.
    Clicking on the title bar expands or collapses the control.  When there
    is not enough space then scroll bars are shown.</p>

    <p>To avoid flickering when painting the sub tool panel the background
    is made transparent and painting it is done by this class.  While
    layouting its children it remembers the gaps between children and stores
    them in maStripeList.  In Paint() those gaps as well as the border
    around all children are painted in the background color.</p>
*/
class SubToolPanel
    : public Control,
      public TreeNode
{
public:
    /** Create a new sub tool panel with the given window as its
        parent.  This will usually be a child window.
    */
    SubToolPanel (Window& i_rParentWindow);
    virtual ~SubToolPanel (void);

    virtual void Paint (const Rectangle& rRect);

    /** Initiate a rearrangement of the controls and title bars.
    */
    virtual void Resize (void);

    virtual void RequestResize (void);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);
    virtual sal_Int32 GetMinimumWidth (void);

    virtual void ExpandControl (
        TreeNode* pControl,
        bool bExpansionState);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

    using Window::GetWindow;
private:
    ::Window maWindowFiller;
    bool mbIsRearrangePending;
    bool mbIsLayoutPending;
    sal_uInt32 mnChildrenWidth;
    /// Border above top-most and below bottom-most control.
    const int mnVerticalBorder;
    /// Gap between two controls.
    const int mnVerticalGap;
    /// Border at the left and right of the controls.
    const int mnHorizontalBorder;
    /** List of horizontal stripes that is created from the gaps between
        children when they are layouted.  The stripes are painted in Paint()
        to fill the space around the children.
    */
    typedef ::std::vector< ::std::pair<int,int> > StripeList;
    StripeList maStripeList;

    /** Calculate position, size, and visibility of the controls.
        Call this method after the list of controls, their expansion
        state, or the size of the sub panel has changed.
    */
    void Rearrange (void);

    /** Determine the minimal size that is necessary to show the controls
        one over the other.  It may be smaller than the available area.
    */
    Size GetRequiredSize (void);

    /** Place the child windows one above the other and return the size of
        the bounding box.
    */
    sal_Int32 LayoutChildren (void);

    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
