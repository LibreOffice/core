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

#include <memory>
#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <cstddef>

#include <hintids.hxx>
#include <hints.hxx>

#include <svl/cintitem.hxx>
#include <svl/stritem.hxx>
#include <fmtanchr.hxx>
#include <fmtfld.hxx>
#include <redline.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <txtfld.hxx>
#include <tox.hxx>
#include <expfld.hxx>
#include <bookmark.hxx>
#include <fltshell.hxx>
#include <rdfhelper.hxx>
#include <utility>

using namespace com::sun::star;

static SwContentNode* GetContentNode(SwPosition& rPos, bool bNext)
{
    SwContentNode * pCNd = rPos.GetNode().GetContentNode();
    if (!pCNd && nullptr == (pCNd = bNext ? SwNodes::GoNext(&rPos)
                                     : SwNodes::GoPrevious(&rPos)))
    {
        pCNd = bNext ? SwNodes::GoPrevious(&rPos) : SwNodes::GoNext(&rPos);
        OSL_ENSURE(pCNd, "no ContentNode found");
    }
    return pCNd;
}

static OUString lcl_getTypePath(OUString& rType)
{
    OUString aRet;
    if (rType.startsWith("urn:bails"))
    {
        rType = "urn:bails";
        aRet = "tscp/bails.rdf";
    }
    return aRet;
}

// Stack entry for all text attributes
SwFltStackEntry::SwFltStackEntry(const SwPosition& rStartPos, std::unique_ptr<SfxPoolItem> pHt)
    : m_aMkPos(rStartPos)
    , m_aPtPos(rStartPos)
    , m_pAttr( std::move(pHt) )
    , m_isAnnotationOnEnd(false)
{
    m_bOld    = false;    // used for marking Attributes *before* skipping field results
    m_bOpen = true;       // lock the attribute --> may first
    m_bConsumedByField = false;
}

SwFltStackEntry::~SwFltStackEntry()
{
    // Although attribute got passed as pointer, it gets deleted here
}

void SwFltStackEntry::SetEndPos(const SwPosition& rEndPos)
{
    // Release attribute and keep track of end
    // Everything with sal_uInt16s, because otherwise the inserting of new text at
    // the cursor position moves the attribute's range
    // That's not the desired behavior!
    m_bOpen = false;                  // release and remember END
    m_aPtPos.FromSwPosition(rEndPos);
}

bool SwFltStackEntry::MakeRegion(SwPaM& rRegion, RegionMode const eCheck,
    const SwFltPosition &rMkPos, const SwFltPosition &rPtPos,
    sal_uInt16 nWhich)
{
    // does this range actually contain something?
    // empty range is allowed if at start of empty paragraph
    // fields are special: never have range, so leave them
    SwNodeOffset nMk = rMkPos.m_nNode.GetIndex() + 1;
    const SwNodes& rMkNodes = rMkPos.m_nNode.GetNodes();
    if (nMk >= rMkNodes.Count())
        return false;
    SwContentNode *const pContentNode(rMkNodes[nMk]->GetContentNode());
    if (rMkPos == rPtPos &&
        ((0 != rPtPos.m_nContent) || (pContentNode && (0 != pContentNode->Len())))
        && ( RES_TXTATR_FIELD != nWhich
             && RES_TXTATR_ANNOTATION != nWhich
             && RES_TXTATR_INPUTFIELD != nWhich ))
    {
        return false;
    }
    // The content indices always apply to the node!
    rRegion.GetPoint()->Assign( rMkPos.m_nNode.GetIndex() + 1 );
    SwContentNode* pCNd = GetContentNode(*rRegion.GetPoint(), true);

    SAL_WARN_IF(pCNd->Len() < rMkPos.m_nContent, "sw.ww8",
        "invalid content index " << rMkPos.m_nContent << " but text node has only " << pCNd->Len());
    rRegion.GetPoint()->SetContent( std::min<sal_Int32>(rMkPos.m_nContent, pCNd->Len()) );
    rRegion.SetMark();
    if (rMkPos.m_nNode != rPtPos.m_nNode)
    {
        SwNodeOffset n = rPtPos.m_nNode.GetIndex() + 1;
        SwNodes& rNodes = rRegion.GetPoint()->GetNodes();
        if (n >= rNodes.Count())
            return false;
        rRegion.GetPoint()->Assign(n);
        pCNd = GetContentNode(*rRegion.GetPoint(), false);
    }
    SAL_WARN_IF(pCNd->Len() < rPtPos.m_nContent, "sw.ww8",
        "invalid content index " << rPtPos.m_nContent << " but text node has only " << pCNd->Len());
    rRegion.GetPoint()->SetContent( std::min<sal_Int32>(rPtPos.m_nContent, pCNd->Len()) );
    OSL_ENSURE( CheckNodesRange( rRegion.Start()->GetNode(),
                             rRegion.End()->GetNode(), true ),
             "attribute or similar crosses section-boundaries" );
    bool bRet = true;
    if (eCheck & RegionMode::CheckNodes)
    {
        bRet &= CheckNodesRange(rRegion.Start()->GetNode(),
                                rRegion.End()->GetNode(), true);
    }
    if (eCheck & RegionMode::CheckFieldmark)
    {
        bRet &= !sw::mark::IsFieldmarkOverlap(rRegion);
    }
    return bRet;
}

