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

#ifndef INCLUDED_SD_SOURCE_UI_INC_SLIDESORTERVIEWSHELL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_SLIDESORTERVIEWSHELL_HXX

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
    TYPEINFO_OVERRIDE();
    SFX_DECL_INTERFACE(SD_IF_SDSLIDESORTERVIEWSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    static ::boost::shared_ptr<SlideSorterViewShell> Create(
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::vcl::Window* pParentWindow,
        FrameView* pFrameView,
        const bool bIsCenterPane);

    virtual ~SlideSorterViewShell (void);

    /** Late initialization that has to be called after a new instance has
        completed its construction.
    */
    virtual void Init (bool bIsMainViewShell) SAL_OVERRIDE;

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
    virtual SdPage* GetActualPage (void) SAL_OVERRIDE;

    /// inherited from sd::ViewShell
    virtual SdPage* getCurrentPage() const SAL_OVERRIDE;

    void ExecCtrl (SfxRequest& rRequest);
    virtual void GetCtrlState (SfxItemSet &rSet);
    virtual void FuSupport (SfxRequest& rRequest);
    virtual void FuTemporary (SfxRequest& rRequest);
    virtual void GetStatusBarState (SfxItemSet& rSet);
    virtual void FuPermanent (SfxRequest& rRequest);
    void GetAttrState (SfxItemSet& rSet);
    void ExecStatusBar (SfxRequest& rRequest);
    virtual void Command (const CommandEvent& rEvent, ::sd::Window* pWindow) SAL_OVERRIDE;
    virtual void GetMenuState (SfxItemSet &rSet);
    virtual void GetClipboardState (SfxItemSet &rSet);

    virtual void ReadFrameViewData (FrameView* pView) SAL_OVERRIDE;
    virtual void WriteFrameViewData (void) SAL_OVERRIDE;

    /** Set the zoom factor.  The given value is clipped against an upper
        bound.
        @param nZoom
            An integer percent value, i.e. nZoom/100 is the actual zoom
            factor.
        */
    virtual void SetZoom (long int nZoom) SAL_OVERRIDE;
    virtual void SetZoomRect (const Rectangle& rZoomRect) SAL_OVERRIDE;

    /** This is a callback method used by the active window to delegate its
        Paint() call to.  This view shell itself delegates it to the view.
    */
    virtual void Paint(const Rectangle& rRect, ::sd::Window* pWin) SAL_OVERRIDE;

    /** Place and size the controls and windows.  You may want to call this
        method when something has changed that for instance affects the
        visibility state of the scroll bars.
    */
    virtual void ArrangeGUIElements (void) SAL_OVERRIDE;

    virtual void Activate (bool IsMDIActivate) SAL_OVERRIDE;
    virtual void Deactivate (bool IsMDIActivate) SAL_OVERRIDE;

    //===== Drag and Drop =====================================================

    virtual void StartDrag (
        const Point& rDragPt,
        ::vcl::Window* pWindow );
    virtual void DragFinished (
        sal_Int8 nDropAction);
    virtual sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND ) SAL_OVERRIDE;
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND) SAL_OVERRIDE;

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

    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController (void) SAL_OVERRIDE;

    /** Create an accessible object representing the specified window.
        @param pWindow
            The returned object makes the document displayed in this window
            accessible.
        @return
            Returns an <type>AccessibleSlideSorterView</type> object.
   */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow) SAL_OVERRIDE;
    // handle SlideSorterView specially because AccessibleSlideSorterView doesn't inherit from AccessibleDocumentViewBase
    virtual void SwitchViewFireFocus( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc ) SAL_OVERRIDE;

    SlideSorter& GetSlideSorter (void) const;

    /** Try to relocate all toplevel window elements to the given parent
        window.
    */
    virtual bool RelocateToParentWindow (::vcl::Window* pParentWindow) SAL_OVERRIDE;

protected:

    virtual SvBorder GetBorder (bool bOuterResize);

    /** This method is overloaded to handle a missing tool bar correctly.
        This is the case when the slide sorter is not the main view shell.
    */
    virtual ::svl::IUndoManager* ImpGetUndoManager (void) const SAL_OVERRIDE;

private:
    ::boost::shared_ptr<SlideSorter> mpSlideSorter;
    bool mbIsArrangeGUIElementsPending;

    SlideSorterViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        vcl::Window* pParentWindow,
        FrameView* pFrameView);
    void Initialize (void);

    /** This method overwrites the one from our base class:  We do our own
        scroll bar and the base class call is thus unnecessary.  It simply
        calls UpdateScrollBars(false).
    */
    virtual void UpdateScrollBars (void) SAL_OVERRIDE;
};

typedef ::boost::shared_ptr<SlideSorterViewShell::PageSelection> SharedPageSelection;

} } // end of namespace ::sd::slidesorter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
