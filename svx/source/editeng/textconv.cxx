/*************************************************************************
 *
 *  $RCSfile: textconv.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-27 15:49:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        const Locale& rLocale,
        BOOL bIsStart,
        EditView* pView ) :
    HangulHanjaConversion( pWindow, rxMSF, rLocale )
{
    DBG_ASSERT( pWindow, "TextConvWrapper: window missing" );

    nUnitOffset = 0;
    nLang = SvxLocaleToLanguage( rLocale );

    // currently this implementation only works for Korean (Hangu/Hanja conversion)
    // since it is derived by 'HangulHanjaConversion' and there is not
    // a more general base class for text conversion yet...
    DBG_ASSERT( nLang == LANGUAGE_KOREAN, "unexpected language" );

    bStartChk   = sal_False;
    bStartDone  = bIsStart;
    bEndDone    = sal_False;
    pWin        = pWindow;
    pEditView   = pView;

    aConvSel    = pEditView->GetSelection();
    aConvSel.Adjust();  // make Start <= End
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
    // modified version of EditSpellWrapper::SpellEnd

    // nothing to be done
}


sal_Bool TextConvWrapper::ConvContinue_impl()
{
    // modified version of EditSpellWrapper::SpellContinue

    aConvText = pEditView->GetImpEditEngine()->ImpConvert( pEditView, nLang, aConvSel );
    return aConvText.getLength() != 0;
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


void TextConvWrapper::GetNextPortion( ::rtl::OUString& /* [out] */ rNextPortion )
{
    FindConvText_impl();
    rNextPortion = aConvText;
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
        ReplacementAction eAction )
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

    pEditView->InsertText( aNewTxt );

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

