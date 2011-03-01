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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <sal/macros.h>
#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/i18n/WordType.hpp>
#include <vcl/metric.hxx>

#include <i18npool/mslangid.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/langtab.hxx>
#include <svtools/filter.hxx>

#include <svl/srchitem.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>

#include <impedit.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/eerdll.hxx>
#include <eerdll2.hxx>
#include <editeng.hrc>
#include <helpid.hrc>
#include <i18npool/lang.h>
#include <vcl/menu.hxx>
#include <vcl/window.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/fontitem.hxx>
#include <unotools/lingucfg.hxx>
#include <osl/file.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/beans/PropertyValues.hdl>
#include <com/sun/star/lang/Locale.hpp>
#include <linguistic/lngprops.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/lingucfg.hxx>
#include <sal/macros.h>

#include <com/sun/star/lang/XServiceInfo.hpp>

using ::rtl::OUString;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::linguistic2;


DBG_NAME( EditView )


// From SW => Create common method
LanguageType lcl_CheckLanguage(
    const OUString &rText,
    Reference< XSpellChecker1 > xSpell,
    Reference< linguistic2::XLanguageGuessing > xLangGuess,
    sal_Bool bIsParaText )
{
    LanguageType nLang = LANGUAGE_NONE;
    if (bIsParaText)    // check longer texts with language-guessing...
    {
        if (!xLangGuess.is())
            return nLang;

        lang::Locale aLocale( xLangGuess->guessPrimaryLanguage( rText, 0, rText.getLength()) );

        // get language as from "Tools/Options - Language Settings - Languages: Locale setting"
        LanguageType nTmpLang = Application::GetSettings().GetLanguage();

        // if the result from language guessing does not provide a 'Country' part
        // try to get it by looking up the locale setting of the office.
        if (aLocale.Country.getLength() == 0)
        {
            lang::Locale aTmpLocale = SvxCreateLocale( nTmpLang );
            if (aTmpLocale.Language == aLocale.Language)
                nLang = nTmpLang;
        }
        if (nLang == LANGUAGE_NONE) // language not found by looking up the sytem language...
            nLang = MsLangId::convertLocaleToLanguageWithFallback( aLocale );
        if (nLang == LANGUAGE_SYSTEM)
            nLang = nTmpLang;
        if (nLang == LANGUAGE_DONTKNOW)
            nLang = LANGUAGE_NONE;
    }
    else    // check single word
    {
            if (!xSpell.is())
            return nLang;

        //
        // build list of languages to check
        //
        LanguageType aLangList[4];
        const AllSettings& rSettings  = Application::GetSettings();
        SvtLinguOptions aLinguOpt;
        SvtLinguConfig().GetOptions( aLinguOpt );
        // The default document language from "Tools/Options - Language Settings - Languages: Western"
        aLangList[0] = aLinguOpt.nDefaultLanguage;
        // The one from "Tools/Options - Language Settings - Languages: User interface"
        aLangList[1] = rSettings.GetUILanguage();
        // The one from "Tools/Options - Language Settings - Languages: Locale setting"
        aLangList[2] = rSettings.GetLanguage();
        // en-US
        aLangList[3] = LANGUAGE_ENGLISH_US;
#ifdef DEBUG
        lang::Locale a0( SvxCreateLocale( aLangList[0] ) );
        lang::Locale a1( SvxCreateLocale( aLangList[1] ) );
        lang::Locale a2( SvxCreateLocale( aLangList[2] ) );
        lang::Locale a3( SvxCreateLocale( aLangList[3] ) );
#endif

        INT32   nCount = SAL_N_ELEMENTS(aLangList);
        for (INT32 i = 0;  i < nCount;  i++)
        {
            INT16 nTmpLang = aLangList[i];
            if (nTmpLang != LANGUAGE_NONE  &&  nTmpLang != LANGUAGE_DONTKNOW)
            {
                if (xSpell->hasLanguage( nTmpLang ) &&
                    xSpell->isValid( rText, nTmpLang, Sequence< PropertyValue >() ))
                {
                    nLang = nTmpLang;
                    break;
                }
            }
        }
    }

    return nLang;
}


// ----------------------------------------------------------------------
// class EditView
// ----------------------------------------------------------------------
EditView::EditView( EditEngine* pEng, Window* pWindow )
{
    DBG_CTOR( EditView, 0 );
    pImpEditView = new ImpEditView( this, pEng, pWindow );
}

EditView::~EditView()
{
    DBG_DTOR( EditView, 0 );
    delete pImpEditView;
}

ImpEditEngine* EditView::GetImpEditEngine() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->pEditEngine->pImpEditEngine;
}

EditEngine* EditView::GetEditEngine() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->pEditEngine;
}

void EditView::Invalidate()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !pImpEditView->DoInvalidateMore() )
        pImpEditView->GetWindow()->Invalidate( pImpEditView->aOutArea );
    else
    {
        Rectangle aRect( pImpEditView->aOutArea );
        long nMore = pImpEditView->GetWindow()->PixelToLogic( Size( pImpEditView->GetInvalidateMore(), 0 ) ).Width();
        aRect.Left() -= nMore;
        aRect.Right() += nMore;
        aRect.Top() -= nMore;
        aRect.Bottom() += nMore;
        pImpEditView->GetWindow()->Invalidate( aRect );
    }
}

void EditView::SetReadOnly( sal_Bool bReadOnly )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->bReadOnly = bReadOnly;
}

sal_Bool EditView::IsReadOnly() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->bReadOnly;
}

void EditView::SetSelection( const ESelection& rESel )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    // If someone has just left an empty attribute, and then the outliner
    // manipulates the selection:
    if ( !pImpEditView->GetEditSelection().HasRange() )
    {
        ContentNode* pNode = pImpEditView->GetEditSelection().Max().GetNode();
        PIMPEE->CursorMoved( pNode );
    }
    EditSelection aNewSelection( PIMPEE->ConvertSelection( rESel.nStartPara, rESel.nStartPos, rESel.nEndPara, rESel.nEndPos ) );

    // If the selection is manipulated after a KeyInput:
    PIMPEE->CheckIdleFormatter();

    // Selection may not start/end at an invisible paragraph:
    ParaPortion* pPortion = PIMPEE->FindParaPortion( aNewSelection.Min().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = PIMPEE->GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : PIMPEE->GetEditDoc().GetObject( 0 );
        aNewSelection.Min() = EditPaM( pNode, pNode->Len() );
    }
    pPortion = PIMPEE->FindParaPortion( aNewSelection.Max().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = PIMPEE->GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : PIMPEE->GetEditDoc().GetObject( 0 );
        aNewSelection.Max() = EditPaM( pNode, pNode->Len() );
    }

    pImpEditView->DrawSelection();
    pImpEditView->SetEditSelection( aNewSelection );
    pImpEditView->DrawSelection();
    sal_Bool bGotoCursor = pImpEditView->DoAutoScroll();
    ShowCursor( bGotoCursor );
}

