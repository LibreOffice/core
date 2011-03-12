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

#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/style.hxx>
#include <vcl/mapmod.hxx>

#include <editeng/forbiddencharacterstable.hxx>

#define _OUTLINER_CXX

#include <editeng/outliner.hxx>
#include <paralist.hxx>
#include <editeng/outlobj.hxx>
#include <outleeng.hxx>
#include <outlundo.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>

DBG_NAMEEX(Outliner)

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

// ======================================================================
// ======================   Simple pass-through   =======================
// ======================================================================

void Outliner::SetUpdateMode( sal_Bool bUpdate )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetUpdateMode( bUpdate );
}


sal_Bool Outliner::GetUpdateMode() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetUpdateMode();
}

const SfxItemSet& Outliner::GetEmptyItemSet() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetEmptyItemSet();
}

void Outliner::EnableUndo( sal_Bool bEnable )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->EnableUndo( bEnable );
}

sal_Bool Outliner::IsUndoEnabled() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsUndoEnabled();
}

MapMode Outliner::GetRefMapMode() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetRefMapMode();
}

void Outliner::SetRefMapMode( const MapMode& rMMode )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetRefMapMode( rMMode );
}

void Outliner::SetBackgroundColor( const Color& rColor )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetBackgroundColor( rColor );
}

Color Outliner::GetBackgroundColor() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetBackgroundColor();
}


void Outliner::ClearModifyFlag()
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->ClearModifyFlag();
}

sal_Bool Outliner::IsModified() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsModified();
}

sal_uLong Outliner::GetTextHeight() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetTextHeight();
}

void Outliner::SetModifyHdl( const Link& rLink )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetModifyHdl( rLink );
}

Link Outliner::GetModifyHdl() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetModifyHdl();
}

void Outliner::SetNotifyHdl( const Link& rLink )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->aOutlinerNotifyHdl = rLink;

    if ( rLink.IsSet() )
        pEditEngine->SetNotifyHdl( LINK( this, Outliner, EditEngineNotifyHdl ) );
    else
        pEditEngine->SetNotifyHdl( Link() );

}

Link Outliner::GetNotifyHdl() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->aOutlinerNotifyHdl;
}

void Outliner::SetStatusEventHdl( const Link& rLink )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetStatusEventHdl( rLink );
}

Link Outliner::GetStatusEventHdl() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetStatusEventHdl();
}

void Outliner::SetDefTab( sal_uInt16 nTab )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetDefTab( nTab );
}

sal_uInt16 Outliner::GetDefTab() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetDefTab();
}

sal_Bool Outliner::IsFlatMode() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsFlatMode();
}

sal_Bool Outliner::UpdateFields()
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->UpdateFields();
}

void Outliner::RemoveFields( sal_Bool bKeepFieldText, TypeId aType )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->RemoveFields( bKeepFieldText, aType );
}

void Outliner::SetWordDelimiters( const String& rDelimiters )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetWordDelimiters( rDelimiters );
}

String Outliner::GetWordDelimiters() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetWordDelimiters();
}

String Outliner::GetWord( sal_uInt16 nPara, sal_uInt16 nIndex )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetWord( nPara, nIndex );
}

void Outliner::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->Draw( pOutDev, rOutRect );
}

void Outliner::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->Draw( pOutDev, rOutRect, rStartDocPos );
}


void Outliner::Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->Draw( pOutDev, rStartPos, nOrientation );
}

void Outliner::SetPaperSize( const Size& rSize )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetPaperSize( rSize );
}

const Size& Outliner::GetPaperSize() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetPaperSize();
}

void Outliner::SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    DBG_CHKTHIS( Outliner, 0 );
    pEditEngine->SetPolygon( rPolyPolygon );
}

void Outliner::SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon)
{
    DBG_CHKTHIS( Outliner, 0 );
    pEditEngine->SetPolygon( rPolyPolygon, pLinePolyPolygon);
}

void Outliner::ClearPolygon()
{
    DBG_CHKTHIS( Outliner, 0 );
    pEditEngine->ClearPolygon();
}

const PolyPolygon* Outliner::GetPolygon()
{
    DBG_CHKTHIS( Outliner, 0 );
    return pEditEngine->GetPolygon();
}

const Size& Outliner::GetMinAutoPaperSize() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetMinAutoPaperSize();
}

void Outliner::SetMinAutoPaperSize( const Size& rSz )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetMinAutoPaperSize( rSz );
}

const Size& Outliner::GetMaxAutoPaperSize() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetMaxAutoPaperSize();
}

void Outliner::SetMaxAutoPaperSize( const Size& rSz )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetMaxAutoPaperSize( rSz );
}

sal_Bool Outliner::IsExpanded( Paragraph* pPara ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->HasVisibleChilds( pPara );
}

Paragraph* Outliner::GetParent( Paragraph* pParagraph ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->GetParent( pParagraph );
}

sal_uLong Outliner::GetChildCount( Paragraph* pParent ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->GetChildCount( pParent );
}

Size Outliner::CalcTextSize()
{
    DBG_CHKTHIS(Outliner,0);
    return Size(pEditEngine->CalcTextWidth(),pEditEngine->GetTextHeight());
}

