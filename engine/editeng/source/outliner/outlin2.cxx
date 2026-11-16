/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editund2.hxx>

#include <svl/style.hxx>
#include <vcl/mapmod.hxx>

#include <editeng/forbiddencharacterstable.hxx>

#include <editeng/outliner.hxx>
#include <editeng/editstat.hxx>


using namespace ::cpo::uno;
using namespace ::com::sun::star::linguistic2;


// ======================   Simple pass-through   =======================


bool Outliner::SetUpdateLayout( bool bUpdate )
{
    return m_pEditEngine->SetUpdateLayout( bUpdate );
}


bool Outliner::IsUpdateLayout() const
{
    return m_pEditEngine->IsUpdateLayout();
}

const SfxItemSet& Outliner::GetEmptyItemSet() const
{
    return m_pEditEngine->GetEmptyItemSet();
}

void Outliner::EnableUndo( bool bEnable )
{
    m_pEditEngine->EnableUndo( bEnable );
}

bool Outliner::IsUndoEnabled() const
{
    return m_pEditEngine->IsUndoEnabled();
}

MapMode const & Outliner::GetRefMapMode() const
{
    return m_pEditEngine->GetRefMapMode();
}

void Outliner::SetRefMapMode( const MapMode& rMMode )
{
    m_pEditEngine->SetRefMapMode( rMMode );
}

void Outliner::SetBackgroundColor( const Color& rColor )
{
    m_pEditEngine->SetBackgroundColor( rColor );
}

Color const & Outliner::GetBackgroundColor() const
{
    return m_pEditEngine->GetBackgroundColor();
}


void Outliner::ClearModifyFlag()
{
    m_pEditEngine->ClearModifyFlag();
}

bool Outliner::IsModified() const
{
    return m_pEditEngine->IsModified();
}

sal_uInt32 Outliner::GetTextHeight() const
{
    return m_pEditEngine->GetTextHeight();
}

void Outliner::SetModifyHdl( const Link<LinkParamNone*,void>& rLink )
{
    m_pEditEngine->SetModifyHdl( rLink );
}

void Outliner::SetNotifyHdl( const Link<EENotify&,void>& rLink )
{
    m_aOutlinerNotifyHdl = rLink;

    if ( rLink.IsSet() )
        m_pEditEngine->SetNotifyHdl( LINK( this, Outliner, EditEngineNotifyHdl ) );
    else
        m_pEditEngine->SetNotifyHdl( Link<EENotify&,void>() );
}

void Outliner::SetStatusEventHdl( const Link<EditStatus&, void>& rLink )
{
    m_pEditEngine->SetStatusEventHdl( rLink );
}

Link<EditStatus&, void> const & Outliner::GetStatusEventHdl() const
{
    return m_pEditEngine->GetStatusEventHdl();
}

void Outliner::SetDefTab( sal_uInt16 nTab )
{
    m_pEditEngine->SetDefTab( nTab );
}

bool Outliner::IsFlatMode() const
{
    return m_pEditEngine->IsFlatMode();
}

bool Outliner::UpdateFields()
{
    return m_pEditEngine->UpdateFields();
}

void Outliner::RemoveFields( const std::function<bool ( const SvxFieldData* )>& isFieldData )
{
    m_pEditEngine->RemoveFields( isFieldData );
}

void Outliner::SetWordDelimiters( const OUString& rDelimiters )
{
    m_pEditEngine->SetWordDelimiters( rDelimiters );
}

OUString const & Outliner::GetWordDelimiters() const
{
    return m_pEditEngine->GetWordDelimiters();
}

OUString Outliner::GetWord(const EPaM& rPos)
{
    return m_pEditEngine->GetWord(rPos);
}

void Outliner::DrawText_ToRectangle( OutputDevice& rOutDev, const tools::Rectangle& rOutRect )
{
    m_pEditEngine->DrawText_ToRectangle( rOutDev, rOutRect, Point( 0, 0 ), true );
}

void Outliner::DrawText_ToPosition( OutputDevice& rOutDev, const Point& rStartPos )
{
    m_pEditEngine->DrawText_ToPosition( rOutDev, rStartPos );
}

void Outliner::SetPaperSize( const Size& rSize )
{
    m_pEditEngine->SetPaperSize( rSize );
}

const Size& Outliner::GetPaperSize() const
{
    return m_pEditEngine->GetPaperSize();
}