ESelection EditView::GetSelection() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    ESelection aSelection;

    aSelection.nStartPara = PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Min().GetNode() );
    aSelection.nEndPara = PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Max().GetNode() );

    aSelection.nStartPos = pImpEditView->GetEditSelection().Min().GetIndex();
    aSelection.nEndPos = pImpEditView->GetEditSelection().Max().GetIndex();

    return aSelection;
}

sal_Bool EditView::HasSelection() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->HasSelection();
}

void EditView::DeleteSelected()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->DeleteSelected();
}

USHORT EditView::GetSelectedScriptType() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->GetScriptType( pImpEditView->GetEditSelection() );
}

void EditView::Paint( const Rectangle& rRect )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Paint( pImpEditView, rRect );
}

void EditView::SetEditEngine( EditEngine* pEditEng )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->pEditEngine = pEditEng;
    EditSelection aStartSel;
    aStartSel = PIMPEE->GetEditDoc().GetStartPaM();
    pImpEditView->SetEditSelection( aStartSel );
}

void EditView::SetWindow( Window* pWin )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->pOutWin = pWin;
    PIMPEE->GetSelEngine().Reset();
}

Window* EditView::GetWindow() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->pOutWin;
}

void EditView::SetVisArea( const Rectangle& rRec )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetVisDocStartPos( rRec.TopLeft() );
}

const Rectangle& EditView::GetVisArea() const
{
    DBG_CHKTHIS( EditView, 0 );
    // Change return value to Rectangle in next incompatible build !!!
    static Rectangle aRect;
    aRect = pImpEditView->GetVisDocArea();
    return aRect;
}

void EditView::SetOutputArea( const Rectangle& rRec )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetOutputArea( rRec );

    // the rest here only if it is an API call:
    pImpEditView->CalcAnchorPoint();
    if ( PIMPEE->GetStatus().AutoPageSize() )
        pImpEditView->RecalcOutputArea();
    pImpEditView->ShowCursor( sal_False, sal_False );
}

const Rectangle& EditView::GetOutputArea() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetOutputArea();
}

void EditView::SetPointer( const Pointer& rPointer )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetPointer( rPointer );
}

const Pointer& EditView::GetPointer() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetPointer();
}

void EditView::SetCursor( const Cursor& rCursor )
{
    DBG_CHKTHIS( EditView, 0 );
    delete pImpEditView->pCursor;
    pImpEditView->pCursor = new Cursor( rCursor );
}

Cursor* EditView::GetCursor() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->pCursor;
}

void EditView::InsertText( const XubString& rStr, sal_Bool bSelect )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    ImpEditEngine* pImpEE = PIMPEE;
    pImpEditView->DrawSelection();

    EditPaM aPaM1;
    if ( bSelect )
    {
        EditSelection aTmpSel( pImpEditView->GetEditSelection() );
        aTmpSel.Adjust( pImpEE->GetEditDoc() );
        aPaM1 = aTmpSel.Min();
    }

    pImpEE->UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM2( pImpEE->InsertText( pImpEditView->GetEditSelection(), rStr ) );
    pImpEE->UndoActionEnd( EDITUNDO_INSERT );

    if ( bSelect )
    {
        DBG_ASSERT( !aPaM1.DbgIsBuggy( pImpEE->GetEditDoc() ), "Insert: PaM broken" );
        pImpEditView->SetEditSelection( EditSelection( aPaM1, aPaM2 ) );
    }
    else
        pImpEditView->SetEditSelection( EditSelection( aPaM2, aPaM2 ) );

    pImpEE->FormatAndUpdate( this );
}

sal_Bool EditView::PostKeyEvent( const KeyEvent& rKeyEvent, Window* pFrameWin )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->PostKeyEvent( rKeyEvent, pFrameWin );
}

sal_Bool EditView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->MouseButtonUp( rMouseEvent );
}

sal_Bool EditView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->MouseButtonDown( rMouseEvent );
}

sal_Bool EditView::MouseMove( const MouseEvent& rMouseEvent )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->MouseMove( rMouseEvent );
}

void EditView::Command( const CommandEvent& rCEvt )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->Command( rCEvt );
}

void EditView::ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    if ( pImpEditView->pEditEngine->HasView( this ) )
    {
        // The control word is more important:
        if ( !pImpEditView->DoAutoScroll() )
            bGotoCursor = sal_False;
        pImpEditView->ShowCursor( bGotoCursor, bForceVisCursor );
    }
}

void EditView::HideCursor()
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->GetCursor()->Hide();
}

Pair EditView::Scroll( long ndX, long ndY, BYTE nRangeCheck )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->Scroll( ndX, ndY, nRangeCheck );
}

const SfxItemSet& EditView::GetEmptyItemSet()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->GetEmptyItemSet();
}

void EditView::SetAttribs( const SfxItemSet& rSet )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    DBG_ASSERT( !pImpEditView->aEditSelection.IsInvalid(), "Blind Selection in ...." );

    pImpEditView->DrawSelection();
    PIMPEE->SetAttribs( pImpEditView->GetEditSelection(), rSet, ATTRSPECIAL_WHOLEWORD );
    PIMPEE->FormatAndUpdate( this );
}

void EditView::SetParaAttribs( const SfxItemSet& rSet, sal_uInt16 nPara )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->SetParaAttribs( nPara, rSet );
    // When you change paragraph attributes you must always format...
    PIMPEE->FormatAndUpdate( this );
}

void EditView::RemoveAttribsKeepLanguages( sal_Bool bRemoveParaAttribs )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    pImpEditView->DrawSelection();
    PIMPEE->UndoActionStart( EDITUNDO_RESETATTRIBS );
    EditSelection aSelection( pImpEditView->GetEditSelection() );

    for (sal_uInt16 nWID = EE_ITEMS_START; nWID <= EE_ITEMS_END; ++nWID)
    {
        bool bIsLang =  EE_CHAR_LANGUAGE     == nWID ||
                        EE_CHAR_LANGUAGE_CJK == nWID ||
                        EE_CHAR_LANGUAGE_CTL == nWID;
        if (!bIsLang)
            PIMPEE->RemoveCharAttribs( aSelection, bRemoveParaAttribs, nWID );
    }

    PIMPEE->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    PIMPEE->FormatAndUpdate( this );
}

