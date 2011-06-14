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
#include "precompiled_sw.hxx"
#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif


#include <hintids.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>         // fuer Undo-Ids
#include <globals.hrc>
#include <splargs.hxx>


#include <vcl/msgbox.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fontitem.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <hhcwrp.hxx>
#include <sdrhhcwrap.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <txatritr.hxx>
#include <mdiexp.hxx>       // Progress
#include <edtwin.hxx>
#include <crsskip.hxx>
#include <index.hxx>
#include <pam.hxx>
#include <swcrsr.hxx>
#include <viscrs.hxx>
#include <ndtxt.hxx>
#include <fmtruby.hxx>
#include <breakit.hxx>

#include <olmenu.hrc>

#include <unomid.h>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;

#define CHAR_PAR_BRK    ((sal_Char) 0x0D)

//////////////////////////////////////////////////////////////////////
//     Beschreibung: Ggf. Rahmen/Objektshell abschalten

static void lcl_ActivateTextShell( SwWrtShell & rWrtSh )
{
    if( rWrtSh.IsSelFrmMode() || rWrtSh.IsObjSelected() )
        rWrtSh.EnterStdMode();
}

//////////////////////////////////////////////////////////////////////

class SwKeepConversionDirectionStateContext
{
public:
    SwKeepConversionDirectionStateContext()
    {
        //!! hack to transport the current conversion direction state settings
        //!! into the next incarnation that iterates over the drawing objets
        //!! ( see SwHHCWrapper::~SwHHCWrapper() )
        editeng::HangulHanjaConversion::SetUseSavedConversionDirectionState( sal_True );
    }

    ~SwKeepConversionDirectionStateContext()
    {
        editeng::HangulHanjaConversion::SetUseSavedConversionDirectionState( sal_False );
    }
};

//////////////////////////////////////////////////////////////////////

SwHHCWrapper::SwHHCWrapper(
        SwView* pSwView,
        const uno::Reference< lang::XMultiServiceFactory >& rxMSF,
        LanguageType nSourceLanguage,
        LanguageType nTargetLanguage,
        const Font *pTargetFont,
        sal_Int32 nConvOptions,
        sal_Bool bIsInteractive,
        sal_Bool bStart, sal_Bool bOther, sal_Bool bSelection ) :
    editeng::HangulHanjaConversion( &pSwView->GetEditWin(), rxMSF,
                                SvxCreateLocale( nSourceLanguage ),
                                SvxCreateLocale( nTargetLanguage ),
                                pTargetFont,
                                nConvOptions,
                                bIsInteractive ),
    rWrtShell( pSwView->GetWrtShell() )
{
    pConvArgs       = 0;
    nLastPos        = 0;
    nUnitOffset     = 0;

    pView           = pSwView;
    pWin            = &pSwView->GetEditWin();
    bIsDrawObj      = sal_False;
    bIsStart        = bStart;
    bIsOtherCntnt   = bStartChk     = bOther;
    bIsConvSpecial  = sal_True;
    bIsSelection    = bSelection;
    bInfoBox        = sal_False;
    bStartDone  = bOther || bStart;
    bEndDone    = sal_False;
//    bLastRet    = sal_True;
    nPageCount  = nPageStart = 0;
}


