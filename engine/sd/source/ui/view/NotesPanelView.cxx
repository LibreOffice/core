/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <NotesPanelViewShell.hxx>
#include <NotesPanelView.hxx>
#include <OutlineView.hxx>
#include <ViewShellBase.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outliner.hxx>
#include <sdresid.hxx>
#include <editeng/editund2.hxx>
#include <sdpage.hxx>
#include <DrawViewShell.hxx>
#include <DrawDocShell.hxx>
#include <ToolBarManager.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <sdmod.hxx>
#include <officecfg/Office/Common.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdviter.hxx>
#include <svx/svxids.hrc>
#include <tools/lazydelete.hxx>
#include <EventMultiplexer.hxx>
#include <app.hrc>
#include <strings.hrc>

#include <span>
#include <vector>

namespace sd
{
namespace
{
/// The notes panes that exist, in any view of any document. Null once VCL has shut down.
std::vector<NotesPanelView*>* notesPanelViewRegistry()
{
    static ::tools::DeleteOnDeinit<std::vector<NotesPanelView*>> aViews;
    return aViews.get();
}

/// Every notes pane that exists, in any view of any document.
std::span<NotesPanelView* const> allNotesPanelViews()
{
    std::vector<NotesPanelView*>* pViews = notesPanelViewRegistry();
    if (!pViews)
        return {};
    return *pViews;
}
}

NotesPanelView::NotesPanelView(DrawDocShell& rDocSh, vcl::Window* pWindow,
                               NotesPanelViewShell& rNotesPanelViewShell)
    : ::sd::SimpleOutlinerView(*rDocSh.GetDoc(), pWindow->GetOutDev(), &rNotesPanelViewShell)
    , mrNotesPanelViewShell(rNotesPanelViewShell)
    , maOutliner(mrDoc, OutlinerMode::TextObject)
    , maOutlinerView(maOutliner, pWindow)
    , aModifyIdle("NotesEditWindow ModifyIdle")
{
    aModifyIdle.SetInvokeHandler(LINK(this, NotesPanelView, ModifyTimerHdl));
    aModifyIdle.SetPriority(TaskPriority::LOWEST);

    maOutliner.Init(OutlinerMode::OutlineView);
    maOutliner.SetRefDevice(SdModule::get()->GetVirtualRefDevice());
    maOutliner.SetPaperSize(mrNotesPanelViewShell.GetActiveWindow()->GetViewSize());

    maOutlinerView.SetOutputArea(
        ::tools::Rectangle{ Point(0, 0), mrNotesPanelViewShell.GetActiveWindow()->GetViewSize() });
    maOutliner.InsertView(&maOutlinerView, EE_APPEND);

    onUpdateStyleSettings();

    // fill Outliner with contents
    FillOutliner();

    if (std::vector<NotesPanelView*>* pViews = notesPanelViewRegistry())
        pViews->push_back(this);

    mrNotesPanelViewShell.GetViewShellBase().GetEventMultiplexer()->AddEventListener(
        LINK(this, NotesPanelView, EventMultiplexerListener));

    // TODO: UNDO
    // sd::UndoManager* pDocUndoMgr = dynamic_cast<sd::UndoManager*>(mpDocSh->GetUndoManager());
    // if (pDocUndoMgr != nullptr)
    //     pDocUndoMgr->SetLinkedUndoManager(&maOutliner.GetUndoManager());
}

NotesPanelView::~NotesPanelView()
{
    if (std::vector<NotesPanelView*>* pViews = notesPanelViewRegistry())
        std::erase(*pViews, this);

    // The notes this view was editing keep its last typing and are free for the other views again.
    if (mbInFocus)
    {
        mbInFocus = false;
        commitNotes();
        refreshViews(getEditedNotesObj(), this);
    }

    mrNotesPanelViewShell.GetViewShellBase().GetEventMultiplexer()->RemoveEventListener(
        LINK(this, NotesPanelView, EventMultiplexerListener));

    ResetLinks();
    // DisconnectFromApplication();
    // mpProgress.reset();
}

void NotesPanelView::FillOutliner()
{
    SdrTextObj* pPreviousNotesObj = getEditedNotesObj();
    const bool bWasInFocus = mbInFocus;

    if (mbInFocus)
        commitNotes();

    maOutliner.GetUndoManager().Clear();
    invalidateUndoState();
    maOutliner.EnableUndo(false);
    ResetLinks();
    maOutliner.Clear();
    mxEditedNotesObj.clear();

    SdrTextObj* pNotesTextObj = getNotesTextObj();
    if (!pNotesTextObj)
    {
        maOutlinerView.SetReadOnly(false);
        if (bWasInFocus)
            refreshViews(pPreviousNotesObj, this);
        return;
    }

    getNotesFromDoc();
    SetLinks();
    maOutliner.EnableUndo(true);

    // Notes that another view is editing are only shown here. A view that comes to them with the
    // focus, for example on a slide change, stops editing.
    const bool bLocked = isLocked();
    if (bLocked)
        mbInFocus = false;
    maOutlinerView.SetReadOnly(bLocked);

    if (mbInFocus)
        clearPlaceholder();

    maContentChangedHdl.Call(nullptr);

    // The other views that show the notes this view left, or the notes it now edits, show who
    // holds them now.
    if (bWasInFocus && pPreviousNotesObj != pNotesTextObj)
        refreshViews(pPreviousNotesObj, this);
    if (mbInFocus)
        refreshViews(pNotesTextObj, this);
}

NotesPanelView* NotesPanelView::getLockingView()
{
    SdrTextObj* pNotesTextObj = getEditedNotesObj();
    if (!pNotesTextObj)
        return nullptr;

    for (NotesPanelView* pView : allNotesPanelViews())
    {
        if (pView != this && pView->mbInFocus && pView->getEditedNotesObj() == pNotesTextObj)
            return pView;
    }
    return nullptr;
}

/// The notes object is in text edit only when a view edits it directly, as the handout page
/// does. The notes panes edit a copy of its text.
bool NotesPanelView::isLocked()
{
    if (getLockingView())
        return true;
    SdrTextObj* pNotesTextObj = getEditedNotesObj();
    return pNotesTextObj && pNotesTextObj->IsInEditMode();
}

sal_Int32 NotesPanelView::getLockingViewId()
{
    if (NotesPanelView* pLockingView = getLockingView())
        return pLockingView->mrNotesPanelViewShell.GetViewShellBase().GetViewShellId().get();

    SdrTextObj* pNotesTextObj = getEditedNotesObj();
    if (!pNotesTextObj || !pNotesTextObj->IsInEditMode())
        return -1;

    sal_Int32 nViewShellId = -1;
    SdrViewIter::ForAllViews(pNotesTextObj, [&nViewShellId, pNotesTextObj](SdrView* pView) {
        if (nViewShellId >= 0 || pView->GetTextEditObject() != pNotesTextObj)
            return;
        if (SfxViewShell* pViewShell = pView->GetSfxViewShell())
            nViewShellId = pViewShell->GetViewShellId().get();
    });
    return nViewShellId;
}

void NotesPanelView::refreshViews(const SdrTextObj* pNotesTextObj, const NotesPanelView* pExcept)
{
    if (!pNotesTextObj)
        return;

    for (NotesPanelView* pView : allNotesPanelViews())
    {
        if (pView != pExcept && !pView->mbInFocus && pView->getEditedNotesObj() == pNotesTextObj)
            pView->FillOutliner();
    }
}

void NotesPanelView::releaseViews(const SdrTextObj* pNotesTextObj)
{
    if (!pNotesTextObj)
        return;

    for (NotesPanelView* pView : allNotesPanelViews())
    {
        if (pView->mbInFocus && pView->getEditedNotesObj() == pNotesTextObj)
            pView->onLoseFocus();
    }
}

SdrTextObj* NotesPanelView::getNotesTextObj()
{
    SdPage* pNotesPage = mrNotesPanelViewShell.getCurrentPage();
    if (!pNotesPage)
        return nullptr;

    SdrObject* pNotesObj = pNotesPage->GetPresObj(PresObjKind::Notes);
    if (!pNotesObj)
        return nullptr;

    return dynamic_cast<SdrTextObj*>(pNotesObj);
}

SdrTextObj* NotesPanelView::getEditedNotesObj() { return mxEditedNotesObj.get().get(); }

void NotesPanelView::SetLinks()
{
    maOutliner.SetStatusEventHdl(LINK(this, NotesPanelView, StatusEventHdl));
}

void NotesPanelView::ResetLinks() { maOutliner.SetStatusEventHdl(Link<EditStatus&, void>()); }

void NotesPanelView::getNotesFromDoc()
{
    SdrTextObj* pNotesTextObj = getNotesTextObj();
    if (!pNotesTextObj)
        return;

    mxEditedNotesObj = pNotesTextObj;

    // Ignore notifications that will rebound from updating the text
    maOutliner.SetModifyHdl(Link<LinkParamNone*, void>());

    if (OutlinerParaObject* pPara = pNotesTextObj->GetOutlinerParaObject())
        maOutliner.SetText(*pPara);

    maOutliner.SetModifyHdl(LINK(this, NotesPanelView, EditModifiedHdl));
}

void NotesPanelView::setNotesToDoc()
{
    SdrTextObj* pNotesTextObj = getEditedNotesObj();
    if (!pNotesTextObj)
        return;

    std::optional<OutlinerParaObject> pNewText = maOutliner.CreateParaObject();
    pNotesTextObj->SetOutlinerParaObject(std::move(pNewText));
    if (pNotesTextObj->IsEmptyPresObj())
        pNotesTextObj->SetEmptyPresObj(false);
}

void NotesPanelView::Paint(const ::tools::Rectangle& rRect, ::sd::Window const* /*pWin*/)
{
    maOutlinerView.DrawText_ToEditView(rRect);
}

OutlinerView* NotesPanelView::GetOutlinerView() { return &maOutlinerView; }

void NotesPanelView::onUpdateStyleSettings()
{
    svtools::ColorConfig aColorConfig;
    const Color aDocColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);

