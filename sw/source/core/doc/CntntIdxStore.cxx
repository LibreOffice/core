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
#include <boost/function.hpp>
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
#include <ndtxt.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <redline.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unocrsr.hxx>
#include <edimp.hxx>
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

    static int lcl_RelativePosition( const SwPosition& rPos, sal_uLong nNode, sal_Int32 nContent )
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
        OffsetUpdater(SwContentNode* pNewContentNode, sal_Int32 nOffset)
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
        LimitUpdater(SwContentNode* pNewContentNode, sal_uLong nLen, sal_Int32 nCorrLen)
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
        std::vector<PaMEntry> m_aUnoCrsrEntries;
        std::vector<PaMEntry> m_aShellCrsrEntries;
        typedef boost::function<void (SwPosition& rPos, sal_Int32 nContent)> updater_t;
        virtual void Clear() SAL_OVERRIDE
        {
            m_aBkmkEntries.clear();
            m_aRedlineEntries.clear();
            m_aFlyEntries.clear();
            m_aUnoCrsrEntries.clear();
            m_aShellCrsrEntries.clear();
        }
        virtual bool Empty() SAL_OVERRIDE
        {
            return m_aBkmkEntries.empty() && m_aRedlineEntries.empty() && m_aFlyEntries.empty() && m_aUnoCrsrEntries.empty() && m_aShellCrsrEntries.empty();
        }
        virtual void Save(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent, bool bSaveFlySplit=false) SAL_OVERRIDE
        {
            SaveBkmks(pDoc, nNode, nContent);
            SaveRedlines(pDoc, nNode, nContent);
            SaveFlys(pDoc, nNode, nContent, bSaveFlySplit);
            SaveUnoCrsrs(pDoc, nNode, nContent);
            SaveShellCrsrs(pDoc, nNode, nContent);
        }
        virtual void Restore(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nOffset=0, bool bAuto = false) SAL_OVERRIDE
        {
            SwContentNode* pCNd = pDoc->GetNodes()[ nNode ]->GetContentNode();
            updater_t aUpdater = OffsetUpdater(pCNd, nOffset);
            RestoreBkmks(pDoc, aUpdater);
            RestoreRedlines(pDoc, aUpdater);
            RestoreFlys(pDoc, aUpdater, bAuto);
            RestoreUnoCrsrs(aUpdater);
            RestoreShellCrsrs(aUpdater);
        }
        virtual void Restore(SwNode& rNd, sal_Int32 nLen, sal_Int32 nCorrLen) SAL_OVERRIDE
        {
            SwContentNode* pCNd = rNd.GetContentNode();
            SwDoc* pDoc = rNd.GetDoc();
            updater_t aUpdater = LimitUpdater(pCNd, nLen, nCorrLen);
            RestoreBkmks(pDoc, aUpdater);
            RestoreRedlines(pDoc, aUpdater);
            RestoreFlys(pDoc, aUpdater, false);
            RestoreUnoCrsrs(aUpdater);
            RestoreShellCrsrs(aUpdater);
        }
        virtual ~ContentIdxStoreImpl(){};
        private:
            inline void SaveBkmks(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent);
            inline void RestoreBkmks(SwDoc* pDoc, updater_t& rUpdater);
            inline void SaveRedlines(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent);
            inline void RestoreRedlines(SwDoc* pDoc, updater_t& rUpdater);
            inline void SaveFlys(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent, bool bSaveFlySplit);
            inline void RestoreFlys(SwDoc* pDoc, updater_t& rUpdater, bool bAuto);
            inline void SaveUnoCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent);
            inline void RestoreUnoCrsrs(updater_t& rUpdater);
            inline void SaveShellCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent);
            inline void RestoreShellCrsrs(updater_t& rUpdater);
            static inline const SwPosition& GetRightMarkPos(::sw::mark::IMark* pMark, bool bOther)
                { return bOther ? pMark->GetOtherMarkPos() : pMark->GetMarkPos(); };
            static inline void SetRightMarkPos(MarkBase* pMark, bool bOther, const SwPosition* const pPos)
                { bOther ? pMark->SetOtherMarkPos(*pPos) : pMark->SetMarkPos(*pPos); };
    };
    static inline void lcl_ChkPaM( std::vector<PaMEntry>& rPaMEntries, const sal_uLong nNode, const sal_Int32 nContent, SwPaM& rPaM, const bool bPoint)
    {
        const SwPosition* pPos = &rPaM.GetBound( bPoint );
        if( pPos->nNode.GetIndex() == nNode && pPos->nContent.GetIndex() < nContent )
        {
            const PaMEntry aEntry = { &rPaM, bPoint, pPos->nContent.GetIndex() };
            rPaMEntries.push_back(aEntry);
        }
    }
    static inline void lcl_ChkPaMBoth( std::vector<PaMEntry>& rPaMEntries, const sal_uLong nNode, const sal_Int32 nContent, SwPaM& rPaM)
    {
        lcl_ChkPaM(rPaMEntries, nNode, nContent, rPaM, true);
        lcl_ChkPaM(rPaMEntries, nNode, nContent, rPaM, false);
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
                const MarkEntry aEntry = { ppBkmk - pMarkAccess->getAllMarksBegin(), false, pBkmk->GetMarkPos().nContent.GetIndex() };
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
                const MarkEntry aEntry = { ppBkmk - pMarkAccess->getAllMarksBegin(), false, pBkmk->GetMarkPos().nContent.GetIndex() };
                m_aBkmkEntries.push_back(aEntry);
            }
            const MarkEntry aEntry = { ppBkmk - pMarkAccess->getAllMarksBegin(), true, pBkmk->GetOtherMarkPos().nContent.GetIndex() };
            m_aBkmkEntries.push_back(aEntry);
        }
    }
}

