/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editview.cxx,v $
 *
 *  $Revision: 1.50 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:43:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <eeng_pch.hxx>

#define _SOLAR__PRIVATE 1

#include <i18npool/mslangid.hxx>
#include <svtools/languageoptions.hxx>

#ifndef SVX_LIGHT
#include <sfx2/srchitem.hxx>
#endif

#include <impedit.hxx>
#include <svx/editeng.hxx>
#include <svx/editview.hxx>
#include <svx/flditem.hxx>
#include <svx/svxacorr.hxx>
#include <svx/langitem.hxx>
#include <eerdll.hxx>
#include <eerdll2.hxx>
#include <editeng.hrc>
#include <dlgutil.hxx>
#include <helpid.hrc>
#include <i18npool/lang.h>
#include <vcl/menu.hxx>
#include <acorrcfg.hxx>

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif

#include <com/sun/star/frame/XStorable.hpp>

#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARY1_HPP_
#include <com/sun/star/linguistic2/XDictionary1.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HDL_
#include <com/sun/star/beans/PropertyValues.hdl>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif


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

        INT32   nCount = sizeof(aLangList) / sizeof(aLangList[0]);
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

    // Falls jemand gerade ein leeres Attribut hinterlassen hat,
    // und dann der Outliner die Selektion manipulitert:
    if ( !pImpEditView->GetEditSelection().HasRange() )
    {
        ContentNode* pNode = pImpEditView->GetEditSelection().Max().GetNode();
        PIMPEE->CursorMoved( pNode );
    }
    EditSelection aNewSelection( PIMPEE->ConvertSelection( rESel.nStartPara, rESel.nStartPos, rESel.nEndPara, rESel.nEndPos ) );

    // Wenn nach einem KeyInput die Selection manipuliert wird:
    PIMPEE->CheckIdleFormatter();

    // Selektion darf nicht bei einem unsichtbaren Absatz Starten/Enden:
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

    pImpEditView->DrawSelection();  // alte Selektion 'weg-zeichnen'
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

    // Rest nur hier, wenn API-Aufruf:
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
        DBG_ASSERT( !aPaM1.DbgIsBuggy( pImpEE->GetEditDoc() ), "Insert: PaM kaputt" );
        pImpEditView->SetEditSelection( EditSelection( aPaM1, aPaM2 ) );
    }
    else
        pImpEditView->SetEditSelection( EditSelection( aPaM2, aPaM2 ) );

    pImpEE->FormatAndUpdate( this );
}

sal_Bool EditView::PostKeyEvent( const KeyEvent& rKeyEvent )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->PostKeyEvent( rKeyEvent );
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

// Draw vertraegt die Assertion nicht, spaeter mal aktivieren
//  DBG_ASSERT( pImpEditView->pEditEngine->HasView( this ), "ShowCursor - View nicht angemeldet!" );
//  DBG_ASSERT( !GetWindow()->IsInPaint(), "ShowCursor - Why in Paint ?!" );

    if ( pImpEditView->pEditEngine->HasView( this ) )
    {
        // Das ControlWord hat mehr Gewicht:
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
    DBG_ASSERT( !pImpEditView->aEditSelection.IsInvalid(), "Blinde Selection in ...." );

    // Kein Undo-Kappseln noetig...
    pImpEditView->DrawSelection();
    PIMPEE->SetAttribs( pImpEditView->GetEditSelection(), rSet, ATTRSPECIAL_WHOLEWORD );
    PIMPEE->FormatAndUpdate( this );
}

void EditView::SetParaAttribs( const SfxItemSet& rSet, sal_uInt16 nPara )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    // Kein Undo-Kappseln noetig...
    PIMPEE->SetParaAttribs( nPara, rSet );
    // Beim Aendern von Absatzattributen muss immer formatiert werden...
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
    DBG_ASSERT( !pImpEditView->aEditSelection.IsInvalid(), "Blinde Selection in ...." );
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
    long nDest = aSel.nStartPara + nDiff;
    if ( nDiff > 0 )
        nDest++;
    DBG_ASSERT( ( nDest >= 0 ) && ( nDest <= pImpEditView->pEditEngine->GetParagraphCount() ), "MoveParagraphs - wrong Parameters!" );
    MoveParagraphs(
        Range( aSel.nStartPara, aSel.nEndPara ),
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

    aTextSel.Min() = aTextSel.Max();    // Selektion nicht behalten.
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

    aTextSel.Min() = aTextSel.Max();    // Selektion nicht behalten.
    pImpEditView->SetEditSelection( aTextSel );
    PIMPEE->FormatAndUpdate( this );
}

