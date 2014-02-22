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


#include <ctype.h>
#include <hintids.hxx>
#include <hints.hxx>
#include <vcl/graphicfilter.hxx>

#include <vcl/graph.hxx>
#include <svl/urihelper.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/charclass.hxx>
#include <txtftn.hxx>
#include <fmtpdsc.hxx>
#include <fmtftn.hxx>
#include <fmtanchr.hxx>
#include <fmtrfmrk.hxx>
#include <fmtclds.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <redline.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <frmatr.hxx>
#include <fldbas.hxx>
#include <charatr.hxx>
#include <swtable.hxx>
#include <tox.hxx>
#include <expfld.hxx>
#include <section.hxx>
#include <tblsel.hxx>
#include <pagedesc.hxx>
#include <docsh.hxx>
#include <fltshell.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>


using namespace com::sun::star;

static SwCntntNode* GetCntntNode(SwDoc* pDoc, SwNodeIndex& rIdx, bool bNext)
{
    SwCntntNode * pCNd = rIdx.GetNode().GetCntntNode();
    if(!pCNd && 0 == (pCNd = bNext ? pDoc->GetNodes().GoNext(&rIdx)
                                     : pDoc->GetNodes().GoPrevious(&rIdx)))
    {
        pCNd = bNext ? pDoc->GetNodes().GoPrevious(&rIdx)
                     : pDoc->GetNodes().GoNext(&rIdx);
        OSL_ENSURE(pCNd, "no ContentNode found");
    }
    return pCNd;
}


SwFltStackEntry::SwFltStackEntry(const SwPosition& rStartPos, SfxPoolItem* pHt)
    : m_aMkPos(rStartPos)
    , m_aPtPos(rStartPos)
    , mnStartCP(-1)
    , mnEndCP(-1)
    , bIsParaEnd(false)
{
    pAttr = pHt;            
    bOld    = sal_False;    
    bOpen = sal_True;       
    bConsumedByField = sal_False;
}

SwFltStackEntry::~SwFltStackEntry()
{
    
    delete pAttr;
}

void SwFltStackEntry::SetEndPos(const SwPosition& rEndPos)
{
    
    
    
    
    bOpen = sal_False;                  
    m_aPtPos.SetPos(rEndPos);
}


bool SwFltStackEntry::MakeRegion(SwDoc* pDoc, SwPaM& rRegion, bool bCheck,
    const SwFltPosition &rMkPos, const SwFltPosition &rPtPos, bool bIsParaEnd,
    sal_uInt16 nWhich)
{
    
    
    
    //
    
    
    SwCntntNode *const pCntntNode(
        SwNodeIndex(rMkPos.m_nNode, +1).GetNode().GetCntntNode());
    if (rMkPos == rPtPos &&
        ((0 != rPtPos.m_nCntnt) || (pCntntNode && (0 != pCntntNode->Len())))
        && ( RES_TXTATR_FIELD != nWhich
             && RES_TXTATR_ANNOTATION != nWhich
             && RES_TXTATR_INPUTFIELD != nWhich )
        && !(bIsParaEnd && pCntntNode && pCntntNode->IsTxtNode() && 0 != pCntntNode->Len() ))
    {
        return false;
    }
    
    rRegion.GetPoint()->nNode = rMkPos.m_nNode.GetIndex() + 1;
    SwCntntNode* pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, true);
    rRegion.GetPoint()->nContent.Assign(pCNd, rMkPos.m_nCntnt);
    rRegion.SetMark();
    if (rMkPos.m_nNode != rPtPos.m_nNode)
    {
        rRegion.GetPoint()->nNode = rPtPos.m_nNode.GetIndex() + 1;
        pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, false);
    }
    rRegion.GetPoint()->nContent.Assign(pCNd, rPtPos.m_nCntnt);
    OSL_ENSURE( CheckNodesRange( rRegion.Start()->nNode,
                             rRegion.End()->nNode, true ),
             "atttribute or similar crosses section-boundaries" );
    if( bCheck )
        return CheckNodesRange( rRegion.Start()->nNode,
                                rRegion.End()->nNode, true );
    else
        return true;
}

bool SwFltStackEntry::MakeRegion(SwDoc* pDoc, SwPaM& rRegion, bool bCheck) const
{
    return MakeRegion(pDoc, rRegion, bCheck, m_aMkPos, m_aPtPos, bIsParaEnd,
        pAttr->Which());
}

SwFltControlStack::SwFltControlStack(SwDoc* pDo, sal_uLong nFieldFl)
    : nFieldFlags(nFieldFl),bHasSdOD(true), bSdODChecked(false), pDoc(pDo), bIsEndStack(false)
{
}


SwFltControlStack::~SwFltControlStack()
{
    OSL_ENSURE(maEntries.empty(), "There are still Attributes on the stack");
}








void SwFltControlStack::MoveAttrs( const SwPosition& rPos )
{
    size_t nCnt = maEntries.size();
    sal_uLong nPosNd = rPos.nNode.GetIndex();
    sal_uInt16 nPosCt = rPos.nContent.GetIndex() - 1;

    for (size_t i=0; i < nCnt; ++i)
    {
        SwFltStackEntry& rEntry = maEntries[i];
        if (
            (rEntry.m_aMkPos.m_nNode.GetIndex()+1 == nPosNd) &&
            (rEntry.m_aMkPos.m_nCntnt >= nPosCt)
           )
        {
            rEntry.m_aMkPos.m_nCntnt++;
            OSL_ENSURE( rEntry.m_aMkPos.m_nCntnt
                <= pDoc->GetNodes()[nPosNd]->GetCntntNode()->Len(),
                    "Attribute ends after end of line" );
        }
        if (
            (rEntry.m_aPtPos.m_nNode.GetIndex()+1 == nPosNd) &&
            (rEntry.m_aPtPos.m_nCntnt >= nPosCt)
           )
        {
            rEntry.m_aPtPos.m_nCntnt++;
            OSL_ENSURE( rEntry.m_aPtPos.m_nCntnt
                <= pDoc->GetNodes()[nPosNd]->GetCntntNode()->Len(),
                    "Attribute ends after end of line" );
        }
    }
}

void SwFltControlStack::MarkAllAttrsOld()
{
    size_t nCnt = maEntries.size();
    for (sal_uInt16 i=0; i < nCnt; ++i)
        maEntries[i].bOld = sal_True;
}

namespace
{
    bool couldExtendEntry(const SwFltStackEntry *pExtendCandidate,
        const SfxPoolItem& rAttr)
    {
        return (pExtendCandidate &&
                !pExtendCandidate->bConsumedByField &&
                
                (isPARATR_LIST(rAttr.Which()) || (isCHRATR(rAttr.Which()) && rAttr.Which() != RES_CHRATR_FONT && rAttr.Which() != RES_CHRATR_FONTSIZE)) &&
                *(pExtendCandidate->pAttr) == rAttr);
    }
}

void SwFltControlStack::NewAttr(const SwPosition& rPos, const SfxPoolItem& rAttr)
{
    sal_uInt16 nWhich = rAttr.Which();
    
    
    SwFltStackEntry *pExtendCandidate = SetAttr(rPos, nWhich);
    if (couldExtendEntry(pExtendCandidate, rAttr))
    {
        
        
        //
        
        
        
        
        pExtendCandidate->SetEndPos(rPos);
        pExtendCandidate->bOpen=true;
    }
    else
    {
        SwFltStackEntry *pTmp = new SwFltStackEntry(rPos, rAttr.Clone() );
        pTmp->SetStartCP(GetCurrAttrCP());
        maEntries.push_back(pTmp);
    }
}

void SwFltControlStack::DeleteAndDestroy(Entries::size_type nCnt)
{
    OSL_ENSURE(nCnt < maEntries.size(), "Out of range!");
    if (nCnt < maEntries.size())
    {
        myEIter aElement = maEntries.begin() + nCnt;
        maEntries.erase(aElement);
    }
    
    
    if ( empty() )
    {
        ClearParaEndPosition();
        bHasSdOD = true;
        bSdODChecked = false;
    }
}






void SwFltControlStack::StealAttr(const SwNodeIndex& rNode, sal_uInt16 nAttrId)
{
    size_t nCnt = maEntries.size();

    while (nCnt)
    {
        nCnt --;
        SwFltStackEntry& rEntry = maEntries[nCnt];
        if (rEntry.m_aPtPos.m_nNode.GetIndex()+1 == rNode.GetIndex() &&
            (!nAttrId || nAttrId == rEntry.pAttr->Which()))
        {
            DeleteAndDestroy(nCnt);     
        }
    }
}





