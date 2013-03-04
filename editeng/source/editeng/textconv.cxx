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


using ::rtl::OUString;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::linguistic2;

//////////////////////////////////////////////////////////////////////

TextConvWrapper::TextConvWrapper( Window* pWindow,
        const Reference< XComponentContext >& rxContext,
        const Locale& rSourceLocale,
        const Locale& rTargetLocale,
        const Font* pTargetFont,
        sal_Int32 nOptions,
        sal_Bool bIsInteractive,
        sal_Bool bIsStart,
        EditView* pView ) :
    HangulHanjaConversion( pWindow, rxContext, rSourceLocale, rTargetLocale, pTargetFont, nOptions, bIsInteractive )
{
    DBG_ASSERT( pWindow, "TextConvWrapper: window missing" );

    nConvTextLang = LANGUAGE_NONE;
    nUnitOffset = 0;

    bStartChk   = sal_False;
    bStartDone  = bIsStart;
    bEndDone    = sal_False;
    pWin        = pWindow;
    pEditView   = pView;

    aConvSel    = pEditView->GetSelection();
    aConvSel.Adjust();  // make Start <= End

    bAllowChange = sal_False;
}


TextConvWrapper::~TextConvWrapper()
{
}


sal_Bool TextConvWrapper::ConvNext_impl()
{
    // modified version of SvxSpellWrapper::SpellNext

    if( bStartChk )
        bStartDone = sal_True;
    else
        bEndDone = sal_True;

    if ( bStartDone && bEndDone )
    {
        if ( ConvMore_impl() )  // examine another document?
        {
            bStartDone = sal_True;
            bEndDone  = sal_False;
            ConvStart_impl( SVX_SPELL_BODY );
            return sal_True;
        }
        return sal_False;

    }

    sal_Bool bGoOn = sal_False;

    if ( bStartDone && bEndDone )
    {
        if ( ConvMore_impl() )  // examine another document?
        {
            bStartDone = sal_True;
            bEndDone  = sal_False;
            ConvStart_impl( SVX_SPELL_BODY );
            return sal_True;
        }
    }
    else if (!aConvSel.HasRange())
    {
        bStartChk = !bStartDone;
        ConvStart_impl( bStartChk ? SVX_SPELL_BODY_START : SVX_SPELL_BODY_END );
        bGoOn = sal_True;
    }
    return bGoOn;
}


sal_Bool TextConvWrapper::FindConvText_impl()
{
    // modified version of SvxSpellWrapper::FindSpellError

    sal_Bool bFound = sal_False;

    pWin->EnterWait();
    sal_Bool bConvert = sal_True;

    while ( bConvert )
    {
        bFound = ConvContinue_impl();
        if (bFound)
        {
            bConvert = sal_False;
        }
        else
        {
            ConvEnd_impl();
            bConvert = ConvNext_impl();
        }
    }
    pWin->LeaveWait();
    return bFound;
}


sal_Bool TextConvWrapper::ConvMore_impl()
{
    // modified version of SvxSpellWrapper::SpellMore

    bool bMore = false;
    EditEngine* pEE = pEditView->GetEditEngine();
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    ConvInfo* pConvInfo = pImpEE->GetConvInfo();
    if ( pConvInfo->bMultipleDoc )
    {
        bMore = pEE->ConvertNextDocument();
        if ( bMore )
        {
            // The text has been entered in this engine ...
            pEditView->GetImpEditView()->SetEditSelection(
                        pEE->GetEditDoc().GetStartPaM() );
        }
    }
    return bMore;
}


