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

#include <config_wasm_strip.h>

#include <AnnotationWin.hxx>

#include <PostItMgr.hxx>

#include <strings.hrc>

#include <uiobject.hxx>

#include <vcl/svapp.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

#include <svl/undo.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <svl/languageoptions.hxx>
#include <osl/diagnose.h>

#include <editeng/eeitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/editund2.hxx>

#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/random.hxx>
#include <docufld.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <SwUndoField.hxx>
#include <edtwin.hxx>
#include "ShadowOverlayObject.hxx"
#include "AnchorOverlayObject.hxx"
#include "OverlayRanges.hxx"
#include "SidebarTxtControl.hxx"
#include "SidebarWinAcc.hxx"

#include <memory>

namespace{

void collectUIInformation( const OUString& aevent , const OUString& aID )
{
    EventDescription aDescription;
    aDescription.aID =  aID;
    aDescription.aParameters = {{"" ,  ""}};
    aDescription.aAction = aevent;
    aDescription.aParent = "MainWindow";
    aDescription.aKeyWord = "SwEditWinUIObject";
    UITestLogger::getInstance().logEvent(aDescription);
}

}

namespace sw::annotation {

// see AnnotationContents in sd for something similar
SwAnnotationWin::SwAnnotationWin( SwEditWin& rEditWin,
                                  SwPostItMgr& aMgr,
                                  SwSidebarItem& rSidebarItem,
                                  SwFormatField* aField )
    : InterimItemWindow(&rEditWin, "modules/swriter/ui/annotation.ui", "Annotation")
    , mrMgr(aMgr)
    , mrView(rEditWin.GetView())
    , mnDeleteEventId(nullptr)
    , meSidebarPosition(sw::sidebarwindows::SidebarPosition::NONE)
    , mPageBorder(0)
    , mbAnchorRectChanged(false)
    , mbResolvedStateUpdated(false)
    , mbMouseOver(false)
    , mLayoutStatus(SwPostItHelper::INVISIBLE)
    , mbReadonly(false)
    , mbIsFollow(false)
    , mrSidebarItem(rSidebarItem)
    , mpAnchorFrame(rSidebarItem.maLayoutInfo.mpAnchorFrame)
    , mpFormatField(aField)
    , mpField( static_cast<SwPostItField*>(aField->GetField()))
{
    set_id("Comment"+OUString::number(mpField->GetPostItId()));

    m_xContainer->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

    mpShadow = sidebarwindows::ShadowOverlayObject::CreateShadowOverlayObject( mrView );
    if ( mpShadow )
    {
        mpShadow->setVisible(false);
    }

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if (rSidebarItem.maLayoutInfo.mpAnchorFrame)
    {
        mrMgr.ConnectSidebarWinToFrame( *(rSidebarItem.maLayoutInfo.mpAnchorFrame),
                                      mrSidebarItem.GetFormatField(),
                                      *this );
    }
#endif

    if (SupportsDoubleBuffering())
        // When double-buffering, allow parents to paint on our area. That's
        // necessary when parents paint the complete buffer.
        SetParentClipMode(ParentClipMode::NoClip);
}

SwAnnotationWin::~SwAnnotationWin()
{
    disposeOnce();
}

void SwAnnotationWin::dispose()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    mrMgr.DisconnectSidebarWinFromFrame( *(mrSidebarItem.maLayoutInfo.mpAnchorFrame),
                                       *this );
#endif
    Disable();

    mxSidebarTextControlWin.reset();
    mxSidebarTextControl.reset();

    mxMetadataAuthor.reset();
    mxMetadataResolved.reset();
    mxMetadataDate.reset();
    mxVScrollbar.reset();

    mpAnchor.reset();
    mpShadow.reset();

    mpTextRangeOverlay.reset();

    mxMenuButton.reset();

    if (mnDeleteEventId)
        Application::RemoveUserEvent(mnDeleteEventId);

    mpOutliner.reset();
    mpOutlinerView.reset();

    InterimItemWindow::dispose();
}

