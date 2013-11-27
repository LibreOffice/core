/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    void    SetZoomIntegral(double fZoom);

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
    double SetZoomFactor(double fZoom);

    /** This method is called when the whole page shall be displayed in the
        window.  Position and zoom factor are set so that the given
        rectangle is displayed as large as possible in the window while at
        the same time maintaining the rectangle's aspect ratio and adding a
        small space at all its four sides (about 3% of width and height).
        The map mode is adapted accordingly.
        @param rZoomRange
            The rectangle is expected to be given relative to the upper left
            corner of the window in logical coordinates (100th of mm).
        @return
            The new zoom factor is returned as integral percent value.
    */
    double SetZoomRange(const basegfx::B2DRange& rZoomRange);

    double GetZoomForRange( const basegfx::B2DRange& rZoomRange );


    void SetMinZoomAutoCalc (bool bAuto);
    void SetCalcMinZoomByMinSide (bool bMin);

    /** Calculate the minimal zoom factor as the value at which the
        application area would completely fill the window.  All values set
        manually or programatically are set to this value if they are
        smaller.  If the currently used zoom factor is smaller than the minimal zoom
        factor than set the minimal zoom factor as the new current zoom
        factor.

        <p>This calculation is performed only when the
        <member>bMinZoomAutoCalc</member> is set (to <true/>).</p>
    */
    void CalcMinZoom (void);
    void SetMinZoom (double fMin);
    double GetMinZoom (void) const;
    void SetMaxZoom (double fMax);
    double GetMaxZoom (void) const;

    double GetZoom (void) const;

    basegfx::B2DPoint GetWinViewPos (void) const;
    basegfx::B2DPoint GetViewOrigin (void) const;
    basegfx::B2DVector GetViewSize (void) const;
    void SetWinViewPos(const basegfx::B2DPoint& rPnt);
    void SetViewOrigin(const basegfx::B2DPoint& rPnt);
    void SetViewSize(const basegfx::B2DVector& rSize);
    void SetCenterAllowed (bool bIsAllowed);

    /** Calculate origin of the map mode accoring to the size of the view
        and window (its size in model coordinates; that takes the zoom
        factor into account), and the bCenterAllowed flag.  When it is not
        set then nothing is changed.  When in any direction the window is
        larger than the view or the value of aWinPos in this direction is -1
        then the window is centered in this direction.
        */
    void UpdateMapOrigin (bool bInvalidate = true);

    void UpdateMapMode (void);

    double  GetVisibleX();          // Interface fuer ScrollBars
    double  GetVisibleY();
    void    SetVisibleXY(double fX, double fY);
    double  GetVisibleWidthRelativeToView();
    double  GetVisibleHeightRelativeToView();
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
    void DropScroll (const basegfx::B2DPoint& rMousePos);
protected:
    ::sd::Window* mpShareWin;
    basegfx::B2DPoint maWinPos;
    basegfx::B2DPoint maViewOrigin;
    basegfx::B2DVector maViewSize;
    double mfMinZoom;
    double mfMaxZoom;
    /** This flag tells whether to re-calculate the minimal zoom factor
        depening on the current zoom factor.  According to task #105436# its
        default value is now false.
    */
    bool mbMinZoomAutoCalc;
    bool mbCalcMinZoomByMinSide;
    bool mbCenterAllowed;
    sal_uInt32 mnTicks;
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
//IAccessibility2 Implementation 2009-----
    virtual void SwitchView();
//-----IAccessibility2 Implementation 2009

    XubString GetSurroundingText() const;
    Selection GetSurroundingTextSelection() const;
};

} // end of namespace sd

#endif