void EditView::RemoveAttribs( sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    pImpEditView->DrawSelection();
    PIMPEE->UndoActionStart( EDITUNDO_RESETATTRIBS );
    PIMPEE->RemoveCharAttribs( pImpEditView->GetEditSelection(), bRemoveParaAttribs, nWhich  );
    PIMPEE->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    PIMPEE->FormatAndUpdate( this );
}

void EditView::RemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->UndoActionStart( EDITUNDO_RESETATTRIBS );
    PIMPEE->RemoveCharAttribs( nPara, nWhich );
    PIMPEE->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    PIMPEE->FormatAndUpdate( this );
}

SfxItemSet EditView::GetAttribs()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    DBG_ASSERT( !pImpEditView->aEditSelection.IsInvalid(), "Blind Selection in ...." );
    return PIMPEE->GetAttribs( pImpEditView->GetEditSelection() );
}

void EditView::Undo()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Undo( this );
}

void EditView::Redo()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Redo( this );
}

ULONG EditView::Read( SvStream& rInput, const String& rBaseURL, EETextFormat eFormat, sal_Bool bSelect, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    EditSelection aOldSel( pImpEditView->GetEditSelection() );
    pImpEditView->DrawSelection();
    PIMPEE->UndoActionStart( EDITUNDO_READ );
    EditPaM aEndPaM = PIMPEE->Read( rInput, rBaseURL, eFormat, aOldSel, pHTTPHeaderAttrs );
    PIMPEE->UndoActionEnd( EDITUNDO_READ );
    EditSelection aNewSel( aEndPaM, aEndPaM );
    if ( bSelect )
    {
        aOldSel.Adjust( PIMPEE->GetEditDoc() );
        aNewSel.Min() = aOldSel.Min();
    }

    pImpEditView->SetEditSelection( aNewSel );
    sal_Bool bGotoCursor = pImpEditView->DoAutoScroll();
    ShowCursor( bGotoCursor );

    return rInput.GetError();
}

#ifndef SVX_LIGHT
ULONG EditView::Write( SvStream& rOutput, EETextFormat eFormat )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Write( rOutput, eFormat, pImpEditView->GetEditSelection() );
    ShowCursor();
    return rOutput.GetError();
}
#endif

void EditView::Cut()
{
    DBG_CHKTHIS( EditView, 0 );
    Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
    pImpEditView->CutCopy( aClipBoard, sal_True );
}

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > EditView::GetTransferable()
{
    uno::Reference< datatransfer::XTransferable > xData = GetEditEngine()->pImpEditEngine->CreateTransferable( pImpEditView->GetEditSelection() );
    return xData;
}

void EditView::Copy()
{
    DBG_CHKTHIS( EditView, 0 );
    Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
    pImpEditView->CutCopy( aClipBoard, sal_False );
}

void EditView::Paste()
{
    DBG_CHKTHIS( EditView, 0 );
    Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
    pImpEditView->Paste( aClipBoard, sal_False );
}

void EditView::PasteSpecial()
{
    DBG_CHKTHIS( EditView, 0 );
    Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
    pImpEditView->Paste(aClipBoard, sal_True );
}

void EditView::EnablePaste( sal_Bool bEnable )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->EnablePaste( bEnable );
}

sal_Bool EditView::IsPasteEnabled() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->IsPasteEnabled();
}

Point EditView::GetWindowPosTopLeft( sal_uInt16 nParagraph )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    Point aDocPos( pImpEditView->pEditEngine->GetDocPosTopLeft( nParagraph ) );
    return pImpEditView->GetWindowPos( aDocPos );
}

sal_uInt16 EditView::GetParagraph( const Point& rMousePosPixel )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    Point aMousePos( rMousePosPixel );
    aMousePos = GetWindow()->PixelToLogic( aMousePos );
    Point aDocPos( pImpEditView->GetDocPos( aMousePos ) );
    sal_uInt16 nParagraph = PIMPEE->GetParaPortions().FindParagraph( aDocPos.Y() );
    return nParagraph;
}

void EditView::IndentBlock()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    PIMPEE->IndentBlock( this, sal_True );
}

void EditView::UnindentBlock()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    PIMPEE->IndentBlock( this, sal_False );
}

EESelectionMode EditView::GetSelectionMode() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetSelectionMode();
}

void EditView::SetSelectionMode( EESelectionMode eMode )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetSelectionMode( eMode );
}

XubString EditView::GetSelected()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->GetSelected( pImpEditView->GetEditSelection() );
}

void EditView::MoveParagraphs( Range aParagraphs, sal_uInt16 nNewPos )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->UndoActionStart( EDITUNDO_MOVEPARAS );
    PIMPEE->MoveParagraphs( aParagraphs, nNewPos, this );
    PIMPEE->UndoActionEnd( EDITUNDO_MOVEPARAS );
}

void EditView::MoveParagraphs( long nDiff )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    ESelection aSel = GetSelection();
    Range aRange( aSel.nStartPara, aSel.nEndPara );
    aRange.Justify();
    long nDest = ( nDiff > 0  ? aRange.Max() : aRange.Min() ) + nDiff;
    if ( nDiff > 0 )
        nDest++;
    DBG_ASSERT( ( nDest >= 0 ) && ( nDest <= pImpEditView->pEditEngine->GetParagraphCount() ), "MoveParagraphs - wrong Parameters!" );
    MoveParagraphs( aRange,
        sal::static_int_cast< USHORT >( nDest ) );
}

void EditView::SetBackgroundColor( const Color& rColor )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->SetBackgroundColor( rColor );
}

Color EditView::GetBackgroundColor() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->GetBackgroundColor();
}

void EditView::SetControlWord( sal_uInt32 nWord )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->nControl = nWord;
}

sal_uInt32 EditView::GetControlWord() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->nControl;
}

EditTextObject* EditView::CreateTextObject()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->CreateTextObject( pImpEditView->GetEditSelection() );
}

void EditView::InsertText( const EditTextObject& rTextObject )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->DrawSelection();

    PIMPEE->UndoActionStart( EDITUNDO_INSERT );
    EditSelection aTextSel( PIMPEE->InsertText( rTextObject, pImpEditView->GetEditSelection() ) );
    PIMPEE->UndoActionEnd( EDITUNDO_INSERT );

    aTextSel.Min() = aTextSel.Max();    // Selection not retained.
    pImpEditView->SetEditSelection( aTextSel );
    PIMPEE->FormatAndUpdate( this );
}

