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

#include <bookmark.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <editsh.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <functional>
#include <mvsave.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <redline.hxx>
#include <sal/types.h>
#include <unocrsr.hxx>
#include <txtfrm.hxx>
#include <frameformats.hxx>
#include <memory>

using namespace ::boost;
using namespace ::sw::mark;

namespace
{
    // #i59534: If a paragraph will be split we have to restore some redline positions
    // This help function checks a position compared with a node and a content index

    const int BEFORE_NODE = 0;          // Position before the given node index
    const int BEFORE_SAME_NODE = 1;     // Same node index but content index before given content index
    const int SAME_POSITION = 2;        // Same node index and samecontent index
    const int BEHIND_SAME_NODE = 3;     // Same node index but content index behind given content index
    const int BEHIND_NODE = 4;          // Position behind the given node index

    int lcl_RelativePosition( const SwPosition& rPos, SwNodeOffset nNode, sal_Int32 nContent )
    {
        SwNodeOffset nIndex = rPos.GetNodeIndex();
        int nReturn = BEFORE_NODE;
        if( nIndex == nNode )
        {
            const sal_Int32 nCntIdx = rPos.GetContentIndex();
            if( nCntIdx < nContent )
                nReturn = BEFORE_SAME_NODE;
            else if( nCntIdx == nContent )
                nReturn = SAME_POSITION;
            else
                nReturn = BEHIND_SAME_NODE;
        }
        else if( nIndex > nNode )
            nReturn = BEHIND_NODE;
        return nReturn;
    }
    struct MarkEntry
    {
        tools::Long m_nIdx;
        bool m_bOther;
        sal_Int32 m_nContent;
#if 0
#include <sal/log.hxx>
        void Dump()
        {
            SAL_INFO("sw.core", "Index: " << m_nIdx << "\tOther: " << m_bOther << "\tContent: " << m_nContent);
        }
#endif
    };
    struct PaMEntry
    {
        SwPaM* m_pPaM;
        bool m_isMark;
        sal_Int32 m_nContent;
    };
    struct OffsetUpdater
    {
        const SwContentNode* m_pNewContentNode;
        const sal_Int32 m_nOffset;
        OffsetUpdater(SwContentNode const * pNewContentNode, sal_Int32 nOffset)
            : m_pNewContentNode(pNewContentNode), m_nOffset(nOffset) {};
        void operator()(SwPosition& rPos, sal_Int32 nContent) const
        {
            rPos.Assign(*m_pNewContentNode, nContent + m_nOffset);
        };
    };
    struct LimitUpdater
    {
        const SwContentNode* m_pNewContentNode;
        const sal_uLong m_nLen;
        const sal_Int32 m_nCorrLen;
        LimitUpdater(SwContentNode const * pNewContentNode, sal_uLong nLen, sal_Int32 nCorrLen)
            : m_pNewContentNode(pNewContentNode), m_nLen(nLen), m_nCorrLen(nCorrLen) {};
        void operator()(SwPosition& rPos, sal_Int32 nContent) const
        {
            if( nContent < m_nCorrLen )
            {
                nContent = std::min( nContent, static_cast<sal_Int32>(m_nLen) );
            }
            else
            {
                nContent -= m_nCorrLen;
            }
            rPos.Assign( *m_pNewContentNode, nContent );
        };
    };
    struct ContentIdxStoreImpl : sw::mark::ContentIdxStore
    {
        std::vector<MarkEntry> m_aBkmkEntries;
        std::vector<MarkEntry> m_aRedlineEntries;
        std::vector<MarkEntry> m_aFlyEntries;
        std::vector<PaMEntry> m_aUnoCursorEntries;
        std::vector<PaMEntry> m_aShellCursorEntries;
        typedef std::function<void (SwPosition& rPos, sal_Int32 nContent)> updater_t;
        virtual bool Empty() override
        {
            return m_aBkmkEntries.empty() && m_aRedlineEntries.empty() && m_aFlyEntries.empty() && m_aUnoCursorEntries.empty() && m_aShellCursorEntries.empty();
        }
        virtual void Save(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent, bool bSaveFlySplit=false) override
        {
            SaveBkmks(rDoc, nNode, nContent);
            SaveRedlines(rDoc, nNode, nContent);
            SaveFlys(rDoc, nNode, nContent, bSaveFlySplit);
            SaveUnoCursors(rDoc, nNode, nContent);
            SaveShellCursors(rDoc, nNode, nContent);
        }
        virtual void Restore(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nOffset=0, bool bAuto = false, bool bAtStart = false, RestoreMode eMode = RestoreMode::All) override
        {
            SwContentNode* pCNd = rDoc.GetNodes()[ nNode ]->GetContentNode();
            updater_t aUpdater = OffsetUpdater(pCNd, nOffset);
            if (eMode & RestoreMode::NonFlys)
            {
                RestoreBkmks(rDoc, aUpdater);
                RestoreRedlines(rDoc, aUpdater);
                RestoreUnoCursors(aUpdater);
                RestoreShellCursors(aUpdater);
            }
            if (eMode & RestoreMode::Flys)
            {
                RestoreFlys(rDoc, aUpdater, bAuto, bAtStart);
            }
        }
        virtual void Restore(SwNode& rNd, sal_Int32 nLen, sal_Int32 nCorrLen, RestoreMode eMode = RestoreMode::All) override
        {
            SwContentNode* pCNd = rNd.GetContentNode();
            SwDoc& rDoc = rNd.GetDoc();
            updater_t aUpdater = LimitUpdater(pCNd, nLen, nCorrLen);
            if (eMode & RestoreMode::NonFlys)
            {
                RestoreBkmks(rDoc, aUpdater);
                RestoreRedlines(rDoc, aUpdater);
                RestoreUnoCursors(aUpdater);
                RestoreShellCursors(aUpdater);
            }
            if (eMode & RestoreMode::Flys)
            {
                RestoreFlys(rDoc, aUpdater, false, false);
            }
        }

