/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <hintids.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>
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
#include <mdiexp.hxx>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;




static void lcl_ActivateTextShell( SwWrtShell & rWrtSh )
{
    if( rWrtSh.IsSelFrmMode() || rWrtSh.IsObjSelected() )
        rWrtSh.EnterStdMode();
}



class SwKeepConversionDirectionStateContext
{
public:
    SwKeepConversionDirectionStateContext()
    {
        
        
        
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
        const Font *pTargetFont,
        sal_Int32 nConvOptions,
        bool bIsInteractive,
        bool bStart, bool bOther, bool bSelection )
    : editeng::HangulHanjaConversion( &pSwView->GetEditWin(), rxContext,
                                LanguageTag::convertToLocale( nSourceLanguage ),
                                LanguageTag::convertToLocale( nTargetLanguage ),
                                pTargetFont,
                                nConvOptions,
                                bIsInteractive )
    , m_pView( pSwView )
    , m_pWin( &pSwView->GetEditWin() )
    , m_rWrtShell( pSwView->GetWrtShell() )
    , m_pConvArgs( 0 )
    , m_nLastPos( 0 )
    , m_nUnitOffset( 0 )
    , m_nPageCount( 0 )
    , m_nPageStart( 0 )
    , m_bIsDrawObj( false )
    , m_bIsOtherCntnt( bOther )
    , m_bStartChk( bOther )
    , m_bIsSelection( bSelection )
    , m_bStartDone( bOther || bStart )
    , m_bEndDone( false )
{
}


SwHHCWrapper::~SwHHCWrapper()
{
    delete m_pConvArgs;

    m_rWrtShell.SetCareWin( NULL );

    
    
    
    if ( IsDrawObj() /*&& bLastRet*/ && m_pView->GetWrtShell().HasDrawView() )
    {
        Cursor *pSave = m_pView->GetWindow()->GetCursor();
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

    
    
    LanguageType nTargetLang = GetTargetLanguage();
    if (IsChinese( nTargetLang ))
    {
        SwDoc *pDoc = m_pView->GetDocShell()->GetDoc();

        
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
        OUString&           rNextPortion,
        LanguageType&       rLangOfPortion,
        bool bAllowChanges )
{
    m_pConvArgs->bAllowImplicitChangesForNotConvertibleText = bAllowChanges;

    FindConvText_impl();
    rNextPortion    = m_pConvArgs->aConvText;
    rLangOfPortion  = m_pConvArgs->nConvTextLang;

    m_nUnitOffset  = 0;

    
    SwPaM* pCrsr = m_rWrtShell.GetCrsr();
    m_nLastPos =  pCrsr->Start()->nContent.GetIndex();
}


void SwHHCWrapper::SelectNewUnit_impl( sal_Int32 nUnitStart, sal_Int32 nUnitEnd )
{
    SwPaM *pCrsr = m_rWrtShell.GetCrsr();
    pCrsr->GetPoint()->nContent = m_nLastPos;
    pCrsr->DeleteMark();

    m_rWrtShell.Right( CRSR_SKIP_CHARS, /*bExpand*/ sal_False,
                  (sal_uInt16) (m_nUnitOffset + nUnitStart), sal_True );
    pCrsr->SetMark();
    m_rWrtShell.Right( CRSR_SKIP_CHARS, /*bExpand*/ sal_True,
                  (sal_uInt16) (nUnitEnd - nUnitStart), sal_True );
    
    
    
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

    
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    m_rWrtShell.EndAllAction();
}


void SwHHCWrapper::ChangeText( const OUString &rNewText,
        const OUString& rOrigText,
        const uno::Sequence< sal_Int32 > *pOffsets,
        SwPaM *pCrsr )
{
    
    

    OSL_ENSURE( !rNewText.isEmpty(), "unexpected empty string" );
    if (rNewText.isEmpty())
        return;

    if (pOffsets && pCrsr)  
    {
        
        const SwPosition *pStart = pCrsr->Start();
        const sal_Int32 nStartIndex = pStart->nContent.GetIndex();
        const SwNodeIndex aStartNodeIndex  = pStart->nNode;
        SwTxtNode *pStartTxtNode = aStartNodeIndex.GetNode().GetTxtNode();

        const sal_Int32  nIndices = pOffsets->getLength();
        const sal_Int32 *pIndices = pOffsets->getConstArray();
        sal_Int32 nConvTextLen = rNewText.getLength();
        sal_Int32 nPos = 0;
        sal_Int32 nChgPos = -1;
        sal_Int32 nChgLen = 0;
        sal_Int32 nConvChgPos = -1;
        sal_Int32 nConvChgLen = 0;

        
        
        
        long nCorrectionOffset = 0;

        OSL_ENSURE(nIndices == 0 || nIndices == nConvTextLen,
                "mismatch between string length and sequence length!" );

        
        while (true)
        {
            
            sal_Int32 nIndex;
            if (nPos < nConvTextLen)
                nIndex = nPos < nIndices ? pIndices[nPos] : nPos;
            else
            {
                nPos   = nConvTextLen;
                nIndex = rOrigText.getLength();
            }

            if (rOrigText[nIndex] == rNewText[nPos] ||
                nPos == nConvTextLen /* end of string also terminates non-matching char sequence */)
            {
                
                if (nChgPos != -1 && nConvChgPos != -1)
                {
                    nChgLen = nIndex - nChgPos;
                    nConvChgLen = nPos - nConvChgPos;
#if OSL_DEBUG_LEVEL > 1
                    OUString aInOrig( rOrigText.copy( nChgPos, nChgLen ) );
#endif
                    OUString aInNew( rNewText.copy( nConvChgPos, nConvChgLen ) );

                    
                    sal_Int32 nChgInNodeStartIndex = nStartIndex + nCorrectionOffset + nChgPos;
                    OSL_ENSURE( m_rWrtShell.GetCrsr()->HasMark(), "cursor misplaced (nothing selected)" );
                    m_rWrtShell.GetCrsr()->GetMark()->nContent.Assign( pStartTxtNode, nChgInNodeStartIndex );
                    m_rWrtShell.GetCrsr()->GetPoint()->nContent.Assign( pStartTxtNode, nChgInNodeStartIndex + nChgLen );
#if OSL_DEBUG_LEVEL > 1
                    OUString aSelTxt1( m_rWrtShell.GetSelTxt() );
#endif

                    
                    
                    
                    ChangeText_impl( aInNew, true );

                    nCorrectionOffset += nConvChgLen - nChgLen;

                    nChgPos = -1;
                    nConvChgPos = -1;
                }
            }
            else
            {
                
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

        
        
        
        m_rWrtShell.ClearMark();
        m_rWrtShell.GetCrsr()->Start()->nContent.Assign( pStartTxtNode, nStartIndex + nConvTextLen );
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
        
        sal_uInt16 aRanges[] = {
                RES_CHRATR_BEGIN, RES_FRMATR_END,
                0, 0, 0  };
        SfxItemSet aItemSet( m_rWrtShell.GetAttrPool(), aRanges );
        
        
        m_rWrtShell.GetCurAttr( aItemSet );

#if OSL_DEBUG_LEVEL > 1
        OUString aSelTxt1( m_rWrtShell.GetSelTxt() );
#endif
        m_rWrtShell.Delete();
        m_rWrtShell.Insert( rNewText );

        
        if (!m_rWrtShell.GetCrsr()->HasMark())
            m_rWrtShell.GetCrsr()->SetMark();
        SwPosition *pMark = m_rWrtShell.GetCrsr()->GetMark();
        pMark->nContent = pMark->nContent.GetIndex() - rNewText.getLength();
#if OSL_DEBUG_LEVEL > 1
        OUString aSelTxt2( m_rWrtShell.GetSelTxt() );
#endif

        
        
        
        
        
        m_rWrtShell.ResetAttr();
        
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

    
    m_rWrtShell.StartAllAction();

    
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    OUString aOrigTxt( m_rWrtShell.GetSelTxt() );
    OUString aNewTxt( rReplaceWith );
    OSL_ENSURE( aOrigTxt == rOrigText, "!! text mismatch !!" );
    SwFmtRuby *pRuby = 0;
    bool bRubyBelow = false;
    OUString  aNewOrigText;
    switch (eAction)
    {
        case eExchange :
        break;
        case eReplacementBracketed :
        {
            aNewTxt = aOrigTxt + "(" + rReplaceWith + ")";
        }
        break;
        case eOriginalBracketed :
        {
            aNewTxt = rReplaceWith + "(" + aOrigTxt + ")";
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
            bRubyBelow = true;
        }
        break;
        case eOriginalBelow :
        {
            pRuby = new SwFmtRuby( aOrigTxt );
            aNewOrigText = rReplaceWith;
            bRubyBelow = true;
        }
        break;
        default:
            OSL_FAIL("unexpected case" );
    }
    m_nUnitOffset += nUnitStart + aNewTxt.getLength();

    if (pRuby)
    {
        m_rWrtShell.StartUndo( UNDO_SETRUBYATTR );
        if (!aNewOrigText.isEmpty())
        {
            
            
            ChangeText( aNewOrigText, rOrigText, NULL, NULL );

            
            
            
            
            
            
            
            m_rWrtShell.EndSelect();

            m_rWrtShell.Left( 0, sal_True, aNewOrigText.getLength(), sal_True, sal_True );
        }

        pRuby->SetPosition( static_cast<sal_uInt16>(bRubyBelow) );
        pRuby->SetAdjustment( RubyAdjust_CENTER );

#if OSL_DEBUG_LEVEL > 1
        SwPaM *pPaM = m_rWrtShell.GetCrsr();
        (void)pPaM;
#endif
        m_rWrtShell.SetAttrItem(*pRuby);
        delete pRuby;
        m_rWrtShell.EndUndo( UNDO_SETRUBYATTR );
    }
    else
    {
        m_rWrtShell.StartUndo( UNDO_OVERWRITE );

        
        
        
        const bool bIsChineseConversion = IsChinese( GetSourceLanguage() );
        if (bIsChineseConversion)
            ChangeText( aNewTxt, rOrigText, &rOffsets, m_rWrtShell.GetCrsr() );
        else
            ChangeText( aNewTxt, rOrigText, NULL, NULL );

        
        if (bIsChineseConversion)
        {
            m_rWrtShell.SetMark();
            m_rWrtShell.GetCrsr()->GetMark()->nContent -= aNewTxt.getLength();

            OSL_ENSURE( GetTargetLanguage() == LANGUAGE_CHINESE_SIMPLIFIED || GetTargetLanguage() == LANGUAGE_CHINESE_TRADITIONAL,
                    "SwHHCWrapper::ReplaceUnit : unexpected target language" );

            sal_uInt16 aRanges[] = {
                    RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
                    RES_CHRATR_CJK_FONT,     RES_CHRATR_CJK_FONT,
                    0, 0, 0  };

            SfxItemSet aSet( m_rWrtShell.GetAttrPool(), aRanges );
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

            m_rWrtShell.SetAttrSet( aSet );

            m_rWrtShell.ClearMark();
        }

        m_rWrtShell.EndUndo( UNDO_OVERWRITE );
    }

    m_rWrtShell.EndAllAction();
}


bool SwHHCWrapper::HasRubySupport() const
{
    return true;
}


void SwHHCWrapper::Convert()
{
    OSL_ENSURE( m_pConvArgs == 0, "NULL pointer expected" );
    {
        SwPaM *pCrsr = m_pView->GetWrtShell().GetCrsr();
        SwPosition* pSttPos = pCrsr->Start();
        SwPosition* pEndPos = pCrsr->End();


        if (pSttPos->nNode.GetNode().IsTxtNode() &&
            pEndPos->nNode.GetNode().IsTxtNode())
        {
            m_pConvArgs = new SwConversionArgs( GetSourceLanguage(),
                            pSttPos->nNode.GetNode().GetTxtNode(), pSttPos->nContent,
                            pEndPos->nNode.GetNode().GetTxtNode(), pEndPos->nContent );
        }
        else    
        {
            
            SwNode& rNode = m_pView->GetDocShell()->GetDoc()->GetNodes().GetEndOfContent();
            SwPaM aPam(rNode);
            aPam.Move( fnMoveBackward, fnGoDoc ); 

            pSttPos = aPam.GetPoint();  
            SwTxtNode *pTxtNode = pSttPos->nNode.GetNode().GetTxtNode();
            
            if (!pTxtNode || !pTxtNode->IsTxtNode())
                return;
            m_pConvArgs = new SwConversionArgs( GetSourceLanguage(),
                            pTxtNode, pSttPos->nContent,
                            pTxtNode, pSttPos->nContent );
        }
        OSL_ENSURE( m_pConvArgs->pStartNode && m_pConvArgs->pStartNode->IsTxtNode(),
                "failed to get proper start text node" );
        OSL_ENSURE( m_pConvArgs->pEndNode && m_pConvArgs->pEndNode->IsTxtNode(),
                "failed to get proper end text node" );

        
        OSL_ENSURE( IsChinese( GetSourceLanguage() ) == IsChinese( GetTargetLanguage() ),
                "source and target language mismatch?" );
        if (IsChinese( GetTargetLanguage() ))
        {
            m_pConvArgs->nConvTargetLang = GetTargetLanguage();
            m_pConvArgs->pTargetFont = GetTargetFont();
            m_pConvArgs->bAllowImplicitChangesForNotConvertibleText = true;
        }

        
        
        
        
        
        
        if (!pCrsr->HasMark())   
        {
            
            
            
            
            
            
            sal_Int32 nStartIdx = -1;
            if (editeng::HangulHanjaConversion::IsChinese( GetSourceLanguage() ) )
                nStartIdx = 0;
            else
            {
                OUString aText( m_pConvArgs->pStartNode->GetTxt() );
                const sal_Int32 nPos = m_pConvArgs->pStartIdx->GetIndex();
                Boundary aBoundary( g_pBreakIt->GetBreakIter()->
                        getWordBoundary( aText, nPos, g_pBreakIt->GetLocale( m_pConvArgs->nConvSrcLang ),
                                WordType::DICTIONARY_WORD, sal_True ) );

                
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

    if ( m_bIsOtherCntnt )
        ConvStart_impl( m_pConvArgs, SVX_SPELL_OTHER );
    else
    {
        m_bStartChk = false;
        ConvStart_impl( m_pConvArgs, SVX_SPELL_BODY_END );
    }

    ConvertDocument();

    ConvEnd_impl( m_pConvArgs );
}


bool SwHHCWrapper::ConvNext_impl( )
{
    

    
    if( m_bStartChk )
        m_bStartDone = true;
    else
        m_bEndDone = true;

    if( m_bIsOtherCntnt && m_bStartDone && m_bEndDone ) 
    {
        return false;
    }

    bool bGoOn = false;

    if ( m_bIsOtherCntnt )
    {
        m_bStartChk = false;
        ConvStart_impl( m_pConvArgs, SVX_SPELL_BODY );
        bGoOn = true;
    }
    else if ( m_bStartDone && m_bEndDone )
    {
        
        if( HasOtherCnt_impl() )
        {
            ConvStart_impl( m_pConvArgs, SVX_SPELL_OTHER );
            m_bIsOtherCntnt = bGoOn = true;
        }
    }
    else
    {
            m_bStartChk = !m_bStartDone;
            ConvStart_impl( m_pConvArgs, m_bStartChk ? SVX_SPELL_BODY_START : SVX_SPELL_BODY_END );
            bGoOn = true;
    }
    return bGoOn;
}


bool SwHHCWrapper::FindConvText_impl()
{
    

    bool bFound = false;

    m_pWin->EnterWait();
    bool bConv = true;

    while ( bConv )
    {
        bFound = ConvContinue_impl( m_pConvArgs );
        if (bFound)
        {
            bConv = false;
        }
        else
        {
            ConvEnd_impl( m_pConvArgs );
            bConv = ConvNext_impl();
        }
    }
    m_pWin->LeaveWait();
    return bFound;
}


bool SwHHCWrapper::HasOtherCnt_impl()
{
    return m_bIsSelection ? false : m_rWrtShell.HasOtherCnt();
}


void SwHHCWrapper::ConvStart_impl( SwConversionArgs /* [out] */ *pConversionArgs, SvxSpellArea eArea )
{
    SetDrawObj( SVX_SPELL_OTHER == eArea );
    m_pView->SpellStart( eArea, m_bStartDone, m_bEndDone, /* [out] */ pConversionArgs );
}


void SwHHCWrapper::ConvEnd_impl( SwConversionArgs *pConversionArgs )
{
    m_pView->SpellEnd( pConversionArgs );
}


bool SwHHCWrapper::ConvContinue_impl( SwConversionArgs *pConversionArgs )
{
    bool bProgress = !m_bIsDrawObj && !m_bIsSelection;
    pConversionArgs->aConvText = OUString();
    pConversionArgs->nConvTextLang = LANGUAGE_NONE;
    m_pView->GetWrtShell().SpellContinue( &m_nPageCount, bProgress ? &m_nPageStart : NULL, pConversionArgs );
    return !pConversionArgs->aConvText.isEmpty();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