void EditView::InsertText( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xDataObj, const String& rBaseURL, BOOL bUseSpecial )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    PIMPEE->UndoActionStart( EDITUNDO_INSERT );
    pImpEditView->DeleteSelected();
    EditSelection aTextSel( PIMPEE->InsertText( xDataObj, rBaseURL, pImpEditView->GetEditSelection().Max(), bUseSpecial ) );
    PIMPEE->UndoActionEnd( EDITUNDO_INSERT );

    aTextSel.Min() = aTextSel.Max();    // Selection not retained.
    pImpEditView->SetEditSelection( aTextSel );
    PIMPEE->FormatAndUpdate( this );
}

sal_Bool EditView::Drop( const DropEvent& )
{
    return FALSE;
}

ESelection EditView::GetDropPos()
{
    OSL_FAIL( "GetDropPos - Why?!" );
    return ESelection();
}

sal_Bool EditView::QueryDrop( DropEvent& )
{
    return FALSE;
}

void EditView::SetEditEngineUpdateMode( sal_Bool bUpdate )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->SetUpdateMode( bUpdate, this );
}

void EditView::ForceUpdate()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->SetUpdateMode( sal_True, this, sal_True );
}

void EditView::SetStyleSheet( SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    EditSelection aSel( pImpEditView->GetEditSelection() );
    PIMPEE->UndoActionStart( EDITUNDO_STYLESHEET );
    PIMPEE->SetStyleSheet( aSel, pStyle );
    PIMPEE->UndoActionEnd( EDITUNDO_STYLESHEET );
}

SfxStyleSheet* EditView::GetStyleSheet() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );


    EditSelection aSel( pImpEditView->GetEditSelection() );
    aSel.Adjust( PIMPEE->GetEditDoc() );
    sal_uInt16 nStartPara = PIMPEE->GetEditDoc().GetPos( aSel.Min().GetNode() );
    sal_uInt16 nEndPara = PIMPEE->GetEditDoc().GetPos( aSel.Max().GetNode() );

    SfxStyleSheet* pStyle = NULL;
    for ( sal_uInt16 n = nStartPara; n <= nEndPara; n++ )
    {
        SfxStyleSheet* pTmpStyle = PIMPEE->GetStyleSheet( n );
        if ( ( n != nStartPara ) && ( pStyle != pTmpStyle ) )
            return NULL;    // Not unique.
        pStyle = pTmpStyle;
    }
    return pStyle;
}

sal_Bool EditView::IsInsertMode() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->IsInsertMode();
}

void EditView::SetInsertMode( sal_Bool bInsert )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetInsertMode( bInsert );
}

void EditView::SetAnchorMode( EVAnchorMode eMode )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetAnchorMode( eMode );
}

EVAnchorMode EditView::GetAnchorMode() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetAnchorMode();
}

void EditView::TransliterateText( sal_Int32 nTransliterationMode )
{
    DBG_CHKTHIS( EditView, 0 );
    EditSelection aOldSel( pImpEditView->GetEditSelection() );
    EditSelection aNewSel = PIMPEE->TransliterateText( pImpEditView->GetEditSelection(), nTransliterationMode );
    if ( aNewSel != aOldSel )
    {
        pImpEditView->DrawSelection();
        pImpEditView->SetEditSelection( aNewSel );
        pImpEditView->DrawSelection();
    }
}


sal_Bool EditView::MatchGroup()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    EditSelection aNewSel( PIMPEE->MatchGroup( pImpEditView->GetEditSelection() ) );
    if ( aNewSel.HasRange() )
    {
        pImpEditView->DrawSelection();
        pImpEditView->SetEditSelection( aNewSel );
        pImpEditView->DrawSelection();
        ShowCursor();
        return sal_True;
    }
    return sal_False;
}

void EditView::CompleteAutoCorrect( Window* pFrameWin )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !pImpEditView->HasSelection() && PIMPEE->GetStatus().DoAutoCorrect() )
    {
        pImpEditView->DrawSelection();
        EditSelection aSel = pImpEditView->GetEditSelection();
        aSel = PIMPEE->EndOfWord( aSel.Max() );
        aSel = PIMPEE->AutoCorrect( aSel, 0, !IsInsertMode(), pFrameWin );
        pImpEditView->SetEditSelection( aSel );
        if ( PIMPEE->IsModified() )
            PIMPEE->FormatAndUpdate( this );
    }
}

EESpellState EditView::StartSpeller( sal_Bool bMultipleDoc )
{
#ifdef SVX_LIGHT
    return EE_SPELL_NOSPELLER;
#else
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !PIMPEE->GetSpeller().is() )
        return EE_SPELL_NOSPELLER;

    return PIMPEE->Spell( this, bMultipleDoc );
#endif
}

EESpellState EditView::StartThesaurus()
{
#ifdef SVX_LIGHT
    return EE_SPELL_NOSPELLER;
#else
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !PIMPEE->GetSpeller().is() )
        return EE_SPELL_NOSPELLER;

    return PIMPEE->StartThesaurus( this );
#endif
}


void EditView::StartTextConversion(
        LanguageType nSrcLang, LanguageType nDestLang, const Font *pDestFont,
        INT32 nOptions, BOOL bIsInteractive, BOOL bMultipleDoc )
{
#ifdef SVX_LIGHT
#else
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Convert( this, nSrcLang, nDestLang, pDestFont, nOptions, bIsInteractive, bMultipleDoc );
#endif
}


sal_uInt16 EditView::StartSearchAndReplace( const SvxSearchItem& rSearchItem )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->StartSearchAndReplace( this, rSearchItem );
}

sal_Bool EditView::IsCursorAtWrongSpelledWord( sal_Bool bMarkIfWrong )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    sal_Bool bIsWrong = sal_False;
    if ( !HasSelection() )
    {
        EditPaM aPaM = pImpEditView->GetEditSelection().Max();
        bIsWrong = pImpEditView->IsWrongSpelledWord( aPaM, bMarkIfWrong );
    }
    return bIsWrong;
}

