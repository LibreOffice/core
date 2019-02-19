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

#include <hintids.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>
#include <globals.hrc>
#include <splargs.hxx>

#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fontitem.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <hhcwrp.hxx>
#include "sdrhhcwrap.hxx"
#include <doc.hxx>
#include <docsh.hxx>
#include <mdiexp.hxx>
#include <edtwin.hxx>
#include <index.hxx>
#include <pam.hxx>
#include <swcrsr.hxx>
#include <viscrs.hxx>
#include <ndtxt.hxx>
#include <fmtruby.hxx>
#include <breakit.hxx>

#include <olmenu.hxx>

#include <unomid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;

//     Description: Turn off frame/object shell if applicable

static void lcl_ActivateTextShell( SwWrtShell & rWrtSh )
{
    if( rWrtSh.IsSelFrameMode() || rWrtSh.IsObjSelected() )
        rWrtSh.EnterStdMode();
}

class SwKeepConversionDirectionStateContext
{
public:
    SwKeepConversionDirectionStateContext()
    {
        //!! hack to transport the current conversion direction state settings
        //!! into the next incarnation that iterates over the drawing objects
        //!! ( see SwHHCWrapper::~SwHHCWrapper() )
        editeng::HangulHanjaConversion::SetUseSavedConversionDirectionState( true );
    }

    ~SwKeepConversionDirectionStateContext()
    {
        editeng::HangulHanjaConversion::SetUseSavedConversionDirectionState( false );
    }
};

SwHHCWrapper::SwHHCWrapper(
        SwView* pSwView,
        const uno::Reference< uno::XComponentContext >& rxContext,
        LanguageType nSourceLanguage,
        LanguageType nTargetLanguage,
        const vcl::Font *pTargetFont,
        sal_Int32 nConvOptions,
        bool bIsInteractive,
        bool bStart, bool bOther, bool bSelection )
    : editeng::HangulHanjaConversion(pSwView->GetEditWin().GetFrameWeld(), rxContext,
                                LanguageTag::convertToLocale( nSourceLanguage ),
                                LanguageTag::convertToLocale( nTargetLanguage ),
                                pTargetFont,
                                nConvOptions,
                                bIsInteractive )
    , m_pView( pSwView )
    , m_pWin( &pSwView->GetEditWin() )
    , m_rWrtShell( pSwView->GetWrtShell() )
    , m_nLastPos( 0 )
    , m_nUnitOffset( 0 )
    , m_nPageCount( 0 )
    , m_nPageStart( 0 )
    , m_bIsDrawObj( false )
    , m_bIsOtherContent( bOther )
    , m_bStartChk( bOther )
    , m_bIsSelection( bSelection )
    , m_bStartDone( bOther || bStart )
    , m_bEndDone( false )
{
}

SwHHCWrapper::~SwHHCWrapper()
{
    m_pConvArgs.reset();

    SwViewShell::SetCareWin( nullptr );

    // check for existence of a draw view which means that there are
    // (or previously were) draw objects present in the document.
    // I.e. we like to check those too.
    if ( m_bIsDrawObj /*&& bLastRet*/ && m_pView->GetWrtShell().HasDrawView() )
    {
        vcl::Cursor *pSave = m_pView->GetWindow()->GetCursor();
        {
            SwKeepConversionDirectionStateContext aContext;

            SdrHHCWrapper aSdrConvWrap( m_pView, GetSourceLanguage(),
                    GetTargetLanguage(), GetTargetFont(),
                    GetConversionOptions(), IsInteractive() );
            aSdrConvWrap.StartTextConversion();
        }
        m_pView->GetWindow()->SetCursor( pSave );
    }

    if( m_nPageCount )
        ::EndProgress( m_pView->GetDocShell() );

    // finally for chinese translation we need to change the documents
    // default language and font to the new ones to be used.
    LanguageType nTargetLang = GetTargetLanguage();
    if (IsChinese( nTargetLang ))
    {
        SwDoc *pDoc = m_pView->GetDocShell()->GetDoc();

        //!! Note: This also effects the default language of text boxes (EditEngine/EditView) !!
        pDoc->SetDefault( SvxLanguageItem( nTargetLang, RES_CHRATR_CJK_LANGUAGE ) );

        const vcl::Font *pFont = GetTargetFont();
        if (pFont)
        {
            SvxFontItem aFontItem( pFont->GetFamilyType(), pFont->GetFamilyName(),
                    pFont->GetStyleName(), pFont->GetPitch(),
                    pFont->GetCharSet(), RES_CHRATR_CJK_FONT );
            pDoc->SetDefault( aFontItem );
        }

    }
}