void SwAnnotationWin::SetPostItText()
{
    //If the cursor was visible, then make it visible again after
    //changing text, e.g. fdo#33599
    vcl::Cursor *pCursor = GetOutlinerView()->GetEditView().GetCursor();
    bool bCursorVisible = pCursor && pCursor->IsVisible();

    //If the new text is the same as the old text, keep the same insertion
    //point .e.g. fdo#33599
    mpField = static_cast<SwPostItField*>(mpFormatField->GetField());
    OUString sNewText = mpField->GetPar2();
    bool bTextUnchanged = sNewText == mpOutliner->GetEditEngine().GetText();
    ESelection aOrigSelection(GetOutlinerView()->GetEditView().GetSelection());

    // get text from SwPostItField and insert into our textview
    mpOutliner->SetModifyHdl( Link<LinkParamNone*,void>() );
    mpOutliner->EnableUndo( false );
    if( mpField->GetTextObject() )
        mpOutliner->SetText( *mpField->GetTextObject() );
    else
    {
        mpOutliner->Clear();
        GetOutlinerView()->SetStyleSheet(SwResId(STR_POOLCOLL_COMMENT));
        GetOutlinerView()->InsertText(sNewText);
    }

    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();
    mpOutliner->EnableUndo( true );
    mpOutliner->SetModifyHdl( LINK( this, SwAnnotationWin, ModifyHdl ) );
    if (bTextUnchanged)
        GetOutlinerView()->GetEditView().SetSelection(aOrigSelection);
    if (bCursorVisible)
        GetOutlinerView()->ShowCursor();
    Invalidate();
}

void SwAnnotationWin::GeneratePostItName()
{
    if (mpField && mpField->GetName().isEmpty())
    {
        mpField->SetName(sw::mark::MarkBase::GenerateNewName(u"__Annotation__"));
    }
}

void SwAnnotationWin::SetResolved(bool resolved)
{
    bool oldState = IsResolved();
    static_cast<SwPostItField*>(mpFormatField->GetField())->SetResolved(resolved);
    if (SwWrtShell* pWrtShell = mrView.GetWrtShellPtr())
    {
        const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
        mrSidebarItem.mbShow = !IsResolved() || (pVOpt->IsResolvedPostIts());
    }

    mpTextRangeOverlay.reset();

    if(IsResolved())
        mxMetadataResolved->show();
    else
        mxMetadataResolved->hide();

    if(IsResolved() != oldState)
        mbResolvedStateUpdated = true;
    UpdateData();
    Invalidate();
    collectUIInformation("SETRESOLVED",get_id());
}

void SwAnnotationWin::ToggleResolved()
{
    SetResolved(!IsResolved());
}

void SwAnnotationWin::ToggleResolvedForThread()
{
    auto pTop = GetTopReplyNote();
    pTop->ToggleResolved();
    mrMgr.UpdateResolvedStatus(pTop);
    mrMgr.LayoutPostIts();
}

sal_uInt32 SwAnnotationWin::GetParaId()
{
    auto pField = static_cast<SwPostItField*>(mpFormatField->GetField());
    auto nParaId = pField->GetParaId();
    if (nParaId == 0)
    {
        // The parent annotation does not have a paraId. This happens when the annotation was just
        // created, and not imported. paraIds are regenerated upon export, thus this new paraId
        // is only generated so that children annotations can refer to it
        nParaId = CreateUniqueParaId();
        pField->SetParaId(nParaId);
    }
    return nParaId;
}

sal_uInt32 SwAnnotationWin::CreateUniqueParaId()
{
    return comphelper::rng::uniform_uint_distribution(0, std::numeric_limits<sal_uInt32>::max());
}

void SwAnnotationWin::DeleteThread()
{
    // Go to the top and delete each comment one by one
    SwAnnotationWin* topNote = GetTopReplyNote();
    for (SwAnnotationWin* current = topNote;;)
    {
        SwAnnotationWin* next = mrMgr.GetNextPostIt(KEY_PAGEDOWN, current);
        current->mnDeleteEventId = Application::PostUserEvent( LINK( current, SwAnnotationWin, DeleteHdl), nullptr, true );
        if (!next || next->GetTopReplyNote() != topNote)
            return;
        current = next;
    }
}