        private:
            void SaveBkmks(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent);
            void RestoreBkmks(SwDoc& rDoc, updater_t const & rUpdater);
            void SaveRedlines(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent);
            void RestoreRedlines(SwDoc& rDoc, updater_t const & rUpdater);
            void SaveFlys(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent, bool bSaveFlySplit);
            void RestoreFlys(SwDoc& rDoc, updater_t const & rUpdater, bool bAuto, bool bAtStart);
            void SaveUnoCursors(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent);
            void RestoreUnoCursors(updater_t const & rUpdater);
            void SaveShellCursors(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent);
            void RestoreShellCursors(updater_t const & rUpdater);
            static const SwPosition& GetRightMarkPos(::sw::mark::MarkBase const * pMark, bool bOther)
                { return bOther ? pMark->GetOtherMarkPos() : pMark->GetMarkPos(); };
            static void SetRightMarkPos(MarkBase* pMark, bool bOther, const SwPosition* const pPos)
                { bOther ? pMark->SetOtherMarkPos(*pPos) : pMark->SetMarkPos(*pPos); };
    };
    void lcl_ChkPaM( std::vector<PaMEntry>& rPaMEntries, const SwNodeOffset nNode, const sal_Int32 nContent, SwPaM& rPaM, const bool bGetPoint, bool bSetMark)
    {
        const SwPosition* pPos = &rPaM.GetBound(bGetPoint);
        if( pPos->GetNodeIndex() == nNode && pPos->GetContentIndex() < nContent )
        {
            const PaMEntry aEntry = { &rPaM, bSetMark, pPos->GetContentIndex() };
            rPaMEntries.push_back(aEntry);
        }
    }
    void lcl_ChkPaMBoth( std::vector<PaMEntry>& rPaMEntries, const SwNodeOffset nNode, const sal_Int32 nContent, SwPaM& rPaM)
    {
        lcl_ChkPaM(rPaMEntries, nNode, nContent, rPaM, true, true);
        lcl_ChkPaM(rPaMEntries, nNode, nContent, rPaM, false, false);
    }
    void lcl_ChkUnoCrsrPaMBoth(std::vector<PaMEntry>& rPaMEntries, const SwNodeOffset nNode, const sal_Int32 nContent, SwPaM& rPaM)
    {
        lcl_ChkPaM(rPaMEntries, nNode, nContent, rPaM, true, false);
        lcl_ChkPaM(rPaMEntries, nNode, nContent, rPaM, false, true);
    }

#if 0
    static void DumpEntries(std::vector<MarkEntry>* pEntries)
    {
        for (MarkEntry& aEntry : *pEntries)
            aEntry.Dump();
    }
#endif
}

