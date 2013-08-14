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

void Outliner::SetWordDelimiters( const OUString& rDelimiters )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetWordDelimiters( rDelimiters );
}

OUString Outliner::GetWordDelimiters() const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetWordDelimiters();
}

String Outliner::GetWord( sal_Int32 nPara, sal_uInt16 nIndex )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetWord( nPara, nIndex );
}

void Outliner::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->Draw( pOutDev, rOutRect );
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
    return pParaList->HasVisibleChildren( pPara );
}

Paragraph* Outliner::GetParent( Paragraph* pParagraph ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->GetParent( pParagraph );
}

sal_Int32 Outliner::GetChildCount( Paragraph* pParent ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->GetChildCount( pParent );
}

Size Outliner::CalcTextSize()
{
    DBG_CHKTHIS(Outliner,0);
    return Size(pEditEngine->CalcTextWidth(),pEditEngine->GetTextHeight());
}

Size Outliner::CalcTextSizeNTP()
{
    DBG_CHKTHIS(Outliner,0);
    return Size(pEditEngine->CalcTextWidth(),pEditEngine->GetTextHeightNTP());
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

SfxStyleSheet* Outliner::GetStyleSheet( sal_Int32 nPara )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetStyleSheet( nPara );
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

void Outliner::SetKernAsianPunctuation( sal_Bool b )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetKernAsianPunctuation( b );
}

void Outliner::SetAddExtLeading( sal_Bool bExtLeading )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetAddExtLeading( bExtLeading );
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
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->GetUndoManager().AddUndoAction( pUndo, sal_False );
}

sal_Bool Outliner::IsInUndo()
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->IsInUndo();
}

sal_uLong Outliner::GetLineCount( sal_Int32 nParagraph ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetLineCount( nParagraph );
}

sal_uInt16 Outliner::GetLineLen( sal_Int32 nParagraph, sal_uInt16 nLine ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetLineLen( nParagraph, nLine );
}

sal_uLong Outliner::GetLineHeight( sal_Int32 nParagraph, sal_uInt16 nLine )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetLineHeight( nParagraph, nLine );
}

void Outliner::QuickRemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich )
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

sal_uLong Outliner::GetTextHeight( sal_Int32 nParagraph ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetTextHeight(nParagraph );
}

Point Outliner::GetDocPos( const Point& rPaperPos ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetDocPos( rPaperPos );
}

Point Outliner::GetDocPosTopLeft( sal_Int32 nParagraph )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetDocPosTopLeft( nParagraph );
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
        sal_Int32 nPara = pEditEngine->FindParagraph( aDocPos.Y() );
        if ( ( nPara != EE_PARA_NOT_FOUND ) && ImplHasNumberFormat( nPara ) )
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

void Outliner::QuickInsertText( const OUString& rText, const ESelection& rSel )
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

    // reset bullet size
    sal_Int32 nParagraphs = pParaList->GetParagraphCount();
    for ( sal_Int32 nPara = 0; nPara < nParagraphs; nPara++ )
    {
        Paragraph* pPara = pParaList->GetParagraph( nPara );
        if ( pPara )
            pPara->aBulSize.Width() = -1;
    }

    pEditEngine->SetGlobalCharStretching( nX, nY );
}

void Outliner::GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY ) const
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->GetGlobalCharStretching( rX, rY );
}

void Outliner::EraseVirtualDevice()
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->EraseVirtualDevice();
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

LanguageType Outliner::GetLanguage( sal_Int32 nPara, sal_uInt16 nPos ) const
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
