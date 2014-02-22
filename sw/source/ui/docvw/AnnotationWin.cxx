/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    , mpFld( static_cast<SwPostItField*>(aField->GetField()))
    , mpButtonPopup(0)
{
}

SwAnnotationWin::~SwAnnotationWin()
{
    delete mpButtonPopup;
}

void SwAnnotationWin::SetPostItText()
{
    
    
    Cursor *pCursor = GetOutlinerView()->GetEditView().GetCursor();
    bool bCursorVisible = pCursor ? pCursor->IsVisible() : false;

    
    
    mpFld = static_cast<SwPostItField*>(mpFmtFld->GetField());
    OUString sNewText = mpFld->GetPar2();
    bool bTextUnchanged = sNewText.equals(Engine()->GetEditEngine().GetText());
    ESelection aOrigSelection(GetOutlinerView()->GetEditView().GetSelection());

    
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
        
        Mgr().SetLayout();
        
        mpFmtFld->Broadcast(SwFmtFldHint( 0, SWFMTFLD_CHANGED));
        DocView().GetDocShell()->SetModified();
    }
    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
}

void SwAnnotationWin::Delete()
{
    SwSidebarWin::Delete();
    
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
    
    
    return Mgr().IsAnswer()
           ? 1
           : 1 + CountFollowing();
}


bool SwAnnotationWin::CalcFollow()
{
    SwTxtFld* pTxtFld = mpFmtFld->GetTxtFld();
    SwPosition aPosition( pTxtFld->GetTxtNode() );
    aPosition.nContent = *pTxtFld->GetStart();
    SwTxtAttr * const pTxtAttr =
        pTxtFld->GetTxtNode().GetTxtAttrForCharAt(
            aPosition.nContent.GetIndex() - 1,
            RES_TXTATR_ANNOTATION );
    const SwField* pFld = pTxtAttr ? pTxtAttr->GetFmtFld().GetField() : 0;
    return pFld && (pFld->Which()== RES_POSTITFLD);
}


sal_uInt32 SwAnnotationWin::CountFollowing()
{
    sal_uInt32 aCount = 1;  
    SwTxtFld* pTxtFld = mpFmtFld->GetTxtFld();
    SwPosition aPosition( pTxtFld->GetTxtNode() );
    aPosition.nContent = *pTxtFld->GetStart();

    SwTxtAttr * pTxtAttr = pTxtFld->GetTxtNode().GetTxtAttrForCharAt(
                                        aPosition.nContent.GetIndex() + 1,
                                        RES_TXTATR_ANNOTATION );
    SwField* pFld = pTxtAttr
                    ? const_cast<SwField*>(pTxtAttr->GetFmtFld().GetField())
                    : 0;
    while ( pFld && ( pFld->Which()== RES_POSTITFLD ) )
    {
        aCount++;
        pTxtAttr = pTxtFld->GetTxtNode().GetTxtAttrForCharAt(
                                        aPosition.nContent.GetIndex() + aCount,
                                        RES_TXTATR_ANNOTATION );
        pFld = pTxtAttr
               ? const_cast<SwField*>(pTxtAttr->GetFmtFld().GetField())
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

    
    
    if (!pText->GetTextObject().GetText(0).isEmpty())
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

    
    GetOutlinerView()->GetEditView().RemoveAttribsKeepLanguages(true);
    GetOutlinerView()->SetAttribs(DefaultItem());
    
    
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

} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