void SwFltControlStack::KillUnlockedAttrs(const SwPosition& rPos)
{
    SwFltPosition aFltPos(rPos);

    size_t nCnt = maEntries.size();
    while( nCnt )
    {
        nCnt --;
        SwFltStackEntry& rEntry = maEntries[nCnt];
        if(    !rEntry.bOld
            && !rEntry.bOpen
            && (rEntry.m_aMkPos == aFltPos)
            && (rEntry.m_aPtPos == aFltPos))
        {
            DeleteAndDestroy( nCnt ); 
        }
    }
}




SwFltStackEntry* SwFltControlStack::SetAttr(const SwPosition& rPos,
    sal_uInt16 nAttrId, sal_Bool bTstEnde, long nHand,
    sal_Bool consumedByField)
{
    SwFltStackEntry *pRet = NULL;

    SwFltPosition aFltPos(rPos);

    OSL_ENSURE(!nAttrId ||
        (POOLATTR_BEGIN <= nAttrId && POOLATTR_END > nAttrId) ||
        (RES_FLTRATTR_BEGIN <= nAttrId && RES_FLTRATTR_END > nAttrId),
        "Wrong id for attribute");

    myEIter aI = maEntries.begin();
    while (aI != maEntries.end())
    {
        bool bLastEntry = aI == maEntries.end() - 1;

        SwFltStackEntry& rEntry = *aI;
        if (rEntry.bOpen)
        {
            
            bool bF = false;
            if (!nAttrId )
            {
                bF = true;
            }
            else if (nAttrId == rEntry.pAttr->Which())
            {
                if( nAttrId != RES_FLTR_BOOKMARK )
                {
                    
                    bF = true;
                }
                else if (nHand == ((SwFltBookmark*)(rEntry.pAttr))->GetHandle())
                {
                    bF = true;
                }
            }
            if (bF)
            {
                rEntry.bConsumedByField = consumedByField;
                rEntry.SetEndPos(rPos);
                rEntry.SetEndCP(GetCurrAttrCP());
                if (bLastEntry && nAttrId == rEntry.pAttr->Which())
                {
                    
                    
                    pRet = &rEntry;
                }
            }
            ++aI;
            continue;
        }

        
        
        
        
        if (bTstEnde)
        {
            if (bIsEndStack)
            {
                ++aI;
                continue;
            }

            
            
            if (rEntry.m_aPtPos.m_nNode.GetIndex() == aFltPos.m_nNode.GetIndex())
            {
                if (bLastEntry && nAttrId == rEntry.pAttr->Which() &&
                    rEntry.m_aPtPos.m_nCntnt == aFltPos.m_nCntnt)
                {
                    
                    
                    pRet = &rEntry;
                }

                ++aI;
                continue;
            }
        }
        SetAttrInDoc(rPos, rEntry);
        aI = maEntries.erase(aI);
    }

    return pRet;
}

static void MakePoint(const SwFltStackEntry& rEntry, SwDoc* pDoc,
    SwPaM& rRegion)
{
    
    
    
    rRegion.DeleteMark();
    rRegion.GetPoint()->nNode = rEntry.m_aMkPos.m_nNode.GetIndex() + 1;
    SwCntntNode* pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, true);
    rRegion.GetPoint()->nContent.Assign(pCNd, rEntry.m_aMkPos.m_nCntnt);
}




static void MakeBookRegionOrPoint(const SwFltStackEntry& rEntry, SwDoc* pDoc,
                    SwPaM& rRegion, sal_Bool bCheck )
{
    if (rEntry.MakeRegion(pDoc, rRegion, bCheck )){
        
        if (rRegion.GetPoint()->nNode.GetNode().FindTableBoxStartNode()
              != rRegion.GetMark()->nNode.GetNode().FindTableBoxStartNode())
        {
            rRegion.Exchange();         
            rRegion.DeleteMark();       
        }
    }else{
        MakePoint(rEntry, pDoc, rRegion);
    }
}



//






static bool IterateNumrulePiece( const SwNodeIndex& rEnd,
                                SwNodeIndex& rTmpStart, SwNodeIndex& rTmpEnd )
{
    while( ( rTmpStart <= rEnd )
           && !( rTmpStart.GetNode().IsTxtNode() ) )    
        ++rTmpStart;

    rTmpEnd = rTmpStart;
    while( ( rTmpEnd <= rEnd )
           && ( rTmpEnd.GetNode().IsTxtNode() ) )       
        ++rTmpEnd;

    rTmpEnd--;                                      

    return rTmpStart <= rTmpEnd;                    
}



bool SwFltControlStack::HasSdOD()
{
    bool bRet = false;

    for (Entries::iterator it = maEntries.begin(); it != maEntries.end(); ++it)
    {
        SwFltStackEntry& rEntry = *it;
        if ( rEntry.mnStartCP == rEntry.mnEndCP )
        {
            if ( CheckSdOD(rEntry.mnStartCP,rEntry.mnEndCP) )
            {
                bRet = true;
                break;
            }
        }
    }

    return bRet;
}

void SwFltControlStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry& rEntry)
{
    SwPaM aRegion( rTmpPos );

    switch(rEntry.pAttr->Which())
    {
    case RES_FLTR_ANCHOR:
        {
            SwFrmFmt* pFmt = ((SwFltAnchor*)rEntry.pAttr)->GetFrmFmt();
            if (pFmt != NULL)
            {
                MakePoint(rEntry, pDoc, aRegion);
                SwFmtAnchor aAnchor(pFmt->GetAnchor());
                aAnchor.SetAnchor(aRegion.GetPoint());
                pFmt->SetFmtAttr(aAnchor);
                
                
                if(pDoc->GetCurrentViewShell()
                   && (FLY_AT_PARA == pFmt->GetAnchor().GetAnchorId()))
                {
                    pFmt->MakeFrms();
                }
            }
        }
        break;
    case RES_FLTR_STYLESHEET:
        break;

    case RES_TXTATR_FIELD:
    case RES_TXTATR_ANNOTATION:
    case RES_TXTATR_INPUTFIELD:
        break;

    case RES_TXTATR_TOXMARK:
        break;

    case RES_FLTR_NUMRULE:          
        {
            const OUString& rNumNm = ((SfxStringItem*)rEntry.pAttr)->GetValue();
            SwNumRule* pRul = pDoc->FindNumRulePtr( rNumNm );
            if( pRul )
            {
                if( rEntry.MakeRegion(pDoc, aRegion, true))
                {
                    SwNodeIndex aTmpStart( aRegion.Start()->nNode );
                    SwNodeIndex aTmpEnd( aTmpStart );
                    SwNodeIndex& rRegEndNd = aRegion.End()->nNode;
                    while( IterateNumrulePiece( rRegEndNd,
                                                aTmpStart, aTmpEnd ) )
                    {
                        SwPaM aTmpPam( aTmpStart, aTmpEnd );
                        
                        pDoc->SetNumRule( aTmpPam, *pRul, false );

                        aTmpStart = aTmpEnd;    
                        ++aTmpStart;
                    }
                }
                else
                    pDoc->DelNumRule( rNumNm );
            }
        }
        break;
    case RES_FLTR_NUMRULE_NUM:
        break;
    case RES_FLTR_BOOKMARK:
        {
            SwFltBookmark* pB = (SwFltBookmark*)rEntry.pAttr;
            const OUString& rName = ((SwFltBookmark*)rEntry.pAttr)->GetName();

            if (IsFlagSet(BOOK_TO_VAR_REF))
            {
                SwFieldType* pFT = pDoc->GetFldType(RES_SETEXPFLD, rName, false);
                if (!pFT)
                {
                    SwSetExpFieldType aS(pDoc, rName, nsSwGetSetExpType::GSE_STRING);
                    pFT = pDoc->InsertFldType(aS);
                }
                SwSetExpField aFld((SwSetExpFieldType*)pFT, pB->GetValSys());
                aFld.SetSubType( nsSwExtendedSubType::SUB_INVISIBLE );
                MakePoint(rEntry, pDoc, aRegion);
                pDoc->InsertPoolItem(aRegion, SwFmtFld(aFld), 0);
                MoveAttrs( *(aRegion.GetPoint()) );
            }
            if ( ( !IsFlagSet(HYPO) || IsFlagSet(BOOK_AND_REF) ) &&
                 !rEntry.bConsumedByField )
            {
                MakeBookRegionOrPoint(rEntry, pDoc, aRegion, sal_True);
                
                const IDocumentMarkAccess::MarkType eBookmarkType =
                    ( pB->IsTOCBookmark() &&
                      IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark( aRegion ) )
                    ? IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK
                    : IDocumentMarkAccess::BOOKMARK;
                pDoc->getIDocumentMarkAccess()->makeMark( aRegion, rName, eBookmarkType );
            }
        }
        break;
    case RES_FLTR_TOX:
        {
            MakePoint(rEntry, pDoc, aRegion);

            SwPosition* pPoint = aRegion.GetPoint();

            SwFltTOX* pTOXAttr = (SwFltTOX*)rEntry.pAttr;

            
            
            SfxItemSet aBkSet( pDoc->GetAttrPool(), RES_PAGEDESC, RES_BREAK );
            SwCntntNode* pNd = 0;
            if( !pTOXAttr->HadBreakItem() || !pTOXAttr->HadPageDescItem() )
            {
                pNd = pPoint->nNode.GetNode().GetCntntNode();
                if( pNd )
                {
                    const SfxItemSet* pSet = pNd->GetpSwAttrSet();
                    const SfxPoolItem* pItem;
                    if( pSet )
                    {
                        if(    !pTOXAttr->HadBreakItem()
                            && SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, false, &pItem ) )
                        {
                            aBkSet.Put( *pItem );
                            pNd->ResetAttr( RES_BREAK );
                        }
                        if(    !pTOXAttr->HadPageDescItem()
                            && SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, false, &pItem ) )
                        {
                            aBkSet.Put( *pItem );
                            pNd->ResetAttr( RES_PAGEDESC );
                        }
                    }
                }
            }

            delete pTOXAttr->GetBase();

            
            if( aBkSet.Count() )
                pNd->SetAttr( aBkSet );
        }
        break;
    case RES_FLTR_SECTION:
        MakePoint(rEntry, pDoc, aRegion);   
        pDoc->InsertSwSection(aRegion,
                *(static_cast<SwFltSection*>(rEntry.pAttr))->GetSectionData(),
                0, 0, false);
        delete (((SwFltSection*)rEntry.pAttr)->GetSectionData());
        break;
    case RES_FLTR_REDLINE:
        {
            if (rEntry.MakeRegion(pDoc, aRegion, true))
            {
              pDoc->SetRedlineMode((RedlineMode_t)(   nsRedlineMode_t::REDLINE_ON
                                              | nsRedlineMode_t::REDLINE_SHOW_INSERT
                                              | nsRedlineMode_t::REDLINE_SHOW_DELETE ));
                SwFltRedline& rFltRedline = *((SwFltRedline*)rEntry.pAttr);

                if( USHRT_MAX != rFltRedline.nAutorNoPrev )
                {
                    SwRedlineData aData(rFltRedline.eTypePrev,
                                        rFltRedline.nAutorNoPrev,
                                        rFltRedline.aStampPrev,
                                        OUString(),
                                        0
                                        );
                    pDoc->AppendRedline(new SwRangeRedline(aData, aRegion), true);
                }
                SwRedlineData aData(rFltRedline.eType,
                                    rFltRedline.nAutorNo,
                                    rFltRedline.aStamp,
                                    OUString(),
                                    0
                                    );
                pDoc->AppendRedline( new SwRangeRedline(aData, aRegion), true );
                pDoc->SetRedlineMode((RedlineMode_t)( nsRedlineMode_t::REDLINE_NONE
                                                | nsRedlineMode_t::REDLINE_SHOW_INSERT
                                                | nsRedlineMode_t::REDLINE_SHOW_DELETE ));
            }
        }
        break;
    default:
        {
            
            if ( !bSdODChecked )
            {
                bHasSdOD = HasSdOD();
                bSdODChecked = true;
            }
            sal_Int32 nStart = rEntry.GetStartCP();
            sal_Int32 nEnd = rEntry.GetEndCP();
            if (nStart != -1 && nEnd != -1 && nEnd >= nStart )
            {
                rEntry.SetIsParaEnd( IsParaEndInCPs(nStart,nEnd,bHasSdOD) );
            }
            if (rEntry.MakeRegion(pDoc, aRegion, false))
            {
                nStart = rEntry.GetStartCP();
                nEnd = rEntry.GetEndCP();
                if (rEntry.IsParaEnd())
                {
                    pDoc->InsertPoolItem(aRegion, *rEntry.pAttr, 0, true);
                }
                else
                {
                    pDoc->InsertPoolItem(aRegion, *rEntry.pAttr, 0);
                }
            }
        }
        break;
    }
}

bool SwFltControlStack::IsParaEndInCPs(sal_Int32 /*nStart*/, sal_Int32 /*nEnd*/,bool /*bSdOD*/) const
{
    return false;
}

bool SwFltControlStack::CheckSdOD(sal_Int32 /*nStart*/, sal_Int32 /*nEnd*/)
{
    return false;
}

SfxPoolItem* SwFltControlStack::GetFmtStackAttr(sal_uInt16 nWhich, sal_uInt16 * pPos)
{
    size_t nSize = maEntries.size();

    while (nSize)
    {
        
        
        SwFltStackEntry &rEntry = maEntries[--nSize];
        if (rEntry.bOpen && rEntry.pAttr->Which() == nWhich)
        {
            if (pPos)
                *pPos = nSize;
            return (SfxPoolItem*)rEntry.pAttr;      
        }
    }
    return 0;
}

const SfxPoolItem* SwFltControlStack::GetOpenStackAttr(const SwPosition& rPos, sal_uInt16 nWhich)
{
    SwFltPosition aFltPos(rPos);

    size_t nSize = maEntries.size();

    while (nSize)
    {
        SwFltStackEntry &rEntry = maEntries[--nSize];
        if (rEntry.bOpen && rEntry.pAttr->Which() == nWhich && rEntry.m_aMkPos == aFltPos)
        {
            return (SfxPoolItem*)rEntry.pAttr;
        }
    }
    return 0;
}

const SfxPoolItem* SwFltControlStack::GetFmtAttr(const SwPosition& rPos, sal_uInt16 nWhich)
{
    SfxPoolItem* pHt = GetFmtStackAttr(nWhich);
    if (pHt)
        return (const SfxPoolItem*)pHt;

    
    SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();

    if (!pNd)           
        return &pDoc->GetAttrPool().GetDefaultItem(nWhich);
    return &pNd->GetAttr(nWhich);
}

void SwFltControlStack::Delete(const SwPaM &rPam)
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    SwNodeIndex aStartNode(pStt->nNode, -1);
    const sal_Int32 nStartIdx = pStt->nContent.GetIndex();
    SwNodeIndex aEndNode(pEnd->nNode, -1);
    const sal_Int32 nEndIdx = pEnd->nContent.GetIndex();

    
    OSL_ENSURE(aEndNode == aStartNode, "nodes must be the same, or this method extended");
    if (aEndNode != aStartNode)
        return;

    for (size_t nSize = maEntries.size(); nSize > 0;)
    {
        SwFltStackEntry& rEntry = maEntries[--nSize];

        bool bEntryStartAfterSelStart =
            (rEntry.m_aMkPos.m_nNode == aStartNode &&
             rEntry.m_aMkPos.m_nCntnt >= nStartIdx);

        bool bEntryStartBeforeSelEnd =
            (rEntry.m_aMkPos.m_nNode == aEndNode &&
             rEntry.m_aMkPos.m_nCntnt <= nEndIdx);

        bool bEntryEndAfterSelStart = false;
        bool bEntryEndBeforeSelEnd = false;
        if (!rEntry.bOpen)
        {
            bEntryEndAfterSelStart =
                (rEntry.m_aPtPos.m_nNode == aStartNode &&
                 rEntry.m_aPtPos.m_nCntnt >= nStartIdx);

            bEntryEndBeforeSelEnd =
                (rEntry.m_aPtPos.m_nNode == aEndNode &&
                 rEntry.m_aPtPos.m_nCntnt <= nEndIdx);
        }

        bool bTotallyContained = false;
        if (
             bEntryStartAfterSelStart && bEntryStartBeforeSelEnd &&
             bEntryEndAfterSelStart && bEntryEndBeforeSelEnd
           )
        {
           bTotallyContained = true;
        }

        if (bTotallyContained)
        {
            
            DeleteAndDestroy(nSize);
            continue;
        }

        const sal_Int32 nCntntDiff = nEndIdx - nStartIdx;

        
        if (bEntryStartAfterSelStart)
        {
            if (bEntryStartBeforeSelEnd)
            {
                
                rEntry.m_aMkPos.SetPos(aStartNode, nStartIdx);
            }
            else
                rEntry.m_aMkPos.m_nCntnt -= nCntntDiff;
        }

        if (bEntryEndAfterSelStart)
        {
            if (bEntryEndBeforeSelEnd)
                rEntry.m_aPtPos.SetPos(aStartNode, nStartIdx);
            else
                rEntry.m_aPtPos.m_nCntnt -= nCntntDiff;
        }

        
        if (rEntry.bOpen)
            rEntry.m_aPtPos = rEntry.m_aMkPos;
    }
}