void ContentIdxStoreImpl::SaveBkmks(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent)
{
    IDocumentMarkAccess* const pMarkAccess = rDoc.getIDocumentMarkAccess();
    const auto ppBkmkEnd = pMarkAccess->getAllMarksEnd();
    for(
        auto ppBkmk = pMarkAccess->getAllMarksBegin();
        ppBkmk != ppBkmkEnd;
        ++ppBkmk)
    {
        const ::sw::mark::MarkBase* pBkmk = *ppBkmk;
        bool bMarkPosEqual = false;
        if(pBkmk->GetMarkPos().GetNodeIndex() == nNode
            && pBkmk->GetMarkPos().GetContentIndex() <= nContent)
        {
            if(pBkmk->GetMarkPos().GetContentIndex() < nContent)
            {
                const MarkEntry aEntry = { static_cast<tools::Long>(ppBkmk - pMarkAccess->getAllMarksBegin()), false, pBkmk->GetMarkPos().GetContentIndex() };
                m_aBkmkEntries.push_back(aEntry);
            }
            else // if a bookmark position is equal nContent, the other position
                bMarkPosEqual = true; // has to decide if it is added to the array
        }
        if(pBkmk->IsExpanded()
            && pBkmk->GetOtherMarkPos().GetNodeIndex() == nNode
            && pBkmk->GetOtherMarkPos().GetContentIndex() <= nContent)
        {
            if(bMarkPosEqual)
            { // the other position is before, the (main) position is equal
                const MarkEntry aEntry = { static_cast<tools::Long>(ppBkmk - pMarkAccess->getAllMarksBegin()), false, pBkmk->GetMarkPos().GetContentIndex() };
                m_aBkmkEntries.push_back(aEntry);
            }
            const MarkEntry aEntry = { static_cast<tools::Long>(ppBkmk - pMarkAccess->getAllMarksBegin()), true, pBkmk->GetOtherMarkPos().GetContentIndex() };
            m_aBkmkEntries.push_back(aEntry);
        }
    }
}

void ContentIdxStoreImpl::RestoreBkmks(SwDoc& rDoc, updater_t const & rUpdater)
{
    IDocumentMarkAccess* const pMarkAccess = rDoc.getIDocumentMarkAccess();
    sal_Int32 nMinIndexModified = SAL_MAX_INT32;
    for (const MarkEntry& aEntry : m_aBkmkEntries)
    {
        if (MarkBase *const pMark = pMarkAccess->getAllMarksBegin()[aEntry.m_nIdx])
        {
            nMinIndexModified = std::min(nMinIndexModified, sal_Int32(aEntry.m_nIdx));
            SwPosition aNewPos(GetRightMarkPos(pMark, aEntry.m_bOther));
            rUpdater(aNewPos, aEntry.m_nContent);
            SetRightMarkPos(pMark, aEntry.m_bOther, &aNewPos);
        }
    }
    if (!m_aBkmkEntries.empty())
    {   // tdf#105705 sort bookmarks because SaveBkmks special handling of
        // "bMarkPosEqual" may destroy sort order
        pMarkAccess->assureSortedMarkContainers(nMinIndexModified);
    }
}

