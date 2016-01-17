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


#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <impedit.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <editeng/langitem.hxx>
#include <editeng/fontitem.hxx>
#include <textconv.hxx>


using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::linguistic2;


TextConvWrapper::TextConvWrapper( vcl::Window* pWindow,
        const Reference< XComponentContext >& rxContext,
        const lang::Locale& rSourceLocale,
        const lang::Locale& rTargetLocale,
        const vcl::Font* pTargetFont,
        sal_Int32 nOptions,
        bool bIsInteractive,
        bool bIsStart,
        EditView* pView ) :
    HangulHanjaConversion( pWindow, rxContext, rSourceLocale, rTargetLocale, pTargetFont, nOptions, bIsInteractive )
    , m_nConvTextLang(LANGUAGE_NONE)
    , m_nUnitOffset(0)
    , m_nLastPos(0)
    , m_aConvSel(pView->GetSelection())
    , m_pEditView(pView)
    , m_pWin(pWindow)
    , m_bStartChk(false)
    , m_bStartDone(bIsStart)
    , m_bEndDone(false)
    , m_bAllowChange(false)
{
    DBG_ASSERT( pWindow, "TextConvWrapper: window missing" );

    m_aConvSel.Adjust();  // make Start <= End
}


TextConvWrapper::~TextConvWrapper()
{
}


bool TextConvWrapper::ConvNext_impl()
{
    // modified version of SvxSpellWrapper::SpellNext

    if( m_bStartChk )
        m_bStartDone = true;
    else
        m_bEndDone = true;

    if ( m_bStartDone && m_bEndDone )
    {
        if ( ConvMore_impl() )  // examine another document?
        {
            m_bStartDone = true;
            m_bEndDone  = false;
            ConvStart_impl( SVX_SPELL_BODY );
            return true;
        }
        return false;

    }

    if ( m_bStartDone && m_bEndDone )
    {
        if ( ConvMore_impl() )  // examine another document?
        {
            m_bStartDone = true;
            m_bEndDone  = false;
            ConvStart_impl( SVX_SPELL_BODY );
            return true;
        }
    }
    else if (!m_aConvSel.HasRange())
    {
        m_bStartChk = !m_bStartDone;
        ConvStart_impl( m_bStartChk ? SVX_SPELL_BODY_START : SVX_SPELL_BODY_END );
        return true;
    }

    return false;
}


bool TextConvWrapper::FindConvText_impl()
{
    // modified version of SvxSpellWrapper::FindSpellError

    bool bFound = false;

    m_pWin->EnterWait();
    bool bConvert = true;

    while ( bConvert )
    {
        bFound = ConvContinue_impl();
        if (bFound)
        {
            bConvert = false;
        }
        else
        {
            bConvert = ConvNext_impl();
        }
    }
    m_pWin->LeaveWait();
    return bFound;
}


bool TextConvWrapper::ConvMore_impl()
{
    // modified version of SvxSpellWrapper::SpellMore

    bool bMore = false;
    EditEngine* pEE = m_pEditView->GetEditEngine();
    ImpEditEngine* pImpEE = m_pEditView->GetImpEditEngine();
    ConvInfo* pConvInfo = pImpEE->GetConvInfo();
    if ( pConvInfo->bMultipleDoc )
    {
        bMore = pEE->ConvertNextDocument();
        if ( bMore )
        {
            // The text has been entered in this engine ...
            m_pEditView->GetImpEditView()->SetEditSelection(
                        pEE->GetEditDoc().GetStartPaM() );
        }
    }
    return bMore;
}


void TextConvWrapper::ConvStart_impl( SvxSpellArea eArea )
{
    // modified version of EditSpellWrapper::SpellStart

    EditEngine* pEE = m_pEditView->GetEditEngine();
    ImpEditEngine* pImpEE = m_pEditView->GetImpEditEngine();
    ConvInfo* pConvInfo = pImpEE->GetConvInfo();

    if ( eArea == SVX_SPELL_BODY_START )
    {
        // Is called when Spell-forward has reached the end, and to start over
        if ( m_bEndDone )
        {
            pConvInfo->bConvToEnd = false;
            pConvInfo->aConvTo = pConvInfo->aConvStart;
            pConvInfo->aConvContinue = EPaM( 0, 0 );
            m_pEditView->GetImpEditView()->SetEditSelection(
                    pEE->GetEditDoc().GetStartPaM() );
        }
        else
        {
            pConvInfo->bConvToEnd = true;
            pConvInfo->aConvTo = pImpEE->CreateEPaM(
                    pEE->GetEditDoc().GetStartPaM() );
        }
    }
    else if ( eArea == SVX_SPELL_BODY_END )
    {
        // Is called when Spell-forward starts
        pConvInfo->bConvToEnd = true;
        if (m_aConvSel.HasRange())
        {
            // user selection: convert to end of selection
            pConvInfo->aConvTo.nPara    = m_aConvSel.nEndPara;
            pConvInfo->aConvTo.nIndex   = m_aConvSel.nEndPos;
            pConvInfo->bConvToEnd       = false;
        }
        else
        {
            // nothing selected: convert to end of document
            pConvInfo->aConvTo = pImpEE->CreateEPaM(
                pEE->GetEditDoc().GetEndPaM() );
        }
    }
    else if ( eArea == SVX_SPELL_BODY )
    {
        // called by ConvNext_impl...
        pConvInfo->aConvContinue = pConvInfo->aConvStart;
        pConvInfo->aConvTo = pImpEE->CreateEPaM(
            pEE->GetEditDoc().GetEndPaM() );
    }
    else
    {
        OSL_FAIL( "ConvStart_impl: Unknown Area!" );
    }
}


