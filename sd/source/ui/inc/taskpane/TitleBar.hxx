/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TitleBar.hxx,v $
 * $Revision: 1.5 $
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

#ifndef SD_TASKPANE_TITLE_BAR_HXX
#define SD_TASKPANE_TITLE_BAR_HXX

#include "taskpane/TaskPaneTreeNode.hxx"
#include <vcl/image.hxx>
#include <tools/string.hxx>
#include <vcl/window.hxx>
#include <memory>

class Rectangle;
class String;
class VirtualDevice;

namespace sd { namespace toolpanel {


/** The title bar above a control in a tool panel or sub tool panel.
    The way the title bar is displayed depends on the TitleBarType
    given to the constructor.  TBT_CONTROL_TITLE and
    TBT_SUB_CONTROL_HEADLINE both show a expansion indicator in front of
    the title string that shows whether the associated control is
    visible (expanded) or not.
    A title bar with TBT_WINDOW_TITLE is typically used only once as the
    title bar of the whole task pane.

    <p>The title bar shows three kinds of indicators: 1) Expansion is
    displayed by two sets of two bitmaps, a triangle pointing to the right
    resp. a minus in a square indicates that the control is collapsed, a
    triangle pointing down resp. a plus in a square stands for an expanded
    control. 2) Keyboard focus is indicated by a dotted rectangle. 3) An
    underlined title string is a mouse over indicator for a
    selectable/expandable control.</p>
*/
class TitleBar
    : public ::Window,
      public TreeNode
{
public:
    enum TitleBarType {
        TBT_WINDOW_TITLE,
        TBT_CONTROL_TITLE,
        TBT_SUB_CONTROL_HEADLINE
        };

    /** Create a new title bar whose content, the given title string,
        will be formatted according to the given type.
    */
    TitleBar (
        ::Window* pParent,
        const String& rsTitle,
        TitleBarType eType,
        bool bIsExpandable);
    virtual ~TitleBar (void);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);
    virtual sal_Int32 GetMinimumWidth (void);

    virtual void Paint (const Rectangle& rBoundingBox);
    virtual bool Expand (bool bFlag = true);
    virtual bool IsExpanded (void) const;
    virtual void SetEnabledState(bool bFlag);

    void SetFocus (bool bFlag);

    virtual void MouseMove(const MouseEvent& rEvent);
    /** Empty implementation prevents forwarding to docking window.
    */
    virtual void MouseButtonDown (const MouseEvent& rEvent);
    /** Empty implementation prevents forwarding to docking window.
    */
    virtual void MouseButtonUp (const MouseEvent& rEvent);

    virtual void DataChanged (const DataChangedEvent& rEvent);

    String GetTitle (void) const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

private:
    TitleBarType meType;
    String msTitle;
    bool mbExpanded;
    bool mbFocused;
    bool mbMouseOver;
    // Size of the bounding box that encloses the title string.
    Size maStringBox;
    ::std::auto_ptr<VirtualDevice> mpDevice;
    bool mbIsExpandable;

    /** Set the mbMouseOver flag to the given value and paint the
        title bar accordingly.
    */
    void SetMouseOver (bool bFlag);

    /** Return whether this TitleBar object has an expansion indicator
        bitmap.  It is safe to call GetExpansionIndicator() when this method
        returns <FALSE/> but unnecessary.
    */
    bool HasExpansionIndicator (void) const;

    /** Return the image of the expansion indicator.
        @return
            When there is no expansion indictor for this TitleBar object,
            then an empty Image is returned.  You better call
            HasExpansionIndicator() to prevent this.
    */
    Image GetExpansionIndicator (void) const;

    /** Calculate the bounding box of the title text.  This takes into
        account indentation due to an expansion indicator and the given
        available width.  When the text can not be displayed on one line, it
        is broken into multiple lines.
        @param nAvailableWidth
            When 0 is given then the natural text width is used, i.e. the
            text is not broken into multiple lines.
    */
    Rectangle CalculateTextBoundingBox (
        int nAvailableWidth,
        bool bEmphasizeExpanded);

    /** Add some space to the given text box and return the bounding box of
        the title bar.
    */
    Rectangle CalculateTitleBarBox (
        const Rectangle& rTextBox,
        int nTitleBarWidth);

    void PaintWindowTitleBar (void);
    void PaintPanelControlTitle (void);
    void PaintSubPanelHeadLineBar (void);

    void PaintBackground (const Rectangle& rTextBox);

    /// Paint a focus indicator that encloses the given rectangle.
    void PaintFocusIndicator (const Rectangle& rIndicatorBox);

    /** Paint a mouse over indicator.  If the mouse is over the title
        bar than the text enclosed by the given rectangle is
        underlined.
    */
    void PaintMouseOverIndicator (const Rectangle& rIndicatorBox);

    Rectangle PaintExpansionIndicator (const Rectangle& rTextBox);

    void PaintText (const Rectangle& rTextBox);

    USHORT GetTextStyle (void);

    const static int snIndentationWidth;

    // Default constructor, copy constructor, and assignment are not supported.
    TitleBar (void);
    TitleBar (const TitleBar&);
    TitleBar& operator= (const TitleBar&);

    using Window::GetWindow;
};

} } // end of namespace ::sd::toolpanel

#endif
