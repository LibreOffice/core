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


#include <sal/macros.h>
#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <vcl/metric.hxx>

#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/langtab.hxx>
#include <vcl/graphicfilter.hxx>

#include <svl/srchitem.hxx>

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
#include <i18nlangtag/lang.h>
#include <vcl/menu.hxx>
#include <vcl/window.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/fontitem.hxx>
#include <unotools/lingucfg.hxx>
#include <osl/file.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <linguistic/lngprops.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

#define PIMPEE pImpEditView->pEditEngine->pImpEditEngine

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


DBG_NAME( EditView )


// static
LanguageType EditView::CheckLanguage(
        const OUString &rText,
        Reference< linguistic2::XSpellChecker1 > xSpell,
        Reference< linguistic2::XLanguageGuessing > xLangGuess,
        bool bIsParaText )
{
    LanguageType nLang = LANGUAGE_NONE;
    if (bIsParaText)    // check longer texts with language-guessing...
    {
        if (!xLangGuess.is())
            return nLang;

        LanguageTag aGuessTag( xLangGuess->guessPrimaryLanguage( rText, 0, rText.getLength()) );

        // If the result from language guessing does not provide a 'Country'
        // part, try to get it by looking up the locale setting of the office,
        // "Tools/Options - Language Settings - Languages: Locale setting", if
        // the language matches.
        if ( aGuessTag.getCountry().isEmpty() )
        {
            const LanguageTag& rAppLocaleTag = Application::GetSettings().GetLanguageTag();
            if (rAppLocaleTag.getLanguage() == aGuessTag.getLanguage())
                nLang = rAppLocaleTag.getLanguageType();
        }
        if (nLang == LANGUAGE_NONE) // language not found by looking up the sytem language...
            nLang = aGuessTag.makeFallback().getLanguageType();     // best known locale match
        if (nLang == LANGUAGE_SYSTEM)
            nLang = Application::GetSettings().GetLanguageTag().getLanguageType();
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
        aLangList[0] = MsLangId::resolveSystemLanguageByScriptType( aLinguOpt.nDefaultLanguage,
                ::com::sun::star::i18n::ScriptType::LATIN);
        // The one from "Tools/Options - Language Settings - Languages: User interface"
        aLangList[1] = rSettings.GetUILanguageTag().getLanguageType();
        // The one from "Tools/Options - Language Settings - Languages: Locale setting"
        aLangList[2] = rSettings.GetLanguageTag().getLanguageType();
        // en-US
        aLangList[3] = LANGUAGE_ENGLISH_US;
#ifdef DEBUG
        lang::Locale a0( LanguageTag::convertToLocale( aLangList[0] ) );
        lang::Locale a1( LanguageTag::convertToLocale( aLangList[1] ) );
        lang::Locale a2( LanguageTag::convertToLocale( aLangList[2] ) );
        lang::Locale a3( LanguageTag::convertToLocale( aLangList[3] ) );
#endif

        sal_Int32 nCount = sizeof (aLangList) / sizeof (aLangList[0]);
        for (sal_Int32 i = 0;  i < nCount;  i++)
        {
            sal_Int16 nTmpLang = aLangList[i];
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
        PIMPE->CursorMoved( pNode );
    }
    EditSelection aNewSelection( PIMPEE->ConvertSelection( rESel.nStartPara, rESel.nStartPos, rESel.nEndPara, rESel.nEndPos ) );

    // If the selection is manipulated after a KeyInput:
    PIMPE->CheckIdleFormatter();

    // Selection may not start/end at an invisible paragraph:
    const ParaPortion* pPortion = PIMPE->FindParaPortion( aNewSelection.Min().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = PIMPE->GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : PIMPE->GetEditDoc().GetObject( 0 );
        aNewSelection.Min() = EditPaM( pNode, pNode->Len() );
    }
    pPortion = PIMPE->FindParaPortion( aNewSelection.Max().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = PIMPE->GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : PIMPE->GetEditDoc().GetObject( 0 );
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

    aSelection.nStartPara = PIMPE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Min().GetNode() );
    aSelection.nEndPara = PIMPE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Max().GetNode() );

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

sal_uInt16 EditView::GetSelectedScriptType() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPE->GetScriptType( pImpEditView->GetEditSelection() );
}

