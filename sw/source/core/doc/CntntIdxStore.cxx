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

#include <bookmrk.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <MarkManager.hxx>
#include <docary.hxx>
#include <editsh.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <functional>
#include <mvsave.hxx>
#include <ndtxt.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <redline.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unocrsr.hxx>
#include <edimp.hxx>
#include <txtfrm.hxx>
#include <memory>

using namespace ::boost;
using namespace ::sw::mark;

namespace
{
    // #i59534: If a paragraph will be splitted we have to restore some redline positions
    // This help function checks a position compared with a node and an content index

    static const int BEFORE_NODE = 0;          // Position before the given node index
    static const int BEFORE_SAME_NODE = 1;     // Same node index but content index before given content index
    static const int SAME_POSITION = 2;        // Same node index and samecontent index
    static const int BEHIND_SAME_NODE = 3;     // Same node index but content index behind given content index
    static const int BEHIND_NODE = 4;          // Position behind the given node index

    int lcl_RelativePosition( const SwPosition& rPos, sal_uLong nNode, sal_Int32 nContent )
    {
        sal_uLong nIndex = rPos.nNode.GetIndex();
        int nReturn = BEFORE_NODE;
        if( nIndex == nNode )
        {
            const sal_Int32 nCntIdx = rPos.nContent.GetIndex();
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
        long int m_nIdx;
        bool m_bOther;
        sal_Int32 m_nContent;
#if 0
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
            rPos.nNode = *m_pNewContentNode;
            rPos.nContent.Assign(const_cast<SwContentNode*>(m_pNewContentNode), nContent + m_nOffset);
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
            rPos.nNode = *m_pNewContentNode;
            if( nContent < m_nCorrLen )
            {
                rPos.nContent.Assign(const_cast<SwContentNode*>(m_pNewContentNode), std::min( nContent, static_cast<sal_Int32>(m_nLen) ) );
            }
            else
            {
                rPos.nContent -= m_nCorrLen;
            }
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
        virtual void Clear() override
        {
            m_aBkmkEntries.clear();
            m_aRedlineEntries.clear();
            m_aFlyEntries.clear();
            m_aUnoCursorEntries.clear();
            m_aShellCursorEntries.clear();
        }
        virtual bool Empty() override
        {
            return m_aBkmkEntries.empty() && m_aRedlineEntries.empty() && m_aFlyEntries.empty() && m_aUnoCursorEntries.empty() && m_aShellCursorEntries.empty();
        }
        virtual void Save(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent, bool bSaveFlySplit=false) override
        {
            SaveBkmks(pDoc, nNode, nContent);
            SaveRedlines(pDoc, nNode, nContent);
            SaveFlys(pDoc, nNode, nContent, bSaveFlySplit);
            SaveUnoCursors(pDoc, nNode, nContent);
            SaveShellCursors(pDoc, nNode, nContent);
        }
        virtual void Restore(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nOffset=0, bool bAuto = false) override
        {
            SwContentNode* pCNd = pDoc->GetNodes()[ nNode ]->GetContentNode();
            updater_t aUpdater = OffsetUpdater(pCNd, nOffset);
            RestoreBkmks(pDoc, aUpdater);
            RestoreRedlines(pDoc, aUpdater);
            RestoreFlys(pDoc, aUpdater, bAuto);
            RestoreUnoCursors(aUpdater);
            RestoreShellCursors(aUpdater);
        }
        virtual void Restore(SwNode& rNd, sal_Int32 nLen, sal_Int32 nCorrLen) override
        {
            SwContentNode* pCNd = rNd.GetContentNode();
            SwDoc* pDoc = rNd.GetDoc();
            updater_t aUpdater = LimitUpdater(pCNd, nLen, nCorrLen);
            RestoreBkmks(pDoc, aUpdater);
            RestoreRedlines(pDoc, aUpdater);
            RestoreFlys(pDoc, aUpdater, false);
            RestoreUnoCursors(aUpdater);
            RestoreShellCursors(aUpdater);
        }

        private:
            inline void SaveBkmks(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent);
            inline void RestoreBkmks(SwDoc* pDoc, updater_t const & rUpdater);
            inline void SaveRedlines(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent);
            inline void RestoreRedlines(SwDoc* pDoc, updater_t const & rUpdater);
            inline void SaveFlys(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent, bool bSaveFlySplit);
            inline void RestoreFlys(SwDoc* pDoc, updater_t const & rUpdater, bool bAuto);
            inline void SaveUnoCursors(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent);
            inline void RestoreUnoCursors(updater_t const & rUpdater);
            inline void SaveShellCursors(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent);
            inline void RestoreShellCursors(updater_t const & rUpdater);
            static const SwPosition& GetRightMarkPos(::sw::mark::IMark const * pMark, bool bOther)
                { return bOther ? pMark->GetOtherMarkPos() : pMark->GetMarkPos(); };
            static void SetRightMarkPos(MarkBase* pMark, bool bOther, const SwPosition* const pPos)
                { bOther ? pMark->SetOtherMarkPos(*pPos) : pMark->SetMarkPos(*pPos); };
    };
    inline void lcl_ChkPaM( std::vector<PaMEntry>& rPaMEntries, const sal_uLong nNode, const sal_Int32 nContent, SwPaM& rPaM, const bool bGetPoint, bool bSetMark)
    {
        const SwPosition* pPos = &rPaM.GetBound(bGetPoint);
        if( pPos->nNode.GetIndex() == nNode && pPos->nContent.GetIndex() < nContent )
        {
            const PaMEntry aEntry = { &rPaM, bSetMark, pPos->nContent.GetIndex() };
            rPaMEntries.push_back(aEntry);
        }
    }
    inline void lcl_ChkPaMBoth( std::vector<PaMEntry>& rPaMEntries, const sal_uLong nNode, const sal_Int32 nContent, SwPaM& rPaM)
    {
        lcl_ChkPaM(rPaMEntries, nNode, nContent, rPaM, true, true);
        lcl_ChkPaM(rPaMEntries, nNode, nContent, rPaM, false, false);
    }
    inline void lcl_ChkUnoCrsrPaMBoth(std::vector<PaMEntry>& rPaMEntries, const sal_uLong nNode, const sal_Int32 nContent, SwPaM& rPaM)
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

void ContentIdxStoreImpl::SaveBkmks(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent)
{
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    const IDocumentMarkAccess::const_iterator_t ppBkmkEnd = pMarkAccess->getAllMarksEnd();
    for(
        IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->getAllMarksBegin();
        ppBkmk != ppBkmkEnd;
        ++ppBkmk)
    {
        const ::sw::mark::IMark* pBkmk = ppBkmk->get();
        bool bMarkPosEqual = false;
        if(pBkmk->GetMarkPos().nNode.GetIndex() == nNode
            && pBkmk->GetMarkPos().nContent.GetIndex() <= nContent)
        {
            if(pBkmk->GetMarkPos().nContent.GetIndex() < nContent)
            {
                const MarkEntry aEntry = { static_cast<long>(ppBkmk - pMarkAccess->getAllMarksBegin()), false, pBkmk->GetMarkPos().nContent.GetIndex() };
                m_aBkmkEntries.push_back(aEntry);
            }
            else // if a bookmark position is equal nContent, the other position
                bMarkPosEqual = true; // has to decide if it is added to the array
        }
        if(pBkmk->IsExpanded()
            && pBkmk->GetOtherMarkPos().nNode.GetIndex() == nNode
            && pBkmk->GetOtherMarkPos().nContent.GetIndex() <= nContent)
        {
            if(bMarkPosEqual)
            { // the other position is before, the (main) position is equal
                const MarkEntry aEntry = { static_cast<long>(ppBkmk - pMarkAccess->getAllMarksBegin()), false, pBkmk->GetMarkPos().nContent.GetIndex() };
                m_aBkmkEntries.push_back(aEntry);
            }
            const MarkEntry aEntry = { static_cast<long>(ppBkmk - pMarkAccess->getAllMarksBegin()), true, pBkmk->GetOtherMarkPos().nContent.GetIndex() };
            m_aBkmkEntries.push_back(aEntry);
        }
    }
}

void ContentIdxStoreImpl::RestoreBkmks(SwDoc* pDoc, updater_t const & rUpdater)
{
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    for (const MarkEntry& aEntry : m_aBkmkEntries)
    {
        if (MarkBase* pMark = dynamic_cast<MarkBase*>(pMarkAccess->getAllMarksBegin()[aEntry.m_nIdx].get()))
        {
            SwPosition aNewPos(GetRightMarkPos(pMark, aEntry.m_bOther));
            rUpdater(aNewPos, aEntry.m_nContent);
            SetRightMarkPos(pMark, aEntry.m_bOther, &aNewPos);
        }
    }
    if (!m_aBkmkEntries.empty())
    {   // tdf#105705 sort bookmarks because SaveBkmks special handling of
        // "bMarkPosEqual" may destroy sort order
        dynamic_cast<sw::mark::MarkManager&>(*pMarkAccess).sortMarks();
    }
}

void ContentIdxStoreImpl::SaveRedlines(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent)
{
    SwRedlineTable const & rRedlineTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    long int nIdx = 0;
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
            const MarkEntry aEntry = { nIdx, false, pRdl->GetPoint()->nContent.GetIndex() };
            m_aRedlineEntries.push_back(aEntry);
        }
        if( pRdl->HasMark() && ( nMarkPos == BEFORE_SAME_NODE ||
            ( nMarkPos == SAME_POSITION && nPointPos < SAME_POSITION ) ) )
        {
            const MarkEntry aEntry = { nIdx, true, pRdl->GetMark()->nContent.GetIndex() };
            m_aRedlineEntries.push_back(aEntry);
        }
        ++nIdx;
    }
}