SwHHCWrapper::~SwHHCWrapper()
{
    delete pConvArgs;

    rWrtShell.SetCareWin( NULL );

    // check for existence of a draw view which means that there are
    // (or previously were) draw objects present in the document.
    // I.e. we like to check those too.
    if ( IsDrawObj() /*&& bLastRet*/ && pView->GetWrtShell().HasDrawView() )
    {
        Cursor *pSave = pView->GetWindow()->GetCursor();
        {
            SwKeepConversionDirectionStateContext aContext;

            SdrHHCWrapper aSdrConvWrap( pView, GetSourceLanguage(),
                    GetTargetLanguage(), GetTargetFont(),
                    GetConversionOptions(), IsInteractive() );
            aSdrConvWrap.StartTextConversion();
        }
        pView->GetWindow()->SetCursor( pSave );
    }

    if( nPageCount )
        ::EndProgress( pView->GetDocShell() );

    // finally for chinese translation we need to change the the documents
    // default language and font to the new ones to be used.
    LanguageType nTargetLang = GetTargetLanguage();
    if (IsChinese( nTargetLang ))
    {
        SwDoc *pDoc = pView->GetDocShell()->GetDoc();

        //!! Note: This also effects the default language of text boxes (EditEngine/EditView) !!
        pDoc->SetDefault( SvxLanguageItem( nTargetLang, RES_CHRATR_CJK_LANGUAGE ) );
        //
        const Font *pFont = GetTargetFont();
        if (pFont)
        {
            SvxFontItem aFontItem( pFont->GetFamily(), pFont->GetName(),
                    pFont->GetStyleName(), pFont->GetPitch(),
                    pFont->GetCharSet(), RES_CHRATR_CJK_FONT );
            pDoc->SetDefault( aFontItem );
        }

    }
}


void SwHHCWrapper::GetNextPortion(
        ::rtl::OUString&    rNextPortion,
        LanguageType&       rLangOfPortion,
        sal_Bool bAllowChanges )
{
    pConvArgs->bAllowImplicitChangesForNotConvertibleText = bAllowChanges;

    FindConvText_impl();
    rNextPortion    = pConvArgs->aConvText;
    rLangOfPortion  = pConvArgs->nConvTextLang;

    nUnitOffset  = 0;

    // build last pos from currently selected text
    SwPaM* pCrsr = rWrtShell.GetCrsr();
    nLastPos =  pCrsr->Start()->nContent.GetIndex();
}


void SwHHCWrapper::SelectNewUnit_impl( sal_Int32 nUnitStart, sal_Int32 nUnitEnd )
{
    SwPaM *pCrsr = rWrtShell.GetCrsr();
    pCrsr->GetPoint()->nContent = nLastPos;
    pCrsr->DeleteMark();

    rWrtShell.Right( CRSR_SKIP_CHARS, /*bExpand*/ sal_False,
                  (sal_uInt16) (nUnitOffset + nUnitStart), sal_True );
    pCrsr->SetMark();
    rWrtShell.Right( CRSR_SKIP_CHARS, /*bExpand*/ sal_True,
                  (sal_uInt16) (nUnitEnd - nUnitStart), sal_True );
    // end selection now. Otherwise SHIFT+HOME (extending the selection)
    // won't work when the dialog is closed without any replacement.
    // (see #116346#)
    rWrtShell.EndSelect();
}


void SwHHCWrapper::HandleNewUnit(
        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd )
{
    OSL_ENSURE( nUnitStart >= 0 && nUnitEnd >= nUnitStart, "wrong arguments" );
    if (!(0 <= nUnitStart && nUnitStart <= nUnitEnd))
        return;

    lcl_ActivateTextShell( rWrtShell );

    rWrtShell.StartAllAction();

    // select current unit
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    rWrtShell.EndAllAction();
}


