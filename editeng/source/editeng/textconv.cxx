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

#define C2U(cChar) OUString::createFromAscii(cChar)

//////////////////////////////////////////////////////////////////////

TextConvWrapper::TextConvWrapper( Window* pWindow,
        const Reference< XMultiServiceFactory >& rxMSF,
        const Locale& rSourceLocale,
        const Locale& rTargetLocale,
        const Font* pTargetFont,
        sal_Int32 nOptions,
        sal_Bool bIsInteractive,
        BOOL bIsStart,
        EditView* pView ) :
    HangulHanjaConversion( pWindow, rxMSF, rSourceLocale, rTargetLocale, pTargetFont, nOptions, bIsInteractive )
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

    sal_Bool bMore = sal_False;
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    ConvInfo* pConvInfo = pImpEE->GetConvInfo();
    if ( pConvInfo->bMultipleDoc )
    {
        bMore = pImpEE->GetEditEnginePtr()->ConvertNextDocument();
        if ( bMore )
        {
            // The text has been entered in this engine ...
            pEditView->GetImpEditView()->SetEditSelection(
                        pImpEE->GetEditDoc().GetStartPaM() );
        }
    }
    return bMore;
}


void TextConvWrapper::ConvStart_impl( SvxSpellArea eArea )
{
    // modified version of EditSpellWrapper::SpellStart

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
                    pImpEE->GetEditDoc().GetStartPaM() );
        }
        else
        {
            pConvInfo->bConvToEnd = sal_True;
            pConvInfo->aConvTo = pImpEE->CreateEPaM(
                    pImpEE->GetEditDoc().GetStartPaM() );
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
                pImpEE->GetEditDoc().GetEndPaM() );
        }
    }
    else if ( eArea == SVX_SPELL_BODY )
    {
        // called by ConvNext_impl...
        pConvInfo->aConvContinue = pConvInfo->aConvStart;
        pConvInfo->aConvTo = pImpEE->CreateEPaM(
            pImpEE->GetEditDoc().GetEndPaM() );
    }
    else
    {
        DBG_ERROR( "ConvStart_impl: Unknown Area!" );
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
    return aConvText.getLength() != 0;
}


void TextConvWrapper::SetLanguageAndFont( const ESelection &rESel,
    LanguageType nLang, USHORT nLangWhichId,
    const Font *pFont,  USHORT nFontWhichId )
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
        aFontItem.GetFamilyName()   = pFont->GetName();
        aFontItem.GetFamily()       = pFont->GetFamily();
        aFontItem.GetStyleName()    = pFont->GetStyleName();
        aFontItem.GetPitch()        = pFont->GetPitch();
        aFontItem.GetCharSet()      = pFont->GetCharSet();
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
    BOOL bOK = 0 <= nUnitStart && 0 <= nUnitEnd && nUnitStart <= nUnitEnd;
    DBG_ASSERT( bOK, "invalid arguments" );
    if (!bOK)
        return;

    ESelection  aSelection = pEditView->GetSelection();
    DBG_ASSERT( aSelection.nStartPara == aSelection.nEndPara,
        "paragraph mismatch in selection" );
    aSelection.nStartPos = (USHORT) (nLastPos + nUnitOffset + nUnitStart);
    aSelection.nEndPos   = (USHORT) (nLastPos + nUnitOffset + nUnitEnd);
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


void TextConvWrapper::ReplaceUnit(
        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
        const ::rtl::OUString& rOrigText,
        const ::rtl::OUString& rReplaceWith,
        const ::com::sun::star::uno::Sequence< sal_Int32 > &rOffsets,
        ReplacementAction eAction,
        LanguageType *pNewUnitLanguage )
{
    BOOL bOK = 0 <= nUnitStart && 0 <= nUnitEnd && nUnitStart <= nUnitEnd;
    DBG_ASSERT( bOK, "invalid arguments" );
    if (!bOK)
        return;

    static OUString aBracketedStart( C2U( "(" ) );
    static OUString aBracketedEnd( C2U( ")" ) );

    // select current unit
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    OUString aOrigTxt( pEditView->GetSelected() );
    OUString aNewTxt( rReplaceWith );
    String  aNewOrigText;
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
            DBG_ERROR( "Rubies not supported" );
            break;
        default:
            DBG_ERROR( "unexpected case" );
    }
    nUnitOffset = sal::static_int_cast< USHORT >(
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
        pConvInfo->aConvContinue.nIndex = sal::static_int_cast< USHORT >(
            pConvInfo->aConvContinue.nIndex + nDelta);

        // if that is the same as the one where the conversions ends
        // the end needs to be updated also
        if (pConvInfo->aConvTo.nPara == pConvInfo->aConvContinue.nPara)
            pConvInfo->aConvTo.nIndex = sal::static_int_cast< USHORT >(
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
        while (sal_True)
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