bool SwAnnotationWin::IsResolved() const
{
    return static_cast<SwPostItField*>(mpFormatField->GetField())->GetResolved();
}

bool SwAnnotationWin::IsThreadResolved()
{
    /// First Get the top note
    // then iterate downwards checking resolved status
    SwAnnotationWin* topNote = GetTopReplyNote();
    for (SwAnnotationWin* current = topNote;;)
    {
        if (!current->IsResolved())
            return false;
        current = mrMgr.GetNextPostIt(KEY_PAGEDOWN, current);
        if (!current || current->GetTopReplyNote() != topNote)
            return true;
    }
}

void SwAnnotationWin::UpdateData()
{
    if ( mpOutliner->IsModified() || mbResolvedStateUpdated )
    {
        IDocumentUndoRedo & rUndoRedo(
            mrView.GetDocShell()->GetDoc()->GetIDocumentUndoRedo());
        std::unique_ptr<SwField> pOldField;
        if (rUndoRedo.DoesUndo())
        {
            pOldField = mpField->Copy();
        }
        mpField->SetPar2(mpOutliner->GetEditEngine().GetText());
        mpField->SetTextObject(mpOutliner->CreateParaObject());
        if (rUndoRedo.DoesUndo())
        {
            SwTextField *const pTextField = mpFormatField->GetTextField();
            SwPosition aPosition( pTextField->GetTextNode(), pTextField->GetStart() );
            rUndoRedo.AppendUndo(
                std::make_unique<SwUndoFieldFromDoc>(aPosition, *pOldField, *mpField, true));
        }
        // so we get a new layout of notes (anchor position is still the same and we would otherwise not get one)
        mrMgr.SetLayout();
        // #i98686# if we have several views, all notes should update their text
        if(mbResolvedStateUpdated)
            mpFormatField->Broadcast(SwFormatFieldHint( nullptr, SwFormatFieldHintWhich::RESOLVED));
        else
            mpFormatField->Broadcast(SwFormatFieldHint( nullptr, SwFormatFieldHintWhich::CHANGED));
        mrView.GetDocShell()->SetModified();
    }
    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();
    mbResolvedStateUpdated = false;
}

void SwAnnotationWin::Delete()
{
    collectUIInformation("DELETE",get_id());
    SwWrtShell* pWrtShell = mrView.GetWrtShellPtr();
    if (!(pWrtShell && pWrtShell->GotoField(*mpFormatField)))
        return;

    if ( mrMgr.GetActiveSidebarWin() == this)
    {
        mrMgr.SetActiveSidebarWin(nullptr);
        // if the note is empty, the previous line will send a delete event, but we are already there
        if (mnDeleteEventId)
        {
            Application::RemoveUserEvent(mnDeleteEventId);
            mnDeleteEventId = nullptr;
        }
    }
    // we delete the field directly, the Mgr cleans up the PostIt by listening
    GrabFocusToDocument();
    pWrtShell->ClearMark();
    pWrtShell->DelRight();
}

void SwAnnotationWin::GotoPos()
{
    mrView.GetDocShell()->GetWrtShell()->GotoField(*mpFormatField);
}

sal_uInt32 SwAnnotationWin::MoveCaret()
{
    // if this is an answer, do not skip over all following ones, but insert directly behind the current one
    // but when just leaving a note, skip all following ones as well to continue typing
    return mrMgr.IsAnswer()
           ? 1
           : 1 + CountFollowing();
}

// counts how many SwPostItField we have right after the current one
sal_uInt32 SwAnnotationWin::CountFollowing()
{
    SwTextField* pTextField = mpFormatField->GetTextField();
    SwPosition aPosition( pTextField->GetTextNode(), pTextField->GetStart() );

    for (sal_Int32 n = 1;; ++n)
    {
        SwTextAttr * pTextAttr = pTextField->GetTextNode().GetTextAttrForCharAt(
                                            aPosition.GetContentIndex() + n,
                                            RES_TXTATR_ANNOTATION );
        if (!pTextAttr)
            return n - 1;
        const SwField* pField = pTextAttr->GetFormatField().GetField();
        if (!pField || pField->Which() != SwFieldIds::Postit)
            return n - 1;
    }
}

