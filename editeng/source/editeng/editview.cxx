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
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


// static
LanguageType EditView::CheckLanguage(
        const OUString &rText,
        const Reference< linguistic2::XSpellChecker1 >& xSpell,
        const Reference< linguistic2::XLanguageGuessing >& xLangGuess,
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
        if (nLang == LANGUAGE_NONE) // language not found by looking up the system language...
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


        // build list of languages to check

        LanguageType aLangList[4];
        const AllSettings& rSettings  = Application::GetSettings();
        SvtLinguOptions aLinguOpt;
        SvtLinguConfig().GetOptions( aLinguOpt );
        // The default document language from "Tools/Options - Language Settings - Languages: Western"
        aLangList[0] = MsLangId::resolveSystemLanguageByScriptType( aLinguOpt.nDefaultLanguage,
                css::i18n::ScriptType::LATIN);
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

// class EditView

EditView::EditView( EditEngine* pEng, vcl::Window* pWindow )
{
    pImpEditView = new ImpEditView( this, pEng, pWindow );
}

EditView::~EditView()
{
    delete pImpEditView;
}

ImpEditEngine* EditView::GetImpEditEngine() const
{
    return pImpEditView->pEditEngine->pImpEditEngine;
}

EditEngine* EditView::GetEditEngine() const
{
    return pImpEditView->pEditEngine;
}

void EditView::Invalidate()
{
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

void EditView::SetReadOnly( bool bReadOnly )
{
    pImpEditView->bReadOnly = bReadOnly;
}

bool EditView::IsReadOnly() const
{
    return pImpEditView->bReadOnly;
}

void EditView::SetSelection( const ESelection& rESel )
{
    // If someone has just left an empty attribute, and then the outliner
    // manipulates the selection:
    if ( !pImpEditView->GetEditSelection().HasRange() )
    {
        ContentNode* pNode = pImpEditView->GetEditSelection().Max().GetNode();
        pImpEditView->pEditEngine->CursorMoved( pNode );
    }
    EditSelection aNewSelection( pImpEditView->pEditEngine->pImpEditEngine->ConvertSelection(
                                            rESel.nStartPara, rESel.nStartPos, rESel.nEndPara, rESel.nEndPos ) );

    // If the selection is manipulated after a KeyInput:
    pImpEditView->pEditEngine->CheckIdleFormatter();

    // Selection may not start/end at an invisible paragraph:
    const ParaPortion* pPortion = pImpEditView->pEditEngine->FindParaPortion( aNewSelection.Min().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = pImpEditView->pEditEngine->GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : pImpEditView->pEditEngine->GetEditDoc().GetObject( 0 );
        aNewSelection.Min() = EditPaM( pNode, pNode->Len() );
    }
    pPortion = pImpEditView->pEditEngine->FindParaPortion( aNewSelection.Max().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = pImpEditView->pEditEngine->GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : pImpEditView->pEditEngine->GetEditDoc().GetObject( 0 );
        aNewSelection.Max() = EditPaM( pNode, pNode->Len() );
    }

    pImpEditView->DrawSelection();
    pImpEditView->SetEditSelection( aNewSelection );
    pImpEditView->DrawSelection();
    bool bGotoCursor = pImpEditView->DoAutoScroll();
    ShowCursor( bGotoCursor );
}

ESelection EditView::GetSelection() const
{
    ESelection aSelection;

    aSelection.nStartPara = pImpEditView->pEditEngine->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Min().GetNode() );
    aSelection.nEndPara = pImpEditView->pEditEngine->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Max().GetNode() );

    aSelection.nStartPos = pImpEditView->GetEditSelection().Min().GetIndex();
    aSelection.nEndPos = pImpEditView->GetEditSelection().Max().GetIndex();

    return aSelection;
}

bool EditView::HasSelection() const
{
    return pImpEditView->HasSelection();
}

void EditView::DeleteSelected()
{
    pImpEditView->DeleteSelected();
}

SvtScriptType EditView::GetSelectedScriptType() const
{
    return pImpEditView->pEditEngine->GetScriptType( pImpEditView->GetEditSelection() );
}

void EditView::GetSelectionRectangles(std::vector<Rectangle>& rLogicRects) const
{
    return pImpEditView->GetSelectionRectangles(rLogicRects);
}

void EditView::Paint( const Rectangle& rRect, OutputDevice* pTargetDevice )
{
    pImpEditView->pEditEngine->pImpEditEngine->Paint( pImpEditView, rRect, pTargetDevice );
}