sal_Bool EditView::IsWrongSpelledWordAtPos( const Point& rPosPixel, sal_Bool bMarkIfWrong )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    Point aPos ( pImpEditView->GetWindow()->PixelToLogic( rPosPixel ) );
    aPos = pImpEditView->GetDocPos( aPos );
    EditPaM aPaM = pImpEditView->pEditEngine->pImpEditEngine->GetPaM( aPos, sal_False );
    return pImpEditView->IsWrongSpelledWord( aPaM , bMarkIfWrong );
}


static Image lcl_GetImageFromPngUrl( const OUString &rFileUrl )
{
    Image aRes;
    OUString aTmp;
    osl::FileBase::getSystemPathFromFileURL( rFileUrl, aTmp );
//    ::rtl::OString aPath = OString( aTmp.getStr(), aTmp.getLength(), osl_getThreadTextEncoding() );
#if defined(WNT)
//    aTmp = lcl_Win_GetShortPathName( aTmp );
#endif
    Graphic aGraphic;
    const String aFilterName( RTL_CONSTASCII_USTRINGPARAM( IMP_PNG ) );
    if( GRFILTER_OK == GraphicFilter::LoadGraphic( aTmp, aFilterName, aGraphic ) )
    {
        aRes = Image( aGraphic.GetBitmapEx() );
    }
    return aRes;
}