void ContentIdxStoreImpl::RestoreRedlines(SwDoc* pDoc, updater_t const & rUpdater)
{
    const SwRedlineTable& rRedlTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    for (const MarkEntry& aEntry : m_aRedlineEntries)
    {
        SwPosition* const pPos = aEntry.m_bOther
            ? rRedlTable[ aEntry.m_nIdx ]->GetMark()
            : rRedlTable[ aEntry.m_nIdx ]->GetPoint();
        rUpdater(*pPos, aEntry.m_nContent);
    }
}

void ContentIdxStoreImpl::SaveFlys(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent, bool bSaveFlySplit)
{
    SwContentNode *pNode = pDoc->GetNodes()[nNode]->GetContentNode();
    if( !pNode )
        return;
    SwFrame* pFrame = pNode->getLayoutFrame( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() );
    if( pFrame )
    {
        // sw_redlinehide: this looks like an invalid optimisation if merged,
        // assuming that flys in deleted redlines should be saved here too.
        if ((!pFrame->IsTextFrame() || !static_cast<SwTextFrame const*>(pFrame)->GetMergedPara())
                && !pFrame->GetDrawObjs())
            return; // if we have a layout and no DrawObjs, we can skip this
    }
    MarkEntry aSave = { 0, false, 0 };
    for (const SwFrameFormat* pFrameFormat : *pDoc->GetSpzFrameFormats())
    {
        if ( RES_FLYFRMFMT == pFrameFormat->Which() || RES_DRAWFRMFMT == pFrameFormat->Which() )
        {
            const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
            SwPosition const*const pAPos = rAnchor.GetContentAnchor();
            if ( pAPos && ( nNode == pAPos->nNode.GetIndex() ) &&
                 ( RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId() ||
                   RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId() ) )
            {
                bool bSkip = false;
                aSave.m_bOther = false;
                aSave.m_nContent = pAPos->nContent.GetIndex();
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

void ContentIdxStoreImpl::RestoreFlys(SwDoc* pDoc, updater_t const & rUpdater, bool bAuto)
{
    SwFrameFormats* pSpz = pDoc->GetSpzFrameFormats();
    for (const MarkEntry& aEntry : m_aFlyEntries)
    {
        if(!aEntry.m_bOther)
        {
            SwFrameFormat *pFrameFormat = (*pSpz)[ aEntry.m_nIdx ];
            const SwFormatAnchor& rFlyAnchor = pFrameFormat->GetAnchor();
            if( rFlyAnchor.GetContentAnchor() )
            {
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
            SwFrameFormat *pFrameFormat = (*pSpz)[ aEntry.m_nIdx ];
            SfxPoolItem const *pAnchor = static_cast<SfxPoolItem const *>(&pFrameFormat->GetAnchor());
            pFrameFormat->NotifyClients( pAnchor, pAnchor );
        }
    }
}

void ContentIdxStoreImpl::SaveUnoCursors(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent)
{
    pDoc->cleanupUnoCursorTable();
    for (const auto& pWeakUnoCursor : pDoc->mvUnoCursorTable)
    {
        auto pUnoCursor(pWeakUnoCursor.lock());
        if(!pUnoCursor)
            continue;
        for(SwPaM& rPaM : pUnoCursor.get()->GetRingContainer())
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

void ContentIdxStoreImpl::SaveShellCursors(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent)
{
    SwCursorShell* pShell = pDoc->GetEditShell();
    if( !pShell )
        return;
    for(SwViewShell& rCurShell : pShell->GetRingContainer())
    {
        if( dynamic_cast<const SwCursorShell *>(&rCurShell) != nullptr )
        {
            SwPaM *_pStackCursor = static_cast<SwCursorShell*>(&rCurShell)->GetStackCursor();
            if( _pStackCursor )
                do {
                    lcl_ChkPaMBoth( m_aShellCursorEntries, nNode, nContent, *_pStackCursor);
                } while ( (_pStackCursor != nullptr ) &&
                    ((_pStackCursor = _pStackCursor->GetNext()) != static_cast<SwCursorShell*>(&rCurShell)->GetStackCursor()) );

            for(SwPaM& rPaM : static_cast<SwCursorShell*>(&rCurShell)->GetCursor_()->GetRingContainer())
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

namespace sw { namespace mark {
    std::shared_ptr<ContentIdxStore> ContentIdxStore::Create()
    {
       return std::make_shared<ContentIdxStoreImpl>();
    }
}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