Point Outliner::GetDocPos( Paragraph* pPara )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetDocPosTopLeft( (sal_uInt16)pParaList->GetAbsPos( pPara ) );
}

void Outliner::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetStyleSheetPool( pSPool );
}

SfxStyleSheetPool* Outliner::GetStyleSheetPool()
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetStyleSheetPool();
}

SfxStyleSheet* Outliner::GetStyleSheet( sal_uLong nPara )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetStyleSheet( (sal_uInt16)nPara );
}

sal_Bool Outliner::IsInSelectionMode() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsInSelectionMode();
}

void Outliner::SetControlWord( sal_uLong nWord )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetControlWord( nWord );
}

sal_uLong Outliner::GetControlWord() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetControlWord();
}

void Outliner::SetAsianCompressionMode( sal_uInt16 n )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetAsianCompressionMode( n );
}

sal_uInt16 Outliner::GetAsianCompressionMode() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetAsianCompressionMode();
}

void Outliner::SetKernAsianPunctuation( sal_Bool b )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetKernAsianPunctuation( b );
}

sal_Bool Outliner::IsKernAsianPunctuation() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsKernAsianPunctuation();
}

void Outliner::SetAddExtLeading( sal_Bool bExtLeading )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetAddExtLeading( bExtLeading );
}

sal_Bool Outliner::IsAddExtLeading() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsAddExtLeading();
}

void Outliner::UndoActionStart( sal_uInt16 nId )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->UndoActionStart( nId );
}

void Outliner::UndoActionEnd( sal_uInt16 nId )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->UndoActionEnd( nId );
}

void Outliner::InsertUndo( EditUndo* pUndo )
{
#ifndef SVX_LIGHT
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->GetUndoManager().AddUndoAction( pUndo, sal_False );
#endif
}

sal_Bool Outliner::IsInUndo()
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsInUndo();
}

sal_uLong Outliner::GetLineCount( sal_uLong nParagraph ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetLineCount( (sal_uInt16)nParagraph );
}

sal_uInt16 Outliner::GetLineLen( sal_uLong nParagraph, sal_uInt16 nLine ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetLineLen( (sal_uInt16)nParagraph, nLine );
}

sal_uLong Outliner::GetLineHeight( sal_uLong nParagraph, sal_uLong nLine )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetLineHeight( (sal_uInt16)nParagraph, (sal_uInt16)nLine );
}

void Outliner::QuickRemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->QuickRemoveCharAttribs( nPara, nWhich );
}

EESpellState Outliner::HasSpellErrors()
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->HasSpellErrors();
}

sal_Bool Outliner::HasConvertibleTextPortion( LanguageType nLang )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->HasConvertibleTextPortion( nLang );
}

sal_Bool Outliner::ConvertNextDocument()
{
    DBG_CHKTHIS(Outliner,0);
    return sal_False;
}

void Outliner::SetDefaultLanguage( LanguageType eLang )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetDefaultLanguage( eLang );
}

LanguageType Outliner::GetDefaultLanguage() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetDefaultLanguage();
}

sal_Bool Outliner::HasOnlineSpellErrors() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->HasOnlineSpellErrors();
}

void Outliner::CompleteOnlineSpelling()
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->CompleteOnlineSpelling();
}

sal_Bool Outliner::HasText( const SvxSearchItem& rSearchItem )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->HasText( rSearchItem );
}

void Outliner::SetEditTextObjectPool( SfxItemPool* pPool )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetEditTextObjectPool( pPool );
}

SfxItemPool* Outliner::GetEditTextObjectPool() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetEditTextObjectPool();
}

sal_Bool Outliner::SpellNextDocument()
{
    DBG_CHKTHIS(Outliner,0);
    return sal_False;
}


void Outliner::SetSpeller( Reference< XSpellChecker1 > &xSpeller )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetSpeller( xSpeller );
}
Reference< XSpellChecker1 > Outliner::GetSpeller()
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetSpeller();
}

void Outliner::SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetForbiddenCharsTable( xForbiddenChars );
}

rtl::Reference<SvxForbiddenCharactersTable> Outliner::GetForbiddenCharsTable() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetForbiddenCharsTable();
}


Reference< XHyphenator > Outliner::GetHyphenator() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetHyphenator();
}

void Outliner::SetHyphenator( Reference< XHyphenator >& xHyph )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetHyphenator( xHyph );
}

OutputDevice* Outliner::GetRefDevice() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetRefDevice();
}

sal_uInt16 Outliner::GetFirstLineOffset( sal_uLong nParagraph )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetFirstLineOffset( (sal_uInt16)nParagraph );
}

sal_uLong Outliner::GetTextHeight( sal_uLong nParagraph ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetTextHeight((sal_uInt16)nParagraph );
}

Point Outliner::GetDocPos( const Point& rPaperPos ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetDocPos( rPaperPos );
}

Point Outliner::GetDocPosTopLeft( sal_uLong nParagraph )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetDocPosTopLeft( (sal_uInt16)nParagraph );
}

