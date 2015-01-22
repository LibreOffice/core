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
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
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

// Stack entry for all text attributes
SwFltStackEntry::SwFltStackEntry(const SwPosition& rStartPos, SfxPoolItem* pHt)
    : m_aMkPos(rStartPos)
    , m_aPtPos(rStartPos)
    , mnStartCP(-1)
    , mnEndCP(-1)
    , bIsParaEnd(false)
{
    pAttr = pHt;            // store a copy of the attribute
    bOld    = false;    // used for marking Attributes *before* skipping field results
    bOpen = true;       // lock the attribute --> may first
    bConsumedByField = false;
}

SwFltStackEntry::~SwFltStackEntry()
{
    // Although attribute got passed as pointer, it gets deleted here
    delete pAttr;
}

void SwFltStackEntry::SetEndPos(const SwPosition& rEndPos)
{
    // Release attribute and keep track of end
    // Everything with sal_uInt16s, lest the inserting of new text at
    // the cursor position moves the attribute's range
    // That's not the desired behavior!
    bOpen = false;                  // release and remember END
    m_aPtPos.SetPos(rEndPos);
}

bool SwFltStackEntry::MakeRegion(SwDoc* pDoc, SwPaM& rRegion, bool bCheck,
    const SwFltPosition &rMkPos, const SwFltPosition &rPtPos, bool bIsParaEnd,
    sal_uInt16 nWhich)
{
    // does this range actually contain something?
    // empty range is allowed if at start of empty paragraph
    // fields are special: never have range, so leave them

    // The only position of 0x0D will not be able to make region in the old logic
    // because it is beyond the length of para...need special consideration here.
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
    // The content indices always apply to the node!
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

// MoveAttrs() is meant to address the following problem:
// When a field like "set variable" is set through the stack, the text
// is shifted by one \xff character, which makes all subsequent
// attribute positions invalid.
// After setting the attribute in the doc, MoveAttrs() needs to be
// called in order to push all attribute positions to the right in the
// same paragraph further out by one character.
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
        maEntries[i].bOld = true;
}

namespace
{
    bool couldExtendEntry(const SwFltStackEntry *pExtendCandidate,
        const SfxPoolItem& rAttr)
    {
        return (pExtendCandidate &&
                !pExtendCandidate->bConsumedByField &&
                //potentially more, but lets keep it simple
                (isPARATR_LIST(rAttr.Which()) || (isCHRATR(rAttr.Which()) && rAttr.Which() != RES_CHRATR_FONT && rAttr.Which() != RES_CHRATR_FONTSIZE)) &&
                *(pExtendCandidate->pAttr) == rAttr);
    }
}

void SwFltControlStack::NewAttr(const SwPosition& rPos, const SfxPoolItem& rAttr)
{
    sal_uInt16 nWhich = rAttr.Which();
    // Set end position of potentially equal attributes on stack, so
    // as to avoid having them accumulate
    SwFltStackEntry *pExtendCandidate = SetAttr(rPos, nWhich);
    if (couldExtendEntry(pExtendCandidate, rAttr))
    {
        //Here we optimize by seeing if there is an attribute uncommited
        //to the document which

        //a) has the same value as this attribute
        //b) is already open, or ends at the same place as where we're starting
        //from. If so we merge it with this one and elide adding another
        //to the stack
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
    //Clear the para end position recorded in reader intermittently for the least impact on loading performance
    //Because the attributes handled based on the unit of para
    if ( empty() )
    {
        ClearParaEndPosition();
        bHasSdOD = true;
        bSdODChecked = false;
    }
}

// SwFltControlStack::StealAttr() removes attributes of the given type
// from the stack. Allowed as nAttrId: 0 meaning any, or a specific
// type.  This makes them disappear from the doc structure. Only
// attributes from the same paragraph as rPos are removed. Used for
// graphic apos -> images.
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
            DeleteAndDestroy(nCnt);     // loesche aus dem Stack
        }
    }
}

// SwFltControlStack::KillUnlockedAttr() removes all attributes from
// the stack, which are assigned to an rPos. This makes them disappear
// from the doc structure. Used in WW import for ignoring attributes
// assigned to the 0x0c section break symbol.
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
            DeleteAndDestroy( nCnt ); // remove from stack
        }
    }
}