void EditView::SetEditEngine( EditEngine* pEditEng )
{
    pImpEditView->pEditEngine = pEditEng;
    EditSelection aStartSel;
    aStartSel = pImpEditView->pEditEngine->GetEditDoc().GetStartPaM();
    pImpEditView->SetEditSelection( aStartSel );
}

void EditView::SetWindow( vcl::Window* pWin )
{
    pImpEditView->pOutWin = pWin;
    pImpEditView->pEditEngine->pImpEditEngine->GetSelEngine().Reset();
}

vcl::Window* EditView::GetWindow() const
{
    return pImpEditView->pOutWin;
}

void EditView::SetVisArea( const Rectangle& rRect )
{
    pImpEditView->SetVisDocStartPos( rRect.TopLeft() );
}

const Rectangle& EditView::GetVisArea() const
{
    // Change return value to Rectangle in next incompatible build !!!
    static Rectangle aRect;
    aRect = pImpEditView->GetVisDocArea();
    return aRect;
}

void EditView::SetOutputArea( const Rectangle& rRect )
{
    pImpEditView->SetOutputArea( rRect );

    // the rest here only if it is an API call:
    pImpEditView->CalcAnchorPoint();
    if ( pImpEditView->pEditEngine->pImpEditEngine->GetStatus().AutoPageSize() )
        pImpEditView->RecalcOutputArea();
    pImpEditView->ShowCursor( false, false );
}

const Rectangle& EditView::GetOutputArea() const
{
    return pImpEditView->GetOutputArea();
}

const Pointer& EditView::GetPointer() const
{
    return pImpEditView->GetPointer();
}

vcl::Cursor* EditView::GetCursor() const
{
    return pImpEditView->pCursor;
}

void EditView::InsertText( const OUString& rStr, bool bSelect )
{

    EditEngine* pEE = pImpEditView->pEditEngine;
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

bool EditView::PostKeyEvent( const KeyEvent& rKeyEvent, vcl::Window* pFrameWin )
{
    return pImpEditView->PostKeyEvent( rKeyEvent, pFrameWin );
}

bool EditView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    return pImpEditView->MouseButtonUp( rMouseEvent );
}

void EditView::ReleaseMouse()
{
    return pImpEditView->ReleaseMouse();
}

bool EditView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    return pImpEditView->MouseButtonDown( rMouseEvent );
}

bool EditView::MouseMove( const MouseEvent& rMouseEvent )
{
    return pImpEditView->MouseMove( rMouseEvent );
}

void EditView::Command( const CommandEvent& rCEvt )
{
    pImpEditView->Command( rCEvt );
}

void EditView::ShowCursor( bool bGotoCursor, bool bForceVisCursor )
{
    if ( pImpEditView->pEditEngine->HasView( this ) )
    {
        // The control word is more important:
        if ( !pImpEditView->DoAutoScroll() )
            bGotoCursor = false;
        pImpEditView->ShowCursor( bGotoCursor, bForceVisCursor );

        if (comphelper::LibreOfficeKit::isActive())
        {
            if (comphelper::LibreOfficeKit::isViewCallback())
                pImpEditView->libreOfficeKitViewCallback(LOK_CALLBACK_CURSOR_VISIBLE, OString::boolean(true).getStr());
            else
                pImpEditView->libreOfficeKitCallback(LOK_CALLBACK_CURSOR_VISIBLE, OString::boolean(true).getStr());
        }
    }
}

void EditView::HideCursor()
{
    pImpEditView->GetCursor()->Hide();

    if (comphelper::LibreOfficeKit::isActive())
    {
        if (comphelper::LibreOfficeKit::isViewCallback())
            pImpEditView->libreOfficeKitViewCallback(LOK_CALLBACK_CURSOR_VISIBLE, OString::boolean(false).getStr());
        else
            pImpEditView->libreOfficeKitCallback(LOK_CALLBACK_CURSOR_VISIBLE, OString::boolean(false).getStr());
    }
}

Pair EditView::Scroll( long ndX, long ndY, ScrollRangeCheck nRangeCheck )
{
    return pImpEditView->Scroll( ndX, ndY, nRangeCheck );
}

const SfxItemSet& EditView::GetEmptyItemSet()
{
    return pImpEditView->pEditEngine->GetEmptyItemSet();
}

void EditView::SetAttribs( const SfxItemSet& rSet )
{
    DBG_ASSERT( !pImpEditView->aEditSelection.IsInvalid(), "Blind Selection in ...." );

    pImpEditView->DrawSelection();
    pImpEditView->pEditEngine->SetAttribs( pImpEditView->GetEditSelection(), rSet, ATTRSPECIAL_WHOLEWORD );
    pImpEditView->pEditEngine->FormatAndUpdate( this );
}