SwFltAnchor::SwFltAnchor(SwFrmFmt* pFmt) :
    SfxPoolItem(RES_FLTR_ANCHOR), pFrmFmt(pFmt)
{
    pClient = new SwFltAnchorClient(this);
    pFrmFmt->Add(pClient);
}

SwFltAnchor::SwFltAnchor(const SwFltAnchor& rCpy) :
    SfxPoolItem(RES_FLTR_ANCHOR), pFrmFmt(rCpy.pFrmFmt)
{
    pClient = new SwFltAnchorClient(this);
    pFrmFmt->Add(pClient);
}

SwFltAnchor::~SwFltAnchor()
{
    delete pClient;
}

void SwFltAnchor::SetFrmFmt(SwFrmFmt * _pFrmFmt)
{
    pFrmFmt = _pFrmFmt;
}

const SwFrmFmt * SwFltAnchor::GetFrmFmt() const
{
    return pFrmFmt;
}

SwFrmFmt * SwFltAnchor::GetFrmFmt()
{
    return pFrmFmt;
}

bool SwFltAnchor::operator==(const SfxPoolItem& rItem) const
{
    return pFrmFmt == ((SwFltAnchor&)rItem).pFrmFmt;
}

SfxPoolItem* SwFltAnchor::Clone(SfxItemPool*) const
{
    return new SwFltAnchor(*this);
}



SwFltAnchorClient::SwFltAnchorClient(SwFltAnchor * pFltAnchor)
: m_pFltAnchor(pFltAnchor)
{
}

void  SwFltAnchorClient::Modify(const SfxPoolItem *, const SfxPoolItem * pNew)
{
    if (pNew->Which() == RES_FMT_CHG)
    {
        const SwFmtChg * pFmtChg = dynamic_cast<const SwFmtChg *> (pNew);

        if (pFmtChg != NULL)
        {
            SwFrmFmt * pFrmFmt = dynamic_cast<SwFrmFmt *> (pFmtChg->pChangedFmt);

            if (pFrmFmt != NULL)
                m_pFltAnchor->SetFrmFmt(pFrmFmt);
        }
    }
}


bool SwFltRedline::operator==(const SfxPoolItem& rItem) const
{
    return this == &rItem;
}

SfxPoolItem* SwFltRedline::Clone( SfxItemPool* ) const
{
    return new SwFltRedline(*this);
}


SwFltBookmark::SwFltBookmark( const OUString& rNa, const OUString& rVa,
                              long nHand, const bool bIsTOCBookmark )
    : SfxPoolItem( RES_FLTR_BOOKMARK )
    , mnHandle( nHand )
    , maName( rNa )
    , maVal( rVa )
    , mbIsTOCBookmark( bIsTOCBookmark )
{
    
    
    
    
    

    if ( IsTOCBookmark() )
    {
        maName = IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix();
        maName += rNa;
    }
}

SwFltBookmark::SwFltBookmark(const SwFltBookmark& rCpy)
    : SfxPoolItem( RES_FLTR_BOOKMARK )
    , mnHandle( rCpy.mnHandle )
    , maName( rCpy.maName )
    , maVal( rCpy.maVal )
    , mbIsTOCBookmark( rCpy.mbIsTOCBookmark )
{
}

bool SwFltBookmark::operator==(const SfxPoolItem& rItem) const
{
    return ( maName == ((SwFltBookmark&)rItem).maName)
            && (mnHandle == ((SwFltBookmark&)rItem).mnHandle);
}

SfxPoolItem* SwFltBookmark::Clone(SfxItemPool*) const
{
    return new SwFltBookmark(*this);
}


SwFltTOX::SwFltTOX(SwTOXBase* pBase, sal_uInt16 _nCols)
    : SfxPoolItem(RES_FLTR_TOX), pTOXBase(pBase), nCols( _nCols ),
      bHadBreakItem( sal_False ), bHadPageDescItem( sal_False )
{
}

SwFltTOX::SwFltTOX(const SwFltTOX& rCpy)
    : SfxPoolItem(RES_FLTR_TOX), pTOXBase(rCpy.pTOXBase), nCols( rCpy.nCols ),
      bHadBreakItem( rCpy.bHadBreakItem ), bHadPageDescItem( rCpy.bHadPageDescItem )
{
}

bool SwFltTOX::operator==(const SfxPoolItem& rItem) const
{
    return pTOXBase == ((SwFltTOX&)rItem).pTOXBase;
}

SfxPoolItem* SwFltTOX::Clone(SfxItemPool*) const
{
    return new SwFltTOX(*this);
}


SwFltSection::SwFltSection(SwSectionData *const pSect)
    : SfxPoolItem(RES_FLTR_SECTION)
    , m_pSection(pSect)
{
}

SwFltSection::SwFltSection(const SwFltSection& rCpy)
    : SfxPoolItem(RES_FLTR_SECTION)
    , m_pSection(rCpy.m_pSection)
{
}

bool SwFltSection::operator==(const SfxPoolItem& rItem) const
{
    return m_pSection == ((SwFltSection&)rItem).m_pSection;
}

SfxPoolItem* SwFltSection::Clone(SfxItemPool*) const
{
    return new SwFltSection(*this);
}




//

SwFltShell::SwFltShell(SwDoc* pDoc, SwPaM& rPaM, const OUString& rBaseURL, sal_Bool bNew, sal_uLong nFieldFl) :
    pCurrentPageDesc(0),
    pSavedPos(0),
    eSubMode(None),
    nAktStyle(0),
    aStack(pDoc, nFieldFl),
    aEndStack(pDoc, nFieldFl),
    pPaM(new SwPaM(*(rPaM.GetPoint()))),
    sBaseURL(rBaseURL),
    nPageDescOffset(GetDoc().GetPageDescCnt()),
    eSrcCharSet(RTL_TEXTENCODING_MS_1252),
    bNewDoc(bNew),
    bStdPD(sal_False),
    bProtect(sal_False)
{
    memset( pColls, 0, sizeof( pColls ) );
    pOutDoc = new SwFltOutDoc( *pDoc, pPaM, aStack, aEndStack );
    pOut = pOutDoc;

    if( !bNewDoc ){     
                        
                        
        const SwPosition* pPos = pPaM->GetPoint();
        const SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();
        if (pPos->nContent.GetIndex() && !pSttNd->GetTxt().isEmpty())
                                            
            pDoc->SplitNode( *pPos, false );        
        if (!pSttNd->GetTxt().isEmpty())
        {   
            pDoc->SplitNode( *pPos, false );        
            pPaM->Move( fnMoveBackward );   
        }

        
        sal_uLong nNd = pPos->nNode.GetIndex();
        bool bReadNoTbl = 0 != pSttNd->FindTableNode() ||
            ( nNd < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
            pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );
        if( bReadNoTbl )
            pOutDoc->SetReadNoTable();
    }
    pCurrentPageDesc =  &pDoc->GetPageDesc( 0 );  

}

SwFltShell::~SwFltShell()
{
    sal_uInt16 i;

    if (eSubMode == Style)
        EndStyle();
    if( pOutDoc->IsInTable() )          
        EndTable();
    if( pOutDoc->IsInFly() )
        EndFly();

    GetDoc().SetUpdateExpFldStat(true);
    GetDoc().SetInitDBFields(sal_True);
    aStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
    aStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
    aEndStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
    aEndStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
    if( bProtect ){     

        SwDoc& rDoc = GetDoc();
                        
        SwSectionFmt* pSFmt = rDoc.MakeSectionFmt( 0 );
        SwSectionData aSectionData(CONTENT_SECTION, OUString("PMW-Protect"));
        aSectionData.SetProtectFlag( true );
                        
        const SwNode* pEndNd = &rDoc.GetNodes().GetEndOfContent();
        SwNodeIndex aEndIdx( *pEndNd, -1L );
        const SwStartNode* pSttNd = pEndNd->StartOfSectionNode();
        SwNodeIndex aSttIdx( *pSttNd, 1L );         
                                                    
                        
        rDoc.GetNodes().InsertTextSection(
                aSttIdx, *pSFmt, aSectionData, 0, &aEndIdx, false );

        if( !IsFlagSet(SwFltControlStack::DONT_HARD_PROTECT) ){
            SwDocShell* pDocSh = rDoc.GetDocShell();
            if( pDocSh )
                pDocSh->SetReadOnlyUI( sal_True );
        }
    }
        
        

    GetDoc().ChgPageDesc( 0, GetDoc().GetPageDesc( 0 ));    
    for (i=nPageDescOffset;i<GetDoc().GetPageDescCnt();i++)
    {
        const SwPageDesc& rPD = GetDoc().GetPageDesc(i);
        GetDoc().ChgPageDesc(i, rPD);
    }

    delete pPaM;
    for (i=0; i<sizeof(pColls)/sizeof(*pColls); i++)
        delete pColls[i];
    delete pOutDoc;
}

