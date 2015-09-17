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

#include <AnnotationWin.hxx>

#include <AnnotationMenuButton.hxx>
#include <PostItMgr.hxx>

#include <annotation.hrc>
#include <popup.hrc>
#include <cmdid.h>

#include <vcl/menu.hxx>

#include <svl/undo.hxx>
#include <unotools/syslocale.hxx>
#include <svl/languageoptions.hxx>

#include <editeng/postitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/langitem.hxx>

#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>

#include <docufld.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <SwUndoField.hxx>

#include <memory>

namespace sw { namespace annotation {

SwAnnotationWin::SwAnnotationWin( SwEditWin& rEditWin,
                                  WinBits nBits,
                                  SwPostItMgr& aMgr,
                                  SwPostItBits aBits,
                                  SwSidebarItem& rSidebarItem,
                                  SwFormatField* aField )
    : SwSidebarWin( rEditWin, nBits, aMgr, aBits, rSidebarItem )
    , mpFormatField(aField)
    , mpField( static_cast<SwPostItField*>(aField->GetField()))
    , mpButtonPopup(0)
{
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
    delete mpButtonPopup;
    sw::sidebarwindows::SwSidebarWin::dispose();
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
    bool bTextUnchanged = sNewText.equals(Engine()->GetEditEngine().GetText());
    ESelection aOrigSelection(GetOutlinerView()->GetEditView().GetSelection());

    // get text from SwPostItField and insert into our textview
    Engine()->SetModifyHdl( Link<LinkParamNone*,void>() );
    Engine()->EnableUndo( false );
    if( mpField->GetTextObject() )
        Engine()->SetText( *mpField->GetTextObject() );
    else
    {
        Engine()->Clear();
        GetOutlinerView()->SetAttribs(DefaultItem());
        GetOutlinerView()->InsertText(sNewText);
    }

    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
    Engine()->EnableUndo( true );
    Engine()->SetModifyHdl( LINK( this, SwAnnotationWin, ModifyHdl ) );
    if (bTextUnchanged)
        GetOutlinerView()->GetEditView().SetSelection(aOrigSelection);
    if (bCursorVisible)
        GetOutlinerView()->ShowCursor();
    Invalidate();
}

void SwAnnotationWin::UpdateData()
{
    if ( Engine()->IsModified() )
    {
        IDocumentUndoRedo & rUndoRedo(
            DocView().GetDocShell()->GetDoc()->GetIDocumentUndoRedo());
        std::unique_ptr<SwField> pOldField;
        if (rUndoRedo.DoesUndo())
        {
            pOldField.reset(mpField->Copy());
        }
        mpField->SetPar2(Engine()->GetEditEngine().GetText());
        mpField->SetTextObject(Engine()->CreateParaObject());
        if (rUndoRedo.DoesUndo())
        {
            SwTextField *const pTextField = mpFormatField->GetTextField();
            SwPosition aPosition( pTextField->GetTextNode() );
            aPosition.nContent = pTextField->GetStart();
            rUndoRedo.AppendUndo(
                new SwUndoFieldFromDoc(aPosition, *pOldField, *mpField, 0, true));
        }
        // so we get a new layout of notes (anchor position is still the same and we would otherwise not get one)
        Mgr().SetLayout();
        // #i98686# if we have several views, all notes should update their text
        mpFormatField->Broadcast(SwFormatFieldHint( 0, SwFormatFieldHintWhich::CHANGED));
        DocView().GetDocShell()->SetModified();
    }
    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
}

void SwAnnotationWin::Delete()
{
    if (DocView().GetWrtShellPtr()->GotoField(*mpFormatField))
    {
        SwSidebarWin::Delete();
        // we delete the field directly, the Mgr cleans up the PostIt by listening
        GrabFocusToDocument();
        DocView().GetWrtShellPtr()->DelRight();
    }
}

void SwAnnotationWin::GotoPos()
{
    DocView().GetDocShell()->GetWrtShell()->GotoField(*mpFormatField);
}

sal_uInt32 SwAnnotationWin::MoveCaret()
{
    // if this is an answer, do not skip over all following ones, but insert directly behind the current one
    // but when just leaving a note, skip all following ones as well to continue typing
    return Mgr().IsAnswer()
           ? 1
           : 1 + CountFollowing();
}

//returns true, if there is another note right before this note
bool SwAnnotationWin::CalcFollow()
{
    SwTextField* pTextField = mpFormatField->GetTextField();
    SwPosition aPosition( pTextField->GetTextNode() );
    aPosition.nContent = pTextField->GetStart();
    SwTextAttr * const pTextAttr =
        pTextField->GetTextNode().GetTextAttrForCharAt(
            aPosition.nContent.GetIndex() - 1,
            RES_TXTATR_ANNOTATION );
    const SwField* pField = pTextAttr ? pTextAttr->GetFormatField().GetField() : 0;
    return pField && (pField->Which()== RES_POSTITFLD);
}

// counts how many SwPostItField we have right after the current one
sal_uInt32 SwAnnotationWin::CountFollowing()
{
    sal_uInt32 aCount = 1;  // we start with 1, so we have to subtract one at the end again
    SwTextField* pTextField = mpFormatField->GetTextField();
    SwPosition aPosition( pTextField->GetTextNode() );
    aPosition.nContent = pTextField->GetStart();

    SwTextAttr * pTextAttr = pTextField->GetTextNode().GetTextAttrForCharAt(
                                        aPosition.nContent.GetIndex() + 1,
                                        RES_TXTATR_ANNOTATION );
    SwField* pField = pTextAttr
                    ? const_cast<SwField*>(pTextAttr->GetFormatField().GetField())
                    : 0;
    while ( pField && ( pField->Which()== RES_POSTITFLD ) )
    {
        aCount++;
        pTextAttr = pTextField->GetTextNode().GetTextAttrForCharAt(
                                        aPosition.nContent.GetIndex() + aCount,
                                        RES_TXTATR_ANNOTATION );
        pField = pTextAttr
               ? const_cast<SwField*>(pTextAttr->GetFormatField().GetField())
               : 0;
    }
    return aCount - 1;
}

VclPtr<MenuButton> SwAnnotationWin::CreateMenuButton()
{
    mpButtonPopup = new PopupMenu(SW_RES(MN_ANNOTATION_BUTTON));
    OUString aText = mpButtonPopup->GetItemText( FN_DELETE_NOTE_AUTHOR );
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1,GetAuthor());
    aText = aRewriter.Apply(aText);
    mpButtonPopup->SetItemText(FN_DELETE_NOTE_AUTHOR,aText);
    VclPtrInstance<AnnotationMenuButton> pMenuButton( *this );
    pMenuButton->SetPopupMenu( mpButtonPopup );
    pMenuButton->Show();
    return pMenuButton;
}

void SwAnnotationWin::InitAnswer(OutlinerParaObject* pText)
{
    //collect our old meta data
    SwSidebarWin* pWin = Mgr().GetNextPostIt(KEY_PAGEUP, this);
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocalData = aSysLocale.GetLocaleData();
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, pWin->GetAuthor());
    const OUString aText = aRewriter.Apply(SW_RESSTR(STR_REPLY))
            + " (" + rLocalData.getDate( pWin->GetDate())
            + ", " + rLocalData.getTime( pWin->GetTime(), false)
            + "): \"";
    GetOutlinerView()->InsertText(aText);