void SwHHCWrapper::GetNextPortion(
        OUString&           rNextPortion,
        LanguageType&       rLangOfPortion,
        bool bAllowChanges )
{
    m_pConvArgs->bAllowImplicitChangesForNotConvertibleText = bAllowChanges;

    FindConvText_impl();
    rNextPortion    = m_pConvArgs->aConvText;
    rLangOfPortion  = m_pConvArgs->nConvTextLang;

    m_nUnitOffset  = 0;

    // build last pos from currently selected text
    SwPaM* pCursor = m_rWrtShell.GetCursor();
    m_nLastPos =  pCursor->Start()->nContent.GetIndex();
}

void SwHHCWrapper::SelectNewUnit_impl( sal_Int32 nUnitStart, sal_Int32 nUnitEnd )
{
    SwPaM *pCursor = m_rWrtShell.GetCursor();
    pCursor->GetPoint()->nContent = m_nLastPos;
    pCursor->DeleteMark();

    m_rWrtShell.Right( CRSR_SKIP_CHARS, /*bExpand*/ false,
                  static_cast<sal_uInt16>(m_nUnitOffset + nUnitStart), true );
    pCursor->SetMark();
    m_rWrtShell.Right( CRSR_SKIP_CHARS, /*bExpand*/ true,
                  static_cast<sal_uInt16>(nUnitEnd - nUnitStart), true );
    // end selection now. Otherwise SHIFT+HOME (extending the selection)
    // won't work when the dialog is closed without any replacement.
    // (see #116346#)
    m_rWrtShell.EndSelect();
}

void SwHHCWrapper::HandleNewUnit(
        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd )
{
    OSL_ENSURE( nUnitStart >= 0 && nUnitEnd >= nUnitStart, "wrong arguments" );
    if (!(0 <= nUnitStart && nUnitStart <= nUnitEnd))
        return;

    lcl_ActivateTextShell( m_rWrtShell );

    m_rWrtShell.StartAllAction();

    // select current unit
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    m_rWrtShell.EndAllAction();
}