void ContentIdxStoreImpl::SaveRedlines(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent)
{
    SwRedlineTable const & rRedlineTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
    tools::Long nIdx = 0;
    for (const SwRangeRedline* pRdl : rRedlineTable)
    {
        int nPointPos = lcl_RelativePosition( *pRdl->GetPoint(), nNode, nContent );
        int nMarkPos = pRdl->HasMark() ? lcl_RelativePosition( *pRdl->GetMark(), nNode, nContent ) :
                                          nPointPos;
        // #i59534: We have to store the positions inside the same node before the insert position
        // and the one at the insert position if the corresponding Point/Mark position is before
        // the insert position.
        if( nPointPos == BEFORE_SAME_NODE ||
            ( nPointPos == SAME_POSITION && nMarkPos < SAME_POSITION ) )
        {
            const MarkEntry aEntry = { nIdx, false, pRdl->GetPoint()->GetContentIndex() };
            m_aRedlineEntries.push_back(aEntry);
        }
        if( pRdl->HasMark() && ( nMarkPos == BEFORE_SAME_NODE ||
            ( nMarkPos == SAME_POSITION && nPointPos < SAME_POSITION ) ) )
        {
            const MarkEntry aEntry = { nIdx, true, pRdl->GetMark()->GetContentIndex() };
            m_aRedlineEntries.push_back(aEntry);
        }
        ++nIdx;
    }
}

void ContentIdxStoreImpl::RestoreRedlines(SwDoc& rDoc, updater_t const & rUpdater)
{
    const SwRedlineTable& rRedlTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
    for (const MarkEntry& aEntry : m_aRedlineEntries)
    {
        SwPosition* const pPos = aEntry.m_bOther
            ? rRedlTable[ aEntry.m_nIdx ]->GetMark()
            : rRedlTable[ aEntry.m_nIdx ]->GetPoint();
        rUpdater(*pPos, aEntry.m_nContent);
    }
}

void ContentIdxStoreImpl::SaveFlys(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent, bool bSaveFlySplit)
{
    SwContentNode *pNode = rDoc.GetNodes()[nNode]->GetContentNode();
    if( !pNode )
        return;
    SwFrame* pFrame = pNode->getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() );
    if( pFrame )
    {
        // sw_redlinehide: this looks like an invalid optimisation if merged,
        // assuming that flys in deleted redlines should be saved here too.
        if ((!pFrame->IsTextFrame() || !static_cast<SwTextFrame const*>(pFrame)->GetMergedPara())
                && !pFrame->GetDrawObjs())
            return; // if we have a layout and no DrawObjs, we can skip this
    }
    MarkEntry aSave = { 0, false, 0 };
    for (const SwFrameFormat* pFrameFormat : *rDoc.GetSpzFrameFormats())
    {
        if ( RES_FLYFRMFMT == pFrameFormat->Which() || RES_DRAWFRMFMT == pFrameFormat->Which() )
        {
            const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
            SwNode const*const pAnchorNode = rAnchor.GetAnchorNode();
            if ( pAnchorNode && ( nNode == pAnchorNode->GetIndex() ) &&
                 ( RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId() ||
                   RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId() ) )
            {
                bool bSkip = false;
                aSave.m_bOther = false;
                aSave.m_nContent = rAnchor.GetAnchorContentOffset();
                if ( RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId() )
                {
                    if( nContent <= aSave.m_nContent )
                    {
                        if( bSaveFlySplit )
                            aSave.m_bOther = true;
                        else
                            bSkip = true;
                    }
                }
                if(!bSkip)
                    m_aFlyEntries.push_back(aSave);
            }
        }
        ++aSave.m_nIdx;
    }
}