    // insert old, selected text or "..."
    // TODO: iterate over all paragraphs, not only first one to find out if it is empty
    if (!pText->GetTextObject().GetText(0).isEmpty())
        GetOutlinerView()->GetEditView().InsertText(pText->GetTextObject());
    else
        GetOutlinerView()->InsertText(OUString("..."));
    GetOutlinerView()->InsertText(OUString("\"\n"));

    GetOutlinerView()->SetSelection(ESelection(0,0,EE_PARA_ALL,EE_TEXTPOS_ALL));
    SfxItemSet aAnswerSet( DocView().GetDocShell()->GetPool() );
    aAnswerSet.Put(SvxFontHeightItem(200,80,EE_CHAR_FONTHEIGHT));
    aAnswerSet.Put(SvxPostureItem(ITALIC_NORMAL,EE_CHAR_ITALIC));
    GetOutlinerView()->SetAttribs(aAnswerSet);
    GetOutlinerView()->SetSelection(ESelection(EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT,EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT));

    //remove all attributes and reset our standard ones
    GetOutlinerView()->GetEditView().RemoveAttribsKeepLanguages(true);
    GetOutlinerView()->SetAttribs(DefaultItem());
    // lets insert an undo step so the initial text can be easily deleted
    // but do not use UpdateData() directly, would set modified state again and reentrance into Mgr
    Engine()->SetModifyHdl( Link<LinkParamNone*,void>() );
    IDocumentUndoRedo & rUndoRedo(
        DocView().GetDocShell()->GetDoc()->GetIDocumentUndoRedo());
    std::unique_ptr<SwField> pOldField;
    if (rUndoRedo.DoesUndo())
    {
        pOldField.reset(mpField->Copy());
    }
    mpField->SetPar2(Engine()->GetEditEngine().GetText());
    mpField->SetTextObject(Engine()->CreateParaObject());
    if (rUndoRedo.DoesUndo())
    {
        SwTextField *const pTextField = mpFormatField->GetTextField();
        SwPosition aPosition( pTextField->GetTextNode() );
        aPosition.nContent = pTextField->GetStart();
        rUndoRedo.AppendUndo(
            new SwUndoFieldFromDoc(aPosition, *pOldField, *mpField, 0, true));
    }
    Engine()->SetModifyHdl( LINK( this, SwAnnotationWin, ModifyHdl ) );
    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
}

SvxLanguageItem SwAnnotationWin::GetLanguage()
{
    // set initial language for outliner
    SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( mpField->GetLanguage() );
    sal_uInt16 nLangWhichId = 0;
    switch (nScriptType)
    {
        case SvtScriptType::LATIN :    nLangWhichId = EE_CHAR_LANGUAGE ; break;
        case SvtScriptType::ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
        case SvtScriptType::COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
        default: OSL_FAIL("GetLanguage: wrong script type");
    }
    return SvxLanguageItem(mpField->GetLanguage(),nLangWhichId);
}

bool SwAnnotationWin::IsProtected()
{
    return SwSidebarWin::IsProtected() ||
           GetLayoutStatus() == SwPostItHelper::DELETED ||
           ( mpFormatField && mpFormatField->IsProtect() );
}

OUString SwAnnotationWin::GetAuthor()
{
    return mpField->GetPar1();
}

Date SwAnnotationWin::GetDate()
{
    return mpField->GetDate();
}

tools::Time SwAnnotationWin::GetTime()
{
    return mpField->GetTime();
}

} } // end of namespace sw::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