void Outliner::SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    m_pEditEngine->SetPolygon( rPolyPolygon );
}

void Outliner::SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon)
{
    m_pEditEngine->SetPolygon( rPolyPolygon, pLinePolyPolygon);
}

void Outliner::ClearPolygon()
{
    m_pEditEngine->ClearPolygon();
}

const Size& Outliner::GetMinAutoPaperSize() const
{
    return m_pEditEngine->GetMinAutoPaperSize();
}

void Outliner::SetMinAutoPaperSize( const Size& rSz )
{
    m_pEditEngine->SetMinAutoPaperSize( rSz );
}

const Size& Outliner::GetMaxAutoPaperSize() const
{
    return m_pEditEngine->GetMaxAutoPaperSize();
}

void Outliner::SetMaxAutoPaperSize( const Size& rSz )
{
    m_pEditEngine->SetMaxAutoPaperSize( rSz );
}

void Outliner::SetMinColumnWrapHeight(tools::Long nVal)
{
    m_pEditEngine->SetMinColumnWrapHeight(nVal);
}

bool Outliner::IsExpanded( sal_Int32 nPara ) const
{
    return HasVisibleChildren( nPara );
}

sal_Int32 Outliner::GetParent( sal_Int32 nPara ) const
{
    sal_Int32 nParaPrev = nPara-1;
    while ( nParaPrev >= 0 && ( m_pEditEngine->GetNumberingDepth(nParaPrev) >= m_pEditEngine->GetNumberingDepth(nPara) ) )
    {
        --nParaPrev;
    }

    return nParaPrev;
}

bool Outliner::HasHiddenChildren( sal_Int32 nPara ) const
{
    sal_Int32 nParaNext = nPara+1;
    if (nParaNext >= m_pEditEngine->GetParagraphCount())
        return false;
    return ( m_pEditEngine->GetNumberingDepth(nParaNext) > m_pEditEngine->GetNumberingDepth(nPara) )
        && !m_pEditEngine->IsBulletVisible(nParaNext);
}

bool Outliner::HasVisibleChildren( sal_Int32 nPara ) const
{
    sal_Int32 nParaNext = nPara+1;
    if (nParaNext >= m_pEditEngine->GetParagraphCount())
        return false;
    return ( m_pEditEngine->GetNumberingDepth(nParaNext) > m_pEditEngine->GetNumberingDepth(nPara) )
        && m_pEditEngine->IsBulletVisible(nParaNext);
}

sal_Int32 Outliner::GetChildCount( sal_Int32 nPara ) const
{
    sal_Int32 nChildCount = 0;
    sal_Int32 nParaNext = nPara+1;
    while ( nParaNext < m_pEditEngine->GetParagraphCount()
            && ( m_pEditEngine->GetNumberingDepth(nParaNext) > m_pEditEngine->GetNumberingDepth(nPara) ) )
    {
        nChildCount++;
        ++nParaNext;
    }
    return nChildCount;
}

Size Outliner::CalcTextSize()
{
    return Size(m_pEditEngine->CalcTextWidth(),m_pEditEngine->GetTextHeight());
}

void Outliner::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    m_pEditEngine->SetStyleSheetPool( pSPool );
}

SfxStyleSheetPool* Outliner::GetStyleSheetPool()
{
    return m_pEditEngine->GetStyleSheetPool();
}

SfxStyleSheet* Outliner::GetStyleSheet( sal_Int32 nPara )
{
    return m_pEditEngine->GetStyleSheet( nPara );
}

bool Outliner::IsInSelectionMode() const
{
    return m_pEditEngine->IsInSelectionMode();
}

void Outliner::SetControlWord( EEControlBits nWord )
{
    m_pEditEngine->SetControlWord( nWord );
}

EEControlBits Outliner::GetControlWord() const
{
    return m_pEditEngine->GetControlWord();
}

void Outliner::SetAsianCompressionMode( CharCompressType n )
{
    m_pEditEngine->SetAsianCompressionMode( n );
}

void Outliner::SetKernAsianPunctuation( bool b )
{
    m_pEditEngine->SetKernAsianPunctuation( b );
}

void Outliner::SetAddExtLeading( bool bExtLeading )
{
    m_pEditEngine->SetAddExtLeading( bExtLeading );
}

void Outliner::UndoActionStart( sal_uInt16 nId )
{
    m_pEditEngine->UndoActionStart( nId );
}