void EditView::ExecuteSpellPopup( const Point& rPosPixel, Link* pCallBack )
{
#ifndef SVX_LIGHT
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    Point aPos ( pImpEditView->GetWindow()->PixelToLogic( rPosPixel ) );
    aPos = pImpEditView->GetDocPos( aPos );
    EditPaM aPaM = pImpEditView->pEditEngine->pImpEditEngine->GetPaM( aPos, sal_False );
    Reference< XSpellChecker1 >  xSpeller( PIMPEE->GetSpeller() );
    ESelection aOldSel = GetSelection();
    if ( xSpeller.is() && pImpEditView->IsWrongSpelledWord( aPaM, sal_True ) )
    {
        PopupMenu aPopupMenu( EditResId( RID_MENU_SPELL ) );
        PopupMenu *pAutoMenu = aPopupMenu.GetPopupMenu( MN_AUTOCORR );
        PopupMenu *pInsertMenu = aPopupMenu.GetPopupMenu( MN_INSERT );  // add word to user-dictionaries
        pInsertMenu->SetMenuFlags( MENU_FLAG_NOAUTOMNEMONICS );         //! necessary to retrieve the correct dictionary names later

        EditPaM aPaM2( aPaM );
        aPaM2.GetIndex()++;

        // Are there any replace suggestions?
        String aSelected( GetSelected() );
        //
        // restrict the maximal number of suggestions displayed
        // in the context menu.
        // Note: That could of course be done by clipping the
        // resulting sequence but the current third party
        // implementations result differs greatly if the number of
        // suggestions to be retuned gets changed. Statistically
        // it gets much better if told to return e.g. only 7 strings
        // than returning e.g. 16 suggestions and using only the
        // first 7. Thus we hand down the value to use to that
        // implementation here by providing an additional parameter.
        Sequence< PropertyValue > aPropVals(1);
        PropertyValue &rVal = aPropVals.getArray()[0];
        rVal.Name = OUString(RTL_CONSTASCII_USTRINGPARAM( UPN_MAX_NUMBER_OF_SUGGESTIONS ));
        rVal.Value <<= (INT16) 7;
        //
        // Are there any replace suggestions?
        Reference< XSpellAlternatives >  xSpellAlt =
                xSpeller->spell( aSelected, PIMPEE->GetLanguage( aPaM2 ), aPropVals );

        Reference< XLanguageGuessing >  xLangGuesser( EE_DLL()->GetGlobalData()->GetLanguageGuesser() );

        // check if text might belong to a different language...
        LanguageType nGuessLangWord = LANGUAGE_NONE;
        LanguageType nGuessLangPara = LANGUAGE_NONE;
        if (xSpellAlt.is() && xLangGuesser.is())
        {
            String aParaText;
            ContentNode *pNode = aPaM.GetNode();
            if (pNode)
            {
                aParaText = *pNode;
            }
            else
            {
                OSL_FAIL( "content node is NULL" );
            }

            nGuessLangWord = lcl_CheckLanguage( xSpellAlt->getWord(), xSpeller, xLangGuesser, sal_False );
            nGuessLangPara = lcl_CheckLanguage( aParaText, xSpeller, xLangGuesser, sal_True );
        }
        if (nGuessLangWord != LANGUAGE_NONE || nGuessLangPara != LANGUAGE_NONE)
        {
            // make sure LANGUAGE_NONE gets not used as menu entry
            if (nGuessLangWord == LANGUAGE_NONE)
                nGuessLangWord = nGuessLangPara;
            if (nGuessLangPara == LANGUAGE_NONE)
                nGuessLangPara = nGuessLangWord;

            aPopupMenu.InsertSeparator();
            String aTmpWord( SvtLanguageTable::GetLanguageString( nGuessLangWord ) );
            String aTmpPara( SvtLanguageTable::GetLanguageString( nGuessLangPara ) );
            String aWordStr( EditResId( RID_STR_WORD ) );
            aWordStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%x" ) ), aTmpWord );
            String aParaStr( EditResId( RID_STR_PARAGRAPH ) );
            aParaStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%x" ) ), aTmpPara );
            aPopupMenu.InsertItem( MN_WORDLANGUAGE, aWordStr );
            aPopupMenu.SetHelpId( MN_WORDLANGUAGE, HID_EDITENG_SPELLER_WORDLANGUAGE );
            aPopupMenu.InsertItem( MN_PARALANGUAGE, aParaStr );
            aPopupMenu.SetHelpId( MN_PARALANGUAGE, HID_EDITENG_SPELLER_PARALANGUAGE );
        }

        // ## Create mnemonics here
        if ( Application::IsAutoMnemonicEnabled() )
        {
            aPopupMenu.CreateAutoMnemonics();
            aPopupMenu.SetMenuFlags( aPopupMenu.GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );
        }

        // Replace suggestions...
        Sequence< OUString > aAlt;
        if (xSpellAlt.is())
            aAlt = xSpellAlt->getAlternatives();
        const OUString *pAlt = aAlt.getConstArray();
        sal_uInt16 nWords = (USHORT) aAlt.getLength();
        if ( nWords )
        {
            for ( sal_uInt16 nW = 0; nW < nWords; nW++ )
            {
                String aAlternate( pAlt[nW] );
                aPopupMenu.InsertItem( MN_ALTSTART+nW, aAlternate, 0, nW );
                pAutoMenu->InsertItem( MN_AUTOSTART+nW, aAlternate, 0, nW );
            }
            aPopupMenu.InsertSeparator( nWords );
        }
        else
            aPopupMenu.RemoveItem( MN_AUTOCORR );   // delete?

        SvtLinguConfig aCfg;

        Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
        Sequence< Reference< XDictionary >  > aDics;
        const Reference< XDictionary >  *pDic = NULL;
        if (xDicList.is())
        {
            // add the default positive dictionary to dic-list (if not already done).
            // This is to ensure that there is at least one dictionary to which
            // words could be added.
            uno::Reference< linguistic2::XDictionary >  xDic( SvxGetOrCreatePosDic( xDicList ) );
            if (xDic.is())
                xDic->setActive( sal_True );

            aDics = xDicList->getDictionaries();
            pDic  = aDics.getConstArray();
            sal_uInt16 nCheckedLanguage = PIMPEE->GetLanguage( aPaM2 );
            sal_uInt16 nDicCount = (USHORT)aDics.getLength();
            for (sal_uInt16 i = 0; i < nDicCount; i++)
            {
                uno::Reference< linguistic2::XDictionary >  xDicTmp( pDic[i], uno::UNO_QUERY );
                if (!xDicTmp.is() || SvxGetIgnoreAllList() == xDicTmp)
                    continue;

                uno::Reference< frame::XStorable > xStor( xDicTmp, uno::UNO_QUERY );
                LanguageType nActLanguage = SvxLocaleToLanguage( xDicTmp->getLocale() );
                if( xDicTmp->isActive()
                    &&  xDicTmp->getDictionaryType() != linguistic2::DictionaryType_NEGATIVE
                    && (nCheckedLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage )
                    && (!xStor.is() || !xStor->isReadonly()) )
                {
                    // the extra 1 is because of the (possible) external
                    // linguistic entry above
                    USHORT nPos = MN_DICTSTART + i;
                    pInsertMenu->InsertItem( nPos, xDicTmp->getName() );

                    uno::Reference< lang::XServiceInfo > xSvcInfo( xDicTmp, uno::UNO_QUERY );
                    if (xSvcInfo.is())
                    {
                        OUString aDictionaryImageUrl( aCfg.GetSpellAndGrammarContextDictionaryImage(
                                xSvcInfo->getImplementationName()) );
                        if (aDictionaryImageUrl.getLength() > 0)
                        {
                            Image aImage( lcl_GetImageFromPngUrl( aDictionaryImageUrl ) );
                            pInsertMenu->SetItemImage( nPos, aImage );
                        }
                    }
                }
            }
        }

        if ( !pInsertMenu->GetItemCount() )
            aPopupMenu.EnableItem( MN_INSERT, sal_False );

        aPopupMenu.RemoveDisabledEntries( sal_True, sal_True );

        Rectangle aTempRect = PIMPEE->PaMtoEditCursor( aPaM, GETCRSR_TXTONLY );
        Point aScreenPos = pImpEditView->GetWindowPos( aTempRect.TopLeft() );
        aScreenPos = pImpEditView->GetWindow()->OutputToScreenPixel( aScreenPos );
        aTempRect = pImpEditView->GetWindow()->LogicToPixel( Rectangle(aScreenPos, aTempRect.GetSize() ));

        sal_uInt16 nId = aPopupMenu.Execute( pImpEditView->GetWindow(), aTempRect, POPUPMENU_NOMOUSEUPCLOSE );
        if ( nId == MN_IGNORE )
        {
            String aWord = pImpEditView->SpellIgnoreOrAddWord( sal_False );
            if ( pCallBack )
            {
                SpellCallbackInfo aInf( SPELLCMD_IGNOREWORD, aWord );
                pCallBack->Call( &aInf );
            }
            SetSelection( aOldSel );
        }
        else if ( ( nId == MN_WORDLANGUAGE ) || ( nId == MN_PARALANGUAGE ) )
        {
            LanguageType nLangToUse = (nId == MN_WORDLANGUAGE) ? nGuessLangWord : nGuessLangPara;
            sal_uInt16 nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nLangToUse );

            SfxItemSet aAttrs = GetEditEngine()->GetEmptyItemSet();
            if (nScriptType == SCRIPTTYPE_LATIN)
                aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE ) );
            if (nScriptType == SCRIPTTYPE_COMPLEX)
                aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CTL ) );
            if (nScriptType == SCRIPTTYPE_ASIAN)
                aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CJK ) );
            if ( nId == MN_PARALANGUAGE )
            {
                ESelection aSel = GetSelection();
                aSel.nStartPos = 0;
                aSel.nEndPos = 0xFFFF;
                SetSelection( aSel );
            }
            SetAttribs( aAttrs );
            PIMPEE->StartOnlineSpellTimer();

            if ( pCallBack )
            {
                SpellCallbackInfo aInf( ( nId == MN_WORDLANGUAGE ) ? SPELLCMD_WORDLANGUAGE : SPELLCMD_PARALANGUAGE, nLangToUse );
                pCallBack->Call( &aInf );
            }
            SetSelection( aOldSel );
        }
        else if ( nId == MN_SPELLING )
        {
            if ( !pCallBack )
            {
                // Set Cursor before word...
                EditPaM aCursor = pImpEditView->GetEditSelection().Min();
                pImpEditView->DrawSelection();
                pImpEditView->SetEditSelection( EditSelection( aCursor, aCursor ) );
                pImpEditView->DrawSelection();
                // Crashes when no SfxApp
                PIMPEE->Spell( this, sal_False );
            }
            else
            {
                SpellCallbackInfo aInf( SPELLCMD_STARTSPELLDLG, String() );
                pCallBack->Call( &aInf );
            }
        }
        else if ( nId >= MN_DICTSTART )
        {
            String aDicName ( pInsertMenu->GetItemText(nId) );

            uno::Reference< linguistic2::XDictionary >      xDic;
            if (xDicList.is())
                xDic = xDicList->getDictionaryByName( aDicName );

            if (xDic.is())
                xDic->add( aSelected, sal_False, String() );
            // save modified user-dictionary if it is persistent
            Reference< frame::XStorable >  xSavDic( xDic, UNO_QUERY );
            if (xSavDic.is())
                xSavDic->store();

            aPaM.GetNode()->GetWrongList()->GetInvalidStart() = 0;
            aPaM.GetNode()->GetWrongList()->GetInvalidEnd() = aPaM.GetNode()->Len();
            PIMPEE->StartOnlineSpellTimer();

            if ( pCallBack )
            {
                SpellCallbackInfo aInf( SPELLCMD_ADDTODICTIONARY, aSelected );
                pCallBack->Call( &aInf );
            }
            SetSelection( aOldSel );
        }
        else if ( nId >= MN_AUTOSTART )
        {
            DBG_ASSERT(nId - MN_AUTOSTART < aAlt.getLength(), "index out of range");
            String aWord = pAlt[nId - MN_AUTOSTART];
            SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get()->GetAutoCorrect();
            if ( pAutoCorrect )
                pAutoCorrect->PutText( aSelected, aWord, PIMPEE->GetLanguage( aPaM2 ) );
            InsertText( aWord );
        }
        else if ( nId >= MN_ALTSTART )  // Replace
        {
            DBG_ASSERT(nId - MN_ALTSTART < aAlt.getLength(), "index out of range");
            String aWord = pAlt[nId - MN_ALTSTART];
            InsertText( aWord );
        }
        else
        {
            SetSelection( aOldSel );
        }
    }