void EditView::RemoveAttribsKeepLanguages( bool bRemoveParaAttribs )
{

    pImpEditView->DrawSelection();
    pImpEditView->pEditEngine->UndoActionStart( EDITUNDO_RESETATTRIBS );
    EditSelection aSelection( pImpEditView->GetEditSelection() );

    for (sal_uInt16 nWID = EE_ITEMS_START; nWID <= EE_ITEMS_END; ++nWID)
    {
        bool bIsLang =  EE_CHAR_LANGUAGE     == nWID ||
                        EE_CHAR_LANGUAGE_CJK == nWID ||
                        EE_CHAR_LANGUAGE_CTL == nWID;
        if (!bIsLang)
            pImpEditView->pEditEngine->RemoveCharAttribs( aSelection, bRemoveParaAttribs, nWID );
    }

    pImpEditView->pEditEngine->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    pImpEditView->pEditEngine->FormatAndUpdate( this );
}

void EditView::RemoveAttribs( bool bRemoveParaAttribs, sal_uInt16 nWhich )
{

    pImpEditView->DrawSelection();
    pImpEditView->pEditEngine->UndoActionStart( EDITUNDO_RESETATTRIBS );
    pImpEditView->pEditEngine->RemoveCharAttribs( pImpEditView->GetEditSelection(), bRemoveParaAttribs, nWhich  );
    pImpEditView->pEditEngine->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    pImpEditView->pEditEngine->FormatAndUpdate( this );
}

void EditView::RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich )
{
    pImpEditView->pEditEngine->UndoActionStart( EDITUNDO_RESETATTRIBS );
    pImpEditView->pEditEngine->RemoveCharAttribs( nPara, nWhich );
    pImpEditView->pEditEngine->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    pImpEditView->pEditEngine->FormatAndUpdate( this );
}

SfxItemSet EditView::GetAttribs()
{
    DBG_ASSERT( !pImpEditView->aEditSelection.IsInvalid(), "Blind Selection in ...." );
    return pImpEditView->pEditEngine->pImpEditEngine->GetAttribs( pImpEditView->GetEditSelection() );
}

void EditView::Undo()
{
    pImpEditView->pEditEngine->Undo( this );
}

void EditView::Redo()
{
    pImpEditView->pEditEngine->Redo( this );
}

sal_uInt32 EditView::Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, bool bSelect, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    EditSelection aOldSel( pImpEditView->GetEditSelection() );
    pImpEditView->DrawSelection();
    pImpEditView->pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_READ );
    EditPaM aEndPaM = pImpEditView->pEditEngine->pImpEditEngine->Read( rInput, rBaseURL, eFormat, aOldSel, pHTTPHeaderAttrs );
    pImpEditView->pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_READ );
    EditSelection aNewSel( aEndPaM, aEndPaM );
    if ( bSelect )
    {
        aOldSel.Adjust( pImpEditView->pEditEngine->GetEditDoc() );
        aNewSel.Min() = aOldSel.Min();
    }

    pImpEditView->SetEditSelection( aNewSel );
    bool bGotoCursor = pImpEditView->DoAutoScroll();
    ShowCursor( bGotoCursor );

    return rInput.GetError();
}

void EditView::Cut()
{
    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
    pImpEditView->CutCopy( aClipBoard, true );
}

css::uno::Reference< css::datatransfer::XTransferable > EditView::GetTransferable()
{
    uno::Reference< datatransfer::XTransferable > xData =
        GetEditEngine()->CreateTransferable( pImpEditView->GetEditSelection() );
    return xData;
}

void EditView::Copy()
{
    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
    pImpEditView->CutCopy( aClipBoard, false );
}

void EditView::Paste()
{
    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
    pImpEditView->Paste( aClipBoard );
}

void EditView::PasteSpecial()
{
    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
    pImpEditView->Paste(aClipBoard, true );
}

Point EditView::GetWindowPosTopLeft( sal_Int32 nParagraph )
{
    Point aDocPos( pImpEditView->pEditEngine->GetDocPosTopLeft( nParagraph ) );
    return pImpEditView->GetWindowPos( aDocPos );
}

void EditView::SetSelectionMode( EESelectionMode eMode )
{
    pImpEditView->SetSelectionMode( eMode );
}

OUString EditView::GetSelected()
{
    return pImpEditView->pEditEngine->pImpEditEngine->GetSelected( pImpEditView->GetEditSelection() );
}

void EditView::MoveParagraphs( Range aParagraphs, sal_Int32 nNewPos )
{
    pImpEditView->pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_MOVEPARAS );
    pImpEditView->pEditEngine->pImpEditEngine->MoveParagraphs( aParagraphs, nNewPos, this );
    pImpEditView->pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_MOVEPARAS );
}

