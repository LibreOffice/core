/*************************************************************************
 *
 *  $RCSfile: SlideSorterViewShell.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:01:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_SLIDE_SORTER_VIEW_SHELL_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_VIEW_SHELL_HXX

#include "ViewShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "glob.hxx"
#ifndef _SFX_SHELL_HXX
#include <sfx2/shell.hxx>
#endif
#ifndef _VIEWFAC_HXX
#include <sfx2/viewfac.hxx>
#endif

class ScrollBarBox;
class TabBar;
class Window;

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }

namespace sd { namespace slidesorter { namespace controller {
class Listener;
class SlideSorterController;
class SlotManager;
} } }

namespace sd { namespace slidesorter {


class SlideSorterViewShell
    : public ViewShell
{
    friend class controller::SlotManager;
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDSLIDESORTERVIEWSHELL);

    enum TabBarEntry
    {
        TBE_SWITCH = 0,
        TBE_SLIDES = 1,
        TBE_MASTER_PAGES = 2
    };

    static SfxShell* CreateInstance (
        sal_Int32 nId,
        SfxShell* pParent,
        void* pUserData,
        ViewShellBase& rBase);

    SlideSorterViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::Window* pParentWindow,
        FrameView* pFrameView);

    virtual ~SlideSorterViewShell (void);

    /** Late initialization that has to be called after a new instance has
        completed its construction.
    */
    virtual void Init (void);

    virtual void GetFocus (void);
    virtual void LoseFocus (void);
    virtual SdPage* GetActualPage (void);

    void ExecCtrl (SfxRequest& rRequest);
    virtual void GetCtrlState (SfxItemSet &rSet);
    virtual void FuSupport (SfxRequest& rRequest);
    virtual void FuTemporary (SfxRequest& rRequest);
    virtual void GetStatusBarState (SfxItemSet& rSet);
    virtual void FuPermanent (SfxRequest& rRequest);
    void GetAttrState (SfxItemSet& rSet);
    void ExecStatusBar (SfxRequest& rRequest);
    virtual void Command (const CommandEvent& rEvent, ::sd::Window* pWindow);
    virtual void GetMenuState (SfxItemSet &rSet);

    virtual void ReadFrameViewData (FrameView* pView);
    virtual void WriteFrameViewData (void);

    /** The UI features are used for selective display of tool bars
        depending on whether the slide sorter is the main view or not.
        @param nFeature
            Valid values are defined (and used) in the implementation file.
    */
    virtual BOOL HasUIFeature (ULONG nFeature);

    /** Set the zoom factor.  The given value is clipped against an upper
        bound.
        @param nZoom
            An integer percent value, i.e. nZoom/100 is the actual zoom
            factor.
        */
    virtual void SetZoom (long int nZoom);
    virtual void SetZoomRect (const Rectangle& rZoomRect);

    /** This is a callback method used by the active window to delegate its
        Paint() call to.  This view shell itself delegates it to the view.
    */
    virtual void Paint(const Rectangle& rRect, ::sd::Window* pWin);

    /** Place and size the controls and windows.  You may want to call this
        method when something has changed that for instance affects the
        visibility state of the scroll bars.
    */
    virtual void ArrangeGUIElements (void);

    /** Return the control of the vertical scroll bar.
    */
    ScrollBar* GetVerticalScrollBar (void) const;

    /** Return the control of the horizontal scroll bar.
    */
    ScrollBar* GetHorizontalScrollBar (void) const;

    /** Return the scroll bar filler that paints the little square that is
        enclosed by the two scroll bars.
    */
    ScrollBarBox* GetScrollBarFiller (void) const;

    /** Set the tab bar to the given mode.
        @param eEntry
            When TBE_SWITCH is given, then switch between the two tabs.
    */
    TabBarEntry SwitchTabBar (TabBarEntry eEntry);

    controller::SlideSorterController& GetSlideSorterController (void);

    //===== Drag and Drop =====================================================

    virtual void StartDrag (
        const Point& rDragPt,
        ::Window* pWindow );
    virtual void DragFinished (
        sal_Int8 nDropAction);
    virtual sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        USHORT nPage = SDRPAGE_NOTFOUND,
        USHORT nLayer = SDRPAGE_NOTFOUND );
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        USHORT nPage = SDRPAGE_NOTFOUND,
        USHORT nLayer = SDRPAGE_NOTFOUND);

    /** Return the selected pages by putting them into the given container.
        The container does not have to be empty.  It is not cleared.
    */
    void GetSelectedPages (::std::vector<SdPage*>& pPageContainer);

    virtual DrawController* GetController (void);

protected:
    ::std::auto_ptr<controller::SlideSorterController> mpSlideSorterController;
    ::std::auto_ptr<model::SlideSorterModel> mpSlideSorterModel;
    ::std::auto_ptr<view::SlideSorterView> mpSlideSorterView;

    virtual SvBorder GetBorder (bool bOuterResize);

    /** This virtual method makes it possible to create a specialization of
        the slide sorter view shell that works with its own implementation
        of model, view, and controller.  The default implementation simply
        calls the CreateModel(), CreateView(), and CreateController()
        methods in this order.
    */
    virtual void CreateModelViewController (void);

    /** Create the model for the view shell.  When called from the default
        implementation of CreateModelViewController() then neither view nor
        controller do exist.  Test their pointers when in doubt.
    */
    virtual model::SlideSorterModel* CreateModel (void);

    /** Create the view for the view shell.  When called from the default
        implementation of CreateModelViewController() then the model but not
        the controller does exist.  Test their pointers when in doubt.
    */
    virtual view::SlideSorterView* CreateView (void);

    /** Create the controller for the view shell.  When called from the default
        implementation of CreateModelViewController() then both the view and
        the controller do exist.  Test their pointers when in doubt.
    */
    virtual controller::SlideSorterController* CreateController (void);

private:
    ::std::auto_ptr<TabBar> mpTabBar;

    /** Set this flag to <TRUE/> to force a layout before the next paint.
    */
    bool mbLayoutPending;

    /** Create the controls for the slide sorter.  This are the tab bar
       for switching the edit mode, the scroll bar, and the actual
       slide sorter view window.
       This method is usually called exactly one time from the
       constructor.
    */
    void SetupControls (::Window* pParentWindow);

    /** This method is usually called exactly one time from the
        constructor.
    */
    void SetupListeners (void);

    /** Release the listeners that have been installed in SetupListeners().
    */
    void ReleaseListeners (void);

    /** This method overwrites the one from our base class:  We do our own
        scroll bar and the base class call is thus unnecessary.  It simply
        calls UpdateScrollBars(false).
    */
    virtual void UpdateScrollBars (void);
};

} } // end of namespace ::sd::slidesorter

#endif