void EditView::Paint( const Rectangle& rRect, OutputDevice* pTargetDevice )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Paint( pImpEditView, rRect, pTargetDevice );
}

void EditView::SetEditEngine( EditEngine* pEditEng )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->pEditEngine = pEditEng;
    EditSelection aStartSel;
    aStartSel = PIMPE->GetEditDoc().GetStartPaM();
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

void EditView::SetVisArea( const Rectangle& rRect )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetVisDocStartPos( rRect.TopLeft() );
}

const Rectangle& EditView::GetVisArea() const
{
    DBG_CHKTHIS( EditView, 0 );
    // Change return value to Rectangle in next incompatible build !!!
    static Rectangle aRect;
    aRect = pImpEditView->GetVisDocArea();
    return aRect;
}

void EditView::SetOutputArea( const Rectangle& rRect )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetOutputArea( rRect );

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

const Pointer& EditView::GetPointer() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetPointer();
}

Cursor* EditView::GetCursor() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->pCursor;
}

void EditView::InsertText( const OUString& rStr, sal_Bool bSelect )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    EditEngine* pEE = PIMPE;
    pImpEditView->DrawSelection();

    EditPaM aPaM1;
    if ( bSelect )
    {
        EditSelection aTmpSel( pImpEditView->GetEditSelection() );
        aTmpSel.Adjust( pEE->GetEditDoc() );
        aPaM1 = aTmpSel.Min();
    }

    pEE->UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM2( pEE->InsertText( pImpEditView->GetEditSelection(), rStr ) );
    pEE->UndoActionEnd( EDITUNDO_INSERT );

    if ( bSelect )
    {
        DBG_ASSERT( !aPaM1.DbgIsBuggy( pEE->GetEditDoc() ), "Insert: PaM broken" );
        pImpEditView->SetEditSelection( EditSelection( aPaM1, aPaM2 ) );
    }
    else
        pImpEditView->SetEditSelection( EditSelection( aPaM2, aPaM2 ) );

    pEE->FormatAndUpdate( this );
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

Pair EditView::Scroll( long ndX, long ndY, sal_uInt8 nRangeCheck )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->Scroll( ndX, ndY, nRangeCheck );
}

const SfxItemSet& EditView::GetEmptyItemSet()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPE->GetEmptyItemSet();
}

void EditView::SetAttribs( const SfxItemSet& rSet )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    DBG_ASSERT( !pImpEditView->aEditSelection.IsInvalid(), "Blind Selection in ...." );

    pImpEditView->DrawSelection();
    PIMPE->SetAttribs( pImpEditView->GetEditSelection(), rSet, ATTRSPECIAL_WHOLEWORD );
    PIMPE->FormatAndUpdate( this );
}

void EditView::RemoveAttribsKeepLanguages( sal_Bool bRemoveParaAttribs )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    pImpEditView->DrawSelection();
    PIMPE->UndoActionStart( EDITUNDO_RESETATTRIBS );
    EditSelection aSelection( pImpEditView->GetEditSelection() );

    for (sal_uInt16 nWID = EE_ITEMS_START; nWID <= EE_ITEMS_END; ++nWID)
    {
        bool bIsLang =  EE_CHAR_LANGUAGE     == nWID ||
                        EE_CHAR_LANGUAGE_CJK == nWID ||
                        EE_CHAR_LANGUAGE_CTL == nWID;
        if (!bIsLang)
            PIMPE->RemoveCharAttribs( aSelection, bRemoveParaAttribs, nWID );
    }

    PIMPE->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    PIMPE->FormatAndUpdate( this );
}

void EditView::RemoveAttribs( sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    pImpEditView->DrawSelection();
    PIMPE->UndoActionStart( EDITUNDO_RESETATTRIBS );
    PIMPE->RemoveCharAttribs( pImpEditView->GetEditSelection(), bRemoveParaAttribs, nWhich  );
    PIMPE->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    PIMPE->FormatAndUpdate( this );
}