void EditView::MoveParagraphs( long nDiff )
{
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
    pImpEditView->SetBackgroundColor( rColor );
}

Color EditView::GetBackgroundColor() const
{
    return pImpEditView->GetBackgroundColor();
}

void EditView::registerLibreOfficeKitCallback(OutlinerSearchable *pSearchable)
{
    pImpEditView->registerLibreOfficeKitCallback(pSearchable);
}

void EditView::registerLibreOfficeKitViewCallback(OutlinerViewCallable *pCallable)
{
    pImpEditView->registerLibreOfficeKitViewCallback(pCallable);
}

void EditView::SetControlWord( EVControlBits nWord )
{
    pImpEditView->nControl = nWord;
}

EVControlBits EditView::GetControlWord() const
{
    return pImpEditView->nControl;
}

EditTextObject* EditView::CreateTextObject()
{
    return pImpEditView->pEditEngine->pImpEditEngine->CreateTextObject( pImpEditView->GetEditSelection() );
}

void EditView::InsertText( const EditTextObject& rTextObject )
{
    pImpEditView->DrawSelection();

    pImpEditView->pEditEngine->UndoActionStart( EDITUNDO_INSERT );
    EditSelection aTextSel( pImpEditView->pEditEngine->InsertText( rTextObject, pImpEditView->GetEditSelection() ) );
    pImpEditView->pEditEngine->UndoActionEnd( EDITUNDO_INSERT );

    aTextSel.Min() = aTextSel.Max();    // Selection not retained.
    pImpEditView->SetEditSelection( aTextSel );
    pImpEditView->pEditEngine->FormatAndUpdate( this );
}

void EditView::InsertText( css::uno::Reference< css::datatransfer::XTransferable > xDataObj, const OUString& rBaseURL, bool bUseSpecial )
{
    pImpEditView->pEditEngine->UndoActionStart( EDITUNDO_INSERT );
    pImpEditView->DeleteSelected();
    EditSelection aTextSel =
        pImpEditView->pEditEngine->InsertText(xDataObj, rBaseURL, pImpEditView->GetEditSelection().Max(), bUseSpecial);
    pImpEditView->pEditEngine->UndoActionEnd( EDITUNDO_INSERT );

    aTextSel.Min() = aTextSel.Max();    // Selection not retained.
    pImpEditView->SetEditSelection( aTextSel );
    pImpEditView->pEditEngine->FormatAndUpdate( this );
}

void EditView::SetEditEngineUpdateMode( bool bUpdate )
{
    pImpEditView->pEditEngine->pImpEditEngine->SetUpdateMode( bUpdate, this );
}

void EditView::ForceUpdate()
{
    pImpEditView->pEditEngine->pImpEditEngine->SetUpdateMode( true, this, true );
}

SfxStyleSheet* EditView::GetStyleSheet()
{
    EditSelection aSel( pImpEditView->GetEditSelection() );
    aSel.Adjust( pImpEditView->pEditEngine->GetEditDoc() );
    sal_Int32 nStartPara = pImpEditView->pEditEngine->GetEditDoc().GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndPara = pImpEditView->pEditEngine->GetEditDoc().GetPos( aSel.Max().GetNode() );

    SfxStyleSheet* pStyle = nullptr;
    for ( sal_Int32 n = nStartPara; n <= nEndPara; n++ )
    {
        SfxStyleSheet* pTmpStyle = pImpEditView->pEditEngine->GetStyleSheet( n );
        if ( ( n != nStartPara ) && ( pStyle != pTmpStyle ) )
            return nullptr;    // Not unique.
        pStyle = pTmpStyle;
    }
    return pStyle;
}

const SfxStyleSheet* EditView::GetStyleSheet() const
{
    return const_cast< EditView* >( this )->GetStyleSheet();
}

bool EditView::IsInsertMode() const
{
    return pImpEditView->IsInsertMode();
}

void EditView::SetInsertMode( bool bInsert )
{
    pImpEditView->SetInsertMode( bInsert );
}

void EditView::SetAnchorMode( EVAnchorMode eMode )
{
    pImpEditView->SetAnchorMode( eMode );
}

EVAnchorMode EditView::GetAnchorMode() const
{
    return pImpEditView->GetAnchorMode();
}