    maOutlinerView.SetBackgroundColor(aDocColor);
    if (vcl::Window* pWindow = maOutlinerView.GetWindow())
        pWindow->SetBackground(Wallpaper(aDocColor));

    maOutliner.SetBackgroundColor(aDocColor);
}

void NotesPanelView::onResize()
{
    ::sd::Window* pWin = mrNotesPanelViewShell.GetActiveWindow();
    if (!pWin)
        return;

    OutlinerView* pOutlinerView = GetOutlinerView();
    if (!pOutlinerView)
        return;

    Size aOutputSize = pWin->PixelToLogic(pWin->GetOutputSizePixel());

    pOutlinerView->SetOutputArea({ Point(0, 0), aOutputSize });
    maOutliner.SetPaperSize(aOutputSize);
    pOutlinerView->ShowCursor();

    const ::tools::Long nMaxVisAreaStart = maOutliner.GetTextHeight() - aOutputSize.Height();

    ::tools::Rectangle aVisArea(pOutlinerView->GetVisArea());

    if (aVisArea.Top() > nMaxVisAreaStart)
    {
        aVisArea.SetTop(std::max<::tools::Long>(nMaxVisAreaStart, 0));
        aVisArea.SetSize(aOutputSize);
        pOutlinerView->SetVisArea(aVisArea);
        pOutlinerView->ShowCursor();
    }

    if (!aVisArea.IsEmpty()) // not when opening
    {
        mrNotesPanelViewShell.InitWindows(Point(0, 0), aVisArea.GetSize(), aVisArea.TopLeft(),
                                          true);
        mrNotesPanelViewShell.UpdateScrollBars();
    }
}

