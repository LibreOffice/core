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
#include "sddllapi.h"
#include <memory>
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
    SFX_DECL_INTERFACE(SD_IF_SDSLIDESORTERVIEWSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    static std::shared_ptr<SlideSorterViewShell> Create(
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        vcl::Window* pParentWindow,
        FrameView* pFrameView,
        const bool bIsCenterPane);

    virtual ~SlideSorterViewShell();

    /** Late initialization that has to be called after a new instance has
        completed its construction.
    */
    virtual void Init (bool bIsMainViewShell) override;

    /** Return a slide sorter that is currently displayed in one of the
        panes that belong to the given ViewShellBase object.
        When there is only one slide sorter visible then that one is
        returned.  When two (or more) are visible then the one in the center
        pane is returned.  When no slidesorter is visible then NULL is
        returned.
    */
    // Exported for unit test
    SD_DLLPUBLIC static SlideSorterViewShell* GetSlideSorter(ViewShellBase& rBase);

    virtual SdPage* GetActualPage() override;

    /// inherited from sd::ViewShell
    virtual SdPage* getCurrentPage() const override;

    void ExecCtrl (SfxRequest& rRequest);
    void GetCtrlState (SfxItemSet &rSet);
    void FuSupport (SfxRequest& rRequest);
    void FuTemporary (SfxRequest& rRequest);
    void GetStatusBarState (SfxItemSet& rSet);
    void FuPermanent (SfxRequest& rRequest);
    void GetAttrState (SfxItemSet& rSet);
    static void ExecStatusBar (SfxRequest& rRequest);
    virtual void Command (const CommandEvent& rEvent, ::sd::Window* pWindow) override;
    void GetMenuState (SfxItemSet &rSet);
    void GetClipboardState (SfxItemSet &rSet);

    virtual void ReadFrameViewData (FrameView* pView) override;
    virtual void WriteFrameViewData() override;

    /** Set the zoom factor.  The given value is clipped against an upper
        bound.
        @param nZoom
            An integer percent value, i.e. nZoom/100 is the actual zoom
            factor.
        */
    virtual void SetZoom (long int nZoom) override;
    virtual void SetZoomRect (const Rectangle& rZoomRect) override;

    /** This is a callback method used by the active window to delegate its
        Paint() call to.  This view shell itself delegates it to the view.
    */
    virtual void Paint(const Rectangle& rRect, ::sd::Window* pWin) override;

    /** Place and size the controls and windows.  You may want to call this
        method when something has changed that for instance affects the
        visibility state of the scroll bars.
    */
    virtual void ArrangeGUIElements() override;

    virtual void Activate (bool IsMDIActivate) override;
    virtual void Deactivate (bool IsMDIActivate) override;

    /** Move slides up and down. Mainly uno commands. */
    void ExecMovePageUp (SfxRequest& rReq);
    void GetStateMovePageUp (SfxItemSet& rSet);

    void ExecMovePageDown (SfxRequest& rReq);
    void GetStateMovePageDown (SfxItemSet& rSet);

    void ExecMovePageFirst (SfxRequest& rReq);
    void GetStateMovePageFirst (SfxItemSet& rSet);

    void ExecMovePageLast (SfxRequest& rReq);
    void GetStateMovePageLast (SfxItemSet& rSet);


    //===== Drag and Drop =====================================================

    void StartDrag (
        const Point& rDragPt,
        vcl::Window* pWindow );
    virtual sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow,
        sal_uInt16 nPage,
        sal_uInt16 nLayer ) override;
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow,
        sal_uInt16 nPage,
        sal_uInt16 nLayer) override;

    typedef ::std::vector<SdPage*> PageSelection;

    /** Return the set of selected pages.
    */
    std::shared_ptr<PageSelection> GetPageSelection() const;

    void SetPageSelection (const std::shared_ptr<PageSelection>& rSelection);

    /** Add a listener that is called when the selection of the slide sorter
        changes.
        @param rListener
            When this method is called multiple times for the same listener
            the second and all following calls are ignored.  Each listener
            is added only once.
    */
    void AddSelectionChangeListener (const Link<LinkParamNone*,void>& rListener);

    /** Remove a listener that was called when the selection of the slide
        sorter changes.
        @param rListener
            It is save to pass a listener that was not added are has been
            removed previously.  Such calls are ignored.
    */
    void RemoveSelectionChangeListener (const Link<LinkParamNone*,void>& rListener);

    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController() override;

    /** Create an accessible object representing the specified window.
        @param pWindow
            The returned object makes the document displayed in this window
            accessible.
        @return
            Returns an <type>AccessibleSlideSorterView</type> object.
   */
    virtual css::uno::Reference<css::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow) override;
    // handle SlideSorterView specially because AccessibleSlideSorterView doesn't inherit from AccessibleDocumentViewBase
    virtual void SwitchViewFireFocus( const css::uno::Reference< css::accessibility::XAccessible >& xAcc ) override;

    // Exported for unit test
    SD_DLLPUBLIC SlideSorter& GetSlideSorter() const;

    /** Try to relocate all toplevel window elements to the given parent
        window.
    */
    virtual bool RelocateToParentWindow (vcl::Window* pParentWindow) override;

protected:

    /** Override this method to handle a missing tool bar correctly.
        This is the case when the slide sorter is not the main view shell.
    */
    virtual ::svl::IUndoManager* ImpGetUndoManager() const override;

private:
    std::shared_ptr<SlideSorter> mpSlideSorter;
    bool mbIsArrangeGUIElementsPending;

    SlideSorterViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        vcl::Window* pParentWindow,
        FrameView* pFrameView);
    void Initialize();

    /** This method overwrites the one from our base class:  We do our own
        scroll bar and the base class call is thus unnecessary.  It simply
        calls UpdateScrollBars(false).
    */
    virtual void UpdateScrollBars() override;

    void PostMoveSlidesActions(const std::shared_ptr<SlideSorterViewShell::PageSelection> &rpSelection);

    void MainViewEndEditAndUnmarkAll();

    /** Select the same pages in the document as are selected in the
        SlideSorterViewShell

        return the page numbers of the first and last selected pages
    */
    std::pair<sal_uInt16, sal_uInt16> SyncPageSelectionToDocument(const std::shared_ptr<SlideSorterViewShell::PageSelection> &rpSelection);
};

typedef std::shared_ptr<SlideSorterViewShell::PageSelection> SharedPageSelection;

} } // end of namespace ::sd::slidesorter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