bool SwFltStackEntry::MakeRegion(SwPaM& rRegion, RegionMode eCheck) const
{
    return MakeRegion(rRegion, eCheck, m_aMkPos, m_aPtPos, m_pAttr->Which());
}

SwFltControlStack::SwFltControlStack(SwDoc& rDo, sal_uLong nFieldFl)
    : m_nFieldFlags(nFieldFl), m_rDoc(rDo), m_bIsEndStack(false)
{
}

SwFltControlStack::~SwFltControlStack()
{
    OSL_ENSURE(m_Entries.empty(), "There are still Attributes on the stack");
}

// MoveAttrs() is meant to address the following problem:
// When a field like "set variable" is set through the stack, the text
// is shifted by one \xff character, which makes all subsequent
// attribute positions invalid.
// After setting the attribute in the doc, MoveAttrs() needs to be
// called in order to push all attribute positions to the right in the
// same paragraph further out by one character.
void SwFltControlStack::MoveAttrs(const SwPosition& rPos, MoveAttrsMode eMode)
{
    SwNodeOffset nPosNd = rPos.GetNodeIndex();
    sal_uInt16 nPosCt = rPos.GetContentIndex() - 1;

    for (size_t i = 0, nCnt = m_Entries.size(); i < nCnt; ++i)
    {
        SwFltStackEntry& rEntry = *m_Entries[i];
        if (
            (rEntry.m_aMkPos.m_nNode.GetIndex()+1 == nPosNd) &&
            (rEntry.m_aMkPos.m_nContent >= nPosCt)
           )
        {
            rEntry.m_aMkPos.m_nContent++;
            OSL_ENSURE( rEntry.m_aMkPos.m_nContent
                <= m_rDoc.GetNodes()[nPosNd]->GetContentNode()->Len(),
                    "Attribute ends after end of line" );
        }
        if (
            (rEntry.m_aPtPos.m_nNode.GetIndex()+1 == nPosNd) &&
            (rEntry.m_aPtPos.m_nContent >= nPosCt)
           )
        {
            if (    !rEntry.m_isAnnotationOnEnd
                ||  rEntry.m_aPtPos.m_nContent > nPosCt)
            {
                assert(!(rEntry.m_isAnnotationOnEnd && rEntry.m_aPtPos.m_nContent > nPosCt));
                if (    eMode == MoveAttrsMode::POSTIT_INSERTED
                    &&  rEntry.m_aPtPos.m_nContent == nPosCt
                    &&  rEntry.m_pAttr->Which() == RES_FLTR_ANNOTATIONMARK)
                {
                    rEntry.m_isAnnotationOnEnd = true;
                    eMode = MoveAttrsMode::DEFAULT; // only set 1 flag
                }
                rEntry.m_aPtPos.m_nContent++;
                OSL_ENSURE( rEntry.m_aPtPos.m_nContent
                    <= m_rDoc.GetNodes()[nPosNd]->GetContentNode()->Len(),
                        "Attribute ends after end of line" );
            }
        }
    }
}

void SwFltControlStack::MarkAllAttrsOld()
{
    size_t nCnt = m_Entries.size();
    for (size_t i=0; i < nCnt; ++i)
        m_Entries[i]->m_bOld = true;
}