sal_Bool EditView::Drop( const DropEvent& )
{
    return FALSE;
}

ESelection EditView::GetDropPos()
{
    DBG_ERROR( "GetDropPos - Why?!" );
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
            return NULL;    // Nicht eindeutig.
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
        pImpEditView->DrawSelection();  // alte Selektion 'weg-zeichnen'
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

void EditView::CompleteAutoCorrect()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !pImpEditView->HasSelection() && PIMPEE->GetStatus().DoAutoCorrect() )
    {
        pImpEditView->DrawSelection();
        EditSelection aSel = pImpEditView->GetEditSelection();
        aSel = PIMPEE->EndOfWord( aSel.Max() );
        // MT 06/00: Why pass EditSelection to AutoCorrect, not EditPaM?!
        aSel = PIMPEE->AutoCorrect( aSel, 0, !IsInsertMode() );
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
        PopupMenu *pInsertMenu = aPopupMenu.GetPopupMenu( MN_INSERT );

        EditPaM aPaM2( aPaM );
        aPaM2.GetIndex()++;

        // Gibt es Replace-Vorschlaege?
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
        rVal.Name = OUString::createFromAscii( UPN_MAX_NUMBER_OF_SUGGESTIONS );
        rVal.Value <<= (INT16) 7;
        //
        // Gibt es Replace-Vorschlaege?
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
                aParaText = *pNode;
            else
                DBG_ERROR( "content node is NULL" );

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
            String aTmpWord( ::GetLanguageString( nGuessLangWord ) );
            String aTmpPara( ::GetLanguageString( nGuessLangPara ) );
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
            aPopupMenu.RemoveItem( MN_AUTOCORR );   // Loeschen?

        Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );

        Sequence< Reference< XDictionary >  > aDics;
        if (xDicList.is())
            aDics = xDicList->getDictionaries();
        const Reference< XDictionary >  *pDic = aDics.getConstArray();
        sal_uInt16 nLanguage = PIMPEE->GetLanguage( aPaM2 );
        sal_uInt16 nDicCount = (USHORT)aDics.getLength();
        for ( sal_uInt16 i = 0; i < nDicCount; i++ )
        {
            Reference< XDictionary1 >  xDic( pDic[i], UNO_QUERY );
            if (xDic.is())
            {
                sal_uInt16 nActLanguage = xDic->getLanguage();
                if( xDic->isActive() &&
                    xDic->getDictionaryType() == DictionaryType_POSITIVE &&
                    (nLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage ) )
                {
                    pInsertMenu->InsertItem( MN_DICTSTART + i, xDic->getName() );
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
                // Cursor vor das Wort setzen...
                EditPaM aCursor = pImpEditView->GetEditSelection().Min();
                pImpEditView->DrawSelection();  // alte Selektion 'weg-zeichnen'
                pImpEditView->SetEditSelection( EditSelection( aCursor, aCursor ) );
                pImpEditView->DrawSelection();
                // Stuerzt ab, wenn keine SfxApp
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
            Reference< XDictionary1 >  xDic( pDic[nId - MN_DICTSTART], UNO_QUERY );
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

sal_Bool EditView::SelectCurrentWord()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    EditSelection aCurSel( pImpEditView->GetEditSelection() );
    pImpEditView->DrawSelection();
    aCurSel = PIMPEE->SelectWord( aCurSel.Max() );
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
    // Nur wenn Cursor vor Feld, keine Selektion, oder nur Feld selektiert
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
                    DBG_ASSERT( pAttr->GetItem()->ISA( SvxFieldItem ), "Kein FeldItem..." );
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
        DBG_ASSERT( aTopLeftRec.Top() == aBottomRightRec.Top(), "Top() in einer Zeile unterschiedlich?" );
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