void EditView::TransliterateText( sal_Int32 nTransliterationMode )
{
    EditSelection aOldSel( pImpEditView->GetEditSelection() );
    EditSelection aNewSel = pImpEditView->pEditEngine->TransliterateText( pImpEditView->GetEditSelection(), nTransliterationMode );
    if ( aNewSel != aOldSel )
    {
        pImpEditView->DrawSelection();
        pImpEditView->SetEditSelection( aNewSel );
        pImpEditView->DrawSelection();
    }
}

void EditView::CompleteAutoCorrect( vcl::Window* pFrameWin )
{
    if ( !pImpEditView->HasSelection() && pImpEditView->pEditEngine->pImpEditEngine->GetStatus().DoAutoCorrect() )
    {
        pImpEditView->DrawSelection();
        EditSelection aSel = pImpEditView->GetEditSelection();
        aSel = pImpEditView->pEditEngine->EndOfWord( aSel.Max() );
        aSel = pImpEditView->pEditEngine->pImpEditEngine->AutoCorrect( aSel, 0, !IsInsertMode(), pFrameWin );
        pImpEditView->SetEditSelection( aSel );
        if ( pImpEditView->pEditEngine->IsModified() )
            pImpEditView->pEditEngine->FormatAndUpdate( this );
    }
}

EESpellState EditView::StartSpeller( bool bMultipleDoc )
{
    if ( !pImpEditView->pEditEngine->pImpEditEngine->GetSpeller().is() )
        return EE_SPELL_NOSPELLER;

    return pImpEditView->pEditEngine->pImpEditEngine->Spell( this, bMultipleDoc );
}

EESpellState EditView::StartThesaurus()
{
    if ( !pImpEditView->pEditEngine->pImpEditEngine->GetSpeller().is() )
        return EE_SPELL_NOSPELLER;

    return pImpEditView->pEditEngine->pImpEditEngine->StartThesaurus( this );
}

void EditView::StartTextConversion(
        LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont,
        sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc )
{
    pImpEditView->pEditEngine->pImpEditEngine->Convert( this, nSrcLang, nDestLang, pDestFont, nOptions, bIsInteractive, bMultipleDoc );
}


sal_Int32 EditView::StartSearchAndReplace( const SvxSearchItem& rSearchItem )
{
    return pImpEditView->pEditEngine->pImpEditEngine->StartSearchAndReplace( this, rSearchItem );
}

bool EditView::IsCursorAtWrongSpelledWord( bool bMarkIfWrong )
{
    bool bIsWrong = false;
    if ( !HasSelection() )
    {
        EditPaM aPaM = pImpEditView->GetEditSelection().Max();
        bIsWrong = pImpEditView->IsWrongSpelledWord( aPaM, bMarkIfWrong );
    }
    return bIsWrong;
}

bool EditView::IsWrongSpelledWordAtPos( const Point& rPosPixel, bool bMarkIfWrong )
{
    Point aPos ( pImpEditView->GetWindow()->PixelToLogic( rPosPixel ) );
    aPos = pImpEditView->GetDocPos( aPos );
    EditPaM aPaM = pImpEditView->pEditEngine->GetPaM(aPos, false);
    return pImpEditView->IsWrongSpelledWord( aPaM , bMarkIfWrong );
}