namespace
{
    bool couldExtendEntry(const SwFltStackEntry *pExtendCandidate,
        const SfxPoolItem& rAttr)
    {
        return (pExtendCandidate &&
                !pExtendCandidate->m_bConsumedByField &&
                //if we bring character attributes into the fold we need to both
                //a) consider RES_CHRATR_FONTSIZE and RES_CHRATR_FONT wrt Word's CJK/CTL variants
                //b) consider crossing table cell boundaries (tdf#102334)
                isPARATR_LIST(rAttr.Which()) &&
                *(pExtendCandidate->m_pAttr) == rAttr);
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
        //Here we optimize by seeing if there is an attribute uncommitted
        //to the document which

        //a) has the same value as this attribute
        //b) is already open, or ends at the same place as where we're starting
        //from. If so we merge it with this one and elide adding another
        //to the stack
        pExtendCandidate->SetEndPos(rPos);
        pExtendCandidate->m_bOpen=true;
    }
    else
    {
        SwFltStackEntry *pTmp = new SwFltStackEntry(rPos, std::unique_ptr<SfxPoolItem>(rAttr.Clone()) );
        m_Entries.push_back(std::unique_ptr<SwFltStackEntry>(pTmp));
    }
}

void SwFltControlStack::DeleteAndDestroy(Entries::size_type nCnt)
{
    OSL_ENSURE(nCnt < m_Entries.size(), "Out of range!");
    if (nCnt < m_Entries.size())
    {
        auto aElement = m_Entries.begin() + nCnt;
        m_Entries.erase(aElement);
    }
}