void ContentIdxStoreImpl::RestoreBkmks(SwDoc* pDoc, updater_t& rUpdater)
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
}

void ContentIdxStoreImpl::SaveRedlines(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent)
{
    SwRedlineTable const & pRedlineTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    long int nIdx = 0;
    for (const SwRangeRedline* pRdl : pRedlineTable)
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

void ContentIdxStoreImpl::RestoreRedlines(SwDoc* pDoc, updater_t& rUpdater)
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
    SwFrm* pFrm = pNode->getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() );
    if( pFrm )
    {
        if( !pFrm->GetDrawObjs() )
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
                 ( FLY_AT_PARA == rAnchor.GetAnchorId() ||
                   FLY_AT_CHAR == rAnchor.GetAnchorId() ) )
            {
                bool bSkip = false;
                aSave.m_bOther = false;
                aSave.m_nContent = pAPos->nContent.GetIndex();
                if ( FLY_AT_CHAR == rAnchor.GetAnchorId() )
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

void ContentIdxStoreImpl::RestoreFlys(SwDoc* pDoc, updater_t& rUpdater, bool bAuto)
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
                if ( FLY_AT_CHAR != rFlyAnchor.GetAnchorId() )
                {
                    aNewPos.nContent.Assign( 0, 0 );
                }
                aNew.SetAnchor( &aNewPos );
                pFrameFormat->SetFormatAttr( aNew );
            }
        }
        else if( bAuto )
        {
            SwFrameFormat *pFrameFormat = (*pSpz)[ aEntry.m_nIdx ];
            SfxPoolItem *pAnchor = (SfxPoolItem*)&pFrameFormat->GetAnchor();
            pFrameFormat->NotifyClients( pAnchor, pAnchor );
        }
    }
}

void ContentIdxStoreImpl::SaveUnoCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent)
{
    for (auto pWeakUnoCrsr : pDoc->mvUnoCrsrTbl)
    {
        auto pUnoCrsr(pWeakUnoCrsr.lock());
        if(!pUnoCrsr)
            continue;
        for(SwPaM& rPaM : (const_cast<SwUnoCrsr*>(pUnoCrsr.get()))->GetRingContainer())
        {
            lcl_ChkPaMBoth( m_aUnoCrsrEntries, nNode, nContent, rPaM);
        }
        const SwUnoTableCrsr* pUnoTblCrsr = dynamic_cast<const SwUnoTableCrsr*>(pUnoCrsr.get());
        if( pUnoTblCrsr )
        {
            for(SwPaM& rPaM : (&(const_cast<SwUnoTableCrsr*>(pUnoTblCrsr))->GetSelRing())->GetRingContainer())
            {
                lcl_ChkPaMBoth( m_aUnoCrsrEntries, nNode, nContent, rPaM);
            }
        }
    }
}

void ContentIdxStoreImpl::RestoreUnoCrsrs(updater_t& rUpdater)
{
    for (const PaMEntry& aEntry : m_aUnoCrsrEntries)
    {
        rUpdater(aEntry.m_pPaM->GetBound(!aEntry.m_isMark), aEntry.m_nContent);
    }
}

void ContentIdxStoreImpl::SaveShellCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nContent)
{
    SwCrsrShell* pShell = pDoc->GetEditShell();
    if( !pShell )
        return;
    for(SwViewShell& rCurShell : pShell->GetRingContainer())
    {
        if( rCurShell.IsA( TYPE( SwCrsrShell )) )
        {
            SwPaM *_pStkCrsr = static_cast<SwCrsrShell*>(&rCurShell)->GetStkCrsr();
            if( _pStkCrsr )
                do {
                    lcl_ChkPaMBoth( m_aShellCrsrEntries, nNode, nContent, *_pStkCrsr);
                } while ( (_pStkCrsr != 0 ) &&
                    ((_pStkCrsr = _pStkCrsr->GetNext()) != static_cast<SwCrsrShell*>(&rCurShell)->GetStkCrsr()) );

            for(SwPaM& rPaM : (static_cast<SwCrsrShell*>(&rCurShell)->_GetCrsr())->GetRingContainer())
            {
                lcl_ChkPaMBoth( m_aShellCrsrEntries, nNode, nContent, rPaM);
            }
        }
    }
}

void ContentIdxStoreImpl::RestoreShellCrsrs(updater_t& rUpdater)
{
    for (const PaMEntry& aEntry : m_aShellCrsrEntries)
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
