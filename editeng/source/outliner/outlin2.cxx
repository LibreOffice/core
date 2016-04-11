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


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;


// ======================   Simple pass-through   =======================


void Outliner::SetUpdateMode( bool bUpdate )
{
    pEditEngine->SetUpdateMode( bUpdate );
}


bool Outliner::GetUpdateMode() const
{
    return pEditEngine->GetUpdateMode();
}

const SfxItemSet& Outliner::GetEmptyItemSet() const
{
    return pEditEngine->GetEmptyItemSet();
}

void Outliner::EnableUndo( bool bEnable )
{
    pEditEngine->EnableUndo( bEnable );
}

bool Outliner::IsUndoEnabled() const
{
    return pEditEngine->IsUndoEnabled();
}

MapMode Outliner::GetRefMapMode() const
{
    return pEditEngine->GetRefMapMode();
}

void Outliner::SetRefMapMode( const MapMode& rMMode )
{
    pEditEngine->SetRefMapMode( rMMode );
}

void Outliner::SetBackgroundColor( const Color& rColor )
{
    pEditEngine->SetBackgroundColor( rColor );
}

Color Outliner::GetBackgroundColor() const
{
    return pEditEngine->GetBackgroundColor();
}


void Outliner::ClearModifyFlag()
{
    pEditEngine->ClearModifyFlag();
}

bool Outliner::IsModified() const
{
    return pEditEngine->IsModified();
}

sal_uLong Outliner::GetTextHeight() const
{
    return pEditEngine->GetTextHeight();
}

void Outliner::SetModifyHdl( const Link<LinkParamNone*,void>& rLink )
{
    pEditEngine->SetModifyHdl( rLink );
}

Link<LinkParamNone*,void> Outliner::GetModifyHdl() const
{
    return pEditEngine->GetModifyHdl();
}

void Outliner::SetNotifyHdl( const Link<EENotify&,void>& rLink )
{
    pEditEngine->aOutlinerNotifyHdl = rLink;

    if ( rLink.IsSet() )
        pEditEngine->SetNotifyHdl( LINK( this, Outliner, EditEngineNotifyHdl ) );
    else
        pEditEngine->SetNotifyHdl( Link<EENotify&,void>() );
}

void Outliner::SetStatusEventHdl( const Link<EditStatus&, void>& rLink )
{
    pEditEngine->SetStatusEventHdl( rLink );
}

Link<EditStatus&, void> Outliner::GetStatusEventHdl() const
{
    return pEditEngine->GetStatusEventHdl();
}

void Outliner::SetDefTab( sal_uInt16 nTab )
{
    pEditEngine->SetDefTab( nTab );
}

bool Outliner::IsFlatMode() const
{
    return pEditEngine->IsFlatMode();
}

bool Outliner::UpdateFields()
{
    return pEditEngine->UpdateFields();
}

void Outliner::RemoveFields( const std::function<bool ( const SvxFieldData* )>& isFieldData )
{
    pEditEngine->RemoveFields( true/*bKeepFieldText*/, isFieldData );
}

void Outliner::SetWordDelimiters( const OUString& rDelimiters )
{
    pEditEngine->SetWordDelimiters( rDelimiters );
}

OUString Outliner::GetWordDelimiters() const
{
    return pEditEngine->GetWordDelimiters();
}

OUString Outliner::GetWord( sal_Int32 nPara, sal_Int32 nIndex )
{
    return pEditEngine->GetWord( nPara, nIndex );
}

void Outliner::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect )
{
    pEditEngine->Draw( pOutDev, rOutRect );
}

void Outliner::Draw( OutputDevice* pOutDev, const Point& rStartPos )
{
    pEditEngine->Draw( pOutDev, rStartPos );
}

void Outliner::SetPaperSize( const Size& rSize )
{
    pEditEngine->SetPaperSize( rSize );
}

const Size& Outliner::GetPaperSize() const
{
    return pEditEngine->GetPaperSize();
}

void Outliner::SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    pEditEngine->SetPolygon( rPolyPolygon );
}

void Outliner::SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon)
{
    pEditEngine->SetPolygon( rPolyPolygon, pLinePolyPolygon);
}

void Outliner::ClearPolygon()
{
    pEditEngine->ClearPolygon();
}

const Size& Outliner::GetMinAutoPaperSize() const
{
    return pEditEngine->GetMinAutoPaperSize();
}

void Outliner::SetMinAutoPaperSize( const Size& rSz )
{
    pEditEngine->SetMinAutoPaperSize( rSz );
}

const Size& Outliner::GetMaxAutoPaperSize() const
{
    return pEditEngine->GetMaxAutoPaperSize();
}

void Outliner::SetMaxAutoPaperSize( const Size& rSz )
{
    pEditEngine->SetMaxAutoPaperSize( rSz );
}