void SwAnnotationWin::InitAnswer(OutlinerParaObject const & rText)
{
    // If tiled annotations is off in lok case, skip adding additional reply text.
    if (comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations())
        return;

    //collect our old meta data
    SwAnnotationWin* pWin = mrMgr.GetNextPostIt(KEY_PAGEUP, this);
    if (!pWin)
        return;

    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocalData = aSysLocale.GetLocaleData();
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, pWin->GetAuthor());
    const OUString aText = aRewriter.Apply(SwResId(STR_REPLY))
            + " (" + rLocalData.getDate( pWin->GetDate())
            + ", " + rLocalData.getTime( pWin->GetTime(), false)
            + "): \"";
    GetOutlinerView()->InsertText(aText);

    // insert old, selected text or "..."
    // TODO: iterate over all paragraphs, not only first one to find out if it is empty
    if (!rText.GetTextObject().GetText(0).isEmpty())
        GetOutlinerView()->GetEditView().InsertText(rText.GetTextObject());
    else
        GetOutlinerView()->InsertText("...");
    GetOutlinerView()->InsertText("\"\n");

    GetOutlinerView()->SetSelection(ESelection(0,0,EE_PARA_ALL,EE_TEXTPOS_ALL));
    SfxItemSet aAnswerSet( mrView.GetDocShell()->GetPool() );
    aAnswerSet.Put(SvxFontHeightItem(200,80,EE_CHAR_FONTHEIGHT));
    aAnswerSet.Put(SvxPostureItem(ITALIC_NORMAL,EE_CHAR_ITALIC));
    GetOutlinerView()->SetAttribs(aAnswerSet);
    GetOutlinerView()->SetSelection(ESelection(EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT,EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT));

    //remove all attributes and reset our standard ones
    GetOutlinerView()->GetEditView().RemoveAttribsKeepLanguages(true);
    // lets insert an undo step so the initial text can be easily deleted
    // but do not use UpdateData() directly, would set modified state again and reentrance into Mgr
    mpOutliner->SetModifyHdl( Link<LinkParamNone*,void>() );
    IDocumentUndoRedo & rUndoRedo(
        mrView.GetDocShell()->GetDoc()->GetIDocumentUndoRedo());
    std::unique_ptr<SwField> pOldField;
    if (rUndoRedo.DoesUndo())
    {
        pOldField = mpField->Copy();
    }
    mpField->SetPar2(mpOutliner->GetEditEngine().GetText());
    mpField->SetTextObject(mpOutliner->CreateParaObject());
    if (rUndoRedo.DoesUndo())
    {
        SwTextField *const pTextField = mpFormatField->GetTextField();
        SwPosition aPosition( pTextField->GetTextNode(), pTextField->GetStart() );
        rUndoRedo.AppendUndo(
            std::make_unique<SwUndoFieldFromDoc>(aPosition, *pOldField, *mpField, true));
    }
    mpOutliner->SetModifyHdl( LINK( this, SwAnnotationWin, ModifyHdl ) );
    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();
}

void SwAnnotationWin::UpdateText(const OUString& aText)
{
    mpOutliner->Clear();
    GetOutlinerView()->InsertText(aText);
    UpdateData();
}

bool SwAnnotationWin::IsReadOnlyOrProtected() const
{
    return mbReadonly ||
           GetLayoutStatus() == SwPostItHelper::DELETED ||
           ( mpFormatField && mpFormatField->IsProtect() );
}

OUString SwAnnotationWin::GetAuthor() const
{
    return mpField->GetPar1();
}

Date SwAnnotationWin::GetDate() const
{
    return mpField->GetDate();
}

tools::Time SwAnnotationWin::GetTime() const
{
    return mpField->GetTime();
}

FactoryFunction SwAnnotationWin::GetUITestFactory() const
{
    return CommentUIObject::create;
}

} // end of namespace sw::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