bool TextConvWrapper::ConvContinue_impl()
{
    // modified version of EditSpellWrapper::SpellContinue

    // get next convertible text portion and its language
    m_aConvText.clear();
    m_nConvTextLang = LANGUAGE_NONE;
    m_pEditView->GetImpEditEngine()->ImpConvert( m_aConvText, m_nConvTextLang,
            m_pEditView, GetSourceLanguage(), m_aConvSel,
            m_bAllowChange, GetTargetLanguage(), GetTargetFont() );
    return !m_aConvText.isEmpty();
}


void TextConvWrapper::SetLanguageAndFont( const ESelection &rESel,
    LanguageType nLang, sal_uInt16 nLangWhichId,
    const vcl::Font *pFont,  sal_uInt16 nFontWhichId )
{
    ESelection aOldSel = m_pEditView->GetSelection();
    m_pEditView->SetSelection( rESel );

    // set new language attribute
    SfxItemSet aNewSet( m_pEditView->GetEmptyItemSet() );
    aNewSet.Put( SvxLanguageItem( nLang, nLangWhichId ) );

    // new font to be set?
    DBG_ASSERT( pFont, "target font missing?" );
    if (pFont)
    {
        // set new font attribute
        SvxFontItem aFontItem = static_cast<const SvxFontItem&>( aNewSet.Get( nFontWhichId ) );
        aFontItem.SetFamilyName( pFont->GetFamilyName());
        aFontItem.SetFamily( pFont->GetFamily());
        aFontItem.SetStyleName( pFont->GetStyleName());
        aFontItem.SetPitch( pFont->GetPitch());
        aFontItem.SetCharSet(pFont->GetCharSet());
        aNewSet.Put( aFontItem );
    }

    // apply new attributes
    m_pEditView->SetAttribs( aNewSet );

    m_pEditView->SetSelection( aOldSel );
}


void TextConvWrapper::SelectNewUnit_impl(
        const sal_Int32 nUnitStart,
        const sal_Int32 nUnitEnd )
{
    const bool bOK = 0 <= nUnitStart && 0 <= nUnitEnd && nUnitStart <= nUnitEnd;
    DBG_ASSERT( bOK, "invalid arguments" );
    if (!bOK)
        return;

    ESelection  aSelection = m_pEditView->GetSelection();
    DBG_ASSERT( aSelection.nStartPara == aSelection.nEndPara,
        "paragraph mismatch in selection" );
    aSelection.nStartPos = (m_nLastPos + m_nUnitOffset + nUnitStart);
    aSelection.nEndPos   = (m_nLastPos + m_nUnitOffset + nUnitEnd);
    m_pEditView->SetSelection( aSelection );
}


void TextConvWrapper::GetNextPortion(
        OUString& /* [out] */ rNextPortion,
        LanguageType&    /* [out] */ rLangOfPortion,
        bool /* [in] */ _bAllowImplicitChangesForNotConvertibleText )
{
    m_bAllowChange = _bAllowImplicitChangesForNotConvertibleText;

    FindConvText_impl();
    rNextPortion    = m_aConvText;
    rLangOfPortion  = m_nConvTextLang;
    m_nUnitOffset = 0;

    ESelection  aSelection = m_pEditView->GetSelection();
    DBG_ASSERT( aSelection.nStartPara == aSelection.nEndPara,
            "paragraph mismatch in selection" );
    DBG_ASSERT( aSelection.nStartPos  <= aSelection.nEndPos,
            "start pos > end pos" );
    m_nLastPos =  aSelection.nStartPos;
}