SwFltShell& SwFltShell::operator << ( const OUString& rStr )
{
    OSL_ENSURE(eSubMode != Style, "char insert while in style-mode");
    GetDoc().InsertString( *pPaM, rStr );
    return *this;
}

OUString SwFltShell::ConvertUStr(const OUString& rInOut)
{
    return GetAppCharClass().uppercase(rInOut);
}


OUString SwFltShell::QuoteStr( const OUString& rIn )
{
    OUStringBuffer sOut( rIn );
    sal_Bool bAllowCr = aStack.IsFlagSet( SwFltControlStack::ALLOW_FLD_CR );

    for( sal_Int32 n = 0; n < sOut.getLength(); ++n )
    {
        switch( sOut[ n ] )
        {
        case 0x0a:
            sOut.remove( n, 1 );             
            break;

        case 0x0b:
        case 0x0c:
        case 0x0d:
            if( bAllowCr )
                sOut[n] = '\n';
            break;
        }
    }
    return sOut.makeStringAndClear();
}

SwFltShell& SwFltShell::operator << ( const sal_Unicode c )
{
    OSL_ENSURE( eSubMode != Style, "char insert while in style-mode");
    GetDoc().InsertString( *pPaM, OUString(c) );
    return *this;
}

SwFltShell& SwFltShell::AddError( const sal_Char* pErr )
{
    OUString aName("ErrorTag");
    SwFieldType* pFT = GetDoc().GetFldType( RES_SETEXPFLD, aName, false );
    if( pFT == 0)
    {
        SwSetExpFieldType aS(&GetDoc(), aName, nsSwGetSetExpType::GSE_STRING);
        pFT = GetDoc().InsertFldType(aS);
    }
    SwSetExpField aFld( (SwSetExpFieldType*)pFT,
                        OUString::createFromAscii( pErr ));
    
    GetDoc().InsertPoolItem(*pPaM, SwFmtFld(aFld), 0);
    return *this;
}

SwFltShell& SwFltShell::operator << (Graphic& rGraphic)
{
    
    GetDoc().Insert(*pPaM, OUString(), OUString(), &rGraphic, NULL, NULL, NULL);
    return *this;
}

void SwFltShell::NextParagraph()
{
    GetDoc().AppendTxtNode(*pPaM->GetPoint());
}

void SwFltShell::NextPage()
{
    NextParagraph();
    GetDoc().InsertPoolItem(*pPaM,
        SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK), 0);
}

SwFltShell& SwFltShell::AddGraphic( const OUString& rPicName )
{
    
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    
    
    
    INetURLObject aDir(
        URIHelper::SmartRel2Abs(
            INetURLObject(GetBaseURL()), rPicName,
            URIHelper::GetMaybeFileHdl()) );
    switch (rFilter.ImportGraphic(aGraphic, aDir))
    {
        case GRFILTER_OK:
            *this << aGraphic;
            break;
        case GRFILTER_OPENERROR:
        case GRFILTER_IOERROR:
        case GRFILTER_FORMATERROR:
        case GRFILTER_VERSIONERROR:
        case GRFILTER_FILTERERROR:
        case GRFILTER_ABORT:
        case GRFILTER_TOOBIG:
        default:
            AddError( "picture import error" );
            break;
    }
    return *this;
}

SwFltShell& SwFltShell::SetStyle( sal_uInt16 nStyle )
{
    SwFltFormatCollection* p = pColls[ nStyle ];

    if (p)
    {
        if( !pOutDoc->IsInTable() && nStyle != nAktStyle )
        {
            if( pColls[nAktStyle]->IsInFly() && pOutDoc->IsInFly() )
                pOutDoc->EndFly();
            if( p->IsInFly() )
                p->BeginStyleFly( pOutDoc );
        }
        GetDoc().SetTxtFmtColl(*pPaM, p->GetColl());
        nAktStyle = nStyle;
    }
    else
    {
        OSL_FAIL( "Invalid SwFltStyleCode" );
    }
    return *this;
}

SwFltShell& SwFltShell::operator << (SwFltBookmark& aBook)
{
    aBook.maName = ConvertUStr( aBook.maName );
    aBook.maVal = QuoteStr(aBook.maVal);
    aEndStack.NewAttr(*pPaM->GetPoint(), aBook);
    return *this;
}

void SwFltShell::SetBookEnd(long nHandle)
{
    aEndStack.SetAttr( *pPaM->GetPoint(), RES_FLTR_BOOKMARK, sal_True, nHandle );
}

SwFltShell& SwFltShell::EndItem( sal_uInt16 nAttrId )
{
    switch( nAttrId )
    {
    case RES_FLTR_BOOKMARK:
        OSL_FAIL( "Wrong invocation for bookmark end" );
        break;

    case RES_FLTR_TOX:
        aEndStack.SetAttr(*pPaM->GetPoint(), nAttrId);
        break;

    default:
        aStack.SetAttr(*pPaM->GetPoint(), nAttrId);
        break;
    }
    return *this;
}

SwFltShell& SwFltShell::operator << (const SwField& rField)
{
    GetDoc().InsertPoolItem(*pPaM, SwFmtFld(rField), 0);
    return *this;
}

/*virtual*/ SwFltOutBase& SwFltOutDoc::operator << (const SfxPoolItem& rItem)
{
    rStack.NewAttr(*pPaM->GetPoint(), rItem);
    return *this;
}

/*virtual*/ SwFltOutBase& SwFltFormatCollection::operator <<
                                (const SfxPoolItem& rItem)
{
    pColl->SetFmtAttr(rItem);
    return *this;
}

const SfxPoolItem& SwFltOutDoc::GetAttr(sal_uInt16 nWhich)
{
    return *rStack.GetFmtAttr(*pPaM->GetPoint(), nWhich);
}

const SfxPoolItem& SwFltFormatCollection::GetAttr(sal_uInt16 nWhich)
{
    return GetColl()->GetFmtAttr(nWhich);   
}