void EditView::RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPE->UndoActionStart( EDITUNDO_RESETATTRIBS );
    PIMPE->RemoveCharAttribs( nPara, nWhich );
    PIMPE->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    PIMPE->FormatAndUpdate( this );
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
    PIMPE->Undo( this );
}

void EditView::Redo()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPE->Redo( this );
}

sal_uLong EditView::Read( SvStream& rInput, const String& rBaseURL, EETextFormat eFormat, sal_Bool bSelect, SvKeyValueIterator* pHTTPHeaderAttrs )
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
        aOldSel.Adjust( PIMPE->GetEditDoc() );
        aNewSel.Min() = aOldSel.Min();
    }

    pImpEditView->SetEditSelection( aNewSel );
    sal_Bool bGotoCursor = pImpEditView->DoAutoScroll();
    ShowCursor( bGotoCursor );

    return rInput.GetError();
}

void EditView::Cut()
{
    DBG_CHKTHIS( EditView, 0 );
    Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
    pImpEditView->CutCopy( aClipBoard, sal_True );
}

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > EditView::GetTransferable()
{
    uno::Reference< datatransfer::XTransferable > xData =
        GetEditEngine()->CreateTransferable( pImpEditView->GetEditSelection() );
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

Point EditView::GetWindowPosTopLeft( sal_Int32 nParagraph )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    Point aDocPos( pImpEditView->pEditEngine->GetDocPosTopLeft( nParagraph ) );
    return pImpEditView->GetWindowPos( aDocPos );
}

void EditView::SetSelectionMode( EESelectionMode eMode )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetSelectionMode( eMode );
}

OUString EditView::GetSelected()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->GetSelected( pImpEditView->GetEditSelection() );
}

void EditView::MoveParagraphs( Range aParagraphs, sal_Int32 nNewPos )
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
    MoveParagraphs( aRange, sal::static_int_cast< sal_Int32 >( nDest ) );
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

    PIMPE->UndoActionStart( EDITUNDO_INSERT );
    EditSelection aTextSel( PIMPE->InsertText( rTextObject, pImpEditView->GetEditSelection() ) );
    PIMPE->UndoActionEnd( EDITUNDO_INSERT );

    aTextSel.Min() = aTextSel.Max();    // Selection not retained.
    pImpEditView->SetEditSelection( aTextSel );
    PIMPE->FormatAndUpdate( this );
}

void EditView::InsertText( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xDataObj, const OUString& rBaseURL, sal_Bool bUseSpecial )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    PIMPE->UndoActionStart( EDITUNDO_INSERT );
    pImpEditView->DeleteSelected();
    EditSelection aTextSel =
        PIMPE->InsertText(xDataObj, rBaseURL, pImpEditView->GetEditSelection().Max(), bUseSpecial);
    PIMPE->UndoActionEnd( EDITUNDO_INSERT );

    aTextSel.Min() = aTextSel.Max();    // Selection not retained.
    pImpEditView->SetEditSelection( aTextSel );
    PIMPE->FormatAndUpdate( this );
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

SfxStyleSheet* EditView::GetStyleSheet()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );


    EditSelection aSel( pImpEditView->GetEditSelection() );
    aSel.Adjust( PIMPE->GetEditDoc() );
    sal_Int32 nStartPara = PIMPE->GetEditDoc().GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndPara = PIMPE->GetEditDoc().GetPos( aSel.Max().GetNode() );

    SfxStyleSheet* pStyle = NULL;
    for ( sal_Int32 n = nStartPara; n <= nEndPara; n++ )
    {
        SfxStyleSheet* pTmpStyle = PIMPE->GetStyleSheet( n );
        if ( ( n != nStartPara ) && ( pStyle != pTmpStyle ) )
            return NULL;    // Not unique.
        pStyle = pTmpStyle;
    }
    return pStyle;
}

const SfxStyleSheet* EditView::GetStyleSheet() const
{
    return const_cast< EditView* >( this )->GetStyleSheet();
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
    EditSelection aNewSel = PIMPE->TransliterateText( pImpEditView->GetEditSelection(), nTransliterationMode );
    if ( aNewSel != aOldSel )
    {
        pImpEditView->DrawSelection();
        pImpEditView->SetEditSelection( aNewSel );
        pImpEditView->DrawSelection();
    }
}