void EditView::ExecuteSpellPopup( const Point& rPosPixel, Link<SpellCallbackInfo&,void>* pCallBack )
{
    Point aPos ( pImpEditView->GetWindow()->PixelToLogic( rPosPixel ) );
    aPos = pImpEditView->GetDocPos( aPos );
    EditPaM aPaM = pImpEditView->pEditEngine->GetPaM(aPos, false);
    Reference< linguistic2::XSpellChecker1 >  xSpeller( pImpEditView->pEditEngine->pImpEditEngine->GetSpeller() );
    ESelection aOldSel = GetSelection();
    if ( xSpeller.is() && pImpEditView->IsWrongSpelledWord( aPaM, true ) )
    {
        PopupMenu aPopupMenu( EditResId( RID_MENU_SPELL ) );
        PopupMenu *pAutoMenu = aPopupMenu.GetPopupMenu( MN_AUTOCORR );
        PopupMenu *pInsertMenu = aPopupMenu.GetPopupMenu( MN_INSERT );  // add word to user-dictionaries
        pInsertMenu->SetMenuFlags( MenuFlags::NoAutoMnemonics );         //! necessary to retrieve the correct dictionary names later

        EditPaM aPaM2( aPaM );
        aPaM2.SetIndex( aPaM2.GetIndex()+1 );

        // Are there any replace suggestions?
        OUString aSelected( GetSelected() );

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
        rVal.Name = UPN_MAX_NUMBER_OF_SUGGESTIONS;
        rVal.Value <<= (sal_Int16) 7;

        // Are there any replace suggestions?
        Reference< linguistic2::XSpellAlternatives >  xSpellAlt =
                xSpeller->spell( aSelected, pImpEditView->pEditEngine->pImpEditEngine->GetLanguage( aPaM2 ), aPropVals );

        Reference< linguistic2::XLanguageGuessing >  xLangGuesser( EE_DLL().GetGlobalData()->GetLanguageGuesser() );

        // check if text might belong to a different language...
        LanguageType nGuessLangWord = LANGUAGE_NONE;
        LanguageType nGuessLangPara = LANGUAGE_NONE;
        if (xSpellAlt.is() && xLangGuesser.is())
        {
            OUString aParaText;
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
            OUString aTmpWord( SvtLanguageTable::GetLanguageString( nGuessLangWord ) );
            OUString aTmpPara( SvtLanguageTable::GetLanguageString( nGuessLangPara ) );
            OUString aWordStr( EE_RESSTR( RID_STR_WORD ) );
            aWordStr = aWordStr.replaceFirst( "%x", aTmpWord );
            OUString aParaStr( EE_RESSTR( RID_STR_PARAGRAPH ) );
            aParaStr = aParaStr.replaceFirst( "%x", aTmpPara );
            aPopupMenu.InsertItem( MN_WORDLANGUAGE, aWordStr );
            aPopupMenu.SetHelpId( MN_WORDLANGUAGE, HID_EDITENG_SPELLER_WORDLANGUAGE );
            aPopupMenu.InsertItem( MN_PARALANGUAGE, aParaStr );
            aPopupMenu.SetHelpId( MN_PARALANGUAGE, HID_EDITENG_SPELLER_PARALANGUAGE );
        }

        // ## Create mnemonics here
        if ( Application::IsAutoMnemonicEnabled() )
        {
            aPopupMenu.CreateAutoMnemonics();
            aPopupMenu.SetMenuFlags( aPopupMenu.GetMenuFlags() | MenuFlags::NoAutoMnemonics );
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
                OUString aAlternate( pAlt[nW] );
                aPopupMenu.InsertItem( MN_ALTSTART+nW, aAlternate, MenuItemBits::NONE, OString(), nW );
                pAutoMenu->InsertItem( MN_AUTOSTART+nW, aAlternate, MenuItemBits::NONE, OString(), nW );
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
            const Reference< linguistic2::XDictionary >  *pDic = nullptr;
            // add the default positive dictionary to dic-list (if not already done).
            // This is to ensure that there is at least one dictionary to which
            // words could be added.
            uno::Reference< linguistic2::XDictionary >  xDic( SvxGetOrCreatePosDic( xDicList ) );
            if (xDic.is())
                xDic->setActive( true );

            aDics = xDicList->getDictionaries();
            pDic  = aDics.getConstArray();
            sal_uInt16 nCheckedLanguage = pImpEditView->pEditEngine->pImpEditEngine->GetLanguage( aPaM2 );
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
                            Image aImage( aDictionaryImageUrl );
                            pInsertMenu->SetItemImage( nPos, aImage );
                        }
                    }
                }
            }
        }
        if ( pInsertMenu->GetItemCount() != 1)
            aPopupMenu.EnableItem( MN_INSERT_SINGLE, false );
        if ( pInsertMenu->GetItemCount() < 2 )
            aPopupMenu.EnableItem( MN_INSERT, false );

        aPopupMenu.RemoveDisabledEntries( true, true );

        Rectangle aTempRect = pImpEditView->pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, GETCRSR_TXTONLY );
        Point aScreenPos = pImpEditView->GetWindowPos( aTempRect.TopLeft() );
        aScreenPos = pImpEditView->GetWindow()->OutputToScreenPixel( aScreenPos );
        aTempRect = pImpEditView->GetWindow()->LogicToPixel( Rectangle(aScreenPos, aTempRect.GetSize() ));

        sal_uInt16 nId = aPopupMenu.Execute( pImpEditView->GetWindow(), aTempRect, PopupMenuFlags::NoMouseUpClose );
        if ( nId == MN_IGNORE )
        {
            OUString aWord = pImpEditView->SpellIgnoreWord();
            if ( pCallBack )
            {
                SpellCallbackInfo aInf( SpellCallbackCommand::IGNOREWORD, aWord );
                pCallBack->Call( aInf );
            }
            SetSelection( aOldSel );
        }
        else if ( ( nId == MN_WORDLANGUAGE ) || ( nId == MN_PARALANGUAGE ) )
        {
            LanguageType nLangToUse = (nId == MN_WORDLANGUAGE) ? nGuessLangWord : nGuessLangPara;
            SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nLangToUse );

            SfxItemSet aAttrs = GetEditEngine()->GetEmptyItemSet();
            if (nScriptType == SvtScriptType::LATIN)
                aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE ) );
            if (nScriptType == SvtScriptType::COMPLEX)
                aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CTL ) );
            if (nScriptType == SvtScriptType::ASIAN)
                aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CJK ) );
            if ( nId == MN_PARALANGUAGE )
            {
                ESelection aSel = GetSelection();
                aSel.nStartPos = 0;
                aSel.nEndPos = EE_TEXTPOS_ALL;
                SetSelection( aSel );
            }
            SetAttribs( aAttrs );
            pImpEditView->pEditEngine->pImpEditEngine->StartOnlineSpellTimer();

            if ( pCallBack )
            {
                SpellCallbackInfo aInf( ( nId == MN_WORDLANGUAGE ) ? SpellCallbackCommand::WORDLANGUAGE : SpellCallbackCommand::PARALANGUAGE, nLangToUse );
                pCallBack->Call( aInf );
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
                pImpEditView->pEditEngine->pImpEditEngine->Spell( this, false );
            }
            else
            {
                SpellCallbackInfo aInf( SpellCallbackCommand::STARTSPELLDLG, OUString() );
                pCallBack->Call( aInf );
            }
        }
        else if ( nId == MN_AUTO_CORRECT_DLG && pCallBack)
        {
            SpellCallbackInfo aInf( SpellCallbackCommand::AUTOCORRECT_OPTIONS, OUString() );
            pCallBack->Call( aInf );
        }
        else if ( nId >= MN_DICTSTART || nId == MN_INSERT_SINGLE )
        {
            OUString aDicName;
            if (nId >= MN_DICTSTART)
                aDicName = pInsertMenu->GetItemText(nId);
            else
                aDicName = aDicNameSingle;

            uno::Reference< linguistic2::XDictionary >      xDic;
            if (xDicList.is())
                xDic = xDicList->getDictionaryByName( aDicName );

            if (xDic.is())
                xDic->add( aSelected, false, OUString() );
            // save modified user-dictionary if it is persistent
            Reference< frame::XStorable >  xSavDic( xDic, UNO_QUERY );
            if (xSavDic.is())
                xSavDic->store();

            aPaM.GetNode()->GetWrongList()->ResetInvalidRange(0, aPaM.GetNode()->Len());
            pImpEditView->pEditEngine->pImpEditEngine->StartOnlineSpellTimer();

            if ( pCallBack )
            {
                SpellCallbackInfo aInf( SpellCallbackCommand::ADDTODICTIONARY, aSelected );
                pCallBack->Call( aInf );
            }
            SetSelection( aOldSel );
        }
        else if ( nId >= MN_AUTOSTART )
        {
            DBG_ASSERT(nId - MN_AUTOSTART < aAlt.getLength(), "index out of range");
            OUString aWord = pAlt[nId - MN_AUTOSTART];
            SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
            if ( pAutoCorrect )
                pAutoCorrect->PutText( aSelected, aWord, pImpEditView->pEditEngine->pImpEditEngine->GetLanguage( aPaM2 ) );
            InsertText( aWord );
        }
        else if ( nId >= MN_ALTSTART )  // Replace
        {
            DBG_ASSERT(nId - MN_ALTSTART < aAlt.getLength(), "index out of range");
            OUString aWord = pAlt[nId - MN_ALTSTART];
            InsertText( aWord );
        }
        else
        {
            SetSelection( aOldSel );
        }
    }
}

