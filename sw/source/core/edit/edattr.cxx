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

#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <txatbase.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <editsh.hxx>
#include <edimp.hxx>
#include <doc.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <ftnidx.hxx>
#include <expfld.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <breakit.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <crsskip.hxx>
#include <txtfrm.hxx>
#include <scriptinfo.hxx>
#include <svl/ctloptions.hxx>
#include <charfmt.hxx>
#include <numrule.hxx>

#include <algorithm>

/*
 * hard Formatting (Attributes)
 */



const sal_uInt16& getMaxLookup()
{
    static const sal_uInt16 nMaxLookup = 1000;
    return nMaxLookup;
}

sal_Bool SwEditShell::GetPaMAttr( SwPaM* pPaM, SfxItemSet& rSet,
                              const bool bMergeIndentValuesOfNumRule ) const
{
    
    if( GetCrsrCnt() > getMaxLookup() )
    {
        rSet.InvalidateAllItems();
        return sal_False;
    }

    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet *pSet = &rSet;

    SwPaM* pStartPaM = pPaM;
    do {
        
        
        if (pPaM->IsInFrontOfLabel())
        {
            SwTxtNode * pTxtNd = pPaM->GetPoint()->nNode.GetNode().GetTxtNode();

            if (pTxtNd)
            {
                SwNumRule * pNumRule = pTxtNd->GetNumRule();

                if (pNumRule)
                {
                    int nListLevel = pTxtNd->GetActualListLevel();

                    if (nListLevel < 0)
                        nListLevel = 0;

                    if (nListLevel >= MAXLEVEL)
                        nListLevel = MAXLEVEL - 1;

                    const OUString & aCharFmtName =
                        pNumRule->Get(static_cast<sal_uInt16>(nListLevel)).GetCharFmtName();
                    SwCharFmt * pCharFmt =
                        GetDoc()->FindCharFmtByName(aCharFmtName);

                    if (pCharFmt)
                        rSet.Put(pCharFmt->GetAttrSet());
                }
            }

            continue;
        }

        sal_uLong nSttNd = pPaM->GetMark()->nNode.GetIndex(),
              nEndNd = pPaM->GetPoint()->nNode.GetIndex();
        sal_Int32 nSttCnt = pPaM->GetMark()->nContent.GetIndex();
        sal_Int32 nEndCnt = pPaM->GetPoint()->nContent.GetIndex();

        if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
        {
            std::swap(nSttNd, nEndNd);
            std::swap(nSttCnt, nEndCnt);
        }

        if( nEndNd - nSttNd >= getMaxLookup() )
        {
            rSet.ClearItem();
            rSet.InvalidateAllItems();
            return sal_False;
        }

        
        
        for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
        {
            SwNode* pNd = GetDoc()->GetNodes()[ n ];
            switch( pNd->GetNodeType() )
            {
            case ND_TEXTNODE:
                {
                    const sal_Int32 nStt = (n == nSttNd) ? nSttCnt : 0;
                    const sal_Int32 nEnd = (n == nEndNd)
                        ? nEndCnt
                        : static_cast<SwTxtNode*>(pNd)->GetTxt().getLength();

                    ((SwTxtNode*)pNd)->GetAttr( *pSet, nStt, nEnd,
                                                false, true,
                                                bMergeIndentValuesOfNumRule );
                }
                break;
            case ND_GRFNODE:
            case ND_OLENODE:
                ((SwCntntNode*)pNd)->GetAttr( *pSet );
                break;

            default:
                pNd = 0;
            }

            if( pNd )
            {
                if( pSet != &rSet )
                    rSet.MergeValues( aSet );

                if( aSet.Count() )
                    aSet.ClearItem();
            }
            pSet = &aSet;
        }

    } while ( ( pPaM = (SwPaM*)pPaM->GetNext() ) != pStartPaM );

    return sal_True;
}

sal_Bool SwEditShell::GetCurAttr( SfxItemSet& rSet,
                              const bool bMergeIndentValuesOfNumRule ) const
{
    return GetPaMAttr( GetCrsr(), rSet, bMergeIndentValuesOfNumRule );
}

sal_Bool SwEditShell::GetCurParAttr( SfxItemSet& rSet) const
{
    return GetPaMParAttr( GetCrsr(), rSet );
}