void Outliner::UndoActionEnd()
{
    m_pEditEngine->UndoActionEnd();
}

void Outliner::InsertUndo( std::unique_ptr<EditUndo> pUndo )
{
    m_pEditEngine->GetUndoManager().AddUndoAction( std::move(pUndo) );
}

bool Outliner::IsInUndo() const
{
    return m_pEditEngine->IsInUndo();
}

sal_Int32 Outliner::GetLineCount( sal_Int32 nParagraph ) const
{
    return m_pEditEngine->GetLineCount( nParagraph );
}

sal_Int32 Outliner::GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const
{
    return m_pEditEngine->GetLineLen( nParagraph, nLine );
}

sal_uInt32 Outliner::GetLineHeight( sal_Int32 nParagraph, sal_Int32 nLine )
{
    return m_pEditEngine->GetLineHeight( nParagraph, nLine );
}

void Outliner::RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich )
{
    m_pEditEngine->RemoveCharAttribs( nPara, nWhich );
}

EESpellState Outliner::HasSpellErrors()
{
    return m_pEditEngine->HasSpellErrors();
}

bool Outliner::HasConvertibleTextPortion( LanguageType nLang )
{
    return m_pEditEngine->HasConvertibleTextPortion( nLang );
}

bool Outliner::ConvertNextDocument()
{
    if (maConvertNextDocumentHdl.IsSet())
        return maConvertNextDocumentHdl.Call(this);
    return false;
}

void Outliner::SetDefaultLanguage( LanguageType eLang )
{
    m_pEditEngine->SetDefaultLanguage( eLang );
}

void Outliner::CompleteOnlineSpelling()
{
    m_pEditEngine->CompleteOnlineSpelling();
}

bool Outliner::HasText( const SvxSearchItem& rSearchItem )
{
    return m_pEditEngine->HasText( rSearchItem );
}

void Outliner::SetEditEnginePool( SfxItemPool* pPool )
{
    m_pEditEngine->SetItemPool( pPool );
}

SfxItemPool* Outliner::GetEditEnginePool() const
{
    return m_pEditEngine->GetItemPool();
}

bool Outliner::SpellNextDocument()
{
    if (maSpellNextDocumentHdl.IsSet())
        return maSpellNextDocumentHdl.Call(this);
    return false;
}


void Outliner::SetSpeller( Reference< XSpellChecker > const &xSpeller )
{
    m_pEditEngine->SetSpeller( xSpeller );
}

Reference< XSpellChecker > const & Outliner::GetSpeller()
{
    return m_pEditEngine->GetSpeller();
}

void Outliner::SetForbiddenCharsTable(const std::shared_ptr<SvxForbiddenCharactersTable>& xForbiddenChars)
{
    EditEngine::SetForbiddenCharsTable(xForbiddenChars);
}

void Outliner::SetHyphenator( Reference< XHyphenator > const & xHyph )
{
    m_pEditEngine->SetHyphenator( xHyph );
}

OutputDevice* Outliner::GetRefDevice() const
{
    return m_pEditEngine->GetRefDevice();
}

tools::Rectangle Outliner::GetParaBounds( sal_Int32 nParagraph ) const
{
    return m_pEditEngine->GetParaBounds(nParagraph );
}

Point Outliner::GetDocPos( const Point& rPaperPos ) const
{
    return m_pEditEngine->GetDocPos( rPaperPos );
}

bool Outliner::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder )
{
    return IsTextPos( rPaperPos, nBorder, nullptr );
}

