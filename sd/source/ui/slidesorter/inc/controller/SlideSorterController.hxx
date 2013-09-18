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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_CONTROLLER_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_CONTROLLER_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include "ViewShell.hxx"

#include <com/sun/star/drawing/XDrawPages.hpp>

#include <sfx2/shell.hxx>
#include <sfx2/viewfac.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <comphelper/implementationreference.hxx>

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace controller {

class Animator;
class Clipboard;
class CurrentSlideManager;
class FocusManager;
class InsertionIndicatorHandler;
class Listener;
class PageSelector;
class ScrollBarManager;
class SelectionFunction;
class SelectionManager;
class SlotManager;
class VisibleAreaManager;

class SlideSorterController
{
public:
    /** Create a new controller for the slide sorter.
        @param pParentWindow
            The window that contains the controls of the new
            controller.
    */
    SlideSorterController (SlideSorter& rSlideSorter);

    /** Late initialization.  Call this method once a new new object has been
        created.
    */
    virtual void Init (void);

    virtual ~SlideSorterController (void);

    void Dispose (void);

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

    /** Return the descriptor of the page that is rendered under the
        given position.  This takes the IsOnlyPreviewTriggersMouseOver
        property into account.
        @return
            Returns a pointer to a page descriptor instead of a
            reference because when no page is found at the position
            then NULL is returned to indicate this.
    */
    model::SharedPageDescriptor GetPageAt (const Point& rPixelPosition);

    PageSelector& GetPageSelector (void);
    FocusManager& GetFocusManager (void);
    controller::Clipboard& GetClipboard (void);

    /** Return the object that manages the scroll bars.
    */
    ScrollBarManager& GetScrollBarManager (void);

    ::boost::shared_ptr<CurrentSlideManager> GetCurrentSlideManager (void) const;
    ::boost::shared_ptr<SlotManager> GetSlotManager (void) const;
    ::boost::shared_ptr<SelectionManager> GetSelectionManager (void) const;
    ::boost::shared_ptr<InsertionIndicatorHandler> GetInsertionIndicatorHandler (void) const;

    /** This method forwards the call to the SlideSorterView and executes
        pending operations like moving selected pages into the visible area.
    */
    void Paint (const Rectangle& rRect, ::Window* pWin);

    void FuTemporary (SfxRequest& rRequest);
    void FuPermanent (SfxRequest& rRequest);
    void FuSupport (SfxRequest& rRequest);
    bool Command (
        const CommandEvent& rEvent,
        ::sd::Window* pWindow);

    void GetCtrlState (SfxItemSet &rSet);
    void GetStatusBarState (SfxItemSet& rSet);

    void ExecCtrl (SfxRequest& rRequest);
    void GetAttrState (SfxItemSet& rSet);
    void ExecStatusBar (SfxRequest& rRequest);

    bool IsLocked (void) const;

    /** Create an object of this inner class to prevent updates due to model
        changes.
    */
    class ModelChangeLock
    {public:
        ModelChangeLock (SlideSorterController& rController);
        ~ModelChangeLock (void);
        void Release (void);
    private:
        SlideSorterController* mpController;
    };
    friend class ModelChangeLock;


    /** Handle a change of the model, that is, handle the removal and
        insertion of whole pages or a change of the edit mode.

        This method is a convenience function that simply calls
        PreModelChange() and then PostModelChange().
    */
    void HandleModelChange (void);

    DECL_LINK(WindowEventHandler, VclWindowEvent*);

    /** Update the display of all pages.  This involves a redraw and
        releasing previews and caches.
    */
    void UpdateAllPages (void);

    /** This factory method creates a selection function.
    */
    virtual FunctionReference CreateSelectionFunction (SfxRequest& rRequest);

    /** When the current function of the view shell is the slide sorter
        selection function then return a reference to it.  Otherwise return
        an empty reference.
    */
    ::rtl::Reference<SelectionFunction> GetCurrentSelectionFunction (void);

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

    /** Call this method when the name of one of the pages has changed.
        This is then notified to the accessibility object, when that exists.
        @param nPageIndex
            The index of the page whose name has been changed.
        @param rsOldName
            The old name of the page.  The new name can be taken from the
            page object.
    */
    void PageNameHasChanged (int nPageIndex, const OUString& rsOldName);

    /** Provide the set of pages to be displayed in the slide sorter.  The
        GetDocumentSlides() method can be found only in the SlideSorterModel.
    */
    void SetDocumentSlides (const css::uno::Reference<css::container::XIndexAccess>& rxSlides);

    /** Return an Animator object.
    */
    ::boost::shared_ptr<Animator> GetAnimator (void) const;

    VisibleAreaManager& GetVisibleAreaManager (void) const;

    void CheckForMasterPageAssignment (void);
    void CheckForSlideTransitionAssignment (void);

private:
    SlideSorter& mrSlideSorter;
    model::SlideSorterModel& mrModel;
    view::SlideSorterView& mrView;
    ::boost::scoped_ptr<PageSelector> mpPageSelector;
    ::boost::scoped_ptr<FocusManager> mpFocusManager;
    ::boost::shared_ptr<SlotManager> mpSlotManager;
    ::boost::scoped_ptr<ScrollBarManager> mpScrollBarManager;
    mutable ::boost::shared_ptr<CurrentSlideManager> mpCurrentSlideManager;
    ::boost::shared_ptr<SelectionManager> mpSelectionManager;
    ::boost::scoped_ptr<controller::Clipboard> mpClipboard;
    ::boost::shared_ptr<InsertionIndicatorHandler> mpInsertionIndicatorHandler;
    ::boost::shared_ptr<Animator> mpAnimator;
    ::boost::scoped_ptr<VisibleAreaManager> mpVisibleAreaManager;

    // The listener listens to UNO events and thus is a UNO object.
    // For proper life time management and at the same time free access to
    // the implementation object we use the ImplementationReference class.
    ::rtl::Reference<controller::Listener> mpListener;

    int mnModelChangeLockCount;
    bool mbIsForcedRearrangePending;

    bool mbPreModelChangeDone;
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

    /** This counter is used to avoid processing of reentrant calls to
        Paint().
    */
    sal_Int32 mnPaintEntranceCount;

    /** Remember whether the context menu is open.
    */
    bool mbIsContextMenuOpen;

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
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
