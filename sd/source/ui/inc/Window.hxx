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

#include <tools/gen.hxx>
#include <tools/long.hxx>
#include <vcl/window.hxx>
#include <vcl/transfer.hxx>

class OutlinerView;

namespace sd
{
class ViewShell;

/** An SdWindow contains the actual working area of ViewShell.

    <p>The zoom factor used by this class controls how much the page and the
    shapes on it are scaled down (<100%) or up (>100%) when displayed on the
    output device represented by the <type>OutputDevice</type>base class.  A
    zoom factor of 100% would result (with a correctly set DPI value for an
    output device) in a one to one mapping of the internal coordinates that
    are stored in 100th of mm.  The zoom factor is stored in the map mode
    member of the <type>OutputDevice</type> base class.  It is calculated to
    be an integer percent value.
*/
class Window : public vcl::Window, public ::DropTargetHelper
{
public:
    Window(vcl::Window* pParent);
    virtual ~Window() override;
    virtual void dispose() override;

    void SetViewShell(ViewShell* pViewSh);
    ViewShell* GetViewShell();

    /** Set the zoom factor to the specified value and center the display
        area around the zoom center.
        @param nZoom
            The zoom factor is given as integral percent value.
    */
    void SetZoomIntegral(::tools::Long nZoom);

    /** This internally used method performs the actual adaptation of the
        window's map mode to the specified zoom factor.
        @param nZoom
            The zoom factor is given as integral percent value.
        @return
            When the given zoom factor lies outside the valid range enclosed
            by the minimal zoom factor previously calculated by
            <member>CalcMinZoom</member> and a constant upper value it is
            forced into that interval.  Therefore the returned value is a
            valid zoom factor.
    */
    ::tools::Long SetZoomFactor(::tools::Long nZoom);

    /** This method is called when the whole page shall be displayed in the
        window.  Position and zoom factor are set so that the given
        rectangle is displayed as large as possible in the window while at
        the same time maintaining the rectangle's aspect ratio and adding a
        small space at all its four sides (about 3% of width and height).
        The map mode is adapted accordingly.
        @param rZoomRect
            The rectangle is expected to be given relative to the upper left
            corner of the window in logical coordinates (100th of mm).
        @return
            The new zoom factor is returned as integral percent value.
    */
    ::tools::Long SetZoomRect(const ::tools::Rectangle& rZoomRect);

    ::tools::Long GetZoomForRect(const ::tools::Rectangle& rZoomRect);

    void SetMinZoomAutoCalc(bool bAuto);

    /** Calculate the minimal zoom factor as the value at which the
        application area would completely fill the window.  All values set
        manually or programmatically are set to this value if they are
        smaller.  If the currently used zoom factor is smaller than the minimal zoom
        factor than set the minimal zoom factor as the new current zoom
        factor.

        <p>This calculation is performed only when the
        <member>bMinZoomAutoCalc</member> is set (to <TRUE/>).</p>
    */
    void CalcMinZoom();
    void SetMinZoom(::tools::Long nMin);
    ::tools::Long GetMinZoom() const { return mnMinZoom; }
    void SetMaxZoom(::tools::Long nMax);
    ::tools::Long GetMaxZoom() const { return mnMaxZoom; }

    ::tools::Long GetZoom() const;

    const Point& GetWinViewPos() const { return maWinPos; }
    const Point& GetViewOrigin() const { return maViewOrigin; }
    const Size& GetViewSize() const { return maViewSize; }
    void SetWinViewPos(const Point& rPnt);
    void SetViewOrigin(const Point& rPnt);
    void SetViewSize(const Size& rSize);
    void SetCenterAllowed(bool bIsAllowed);

    /** Calculate origin of the map mode according to the size of the view
        and window (its size in model coordinates; that takes the zoom
        factor into account), and the bCenterAllowed flag.  When it is not
        set then nothing is changed.  When in any direction the window is
        larger than the view or the value of aWinPos in this direction is -1
        then the window is centered in this direction.
        */
    void UpdateMapOrigin(bool bInvalidate = true);

    void UpdateMapMode();

    double GetVisibleX() const; // interface for ScrollBars
    double GetVisibleY() const;
    void SetVisibleXY(double fX, double fY);
    double GetVisibleWidth() const;
    double GetVisibleHeight() const;
    Point GetVisibleCenter();
    double GetScrlLineWidth() const;
    double GetScrlLineHeight() const;
    double GetScrlPageWidth() const;
    double GetScrlPageHeight() const;
    void GrabFocus();
    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;

    // DropTargetHelper
    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override;

    /** The DropScroll() method is used by AcceptDrop() to scroll the
        content of the window while dragging and dropping.  With this method
        you can control whether DropScroll() shall be used.
    */
    void SetUseDropScroll(bool bUseDropScroll);
    void DropScroll(const Point& rMousePos);
    virtual void KeyInput(const KeyEvent& rKEvt) override;

private:
    OutlinerView* GetOutlinerView() const;

protected:
    Point maWinPos;
    Point maViewOrigin;
    Size maViewSize;
    Size maPrevSize; // contains previous window size in logical coords
    sal_uInt16 mnMinZoom;
    sal_uInt16 mnMaxZoom;

    /** This flag tells whether to re-calculate the minimal zoom factor
        depending on the current zoom factor.  Its default value is now false.
    */
    bool mbMinZoomAutoCalc;
    bool mbCenterAllowed;
    ::tools::Long mnTicks;

    ViewShell* mpViewShell;
    bool mbUseDropScroll;

    virtual void Resize() override;
    virtual void PrePaint(vcl::RenderContext& rRenderContext) override;
    virtual void Paint(vcl::RenderContext& rRenderContext,
                       const ::tools::Rectangle& rRect) override;
    virtual void MouseMove(const MouseEvent& rMEvt) override;
    virtual void MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void Command(const CommandEvent& rCEvt) override;
    virtual void RequestHelp(const HelpEvent& rEvt) override;
    virtual void LoseFocus() override;
    virtual bool EventNotify(NotifyEvent& rNEvt) override;

    /** Create an accessibility object that makes this window accessible.

        @return
            The returned reference is empty if an accessible object could
            not be created.
    */
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    OUString GetSurroundingText() const override;
    Selection GetSurroundingTextSelection() const override;
    bool DeleteSurroundingText(const Selection& rSelection) override;

    /// @see Window::LogicInvalidate().
    void LogicInvalidate(const ::tools::Rectangle* pRectangle) override;
    /// Same as MouseButtonDown(), but coordinates are in logic unit.
    virtual void LogicMouseButtonDown(const MouseEvent& rMouseEvent) override;
    /// Same as MouseButtonUp(), but coordinates are in logic unit.
    virtual void LogicMouseButtonUp(const MouseEvent& rMouseEvent) override;
    /// Same as MouseMove(), but coordinates are in logic unit.
    virtual void LogicMouseMove(const MouseEvent& rMouseEvent) override;

    FactoryFunction GetUITestFactory() const override;
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