void TextConvWrapper::ConvStart_impl( SvxSpellArea eArea )
{
    // modified version of EditSpellWrapper::SpellStart

    EditEngine* pEE = pEditView->GetEditEngine();
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    ConvInfo* pConvInfo = pImpEE->GetConvInfo();

    if ( eArea == SVX_SPELL_BODY_START )
    {
        // Is called when Spell-forward has reached the end, and to start over
        if ( bEndDone )
        {
            pConvInfo->bConvToEnd = sal_False;
            pConvInfo->aConvTo = pConvInfo->aConvStart;
            pConvInfo->aConvContinue = EPaM( 0, 0 );
            pEditView->GetImpEditView()->SetEditSelection(
                    pEE->GetEditDoc().GetStartPaM() );
        }
        else
        {
            pConvInfo->bConvToEnd = sal_True;
            pConvInfo->aConvTo = pImpEE->CreateEPaM(
                    pEE->GetEditDoc().GetStartPaM() );
        }
    }
    else if ( eArea == SVX_SPELL_BODY_END )
    {
        // Is called when Spell-forward starts
        pConvInfo->bConvToEnd = sal_True;
        if (aConvSel.HasRange())
        {
            // user selection: convert to end of selection
            pConvInfo->aConvTo.nPara    = aConvSel.nEndPara;
            pConvInfo->aConvTo.nIndex   = aConvSel.nEndPos;
            pConvInfo->bConvToEnd       = sal_False;
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


void TextConvWrapper::ConvEnd_impl()
{
}


sal_Bool TextConvWrapper::ConvContinue_impl()
{
    // modified version of EditSpellWrapper::SpellContinue

    // get next convertible text portion and its language
    aConvText = rtl::OUString();
    nConvTextLang = LANGUAGE_NONE;
    pEditView->GetImpEditEngine()->ImpConvert( aConvText, nConvTextLang,
            pEditView, GetSourceLanguage(), aConvSel,
            bAllowChange, GetTargetLanguage(), GetTargetFont() );
    return !aConvText.isEmpty();
}


void TextConvWrapper::SetLanguageAndFont( const ESelection &rESel,
    LanguageType nLang, sal_uInt16 nLangWhichId,
    const Font *pFont,  sal_uInt16 nFontWhichId )
{
    ESelection aOldSel = pEditView->GetSelection();
    pEditView->SetSelection( rESel );

    // set new language attribute
    SfxItemSet aNewSet( pEditView->GetEmptyItemSet() );
    aNewSet.Put( SvxLanguageItem( nLang, nLangWhichId ) );

    // new font to be set?
    DBG_ASSERT( pFont, "target font missing?" );
    if (pFont)
    {
        // set new font attribute
        SvxFontItem aFontItem = (SvxFontItem&) aNewSet.Get( nFontWhichId );
        aFontItem.SetFamilyName( pFont->GetName());
        aFontItem.SetFamily( pFont->GetFamily());
        aFontItem.SetStyleName( pFont->GetStyleName());
        aFontItem.SetPitch( pFont->GetPitch());
        aFontItem.SetCharSet(pFont->GetCharSet());
        aNewSet.Put( aFontItem );
    }

    // apply new attributes
    pEditView->SetAttribs( aNewSet );

    pEditView->SetSelection( aOldSel );
}


void TextConvWrapper::SelectNewUnit_impl(
        const sal_Int32 nUnitStart,
        const sal_Int32 nUnitEnd )
{
    sal_Bool bOK = 0 <= nUnitStart && 0 <= nUnitEnd && nUnitStart <= nUnitEnd;
    DBG_ASSERT( bOK, "invalid arguments" );
    if (!bOK)
        return;

    ESelection  aSelection = pEditView->GetSelection();
    DBG_ASSERT( aSelection.nStartPara == aSelection.nEndPara,
        "paragraph mismatch in selection" );
    aSelection.nStartPos = (sal_uInt16) (nLastPos + nUnitOffset + nUnitStart);
    aSelection.nEndPos   = (sal_uInt16) (nLastPos + nUnitOffset + nUnitEnd);
    pEditView->SetSelection( aSelection );
}


void TextConvWrapper::GetNextPortion(
        ::rtl::OUString& /* [out] */ rNextPortion,
        LanguageType&    /* [out] */ rLangOfPortion,
        sal_Bool /* [in] */ _bAllowImplicitChangesForNotConvertibleText )
{
    bAllowChange = _bAllowImplicitChangesForNotConvertibleText;

    FindConvText_impl();
    rNextPortion    = aConvText;
    rLangOfPortion  = nConvTextLang;
    nUnitOffset  = 0;

    ESelection  aSelection = pEditView->GetSelection();
    DBG_ASSERT( aSelection.nStartPara == aSelection.nEndPara,
            "paragraph mismatch in selection" );
    DBG_ASSERT( aSelection.nStartPos  <= aSelection.nEndPos,
            "start pos > end pos" );
    nLastPos =  aSelection.nStartPos;
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
    sal_Bool IsSimilarChinese( LanguageType nLang1, LanguageType nLang2 )
    {
        using namespace editeng;
        return (HangulHanjaConversion::IsTraditional(nLang1) && HangulHanjaConversion::IsTraditional(nLang2)) ||
               (HangulHanjaConversion::IsSimplified(nLang1)  && HangulHanjaConversion::IsSimplified(nLang2));
    }
}
#endif

void TextConvWrapper::ReplaceUnit(
        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
        const ::rtl::OUString& rOrigText,
        const ::rtl::OUString& rReplaceWith,
        const ::com::sun::star::uno::Sequence< sal_Int32 > &rOffsets,
        ReplacementAction eAction,
        LanguageType *pNewUnitLanguage )
{
    sal_Bool bOK = 0 <= nUnitStart && 0 <= nUnitEnd && nUnitStart <= nUnitEnd;
    DBG_ASSERT( bOK, "invalid arguments" );
    if (!bOK)
        return;

    static OUString aBracketedStart( "(" );
    static OUString aBracketedEnd( ")" );

    // select current unit
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    OUString aOrigTxt( pEditView->GetSelected() );
    OUString aNewTxt( rReplaceWith );
    switch (eAction)
    {
        case eExchange :
        break;
        case eReplacementBracketed :
            (((aNewTxt = aOrigTxt) += aBracketedStart) += rReplaceWith) += aBracketedEnd;
        break;
        case eOriginalBracketed :
            (((aNewTxt = rReplaceWith) += aBracketedStart) += aOrigTxt) += aBracketedEnd;
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
    nUnitOffset = sal::static_int_cast< sal_uInt16 >(
        nUnitOffset + nUnitStart + aNewTxt.getLength());

    // remember current original language for kater use
    ImpEditEngine *pImpEditEng = pEditView->GetImpEditEngine();
    ESelection _aOldSel     = pEditView->GetSelection();
    //EditSelection aOldEditSel = pEditView->GetImpEditView()->GetEditSelection();

#ifdef DBG_UTIL
    LanguageType nOldLang   = pImpEditEng->GetLanguage( pImpEditEng->CreateSel( _aOldSel ).Min() );
#endif

    pImpEditEng->UndoActionStart( EDITUNDO_INSERT );

    // according to FT we should currently not bother about keeping
    // attributes in Hangul/Hanja conversion and leave that untouched.
    // Thus we do this only for Chinese translation...
    sal_Bool bIsChineseConversion = IsChinese( GetSourceLanguage() );
    if (bIsChineseConversion)
        ChangeText( aNewTxt, rOrigText, &rOffsets, &_aOldSel );
    else
        ChangeText( aNewTxt, rOrigText, NULL, NULL );

    // change language and font if necessary
    if (bIsChineseConversion)
    {
        DBG_ASSERT( GetTargetLanguage() == LANGUAGE_CHINESE_SIMPLIFIED || GetTargetLanguage() == LANGUAGE_CHINESE_TRADITIONAL,
                "TextConvWrapper::ReplaceUnit : unexpected target language" );

        ESelection aOldSel = pEditView->GetSelection();
        ESelection aNewSel( aOldSel );
        aNewSel.nStartPos = sal::static_int_cast< xub_StrLen >(
            aNewSel.nStartPos - aNewTxt.getLength());

        if (pNewUnitLanguage)
        {
            DBG_ASSERT(!IsSimilarChinese( *pNewUnitLanguage, nOldLang ),
                    "similar language should not be changed!");
            SetLanguageAndFont( aNewSel, *pNewUnitLanguage, EE_CHAR_LANGUAGE_CJK,
                                          GetTargetFont(), EE_CHAR_FONTINFO_CJK );
        }
    }

    pImpEditEng->UndoActionEnd( EDITUNDO_INSERT );

    // adjust ConvContinue / ConvTo if necessary
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    ConvInfo* pConvInfo = pImpEE->GetConvInfo();
    sal_Int32 nDelta = aNewTxt.getLength() - aOrigTxt.getLength();
    if (nDelta != 0)
    {
        // Note: replacement is always done in the current paragraph
        // which is the one ConvContinue points to
        pConvInfo->aConvContinue.nIndex = sal::static_int_cast< sal_uInt16 >(
            pConvInfo->aConvContinue.nIndex + nDelta);

        // if that is the same as the one where the conversions ends
        // the end needs to be updated also
        if (pConvInfo->aConvTo.nPara == pConvInfo->aConvContinue.nPara)
            pConvInfo->aConvTo.nIndex = sal::static_int_cast< sal_uInt16 >(
                pConvInfo->aConvTo.nIndex + nDelta);
    }
}


void TextConvWrapper::ChangeText( const String &rNewText,
        const OUString& rOrigText,
        const uno::Sequence< sal_Int32 > *pOffsets,
        ESelection *pESelection )
{
    //!! code is a modifed copy of SwHHCWrapper::ChangeText from sw !!

    DBG_ASSERT( rNewText.Len() != 0, "unexpected empty string" );
    if (rNewText.Len() == 0)
        return;

    if (pOffsets && pESelection)  // try to keep as much attributation as possible ?
    {
        pESelection->Adjust();

        // remember cursor start position for later setting of the cursor
        const xub_StrLen nStartIndex = pESelection->nStartPos;

        const sal_Int32  nIndices = pOffsets->getLength();
        const sal_Int32 *pIndices = pOffsets->getConstArray();
        xub_StrLen nConvTextLen = rNewText.Len();
        xub_StrLen nPos = 0;
        xub_StrLen nChgPos = STRING_NOTFOUND;
        xub_StrLen nChgLen = 0;
        xub_StrLen nConvChgPos = STRING_NOTFOUND;
        xub_StrLen nConvChgLen = 0;

        // offset to calculate the position in the text taking into
        // account that text may have been replaced with new text of
        // different length. Negative values allowed!
        long nCorrectionOffset = 0;

        DBG_ASSERT(nIndices == 0 || nIndices == nConvTextLen,
                "mismatch between string length and sequence length!" );

        // find all substrings that need to be replaced (and only those)
        while (true)
        {
            // get index in original text that matches nPos in new text
            xub_StrLen nIndex;
            if (nPos < nConvTextLen)
                nIndex = (sal_Int32) nPos < nIndices ? (xub_StrLen) pIndices[nPos] : nPos;
            else
            {
                nPos   = nConvTextLen;
                nIndex = static_cast< xub_StrLen >( rOrigText.getLength() );
            }

            if (rOrigText.getStr()[nIndex] == rNewText.GetChar(nPos) ||
                nPos == nConvTextLen /* end of string also terminates non-matching char sequence */)
            {
                // substring that needs to be replaced found?
                if (nChgPos != STRING_NOTFOUND && nConvChgPos != STRING_NOTFOUND)
                {
                    nChgLen = nIndex - nChgPos;
                    nConvChgLen = nPos - nConvChgPos;
#ifdef DEBUG
                    String aInOrig( rOrigText.copy( nChgPos, nChgLen ) );
#endif
                    String aInNew( rNewText.Copy( nConvChgPos, nConvChgLen ) );

                    // set selection to sub string to be replaced in original text
                    ESelection aSel( *pESelection );
                    xub_StrLen nChgInNodeStartIndex = static_cast< xub_StrLen >( nStartIndex + nCorrectionOffset + nChgPos );
                    aSel.nStartPos = nChgInNodeStartIndex;
                    aSel.nEndPos   = nChgInNodeStartIndex + nChgLen;
                    pEditView->SetSelection( aSel );
#ifdef DEBUG
                    String aSelTxt1( pEditView->GetSelected() );
#endif

                    // replace selected sub string with the corresponding
                    // sub string from the new text while keeping as
                    // much from the attributes as possible
                    ChangeText_impl( aInNew, sal_True );

                    nCorrectionOffset += nConvChgLen - nChgLen;

                    nChgPos = STRING_NOTFOUND;
                    nConvChgPos = STRING_NOTFOUND;
                }
            }
            else
            {
                // begin of non-matching char sequence found ?
                if (nChgPos == STRING_NOTFOUND && nConvChgPos == STRING_NOTFOUND)
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
        ChangeText_impl( rNewText, sal_False );
    }
}


void TextConvWrapper::ChangeText_impl( const String &rNewText, sal_Bool bKeepAttributes )
{
    if (bKeepAttributes)
    {
        // save attributes to be restored
        SfxItemSet aSet( pEditView->GetAttribs() );

#ifdef DEBUG
        String aSelTxt1( pEditView->GetSelected() );
#endif
        // replace old text and select new text
        pEditView->InsertText( rNewText, sal_True );
#ifdef DEBUG
        String aSelTxt2( pEditView->GetSelected() );
#endif

        // since 'SetAttribs' below function like merging with the attributes
        // from the itemset with any existing ones we have to get rid of all
        // all attributes now. (Those attributes that may take effect left
        // to the position where the new text gets inserted after the old text
        // was deleted)
        pEditView->RemoveAttribs();
        // apply saved attributes to new inserted text
        pEditView->SetAttribs( aSet );
    }
    else
    {
        pEditView->InsertText( rNewText );
    }
}


void TextConvWrapper::Convert()
{
    bStartChk = sal_False;
    ConvStart_impl( SVX_SPELL_BODY_END );
    ConvertDocument();
    ConvEnd_impl();
}


sal_Bool TextConvWrapper::HasRubySupport() const
{
    return sal_False;
}

//////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
