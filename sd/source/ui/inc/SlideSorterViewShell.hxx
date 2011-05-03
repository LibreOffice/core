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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_VIEW_SHELL_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_VIEW_SHELL_HXX

#include "ViewShell.hxx"
#include "glob.hxx"
#include <sfx2/shell.hxx>
#include <sfx2/viewfac.hxx>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace sd { namespace slidesorter { namespace controller {
class SlotManager;
} } }


namespace sd { namespace slidesorter {

class SlideSorter;

class SlideSorterViewShell
    : public ViewShell
{
    friend class controller::SlotManager;

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDSLIDESORTERVIEWSHELL)

    static ::boost::shared_ptr<SlideSorterViewShell> Create(
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::Window* pParentWindow,
        FrameView* pFrameView,
        const bool bIsCenterPane);

    virtual ~SlideSorterViewShell (void);

    /** Late initialization that has to be called after a new instance has
        completed its construction.
    */
    virtual void Init (bool bIsMainViewShell);

    /** Return a slide sorter that is currently displayed in one of the
        panes that belong to the given ViewShellBase object.
        When there is only one slide sorter visible then that one is
        returned.  When two (or more) are visible then the one in the center
        pane is returned.  When no slidesorter is visible then NULL is
        returned.
    */
    static SlideSorterViewShell* GetSlideSorter (ViewShellBase& rBase);

    virtual void GetFocus (void);
    virtual void LoseFocus (void);
    virtual SdPage* GetActualPage (void);

    /// inherited from sd::ViewShell
    virtual SdPage* getCurrentPage() const;

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
    virtual void GetClipboardState (SfxItemSet &rSet);

    virtual void ReadFrameViewData (FrameView* pView);
    virtual void WriteFrameViewData (void);

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

    virtual void Activate (sal_Bool IsMDIActivate);

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
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND );
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND);

    typedef ::std::vector<SdPage*> PageSelection;

    /** Return the set of selected pages.
    */
    ::boost::shared_ptr<PageSelection> GetPageSelection (void) const;

    void SetPageSelection (const ::boost::shared_ptr<PageSelection>& rSelection);

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

    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController (void);

    /** Create an accessible object representing the specified window.
        @param pWindow
            The returned object makes the document displayed in this window
            accessible.
        @return
            Returns an <type>AccessibleSlideSorterView</type> object.
   */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow);

    SlideSorter& GetSlideSorter (void) const;

    /** Try to relocate all toplevel window elements to the given parent
        window.
    */
    virtual bool RelocateToParentWindow (::Window* pParentWindow);

protected:

    virtual SvBorder GetBorder (bool bOuterResize);

    /** This method is overloaded to handle a missing tool bar correctly.
        This is the case when the slide sorter is not the main view shell.
    */
    virtual ::svl::IUndoManager* ImpGetUndoManager (void) const;

private:
    ::boost::shared_ptr<SlideSorter> mpSlideSorter;
    bool mbIsArrangeGUIElementsPending;

    SlideSorterViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::Window* pParentWindow,
        FrameView* pFrameView);
    void Initialize (void);

    /** This method overwrites the one from our base class:  We do our own
        scroll bar and the base class call is thus unnecessary.  It simply
        calls UpdateScrollBars(false).
    */
    virtual void UpdateScrollBars (void);
};

typedef ::boost::shared_ptr<SlideSorterViewShell::PageSelection> SharedPageSelection;

} } // end of namespace ::sd::slidesorter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