void SwHHCWrapper::ChangeText( const String &rNewText,
        const OUString& rOrigText,
        const uno::Sequence< sal_Int32 > *pOffsets,
        SwPaM *pCrsr )
{
    //!! please see also TextConvWrapper::ChangeText with is a modified
    //!! copy of this code

    OSL_ENSURE( rNewText.Len() != 0, "unexpected empty string" );
    if (rNewText.Len() == 0)
        return;

    if (pOffsets && pCrsr)  // try to keep as much attributation as possible ?
    {
        // remember cursor start position for later setting of the cursor
        const SwPosition *pStart = pCrsr->Start();
        const xub_StrLen nStartIndex = pStart->nContent.GetIndex();
        const SwNodeIndex aStartNodeIndex  = pStart->nNode;
        SwTxtNode *pStartTxtNode = aStartNodeIndex.GetNode().GetTxtNode();

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

        OSL_ENSURE(nIndices == 0 || nIndices == nConvTextLen,
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
                    xub_StrLen nChgInNodeStartIndex = static_cast< xub_StrLen >( nStartIndex + nCorrectionOffset + nChgPos );
                    OSL_ENSURE( rWrtShell.GetCrsr()->HasMark(), "cursor misplaced (nothing selected)" );
                    rWrtShell.GetCrsr()->GetMark()->nContent.Assign( pStartTxtNode, nChgInNodeStartIndex );
                    rWrtShell.GetCrsr()->GetPoint()->nContent.Assign( pStartTxtNode, nChgInNodeStartIndex + nChgLen );
#ifdef DEBUG
                    String aSelTxt1( rWrtShell.GetSelTxt() );
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

        // set cursor to the end of all the new text
        // (as it would happen after ChangeText_impl (Delete and Insert)
        // of the whole text in the 'else' branch below)
        rWrtShell.ClearMark();
        rWrtShell.GetCrsr()->Start()->nContent.Assign( pStartTxtNode, nStartIndex + nConvTextLen );
    }
    else
    {
        ChangeText_impl( rNewText, sal_False );
    }
}


void SwHHCWrapper::ChangeText_impl( const String &rNewText, sal_Bool bKeepAttributes )
{
    if (bKeepAttributes)
    {
        // get item set with all relevant attributes
        sal_uInt16 aRanges[] = {
                RES_CHRATR_BEGIN, RES_FRMATR_END,
                0, 0, 0  };
        SfxItemSet aItemSet( rWrtShell.GetAttrPool(), aRanges );
        // get all attributes spanning the whole selection in order to
        // restore those for the new text
        rWrtShell.GetCurAttr( aItemSet );

#ifdef DEBUG
        String aSelTxt1( rWrtShell.GetSelTxt() );
#endif
        rWrtShell.Delete();
        rWrtShell.Insert( rNewText );

        // select new inserted text (currently the Point is right after the new text)
        if (!rWrtShell.GetCrsr()->HasMark())
            rWrtShell.GetCrsr()->SetMark();
        SwPosition *pMark = rWrtShell.GetCrsr()->GetMark();
        pMark->nContent = pMark->nContent.GetIndex() - rNewText.Len();
#ifdef DEBUG
        String aSelTxt2( rWrtShell.GetSelTxt() );
#endif

        // since 'SetAttr' below functions like merging with the attributes
        // from the itemset with any existing ones we have to get rid of all
        // all attributes now. (Those attributes that may take effect left
        // to the position where the new text gets inserted after the old text
        // was deleted)
        rWrtShell.ResetAttr();
        // apply previously saved attributes to new text
        rWrtShell.SetAttr( aItemSet );
    }
    else
    {
        rWrtShell.Delete();
        rWrtShell.Insert( rNewText );
    }
}


void SwHHCWrapper::ReplaceUnit(
         const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
         const ::rtl::OUString& rOrigText,
         const OUString& rReplaceWith,
         const uno::Sequence< sal_Int32 > &rOffsets,
         ReplacementAction eAction,
         LanguageType *pNewUnitLanguage )
{
    static OUString aBracketedStart( C2U( "(" ) );
    static OUString aBracketedEnd( C2U( ")" ) );

    OSL_ENSURE( nUnitStart >= 0 && nUnitEnd >= nUnitStart, "wrong arguments" );
    if (!(nUnitStart >= 0 && nUnitEnd >= nUnitStart))
        return;

    lcl_ActivateTextShell( rWrtShell );

    // Das aktuelle Wort austauschen
    rWrtShell.StartAllAction();

    // select current unit
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    OUString aOrigTxt( rWrtShell.GetSelTxt() );
    OUString aNewTxt( rReplaceWith );
    OSL_ENSURE( aOrigTxt == rOrigText, "!! text mismatch !!" );
    SwFmtRuby *pRuby = 0;
    sal_Bool bRubyBelow = sal_False;
    String  aNewOrigText;
    switch (eAction)
    {
        case eExchange :
        break;
        case eReplacementBracketed :
        {
            (((aNewTxt = aOrigTxt) += aBracketedStart) += rReplaceWith) += aBracketedEnd;
        }
        break;
        case eOriginalBracketed :
        {
            (((aNewTxt = rReplaceWith) += aBracketedStart) += aOrigTxt) += aBracketedEnd;
        }
        break;
        case eReplacementAbove  :
        {
            pRuby = new SwFmtRuby( rReplaceWith );
        }
        break;
        case eOriginalAbove :
        {
            pRuby = new SwFmtRuby( aOrigTxt );
            aNewOrigText = rReplaceWith;
        }
        break;
        case eReplacementBelow :
        {
            pRuby = new SwFmtRuby( rReplaceWith );
            bRubyBelow = sal_True;
        }
        break;
        case eOriginalBelow :
        {
            pRuby = new SwFmtRuby( aOrigTxt );
            aNewOrigText = rReplaceWith;
            bRubyBelow = sal_True;
        }
        break;
        default:
            OSL_FAIL("unexpected case" );
    }
    nUnitOffset += nUnitStart + aNewTxt.getLength();

    if (pRuby)
    {
        rWrtShell.StartUndo( UNDO_SETRUBYATTR );
        if (aNewOrigText.Len())
        {
            // according to FT we currently should not bother about keeping
            // attributes in Hangul/Hanja conversion
            ChangeText( aNewOrigText, rOrigText, NULL, NULL );

            //!! since Delete, Insert in 'ChangeText' do not set the WrtShells
            //!! bInSelect flag
            //!! back to false we do it now manually in order for the selection
            //!! to be done properly in the following call to Left.
            // We didn't fix it in Delete and Insert since it is currently
            // unclear if someone depends on this incorrect behvaiour
            // of the flag.
            rWrtShell.EndSelect();

            rWrtShell.Left( 0, sal_True, aNewOrigText.Len(), sal_True, sal_True );
        }

        pRuby->SetPosition( bRubyBelow );
        pRuby->SetAdjustment( RubyAdjust_CENTER );

#ifdef DEBUG
        SwPaM *pPaM = rWrtShell.GetCrsr();
        (void)pPaM;
#endif
        rWrtShell.SetAttr(*pRuby);
        delete pRuby;
        rWrtShell.EndUndo( UNDO_SETRUBYATTR );
    }
    else
    {
        rWrtShell.StartUndo( UNDO_OVERWRITE );

        // according to FT we should currently not bother about keeping
        // attributes in Hangul/Hanja conversion and leave that untouched.
        // Thus we do this only for Chinese translation...
        sal_Bool bIsChineseConversion = IsChinese( GetSourceLanguage() );
        if (bIsChineseConversion)
            ChangeText( aNewTxt, rOrigText, &rOffsets, rWrtShell.GetCrsr() );
        else
            ChangeText( aNewTxt, rOrigText, NULL, NULL );

        // change language and font if necessary
        if (bIsChineseConversion)
        {
            rWrtShell.SetMark();
            rWrtShell.GetCrsr()->GetMark()->nContent -= (xub_StrLen) aNewTxt.getLength();

            OSL_ENSURE( GetTargetLanguage() == LANGUAGE_CHINESE_SIMPLIFIED || GetTargetLanguage() == LANGUAGE_CHINESE_TRADITIONAL,
                    "SwHHCWrapper::ReplaceUnit : unexpected target language" );

            sal_uInt16 aRanges[] = {
                    RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
                    RES_CHRATR_CJK_FONT,     RES_CHRATR_CJK_FONT,
                    0, 0, 0  };

            SfxItemSet aSet( rWrtShell.GetAttrPool(), aRanges );
            if (pNewUnitLanguage)
            {
                aSet.Put( SvxLanguageItem( *pNewUnitLanguage, RES_CHRATR_CJK_LANGUAGE ) );
            }

            const Font *pTargetFont = GetTargetFont();
            OSL_ENSURE( pTargetFont, "target font missing?" );
            if (pTargetFont && pNewUnitLanguage)
            {
                SvxFontItem aFontItem = (SvxFontItem&) aSet.Get( RES_CHRATR_CJK_FONT );
                aFontItem.SetFamilyName(    pTargetFont->GetName());
                aFontItem.SetFamily(        pTargetFont->GetFamily());
                aFontItem.SetStyleName(     pTargetFont->GetStyleName());
                aFontItem.SetPitch(         pTargetFont->GetPitch());
                aFontItem.SetCharSet( pTargetFont->GetCharSet() );
                aSet.Put( aFontItem );
            }

            rWrtShell.SetAttr( aSet );

            rWrtShell.ClearMark();
        }

        rWrtShell.EndUndo( UNDO_OVERWRITE );
    }

    rWrtShell.EndAllAction();
}


sal_Bool SwHHCWrapper::HasRubySupport() const
{
    return sal_True;
}


void SwHHCWrapper::Convert()
{
    OSL_ENSURE( pConvArgs == 0, "NULL pointer expected" );
    {
        SwPaM *pCrsr = pView->GetWrtShell().GetCrsr();
        SwPosition* pSttPos = pCrsr->Start();
        SwPosition* pEndPos = pCrsr->End();


        if (pSttPos->nNode.GetNode().IsTxtNode() &&
            pEndPos->nNode.GetNode().IsTxtNode())
        {
            pConvArgs = new SwConversionArgs( GetSourceLanguage(),
                            pSttPos->nNode.GetNode().GetTxtNode(), pSttPos->nContent,
                            pEndPos->nNode.GetNode().GetTxtNode(), pEndPos->nContent );
        }
        else    // we are not in the text (maybe a graphic or OLE object is selected) let's start from the top
        {
            // get PaM that points to the start of the document
            SwNode& rNode = pView->GetDocShell()->GetDoc()->GetNodes().GetEndOfContent();
            SwPaM aPam(rNode);
            aPam.Move( fnMoveBackward, fnGoDoc ); // move to start of document

            pSttPos = aPam.GetPoint();  //! using a PaM here makes sure we will get only text nodes
            SwTxtNode *pTxtNode = pSttPos->nNode.GetNode().GetTxtNode();
            // just in case we check anyway...
            if (!pTxtNode || !pTxtNode->IsTxtNode())
                return;
            pConvArgs = new SwConversionArgs( GetSourceLanguage(),
                            pTxtNode, pSttPos->nContent,
                            pTxtNode, pSttPos->nContent );
        }
        OSL_ENSURE( pConvArgs->pStartNode && pConvArgs->pStartNode->IsTxtNode(),
                "failed to get proper start text node" );
        OSL_ENSURE( pConvArgs->pEndNode && pConvArgs->pEndNode->IsTxtNode(),
                "failed to get proper end text node" );

        // chinese conversion specific settings
        OSL_ENSURE( IsChinese( GetSourceLanguage() ) == IsChinese( GetTargetLanguage() ),
                "source and target language mismatch?" );
        if (IsChinese( GetTargetLanguage() ))
        {
            pConvArgs->nConvTargetLang = GetTargetLanguage();
            pConvArgs->pTargetFont = GetTargetFont();
            pConvArgs->bAllowImplicitChangesForNotConvertibleText = sal_True;
        }

        // if it is not just a selection and we are about to begin
        // with the current conversion for the very first time
        // we need to find the start of the current (initial)
        // convertible unit in order for the text conversion to give
        // the correct result for that. Since it is easier to obtain
        // the start of the word we use that though.
        if (!pCrsr->HasMark())   // is not a selection?
        {
            // since #118246 / #117803 still occurs if the cursor is placed
            // between the two chinese characters to be converted (because both
            // of them are words on their own!) using the word boundary here does
            // not work. Thus since chinese conversion is not interactive we start
            // at the begin of the paragraph to solve the problem, i.e. have the
            // TextConversion service get those charcters together in the same call.
            xub_StrLen nStartIdx = STRING_MAXLEN;
            if (editeng::HangulHanjaConversion::IsChinese( GetSourceLanguage() ) )
                nStartIdx = 0;
            else
            {
                OUString aText( pConvArgs->pStartNode->GetTxt() );
                long     nPos = pConvArgs->pStartIdx->GetIndex();
                Boundary aBoundary( pBreakIt->GetBreakIter()->
                        getWordBoundary( aText, nPos, pBreakIt->GetLocale( pConvArgs->nConvSrcLang ),
                                WordType::DICTIONARY_WORD, sal_True ) );

                // valid result found?
                if (aBoundary.startPos < aText.getLength() &&
                    aBoundary.startPos != aBoundary.endPos)
                {
                    nStartIdx = static_cast< xub_StrLen >(aBoundary.startPos );
                }
            }

            if (STRING_MAXLEN != nStartIdx)
                *pConvArgs->pStartIdx = nStartIdx;
        }
    }

    if ( bIsOtherCntnt )
        ConvStart_impl( pConvArgs, SVX_SPELL_OTHER );
    else
    {
        bStartChk = sal_False;
        ConvStart_impl( pConvArgs,  SVX_SPELL_BODY_END );
    }

    ConvertDocument();

    ConvEnd_impl( pConvArgs );
}


sal_Bool SwHHCWrapper::ConvNext_impl( )
{
    //! modified version of SvxSpellWrapper::SpellNext

    // Keine Richtungsaenderung, also ist der gewuenschte Bereich ( bStartChk )
    // vollstaendig abgearbeitet.
    if( bStartChk )
        bStartDone = sal_True;
    else
        bEndDone = sal_True;

    if( bIsOtherCntnt && bStartDone && bEndDone ) // Dokument komplett geprueft?
    {
        bInfoBox = sal_True;
        return sal_False;
    }

    sal_Bool bGoOn = sal_False;

    if ( bIsOtherCntnt )
    {
        bStartChk = sal_False;
        ConvStart_impl( pConvArgs, SVX_SPELL_BODY );
        bGoOn = sal_True;
    }
    else if ( bStartDone && bEndDone )
    {
        // Bodybereich erledigt, Frage nach Sonderbereich
        if( bIsConvSpecial && HasOtherCnt_impl() )
        {
            ConvStart_impl( pConvArgs, SVX_SPELL_OTHER );
            bIsOtherCntnt = bGoOn = sal_True;
        }
        else
            bInfoBox = sal_True;
    }
    else
    {
            bStartChk = !bStartDone;
            ConvStart_impl( pConvArgs, bStartChk ? SVX_SPELL_BODY_START : SVX_SPELL_BODY_END );
            bGoOn = sal_True;
    }
    return bGoOn;
}


sal_Bool SwHHCWrapper::FindConvText_impl()
{
    //! modified version of SvxSpellWrapper::FindSpellError

    sal_Bool bFound = sal_False;

    pWin->EnterWait();
    sal_Bool bConv = sal_True;

    while ( bConv )
    {
        bFound = ConvContinue_impl( pConvArgs );
        if (bFound)
        {
            bConv = sal_False;
        }
        else
        {
            ConvEnd_impl( pConvArgs );
            bConv = ConvNext_impl();
        }
    }
    pWin->LeaveWait();
    return bFound;
}


sal_Bool SwHHCWrapper::HasOtherCnt_impl()
{
    return bIsSelection ? sal_False : rWrtShell.HasOtherCnt();
}


void SwHHCWrapper::ConvStart_impl( SwConversionArgs /* [out] */ *pConversionArgs, SvxSpellArea eArea )
{
    SetDrawObj( SVX_SPELL_OTHER == eArea );
    pView->SpellStart( eArea, bStartDone, bEndDone, /* [out] */ pConversionArgs );
}


void SwHHCWrapper::ConvEnd_impl( SwConversionArgs *pConversionArgs )
{
    pView->SpellEnd( pConversionArgs );
}


sal_Bool SwHHCWrapper::ConvContinue_impl( SwConversionArgs *pConversionArgs )
{
    sal_Bool bProgress = !bIsDrawObj && !bIsSelection;
    pConversionArgs->aConvText = OUString();
    pConversionArgs->nConvTextLang = LANGUAGE_NONE;
    uno::Any  aRet = bProgress ?
        pView->GetWrtShell().SpellContinue( &nPageCount, &nPageStart, pConversionArgs ) :
        pView->GetWrtShell().SpellContinue( &nPageCount, NULL, pConversionArgs );
    return pConversionArgs->aConvText.getLength() != 0;
}

//////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