void EditView::SelectCurrentWord( sal_Int16 nWordType )
{
    EditSelection aCurSel( pImpEditView->GetEditSelection() );
    pImpEditView->DrawSelection();
    aCurSel = pImpEditView->pEditEngine->SelectWord(aCurSel.Max(), nWordType);
    pImpEditView->SetEditSelection( aCurSel );
    pImpEditView->DrawSelection();
    ShowCursor( true, false );
}

void EditView::InsertField( const SvxFieldItem& rFld )
{
    EditEngine* pEE = pImpEditView->pEditEngine;
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
    sal_Int32 nPara;
    sal_Int32 nPos;
    return GetFieldUnderMousePointer( nPara, nPos );
}

const SvxFieldItem* EditView::GetField( const Point& rPos, sal_Int32* pPara, sal_Int32* pPos ) const
{
    return pImpEditView->GetField( rPos, pPara, pPos );
}

const SvxFieldItem* EditView::GetFieldUnderMousePointer( sal_Int32& nPara, sal_Int32& nPos ) const
{
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
        const sal_Int32 nXPos = aPaM.GetIndex();
        for (size_t nAttr = rAttrs.size(); nAttr; )
        {
            const EditCharAttrib& rAttr = *rAttrs[--nAttr].get();
            if (rAttr.GetStart() == nXPos)
                if (rAttr.Which() == EE_FEATURE_FIELD)
                {
                    DBG_ASSERT(dynamic_cast<const SvxFieldItem* >(rAttr.GetItem() ) != nullptr, "No FeldItem...");
                    return static_cast<const SvxFieldItem*>(rAttr.GetItem());
                }
        }
    }
    return nullptr;
}