sal_Bool Outliner::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder )
{
    return IsTextPos( rPaperPos, nBorder, NULL );
}

sal_Bool Outliner::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder, sal_Bool* pbBullet )
{
    DBG_CHKTHIS(Outliner,0);
    if ( pbBullet)
        *pbBullet = sal_False;
    sal_Bool bTextPos = pEditEngine->IsTextPos( rPaperPos, nBorder );
    if ( !bTextPos )
    {
        Point aDocPos = GetDocPos( rPaperPos );
        sal_uInt16 nPara = pEditEngine->FindParagraph( aDocPos.Y() );
        if ( ( nPara != EE_PARA_NOT_FOUND ) && ImplHasBullet( nPara ) )
        {
            Rectangle aBulArea = ImpCalcBulletArea( nPara, sal_True, sal_True );
            if ( aBulArea.IsInside( rPaperPos ) )
            {
                bTextPos = sal_True;
                if ( pbBullet)
                    *pbBullet = sal_True;
            }
        }
    }

    return bTextPos;
}

void Outliner::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->QuickSetAttribs( rSet, rSel );
}

void Outliner::QuickInsertText( const XubString& rText, const ESelection& rSel )
{
    bFirstParaIsEmpty = sal_False;
    pEditEngine->QuickInsertText( rText, rSel );
}

void Outliner::QuickDelete( const ESelection& rSel )
{
    bFirstParaIsEmpty = sal_False;
    pEditEngine->QuickDelete( rSel );
}

void Outliner::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    DBG_CHKTHIS(Outliner,0);
    bFirstParaIsEmpty = sal_False;
    pEditEngine->QuickInsertField( rFld, rSel );
}

void Outliner::QuickInsertLineBreak( const ESelection& rSel )
{
    DBG_CHKTHIS(Outliner,0);
    bFirstParaIsEmpty = sal_False;
    pEditEngine->QuickInsertLineBreak( rSel );
}

void Outliner::QuickFormatDoc( sal_Bool bFull )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->QuickFormatDoc( bFull );
}

void Outliner::SetGlobalCharStretching( sal_uInt16 nX, sal_uInt16 nY )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetGlobalCharStretching( nX, nY );
}

void Outliner::GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->GetGlobalCharStretching( rX, rY );
}


void Outliner::DoStretchChars( sal_uInt16 nX, sal_uInt16 nY )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->DoStretchChars( nX, nY );
}

void Outliner::EraseVirtualDevice()
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->EraseVirtualDevice();
}

void Outliner::SetBigTextObjectStart( sal_uInt16 nStartAtPortionCount )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetBigTextObjectStart( nStartAtPortionCount );
}

sal_uInt16 Outliner::GetBigTextObjectStart() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetBigTextObjectStart();
}

sal_Bool Outliner::ShouldCreateBigTextObject() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->ShouldCreateBigTextObject();
}

void Outliner::SetVertical( sal_Bool b )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetVertical( b );
}

sal_Bool Outliner::IsVertical() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsVertical();
}

void Outliner::SetFixedCellHeight( sal_Bool bUseFixedCellHeight )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetFixedCellHeight( bUseFixedCellHeight );
}

sal_Bool Outliner::IsFixedCellHeight() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsFixedCellHeight();
}


void Outliner::SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetDefaultHorizontalTextDirection( eHTextDir );
}

EEHorizontalTextDirection Outliner::GetDefaultHorizontalTextDirection() const
{
    return pEditEngine->GetDefaultHorizontalTextDirection();
}

sal_uInt16 Outliner::GetScriptType( const ESelection& rSelection ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetScriptType( rSelection );
}

LanguageType Outliner::GetLanguage( sal_uInt16 nPara, sal_uInt16 nPos ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetLanguage( nPara, nPos );
}

void Outliner::RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->RemoveAttribs( rSelection, bRemoveParaAttribs, nWhich );
}

void Outliner::EnableAutoColor( sal_Bool b )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->EnableAutoColor( b );
}

sal_Bool Outliner::IsAutoColorEnabled() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsAutoColorEnabled();
}

void Outliner::ForceAutoColor( sal_Bool b )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->ForceAutoColor( b );
}

sal_Bool Outliner::IsForceAutoColor() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsForceAutoColor();
}

void Outliner::StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc)
{
    pEditEngine->StartSpelling(rEditView, bMultipleDoc);
}

void Outliner::EndSpelling()
{
    pEditEngine->EndSpelling();
}

bool Outliner::SpellSentence(EditView& rEditView, ::svx::SpellPortions& rToFill, bool bIsGrammarChecking )
{
    return pEditEngine->SpellSentence(rEditView, rToFill, bIsGrammarChecking );
}

void Outliner::PutSpellingToSentenceStart( EditView& rEditView )
{
    pEditEngine->PutSpellingToSentenceStart( rEditView );
}

void Outliner::ApplyChangedSentence(EditView& rEditView, const ::svx::SpellPortions& rNewPortions, bool bRecheck )
{
    pEditEngine->ApplyChangedSentence( rEditView, rNewPortions, bRecheck );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