void TextConvWrapper::HandleNewUnit(
        const sal_Int32 nUnitStart,
        const sal_Int32 nUnitEnd )
{
    SelectNewUnit_impl( nUnitStart, nUnitEnd );
}

#ifdef DBG_UTIL
namespace
{
    bool IsSimilarChinese( LanguageType nLang1, LanguageType nLang2 )
    {
        using namespace editeng;
        return (HangulHanjaConversion::IsTraditional(nLang1) && HangulHanjaConversion::IsTraditional(nLang2)) ||
               (HangulHanjaConversion::IsSimplified(nLang1)  && HangulHanjaConversion::IsSimplified(nLang2));
    }
}
#endif

void TextConvWrapper::ReplaceUnit(
        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
        const OUString& rOrigText,
        const OUString& rReplaceWith,
        const css::uno::Sequence< sal_Int32 > &rOffsets,
        ReplacementAction eAction,
        LanguageType *pNewUnitLanguage )
{
    const bool bOK = 0 <= nUnitStart && 0 <= nUnitEnd && nUnitStart <= nUnitEnd;
    DBG_ASSERT( bOK, "invalid arguments" );
    if (!bOK)
        return;

    // select current unit
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    OUString aOrigTxt( m_pEditView->GetSelected() );
    OUString aNewTxt( rReplaceWith );
    switch (eAction)
    {
        case eExchange :
        break;
        case eReplacementBracketed :
            aNewTxt = aOrigTxt + "(" + rReplaceWith + ")";
        break;
        case eOriginalBracketed :
            aNewTxt = rReplaceWith + "(" + aOrigTxt + ")";
        break;
        case eReplacementAbove  :
        case eOriginalAbove :
        case eReplacementBelow :
        case eOriginalBelow :
            OSL_FAIL( "Rubies not supported" );
            break;
        default:
            OSL_FAIL( "unexpected case" );
    }
    m_nUnitOffset = m_nUnitOffset + nUnitStart + aNewTxt.getLength();

    // remember current original language for later use
    ImpEditEngine *pImpEditEng = m_pEditView->GetImpEditEngine();
    ESelection aOldSel     = m_pEditView->GetSelection();
    //EditSelection aOldEditSel = pEditView->GetImpEditView()->GetEditSelection();

#ifdef DBG_UTIL
    LanguageType nOldLang   = pImpEditEng->GetLanguage( pImpEditEng->CreateSel( aOldSel ).Min() );
#endif

    pImpEditEng->UndoActionStart( EDITUNDO_INSERT );

    // according to FT we should currently not bother about keeping
    // attributes in Hangul/Hanja conversion and leave that untouched.
    // Thus we do this only for Chinese translation...
    bool bIsChineseConversion = IsChinese( GetSourceLanguage() );
    if (bIsChineseConversion)
        ChangeText( aNewTxt, rOrigText, &rOffsets, &aOldSel );
    else
        ChangeText( aNewTxt, rOrigText, nullptr, nullptr );

    // change language and font if necessary
    if (bIsChineseConversion)
    {
        DBG_ASSERT( GetTargetLanguage() == LANGUAGE_CHINESE_SIMPLIFIED || GetTargetLanguage() == LANGUAGE_CHINESE_TRADITIONAL,
                "TextConvWrapper::ReplaceUnit : unexpected target language" );

        ESelection aNewSel( aOldSel );
        aNewSel.nStartPos = aNewSel.nStartPos - aNewTxt.getLength();

        if (pNewUnitLanguage)
        {
#ifdef DBG_UTIL
            DBG_ASSERT(!IsSimilarChinese( *pNewUnitLanguage, nOldLang ),
                    "similar language should not be changed!");
#endif
            SetLanguageAndFont( aNewSel, *pNewUnitLanguage, EE_CHAR_LANGUAGE_CJK,
                                          GetTargetFont(), EE_CHAR_FONTINFO_CJK );
        }
    }

    pImpEditEng->UndoActionEnd( EDITUNDO_INSERT );

    // adjust ConvContinue / ConvTo if necessary
    ImpEditEngine* pImpEE = m_pEditView->GetImpEditEngine();
    ConvInfo* pConvInfo = pImpEE->GetConvInfo();
    sal_Int32 nDelta = aNewTxt.getLength() - aOrigTxt.getLength();
    if (nDelta != 0)
    {
        // Note: replacement is always done in the current paragraph
        // which is the one ConvContinue points to
        pConvInfo->aConvContinue.nIndex = pConvInfo->aConvContinue.nIndex + nDelta;

        // if that is the same as the one where the conversions ends
        // the end needs to be updated also
        if (pConvInfo->aConvTo.nPara == pConvInfo->aConvContinue.nPara)
            pConvInfo->aConvTo.nIndex = pConvInfo->aConvTo.nIndex + nDelta;
    }
}