void EditView::SetInvalidateMore( sal_uInt16 nPixel )
{
    pImpEditView->SetInvalidateMore( nPixel );
}

sal_uInt16 EditView::GetInvalidateMore() const
{
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

    EditEngine& rEditEngine = *pImpEditView->pEditEngine;

    ESelection aSel( GetSelection() );
    ESelection aOldSelection( aSel );
    aSel.Adjust();

    if( !aSel.HasRange() )
    {
        aSel = rEditEngine.GetWord( aSel, css::i18n::WordType::DICTIONARY_WORD );
    }

    if( aSel.HasRange() )
    {
        for( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
        {
            std::vector<sal_Int32> aPortions;
            rEditEngine.GetPortions( nPara, aPortions );

            if( aPortions.empty() )
                aPortions.push_back( rEditEngine.GetTextLen(nPara) );

            const sal_Int32 nBeginPos = (nPara == aSel.nStartPara) ? aSel.nStartPos : 0;
            const sal_Int32 nEndPos = (nPara == aSel.nEndPara) ? aSel.nEndPos : EE_TEXTPOS_ALL;

            for ( size_t nPos = 0; nPos < aPortions.size(); ++nPos )
            {
                sal_Int32 nPortionEnd   = aPortions[ nPos ];
                sal_Int32 nPortionStart = nPos > 0 ? aPortions[ nPos - 1 ] : 0;

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

        vcl::FontInfo aFontInfo = pFontList->Get( pFontItem->GetFamilyName(), pFontItem->GetStyleName() );
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

OUString EditView::GetSurroundingText() const
{
    EditSelection aSel( pImpEditView->GetEditSelection() );
    aSel.Adjust( pImpEditView->pEditEngine->GetEditDoc() );

    if( HasSelection() )
    {
        OUString aStr = pImpEditView->pEditEngine->GetSelected(aSel);

        // Stop reconversion if the selected text includes a line break.
        if ( aStr.indexOf( 0x0A ) == -1 )
          return aStr;
        else
          return OUString();
    }
    else
    {
        aSel.Min().SetIndex( 0 );
        aSel.Max().SetIndex( aSel.Max().GetNode()->Len() );
        return pImpEditView->pEditEngine->GetSelected(aSel);
    }
}

Selection EditView::GetSurroundingTextSelection() const
{
    ESelection aSelection( GetSelection() );
    aSelection.Adjust();

    if( HasSelection() )
    {
        EditSelection aSel( pImpEditView->GetEditSelection() );
        aSel.Adjust( pImpEditView->pEditEngine->GetEditDoc() );
        OUString aStr = pImpEditView->pEditEngine->GetSelected(aSel);

        // Stop reconversion if the selected text includes a line break.
        if ( aStr.indexOf( 0x0A ) == -1 )
            return Selection( 0, aSelection.nEndPos - aSelection.nStartPos );
        else
            return Selection( 0, 0 );
    }
    else
    {
        return Selection( aSelection.nStartPos, aSelection.nEndPos );
    }
}

void EditView::SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark)
{
    Point aDocPos(pImpEditView->GetDocPos(rPosition));
    EditPaM aPaM = pImpEditView->pEditEngine->GetPaM(aDocPos);
    EditSelection aSelection(pImpEditView->GetEditSelection());

    // Explicitly create or delete the selection.
    if (bClearMark)
    {
        pImpEditView->DeselectAll();
        aSelection = pImpEditView->GetEditSelection();
    }
    else
        pImpEditView->CreateAnchor();

    if (bPoint)
        aSelection.Max() = aPaM;
    else
        aSelection.Min() = aPaM;

    if (pImpEditView->GetEditSelection().Min() != aSelection.Min())
        pImpEditView->pEditEngine->CursorMoved(pImpEditView->GetEditSelection().Min().GetNode());
    pImpEditView->DrawSelection(aSelection);
    if (pImpEditView->GetEditSelection() != aSelection)
        pImpEditView->SetEditSelection(aSelection);
    ShowCursor(/*bGotoCursor=*/false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