sal_Bool SwEditShell::GetPaMParAttr( SwPaM* pPaM, SfxItemSet& rSet ) const
{
    
    sal_uInt16 numberOfLookup = 0;

    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet* pSet = &rSet;

    SwPaM* pStartPaM = pPaM;
    do { 

        
        sal_uLong nSttNd = pPaM->GetMark()->nNode.GetIndex(),
              nEndNd = pPaM->GetPoint()->nNode.GetIndex();

        
        if( nSttNd > nEndNd )
            std::swap(nSttNd, nEndNd);

        
        
        
        for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
        {
            
            SwNode* pNd = GetDoc()->GetNodes()[ n ];

            if( pNd->IsTxtNode() )
            {
                
                static_cast<SwCntntNode*>(pNd)->GetAttr(*pSet);

                if( pSet != &rSet && aSet.Count() )
                {
                    rSet.MergeValues( aSet );
                    aSet.ClearItem();
                }

                pSet = &aSet;
            }

            ++numberOfLookup;

            
            if (numberOfLookup >= getMaxLookup())
                return sal_False;
        }
    } while ( ( pPaM = static_cast<SwPaM*>(pPaM->GetNext()) ) != pStartPaM );

    return sal_True;
}

SwTxtFmtColl* SwEditShell::GetCurTxtFmtColl( ) const
{
    return GetPaMTxtFmtColl( GetCrsr() );
}

SwTxtFmtColl* SwEditShell::GetPaMTxtFmtColl( SwPaM* pPaM ) const
{
    
    sal_uInt16 numberOfLookup = 0;

    SwPaM* pStartPaM = pPaM;
    do { 

        
        sal_uLong nSttNd = pPaM->GetMark()->nNode.GetIndex(),
              nEndNd = pPaM->GetPoint()->nNode.GetIndex();

        
        if( nSttNd > nEndNd )
            std::swap(nSttNd, nEndNd);

        
        for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
        {
            
            SwNode* pNd = GetDoc()->GetNodes()[ n ];

            ++numberOfLookup;

            
            if (numberOfLookup >= getMaxLookup())
                return NULL;

            if( pNd->IsTxtNode() )
            {
                
                SwTxtFmtColl* pFmt = static_cast<SwTxtNode*>(pNd)->GetTxtColl();

                
                if( pFmt != NULL )
                    return pFmt;
            }
        }
    } while ( ( pPaM = static_cast<SwPaM*>(pPaM->GetNext()) ) != pStartPaM );

    
    return NULL;
}