void SwHHCWrapper::ChangeText( const OUString &rNewText,
        const OUString& rOrigText,
        const uno::Sequence< sal_Int32 > *pOffsets,
        SwPaM *pCursor )
{
    //!! please see also TextConvWrapper::ChangeText with is a modified
    //!! copy of this code

    OSL_ENSURE( !rNewText.isEmpty(), "unexpected empty string" );
    if (rNewText.isEmpty())
        return;

    if (pOffsets && pCursor)  // try to keep as much attributation as possible ?
    {
        // remember cursor start position for later setting of the cursor
        const SwPosition *pStart = pCursor->Start();
        const sal_Int32 nStartIndex = pStart->nContent.GetIndex();
        const SwNodeIndex aStartNodeIndex  = pStart->nNode;
        SwTextNode *pStartTextNode = aStartNodeIndex.GetNode().GetTextNode();

        const sal_Int32  nIndices = pOffsets->getLength();
        const sal_Int32 *pIndices = pOffsets->getConstArray();
        sal_Int32 nConvTextLen = rNewText.getLength();
        sal_Int32 nPos = 0;
        sal_Int32 nChgPos = -1;
        sal_Int32 nChgLen = 0;
        sal_Int32 nConvChgPos = -1;
        sal_Int32 nConvChgLen = 0;

        // offset to calculate the position in the text taking into
        // account that text may have been replaced with new text of
        // different length. Negative values allowed!
        long nCorrectionOffset = 0;

        OSL_ENSURE(nIndices == 0 || nIndices == nConvTextLen,
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

            if (nPos == nConvTextLen || /* end of string also terminates non-matching char sequence */
                rOrigText[nIndex] == rNewText[nPos])
            {
                // substring that needs to be replaced found?
                if (nChgPos != -1 && nConvChgPos != -1)
                {
                    nChgLen = nIndex - nChgPos;
                    nConvChgLen = nPos - nConvChgPos;
                    OUString aInNew( rNewText.copy( nConvChgPos, nConvChgLen ) );

                    // set selection to sub string to be replaced in original text
                    sal_Int32 nChgInNodeStartIndex = nStartIndex + nCorrectionOffset + nChgPos;
                    OSL_ENSURE( m_rWrtShell.GetCursor()->HasMark(), "cursor misplaced (nothing selected)" );
                    m_rWrtShell.GetCursor()->GetMark()->nContent.Assign( pStartTextNode, nChgInNodeStartIndex );
                    m_rWrtShell.GetCursor()->GetPoint()->nContent.Assign( pStartTextNode, nChgInNodeStartIndex + nChgLen );

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
                if (nChgPos == -1 && nConvChgPos == -1)
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
        m_rWrtShell.ClearMark();
        m_rWrtShell.GetCursor()->Start()->nContent.Assign( pStartTextNode, nStartIndex + nConvTextLen );
    }
    else
    {
        ChangeText_impl( rNewText, false );
    }
}

void SwHHCWrapper::ChangeText_impl( const OUString &rNewText, bool bKeepAttributes )
{
    if (bKeepAttributes)
    {
        // get item set with all relevant attributes
        sal_uInt16 const aRanges[] {
                RES_CHRATR_BEGIN, RES_FRMATR_END,
                0, 0, 0  };
        SfxItemSet aItemSet( m_rWrtShell.GetAttrPool(), aRanges );
        // get all attributes spanning the whole selection in order to
        // restore those for the new text
        m_rWrtShell.GetCurAttr( aItemSet );

        m_rWrtShell.Delete();
        m_rWrtShell.Insert( rNewText );

        // select new inserted text (currently the Point is right after the new text)
        if (!m_rWrtShell.GetCursor()->HasMark())
            m_rWrtShell.GetCursor()->SetMark();
        SwPosition *pMark = m_rWrtShell.GetCursor()->GetMark();
        pMark->nContent = pMark->nContent.GetIndex() - rNewText.getLength();

        // since 'SetAttr' below functions like merging with the attributes
        // from the itemset with any existing ones we have to get rid of all
        // all attributes now. (Those attributes that may take effect left
        // to the position where the new text gets inserted after the old text
        // was deleted)
        m_rWrtShell.ResetAttr();
        // apply previously saved attributes to new text
        m_rWrtShell.SetAttrSet( aItemSet );
    }
    else
    {
        m_rWrtShell.Delete();
        m_rWrtShell.Insert( rNewText );
    }
}

void SwHHCWrapper::ReplaceUnit(
         const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
         const OUString& rOrigText,
         const OUString& rReplaceWith,
         const uno::Sequence< sal_Int32 > &rOffsets,
         ReplacementAction eAction,
         LanguageType *pNewUnitLanguage )
{
    OSL_ENSURE( nUnitStart >= 0 && nUnitEnd >= nUnitStart, "wrong arguments" );
    if (!(nUnitStart >= 0 && nUnitEnd >= nUnitStart))
        return;

    lcl_ActivateTextShell( m_rWrtShell );

    // replace the current word
    m_rWrtShell.StartAllAction();

    // select current unit
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    OUString aOrigText( m_rWrtShell.GetSelText() );
    OUString aNewText( rReplaceWith );
    OSL_ENSURE( aOrigText == rOrigText, "!! text mismatch !!" );
    std::unique_ptr<SwFormatRuby> pRuby;
    bool bRubyBelow = false;
    OUString  aNewOrigText;
    switch (eAction)
    {
        case eExchange :
        break;
        case eReplacementBracketed :
        {
            aNewText = aOrigText + "(" + rReplaceWith + ")";
        }
        break;
        case eOriginalBracketed :
        {
            aNewText = rReplaceWith + "(" + aOrigText + ")";
        }
        break;
        case eReplacementAbove  :
        {
            pRuby.reset(new SwFormatRuby( rReplaceWith ));
        }
        break;
        case eOriginalAbove :
        {
            pRuby.reset(new SwFormatRuby( aOrigText ));
            aNewOrigText = rReplaceWith;
        }
        break;
        case eReplacementBelow :
        {
            pRuby.reset(new SwFormatRuby( rReplaceWith ));
            bRubyBelow = true;
        }
        break;
        case eOriginalBelow :
        {
            pRuby.reset(new SwFormatRuby( aOrigText ));
            aNewOrigText = rReplaceWith;
            bRubyBelow = true;
        }
        break;
        default:
            OSL_FAIL("unexpected case" );
    }
    m_nUnitOffset += nUnitStart + aNewText.getLength();

    if (pRuby)
    {
        m_rWrtShell.StartUndo( SwUndoId::SETRUBYATTR );
        if (!aNewOrigText.isEmpty())
        {
            // according to FT we currently should not bother about keeping
            // attributes in Hangul/Hanja conversion
            ChangeText( aNewOrigText, rOrigText, nullptr, nullptr );

            //!! since Delete, Insert in 'ChangeText' do not set the WrtShells
            //!! bInSelect flag
            //!! back to false we do it now manually in order for the selection
            //!! to be done properly in the following call to Left.
            // We didn't fix it in Delete and Insert since it is currently
            // unclear if someone depends on this incorrect behaviour
            // of the flag.
            m_rWrtShell.EndSelect();

            m_rWrtShell.Left( 0, true, aNewOrigText.getLength(), true, true );
        }

        pRuby->SetPosition( static_cast<sal_uInt16>(bRubyBelow) );
        pRuby->SetAdjustment( RubyAdjust_CENTER );

        m_rWrtShell.SetAttrItem(*pRuby);
        pRuby.reset();
        m_rWrtShell.EndUndo( SwUndoId::SETRUBYATTR );
    }
    else
    {
        m_rWrtShell.StartUndo( SwUndoId::OVERWRITE );

        // according to FT we should currently not bother about keeping
        // attributes in Hangul/Hanja conversion and leave that untouched.
        // Thus we do this only for Chinese translation...
        const bool bIsChineseConversion = IsChinese( GetSourceLanguage() );
        if (bIsChineseConversion)
            ChangeText( aNewText, rOrigText, &rOffsets, m_rWrtShell.GetCursor() );
        else
            ChangeText( aNewText, rOrigText, nullptr, nullptr );

        // change language and font if necessary
        if (bIsChineseConversion)
        {
            m_rWrtShell.SetMark();
            m_rWrtShell.GetCursor()->GetMark()->nContent -= aNewText.getLength();

            OSL_ENSURE( GetTargetLanguage() == LANGUAGE_CHINESE_SIMPLIFIED || GetTargetLanguage() == LANGUAGE_CHINESE_TRADITIONAL,
                    "SwHHCWrapper::ReplaceUnit : unexpected target language" );

            sal_uInt16 const aRanges[] {
                    RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
                    RES_CHRATR_CJK_FONT,     RES_CHRATR_CJK_FONT,
                    0, 0, 0  };

            SfxItemSet aSet( m_rWrtShell.GetAttrPool(), aRanges );
            if (pNewUnitLanguage)
            {
                aSet.Put( SvxLanguageItem( *pNewUnitLanguage, RES_CHRATR_CJK_LANGUAGE ) );
            }

            const vcl::Font *pTargetFont = GetTargetFont();
            OSL_ENSURE( pTargetFont, "target font missing?" );
            if (pTargetFont && pNewUnitLanguage)
            {
                SvxFontItem aFontItem( aSet.Get( RES_CHRATR_CJK_FONT ) );
                aFontItem.SetFamilyName(    pTargetFont->GetFamilyName());
                aFontItem.SetFamily(        pTargetFont->GetFamilyType());
                aFontItem.SetStyleName(     pTargetFont->GetStyleName());
                aFontItem.SetPitch(         pTargetFont->GetPitch());
                aFontItem.SetCharSet( pTargetFont->GetCharSet() );
                aSet.Put( aFontItem );
            }

            m_rWrtShell.SetAttrSet( aSet );

            m_rWrtShell.ClearMark();
        }

        m_rWrtShell.EndUndo( SwUndoId::OVERWRITE );
    }

    m_rWrtShell.EndAllAction();
}

bool SwHHCWrapper::HasRubySupport() const
{
    return true;
}

void SwHHCWrapper::Convert()
{
    OSL_ENSURE( m_pConvArgs == nullptr, "NULL pointer expected" );
    {
        SwPaM *pCursor = m_pView->GetWrtShell().GetCursor();
        SwPosition* pSttPos = pCursor->Start();
        SwPosition* pEndPos = pCursor->End();

        if (pSttPos->nNode.GetNode().IsTextNode() &&
            pEndPos->nNode.GetNode().IsTextNode())
        {
            m_pConvArgs.reset( new SwConversionArgs( GetSourceLanguage(),
                            pSttPos->nNode.GetNode().GetTextNode(), pSttPos->nContent,
                            pEndPos->nNode.GetNode().GetTextNode(), pEndPos->nContent ) );
        }
        else    // we are not in the text (maybe a graphic or OLE object is selected) let's start from the top
        {
            // get PaM that points to the start of the document
            SwNode& rNode = m_pView->GetDocShell()->GetDoc()->GetNodes().GetEndOfContent();
            SwPaM aPam(rNode);
            aPam.Move( fnMoveBackward, GoInDoc ); // move to start of document

            pSttPos = aPam.GetPoint();  //! using a PaM here makes sure we will get only text nodes
            SwTextNode *pTextNode = pSttPos->nNode.GetNode().GetTextNode();
            // just in case we check anyway...
            if (!pTextNode || !pTextNode->IsTextNode())
                return;
            m_pConvArgs.reset( new SwConversionArgs( GetSourceLanguage(),
                            pTextNode, pSttPos->nContent,
                            pTextNode, pSttPos->nContent ) );
        }
        OSL_ENSURE( m_pConvArgs->pStartNode && m_pConvArgs->pStartNode->IsTextNode(),
                "failed to get proper start text node" );
        OSL_ENSURE( m_pConvArgs->pEndNode && m_pConvArgs->pEndNode->IsTextNode(),
                "failed to get proper end text node" );

        // chinese conversion specific settings
        OSL_ENSURE( IsChinese( GetSourceLanguage() ) == IsChinese( GetTargetLanguage() ),
                "source and target language mismatch?" );
        if (IsChinese( GetTargetLanguage() ))
        {
            m_pConvArgs->nConvTargetLang = GetTargetLanguage();
            m_pConvArgs->pTargetFont = GetTargetFont();
            m_pConvArgs->bAllowImplicitChangesForNotConvertibleText = true;
        }

        // if it is not just a selection and we are about to begin
        // with the current conversion for the very first time
        // we need to find the start of the current (initial)
        // convertible unit in order for the text conversion to give
        // the correct result for that. Since it is easier to obtain
        // the start of the word we use that though.
        if (!pCursor->HasMark())   // is not a selection?
        {
            // since #118246 / #117803 still occurs if the cursor is placed
            // between the two chinese characters to be converted (because both
            // of them are words on their own!) using the word boundary here does
            // not work. Thus since chinese conversion is not interactive we start
            // at the begin of the paragraph to solve the problem, i.e. have the
            // TextConversion service get those characters together in the same call.
            sal_Int32 nStartIdx = -1;
            if (editeng::HangulHanjaConversion::IsChinese( GetSourceLanguage() ) )
                nStartIdx = 0;
            else
            {
                OUString aText( m_pConvArgs->pStartNode->GetText() );
                const sal_Int32 nPos = m_pConvArgs->pStartIdx->GetIndex();
                Boundary aBoundary( g_pBreakIt->GetBreakIter()->
                        getWordBoundary( aText, nPos, g_pBreakIt->GetLocale( m_pConvArgs->nConvSrcLang ),
                                WordType::DICTIONARY_WORD, true ) );

                // valid result found?
                if (aBoundary.startPos < aText.getLength() &&
                    aBoundary.startPos != aBoundary.endPos)
                {
                    nStartIdx = aBoundary.startPos;
                }
            }

            if (nStartIdx != -1)
                *m_pConvArgs->pStartIdx = nStartIdx;
        }
    }

    if ( m_bIsOtherContent )
        ConvStart_impl( m_pConvArgs.get(), SvxSpellArea::Other );
    else
    {
        m_bStartChk = false;
        ConvStart_impl( m_pConvArgs.get(), SvxSpellArea::BodyEnd );
    }

    ConvertDocument();

    ConvEnd_impl( m_pConvArgs.get() );
}

bool SwHHCWrapper::ConvNext_impl( )
{
    //! modified version of SvxSpellWrapper::SpellNext

    // no change of direction so the desired region is fully processed
    if( m_bStartChk )
        m_bStartDone = true;
    else
        m_bEndDone = true;

    if( m_bIsOtherContent && m_bStartDone && m_bEndDone ) // document completely checked?
    {
        return false;
    }

    bool bGoOn = false;

    if ( m_bIsOtherContent )
    {
        m_bStartChk = false;
        ConvStart_impl( m_pConvArgs.get(), SvxSpellArea::Body );
        bGoOn = true;
    }
    else if ( m_bStartDone && m_bEndDone )
    {
        // body region done, ask about special region
        if( !m_bIsSelection && m_rWrtShell.HasOtherCnt() )
        {
            ConvStart_impl( m_pConvArgs.get(), SvxSpellArea::Other );
            m_bIsOtherContent = bGoOn = true;
        }
    }
    else
    {
            m_bStartChk = !m_bStartDone;
            ConvStart_impl( m_pConvArgs.get(), m_bStartChk ? SvxSpellArea::BodyStart : SvxSpellArea::BodyEnd );
            bGoOn = true;
    }
    return bGoOn;
}

void SwHHCWrapper::FindConvText_impl()
{
    //! modified version of SvxSpellWrapper::FindSpellError

    bool bFound = false;

    m_pWin->EnterWait();
    bool bConv = true;

    while ( bConv )
    {
        bFound = ConvContinue_impl( m_pConvArgs.get() );
        if (bFound)
        {
            bConv = false;
        }
        else
        {
            ConvEnd_impl( m_pConvArgs.get() );
            bConv = ConvNext_impl();
        }
    }
    m_pWin->LeaveWait();
}

void SwHHCWrapper::ConvStart_impl( SwConversionArgs /* [out] */ *pConversionArgs, SvxSpellArea eArea )
{
    m_bIsDrawObj = SvxSpellArea::Other == eArea;
    m_pView->SpellStart( eArea, m_bStartDone, m_bEndDone, /* [out] */ pConversionArgs );
}

void SwHHCWrapper::ConvEnd_impl( SwConversionArgs const *pConversionArgs )
{
    m_pView->SpellEnd( pConversionArgs );
}

bool SwHHCWrapper::ConvContinue_impl( SwConversionArgs *pConversionArgs )
{
    bool bProgress = !m_bIsDrawObj && !m_bIsSelection;
    pConversionArgs->aConvText.clear();
    pConversionArgs->nConvTextLang = LANGUAGE_NONE;
    m_pView->GetWrtShell().SpellContinue( &m_nPageCount, bProgress ? &m_nPageStart : nullptr, pConversionArgs );
    return !pConversionArgs->aConvText.isEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