const SfxPoolItem& SwFltOutDoc::GetNodeOrStyAttr(sal_uInt16 nWhich)
{
    SwCntntNode * pNd = pPaM->GetPoint()->nNode.GetNode().GetCntntNode();
    if (pNd)            
        return pNd->GetAttr(nWhich);
    else                
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

const SfxPoolItem& SwFltFormatCollection::GetNodeOrStyAttr(sal_uInt16 nWhich)
{
    return GetColl()->GetFmtAttr(nWhich);   
}

const SfxPoolItem& SwFltShell::GetNodeOrStyAttr(sal_uInt16 nWhich)
{
    return pOut->GetNodeOrStyAttr( nWhich );
}

const SfxPoolItem& SwFltShell::GetAttr(sal_uInt16 nWhich)
{
    return pOut->GetAttr( nWhich );
}

const SfxPoolItem& SwFltShell::GetFlyFrmAttr(sal_uInt16 nWhich)
{
    return pOut->GetFlyFrmAttr( nWhich );
}

SwFieldType* SwFltShell::GetSysFldType(sal_uInt16 eWhich)
{
    return GetDoc().GetSysFldType(eWhich);
}

bool SwFltShell::GetWeightBold()
{
    return ((SvxWeightItem&)GetNodeOrStyAttr(RES_CHRATR_WEIGHT)).GetWeight()
                                != WEIGHT_NORMAL;
}

bool SwFltShell::GetPostureItalic()
{
    return ((SvxPostureItem&)GetNodeOrStyAttr(RES_CHRATR_POSTURE)).GetPosture()
                                != ITALIC_NONE;
}

bool SwFltShell::GetCrossedOut()
{
    return ((SvxCrossedOutItem&)GetNodeOrStyAttr(RES_CHRATR_CROSSEDOUT))
                                    .GetStrikeout() != STRIKEOUT_NONE;
}

bool SwFltShell::GetContour()
{
    return ((SvxContourItem&)GetNodeOrStyAttr(RES_CHRATR_CONTOUR)).GetValue();
}

bool SwFltShell::GetCaseKapitaelchen()
{
    return ((SvxCaseMapItem&)GetNodeOrStyAttr(RES_CHRATR_CASEMAP))
                                    .GetCaseMap() == SVX_CASEMAP_KAPITAELCHEN;
}

bool SwFltShell::GetCaseVersalien()
{
    return ((SvxCaseMapItem&)GetNodeOrStyAttr(RES_CHRATR_CASEMAP))
                                    .GetCaseMap() == SVX_CASEMAP_VERSALIEN;
}


SwFltOutBase::~SwFltOutBase()
{
}

SwFltOutBase::SwFltOutBase(SwDoc& rDocu)
    : rDoc(rDocu), eFlyAnchor(FLY_AT_PARA), bFlyAbsPos(false)
{
}

const SfxPoolItem& SwFltOutBase::GetCellAttr(sal_uInt16 nWhich)
{
    OSL_FAIL("GetCellAttr outside of normal text");
    return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

bool SwFltOutBase::BeginTable()
{
    OSL_FAIL("BeginTable outside of normal text");
    return false;
}

void SwFltOutBase::NextTableCell()
{
    OSL_FAIL("NextTableCell outside of normal text");
}

void SwFltOutBase::NextTableRow()
{
    OSL_FAIL("NextTableRow outside of normal text");
}

void SwFltOutBase::SetTableWidth(SwTwips /*nW*/)
{
    OSL_FAIL("SetTableWidth outside of normal text");
}

void SwFltOutBase::SetTableOrient(sal_Int16 /*eOri*/)
{
    OSL_FAIL("SetTableOrient outside of normal text");
}

void SwFltOutBase::SetCellWidth(SwTwips /*nWidth*/, sal_uInt16 /*nCell*/)
{
    OSL_FAIL("SetCellWidth outside of normal text");
}

void SwFltOutBase::SetCellHeight(SwTwips /*nH*/)
{
    OSL_FAIL("SetCellHeight outside of normal text");
}

void SwFltOutBase::SetCellBorder(const SvxBoxItem& /*rFmtBox*/, sal_uInt16 /*nCell*/)
{
    OSL_FAIL("SetCellBorder outside of normal text");
}

void SwFltOutBase::SetCellSpace(sal_uInt16 /*nSp*/)
{
    OSL_FAIL("SetCellSpace outside of normal text");
}

void SwFltOutBase::DeleteCell(sal_uInt16 /*nCell*/)
{
    OSL_FAIL("DeleteCell outside of normal text");
}

void SwFltOutBase::EndTable()
{
    OSL_FAIL("EndTable outside of normal text");
}

/*virtual*/ sal_Bool SwFltOutDoc::IsInTable()
{
    return pTable != 0;
};

bool SwFltOutDoc::BeginTable()
{
    if(bReadNoTbl)
        return false;

    if (pTable){
        OSL_FAIL("BeginTable in Table");
        return false;
    }
                            
                            
    rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );


    OSL_ENSURE(pTabSavedPos == NULL, "SwFltOutDoc");
    pTabSavedPos = new SwPosition(*pPaM->GetPoint());
    pTable = GetDoc().InsertTable(
            SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 1 ),
            *pTabSavedPos, 1, 1, text::HoriOrientation::LEFT, 0, 0, sal_False, sal_False ); 
    nTableWidth = 0;
    ((SwTable*)pTable)->LockModify();   

    usTableX =
    usTableY = 0;
    SeekCell(usTableY, usTableX, sal_True);
    return true;
}

SwTableBox* SwFltOutDoc::GetBox(sal_uInt16 ny, sal_uInt16 nx /*= USHRT_MAX */)
{
    if(!pTable){
        OSL_ENSURE(pTable, "GetBox without table");
        return 0;
    }
    if( nx == USHRT_MAX )   
        nx = usTableX;

    
    const SwTableLines* pTableLines = &pTable->GetTabLines();
    if(!pTableLines){
        OSL_FAIL("SwFltOutDoc:GetBox:pTableLines");
        return 0;
    }
    if( ny >= pTableLines->size() ){   
        OSL_FAIL( "SwFltOutDoc:GetBox:ny >= Count()");
        ny = pTableLines->size() - 1;
    }
    SwTableLine* pTableLine = (*pTableLines)[ny];
    if(!pTableLine){
        OSL_FAIL("SwFltOutDoc:GetBox:pTableLine");
        return 0;
    }
    SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
    if(!pTableBoxes){
        OSL_FAIL("SwFltOutDoc:GetBox:pTableBoxes");
        return 0;
    }
    if( nx >= pTableBoxes->size() ){   
        OSL_FAIL("SwFltOutDoc:GetBox:nx >= Count()");
        nx = pTableBoxes->size() - 1;
    }
    SwTableBox* pTableBox = (*pTableBoxes)[nx];

    OSL_ENSURE(pTableBox != 0, "SwFltOutDoc:GetBox:pTableBox");
    return pTableBox;
}

void SwFltOutDoc::NextTableCell()
{
    if(!pTable){
        OSL_ENSURE(pTable, "NextTableCell without table");
        return;
    }
    const SwTableLines* pTableLines = &pTable->GetTabLines();
    SwTableLine* pTableLine = (*pTableLines)[usTableY];
    SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
    SwTableBox* pTableBox = (*pTableBoxes)[usTableX];
    OSL_ENSURE(pTableBox != 0, "SwFltOutDoc:NextTableCell:pTableBox");
    if(!pTableBox)
        return;
    
    
    if (++usTableX >= pTableBoxes->size())
        GetDoc().GetNodes().InsBoxen(
         GetDoc().IsIdxInTbl(pPaM->GetPoint()->nNode),
         pTableLine,
         (SwTableBoxFmt*)pTableBox->GetFrmFmt(),
         GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ),
         0,
         pTableBoxes->size());
    SeekCell(usTableY, usTableX, sal_True);
    pTableBox = (*pTableBoxes)[usTableX];
    OSL_ENSURE(pTableBox != 0, "SwFltOutDoc:pTableBox");
    if(pTableBox)
        (*pTableBoxes)[usTableX]->ClaimFrmFmt();
}

void SwFltOutDoc::NextTableRow()
{
    SwTableBox* pTableBox = GetBox(usTableY, 0);
    if (pTableBox)
    {
        
        SwSelBoxes aSelBoxes;
        aSelBoxes.insert( pTableBox );
        GetDoc().InsertRow(aSelBoxes);
        usTableX = 0;
        SeekCell(++usTableY, usTableX, sal_True);
        GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ));
    }
}

void SwFltOutDoc::SetTableWidth(SwTwips nSwWidth)
{
    if(!pTable){
        OSL_ENSURE(pTable, "SetTableWidth without table");
        return;
    }
    OSL_ENSURE( nSwWidth > MINLAY, "Table width <= MINLAY" );
    if( nSwWidth != nTableWidth ){
        if( nTableWidth )           
            SplitTable();
        pTable->GetFrmFmt()->SetFmtAttr( SwFmtFrmSize(ATT_VAR_SIZE, nSwWidth));
        nTableWidth = nSwWidth;
    }
}

void SwFltOutDoc::SetTableOrient(sal_Int16 eOri)
{
    if(!pTable){
        OSL_ENSURE(pTable, "SetTableOrient without table");
        return;
    }
    pTable->GetFrmFmt()->SetFmtAttr( SwFmtHoriOrient( 0, eOri ));
}

void SwFltOutDoc::SetCellWidth(SwTwips nWidth, sal_uInt16 nCell /* = USHRT_MAX */ )
{
    if(!pTable){
        OSL_ENSURE(pTable, "SetCellWidth without table");
        return;
    }
    OSL_ENSURE( nWidth > MINLAY, "Table cell width <= MINLAY" );
    if (nWidth < MINLAY)
        nWidth = MINLAY;

    SwTableBox* pTableBox = GetBox(usTableY, nCell);
    if(pTableBox && pTableBox->GetFrmFmt() ){
        SwFmtFrmSize aFmtFrmSize(ATT_FIX_SIZE);
        aFmtFrmSize.SetWidth(nWidth);
        pTableBox->GetFrmFmt()->SetFmtAttr(aFmtFrmSize);
    }
}

void SwFltOutDoc::SetCellHeight(SwTwips nHeight)
{
    if(!pTable){
        OSL_ENSURE(pTable, "SetCellHeight without table");
        return;
    }

    const SwTableLines* pTableLines = &pTable->GetTabLines();
    SwTableLine* pTableLine = (*pTableLines)[usTableY];
    SwFmtFrmSize aFmtFrmSize(ATT_MIN_SIZE, 0, 0);
    if (nHeight < MINLAY)
        nHeight = MINLAY;
    aFmtFrmSize.SetHeight(nHeight);
    pTableLine->GetFrmFmt()->SetFmtAttr(aFmtFrmSize);
}

