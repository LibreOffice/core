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

#include <docufld.hxx> // SwPostItField
#include <txtfld.hxx>
#include <ndtxt.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <SwUndoField.hxx>

#include <boost/scoped_ptr.hpp>


namespace sw { namespace annotation {

SwAnnotationWin::SwAnnotationWin( SwEditWin& rEditWin,
                                  WinBits nBits,
                                  SwPostItMgr& aMgr,
                                  SwPostItBits aBits,
                                  SwSidebarItem& rSidebarItem,
                                  SwFmtFld* aField )
    : SwSidebarWin( rEditWin, nBits, aMgr, aBits, rSidebarItem )
    , mpFmtFld(aField)
    , mpFld( static_cast<SwPostItField*>(aField->GetFld()))
    , mpButtonPopup(0)
{
}

SwAnnotationWin::~SwAnnotationWin()
{
    delete mpButtonPopup;
}

void SwAnnotationWin::SetPostItText()
{
    //If the cursor was visible, then make it visible again after
    //changing text, e.g. fdo#33599
    Cursor *pCursor = GetOutlinerView()->GetEditView().GetCursor();
    bool bCursorVisible = pCursor ? pCursor->IsVisible() : false;

    //If the new text is the same as the old text, keep the same insertion
    //point .e.g. fdo#33599
    mpFld = static_cast<SwPostItField*>(mpFmtFld->GetFld());
    OUString sNewText = mpFld->GetPar2();
    bool bTextUnchanged = sNewText.equals(Engine()->GetEditEngine().GetText());
    ESelection aOrigSelection(GetOutlinerView()->GetEditView().GetSelection());

    // get text from SwPostItField and insert into our textview
    Engine()->SetModifyHdl( Link() );
    Engine()->EnableUndo( sal_False );
    if( mpFld->GetTextObject() )
        Engine()->SetText( *mpFld->GetTextObject() );
    else
    {
        Engine()->Clear();
        GetOutlinerView()->SetAttribs(DefaultItem());
        GetOutlinerView()->InsertText(sNewText,false);
    }

    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
    Engine()->EnableUndo( sal_True );
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
        boost::scoped_ptr<SwField> pOldField;
        if (rUndoRedo.DoesUndo())
        {
            pOldField.reset(mpFld->Copy());
        }
        mpFld->SetPar2(Engine()->GetEditEngine().GetText());
        mpFld->SetTextObject(Engine()->CreateParaObject());
        if (rUndoRedo.DoesUndo())
        {
            SwTxtFld *const pTxtFld = mpFmtFld->GetTxtFld();
            SwPosition aPosition( pTxtFld->GetTxtNode() );
            aPosition.nContent = *pTxtFld->GetStart();
            rUndoRedo.AppendUndo(
                new SwUndoFieldFromDoc(aPosition, *pOldField, *mpFld, 0, true));
        }
        // so we get a new layout of notes (anchor position is still the same and we would otherwise not get one)
        Mgr().SetLayout();
        // #i98686# if we have several views, all notes should update their text
        mpFmtFld->Broadcast(SwFmtFldHint( 0, SWFMTFLD_CHANGED));
        DocView().GetDocShell()->SetModified();
    }
    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
}

void SwAnnotationWin::Delete()
{
    SwSidebarWin::Delete();
    // we delete the field directly, the Mgr cleans up the PostIt by listening
    DocView().GetWrtShellPtr()->GotoField(*mpFmtFld);
    GrabFocusToDocument();
    DocView().GetWrtShellPtr()->DelRight();
}

void SwAnnotationWin::GotoPos()
{
    DocView().GetDocShell()->GetWrtShell()->GotoField(*mpFmtFld);
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
    SwTxtFld* pTxtFld = mpFmtFld->GetTxtFld();
    SwPosition aPosition( pTxtFld->GetTxtNode() );
    aPosition.nContent = *pTxtFld->GetStart();
    SwTxtAttr * const pTxtAttr = pTxtFld->GetTxtNode().GetTxtAttrForCharAt(
                    aPosition.nContent.GetIndex() - 1, RES_TXTATR_FIELD );
    const SwField* pFld = pTxtAttr ? pTxtAttr->GetFld().GetFld() : 0;
    return pFld && (pFld->Which()== RES_POSTITFLD);
}

// counts how many SwPostItField we have right after the current one
sal_uInt32 SwAnnotationWin::CountFollowing()
{
    sal_uInt32 aCount = 1;  // we start with 1, so we have to subtract one at the end again
    SwTxtFld* pTxtFld = mpFmtFld->GetTxtFld();
    SwPosition aPosition( pTxtFld->GetTxtNode() );
    aPosition.nContent = *pTxtFld->GetStart();

    SwTxtAttr * pTxtAttr = pTxtFld->GetTxtNode().GetTxtAttrForCharAt(
                                        aPosition.nContent.GetIndex() + 1,
                                        RES_TXTATR_FIELD );
    SwField* pFld = pTxtAttr
                    ? const_cast<SwField*>(pTxtAttr->GetFld().GetFld())
                    : 0;
    while ( pFld && ( pFld->Which()== RES_POSTITFLD ) )
    {
        aCount++;
        pTxtAttr = pTxtFld->GetTxtNode().GetTxtAttrForCharAt(
                                        aPosition.nContent.GetIndex() + aCount,
                                        RES_TXTATR_FIELD );
        pFld = pTxtAttr
               ? const_cast<SwField*>(pTxtAttr->GetFld().GetFld())
               : 0;
    }
    return aCount - 1;
}

MenuButton* SwAnnotationWin::CreateMenuButton()
{
    mpButtonPopup = new PopupMenu(SW_RES(MN_ANNOTATION_BUTTON));
    OUString aText = mpButtonPopup->GetItemText( FN_DELETE_NOTE_AUTHOR );
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1,GetAuthor());
    aText = aRewriter.Apply(aText);
    mpButtonPopup->SetItemText(FN_DELETE_NOTE_AUTHOR,aText);
    MenuButton* pMenuButton = new AnnotationMenuButton( *this );
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
    GetOutlinerView()->InsertText(aText,false);

