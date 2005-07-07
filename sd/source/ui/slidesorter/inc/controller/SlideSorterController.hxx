/*************************************************************************
 *
 *  $RCSfile: SlideSorterController.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:36:42 $
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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_CONTROLLER_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_CONTROLLER_HXX

#include "ViewShell.hxx"

#ifndef _SFX_SHELL_HXX
#include <sfx2/shell.hxx>
#endif
#ifndef _VIEWFAC_HXX
#include <sfx2/viewfac.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <memory>
#include <comphelper/implementationreference.hxx>

class TabBar;


namespace sd { namespace slidesorter {
class SlideSorterViewShell;
} }

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
class PageDescriptor;
} } }

namespace sd { namespace slidesorter { namespace controller {

class Clipboard;
class FocusManager;
class Listener;
class PageSelector;
class ScrollBarManager;
class SlotManager;

class SlideSorterController
{
public:
    /** Create a new controller for the slide sorter.
        @param pParentWindow
            The window that contains the controls of the new
            controller.
    */
    SlideSorterController (
        SfxViewFrame *pFrame,
        ::Window* pParentWindow,
        SlideSorterViewShell& rViewShell,
        model::SlideSorterModel& rModel,
        view::SlideSorterView& rView);

    /** Late initialization.  Call this method once a new new object has been
        created.
    */
    virtual void Init (void);

    virtual ~SlideSorterController (void);

    /** Place and size the scroll bars and the browser window so that the
        given rectangle is filled.
        @return
            The space occupied by the browser window is returned.
    */
    Rectangle Resize (const Rectangle& rAvailableSpace);

    /** Determine which of the UI elements--the scroll bars, the scroll bar
        filler, the actual slide sorter view--are visible and place them in
        the area last passed to Resize().
        @param bForce
            When <TRUE/> is given (<FALSE/> is the default) then the content
            window and with it the SlideSorterView is resized event when its
            size does not change (the size does change when the visibility
            of scroll bars changes.)
        @return
            Returns the space occupied by the browser window.
    */
    Rectangle Rearrange (bool bForce = false);

    SlideSorterViewShell& GetViewShell (void) const;
    model::SlideSorterModel& GetModel (void) const;
    view::SlideSorterView& GetView (void) const;

    /** Return the descriptor of the page that is rendered under the
        given position.
        @return
            Returns a pointer to a page descriptor instead of a
            reference because when no page is found at the position
            then NULL is returned to indicate this.
    */
    model::PageDescriptor* GetPageAt (const Point& rPixelPosition);

    /** Return the descriptor of the page that is associated to the page
        visible fade symbol at the given position.
        @return
            When the given position is not over a visible page fade symbol
            then NULL is returned.
    */
    model::PageDescriptor* GetFadePageAt (const Point& rPixelPosition);

    PageSelector& GetPageSelector (void);
    FocusManager& GetFocusManager (void);
    controller::Clipboard& GetClipboard (void);

    /** Return the object that manages the scroll bars.
    */
    ScrollBarManager& GetScrollBarManager (void);

    /** This method forwards the call to the SlideSorterView and executes
        pending operations like moving selected pages into the visible area.
    */
    void Paint (const Rectangle& rRect, ::sd::Window* pWin);

    /** The name of this method is taken from the ViewShell class.  It
        returns the page that (hopefully) has currently the attention of the
        user.
        @return
            When the slide sorter is not displayed in the center pane then
            the current page of the center pane view shell is returned.
            Other wise the focused page is returned or, if the focus
            indicator is not currently visible, the first selected page is
            returned.  If all that fails then NULL is returned.
    */
    SdPage* GetActualPage (void);

    void FuTemporary (SfxRequest& rRequest);
    void FuPermanent (SfxRequest& rRequest);
    void FuSupport (SfxRequest& rRequest);
    bool Command (
        const CommandEvent& rEvent,
        ::sd::Window* pWindow);

    void GetCtrlState (SfxItemSet &rSet);
    void GetMenuState (SfxItemSet &rSet);
    void GetStatusBarState (SfxItemSet& rSet);

    void ExecCtrl (SfxRequest& rRequest);
    void GetAttrState (SfxItemSet& rSet);
    void ExecStatusBar (SfxRequest& rRequest);

    /** Prepare for several model changes, i.e. prevent time-consuming and
        non-critical operations like repaints until UnlockModelChange() is
        called.  Ciritcal operations like releasing references to pages that
        do not exist anymore are executed.
    */
    void LockModelChange (void);

    /** Further calls to HandleModelChange() will result in a full featured
        update of model, view, and controller.  When HandleModelChange() has
        been called since the last LockModelChange() then this is done right
        away to bring the view up-to-date.
    */
    void UnlockModelChange (void);

    /** Create an object of this inner class to prevent updates due to model
        changes.
    */
    class ModelChangeLock
    {public:
        ModelChangeLock (SlideSorterController& rController);
        ~ModelChangeLock (void);
        void ModelHasChanged (void);
    private:
        SlideSorterController& mrController;
    };

    /** Prepare for a model change.  This method does all the things that
        need to be done _before_ the model changes, e.g. because they need
        access to the model data before the change.
    */
    void PreModelChange (void);

    /** Complete a model change.  This includes the recreation of data
        structures that depend on the model and the request for a repaint to
        show the changes.
    */
    void PostModelChange (void);

    /** Handle a change of the model, that is, handle the removal and
        insertion of whole pages or a change of the edit mode.

        This method is a convenience function that simply calls
        PreModelChange() and then PostModelChange().
    */
    void HandleModelChange (void);

    void DeleteSelectedPages (void);

    /** Move the maked pages to a position directly after the specified page.
    */
    bool MoveSelectedPages (USHORT nTargetPage);

    DECL_LINK(TabBarHandler, TabBar*);
    DECL_LINK(WindowEventHandler, VclWindowEvent*);

    /** Update the display of all pages.  This involves a redraw and
        releasing previews and caches.
    */
    void UpdateAllPages (void);

    /** Call this method after the selection has changed (possible several
        calls to the PageSelector) to invalidate the relevant slots and send
        appropriate events.
    */
    void SelectionHasChanged (bool bMakeSelectionVisible = true);

    enum SelectionHint { SH_FIRST, SH_LAST, SH_RECENT };

    /** Try to make all currently selected page objects visible, i.e. set
        the origin so that the page objects lie inside the visible area.
        When the selection is empty then the visible area is not modified.
        @param eSelectionHint
            This is an advice on which selected page object to handle with
            the highest priority when the whole selection does not fit in to
            the visible area.
        @return
            Returns the vertical translation of the visible area.  It is 0
            when no update of the visible area was done.
    */
    sal_Int32 MakeSelectionVisible (
        SelectionHint eSelectionHint = SH_RECENT);

    /** Modify the origin of the visible area so that the given rectangle
        comes into view.  This is done with the smallest change: no
        scrolling takes place when the given rectangle already lies in the
        visible area.  Otherwise either the top or the bottom of the given
        rectangle is aligned with the top or the bottom of the visible area.
        @return
            Returns the vertical translation of the visible area.  It is 0
            when no update of the visible area was done.
    */
    sal_Int32 MakeRectangleVisible (const Rectangle& rBox);

    /** Set the zoom factor.  The given value is clipped against an upper
        bound.
        @param nZoom
            An integer percent value, i.e. nZoom/100 is the actual zoom
            factor.
        */
    void SetZoom (long int nZoom);

    /** This factory method creates a selection function.
    */
    virtual FuPoor* CreateSelectionFunction (SfxRequest& rRequest);

    /** Add a listener that is called when the selection of the slide sorter
        changes.
        @param rListener
            When this method is called multiple times for the same listener
            the second and all following calls are ignored.  Each listener
            is added only once.
    */
    void AddSelectionChangeListener (const Link& rListener);

    /** Remove a listener that was called when the selection of the slide
        sorter changes.
        @param rListener
            It is save to pass a listener that was not added are has been
            removed previously.  Such calls are ignored.
    */
    void RemoveSelectionChangeListener (const Link& rListener);

    /** Prepare for a change of the edit mode.  Depending on the current
        edit mode we may save the selection so that it can be restored when
        later changing back to the current edit mode.
    */
    void PrepareEditModeChange (void);

    /** Set a new edit mode and return whether the edit mode really
        has been changed.  For proper saving and restoring of the selection
        this method should be called between calls to
        PrepareEditModeChange() and FinishEditModeChange().
        @return
            A return value of <TRUE/> indicates that the edit mode has
            changed.
    */
    bool ChangeEditMode (EditMode eEditMode);

    /** Finish the change of the edit mode.  Here we may select a page or
        restore a previously saved selection.
    */
    void FinishEditModeChange (void);

