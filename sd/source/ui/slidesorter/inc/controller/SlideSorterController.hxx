/*************************************************************************
 *
 *  $RCSfile: SlideSorterController.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:19:05 $
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

    /** Return the page that currently has the focus or is the first page of
        the selection.
        @return
            When there is no focus and the selection is empty then NULL is
            returned.
    */
    virtual SdPage* GetActualPage (void);

    virtual void FuTemporary (SfxRequest& rRequest);
    virtual void FuPermanent (SfxRequest& rRequest);
    virtual void FuSupport (SfxRequest& rRequest);
    virtual bool Command (
        const CommandEvent& rEvent,
        ::sd::Window* pWindow);

    virtual void GetCtrlState (SfxItemSet &rSet);
    virtual void GetMenuState (SfxItemSet &rSet);
    virtual void GetStatusBarState (SfxItemSet& rSet);

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
    void MoveSelectedPages (USHORT nTargetPage);

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
        the zoom factor and origin so that the page objects lie inside the
        visible area.
        @param eSelectionHint
            This is an advice on which selected page object is handled with
            the highest priority when the whole selection does not fit in to
            the visible area.
    */
    void MakeSelectionVisible (
        SelectionHint eSelectionHint = SH_RECENT);

    void MakeRectangleVisible (const Rectangle& rBox);

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
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
