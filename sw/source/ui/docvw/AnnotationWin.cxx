/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/************************************************************************* *
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#include "precompiled_sw.hxx"

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
    // get text from SwPostItField and insert into our textview
    Engine()->SetModifyHdl( Link() );
    Engine()->EnableUndo( sal_False );
    mpFld = static_cast<SwPostItField*>(mpFmtFld->GetFld());
    if( mpFld->GetTextObject() )
        Engine()->SetText( *mpFld->GetTextObject() );
    else
    {
        Engine()->Clear();
        GetOutlinerView()->SetAttribs(DefaultItem());
        GetOutlinerView()->InsertText(mpFld->GetPar2(),false);
    }

    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
    Engine()->EnableUndo( sal_True );
    Engine()->SetModifyHdl( LINK( this, SwAnnotationWin, ModifyHdl ) );
    Invalidate();
}

void SwAnnotationWin::UpdateData()
{
    if ( Engine()->IsModified() )
    {
        SwTxtFld* pTxtFld = mpFmtFld->GetTxtFld();
        SwPosition aPosition( pTxtFld->GetTxtNode() );
        aPosition.nContent = *pTxtFld->GetStart();
        SwField* pOldField = mpFld->Copy();
        mpFld->SetPar2(Engine()->GetEditEngine().GetText());
        mpFld->SetTextObject(Engine()->CreateParaObject());
        DocView().GetDocShell()->GetDoc()->GetIDocumentUndoRedo().AppendUndo(
            new SwUndoFieldFromDoc(aPosition, *pOldField, *mpFld, 0, true));
        delete pOldField;
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
    XubString aText = mpButtonPopup->GetItemText( FN_DELETE_NOTE_AUTHOR );
    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1,GetAuthor());
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
    String aText = String(SW_RES(STR_REPLY));
        SwRewriter aRewriter;
        aRewriter.AddRule(UNDO_ARG1, pWin->GetAuthor());
        aText = aRewriter.Apply(aText);
        aText.Append(String(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (")) +
        String(rLocalData.getDate( pWin->GetDate())) + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(", ")) +
        String(rLocalData.getTime( pWin->GetTime(),false)) + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("): \""))));
    GetOutlinerView()->InsertText(aText,false);

    // insert old, selected text or "..."
    // TOOD: iterate over all paragraphs, not only first one to find out if it is empty
    if (pText->GetTextObject().GetText(0).Len())
        GetOutlinerView()->GetEditView().InsertText(pText->GetTextObject());
    else
        GetOutlinerView()->InsertText(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("...")),false);
    GetOutlinerView()->InsertText(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n")),false);

    GetOutlinerView()->SetSelection(ESelection(0x0,0x0,0xFFFF,0xFFFF));
    SfxItemSet aAnswerSet( DocView().GetDocShell()->GetPool() );
    aAnswerSet.Put(SvxFontHeightItem(200,80,EE_CHAR_FONTHEIGHT));
    aAnswerSet.Put(SvxPostureItem(ITALIC_NORMAL,EE_CHAR_ITALIC));
    GetOutlinerView()->SetAttribs(aAnswerSet);
    GetOutlinerView()->SetSelection(ESelection(0xFFFF,0xFFFF,0xFFFF,0xFFFF));

    //remove all attributes and reset our standard ones
    GetOutlinerView()->GetEditView().RemoveAttribsKeepLanguages(true);
    GetOutlinerView()->SetAttribs(DefaultItem());
    // lets insert an undo step so the initial text can be easily deleted
    // but do not use UpdateData() directly, would set modified state again and reentrance into Mgr
    Engine()->SetModifyHdl( Link() );
    SwTxtFld* pTxtFld = mpFmtFld->GetTxtFld();
    SwPosition aPosition( pTxtFld->GetTxtNode() );
    aPosition.nContent = *pTxtFld->GetStart();
    SwField* pOldField = mpFld->Copy();
    mpFld->SetPar2(Engine()->GetEditEngine().GetText());
    mpFld->SetTextObject(Engine()->CreateParaObject());
    DocView().GetDocShell()->GetDoc()->GetIDocumentUndoRedo().AppendUndo(
        new SwUndoFieldFromDoc(aPosition, *pOldField, *mpFld, 0, true));
    delete pOldField;
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

String SwAnnotationWin::GetAuthor()
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
