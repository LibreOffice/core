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

#ifndef SD_WINDOW_HXX
#define SD_WINDOW_HXX


#include <tools/gen.hxx>
#include <vcl/window.hxx>
#include <svtools/transfer.hxx>

namespace sd {

class ViewShell;

// Since we removed all old SV-stuff, there is no brush any more
// and so there is no BRUSH_SIZE defined in VCL.
// So I define it here
// #i2237#
// removed old stuff here which still forced zoom to be
// %BRUSH_SIZE which is outdated now
//#define BRUSH_SIZE        8

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
class Window
    : public ::Window,
      public ::DropTargetHelper
{
public:
    Window (::Window* pParent);
    virtual ~Window (void);

    void    SetViewShell (ViewShell* pViewSh);

    /** Set the zoom factor to the specified value and center the display
        area arround the zoom center.
        @param nZoom
            The zoom factor is given as integral percent value.
    */
    void    SetZoomIntegral(long nZoom);

    /** This internally used method performs the actual adaption of the
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
    long    SetZoomFactor(long nZoom);

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
    long SetZoomRect (const Rectangle& rZoomRect);

    long GetZoomForRect( const Rectangle& rZoomRect );


    void SetMinZoomAutoCalc (bool bAuto);
    void SetCalcMinZoomByMinSide (bool bMin);

    /** Calculate the minimal zoom factor as the value at which the
        application area would completely fill the window.  All values set
        manually or programatically are set to this value if they are
        smaller.  If the currently used zoom factor is smaller than the minimal zoom
        factor than set the minimal zoom factor as the new current zoom
        factor.

        <p>This calculation is performed only when the
        <member>bMinZoomAutoCalc</member> is set (to <TRUE/>).</p>
    */
    void CalcMinZoom (void);
    void SetMinZoom (long int nMin);
    long GetMinZoom (void) const;
    void SetMaxZoom (long int nMax);
    long GetMaxZoom (void) const;

    long GetZoom (void) const;

    Point GetWinViewPos (void) const;
    Point GetViewOrigin (void) const;
    Size GetViewSize (void) const;
    void SetWinViewPos(const Point& rPnt);
    void SetViewOrigin(const Point& rPnt);
    void SetViewSize(const Size& rSize);
    void SetCenterAllowed (bool bIsAllowed);

    /** Calculate origin of the map mode accoring to the size of the view
        and window (its size in model coordinates; that takes the zoom
        factor into account), and the bCenterAllowed flag.  When it is not
        set then nothing is changed.  When in any direction the window is
        larger than the view or the value of aWinPos in this direction is -1
        then the window is centered in this direction.
        */
    void UpdateMapOrigin (sal_Bool bInvalidate = sal_True);

    void UpdateMapMode (void);

    double  GetVisibleX();          // Interface fuer ScrollBars
    double  GetVisibleY();
    void    SetVisibleXY(double fX, double fY);
    double  GetVisibleWidth();
    double  GetVisibleHeight();
    double  GetScrlLineWidth();
    double  GetScrlLineHeight();
    double  GetScrlPageWidth();
    double  GetScrlPageHeight();
    virtual void GrabFocus();
    virtual void DataChanged( const DataChangedEvent& rDCEvt );

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    /** The DropScroll() method is used by AcceptDrop() to scroll the
        content of the window while dragging and dropping.  With this method
        you can control whether DropScroll() shall be used.
    */
    void SetUseDropScroll (bool bUseDropScroll);
    void DropScroll (const Point& rMousePos);
protected:
    ::sd::Window* mpShareWin;
    Point maWinPos;
    Point maViewOrigin;
    Size maViewSize;
    sal_uInt16 mnMinZoom;
    sal_uInt16 mnMaxZoom;

    sal_uInt16 mnMaxZoom;
    /** This flag tells whether to re-calculate the minimal zoom factor
        depening on the current zoom factor.  Its default value is now sal_False.
    */
    bool mbMinZoomAutoCalc;
    bool mbCalcMinZoomByMinSide;
    bool mbCenterAllowed;
    long mnTicks;
    bool mbDraggedFrom;

    ViewShell* mpViewShell;
    bool mbUseDropScroll;

    virtual void Resize();
    virtual void PrePaint();
    virtual void Paint(const Rectangle& rRect);
    virtual void KeyInput(const KeyEvent& rKEvt);
    virtual void MouseMove(const MouseEvent& rMEvt);
    virtual void MouseButtonUp(const MouseEvent& rMEvt);
    virtual void MouseButtonDown(const MouseEvent& rMEvt);
    virtual void Command(const CommandEvent& rCEvt);
    virtual void RequestHelp( const HelpEvent& rEvt );
    virtual void LoseFocus();
    virtual long Notify( NotifyEvent& rNEvt );

    /** Create an accessibility object that makes this window accessible.

        @return
            The returned reference is empty if an accessible object could
            not be created.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessible (void);

    XubString GetSurroundingText() const;
    Selection GetSurroundingTextSelection() const;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