// Unlock all locked attributes and move to the end, all others will
// be applied to the document and removed from the stack.
// Returns if there were any selected attributes on the stack
SwFltStackEntry* SwFltControlStack::SetAttr(const SwPosition& rPos,
    sal_uInt16 nAttrId, bool bTstEnde, long nHand,
    bool consumedByField)
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
            // set end of attribute
            bool bF = false;
            if (!nAttrId )
            {
                bF = true;
            }
            else if (nAttrId == rEntry.pAttr->Which())
            {
                if( nAttrId != RES_FLTR_BOOKMARK && nAttrId != RES_FLTR_ANNOTATIONMARK )
                {
                    // query handle
                    bF = true;
                }
                else if (nAttrId == RES_FLTR_BOOKMARK && nHand == static_cast<SwFltBookmark*>(rEntry.pAttr)->GetHandle())
                {
                    bF = true;
                }
                else if (nAttrId == RES_FLTR_ANNOTATIONMARK && nHand == static_cast<CntUInt16Item*>(rEntry.pAttr)->GetValue())
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
                    //potential candidate for merging with an identical
                    //property beginning at rPos
                    pRet = &rEntry;
                }
            }
            ++aI;
            continue;
        }

        // if the end position is equal to the cursor position, then
        // refrain from applying it; there needs to be following text,
        // except at the very end. (attribute expansion !!)
        // Never apply end stack except at document ending
        if (bTstEnde)
        {
            if (bIsEndStack)
            {
                ++aI;
                continue;
            }

            //defer inserting this attribute into the document until
            //we advance to the next node, or finish processing the document
            if (rEntry.m_aPtPos.m_nNode.GetIndex() == aFltPos.m_nNode.GetIndex())
            {
                if (bLastEntry && nAttrId == rEntry.pAttr->Which() &&
                    rEntry.m_aPtPos.m_nCntnt == aFltPos.m_nCntnt)
                {
                    //potential candidate for merging with an identical
                    //property beginning at rPos
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
    // the anchor is the Pam's Point. It's modified when inserting
    // text, etc.; therefore it is kept on the stack. Only the
    // attribute's format needs to be set.
    rRegion.DeleteMark();
    rRegion.GetPoint()->nNode = rEntry.m_aMkPos.m_nNode.GetIndex() + 1;
    SwCntntNode* pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, true);
    rRegion.GetPoint()->nContent.Assign(pCNd, rEntry.m_aMkPos.m_nCntnt);
}

// MakeBookRegionOrPoint() behaves like MakeRegionOrPoint, except that
// it adheres to certain restrictions on bookmarks in tables (cannot
// span more than one cell)
static void MakeBookRegionOrPoint(const SwFltStackEntry& rEntry, SwDoc* pDoc,
                    SwPaM& rRegion, bool bCheck )
{
    if (rEntry.MakeRegion(pDoc, rRegion, bCheck )){
        // sal_Bool b1 = rNds[rRegion.GetPoint()->nNode]->FindTableNode() != 0;
        if (rRegion.GetPoint()->nNode.GetNode().FindTableBoxStartNode()
              != rRegion.GetMark()->nNode.GetNode().FindTableBoxStartNode())
        {
            rRegion.Exchange();         // invalid range
            rRegion.DeleteMark();       // -> both to mark
        }
    }else{
        MakePoint(rEntry, pDoc, rRegion);
    }
}

// IterateNumrulePiece() looks for the first range valid for Numrules
// between rTmpStart and rEnd.

// rNds denotes the doc nodes
// rEnd denotes the range end,
// rTmpStart is an in/out parameter: in: start of range to be searched,
//                                   out: start of valid range
// rTmpEnd is an out parameter
// Returns true for valid range
static bool IterateNumrulePiece( const SwNodeIndex& rEnd,
                                SwNodeIndex& rTmpStart, SwNodeIndex& rTmpEnd )
{
    while( ( rTmpStart <= rEnd )
           && !( rTmpStart.GetNode().IsTxtNode() ) )    // look for valid start
        ++rTmpStart;

    rTmpEnd = rTmpStart;
    while( ( rTmpEnd <= rEnd )
           && ( rTmpEnd.GetNode().IsTxtNode() ) )       // look for valid end + 1
        ++rTmpEnd;

    rTmpEnd--;                                      // valid end

    return rTmpStart <= rTmpEnd;                    // valid ?
}

//***This function will check whether there is existing individual attribute position for 0x0D***/
//The check will happen only once for a paragraph during loading
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
                // So the frames will be created when inserting into
                // existing doc (after setting the anchor!):
                if(pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()
                   && (FLY_AT_PARA == pFmt->GetAnchor().GetAnchorId()))
                {
                    pFmt->MakeFrms();
                }
            }
        }
        break;

    case RES_TXTATR_FIELD:
    case RES_TXTATR_ANNOTATION:
    case RES_TXTATR_INPUTFIELD:
        break;

    case RES_TXTATR_TOXMARK:
        break;

    case RES_FLTR_NUMRULE:          // insert Numrule
        {
            const OUString& rNumNm = ((SfxStringItem*)rEntry.pAttr)->GetValue();
            SwNumRule* pNumRule = pDoc->FindNumRulePtr( rNumNm );
            if( pNumRule )
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
                        // no start of a new list
                        pDoc->SetNumRule( aTmpPam, *pNumRule, false );

                        aTmpStart = aTmpEnd;    // here starts the next range
                        ++aTmpStart;
                    }
                }
                else
                    pDoc->DelNumRule( rNumNm );
            }
        }
        break;

    case RES_FLTR_BOOKMARK:
        {
            SwFltBookmark* pB = (SwFltBookmark*)rEntry.pAttr;
            const OUString& rName = ((SwFltBookmark*)rEntry.pAttr)->GetName();

            if (IsFlagSet(BOOK_TO_VAR_REF))
            {
                SwFieldType* pFT = pDoc->getIDocumentFieldsAccess().GetFldType(RES_SETEXPFLD, rName, false);
                if (!pFT)
                {
                    SwSetExpFieldType aS(pDoc, rName, nsSwGetSetExpType::GSE_STRING);
                    pFT = pDoc->getIDocumentFieldsAccess().InsertFldType(aS);
                }
                SwSetExpField aFld((SwSetExpFieldType*)pFT, pB->GetValSys());
                aFld.SetSubType( nsSwExtendedSubType::SUB_INVISIBLE );
                MakePoint(rEntry, pDoc, aRegion);
                pDoc->getIDocumentContentOperations().InsertPoolItem(aRegion, SwFmtFld(aFld), 0);
                MoveAttrs( *(aRegion.GetPoint()) );
            }
            if ( ( !IsFlagSet(HYPO) || IsFlagSet(BOOK_AND_REF) ) &&
                 !rEntry.bConsumedByField )
            {
                MakeBookRegionOrPoint(rEntry, pDoc, aRegion, true);
                // #i120879# - create a cross reference heading bookmark if appropriate.
                const IDocumentMarkAccess::MarkType eBookmarkType =
                    ( pB->IsTOCBookmark() &&
                      IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark( aRegion ) )
                    ? IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK
                    : IDocumentMarkAccess::BOOKMARK;
                pDoc->getIDocumentMarkAccess()->makeMark( aRegion, rName, eBookmarkType );
            }
        }
        break;
    case RES_FLTR_ANNOTATIONMARK:
        {
            MakeBookRegionOrPoint(rEntry, pDoc, aRegion, true);
            pDoc->getIDocumentMarkAccess()->makeAnnotationMark(aRegion, OUString());
        }
        break;
    case RES_FLTR_TOX:
        {
            MakePoint(rEntry, pDoc, aRegion);

            SwPosition* pPoint = aRegion.GetPoint();

            SwFltTOX* pTOXAttr = (SwFltTOX*)rEntry.pAttr;

            // test if on this node there had been a pagebreak BEFORE the
            //     tox attribute was put on the stack
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
                            && SfxItemState::SET == pSet->GetItemState( RES_BREAK, false, &pItem ) )
                        {
                            aBkSet.Put( *pItem );
                            pNd->ResetAttr( RES_BREAK );
                        }
                        if(    !pTOXAttr->HadPageDescItem()
                            && SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC, false, &pItem ) )
                        {
                            aBkSet.Put( *pItem );
                            pNd->ResetAttr( RES_PAGEDESC );
                        }
                    }
                }
            }

            delete pTOXAttr->GetBase();

            // set (aboved saved and removed) the break item at the node following the TOX
            if( aBkSet.Count() )
                pNd->SetAttr( aBkSet );
        }
        break;
    case RES_FLTR_REDLINE:
        {
            if (rEntry.MakeRegion(pDoc, aRegion, true))
            {
              pDoc->getIDocumentRedlineAccess().SetRedlineMode((RedlineMode_t)(   nsRedlineMode_t::REDLINE_ON
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
                    pDoc->getIDocumentRedlineAccess().AppendRedline(new SwRangeRedline(aData, aRegion), true);
                }
                SwRedlineData aData(rFltRedline.eType,
                                    rFltRedline.nAutorNo,
                                    rFltRedline.aStamp,
                                    OUString(),
                                    0
                                    );
                pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline(aData, aRegion), true );
                pDoc->getIDocumentRedlineAccess().SetRedlineMode((RedlineMode_t)( nsRedlineMode_t::REDLINE_NONE
                                                | nsRedlineMode_t::REDLINE_SHOW_INSERT
                                                | nsRedlineMode_t::REDLINE_SHOW_DELETE ));
            }
        }
        break;
    default:
        {
            // Revised for more complex situations should be considered
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
                    pDoc->getIDocumentContentOperations().InsertPoolItem(aRegion, *rEntry.pAttr, 0, true);
                }
                else
                {
                    pDoc->getIDocumentContentOperations().InsertPoolItem(aRegion, *rEntry.pAttr, 0);
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
        // is it the looked-for attribute ? (only applies to locked, meaning
        // currently set attributes!!)
        SwFltStackEntry &rEntry = maEntries[--nSize];
        if (rEntry.bOpen && rEntry.pAttr->Which() == nWhich)
        {
            if (pPos)
                *pPos = nSize;
            return (SfxPoolItem*)rEntry.pAttr;      // Ok, so stop
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

void SwFltControlStack::Delete(const SwPaM &rPam)
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    SwNodeIndex aStartNode(pStt->nNode, -1);
    const sal_Int32 nStartIdx = pStt->nContent.GetIndex();
    SwNodeIndex aEndNode(pEnd->nNode, -1);
    const sal_Int32 nEndIdx = pEnd->nContent.GetIndex();

    // We don't support deleting content that is over one node, or removing a node.
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
            // after start, before end, delete
            DeleteAndDestroy(nSize);
            continue;
        }

        const sal_Int32 nCntntDiff = nEndIdx - nStartIdx;

        // to be adjusted
        if (bEntryStartAfterSelStart)
        {
            if (bEntryStartBeforeSelEnd)
            {
                // move start to new start
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

        //That's what Open is, end equal to start, and nPtCntnt is invalid
        if (rEntry.bOpen)
            rEntry.m_aPtPos = rEntry.m_aMkPos;
    }
}

// methods of SwFltAnchor follow
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

// methods of SwFltRedline follow
bool SwFltRedline::operator==(const SfxPoolItem& rItem) const
{
    return this == &rItem;
}

SfxPoolItem* SwFltRedline::Clone( SfxItemPool* ) const
{
    return new SwFltRedline(*this);
}

// methods of SwFltBookmark follow
SwFltBookmark::SwFltBookmark( const OUString& rNa, const OUString& rVa,
                              long nHand, const bool bIsTOCBookmark )
    : SfxPoolItem( RES_FLTR_BOOKMARK )
    , mnHandle( nHand )
    , maName( rNa )
    , maVal( rVa )
    , mbIsTOCBookmark( bIsTOCBookmark )
{
    // eSrc: CHARSET_DONTKNOW for no transform at operator <<
    // Upcase is always done.
    // Transform is never done at XXXStack.NewAttr(...).
    // otherwise: Src Charset from argument for aName
    // Src Charset from filter for aVal ( Text )

    if ( IsTOCBookmark() && ! rNa.startsWith(IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix()) )
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

// methods of SwFltTOX follow
SwFltTOX::SwFltTOX(SwTOXBase* pBase, sal_uInt16 _nCols)
    : SfxPoolItem(RES_FLTR_TOX), pTOXBase(pBase), nCols( _nCols ),
      bHadBreakItem( false ), bHadPageDescItem( false )
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

// UpdatePageDescs needs to be called at end of parsing to make Writer actually
// accept Pagedescs contents
void UpdatePageDescs(SwDoc &rDoc, sal_uInt16 nInPageDescOffset)
{
    // Update document page descriptors (only this way also left pages
    // get adjusted)

    // PageDesc "Standard"
    rDoc.ChgPageDesc(0, rDoc.GetPageDesc(0));

    // PageDescs "Convert..."
    for (sal_uInt16 i = nInPageDescOffset; i < rDoc.GetPageDescCnt(); ++i)
        rDoc.ChgPageDesc(i, rDoc.GetPageDesc(i));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