const SfxPoolItem& SwFltOutDoc::GetCellAttr(sal_uInt16 nWhich)
{
    if (!pTable){
        OSL_ENSURE(pTable, "GetCellAttr without table");
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    }

    SwTableBox* pTableBox = GetBox(usTableY, usTableX);
    if(!pTableBox)
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    return pTableBox->GetFrmFmt()->GetFmtAttr( nWhich );
}

void SwFltOutDoc::SetCellBorder(const SvxBoxItem& rFmtBox,
                               sal_uInt16 nCell /* = USHRT_MAX */ )
{
    SwTableBox* pTableBox = GetBox(usTableY, nCell);
    if(pTableBox)
        pTableBox->GetFrmFmt()->SetFmtAttr(rFmtBox);
}


void SwFltOutDoc::SetCellSpace(sal_uInt16 nDist)
{
    if(!pTable){
        OSL_ENSURE(pTable, "SetCellSpace without table");
        return;
    }
    SwTableBox* pTableBox = GetBox(usTableY, usTableX);
    if(!pTableBox)
        return;

    SvxBoxItem aFmtBox( *((SvxBoxItem*)
                        &pTableBox->GetFrmFmt()->GetFmtAttr( RES_BOX )));

    
    if (nDist > 42) 
        nDist = 42;
    else
        if (nDist < MIN_BORDER_DIST)
            nDist = MIN_BORDER_DIST;
    aFmtBox.SetDistance(nDist);
    pTableBox->GetFrmFmt()->SetFmtAttr(aFmtBox);
}

void SwFltOutDoc::DeleteCell(sal_uInt16 nCell /* = USHRT_MAX */)
{
    SwTableBox* pTableBox = GetBox(usTableY, nCell);
    if( pTableBox )
    {
        SwSelBoxes aSelBoxes;
        aSelBoxes.insert( pTableBox );
        GetDoc().DeleteRowCol(aSelBoxes);
        usTableX--;
    }
}

void SwFltOutDoc::SplitTable()
{
    if(!pTable)
    {
        OSL_ENSURE(pTable, "SplitTable without table");
        return;
    }
    SwTableBox* pAktBox = GetBox(usTableY, usTableX);
    SwTableBox* pSplitBox = GetBox(usTableY - 1, 0);
    GetDoc().GetNodes().SplitTable(SwNodeIndex(*pSplitBox->GetSttNd()), false);
    pTable = &pAktBox->GetSttNd()->FindTableNode()->GetTable();
    usTableY = 0;
}

void SwFltOutDoc::EndTable()
{
    if (!pTable){
        OSL_ENSURE(pTable, "EndTable without table");
        return;
    }
    
    
    rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

    if (GetDoc().GetCurrentViewShell()){
        SwTableNode* pTableNode = GetDoc().IsIdxInTbl(
            pPaM->GetPoint()->nNode);
        pTableNode->DelFrms();
        pTableNode->MakeFrms(&pPaM->GetPoint()->nNode);
    }

    *pPaM->GetPoint() = *pTabSavedPos;              
    delete pTabSavedPos;
    pTabSavedPos = 0;
    ((SwTable*)pTable)->UnlockModify(); 
    pTable = 0;
    nTableWidth = 0;
}

sal_Bool SwFltOutDoc::SeekCell(short nRow, short nCol, sal_Bool bPam)
{
    
    const SwTableLines* pTableLines = &pTable->GetTabLines();
    SwTableLine* pTableLine = (*pTableLines)[usTableY];
    SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
    SwTableBox* pTableBox = (*pTableBoxes)[usTableX];

    if ((sal_uInt16)nRow >= pTableLines->size())
    {
       OSL_ENSURE((sal_uInt16)nRow >= pTableLines->size(), "SwFltOutDoc");
        return sal_False;
    }
    pTableLine = (*pTableLines)[nRow];
    pTableBoxes = &pTableLine->GetTabBoxes();
    if (nCol >= (short)pTableBoxes->size())
        return sal_False;
    pTableBox = (*pTableBoxes)[nCol];
    if( !pTableBox->GetSttNd() )
    {
        OSL_ENSURE(pTableBox->GetSttNd(), "SwFltOutDoc");
        return sal_False;
    }
    if(bPam)
    {
        pPaM->GetPoint()->nNode = pTableBox->GetSttIdx() + 1;
        pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
        GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ));
    }
    return sal_True;
}


SfxItemSet* SwFltOutBase::NewFlyDefaults()
{
    
    
    SfxItemSet* p = new SfxItemSet( GetDoc().GetAttrPool(),
                                    RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    SwFmtFrmSize aSz( ATT_VAR_SIZE, MINFLY, MINFLY );
                                        
    aSz.SetWidthPercent( 100 );         
    p->Put( aSz );
    p->Put( SwFmtHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::FRAME ));
    return p;
}

bool SwFltOutBase::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
                           sal_Bool bAbsolutePos /*= sal_False*/,
                           const SfxItemSet* pMoreAttrs /*= 0*/)
{
    (void) pMoreAttrs; 
    OSL_ENSURE(!pMoreAttrs, "SwFltOutBase:BeginFly with pMoreAttrs" );
    eFlyAnchor = eAnchor;
    bFlyAbsPos = bAbsolutePos;      
    return true;
}

/*virtual*/ void SwFltOutBase::SetFlyAnchor( RndStdIds eAnchor )
{
    if( !IsInFly() ){
        OSL_FAIL( "SetFlyAnchor() without Fly" );
        return;
    }
    if ( eAnchor == FLY_AS_CHAR ){
        OSL_FAIL( "SetFlyAnchor( FLY_AS_CHAR ) not implemented" );
        return;
    }
    SwFmtAnchor& rAnchor = (SwFmtAnchor&)GetFlyFrmAttr( RES_ANCHOR );
    rAnchor.SetType( eAnchor );
}

void SwFltOutBase::EndFly()
{
    if( bFlyAbsPos ){
        
        
    }
}


/* virtual */ bool SwFltOutDoc::IsInFly()
{
    return pFly != 0;
};

SwFrmFmt* SwFltOutDoc::MakeFly( RndStdIds eAnchor, SfxItemSet* pSet )
{
    pFly = (SwFlyFrmFmt*)GetDoc().MakeFlySection( eAnchor, pPaM->GetPoint(),
                                                    pSet );
    return pFly;
}

bool SwFltOutDoc::BeginFly( RndStdIds eAnchor,
                           sal_Bool bAbsolutePos ,
                           const SfxItemSet* pMoreAttrs)

{
    SwFltOutBase::BeginFly( eAnchor, bAbsolutePos, 0 );
    SfxItemSet* pSet = NewFlyDefaults();

    
    
    rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

    
    OSL_ENSURE(pFlySavedPos == NULL, "BeginFly in Fly");    
    pFlySavedPos = new SwPosition(*pPaM->GetPoint());


    SwFmtAnchor aAnchor( eAnchor, 1 );

    
    if (pMoreAttrs)
        pSet->Put(*pMoreAttrs);

    
    aAnchor.SetAnchor(pPaM->GetPoint());    
                                            

    pSet->Put( aAnchor );
    SwFrmFmt* pF = MakeFly( eAnchor, pSet );
    delete pSet;

    
    const SwFmtCntnt& rCntnt = pF->GetCntnt();
    OSL_ENSURE( rCntnt.GetCntntIdx(), "No prepared content." );
    pPaM->GetPoint()->nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
    SwCntntNode *pNode = pPaM->GetCntntNode();
    pPaM->GetPoint()->nContent.Assign( pNode, 0 );

    return true;
}

/*virtual*/ void SwFltOutDoc::SetFlyFrmAttr(const SfxPoolItem& rAttr)
{
    if (pFly){
        pFly->SetFmtAttr( rAttr );
    }else{
        OSL_ENSURE(pFly, "SetFlyAttr without Doc-Fly");
        return;
    }
}

/*virtual*/ const SfxPoolItem& SwFltOutDoc::GetFlyFrmAttr(sal_uInt16 nWhich)
{
    if (pFly){
        return pFly->GetFmtAttr( nWhich );
    }else{
        OSL_ENSURE(pFly, "GetFlyAttr without Fly");
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    }
}

void SwFltOutDoc::EndFly()
{
    if( pTable ){
        OSL_FAIL( "SwFltOutDoc::EndFly() in Table" );
        return;
    }
    
    
    rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

    *pPaM->GetPoint() = *pFlySavedPos;              
    delete pFlySavedPos;
    pFlySavedPos = 0;
    SwFltOutBase::EndFly();
    pFly = 0;
}