void NotesPanelView::onGrabFocus()
{
    // Notes that another view is editing stay as they are until that view leaves them.
    if (mbInFocus || isLocked())
        return;

    // The notes object can change without this view being told, for example by an undo in
    // another view, so the copy here is read again when it differs.
    if (SdrTextObj* pNotesTextObj = getEditedNotesObj())
    {
        const OutlinerParaObject* pDocText = pNotesTextObj->GetOutlinerParaObject();
        const std::optional<OutlinerParaObject> pPaneText = maOutliner.CreateParaObject();
        if (pDocText && (!pPaneText || *pDocText != *pPaneText))
            FillOutliner();
    }
    mbInFocus = true;

    clearPlaceholder();
    invalidateUndoState();
    refreshViews(getEditedNotesObj(), this);
}

void NotesPanelView::onLoseFocus()
{
    if (!mbInFocus)
        return;
    mbInFocus = false;

    commitNotes();

    // Notes left empty show the placeholder text again.
    if (!maOutliner.GetEditEngine().HasText())
        FillOutliner();

    invalidateUndoState();
    refreshViews(getEditedNotesObj(), this);
}

/// While the notes have the focus, the Undo and Redo state comes from the notes history, so it
/// changes with each edit and with each focus change.
void NotesPanelView::invalidateUndoState()
{
    SfxViewFrame* pViewFrame = mrNotesPanelViewShell.GetViewFrame();
    if (!pViewFrame)
        return;

    static const sal_uInt16 aUndoSlots[]
        = { SID_REDO, SID_UNDO, SID_GETUNDOSTRINGS, SID_GETREDOSTRINGS, 0 };
    pViewFrame->GetBindings().Invalidate(aUndoSlots);
}