void ContentIdxStoreImpl::RestoreFlys(SwDoc& rDoc, updater_t const & rUpdater, bool bAuto, bool bAtStart)
{
    sw::SpzFrameFormats* pSpz = rDoc.GetSpzFrameFormats();
    for (const MarkEntry& aEntry : m_aFlyEntries)
    {
        if(!aEntry.m_bOther)
        {
            sw::SpzFrameFormat* pFrameFormat = (*pSpz)[ aEntry.m_nIdx ];
            const SwFormatAnchor& rFlyAnchor = pFrameFormat->GetAnchor();
            if( rFlyAnchor.GetContentAnchor() )
            {
                if(bAtStart && RndStdIds::FLY_AT_PARA == rFlyAnchor.GetAnchorId())
                    continue;
                SwFormatAnchor aNew( rFlyAnchor );
                SwPosition aNewPos( *rFlyAnchor.GetContentAnchor() );
                rUpdater(aNewPos, aEntry.m_nContent);
                if ( RndStdIds::FLY_AT_CHAR != rFlyAnchor.GetAnchorId() )
                {
                    aNewPos.nContent.Assign( nullptr, 0 );
                }
                aNew.SetAnchor( &aNewPos );
                pFrameFormat->SetFormatAttr( aNew );
            }
        }
        else if( bAuto )
        {
            SwFrameFormat* pFrameFormat = (*pSpz)[ aEntry.m_nIdx ];
            const SfxPoolItem* pAnchor = &pFrameFormat->GetAnchor();
            pFrameFormat->CallSwClientNotify(sw::LegacyModifyHint(pAnchor, pAnchor));
        }
    }
}

void ContentIdxStoreImpl::SaveUnoCursors(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent)
{
    rDoc.cleanupUnoCursorTable();
    for (const auto& pWeakUnoCursor : rDoc.mvUnoCursorTable)
    {
        auto pUnoCursor(pWeakUnoCursor.lock());
        if(!pUnoCursor)
            continue;
        for(SwPaM& rPaM : pUnoCursor->GetRingContainer())
        {
            lcl_ChkUnoCrsrPaMBoth(m_aUnoCursorEntries, nNode, nContent, rPaM);
        }
        const SwUnoTableCursor* pUnoTableCursor = dynamic_cast<const SwUnoTableCursor*>(pUnoCursor.get());
        if( pUnoTableCursor )
        {
            for(SwPaM& rPaM : const_cast<SwUnoTableCursor*>(pUnoTableCursor)->GetSelRing().GetRingContainer())
            {
                lcl_ChkUnoCrsrPaMBoth(m_aUnoCursorEntries, nNode, nContent, rPaM);
            }
        }
    }
}

void ContentIdxStoreImpl::RestoreUnoCursors(updater_t const & rUpdater)
{
    for (const PaMEntry& aEntry : m_aUnoCursorEntries)
    {
        rUpdater(aEntry.m_pPaM->GetBound(!aEntry.m_isMark), aEntry.m_nContent);
    }
}

void ContentIdxStoreImpl::SaveShellCursors(SwDoc& rDoc, SwNodeOffset nNode, sal_Int32 nContent)
{
    SwCursorShell* pShell = rDoc.GetEditShell();
    if (!pShell)
        return;
    for(SwViewShell& rCurShell : pShell->GetRingContainer())
    {
        if( auto pCursorShell = dynamic_cast<SwCursorShell *>(&rCurShell) )
        {
            SwPaM *_pStackCursor = pCursorShell->GetStackCursor();
            if( _pStackCursor )
                for (;;)
                {
                    lcl_ChkPaMBoth( m_aShellCursorEntries, nNode, nContent, *_pStackCursor);
                    if (!_pStackCursor)
                        break;
                    _pStackCursor = _pStackCursor->GetNext();
                    if (_pStackCursor == pCursorShell->GetStackCursor())
                        break;
                }

            for(SwPaM& rPaM : pCursorShell->GetCursor_()->GetRingContainer())
            {
                lcl_ChkPaMBoth( m_aShellCursorEntries, nNode, nContent, rPaM);
            }
        }
    }
}

void ContentIdxStoreImpl::RestoreShellCursors(updater_t const & rUpdater)
{
    for (const PaMEntry& aEntry : m_aShellCursorEntries)
    {
        rUpdater(aEntry.m_pPaM->GetBound(aEntry.m_isMark), aEntry.m_nContent);
    }
}

namespace sw::mark {
    std::shared_ptr<ContentIdxStore> ContentIdxStore::Create()
    {
       return std::make_shared<ContentIdxStoreImpl>();
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