/*virtual*/ bool SwFltFormatCollection::IsInFly()
{
    return bHasFly;
};

/*virtual*/ void SwFltFormatCollection::SetFlyFrmAttr(const SfxPoolItem& rAttr)
{
    if (!pFlyAttrs)
        pFlyAttrs = new SfxItemSet( GetDoc().GetAttrPool(),
                             RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    pFlyAttrs->Put( rAttr );
}

/*virtual*/ const SfxPoolItem& SwFltFormatCollection::GetFlyFrmAttr(sal_uInt16 nWhich)
{
    if( pFlyAttrs )
        return pFlyAttrs->Get( nWhich, sal_False );
    else
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

bool SwFltFormatCollection::BeginFly( RndStdIds eAnchor,
                           sal_Bool bAbsolutePos,
                           const SfxItemSet* pMoreAttrs)

{
    SwFltOutBase::BeginFly( eAnchor, bAbsolutePos, pMoreAttrs );
    bHasFly = true;
    return true;
}

void SwFltFormatCollection::EndFly()    
{
}

bool SwFltFormatCollection::BeginStyleFly( SwFltOutDoc* pOutDoc )
{
    OSL_ENSURE( pOutDoc, "BeginStyleFly without pOutDoc" );
    OSL_ENSURE( pOutDoc && !pOutDoc->IsInFly(), "BeginStyleFly in Fly" );
    if( pOutDoc && !pOutDoc->IsInFly() )
        return pOutDoc->BeginFly( eFlyAnchor, bFlyAbsPos, pFlyAttrs );
    else
        return false;
}

bool SwFltShell::BeginFly( RndStdIds eAnchor,
                           sal_Bool bAbsolutePos)
{
    if (pOut->IsInFly()){
        OSL_FAIL("BeginFly in Fly");
        return false;
    }
    if (pOutDoc->IsInTable()){
        OSL_FAIL("BeginFly in Table");
        return false;
    }
    pOut->BeginFly( eAnchor, bAbsolutePos, pColls[nAktStyle]->GetpFlyAttrs() );
    eSubMode = Fly;
    return true;
}

void SwFltShell::SetFlyXPos( short nXPos, sal_Int16 eHRel,
                             sal_Int16 eHAlign)
{
    SetFlyFrmAttr( SwFmtHoriOrient( nXPos, eHAlign, eHRel ) );
}

void SwFltShell::SetFlyYPos( short nYPos, sal_Int16 eVRel,
                             sal_Int16 eVAlign)
{
    SetFlyFrmAttr( SwFmtVertOrient( nYPos, eVAlign, eVRel ) );
}


void SwFltShell::EndFly()
{
    if (!pOut->IsInFly()){
        OSL_FAIL("EndFly without Fly");
        return;
    }
    if (pOutDoc->IsInTable()){      
        OSL_FAIL("EndFly in Table ( intermingled )");
        EndTable();
    }
    pOut->EndFly();
    eSubMode = None;
}

void SwFltShell::BeginFootnote()
{
    if( pOut->IsInFly() ){          
        OSL_FAIL("Footnote in Fly not permitted");
        return;
    }
    if( pOutDoc->IsInTable() ){
        OSL_FAIL("Footnote in table currently not allowed");
        return;
    }

    
    
    aStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    
    

    SwFmtFtn aFtn;
    GetDoc().InsertPoolItem(*pPaM, aFtn, 0);
    OSL_ENSURE(pSavedPos == NULL, "SwFltShell");
    pSavedPos = new SwPosition(*pPaM->GetPoint());
    pPaM->Move(fnMoveBackward, fnGoCntnt);
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();
    SwTxtAttr *const pFN = pTxt->GetTxtAttrForCharAt(
        pPaM->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    if( !pFN ){         
        OSL_ENSURE(pFN, "Problems with creating footnote text");
        return;
    }
    const SwNodeIndex* pStartIndex = ((SwTxtFtn*)pFN)->GetStartNode();
    OSL_ENSURE(pStartIndex, "Problems with creating footnote text");
    pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
    eSubMode = Footnote;
}

void SwFltShell::EndFootnote()
{
    if(!pSavedPos)
        return;
    
    
    
    aStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    
    

    *pPaM->GetPoint() = *pSavedPos;             
    delete pSavedPos;
    pSavedPos = 0;
}

void SwFltShell::BeginHeader(SwPageDesc* /*pPD*/)
{
    SwFrmFmt* pFmt = &pCurrentPageDesc->GetMaster();
    SwFrmFmt* pHdFtFmt;
    pFmt->SetFmtAttr(SwFmtHeader(sal_True));
    pHdFtFmt = (SwFrmFmt*)pFmt->GetHeader().GetHeaderFmt();
    const SwNodeIndex* pStartIndex = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pStartIndex)
        return;
    OSL_ENSURE(pSavedPos == NULL, "SwFltShell");
    pSavedPos = new SwPosition(*pPaM->GetPoint());
    pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
    eSubMode = Header;
}

void SwFltShell::BeginFooter(SwPageDesc* /*pPD*/)
{
    SwFrmFmt* pFmt =  &pCurrentPageDesc->GetMaster();
    SwFrmFmt* pHdFtFmt;
    pFmt->SetFmtAttr(SwFmtFooter(sal_True));
    pHdFtFmt = (SwFrmFmt*)pFmt->GetFooter().GetFooterFmt();
    const SwNodeIndex* pStartIndex = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pStartIndex)
        return;
    OSL_ENSURE(pSavedPos == NULL, "SwFltShell");
    pSavedPos = new SwPosition(*pPaM->GetPoint());
    pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
    eSubMode = Footer;
}

void SwFltShell::EndHeaderFooter()
{
    *pPaM->GetPoint() = *pSavedPos;             
    delete pSavedPos;
    pSavedPos = 0;
}

SwPageDesc* SwFltShell::MakePageDesc(SwPageDesc* pFirstPageDesc)
{
    if(bStdPD)                      
        return pCurrentPageDesc;

    bool bFollow = (pFirstPageDesc != 0);
    SwPageDesc* pNewPD;
    sal_uInt16 nPos;
    if (bFollow && pFirstPageDesc->GetFollow() != pFirstPageDesc)
        return pFirstPageDesc;      
    
    

    nPos = GetDoc().MakePageDesc( SwViewShell::GetShellRes()->GetPageDescName(
                                   GetDoc().GetPageDescCnt(), bFollow ? ShellResource::FOLLOW_PAGE : ShellResource::NORMAL_PAGE),
                                pFirstPageDesc, false );

    pNewPD =  &GetDoc().GetPageDesc(nPos);
    if (bFollow)
    {
        
        pFirstPageDesc->SetFollow(pNewPD);
        pNewPD->SetFollow(pNewPD);
    }
    else
    {
        GetDoc().InsertPoolItem( *pPaM, SwFmtPageDesc( pNewPD ), 0 );
    }
    pNewPD->WriteUseOn( 
     (UseOnPage)(nsUseOnPage::PD_ALL | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE));
    return pNewPD;
}

SwFltFormatCollection::SwFltFormatCollection(
    SwDoc& _rDoc, RES_POOL_COLLFMT_TYPE nType ) :
    SwFltOutBase(_rDoc),
    pColl(_rDoc.GetTxtCollFromPool( static_cast< sal_uInt16 >(nType), false )),
    pFlyAttrs( 0 ),
    bHasFly( false )
{
    Reset();            
}

SwFltFormatCollection::SwFltFormatCollection(
    SwDoc& _rDoc, const OUString& rName ) :
    SwFltOutBase(_rDoc),
    pFlyAttrs( 0 ),
    bHasFly( false )
{
    pColl = _rDoc.MakeTxtFmtColl(rName, (SwTxtFmtColl*)_rDoc.GetDfltTxtFmtColl());
    Reset();            
}

void SwFltShell::NextStyle(sal_uInt16 nWhich, sal_uInt16 nNext)
{
        OSL_ENSURE(pColls[nWhich], "Next style for noexistent style" );
        OSL_ENSURE(pColls[nNext], "Next style to noexistent style" );
        if( pColls[nWhich] && pColls[nNext] )
            pColls[nWhich]->GetColl()->SetNextTxtFmtColl(
                 *pColls[nNext]->GetColl() );
}



void UpdatePageDescs(SwDoc &rDoc, sal_uInt16 nInPageDescOffset)
{
    
    

    
    rDoc.ChgPageDesc(0, rDoc.GetPageDesc(0));

    
    for (sal_uInt16 i = nInPageDescOffset; i < rDoc.GetPageDescCnt(); ++i)
        rDoc.ChgPageDesc(i, rDoc.GetPageDesc(i));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