void NotesPanelView::clearPlaceholder()
{
    SdrTextObj* pNotesTextObj = getEditedNotesObj();
    if (pNotesTextObj && pNotesTextObj->IsEmptyPresObj())
        maOutliner.SetToEmptyText();
}

/// Writes an edit still waiting for the modify timer back to the object the notes came from, or,
/// when the notes were left empty, restores that object's placeholder text and state.
void NotesPanelView::commitNotes()
{
    // Typing is saved when the modify timer fires, so only an edit still waiting for it is saved
    // here. Leaving the notes without an edit keeps them in the document as they are.
    const bool bUnsavedEdit = aModifyIdle.IsActive();
    aModifyIdle.Stop();
    SdrTextObj* pNotesTextObj = getEditedNotesObj();
    if (!pNotesTextObj)
        return;

    if (bUnsavedEdit)
        setNotesToDoc();

    if (maOutliner.GetEditEngine().HasText())
        return;

    // The document gets the placeholder back only when its notes are empty too, so notes
    // written meanwhile in another view stay.
    if (!pNotesTextObj->IsEmptyPresObj() && !pNotesTextObj->HasText())
    {
        if (SdPage* pPage = dynamic_cast<SdPage*>(pNotesTextObj->getSdrPageFromSdrObject()))
            pPage->RestoreDefaultText(pNotesTextObj, pNotesTextObj->GetCustomPromptText());
    }
}

/**
 * Handler for StatusEvents
 */
IMPL_LINK_NOARG(NotesPanelView, StatusEventHdl, EditStatus&, void) { onResize(); }

IMPL_LINK_NOARG(NotesPanelView, EditModifiedHdl, LinkParamNone*, void)
{
    // EditEngine calls ModifyHdl many times in succession for some edits.
    // (e.g. when deleting multiple lines)
    // Debounce the rapid ModifyHdl calls using a timer.
    aModifyIdle.Start();

    invalidateUndoState();
    maContentChangedHdl.Call(nullptr);
    return;
}

IMPL_LINK_NOARG(NotesPanelView, ModifyTimerHdl, Timer*, void)
{
    setNotesToDoc();
    aModifyIdle.Stop();

    // The other views that show these notes follow the typing.
    refreshViews(getEditedNotesObj(), this);
}

IMPL_LINK(NotesPanelView, EventMultiplexerListener, sdtools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::CurrentPageChanged:
        case EventMultiplexerEventId::MainViewRemoved:
        case EventMultiplexerEventId::MainViewAdded:
            FillOutliner();
            onResize();
            break;
        default:
            break;
    }
}

OutlinerView* NotesPanelView::GetViewByWindow(vcl::Window const* /*pWin*/) const
{
    return const_cast<NotesPanelView*>(this)->GetOutlinerView();
}

/**
 * Set attributes of the selected text
 */
bool NotesPanelView::SetAttributes(const SfxItemSet& rSet, bool /*bSlide*/, bool /*bReplaceAll*/,
                                   bool /*bMaster*/)
{
    bool bOk = false;

    OutlinerView* pOlView = GetOutlinerView();

    if (pOlView)
    {
        pOlView->SetAttribs(rSet);
        bOk = true;
    }

    mrNotesPanelViewShell.Invalidate(SID_PREVIEW_STATE);

    return bOk;
}

/**
 * Get attributes of the selected text
 */
void NotesPanelView::GetAttributes(SfxItemSet& rTargetSet, bool) const
{
    rTargetSet.Put(const_cast<OutlinerView&>(maOutlinerView).GetAttribs(), false);
}

SvtScriptType NotesPanelView::GetScriptType() const
{
    SvtScriptType nScriptType = ::sd::View::GetScriptType();

    std::optional<OutlinerParaObject> pTempOPObj = maOutliner.CreateParaObject();
    if (pTempOPObj)
    {
        nScriptType = pTempOPObj->GetTextObject().GetScriptType();
    }

    return nScriptType;
}

sal_Int8 NotesPanelView::AcceptDrop(const AcceptDropEvent&, DropTargetHelper&, SdrLayerID)
{
    return DND_ACTION_NONE;
}

sal_Int8 NotesPanelView::ExecuteDrop(const ExecuteDropEvent&, ::sd::Window*, sal_uInt16, SdrLayerID)
{
    return DND_ACTION_NONE;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