sal_Bool SwEditShell::GetCurFtn( SwFmtFtn* pFillFtn )
{
    
    SwPaM* pCrsr = GetCrsr();
    SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();
    if( !pTxtNd )
        return sal_False;

    SwTxtAttr *const pFtn = pTxtNd->GetTxtAttrForCharAt(
        pCrsr->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    if( pFtn && pFillFtn )
    {
        
        const SwFmtFtn &rFtn = ((SwTxtFtn*)pFtn)->GetFtn();
        pFillFtn->SetNumber( rFtn );
        pFillFtn->SetEndNote( rFtn.IsEndNote() );
    }
    return 0 != pFtn;
}

bool SwEditShell::SetCurFtn( const SwFmtFtn& rFillFtn )
{
    bool bChgd = false;
    StartAllAction();

    SwPaM* pCrsr = GetCrsr(), *pFirst = pCrsr;
    do {
        bChgd |=
            mpDoc->SetCurFtn( *pCrsr, rFillFtn.GetNumStr(), rFillFtn.GetNumber(), rFillFtn.IsEndNote() );

    } while( pFirst != ( pCrsr = (SwPaM*)pCrsr->GetNext() ));

    EndAllAction();
    return bChgd;
}

bool SwEditShell::HasFtns( bool bEndNotes ) const
{
    const SwFtnIdxs &rIdxs = mpDoc->GetFtnIdxs();
    for ( sal_uInt16 i = 0; i < rIdxs.size(); ++i )
    {
        const SwFmtFtn &rFtn = rIdxs[i]->GetFtn();
        if ( bEndNotes == rFtn.IsEndNote() )
            return true;
    }
    return false;
}


sal_uInt16 SwEditShell::GetSeqFtnList( SwSeqFldList& rList, bool bEndNotes )
{
    rList.Clear();

    sal_uInt16 n, nFtnCnt = mpDoc->GetFtnIdxs().size();
    SwTxtFtn* pTxtFtn;
    for( n = 0; n < nFtnCnt; ++n )
    {
        pTxtFtn = mpDoc->GetFtnIdxs()[ n ];
        const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
        if ( rFtn.IsEndNote() != bEndNotes )
            continue;

        SwNodeIndex* pIdx = pTxtFtn->GetStartNode();
        if( pIdx )
        {
            SwNodeIndex aIdx( *pIdx, 1 );
            SwTxtNode* pTxtNd = aIdx.GetNode().GetTxtNode();
            if( !pTxtNd )
                pTxtNd = (SwTxtNode*)mpDoc->GetNodes().GoNext( &aIdx );

            if( pTxtNd )
            {
                OUString sTxt( rFtn.GetViewNumStr( *mpDoc ));
                if( !sTxt.isEmpty() )
                    sTxt += " ";
                sTxt += pTxtNd->GetExpandTxt( 0, -1 );

                _SeqFldLstElem* pNew = new _SeqFldLstElem( sTxt,
                                            pTxtFtn->GetSeqRefNo() );
                while( rList.InsertSort( pNew ) )
                    pNew->sDlgEntry += " ";
            }
        }
    }

    return rList.Count();
}


bool SwEditShell::IsMoveLeftMargin( bool bRight, bool bModulus ) const
{
    bool bRet = true;

    const SvxTabStopItem& rTabItem = (SvxTabStopItem&)GetDoc()->
                                GetDefault( RES_PARATR_TABSTOP );
    sal_uInt16 nDefDist = static_cast<sal_uInt16>(rTabItem.Count() ? rTabItem[0].GetTabPos() : 1134);
    if( !nDefDist )
        return false;

    FOREACHPAM_START(GetCrsr())

        sal_uLong nSttNd = PCURCRSR->GetMark()->nNode.GetIndex(),
              nEndNd = PCURCRSR->GetPoint()->nNode.GetIndex();

        if( nSttNd > nEndNd )
            std::swap(nSttNd, nEndNd);

        SwCntntNode* pCNd;
        for( sal_uLong n = nSttNd; bRet && n <= nEndNd; ++n )
            if( 0 != ( pCNd = GetDoc()->GetNodes()[ n ]->GetTxtNode() ))
            {
                const SvxLRSpaceItem& rLS = (SvxLRSpaceItem&)
                                            pCNd->GetAttr( RES_LR_SPACE );
                if( bRight )
                {
                    long nNext = rLS.GetTxtLeft() + nDefDist;
                    if( bModulus )
                        nNext = ( nNext / nDefDist ) * nDefDist;
                    SwFrm* pFrm = pCNd->getLayoutFrm( GetLayout() );
                    if ( pFrm )
                    {
                        const sal_uInt16 nFrmWidth = static_cast<sal_uInt16>( pFrm->IsVertical() ?
                                                 pFrm->Frm().Height() :
                                                 pFrm->Frm().Width() );
                        bRet = nFrmWidth > ( nNext + MM50 );
                    }
                    else
                        bRet = false;
                }
            }

        if( !bRet )
            break;

    FOREACHPAM_END()
    return bRet;
}

void SwEditShell::MoveLeftMargin( bool bRight, bool bModulus )
{
    StartAllAction();
    StartUndo( UNDO_START );

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         
    {
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->MoveLeftMargin( aRangeArr.SetPam( n, aPam ),
                                        bRight, bModulus );
    }
    else
        GetDoc()->MoveLeftMargin( *pCrsr, bRight, bModulus );

    EndUndo( UNDO_END );
    EndAllAction();
}

static inline sal_uInt16 lcl_SetScriptFlags( sal_uInt16 nType )
{
    sal_uInt16 nRet;
       switch( nType )
    {
    case ::com::sun::star::i18n::ScriptType::LATIN:     nRet = SCRIPTTYPE_LATIN;    break;
    case ::com::sun::star::i18n::ScriptType::ASIAN:     nRet = SCRIPTTYPE_ASIAN;    break;
    case ::com::sun::star::i18n::ScriptType::COMPLEX:   nRet = SCRIPTTYPE_COMPLEX;  break;
    default: nRet = 0;
    }
    return nRet;
}

static bool lcl_IsNoEndTxtAttrAtPos( const SwTxtNode& rTNd, sal_Int32 nPos,
                            sal_uInt16 &rScrpt, bool bInSelection, bool bNum )
{
    bool bRet = false;
    OUString sExp;

    
    if ( bNum )
    {
        bRet = false;

        if ( rTNd.IsInList() )
        {
            OSL_ENSURE( rTNd.GetNumRule(),
                    "<lcl_IsNoEndTxtAttrAtPos(..)> - no list style found at text node. Serious defect -> please inform OD." );
            const SwNumRule* pNumRule = rTNd.GetNumRule();
            if(pNumRule)
            {
                int nListLevel = rTNd.GetActualListLevel();

                if (nListLevel < 0)
                    nListLevel = 0;

                if (nListLevel >= MAXLEVEL)
                    nListLevel = MAXLEVEL - 1;

                const SwNumFmt &rNumFmt = pNumRule->Get( static_cast<sal_uInt16>(nListLevel) );
                if( SVX_NUM_BITMAP != rNumFmt.GetNumberingType() )
                {
                    if ( SVX_NUM_CHAR_SPECIAL == rNumFmt.GetNumberingType() )
                        sExp = OUString(rNumFmt.GetBulletChar());
                    else
                        sExp = rTNd.GetNumString();
                }
            }
        }
    }

    
    if (nPos < rTNd.GetTxt().getLength() && CH_TXTATR_BREAKWORD == rTNd.GetTxt()[nPos])
    {
        const SwTxtAttr* const pAttr = rTNd.GetTxtAttrForCharAt( nPos );
        if (pAttr)
        {
            bRet = true; 
                         
            if ( RES_TXTATR_FIELD == pAttr->Which() )
            {
                const SwField* const pFld = pAttr->GetFmtFld().GetField();
                if (pFld)
                {
                    sExp += pFld->ExpandField(true);
                }
            }
        }
    }

    const sal_Int32 nEnd = sExp.getLength();
    if ( nEnd )
    {
        if( bInSelection )
        {
            sal_uInt16 nScript;
            for( sal_Int32 n = 0; n < nEnd;
                 n = g_pBreakIt->GetBreakIter()->endOfScript( sExp, n, nScript ))
            {
                nScript = g_pBreakIt->GetBreakIter()->getScriptType( sExp, n );
                rScrpt |= lcl_SetScriptFlags( nScript );
            }
        }
        else
            rScrpt |= lcl_SetScriptFlags( g_pBreakIt->GetBreakIter()->
                                        getScriptType( sExp, nEnd-1 ));
    }

    return bRet;
}


sal_uInt16 SwEditShell::GetScriptType() const
{
    sal_uInt16 nRet = 0;

    {
        FOREACHPAM_START(GetCrsr())

            const SwPosition *pStt = PCURCRSR->Start(),
                             *pEnd = pStt == PCURCRSR->GetMark()
                                    ? PCURCRSR->GetPoint()
                                    : PCURCRSR->GetMark();
            if( pStt == pEnd || *pStt == *pEnd )
            {
                const SwTxtNode* pTNd = pStt->nNode.GetNode().GetTxtNode();
                if( pTNd )
                {
                    
                    const SwScriptInfo* pScriptInfo = SwScriptInfo::GetScriptInfo( *pTNd );

                    sal_Int32 nPos = pStt->nContent.GetIndex();
                    
                    
                    if( nPos )
                    {
                        SwIndex aIdx( pStt->nContent );
                        if( pTNd->GoPrevious( &aIdx, CRSR_SKIP_CHARS ) )
                            nPos = aIdx.GetIndex();
                    }

                    sal_uInt16 nScript;

                    if (!pTNd->GetTxt().isEmpty())
                    {
                        nScript = pScriptInfo ?
                                  pScriptInfo->ScriptType( nPos ) :
                                  g_pBreakIt->GetBreakIter()->getScriptType( pTNd->GetTxt(), nPos );
                    }
                    else
                        nScript = GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );

                    if( !lcl_IsNoEndTxtAttrAtPos( *pTNd, nPos, nRet, false, false ))
                        nRet |= lcl_SetScriptFlags( nScript );
                }
            }
            else if ( g_pBreakIt->GetBreakIter().is() )
            {
                sal_uLong nEndIdx = pEnd->nNode.GetIndex();
                SwNodeIndex aIdx( pStt->nNode );
                for( ; aIdx.GetIndex() <= nEndIdx; ++aIdx )
                    if( aIdx.GetNode().IsTxtNode() )
                    {
                        const SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
                        const OUString& rTxt = pTNd->GetTxt();

                        
                        const SwScriptInfo* pScriptInfo = SwScriptInfo::GetScriptInfo( *pTNd );

                        sal_Int32 nChg = aIdx == pStt->nNode
                                                ? pStt->nContent.GetIndex()
                                                : 0;
                        sal_Int32 nEndPos = aIdx == nEndIdx
                                                ? pEnd->nContent.GetIndex()
                                                : rTxt.getLength();

                        OSL_ENSURE( nEndPos <= rTxt.getLength(),
                                "Index outside the range - endless loop!" );
                        if (nEndPos > rTxt.getLength())
                            nEndPos = rTxt.getLength();

                        sal_uInt16 nScript;
                        while( nChg < nEndPos )
                        {
                            nScript = pScriptInfo ?
                                      pScriptInfo->ScriptType( nChg ) :
                                      g_pBreakIt->GetBreakIter()->getScriptType(
                                                                rTxt, nChg );

                            if( !lcl_IsNoEndTxtAttrAtPos( *pTNd, nChg, nRet, true,
                                      0 == nChg && rTxt.getLength() == nEndPos))
                                nRet |= lcl_SetScriptFlags( nScript );

                            if( (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN |
                                SCRIPTTYPE_COMPLEX) == nRet )
                                break;

                            sal_Int32 nFldPos = nChg+1;

                            nChg = pScriptInfo ?
                                   pScriptInfo->NextScriptChg( nChg ) :
                                   g_pBreakIt->GetBreakIter()->endOfScript(
                                                    rTxt, nChg, nScript );

                            nFldPos = rTxt.indexOf(
                                            CH_TXTATR_BREAKWORD, nFldPos);
                            if ((-1 != nFldPos) && (nFldPos < nChg))
                                nChg = nFldPos;
                        }
                        if( (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN |
                                SCRIPTTYPE_COMPLEX) == nRet )
                            break;
                    }
            }
            if( (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN |
                                SCRIPTTYPE_COMPLEX) == nRet )
                break;

        FOREACHPAM_END()
    }
    if( !nRet )
        nRet = SvtLanguageOptions::GetScriptTypeOfLanguage( LANGUAGE_SYSTEM );
    return nRet;
}