bool Outliner::IsExpanded( Paragraph* pPara ) const
{
    return pParaList->HasVisibleChildren( pPara );
}

Paragraph* Outliner::GetParent( Paragraph* pParagraph ) const
{
    return pParaList->GetParent( pParagraph );
}

sal_Int32 Outliner::GetChildCount( Paragraph* pParent ) const
{
    return pParaList->GetChildCount( pParent );
}

Size Outliner::CalcTextSize()
{
    return Size(pEditEngine->CalcTextWidth(),pEditEngine->GetTextHeight());
}

Size Outliner::CalcTextSizeNTP()
{
    return Size(pEditEngine->CalcTextWidth(),pEditEngine->GetTextHeightNTP());
}

void Outliner::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    pEditEngine->SetStyleSheetPool( pSPool );
}

SfxStyleSheetPool* Outliner::GetStyleSheetPool()
{
    return pEditEngine->GetStyleSheetPool();
}

SfxStyleSheet* Outliner::GetStyleSheet( sal_Int32 nPara )
{
    return pEditEngine->GetStyleSheet( nPara );
}

bool Outliner::IsInSelectionMode() const
{
    return pEditEngine->IsInSelectionMode();
}

void Outliner::SetControlWord( EEControlBits nWord )
{
    pEditEngine->SetControlWord( nWord );
}

EEControlBits Outliner::GetControlWord() const
{
    return pEditEngine->GetControlWord();
}

void Outliner::SetAsianCompressionMode( sal_uInt16 n )
{
    pEditEngine->SetAsianCompressionMode( n );
}

void Outliner::SetKernAsianPunctuation( bool b )
{
    pEditEngine->SetKernAsianPunctuation( b );
}

void Outliner::SetAddExtLeading( bool bExtLeading )
{
    pEditEngine->SetAddExtLeading( bExtLeading );
}

void Outliner::UndoActionStart( sal_uInt16 nId )
{
    pEditEngine->UndoActionStart( nId );
}

void Outliner::UndoActionEnd( sal_uInt16 nId )
{
    pEditEngine->UndoActionEnd( nId );
}

void Outliner::InsertUndo( EditUndo* pUndo )
{
    pEditEngine->GetUndoManager().AddUndoAction( pUndo );
}

bool Outliner::IsInUndo()
{
    return pEditEngine->IsInUndo();
}

sal_uLong Outliner::GetLineCount( sal_Int32 nParagraph ) const
{
    return pEditEngine->GetLineCount( nParagraph );
}

sal_Int32 Outliner::GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const
{
    return pEditEngine->GetLineLen( nParagraph, nLine );
}

sal_uLong Outliner::GetLineHeight( sal_Int32 nParagraph )
{
    return pEditEngine->GetLineHeight( nParagraph );
}

void Outliner::RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich )
{
    pEditEngine->RemoveCharAttribs( nPara, nWhich );
}

EESpellState Outliner::HasSpellErrors()
{
    return pEditEngine->HasSpellErrors();
}

bool Outliner::HasConvertibleTextPortion( LanguageType nLang )
{
    return pEditEngine->HasConvertibleTextPortion( nLang );
}

bool Outliner::ConvertNextDocument()
{
    return false;
}

void Outliner::SetDefaultLanguage( LanguageType eLang )
{
    pEditEngine->SetDefaultLanguage( eLang );
}

LanguageType Outliner::GetDefaultLanguage() const
{
    return pEditEngine->GetDefaultLanguage();
}

void Outliner::CompleteOnlineSpelling()
{
    pEditEngine->CompleteOnlineSpelling();
}

bool Outliner::HasText( const SvxSearchItem& rSearchItem )
{
    return pEditEngine->HasText( rSearchItem );
}

void Outliner::SetEditTextObjectPool( SfxItemPool* pPool )
{
    pEditEngine->SetEditTextObjectPool( pPool );
}

SfxItemPool* Outliner::GetEditTextObjectPool() const
{
    return pEditEngine->GetEditTextObjectPool();
}

bool Outliner::SpellNextDocument()
{
    return false;
}


void Outliner::SetSpeller( Reference< XSpellChecker1 > &xSpeller )
{
    pEditEngine->SetSpeller( xSpeller );
}
Reference< XSpellChecker1 > Outliner::GetSpeller()
{
    return pEditEngine->GetSpeller();
}

void Outliner::SetForbiddenCharsTable( const rtl::Reference<SvxForbiddenCharactersTable>& xForbiddenChars )
{
    EditEngine::SetForbiddenCharsTable( xForbiddenChars );
}

void Outliner::SetHyphenator( Reference< XHyphenator >& xHyph )
{
    pEditEngine->SetHyphenator( xHyph );
}

