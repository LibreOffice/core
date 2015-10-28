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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TASKPANE_TITLEBAR_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TASKPANE_TITLEBAR_HXX

#include <vcl/image.hxx>
#include <vcl/window.hxx>
#include <memory>

class Rectangle;
class VirtualDevice;

namespace sd { namespace toolpanel {

/** The title bar above a control in a sub tool panel.

    <p>The title bar shows two kinds of indicators: 1) Expansion is
    displayed by two sets of two bitmaps, a triangle pointing to the right
    resp. a minus in a square indicates that the control is collapsed, a
    triangle pointing down resp. a plus in a square stands for an expanded
    control. 2) Keyboard focus is indicated by a dotted rectangle.
*/
class TitleBar
    : public vcl::Window,
      public TreeNode
{
public:
    enum TitleBarType {
        TBT_SUB_CONTROL_HEADLINE
    };

    /** Create a new title bar whose content, the given title string,
        will be formatted according to the given type.
    */
    TitleBar (
        vcl::Window* pParent,
        const OUString& rsTitle,
        TitleBarType eType,
        bool bIsExpandable);
    virtual ~TitleBar();

    virtual Size GetPreferredSize();
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable();
    virtual vcl::Window* GetWindow();
    virtual sal_Int32 GetMinimumWidth();

    virtual void Paint (const Rectangle& rBoundingBox);
    virtual bool Expand (bool bFlag = true);
    virtual bool IsExpanded() const;
    virtual void SetEnabledState(bool bFlag);
    virtual void GetFocus();
    virtual void LoseFocus();

    virtual void MouseMove(const MouseEvent& rEvent);
    /** Empty implementation prevents forwarding to docking window.
    */
    virtual void MouseButtonDown (const MouseEvent& rEvent);
    /** Empty implementation prevents forwarding to docking window.
    */
    virtual void MouseButtonUp (const MouseEvent& rEvent);

    virtual void DataChanged (const DataChangedEvent& rEvent);

    OUString GetTitle() const { return msTitle; }

    css::uno::Reference<css::accessibility::XAccessible > CreateAccessibleObject (
            const css::uno::Reference<css::accessibility::XAccessible>& rxParent);

private:
    TitleBarType meType;
    OUString msTitle;
    bool mbExpanded;
    bool mbFocused;
    // Size of the bounding box that encloses the title string.
    ::std::unique_ptr<VirtualDevice> mpDevice;
    bool mbIsExpandable;

    /** Return whether this TitleBar object has an expansion indicator
        bitmap.  It is safe to call GetExpansionIndicator() when this method
        returns <FALSE/> but unnecessary.
    */
    bool HasExpansionIndicator() const;

    /** Return the image of the expansion indicator.
        @return
            When there is no expansion indicator for this TitleBar object,
            then an empty Image is returned.  You better call
            HasExpansionIndicator() to prevent this.
    */
    Image GetExpansionIndicator() const;

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

    void PaintSubPanelHeadLineBar();

    void PaintBackground (const Rectangle& rTextBox);

    /// Paint a focus indicator that encloses the given rectangle.
    void PaintFocusIndicator (const Rectangle& rIndicatorBox);

    Rectangle PaintExpansionIndicator (const Rectangle& rTextBox);

    void PaintText (const Rectangle& rTextBox);

    sal_uInt16 GetTextStyle();

    const static int snIndentationWidth;

    // Default constructor, copy constructor, and assignment are not supported.
    TitleBar();
    TitleBar (const TitleBar&);
    TitleBar& operator= (const TitleBar&);

    using Window::GetWindow;
};

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
