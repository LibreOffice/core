/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textconv.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 14:39:57 $
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
#include <eeng_pch.hxx>

#pragma hdrstop

#include <impedit.hxx>
#include <editview.hxx>
#include <editeng.hxx>

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _SVX_LANGITEM_HXX
#include <langitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <fontitem.hxx>
#endif

#ifndef _TEXTCONV_HXX
#include "textconv.hxx"
#endif


using namespace rtl;
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
        if ( ConvMore_impl() )  // ein weiteres Dokument pruefen?
        {
            bStartDone = sal_True;
            bEndDone  = sal_False;
            ConvStart_impl( SVX_SPELL_BODY );
            return sal_True;
        }
        return sal_False;

    }

    //ResMgr* pMgr = DIALOG_MGR();
    sal_Bool bGoOn = sal_False;

    if ( bStartDone && bEndDone )
    {
        if ( ConvMore_impl() )  // ein weiteres Dokument pruefen?
        {
            bStartDone = sal_True;
            bEndDone  = sal_False;
            ConvStart_impl( SVX_SPELL_BODY );
            return sal_True;
        }
    }
    else
    {
        // Ein BODY_Bereich erledigt, Frage nach dem anderen BODY_Bereich
/*
        pWin->LeaveWait();

        sal_uInt16 nResId = bReverse ? RID_SVXQB_BW_CONTINUE : RID_SVXQB_CONTINUE;
        QueryBox aBox( pWin, ResId( nResId, pMgr ) );
        if ( aBox.Execute() != RET_YES )
        {
            // Verzicht auf den anderen Bereich, ggf. Frage nach Sonderbereich
            pWin->EnterWait();
            bStartDone = bEndDone = sal_True;
            return ConvNext_impl();
        }
        else
        {
*/
            if (!aConvSel.HasRange())
            {
                bStartChk = !bStartDone;
                ConvStart_impl( bStartChk ? SVX_SPELL_BODY_START : SVX_SPELL_BODY_END );
                bGoOn = sal_True;
            }
/*
        }
        pWin->EnterWait();
*/
    }
    return bGoOn;
}


sal_Bool TextConvWrapper::FindConvText_impl()
{
    // modified version of SvxSpellWrapper::FindSpellError

    //ShowLanguageErrors();

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
            // Der Text wurde in diese Engine getreten...
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
        // Wird gerufen, wenn Spell-Forwad am Ende angekomment ist
        // und soll von vorne beginnen
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
        // Wird gerufen, wenn Spell-Forwad gestartet wird
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
        // pSpellInfo->bSpellToEnd = sal_True;
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
        const ::rtl::OUString& rReplaceWith,
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
    nUnitOffset += (USHORT) (nUnitStart + aNewTxt.getLength() );

    // remember current original language for kater use
    ImpEditEngine *pImpEditEng = pEditView->GetImpEditEngine();
    ESelection aOldSel      = pEditView->GetSelection();
    LanguageType nOldLang   = pImpEditEng->GetLanguage( pImpEditEng->CreateSel( aOldSel ).Min() );

    pEditView->InsertText( aNewTxt );

    // change language and font if necessary
    if (IsChinese( GetSourceLanguage() ))
    {
        DBG_ASSERT( GetTargetLanguage() == LANGUAGE_CHINESE_SIMPLIFIED || GetTargetLanguage() == LANGUAGE_CHINESE_TRADITIONAL,
                "TextConvWrapper::ReplaceUnit : unexpected target language" );

        ESelection aOldSel = pEditView->GetSelection();
        ESelection aNewSel( aOldSel );
        aNewSel.nStartPos -= (xub_StrLen) aNewTxt.getLength();
        DBG_ASSERT( aOldSel.nEndPos >= 0, "error while building selection" );

        if (pNewUnitLanguage)
        {
            DBG_ASSERT(!IsSimilarChinese( *pNewUnitLanguage, nOldLang ),
                    "similar language should not be changed!");
            SetLanguageAndFont( aNewSel, *pNewUnitLanguage, EE_CHAR_LANGUAGE_CJK,
                                          GetTargetFont(), EE_CHAR_FONTINFO_CJK );
        }
    }

    // adjust ConvContinue / ConvTo if necessary
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    ConvInfo* pConvInfo = pImpEE->GetConvInfo();
    sal_Int32 nDelta = aNewTxt.getLength() - aOrigTxt.getLength();
    if (nDelta != 0)
    {
        // Note: replacement is always done in the current paragraph
        // which is the one ConvContinue points to
        pConvInfo->aConvContinue.nIndex += (USHORT) nDelta;

        // if that is the same as the one where the conversions ends
        // the end needs to be updated also
        if (pConvInfo->aConvTo.nPara == pConvInfo->aConvContinue.nPara)
            pConvInfo->aConvTo.nIndex += (USHORT) nDelta;
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