void TextConvWrapper::ChangeText( const OUString &rNewText,
        const OUString& rOrigText,
        const uno::Sequence< sal_Int32 > *pOffsets,
        ESelection *pESelection )
{
    //!! code is a modified copy of SwHHCWrapper::ChangeText from sw !!

    DBG_ASSERT( !rNewText.isEmpty(), "unexpected empty string" );
    if (rNewText.isEmpty())
        return;

    if (pOffsets && pESelection)  // try to keep as much attributation as possible ?
    {
        pESelection->Adjust();

        // remember cursor start position for later setting of the cursor
        const sal_Int32 nStartIndex = pESelection->nStartPos;

        const sal_Int32  nIndices = pOffsets->getLength();
        const sal_Int32 *pIndices = pOffsets->getConstArray();
        const sal_Int32  nConvTextLen = rNewText.getLength();
        sal_Int32 nPos = 0;
        sal_Int32 nChgPos = -1;
        sal_Int32 nConvChgPos = -1;

        // offset to calculate the position in the text taking into
        // account that text may have been replaced with new text of
        // different length. Negative values allowed!
        sal_Int32 nCorrectionOffset = 0;

        DBG_ASSERT(nIndices == 0 || nIndices == nConvTextLen,
                "mismatch between string length and sequence length!" );

        // find all substrings that need to be replaced (and only those)
        while (true)
        {
            // get index in original text that matches nPos in new text
            sal_Int32 nIndex;
            if (nPos < nConvTextLen)
                nIndex = nPos < nIndices ? pIndices[nPos] : nPos;
            else
            {
                nPos   = nConvTextLen;
                nIndex = rOrigText.getLength();
            }

            // end of string also terminates non-matching char sequence
            if (nPos == nConvTextLen || rOrigText[nIndex] == rNewText[nPos])
            {
                // substring that needs to be replaced found?
                if (nChgPos>=0 && nConvChgPos>=0)
                {
                    const sal_Int32 nChgLen = nIndex - nChgPos;
                    const sal_Int32 nConvChgLen = nPos - nConvChgPos;
                    OUString aInNew( rNewText.copy( nConvChgPos, nConvChgLen ) );

                    // set selection to sub string to be replaced in original text
                    ESelection aSel( *pESelection );
                    sal_Int32 nChgInNodeStartIndex = nStartIndex + nCorrectionOffset + nChgPos;
                    aSel.nStartPos = nChgInNodeStartIndex;
                    aSel.nEndPos   = nChgInNodeStartIndex + nChgLen;
                    m_pEditView->SetSelection( aSel );

                    // replace selected sub string with the corresponding
                    // sub string from the new text while keeping as
                    // much from the attributes as possible
                    ChangeText_impl( aInNew, true );

                    nCorrectionOffset += nConvChgLen - nChgLen;

                    nChgPos = -1;
                    nConvChgPos = -1;
                }
            }
            else
            {
                // begin of non-matching char sequence found ?
                if (nChgPos<0 && nConvChgPos<0)
                {
                    nChgPos = nIndex;
                    nConvChgPos = nPos;
                }
            }
            if (nPos >= nConvTextLen)
                break;
            ++nPos;
        }

        // set cursor to the end of the inserted text
        // (as it would happen after ChangeText_impl (Delete and Insert)
        // of the whole text in the 'else' branch below)
        pESelection->nStartPos = pESelection->nEndPos = nStartIndex + nConvTextLen;
    }
    else
    {
        ChangeText_impl( rNewText, false );
    }
}


void TextConvWrapper::ChangeText_impl( const OUString &rNewText, bool bKeepAttributes )
{
    if (bKeepAttributes)
    {
        // save attributes to be restored
        SfxItemSet aSet( m_pEditView->GetAttribs() );

        // replace old text and select new text
        m_pEditView->InsertText( rNewText, true );

        // since 'SetAttribs' below function like merging with the attributes
        // from the itemset with any existing ones we have to get rid of all
        // all attributes now. (Those attributes that may take effect left
        // to the position where the new text gets inserted after the old text
        // was deleted)
        m_pEditView->RemoveAttribs();
        // apply saved attributes to new inserted text
        m_pEditView->SetAttribs( aSet );
    }
    else
    {
        m_pEditView->InsertText( rNewText );
    }
}


void TextConvWrapper::Convert()
{
    m_bStartChk = false;
    ConvStart_impl( SVX_SPELL_BODY_END );
    ConvertDocument();
}


bool TextConvWrapper::HasRubySupport() const
{
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