void EditView::CompleteAutoCorrect( Window* pFrameWin )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !pImpEditView->HasSelection() && PIMPEE->GetStatus().DoAutoCorrect() )
    {
        pImpEditView->DrawSelection();
        EditSelection aSel = pImpEditView->GetEditSelection();
        aSel = PIMPE->EndOfWord( aSel.Max() );
        aSel = PIMPEE->AutoCorrect( aSel, 0, !IsInsertMode(), pFrameWin );
        pImpEditView->SetEditSelection( aSel );
        if ( PIMPE->IsModified() )
            PIMPE->FormatAndUpdate( this );
    }
}

EESpellState EditView::StartSpeller( sal_Bool bMultipleDoc )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !PIMPEE->GetSpeller().is() )
        return EE_SPELL_NOSPELLER;

    return PIMPEE->Spell( this, bMultipleDoc );
}

EESpellState EditView::StartThesaurus()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !PIMPEE->GetSpeller().is() )
        return EE_SPELL_NOSPELLER;

    return PIMPEE->StartThesaurus( this );
}


void EditView::StartTextConversion(
        LanguageType nSrcLang, LanguageType nDestLang, const Font *pDestFont,
        sal_Int32 nOptions, sal_Bool bIsInteractive, sal_Bool bMultipleDoc )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Convert( this, nSrcLang, nDestLang, pDestFont, nOptions, bIsInteractive, bMultipleDoc );
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
    EditPaM aPaM = pImpEditView->pEditEngine->GetPaM(aPos, false);
    return pImpEditView->IsWrongSpelledWord( aPaM , bMarkIfWrong );
}


static Image lcl_GetImageFromPngUrl( const OUString &rFileUrl )
{
    Image aRes;
    OUString aTmp;
    osl::FileBase::getSystemPathFromFileURL( rFileUrl, aTmp );
//    OString aPath = OString( aTmp.getStr(), aTmp.getLength(), osl_getThreadTextEncoding() );
#if defined(WNT)
//    aTmp = lcl_Win_GetShortPathName( aTmp );
#endif
    Graphic aGraphic;
    const String aFilterName(  IMP_PNG  );
    if( GRFILTER_OK == GraphicFilter::LoadGraphic( aTmp, aFilterName, aGraphic ) )
    {
        aRes = Image( aGraphic.GetBitmapEx() );
    }
    return aRes;
}