// SwFltControlStack::StealAttr() removes attributes of the given type
// from the stack. Allowed as nAttrId: 0 meaning any, or a specific
// type.  This makes them disappear from the doc structure. Only
// attributes from the same paragraph as rPos are removed. Used for
// graphic apos -> images.
void SwFltControlStack::StealAttr(const SwNode& rNode)
{
    size_t nCnt = m_Entries.size();

    while (nCnt)
    {
        nCnt --;
        SwFltStackEntry& rEntry = *m_Entries[nCnt];
        if (rEntry.m_aPtPos.m_nNode.GetIndex()+1 == rNode.GetIndex())
        {
            DeleteAndDestroy(nCnt);     // delete from the stack
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

    size_t nCnt = m_Entries.size();
    while( nCnt )
    {
        nCnt --;
        SwFltStackEntry& rEntry = *m_Entries[nCnt];
        if(    !rEntry.m_bOld
            && !rEntry.m_bOpen
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
    sal_uInt16 nAttrId, bool bTstEnd, tools::Long nHand,
    bool consumedByField)
{
    SwFltStackEntry *pRet = nullptr;

    SwFltPosition aFltPos(rPos);

    OSL_ENSURE(!nAttrId ||
        (POOLATTR_BEGIN <= nAttrId && POOLATTR_END > nAttrId) ||
        (RES_FLTRATTR_BEGIN <= nAttrId && RES_FLTRATTR_END > nAttrId),
        "Wrong id for attribute");

    auto aI = m_Entries.begin();
    while (aI != m_Entries.end())
    {
        bool bLastEntry = aI == m_Entries.end() - 1;

        SwFltStackEntry& rEntry = **aI;
        if (rEntry.m_bOpen)
        {
            // set end of attribute
            bool bF = false;
            if (!nAttrId )
            {
                bF = true;
            }
            else if (nAttrId == rEntry.m_pAttr->Which())
            {
                if( nAttrId != RES_FLTR_BOOKMARK && nAttrId != RES_FLTR_ANNOTATIONMARK && nAttrId != RES_FLTR_RDFMARK )
                {
                    // query handle
                    bF = true;
                }
                else if (nAttrId == RES_FLTR_BOOKMARK && nHand == static_cast<SwFltBookmark*>(rEntry.m_pAttr.get())->GetHandle())
                {
                    bF = true;
                }
                else if (nAttrId == RES_FLTR_ANNOTATIONMARK && nHand == static_cast<CntUInt16Item*>(rEntry.m_pAttr.get())->GetValue())
                {
                    bF = true;
                }
                else if (nAttrId == RES_FLTR_RDFMARK && nHand == static_cast<SwFltRDFMark*>(rEntry.m_pAttr.get())->GetHandle())
                {
                    bF = true;
                }
            }
            if (bF)
            {
                rEntry.m_bConsumedByField = consumedByField;
                rEntry.SetEndPos(rPos);
                if (bLastEntry && nAttrId == rEntry.m_pAttr->Which())
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
        if (bTstEnd)
        {
            if (m_bIsEndStack)
            {
                ++aI;
                continue;
            }

            //defer inserting this attribute into the document until
            //we advance to the next node, or finish processing the document
            if (rEntry.m_aPtPos.m_nNode.GetIndex() == aFltPos.m_nNode.GetIndex())
            {
                if (bLastEntry && nAttrId == rEntry.m_pAttr->Which() &&
                    rEntry.m_aPtPos.m_nContent == aFltPos.m_nContent)
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
        aI = m_Entries.erase(aI);
    }

    return pRet;
}

static bool MakePoint(const SwFltStackEntry& rEntry, SwPaM& rRegion)
{
    // the anchor is the Pam's Point. It's modified when inserting
    // text, etc.; therefore it is kept on the stack. Only the
    // attribute's format needs to be set.
    rRegion.DeleteMark();

    SwNodeOffset nMk = rEntry.m_aMkPos.m_nNode.GetIndex() + 1;
    const SwNodes& rMkNodes = rEntry.m_aMkPos.m_nNode.GetNodes();
    if (nMk >= rMkNodes.Count())
        return false;

    rRegion.GetPoint()->Assign(nMk);
    GetContentNode(*rRegion.GetPoint(), true);
    rRegion.GetPoint()->SetContent(rEntry.m_aMkPos.m_nContent);
    return true;
}

// MakeBookRegionOrPoint() behaves like MakeRegionOrPoint, except that
// it adheres to certain restrictions on bookmarks in tables (cannot
// span more than one cell)
static bool MakeBookRegionOrPoint(const SwFltStackEntry& rEntry, SwPaM& rRegion )
{
    if (rEntry.MakeRegion(rRegion, SwFltStackEntry::RegionMode::CheckNodes))
    {
        if (rRegion.GetPoint()->GetNode().FindTableBoxStartNode()
              != rRegion.GetMark()->GetNode().FindTableBoxStartNode())
        {
            rRegion.Exchange();         // invalid range
            rRegion.DeleteMark();       // -> both to mark
        }
        return true;
    }
    return MakePoint(rEntry, rRegion);
}

// IterateNumrulePiece() looks for the first range valid for Numrules
// between rTmpStart and rEnd.

// rNds denotes the doc nodes
// rEnd denotes the range end,
// rTmpStart is an in/out parameter: in: start of range to be searched,
//                                   out: start of valid range
// rTmpEnd is an out parameter
// Returns true for valid range
static bool IterateNumrulePiece( const SwPosition& rEnd,
                                SwNodeIndex& rTmpStart, SwNodeIndex& rTmpEnd )
{
    while( ( rTmpStart <= rEnd.GetNode() )
           && !( rTmpStart.GetNode().IsTextNode() ) )    // look for valid start
        ++rTmpStart;

    rTmpEnd = rTmpStart;
    while( ( rTmpEnd <= rEnd.GetNode() )
           && ( rTmpEnd.GetNode().IsTextNode() ) )       // look for valid end + 1
        ++rTmpEnd;

    --rTmpEnd;                                      // valid end

    return rTmpStart <= rTmpEnd;                    // valid ?
}

void SwFltControlStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry& rEntry)
{
    SwPaM aRegion( rTmpPos );

    switch(rEntry.m_pAttr->Which())
    {
    case RES_FLTR_ANCHOR:
        {
            SwFrameFormat* pFormat = static_cast<SwFltAnchor*>(rEntry.m_pAttr.get())->GetFrameFormat();
            if (pFormat != nullptr)
            {
                MakePoint(rEntry, aRegion);
                SwFormatAnchor aAnchor(pFormat->GetAnchor());
                aAnchor.SetAnchor(aRegion.GetPoint());
                pFormat->SetFormatAttr(aAnchor);
                // So the frames will be created when inserting into
                // existing doc (after setting the anchor!):
                if (m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell()
                   && (RndStdIds::FLY_AT_PARA == pFormat->GetAnchor().GetAnchorId()))
                {
                    pFormat->MakeFrames();
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
            const OUString& rNumNm = static_cast<SfxStringItem*>(rEntry.m_pAttr.get())->GetValue();
            SwNumRule* pNumRule = m_rDoc.FindNumRulePtr( rNumNm );
            if( pNumRule )
            {
                if (rEntry.MakeRegion(aRegion, SwFltStackEntry::RegionMode::CheckNodes))
                {
                    SwNodeIndex aTmpStart( aRegion.Start()->GetNode() );
                    SwNodeIndex aTmpEnd( aTmpStart );
                    SwPosition& rRegEndNd = *aRegion.End();
                    while( IterateNumrulePiece( rRegEndNd,
                                                aTmpStart, aTmpEnd ) )
                    {
                        SwPaM aTmpPam( aTmpStart, aTmpEnd );
                        // no start of a new list
                        m_rDoc.SetNumRule(aTmpPam, *pNumRule, SwDoc::SetNumRuleMode::Default);

                        aTmpStart = aTmpEnd;    // here starts the next range
                        ++aTmpStart;
                    }
                }
                else
                    m_rDoc.DelNumRule( rNumNm );
            }
        }
        break;

    case RES_FLTR_BOOKMARK:
        {
            SwFltBookmark* pB = static_cast<SwFltBookmark*>(rEntry.m_pAttr.get());
            const OUString& rName = static_cast<SwFltBookmark*>(rEntry.m_pAttr.get())->GetName();

            if (IsFlagSet(BOOK_TO_VAR_REF))
            {
                SwFieldType* pFT = m_rDoc.getIDocumentFieldsAccess().GetFieldType(SwFieldIds::SetExp, rName, false);
                if (!pFT)
                {
                    SwSetExpFieldType aS(&m_rDoc, rName, nsSwGetSetExpType::GSE_STRING);
                    pFT = m_rDoc.getIDocumentFieldsAccess().InsertFieldType(aS);
                }
                SwSetExpField aField(static_cast<SwSetExpFieldType*>(pFT), pB->GetValSys());
                aField.SetSubType( nsSwExtendedSubType::SUB_INVISIBLE );
                MakePoint(rEntry, aRegion);
                m_rDoc.getIDocumentContentOperations().InsertPoolItem(aRegion, SwFormatField(aField));
                MoveAttrs( *(aRegion.GetPoint()) );
            }
            if ( ( !IsFlagSet(HYPO) || IsFlagSet(BOOK_AND_REF) ) &&
                 !rEntry.m_bConsumedByField )
            {
                MakeBookRegionOrPoint(rEntry, aRegion);
                // #i120879# - create a cross reference heading bookmark if appropriate.
                const IDocumentMarkAccess::MarkType eBookmarkType =
                    ( pB->IsTOCBookmark() &&
                      IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark( aRegion ) )
                    ? IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK
                    : IDocumentMarkAccess::MarkType::BOOKMARK;
                m_rDoc.getIDocumentMarkAccess()->makeMark(aRegion, rName, eBookmarkType, sw::mark::InsertMode::New);
            }
        }
        break;
    case RES_FLTR_ANNOTATIONMARK:
        {
            if (MakeBookRegionOrPoint(rEntry, aRegion))
            {
                SwTextNode const*const pTextNode(
                        aRegion.End()->GetNode().GetTextNode());
                SwTextField const*const pField = pTextNode ? pTextNode->GetFieldTextAttrAt(
                        aRegion.End()->GetContentIndex() - 1, ::sw::GetTextAttrMode::Default) : nullptr;
                if (pField)
                {
                    SwPostItField const*const pPostIt(
                        dynamic_cast<SwPostItField const*>(pField->GetFormatField().GetField()));
                    if (pPostIt)
                    {
                        assert(pPostIt->GetName().isEmpty());

                        if (!aRegion.HasMark())
                        {
                            // Annotation range was found in the file, but start/end is the same,
                            // pointing after the postit placeholder (see assert above).
                            // Adjust the start of the range to actually cover the comment, similar
                            // to what the UI and the UNO API does.
                            aRegion.SetMark();
                            aRegion.Start()->AdjustContent(-1);
                        }

                        m_rDoc.getIDocumentMarkAccess()->makeAnnotationMark(aRegion, OUString());
                    }
                    else
                    {
                        SAL_WARN("sw", "RES_FLTR_ANNOTATIONMARK: unexpected field");
                    }
                }
                else
                {
                    SAL_WARN("sw", "RES_FLTR_ANNOTATIONMARK: missing field");
                }
            }
            else
                SAL_WARN("sw", "failed to make book region or point");
        }
        break;
    case RES_FLTR_RDFMARK:
        {
            if (MakeBookRegionOrPoint(rEntry, aRegion))
            {
                SwFltRDFMark* pMark = static_cast<SwFltRDFMark*>(rEntry.m_pAttr.get());
                if (aRegion.GetPointNode().IsTextNode())
                {
                    SwTextNode& rTextNode = *aRegion.GetPointNode().GetTextNode();

                    for (const std::pair<OUString, OUString>& rAttribute : pMark->GetAttributes())
                    {
                        OUString aTypeNS = rAttribute.first;
                        OUString aMetadataFilePath = lcl_getTypePath(aTypeNS);
                        if (aMetadataFilePath.isEmpty())
                            continue;

                        SwRDFHelper::addTextNodeStatement(aTypeNS, aMetadataFilePath, rTextNode, rAttribute.first, rAttribute.second);
                    }
                }
            }
            else
                SAL_WARN("sw", "failed to make book region or point");
        }
        break;
    case RES_FLTR_TOX:
        {
            MakePoint(rEntry, aRegion);

            SwPosition* pPoint = aRegion.GetPoint();

            SwFltTOX* pTOXAttr = static_cast<SwFltTOX*>(rEntry.m_pAttr.get());

            // test if on this node there had been a pagebreak BEFORE the
            //     tox attribute was put on the stack
            SfxItemSetFixed<RES_PAGEDESC, RES_BREAK> aBkSet( m_rDoc.GetAttrPool() );
            SwContentNode* pNd = nullptr;
            if( !pTOXAttr->HadBreakItem() || !pTOXAttr->HadPageDescItem() )
            {
                pNd = pPoint->GetNode().GetContentNode();
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

            // set (above saved and removed) the break item at the node following the TOX
            if (pNd && aBkSet.Count())
                pNd->SetAttr(aBkSet);
        }
        break;
    case RES_FLTR_REDLINE:
        {
            if (rEntry.MakeRegion(aRegion,
                    SwFltStackEntry::RegionMode::CheckNodes|SwFltStackEntry::RegionMode::CheckFieldmark))
            {
                m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags( RedlineFlags::On
                                              | RedlineFlags::ShowInsert
                                              | RedlineFlags::ShowDelete );
                SwFltRedline& rFltRedline = *static_cast<SwFltRedline*>(rEntry.m_pAttr.get());

                SwRedlineData aData(rFltRedline.m_eType,
                                    rFltRedline.m_nAutorNo,
                                    rFltRedline.m_aStamp,
                                    0,
                                    OUString(),
                                    nullptr
                                    );
                m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline(aData, aRegion), true );
                m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags( RedlineFlags::NONE
                                                | RedlineFlags::ShowInsert
                                                | RedlineFlags::ShowDelete );
            }
        }
        break;
    default:
        {
            if (rEntry.MakeRegion(aRegion, SwFltStackEntry::RegionMode::NoCheck))
            {
                m_rDoc.getIDocumentContentOperations().InsertPoolItem(aRegion, *rEntry.m_pAttr);
            }
        }
        break;
    }
}

SfxPoolItem* SwFltControlStack::GetFormatStackAttr(sal_uInt16 nWhich, sal_uInt16 * pPos)
{
    size_t nSize = m_Entries.size();

    while (nSize)
    {
        // is it the looked-for attribute ? (only applies to locked, meaning
        // currently set attributes!!)
        SwFltStackEntry &rEntry = *m_Entries[--nSize];
        if (rEntry.m_bOpen && rEntry.m_pAttr->Which() == nWhich)
        {
            if (pPos)
                *pPos = nSize;
            return rEntry.m_pAttr.get();      // Ok, so stop
        }
    }
    return nullptr;
}

const SfxPoolItem* SwFltControlStack::GetOpenStackAttr(const SwPosition& rPos, sal_uInt16 nWhich)
{
    SwFltPosition aFltPos(rPos);

    size_t nSize = m_Entries.size();

    while (nSize)
    {
        SwFltStackEntry &rEntry = *m_Entries[--nSize];
        if (rEntry.m_bOpen && rEntry.m_pAttr->Which() == nWhich && rEntry.m_aMkPos == aFltPos)
        {
            return rEntry.m_pAttr.get();
        }
    }
    return nullptr;
}

void SwFltControlStack::Delete(const SwPaM &rPam)
{
    auto [pStt, pEnd] = rPam.StartEnd(); // SwPosition*

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    SwNodeIndex aStartNode(pStt->GetNode(), -1);
    const sal_Int32 nStartIdx = pStt->GetContentIndex();
    SwNodeIndex aEndNode(pEnd->GetNode(), -1);
    const sal_Int32 nEndIdx = pEnd->GetContentIndex();

    // We don't support deleting content that is over one node, or removing a node.
    OSL_ENSURE(aEndNode == aStartNode, "nodes must be the same, or this method extended");
    if (aEndNode != aStartNode)
        return;

    for (size_t nSize = m_Entries.size(); nSize > 0;)
    {
        SwFltStackEntry& rEntry = *m_Entries[--nSize];

        bool bEntryStartAfterSelStart =
            (rEntry.m_aMkPos.m_nNode == aStartNode &&
             rEntry.m_aMkPos.m_nContent >= nStartIdx);

        bool bEntryStartBeforeSelEnd =
            (rEntry.m_aMkPos.m_nNode == aEndNode &&
             rEntry.m_aMkPos.m_nContent <= nEndIdx);

        bool bEntryEndAfterSelStart = false;
        bool bEntryEndBeforeSelEnd = false;
        if (!rEntry.m_bOpen)
        {
            bEntryEndAfterSelStart =
                (rEntry.m_aPtPos.m_nNode == aStartNode &&
                 rEntry.m_aPtPos.m_nContent >= nStartIdx);

            bEntryEndBeforeSelEnd =
                (rEntry.m_aPtPos.m_nNode == aEndNode &&
                 rEntry.m_aPtPos.m_nContent <= nEndIdx);
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

        const sal_Int32 nContentDiff = nEndIdx - nStartIdx;

        // to be adjusted
        if (bEntryStartAfterSelStart)
        {
            if (bEntryStartBeforeSelEnd)
            {
                // move start to new start
                rEntry.m_aMkPos.SetPos(aStartNode, nStartIdx);
            }
            else
                rEntry.m_aMkPos.m_nContent -= nContentDiff;
        }

        if (bEntryEndAfterSelStart)
        {
            if (bEntryEndBeforeSelEnd)
                rEntry.m_aPtPos.SetPos(aStartNode, nStartIdx);
            else
                rEntry.m_aPtPos.m_nContent -= nContentDiff;
        }

        //That's what Open is, end equal to start, and nPtContent is invalid
        if (rEntry.m_bOpen)
            rEntry.m_aPtPos = rEntry.m_aMkPos;
    }
}

// methods of SwFltAnchor follow
SwFltAnchor::SwFltAnchor(SwFrameFormat* pFormat) :
    SfxPoolItem(RES_FLTR_ANCHOR), m_pFrameFormat(pFormat)
{
    m_pListener.reset(new SwFltAnchorListener(this));
    m_pListener->StartListening(m_pFrameFormat->GetNotifier());
}

SwFltAnchor::SwFltAnchor(const SwFltAnchor& rCpy) :
    SfxPoolItem(RES_FLTR_ANCHOR), m_pFrameFormat(rCpy.m_pFrameFormat)
{
    m_pListener.reset(new SwFltAnchorListener(this));
    m_pListener->StartListening(m_pFrameFormat->GetNotifier());
}

SwFltAnchor::~SwFltAnchor()
{
}

void SwFltAnchor::SetFrameFormat(SwFrameFormat * _pFrameFormat)
{
    m_pFrameFormat = _pFrameFormat;
}


bool SwFltAnchor::operator==(const SfxPoolItem& rItem) const
{
    return SfxPoolItem::operator==(rItem) &&
        m_pFrameFormat == static_cast<const SwFltAnchor&>(rItem).m_pFrameFormat;
}

SwFltAnchor* SwFltAnchor::Clone(SfxItemPool*) const
{
    return new SwFltAnchor(*this);
}

SwFltAnchorListener::SwFltAnchorListener(SwFltAnchor* pFltAnchor)
    : m_pFltAnchor(pFltAnchor)
{ }

void SwFltAnchorListener::Notify(const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
        m_pFltAnchor->SetFrameFormat(nullptr);
    else if (rHint.GetId() == SfxHintId::SwDrawFrameFormat)
    {
        auto pDrawFrameFormatHint = static_cast<const sw::DrawFrameFormatHint*>(&rHint);
        if (pDrawFrameFormatHint->m_eId != sw::DrawFrameFormatHintId::DYING)
            return;
        m_pFltAnchor->SetFrameFormat(nullptr);
    }
    else if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacyHint = static_cast<const sw::LegacyModifyHint*>(&rHint);
        if(pLegacyHint->m_pNew->Which() != RES_FMT_CHG)
            return;
        auto pFormatChg = dynamic_cast<const SwFormatChg*>(pLegacyHint->m_pNew);
        auto pFrameFormat = pFormatChg ? dynamic_cast<SwFrameFormat*>(pFormatChg->pChangedFormat) : nullptr;
        if(pFrameFormat)
            m_pFltAnchor->SetFrameFormat(pFrameFormat);
    }
}

// methods of SwFltRedline follow
bool SwFltRedline::operator==(const SfxPoolItem& rItem) const
{
    return SfxPoolItem::operator==(rItem) &&
        SfxPoolItem::areSame(*this, rItem);
}

SwFltRedline* SwFltRedline::Clone( SfxItemPool* ) const
{
    return new SwFltRedline(*this);
}

// methods of SwFltBookmark follow
SwFltBookmark::SwFltBookmark( const OUString& rNa, OUString aVa,
                              tools::Long nHand, const bool bIsTOCBookmark )
    : SfxPoolItem( RES_FLTR_BOOKMARK )
    , mnHandle( nHand )
    , maName( rNa )
    , maVal(std::move( aVa ))
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

bool SwFltBookmark::operator==(const SfxPoolItem& rItem) const
{
    return SfxPoolItem::operator==(rItem)
        && maName == static_cast<const SwFltBookmark&>(rItem).maName
        && mnHandle == static_cast<const SwFltBookmark&>(rItem).mnHandle;
}

SwFltBookmark* SwFltBookmark::Clone(SfxItemPool*) const
{
    return new SwFltBookmark(*this);
}

SwFltRDFMark::SwFltRDFMark()
    : SfxPoolItem(RES_FLTR_RDFMARK),
      m_nHandle(0)
{
}

bool SwFltRDFMark::operator==(const SfxPoolItem& rItem) const
{
    if (!SfxPoolItem::operator==(rItem))
        return false;

    const SwFltRDFMark& rMark = static_cast<const SwFltRDFMark&>(rItem);

    return m_nHandle == rMark.m_nHandle && m_aAttributes == rMark.m_aAttributes;
}

SwFltRDFMark* SwFltRDFMark::Clone(SfxItemPool*) const
{
    return new SwFltRDFMark(*this);
}

void SwFltRDFMark::SetHandle(tools::Long nHandle)
{
    m_nHandle = nHandle;
}

tools::Long SwFltRDFMark::GetHandle() const
{
    return m_nHandle;
}

void SwFltRDFMark::SetAttributes( std::vector< std::pair<OUString, OUString> >&& rAttributes)
{
    m_aAttributes = std::move(rAttributes);
}

const std::vector< std::pair<OUString, OUString> >& SwFltRDFMark::GetAttributes() const
{
    return m_aAttributes;
}

// methods of SwFltTOX follow
SwFltTOX::SwFltTOX(std::shared_ptr<SwTOXBase> xBase)
    : SfxPoolItem(RES_FLTR_TOX), m_xTOXBase(std::move(xBase)),
      m_bHadBreakItem( false ), m_bHadPageDescItem( false )
{
}

bool SwFltTOX::operator==(const SfxPoolItem& rItem) const
{
    return SfxPoolItem::operator==(rItem) &&
        m_xTOXBase.get() == static_cast<const SwFltTOX&>(rItem).m_xTOXBase.get();
}

SwFltTOX* SwFltTOX::Clone(SfxItemPool*) const
{
    return new SwFltTOX(*this);
}

// UpdatePageDescs needs to be called at end of parsing to make Writer actually
// accept Pagedescs contents
void UpdatePageDescs(SwDoc &rDoc, size_t nInPageDescOffset)
{
    // Update document page descriptors (only this way also left pages
    // get adjusted)

    // PageDesc "Standard"
    rDoc.ChgPageDesc(0, rDoc.GetPageDesc(0));

    // PageDescs "Convert..."
    for (size_t i = nInPageDescOffset; i < rDoc.GetPageDescCnt(); ++i)
        rDoc.ChgPageDesc(i, rDoc.GetPageDesc(i));
}

FrameDeleteWatch::FrameDeleteWatch(SwFrameFormat* pFormat)
    : m_pFormat(pFormat)
{
    if(m_pFormat)
        StartListening(pFormat->GetNotifier());
}

void FrameDeleteWatch::Notify(const SfxHint& rHint)
{
    bool bDying = false;
    if (rHint.GetId() == SfxHintId::Dying)
        bDying = true;
    else if (rHint.GetId() == SfxHintId::SwDrawFrameFormat)
    {
        auto pDrawFrameFormatHint = static_cast<const sw::DrawFrameFormatHint*>(&rHint);
        bDying = pDrawFrameFormatHint->m_eId == sw::DrawFrameFormatHintId::DYING;
    }
    if (bDying)
    {
        m_pFormat = nullptr;
        EndListeningAll();
    }
}

FrameDeleteWatch::~FrameDeleteWatch()
{
    m_pFormat = nullptr;
    EndListeningAll();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
