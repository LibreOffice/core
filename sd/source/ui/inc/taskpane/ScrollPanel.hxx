/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_TOOLPANEL_SCROLL_PANEL_HXX
#define SD_TOOLPANEL_SCROLL_PANEL_HXX

#include "taskpane/TaskPaneTreeNode.hxx"

#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>
#include <memory>
#include <vector>

namespace sd { namespace toolpanel {

class TitledControl;

/** The scroll panel shows its controls one above the other.  When their
    total height is larger than the height of the scroll area then only a
    part of the controls is visible.  Scroll bars control which part that
    is.

    The scroll panel registers itself as window event listener at the
    controls and their title bars (conceptually; it really is the
    TitledControl) to track changes of the selection and focus rectangles.
    On such a change it tries to move the selected or focused part into the
    visible area.  At the moment this moving into view only works with
    valuesets and TitleBars.
*/
class ScrollPanel
    : public ::Control,
      public TreeNode
{
public:
    /** Create a new scroll panel which itself is the root of a TreeNode hierarchy
        parent.  This will usually be a child window.
    */
    ScrollPanel (::Window& i_rParentWindow);
    virtual ~ScrollPanel (void);

    /** Add a control to the sub panel.  An title bar is added above the
        control.
        @param rTitle
            The title that will be shown in the two title bars that
            belong to the control.
        @param nHelpId
            The help id is set at the title bar not the actual control.
        @return
            The new titled control that contains the given control and a new
            title bar as children is returned.
    */
    TitledControl* AddControl (
        ::std::auto_ptr<TreeNode> pControl,
        const String& rTitle,
        const rtl::OString& sHelpId);

    /** Add a control to the sub panel without a title bar.
    */
    void AddControl (::std::auto_ptr<TreeNode> pControl);

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

    bool IsVerticalScrollBarVisible (void) const;
    bool IsHorizontalScrollBarVisible (void) const;
    ScrollBar& GetVerticalScrollBar (void);
    ScrollBar& GetHorizontalScrollBar (void);

    // ::Window
    virtual long Notify( NotifyEvent& rNEvt );

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

    /** Scroll the given rectangle into the visible area.
        @param aRectangle
            The box to move into the visible area in pixel coordinates
            relative to the given window.
        @param pWindow
            This window is used to translate the given coordinates into ones
            that are relative to the scroll panel.

    */
    void MakeRectangleVisible (
        Rectangle& aRectangle,
        ::Window* pWindow);

private:
    ::Control maScrollWindow;
    ScrollBar maVerticalScrollBar;
    ScrollBar maHorizontalScrollBar;
    ::Window maScrollBarFiller;
    ::Window maScrollWindowFiller;
    Point maScrollOffset;
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
        to fill the space arround the children.
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

    /** ctor-impl
    */
    void    Construct();

    Size SetupScrollBars (const Size& rRequiresSize);
    sal_Int32 SetupVerticalScrollBar (bool bShow, sal_Int32 nRange);
    sal_Int32 SetupHorizontalScrollBar (bool bShow, sal_Int32 nRange);

    DECL_LINK(ScrollBarHandler, ScrollBar*);
    DECL_LINK(WindowEventListener, VclSimpleEvent*);

    using Window::GetWindow;
};

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