void EditView::ExecuteSpellPopup( const Point& rPosPixel, Link* pCallBack )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    Point aPos ( pImpEditView->GetWindow()->PixelToLogic( rPosPixel ) );
    aPos = pImpEditView->GetDocPos( aPos );
    EditPaM aPaM = pImpEditView->pEditEngine->GetPaM(aPos, false);
    Reference< linguistic2::XSpellChecker1 >  xSpeller( PIMPEE->GetSpeller() );
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
        rVal.Name = OUString( UPN_MAX_NUMBER_OF_SUGGESTIONS );
        rVal.Value <<= (sal_Int16) 7;
        //
        // Are there any replace suggestions?
        Reference< linguistic2::XSpellAlternatives >  xSpellAlt =
                xSpeller->spell( aSelected, PIMPEE->GetLanguage( aPaM2 ), aPropVals );

        Reference< linguistic2::XLanguageGuessing >  xLangGuesser( EE_DLL().GetGlobalData()->GetLanguageGuesser() );

        // check if text might belong to a different language...
        LanguageType nGuessLangWord = LANGUAGE_NONE;
        LanguageType nGuessLangPara = LANGUAGE_NONE;
        if (xSpellAlt.is() && xLangGuesser.is())
        {
            String aParaText;
            ContentNode *pNode = aPaM.GetNode();
            if (pNode)
            {
                aParaText = pNode->GetString();
            }
            else
            {
                OSL_FAIL( "content node is NULL" );
            }

            nGuessLangWord = CheckLanguage( xSpellAlt->getWord(), xSpeller, xLangGuesser, false );
            nGuessLangPara = CheckLanguage( aParaText, xSpeller, xLangGuesser, true );
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
            String aWordStr( EE_RESSTR( RID_STR_WORD ) );
            aWordStr.SearchAndReplace( String(  "%x"  ), aTmpWord );
            String aParaStr( EE_RESSTR( RID_STR_PARAGRAPH ) );
            aParaStr.SearchAndReplace( String(  "%x"  ), aTmpPara );
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
        sal_uInt16 nWords = (sal_uInt16) aAlt.getLength();
        if ( nWords )
        {
            for ( sal_uInt16 nW = 0; nW < nWords; nW++ )
            {
                String aAlternate( pAlt[nW] );
                aPopupMenu.InsertItem( MN_ALTSTART+nW, aAlternate, 0, OString(), nW );
                pAutoMenu->InsertItem( MN_AUTOSTART+nW, aAlternate, 0, OString(), nW );
            }
            aPopupMenu.InsertSeparator(OString(), nWords);
        }
        else
            aPopupMenu.RemoveItem( MN_AUTOCORR );   // delete?

        SvtLinguConfig aCfg;

        Reference< linguistic2::XSearchableDictionaryList >  xDicList( SvxGetDictionaryList() );
        Sequence< Reference< linguistic2::XDictionary >  > aDics;
        if (xDicList.is())
        {
            const Reference< linguistic2::XDictionary >  *pDic = NULL;
            // add the default positive dictionary to dic-list (if not already done).
            // This is to ensure that there is at least one dictionary to which
            // words could be added.
            uno::Reference< linguistic2::XDictionary >  xDic( SvxGetOrCreatePosDic( xDicList ) );
            if (xDic.is())
                xDic->setActive( sal_True );

            aDics = xDicList->getDictionaries();
            pDic  = aDics.getConstArray();
            sal_uInt16 nCheckedLanguage = PIMPEE->GetLanguage( aPaM2 );
            sal_uInt16 nDicCount = (sal_uInt16)aDics.getLength();
            for (sal_uInt16 i = 0; i < nDicCount; i++)
            {
                uno::Reference< linguistic2::XDictionary >  xDicTmp( pDic[i], uno::UNO_QUERY );
                if (!xDicTmp.is() || SvxGetIgnoreAllList() == xDicTmp)
                    continue;

                uno::Reference< frame::XStorable > xStor( xDicTmp, uno::UNO_QUERY );
                LanguageType nActLanguage = LanguageTag( xDicTmp->getLocale() ).getLanguageType();
                if( xDicTmp->isActive()
                    &&  xDicTmp->getDictionaryType() != linguistic2::DictionaryType_NEGATIVE
                    && (nCheckedLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage )
                    && (!xStor.is() || !xStor->isReadonly()) )
                {
                    // the extra 1 is because of the (possible) external
                    // linguistic entry above
                    sal_uInt16 nPos = MN_DICTSTART + i;
                    pInsertMenu->InsertItem( nPos, xDicTmp->getName() );
                    aDicNameSingle = xDicTmp->getName();

                    uno::Reference< lang::XServiceInfo > xSvcInfo( xDicTmp, uno::UNO_QUERY );
                    if (xSvcInfo.is())
                    {
                        OUString aDictionaryImageUrl( aCfg.GetSpellAndGrammarContextDictionaryImage(
                                xSvcInfo->getImplementationName()) );
                        if (!aDictionaryImageUrl.isEmpty() )
                        {
                            Image aImage( lcl_GetImageFromPngUrl( aDictionaryImageUrl ) );
                            pInsertMenu->SetItemImage( nPos, aImage );
                        }
                    }
                }
            }
        }
        if ( pInsertMenu->GetItemCount() != 1)
            aPopupMenu.EnableItem( MN_INSERT_SINGLE, sal_False );
        if ( pInsertMenu->GetItemCount() < 2 )
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
                aSel.nEndPos = EE_TEXTPOS_ALL;
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
        else if ( nId >= MN_DICTSTART || nId == MN_INSERT_SINGLE )
        {
            String aDicName;
            if (nId >= MN_DICTSTART)
                aDicName = pInsertMenu->GetItemText(nId);
            else
                aDicName = aDicNameSingle;

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
            SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
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
}

sal_Bool EditView::SelectCurrentWord( sal_Int16 nWordType )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    EditSelection aCurSel( pImpEditView->GetEditSelection() );
    pImpEditView->DrawSelection();
    aCurSel = PIMPE->SelectWord(aCurSel.Max(), nWordType);
    pImpEditView->SetEditSelection( aCurSel );
    pImpEditView->DrawSelection();
    ShowCursor( sal_True, sal_False );
    return aCurSel.HasRange() ? sal_True : sal_False;
}