sal_uInt16 SwEditShell::GetCurLang() const
{
    const SwPaM* pCrsr = GetCrsr();
    const SwPosition& rPos = *pCrsr->GetPoint();
    const SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();
    sal_uInt16 nLang;
    if( pTNd )
    {
        
        
        sal_Int32 nPos = rPos.nContent.GetIndex();
        if( nPos && !pCrsr->HasMark() )
            --nPos;
        nLang = pTNd->GetLang( nPos );
    }
    else
        nLang = LANGUAGE_DONTKNOW;
    return nLang;
}

sal_uInt16 SwEditShell::GetScalingOfSelectedText() const
{
    const SwPaM* pCrsr = GetCrsr();
    const SwPosition* pStt = pCrsr->Start();
    const SwTxtNode* pTNd = pStt->nNode.GetNode().GetTxtNode();
    OSL_ENSURE( pTNd, "no textnode available" );

    sal_uInt16 nScaleWidth;
    if( pTNd )
    {
        const SwPosition* pEnd = pStt == pCrsr->GetPoint()
                                        ? pCrsr->GetMark()
                                        : pCrsr->GetPoint();
        const sal_Int32 nStt = pStt->nContent.GetIndex();
        const sal_Int32 nEnd = pStt->nNode == pEnd->nNode
            ? pEnd->nContent.GetIndex()
            : pTNd->GetTxt().getLength();
        nScaleWidth = pTNd->GetScalingOfSelectedText( nStt, nEnd );
    }
    else
        nScaleWidth = 100;              
    return nScaleWidth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