bool Outliner::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder, bool* pbBullet )
{
    if ( pbBullet)
        *pbBullet = false;
    bool bTextPos = m_pEditEngine->IsTextPos( rPaperPos, nBorder );
    if ( !bTextPos )
    {
        Point aDocPos = GetDocPos( rPaperPos );
        sal_Int32 nPara = m_pEditEngine->FindParagraph( aDocPos.Y() );
        if ((nPara != EE_PARA_MAX) && ImplHasNumberFormat(nPara))
        {
            tools::Rectangle aBulArea = ImpCalcBulletHitArea( nPara );
            if ( aBulArea.Contains( rPaperPos ) )
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
    m_pEditEngine->QuickSetAttribs( rSet, rSel );
}

void Outliner::QuickInsertText( const OUString& rText, const ESelection& rSel )
{
    m_bFirstParaIsEmpty = false;
    m_pEditEngine->QuickInsertText( rText, rSel );
}

void Outliner::QuickDelete( const ESelection& rSel )
{
    m_bFirstParaIsEmpty = false;
    m_pEditEngine->QuickDelete( rSel );
}

void Outliner::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    m_bFirstParaIsEmpty = false;
    m_pEditEngine->QuickInsertField( rFld, rSel );
}

ESelection Outliner::QuickInsertUrlField( const SvxURLField& rField, const ESelection& rSel )
{
    m_bFirstParaIsEmpty = false;
    return m_pEditEngine->QuickInsertUrlField( rField, rSel );
}

void Outliner::QuickInsertLineBreak( const ESelection& rSel )
{
    m_bFirstParaIsEmpty = false;
    m_pEditEngine->QuickInsertLineBreak( rSel );
}

void Outliner::QuickFormatDoc()
{
    m_pEditEngine->QuickFormatDoc();
}

const ScalingParameters & Outliner::getScalingParameters() const
{
    return m_pEditEngine->getScalingParameters();
}

void Outliner::setScalingParameters(ScalingParameters const& rScalingParameters)
{
    m_pEditEngine->setScalingParameters(rScalingParameters);
}

void Outliner::setRoundFontSizeToPt(bool bRound) const
{
    m_pEditEngine->setRoundFontSizeToPt(bRound);
}

void Outliner::setLineSpacingBelowBaseline(bool bBelow) const
{
    m_pEditEngine->setLineSpacingBelowBaseline(bBelow);
}

void Outliner::setTextFitting(EETextFitting eFitting) const
{
    m_pEditEngine->setTextFitting(eFitting);
}

void Outliner::setTextFittingHeight(tools::Long nHeight) const
{
    m_pEditEngine->setTextFittingHeight(nHeight);
}

void Outliner::EraseVirtualDevice()
{
    m_pEditEngine->EraseVirtualDevice();
}

bool Outliner::ShouldCreateBigTextObject() const
{
    return m_pEditEngine->ShouldCreateBigTextObject();
}

void Outliner::SetVertical(bool bVertical)
{
    m_pEditEngine->SetVertical(bVertical);
}

void Outliner::SetRotation(TextRotation nRotation)
{
    m_pEditEngine->SetRotation(nRotation);
}

bool Outliner::IsVertical() const
{
    return m_pEditEngine->IsEffectivelyVertical();
}

bool Outliner::IsTopToBottom() const
{
    return m_pEditEngine->IsTopToBottom();
}

void Outliner::SetTextColumns(sal_Int16 nColumns, sal_Int32 nSpacing)
{
    m_pEditEngine->SetTextColumns(nColumns, nSpacing);
}

void Outliner::SetFixedCellHeight( bool bUseFixedCellHeight )
{
    m_pEditEngine->SetFixedCellHeight( bUseFixedCellHeight );
}

void Outliner::SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir )
{
    m_pEditEngine->SetDefaultHorizontalTextDirection( eHTextDir );
}

EEHorizontalTextDirection Outliner::GetDefaultHorizontalTextDirection() const
{
    return m_pEditEngine->GetDefaultHorizontalTextDirection();
}

LanguageType Outliner::GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const
{
    return m_pEditEngine->GetLanguage( nPara, nPos ).nLang;
}

void Outliner::RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    m_pEditEngine->RemoveAttribs( rSelection, bRemoveParaAttribs, nWhich );
}

void Outliner::EnableAutoColor( bool b )
{
    m_pEditEngine->EnableAutoColor( b );
}

void Outliner::ForceAutoColor( bool b )
{
    m_pEditEngine->ForceAutoColor( b );
}

bool Outliner::IsForceAutoColor() const
{
    return m_pEditEngine->IsForceAutoColor();
}

bool Outliner::SpellSentence(EditView const & rEditView, svx::SpellPortions& rToFill )
{
    return m_pEditEngine->SpellSentence(rEditView, rToFill );
}

void Outliner::PutSpellingToSentenceStart( EditView const & rEditView )
{
    m_pEditEngine->PutSpellingToSentenceStart( rEditView );
}

void Outliner::ApplyChangedSentence(EditView const & rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck )
{
    m_pEditEngine->ApplyChangedSentence( rEditView, rNewPortions, bRecheck );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