void EditView::InsertField( const SvxFieldItem& rFld )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    EditEngine* pEE = PIMPE;
    pImpEditView->DrawSelection();
    pEE->UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM( pEE->InsertField( pImpEditView->GetEditSelection(), rFld ) );
    pEE->UndoActionEnd( EDITUNDO_INSERT );
    pImpEditView->SetEditSelection( EditSelection( aPaM, aPaM ) );
    pEE->UpdateFields();
    pEE->FormatAndUpdate( this );
}

const SvxFieldItem* EditView::GetFieldUnderMousePointer() const
{
    DBG_CHKTHIS( EditView, 0 );
    sal_Int32 nPara;
    sal_uInt16 nPos;
    return GetFieldUnderMousePointer( nPara, nPos );
}

const SvxFieldItem* EditView::GetField( const Point& rPos, sal_Int32* pPara, sal_uInt16* pPos ) const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->GetField( rPos, pPara, pPos );
}

const SvxFieldItem* EditView::GetFieldUnderMousePointer( sal_Int32& nPara, sal_uInt16& nPos ) const
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
    aSel.Adjust( pImpEditView->pEditEngine->GetEditDoc() );
    // Only when cursor is in font of field, no selection,
    // or only selecting field
    if ( ( aSel.Min().GetNode() == aSel.Max().GetNode() ) &&
         ( ( aSel.Max().GetIndex() == aSel.Min().GetIndex() ) ||
           ( aSel.Max().GetIndex() == aSel.Min().GetIndex()+1 ) ) )
    {
        EditPaM aPaM = aSel.Min();
        const CharAttribList::AttribsType& rAttrs = aPaM.GetNode()->GetCharAttribs().GetAttribs();
        sal_uInt16 nXPos = aPaM.GetIndex();
        for (size_t nAttr = rAttrs.size(); nAttr; )
        {
            const EditCharAttrib& rAttr = rAttrs[--nAttr];
            if (rAttr.GetStart() == nXPos)
                if (rAttr.Which() == EE_FEATURE_FIELD)
                {
                    DBG_ASSERT(rAttr.GetItem()->ISA( SvxFieldItem ), "No FeldItem...");
                    return static_cast<const SvxFieldItem*>(rAttr.GetItem());
                }
        }
    }
    return 0;
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
        for( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
        {
            std::vector<sal_uInt16> aPortions;
            rEditEngine.GetPortions( nPara, aPortions );

            if( aPortions.empty() )
                aPortions.push_back( rEditEngine.GetTextLen(nPara) );

            const sal_uInt16 nBeginPos = (nPara == aSel.nStartPara) ? aSel.nStartPos : 0;
            const sal_uInt16 nEndPos = (nPara == aSel.nEndPara) ? aSel.nEndPos : EE_TEXTPOS_ALL;

            for ( size_t nPos = 0; nPos < aPortions.size(); ++nPos )
            {
                sal_uInt16 nPortionEnd   = aPortions[ nPos ];
                sal_uInt16 nPortionStart = nPos > 0 ? aPortions[ nPos - 1 ] : 0;

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
        const sal_IntPtr* pAry = pFontList->GetSizeAry( aFontInfo );

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
    aSel.Adjust( PIMPE->GetEditDoc() );

    if( HasSelection() )
    {
        String aStr = PIMPE->GetSelected(aSel);

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
        return PIMPE->GetSelected(aSel);
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
        aSel.Adjust( PIMPE->GetEditDoc() );
        String aStr = PIMPE->GetSelected(aSel);

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