#endif
}

void EditView::SpellIgnoreWord()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->SpellIgnoreOrAddWord( sal_False );
}

sal_Bool EditView::SelectCurrentWord( sal_Int16 nWordType )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    EditSelection aCurSel( pImpEditView->GetEditSelection() );
    pImpEditView->DrawSelection();
    aCurSel = PIMPEE->SelectWord( aCurSel.Max(), nWordType );
    pImpEditView->SetEditSelection( aCurSel );
    pImpEditView->DrawSelection();
    ShowCursor( sal_True, sal_False );
    return aCurSel.HasRange() ? sal_True : sal_False;
}

void EditView::InsertField( const SvxFieldItem& rFld )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    ImpEditEngine* pImpEE = PIMPEE;
    pImpEditView->DrawSelection();
    pImpEE->UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM( pImpEE->InsertField( pImpEditView->GetEditSelection(), rFld ) );
    pImpEE->UndoActionEnd( EDITUNDO_INSERT );
    pImpEditView->SetEditSelection( EditSelection( aPaM, aPaM ) );
    pImpEE->UpdateFields();
    pImpEE->FormatAndUpdate( this );
}

const SvxFieldItem* EditView::GetFieldUnderMousePointer() const
{
    DBG_CHKTHIS( EditView, 0 );
    sal_uInt16 nPara, nPos;
    return GetFieldUnderMousePointer( nPara, nPos );
}

const SvxFieldItem* EditView::GetField( const Point& rPos, sal_uInt16* pPara, sal_uInt16* pPos ) const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->GetField( rPos, pPara, pPos );
}

const SvxFieldItem* EditView::GetFieldUnderMousePointer( sal_uInt16& nPara, sal_uInt16& nPos ) const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    Point aPos = pImpEditView->GetWindow()->GetPointerPosPixel();
    aPos = pImpEditView->GetWindow()->PixelToLogic( aPos );
    return GetField( aPos, &nPara, &nPos );
}

const SvxFieldItem* EditView::GetFieldAtSelection() const
{
    EditSelection aSel( pImpEditView->GetEditSelection() );
    aSel.Adjust( pImpEditView->pEditEngine->pImpEditEngine->GetEditDoc() );
    // Only when cursor is in font of field, no selection,
    // or only selecting field
    if ( ( aSel.Min().GetNode() == aSel.Max().GetNode() ) &&
         ( ( aSel.Max().GetIndex() == aSel.Min().GetIndex() ) ||
           ( aSel.Max().GetIndex() == aSel.Min().GetIndex()+1 ) ) )
    {
        EditPaM aPaM = aSel.Min();
        const CharAttribArray& rAttrs = aPaM.GetNode()->GetCharAttribs().GetAttribs();
        sal_uInt16 nXPos = aPaM.GetIndex();
        for ( sal_uInt16 nAttr = rAttrs.Count(); nAttr; )
        {
            EditCharAttrib* pAttr = rAttrs[--nAttr];
            if ( pAttr->GetStart() == nXPos )
                if ( pAttr->Which() == EE_FEATURE_FIELD )
                {
                    DBG_ASSERT( pAttr->GetItem()->ISA( SvxFieldItem ), "No FeldItem..." );
                    return (const SvxFieldItem*)pAttr->GetItem();
                }
        }
    }
    return 0;
}

XubString EditView::GetWordUnderMousePointer() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    Rectangle aRect;
    return GetWordUnderMousePointer( aRect );
}

XubString EditView::GetWordUnderMousePointer( Rectangle& rWordRect ) const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    Point aPos = pImpEditView->GetWindow()->GetPointerPosPixel();
    aPos = pImpEditView->GetWindow()->PixelToLogic( aPos );

    XubString aWord;

    if( GetOutputArea().IsInside( aPos ) )
    {
        ImpEditEngine* pImpEE = pImpEditView->pEditEngine->pImpEditEngine;
        Point aDocPos( pImpEditView->GetDocPos( aPos ) );
        EditPaM aPaM = pImpEE->GetPaM( aDocPos, sal_False );
        EditSelection aWordSel = pImpEE->SelectWord( aPaM );

        Rectangle aTopLeftRec( pImpEE->PaMtoEditCursor( aWordSel.Min() ) );
        Rectangle aBottomRightRec( pImpEE->PaMtoEditCursor( aWordSel.Max() ) );

#if OSL_DEBUG_LEVEL > 1
        DBG_ASSERT( aTopLeftRec.Top() == aBottomRightRec.Top(), "Top () is different in one line?");
#endif

        Point aPnt1( pImpEditView->GetWindowPos( aTopLeftRec.TopLeft() ) );
        Point aPnt2( pImpEditView->GetWindowPos( aBottomRightRec.BottomRight()) );
        rWordRect = Rectangle( aPnt1, aPnt2 );
        aWord = pImpEE->GetSelected( aWordSel );
    }

    return aWord;
}

void EditView::SetInvalidateMore( sal_uInt16 nPixel )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetInvalidateMore( nPixel );
}

sal_uInt16 EditView::GetInvalidateMore() const
{
    DBG_CHKTHIS( EditView, 0 );
    return (sal_uInt16)pImpEditView->GetInvalidateMore();
}

