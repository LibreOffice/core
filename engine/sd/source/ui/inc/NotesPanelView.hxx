/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "OutlineView.hxx"
#include <Outliner.hxx>
#include <unotools/weakref.hxx>

class SdrTextObj;

namespace sdtools
{
class EventMultiplexerEvent;
}

namespace sd
{
class DrawDocShell;
class NotesPanelViewShell;

/**
 * Derivative of ::sd::SimpleOutlinerView for the notes panel
|*
\************************************************************************/

class NotesPanelView final : public ::sd::SimpleOutlinerView
{
    NotesPanelViewShell& mrNotesPanelViewShell;
    SdOutliner maOutliner;
    OutlinerView maOutlinerView;

    Idle aModifyIdle;

    /// Called after every change to the notes text, including a refill on a slide change.
    Link<LinkParamNone*, void> maContentChangedHdl;

    bool mbInFocus = false;

    /// The notes object the outliner was filled from.
    ::unotools::WeakReference<SdrTextObj> mxEditedNotesObj;

    void getNotesFromDoc();
    void setNotesToDoc();
    SdrTextObj* getNotesTextObj();
    SdrTextObj* getEditedNotesObj();
    void clearPlaceholder();
    void commitNotes();
    void invalidateUndoState();
    bool isLocked();

public:
    NotesPanelView(DrawDocShell& rDocSh, vcl::Window* pWindow,
                   NotesPanelViewShell& rNotesPanelViewSh);
    virtual ~NotesPanelView() override;

    void Paint(const ::tools::Rectangle& rRect, ::sd::Window const* pWin);
    void onResize();
    void onGrabFocus();
    void onLoseFocus();
    bool isInFocus() const { return mbInFocus; }

    /// The notes pane of another view that has the focus on the notes shown here, or null. That
    /// view holds the notes, and this one only shows them.
    NotesPanelView* getLockingView();
    /// The view shell id of the view that holds the notes shown here, or -1 when no other view
    /// holds them or that view is not known. A view holds the notes when its notes pane has the
    /// focus on them, or when it edits the notes object itself, as on the handout page.
    sal_Int32 getLockingViewId();

    /// Reloads the notes in every notes pane other than pExcept that shows pNotesTextObj and does
    /// not edit it, so those panes show its current text and whether another view holds it.
    static void refreshViews(const SdrTextObj* pNotesTextObj, const NotesPanelView* pExcept);
    /// Makes every notes pane that holds pNotesTextObj write its text back and let the notes go.
    static void releaseViews(const SdrTextObj* pNotesTextObj);

    OutlinerView* GetOutlinerView();
    OutlinerView* GetViewByWindow(vcl::Window const* pWin) const override;

    SdOutliner& GetOutliner() { return maOutliner; }

    void SetContentChangedHdl(const Link<LinkParamNone*, void>& rLink)
    {
        maContentChangedHdl = rLink;
    }

    void FillOutliner();
    void onUpdateStyleSettings();
    virtual SvtScriptType GetScriptType() const override;

    void SetLinks();
    void ResetLinks();

    virtual void GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr = false) const override;
    virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll = false,
                               bool bSlide = false, bool bMaster = false) override;

    // SdrObjEditView's Outliner access overrides to use TextObjectBar implementations.
    virtual const SdrOutliner* GetTextEditOutliner() const override { return &maOutliner; }
    virtual SdrOutliner* GetTextEditOutliner() override { return &maOutliner; }
    virtual const OutlinerView* GetTextEditOutlinerView() const override { return &maOutlinerView; }
    virtual OutlinerView* GetTextEditOutlinerView() override { return &maOutlinerView; }

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                SdrLayerID nLayer) override;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt, ::sd::Window* pTargetWindow,
                                 sal_uInt16 nPage, SdrLayerID nLayer) override;

    DECL_LINK(StatusEventHdl, EditStatus&, void);
    DECL_LINK(EditModifiedHdl, LinkParamNone*, void);
    DECL_LINK(ModifyTimerHdl, Timer*, void);
    DECL_LINK(EventMultiplexerListener, sdtools::EventMultiplexerEvent&, void);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