private:
    SlideSorterViewShell& mrViewShell;
    model::SlideSorterModel& mrModel;
    view::SlideSorterView& mrView;
    ::std::auto_ptr<PageSelector> mpPageSelector;
    ::std::auto_ptr<FocusManager> mpFocusManager;
    ::std::auto_ptr<SlotManager> mpSlotManager;
    ::std::auto_ptr<controller::Clipboard> mpClipboard;
    ::std::auto_ptr<ScrollBarManager> mpScrollBarManager;

    // The listener listens to UNO events and thus is a UNO object.
    // For proper life time management and at the same time free access to
    // the implementation object we use the ImplementationReference class.
    ::comphelper::ImplementationReference
        <controller::Listener,
        ::com::sun::star::uno::XInterface,
         ::com::sun::star::uno::XWeak>
        mpListener;

    int mnModelChangeLockCount;

    /** In this flag we remember whether a call to PreModelChange() has been
        made and one to PostModelChange() is pending.
    */
    bool mbPostModelChangePending;

    ::std::vector<Link> maSelectionChangeListeners;

    /** This array stores the indices of the  selected page descriptors at
        the time when the edit mode is switched to EM_MASTERPAGE.  With this
        we can restore the selection when switching back to EM_PAGE mode.
    */
    ::std::vector<SdPage*> maSelectionBeforeSwitch;
    /// The current page before the edit mode is switched to EM_MASTERPAGE.
    int mnCurrentPageBeforeSwitch;

    /** The master page to select after the edit mode is changed.  This
        member is used to pass the pointer from PrepareEditModeChange() to
        FinishEditModeChange().
    */
    SdPage* mpEditModeChangeMasterPage;

    /** This rectangle in the parent window encloses scroll bars and slide
        sorter window.  It is set when Resize() is called.
    */
    Rectangle maTotalWindowArea;

    /** When this flag is set then on the next call to Paint() the selection
        is moved into the visible area.
    */
    bool mbIsMakeSelectionVisiblePending;

    /** This counter is used to avoid processing of reentrant calls to
        Paint().
    */
    sal_Int32 mnPaintEntranceCount;

    /** Delete the given list of normal pages.  This method is a helper
        function for DeleteSelectedPages().
        @param rSelectedNormalPages
            A list of normal pages.  Supplying master pages is an error.
    */
    void DeleteSelectedNormalPages (const ::std::vector<SdPage*>& rSelectedNormalPages);

    /** Delete the given list of master pages.  This method is a helper
        function for DeleteSelectedPages().
        @param rSelectedMasterPages
            A list of master pages.  Supplying normal pages is an error.
    */
    void DeleteSelectedMasterPages (const ::std::vector<SdPage*>& rSelectedMasterPages);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