static void ChangeFontSizeImpl( EditView* pEditView, bool bGrow, const ESelection& rSel, const FontList* pFontList )
{
    pEditView->SetSelection( rSel );

    SfxItemSet aSet( pEditView->GetAttribs() );
    if( EditView::ChangeFontSize( bGrow, aSet, pFontList ) )
    {
        SfxItemSet aNewSet( pEditView->GetEmptyItemSet() );
        aNewSet.Put( aSet.Get( EE_CHAR_FONTHEIGHT ), EE_CHAR_FONTHEIGHT );
        aNewSet.Put( aSet.Get( EE_CHAR_FONTHEIGHT_CJK ), EE_CHAR_FONTHEIGHT_CJK );
        aNewSet.Put( aSet.Get( EE_CHAR_FONTHEIGHT_CTL ), EE_CHAR_FONTHEIGHT_CTL );
        pEditView->SetAttribs( aNewSet );
    }
}

void EditView::ChangeFontSize( bool bGrow, const FontList* pFontList )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    EditEngine& rEditEngine = *pImpEditView->pEditEngine;

    ESelection aSel( GetSelection() );
    ESelection aOldSelection( aSel );
    aSel.Adjust();

    if( !aSel.HasRange() )
    {
        aSel = rEditEngine.GetWord( aSel, com::sun::star::i18n::WordType::DICTIONARY_WORD );
    }

    if( aSel.HasRange() )
    {
        for( USHORT nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
        {
            SvUShorts aPortions;
            rEditEngine.GetPortions( nPara, aPortions );

            if( aPortions.Count() == 0 )
                aPortions.Insert( rEditEngine.GetTextLen(nPara), 0 );

            const USHORT nBeginPos = (nPara == aSel.nStartPara) ? aSel.nStartPos : 0;
            const USHORT nEndPos = (nPara == aSel.nEndPara) ? aSel.nEndPos : 0xffff;

            for ( USHORT nPos = 0; nPos < aPortions.Count(); ++nPos )
            {
                USHORT nPortionEnd   = aPortions.GetObject( nPos );
                USHORT nPortionStart = nPos > 0 ? aPortions.GetObject( nPos - 1 ) : 0;

                if( (nPortionEnd < nBeginPos) || (nPortionStart > nEndPos) )
                    continue;

                if( nPortionStart < nBeginPos )
                    nPortionStart = nBeginPos;
                if( nPortionEnd > nEndPos )
                    nPortionEnd = nEndPos;

                if( nPortionStart == nPortionEnd )
                    continue;

                ESelection aPortionSel( nPara, nPortionStart, nPara, nPortionEnd );
                ChangeFontSizeImpl( this, bGrow, aPortionSel, pFontList );
            }
        }
    }
    else
    {
        ChangeFontSizeImpl( this, bGrow, aSel, pFontList );
    }

    SetSelection( aOldSelection );
}

bool EditView::ChangeFontSize( bool bGrow, SfxItemSet& rSet, const FontList* pFontList )
{
    static const sal_uInt16 gFontSizeWichMap[] = { EE_CHAR_FONTHEIGHT, EE_CHAR_FONTHEIGHT_CJK, EE_CHAR_FONTHEIGHT_CTL, 0 };

    const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(&rSet.Get( EE_CHAR_FONTINFO ));
    if( !pFontItem || !pFontList )
        return false;

    bool bRet = false;

    const sal_uInt16* pWhich = gFontSizeWichMap;
    while( *pWhich )
    {
        SvxFontHeightItem aFontHeightItem( static_cast<const SvxFontHeightItem&>(rSet.Get( *pWhich )) );
        long nHeight = aFontHeightItem.GetHeight();
        const SfxMapUnit eUnit = rSet.GetPool()->GetMetric( *pWhich );
        nHeight = OutputDevice::LogicToLogic( nHeight * 10, (MapUnit)eUnit, MAP_POINT );

        FontInfo aFontInfo = pFontList->Get( pFontItem->GetFamilyName(), pFontItem->GetStyleName() );
        const long* pAry = pFontList->GetSizeAry( aFontInfo );

        if( bGrow )
        {
            while( *pAry )
            {
                if( *pAry > nHeight )
                {
                    nHeight = *pAry;
                    break;
                }
                pAry++;
            }

            if( *pAry == 0 )
            {
                nHeight += (nHeight + 5) / 10;
                if( nHeight > 9999 )
                    nHeight = 9999;
            }

        }
        else if( *pAry )
        {
            bool bFound = false;
            if( *pAry < nHeight )
            {
                pAry++;
                while( *pAry )
                {
                    if( *pAry >= nHeight )
                    {
                        nHeight = pAry[-1];
                        bFound = true;
                        break;
                    }
                    pAry++;
                }
            }

            if( !bFound )
            {
                nHeight -= (nHeight + 5) / 10;
                if( nHeight < 2 )
                    nHeight = 2;
            }
        }

        if( (nHeight >= 2) && (nHeight <= 9999 ) )
        {
            nHeight = OutputDevice::LogicToLogic( nHeight, MAP_POINT, (MapUnit)eUnit  ) / 10;

            if( nHeight != (long)aFontHeightItem.GetHeight() )
            {
                aFontHeightItem.SetHeight( nHeight );
                rSet.Put( aFontHeightItem, *pWhich );
                bRet = true;
            }
        }
        pWhich++;
    }
    return bRet;
}

String EditView::GetSurroundingText() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    EditSelection aSel( pImpEditView->GetEditSelection() );
    aSel.Adjust( PIMPEE->GetEditDoc() );

    if( HasSelection() )
    {
        XubString aStr = PIMPEE->GetSelected( aSel );

        // Stop reconversion if the selected text includes a line break.
        if ( aStr.Search( 0x0A ) == STRING_NOTFOUND )
        return aStr;
        else
        return String();
    }
    else
    {
        aSel.Min().SetIndex( 0 );
        aSel.Max().SetIndex( aSel.Max().GetNode()->Len() );
        return PIMPEE->GetSelected( aSel );
    }
}

Selection EditView::GetSurroundingTextSelection() const
{
    DBG_CHKTHIS( EditView, 0 );

    ESelection aSelection( GetSelection() );
    aSelection.Adjust();

    if( HasSelection() )
    {
        EditSelection aSel( pImpEditView->GetEditSelection() );
        aSel.Adjust( PIMPEE->GetEditDoc() );
        XubString aStr = PIMPEE->GetSelected( aSel );

        // Stop reconversion if the selected text includes a line break.
        if ( aStr.Search( 0x0A ) == STRING_NOTFOUND )
        return Selection( 0, aSelection.nEndPos - aSelection.nStartPos );
        else
        return Selection( 0, 0 );
    }
    else
    {
        return Selection( aSelection.nStartPos, aSelection.nEndPos );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