    // insert old, selected text or "..."
    // TODO: iterate over all paragraphs, not only first one to find out if it is empty
    if (pText->GetTextObject().GetText(0).Len())
        GetOutlinerView()->GetEditView().InsertText(pText->GetTextObject());
    else
        GetOutlinerView()->InsertText(OUString("..."),false);
    GetOutlinerView()->InsertText(OUString("\"\n"),false);

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
    Engine()->SetModifyHdl( Link() );
    IDocumentUndoRedo & rUndoRedo(
        DocView().GetDocShell()->GetDoc()->GetIDocumentUndoRedo());
    boost::scoped_ptr<SwField> pOldField;
    if (rUndoRedo.DoesUndo())
    {
        pOldField.reset(mpFld->Copy());
    }
    mpFld->SetPar2(Engine()->GetEditEngine().GetText());
    mpFld->SetTextObject(Engine()->CreateParaObject());
    if (rUndoRedo.DoesUndo())
    {
        SwTxtFld *const pTxtFld = mpFmtFld->GetTxtFld();
        SwPosition aPosition( pTxtFld->GetTxtNode() );
        aPosition.nContent = *pTxtFld->GetStart();
        rUndoRedo.AppendUndo(
            new SwUndoFieldFromDoc(aPosition, *pOldField, *mpFld, 0, true));
    }
    Engine()->SetModifyHdl( LINK( this, SwAnnotationWin, ModifyHdl ) );
    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
}

SvxLanguageItem SwAnnotationWin::GetLanguage(void)
{
    // set initial language for outliner
    sal_uInt16 nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( mpFld->GetLanguage() );
    sal_uInt16 nLangWhichId = 0;
    switch (nScriptType)
    {
        case SCRIPTTYPE_LATIN :    nLangWhichId = EE_CHAR_LANGUAGE ; break;
        case SCRIPTTYPE_ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
        case SCRIPTTYPE_COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
        default: OSL_FAIL("GetLanguage: wrong script type");
    }
    return SvxLanguageItem(mpFld->GetLanguage(),nLangWhichId);
}

bool SwAnnotationWin::IsProtected()
{
    return SwSidebarWin::IsProtected() ||
           GetLayoutStatus() == SwPostItHelper::DELETED ||
           ( mpFmtFld ? mpFmtFld->IsProtect() : false );
}

OUString SwAnnotationWin::GetAuthor()
{
    return mpFld->GetPar1();
}

Date SwAnnotationWin::GetDate()
{
    return mpFld->GetDate();
}

Time SwAnnotationWin::GetTime()
{
    return mpFld->GetTime();
}

} } // end of namespace sw::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