OutputDevice* Outliner::GetRefDevice() const
{
    return pEditEngine->GetRefDevice();
}

sal_uLong Outliner::GetTextHeight( sal_Int32 nParagraph ) const
{
    return pEditEngine->GetTextHeight(nParagraph );
}

Point Outliner::GetDocPos( const Point& rPaperPos ) const
{
    return pEditEngine->GetDocPos( rPaperPos );
}

Point Outliner::GetDocPosTopLeft( sal_Int32 nParagraph )
{
    return pEditEngine->GetDocPosTopLeft( nParagraph );
}

bool Outliner::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder )
{
    return IsTextPos( rPaperPos, nBorder, nullptr );
}

bool Outliner::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder, bool* pbBullet )
{
    if ( pbBullet)
        *pbBullet = false;
    bool bTextPos = pEditEngine->IsTextPos( rPaperPos, nBorder );
    if ( !bTextPos )
    {
        Point aDocPos = GetDocPos( rPaperPos );
        sal_Int32 nPara = pEditEngine->FindParagraph( aDocPos.Y() );
        if ( ( nPara != EE_PARA_NOT_FOUND ) && ImplHasNumberFormat( nPara ) )
        {
            Rectangle aBulArea = ImpCalcBulletArea( nPara, true, true );
            if ( aBulArea.IsInside( rPaperPos ) )
            {
                bTextPos = true;
                if ( pbBullet)
                    *pbBullet = true;
            }
        }
    }

    return bTextPos;
}

void Outliner::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    pEditEngine->QuickSetAttribs( rSet, rSel );
}

void Outliner::QuickInsertText( const OUString& rText, const ESelection& rSel )
{
    bFirstParaIsEmpty = false;
    pEditEngine->QuickInsertText( rText, rSel );
}

void Outliner::QuickDelete( const ESelection& rSel )
{
    bFirstParaIsEmpty = false;
    pEditEngine->QuickDelete( rSel );
}

void Outliner::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    bFirstParaIsEmpty = false;
    pEditEngine->QuickInsertField( rFld, rSel );
}

void Outliner::QuickInsertLineBreak( const ESelection& rSel )
{
    bFirstParaIsEmpty = false;
    pEditEngine->QuickInsertLineBreak( rSel );
}

void Outliner::QuickFormatDoc()
{
    pEditEngine->QuickFormatDoc();
}

void Outliner::SetGlobalCharStretching( sal_uInt16 nX, sal_uInt16 nY )
{

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
    pEditEngine->GetGlobalCharStretching( rX, rY );
}

void Outliner::EraseVirtualDevice()
{
    pEditEngine->EraseVirtualDevice();
}

bool Outliner::ShouldCreateBigTextObject() const
{
    return pEditEngine->ShouldCreateBigTextObject();
}

const EditEngine& Outliner::GetEditEngine() const
{
    return *pEditEngine;
}

void Outliner::SetVertical( bool b )
{
    pEditEngine->SetVertical( b );
}

bool Outliner::IsVertical() const
{
    return pEditEngine->IsVertical();
}

void Outliner::SetFixedCellHeight( bool bUseFixedCellHeight )
{
    pEditEngine->SetFixedCellHeight( bUseFixedCellHeight );
}

void Outliner::SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir )
{
    pEditEngine->SetDefaultHorizontalTextDirection( eHTextDir );
}

EEHorizontalTextDirection Outliner::GetDefaultHorizontalTextDirection() const
{
    return pEditEngine->GetDefaultHorizontalTextDirection();
}

SvtScriptType Outliner::GetScriptType( const ESelection& rSelection ) const
{
    return pEditEngine->GetScriptType( rSelection );
}

LanguageType Outliner::GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const
{
    return pEditEngine->GetLanguage( nPara, nPos );
}

void Outliner::RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    pEditEngine->RemoveAttribs( rSelection, bRemoveParaAttribs, nWhich );
}

void Outliner::EnableAutoColor( bool b )
{
    pEditEngine->EnableAutoColor( b );
}

void Outliner::ForceAutoColor( bool b )
{
    pEditEngine->ForceAutoColor( b );
}

bool Outliner::IsForceAutoColor() const
{
    return pEditEngine->IsForceAutoColor();
}

bool Outliner::SpellSentence(EditView& rEditView, svx::SpellPortions& rToFill, bool bIsGrammarChecking )
{
    return pEditEngine->SpellSentence(rEditView, rToFill, bIsGrammarChecking );
}

void Outliner::PutSpellingToSentenceStart( EditView& rEditView )
{
    pEditEngine->PutSpellingToSentenceStart( rEditView );
}

void Outliner::ApplyChangedSentence(EditView& rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck )
{
    pEditEngine->ApplyChangedSentence( rEditView, rNewPortions, bRecheck );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
