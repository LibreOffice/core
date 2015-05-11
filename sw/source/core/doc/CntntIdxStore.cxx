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

    static int lcl_RelativePosition( const SwPosition& rPos, sal_uLong nNode, sal_Int32 nCntnt )
    {
        sal_uLong nIndex = rPos.nNode.GetIndex();
        int nReturn = BEFORE_NODE;
        if( nIndex == nNode )
        {
            const sal_Int32 nCntIdx = rPos.nContent.GetIndex();
            if( nCntIdx < nCntnt )
                nReturn = BEFORE_SAME_NODE;
            else if( nCntIdx == nCntnt )
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
        sal_Int32 m_nCntnt;
#if 0
        void Dump()
        {
            SAL_INFO("sw.core", "Index: " << m_nIdx << "\tOther: " << m_bOther << "\tContent: " << m_nCntnt);
        }
#endif
    };
    struct PaMEntry
    {
        SwPaM* m_pPaM;
        bool m_isMark;
        sal_Int32 m_nCntnt;
    };
    struct OffsetUpdater
    {
        const SwCntntNode* m_pNewCntntNode;
        const sal_Int32 m_nOffset;
        OffsetUpdater(SwCntntNode* pNewCntntNode, sal_Int32 nOffset)
            : m_pNewCntntNode(pNewCntntNode), m_nOffset(nOffset) {};
        void operator()(SwPosition& rPos, sal_Int32 nCntnt) const
        {
            rPos.nNode = *m_pNewCntntNode;
            rPos.nContent.Assign(const_cast<SwCntntNode*>(m_pNewCntntNode), nCntnt + m_nOffset);
        };
    };
    struct LimitUpdater
    {
        const SwCntntNode* m_pNewCntntNode;
        const sal_uLong m_nLen;
        const sal_Int32 m_nCorrLen;
        LimitUpdater(SwCntntNode* pNewCntntNode, sal_uLong nLen, sal_Int32 nCorrLen)
            : m_pNewCntntNode(pNewCntntNode), m_nLen(nLen), m_nCorrLen(nCorrLen) {};
        void operator()(SwPosition& rPos, sal_Int32 nCntnt) const
        {
            rPos.nNode = *m_pNewCntntNode;
            if( nCntnt < m_nCorrLen )
            {
                rPos.nContent.Assign(const_cast<SwCntntNode*>(m_pNewCntntNode), std::min( nCntnt, static_cast<sal_Int32>(m_nLen) ) );
            }
            else
            {
                rPos.nContent -= m_nCorrLen;
            }
        };
    };
    struct CntntIdxStoreImpl : sw::mark::CntntIdxStore
    {
        std::vector<MarkEntry> m_aBkmkEntries;
        std::vector<MarkEntry> m_aRedlineEntries;
        std::vector<MarkEntry> m_aFlyEntries;
        std::vector<PaMEntry> m_aUnoCrsrEntries;
        std::vector<PaMEntry> m_aShellCrsrEntries;
        typedef boost::function<void (SwPosition& rPos, sal_Int32 nCntnt)> updater_t;
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
        virtual void Save(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt, bool bSaveFlySplit=false) SAL_OVERRIDE
        {
            SaveBkmks(pDoc, nNode, nCntnt);
            SaveRedlines(pDoc, nNode, nCntnt);
            SaveFlys(pDoc, nNode, nCntnt, bSaveFlySplit);
            SaveUnoCrsrs(pDoc, nNode, nCntnt);
            SaveShellCrsrs(pDoc, nNode, nCntnt);
        }
        virtual void Restore(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nOffset=0, bool bAuto = false) SAL_OVERRIDE
        {
            SwCntntNode* pCNd = pDoc->GetNodes()[ nNode ]->GetCntntNode();
            updater_t aUpdater = OffsetUpdater(pCNd, nOffset);
            RestoreBkmks(pDoc, aUpdater);
            RestoreRedlines(pDoc, aUpdater);
            RestoreFlys(pDoc, aUpdater, bAuto);
            RestoreUnoCrsrs(aUpdater);
            RestoreShellCrsrs(aUpdater);
        }
        virtual void Restore(SwNode& rNd, sal_Int32 nLen, sal_Int32 nCorrLen) SAL_OVERRIDE
        {
            SwCntntNode* pCNd = rNd.GetCntntNode();
            SwDoc* pDoc = rNd.GetDoc();
            updater_t aUpdater = LimitUpdater(pCNd, nLen, nCorrLen);
            RestoreBkmks(pDoc, aUpdater);
            RestoreRedlines(pDoc, aUpdater);
            RestoreFlys(pDoc, aUpdater, false);
            RestoreUnoCrsrs(aUpdater);
            RestoreShellCrsrs(aUpdater);
        }
        virtual ~CntntIdxStoreImpl(){};
        private:
            inline void SaveBkmks(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt);
            inline void RestoreBkmks(SwDoc* pDoc, updater_t& rUpdater);
            inline void SaveRedlines(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt);
            inline void RestoreRedlines(SwDoc* pDoc, updater_t& rUpdater);
            inline void SaveFlys(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt, bool bSaveFlySplit);
            inline void RestoreFlys(SwDoc* pDoc, updater_t& rUpdater, bool bAuto);
            inline void SaveUnoCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt);
            inline void RestoreUnoCrsrs(updater_t& rUpdater);
            inline void SaveShellCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt);
            inline void RestoreShellCrsrs(updater_t& rUpdater);
            static inline const SwPosition& GetRightMarkPos(::sw::mark::IMark* pMark, bool bOther)
                { return bOther ? pMark->GetOtherMarkPos() : pMark->GetMarkPos(); };
            static inline void SetRightMarkPos(MarkBase* pMark, bool bOther, const SwPosition* const pPos)
                { bOther ? pMark->SetOtherMarkPos(*pPos) : pMark->SetMarkPos(*pPos); };
    };
    static inline void lcl_ChkPaM( std::vector<PaMEntry>& rPaMEntries, const sal_uLong nNode, const sal_Int32 nCntnt, SwPaM& rPaM, const bool bPoint)
    {
        const SwPosition* pPos = &rPaM.GetBound( bPoint );
        if( pPos->nNode.GetIndex() == nNode && pPos->nContent.GetIndex() < nCntnt )
        {
            const PaMEntry aEntry = { &rPaM, bPoint, pPos->nContent.GetIndex() };
            rPaMEntries.push_back(aEntry);
        }
    }
    static inline void lcl_ChkPaMBoth( std::vector<PaMEntry>& rPaMEntries, const sal_uLong nNode, const sal_Int32 nCntnt, SwPaM& rPaM)
    {
        lcl_ChkPaM(rPaMEntries, nNode, nCntnt, rPaM, true);
        lcl_ChkPaM(rPaMEntries, nNode, nCntnt, rPaM, false);
    }

#if 0
    static void DumpEntries(std::vector<MarkEntry>* pEntries)
    {
        for (MarkEntry& aEntry : *pEntries)
            aEntry.Dump();
    }
#endif
}

void CntntIdxStoreImpl::SaveBkmks(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt)
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
            && pBkmk->GetMarkPos().nContent.GetIndex() <= nCntnt)
        {
            if(pBkmk->GetMarkPos().nContent.GetIndex() < nCntnt)
            {
                const MarkEntry aEntry = { ppBkmk - pMarkAccess->getAllMarksBegin(), false, pBkmk->GetMarkPos().nContent.GetIndex() };
                m_aBkmkEntries.push_back(aEntry);
            }
            else // if a bookmark position is equal nCntnt, the other position
                bMarkPosEqual = true; // has to decide if it is added to the array
        }
        if(pBkmk->IsExpanded()
            && pBkmk->GetOtherMarkPos().nNode.GetIndex() == nNode
            && pBkmk->GetOtherMarkPos().nContent.GetIndex() <= nCntnt)
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

void CntntIdxStoreImpl::RestoreBkmks(SwDoc* pDoc, updater_t& rUpdater)
{
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    for (const MarkEntry& aEntry : m_aBkmkEntries)
    {
        if (MarkBase* pMark = dynamic_cast<MarkBase*>(pMarkAccess->getAllMarksBegin()[aEntry.m_nIdx].get()))
        {
            SwPosition aNewPos(GetRightMarkPos(pMark, aEntry.m_bOther));
            rUpdater(aNewPos, aEntry.m_nCntnt);
            SetRightMarkPos(pMark, aEntry.m_bOther, &aNewPos);
        }
    }
}

void CntntIdxStoreImpl::SaveRedlines(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt)
{
    SwRedlineTbl const & pRedlineTbl = pDoc->getIDocumentRedlineAccess().GetRedlineTbl();
    long int nIdx = 0;
    for (const SwRangeRedline* pRdl : pRedlineTbl)
    {
        int nPointPos = lcl_RelativePosition( *pRdl->GetPoint(), nNode, nCntnt );
        int nMarkPos = pRdl->HasMark() ? lcl_RelativePosition( *pRdl->GetMark(), nNode, nCntnt ) :
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

void CntntIdxStoreImpl::RestoreRedlines(SwDoc* pDoc, updater_t& rUpdater)
{
    const SwRedlineTbl& rRedlTbl = pDoc->getIDocumentRedlineAccess().GetRedlineTbl();
    for (const MarkEntry& aEntry : m_aRedlineEntries)
    {
        SwPosition* const pPos = aEntry.m_bOther
            ? rRedlTbl[ aEntry.m_nIdx ]->GetMark()
            : rRedlTbl[ aEntry.m_nIdx ]->GetPoint();
        rUpdater(*pPos, aEntry.m_nCntnt);
    }
}

void CntntIdxStoreImpl::SaveFlys(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt, bool bSaveFlySplit)
{
    SwCntntNode *pNode = pDoc->GetNodes()[nNode]->GetCntntNode();
    if( !pNode )
        return;
    SwFrm* pFrm = pNode->getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() );
    if( pFrm )
    {
        if( !pFrm->GetDrawObjs() )
            return; // if we have a layout and no DrawObjs, we can skip this
    }
    MarkEntry aSave = { 0, false, 0 };
    for (const SwFrmFmt* pFrmFmt : *pDoc->GetSpzFrmFmts())
    {
        if ( RES_FLYFRMFMT == pFrmFmt->Which() || RES_DRAWFRMFMT == pFrmFmt->Which() )
        {
            const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
            SwPosition const*const pAPos = rAnchor.GetCntntAnchor();
            if ( pAPos && ( nNode == pAPos->nNode.GetIndex() ) &&
                 ( FLY_AT_PARA == rAnchor.GetAnchorId() ||
                   FLY_AT_CHAR == rAnchor.GetAnchorId() ) )
            {
                bool bSkip = false;
                aSave.m_bOther = false;
                aSave.m_nCntnt = pAPos->nContent.GetIndex();
                if ( FLY_AT_CHAR == rAnchor.GetAnchorId() )
                {
                    if( nCntnt <= aSave.m_nCntnt )
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

void CntntIdxStoreImpl::RestoreFlys(SwDoc* pDoc, updater_t& rUpdater, bool bAuto)
{
    SwFrmFmts* pSpz = pDoc->GetSpzFrmFmts();
    for (const MarkEntry& aEntry : m_aFlyEntries)
    {
        if(!aEntry.m_bOther)
        {
            SwFrmFmt *pFrmFmt = (*pSpz)[ aEntry.m_nIdx ];
            const SwFmtAnchor& rFlyAnchor = pFrmFmt->GetAnchor();
            if( rFlyAnchor.GetCntntAnchor() )
            {
                SwFmtAnchor aNew( rFlyAnchor );
                SwPosition aNewPos( *rFlyAnchor.GetCntntAnchor() );
                rUpdater(aNewPos, aEntry.m_nCntnt);
                if ( FLY_AT_CHAR != rFlyAnchor.GetAnchorId() )
                {
                    aNewPos.nContent.Assign( 0, 0 );
                }
                aNew.SetAnchor( &aNewPos );
                pFrmFmt->SetFmtAttr( aNew );
            }
        }
        else if( bAuto )
        {
            SwFrmFmt *pFrmFmt = (*pSpz)[ aEntry.m_nIdx ];
            SfxPoolItem *pAnchor = (SfxPoolItem*)&pFrmFmt->GetAnchor();
            pFrmFmt->NotifyClients( pAnchor, pAnchor );
        }
    }
}

void CntntIdxStoreImpl::SaveUnoCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt)
{
    for (const SwUnoCrsr* pUnoCrsr : pDoc->GetUnoCrsrTbl())
    {
        for(SwPaM& rPaM : (const_cast<SwUnoCrsr*>(pUnoCrsr))->GetRingContainer())
        {
            lcl_ChkPaMBoth( m_aUnoCrsrEntries, nNode, nCntnt, rPaM);
        }
        const SwUnoTableCrsr* pUnoTblCrsr = dynamic_cast<const SwUnoTableCrsr*>(pUnoCrsr);
        if( pUnoTblCrsr )
        {
            for(SwPaM& rPaM : (&(const_cast<SwUnoTableCrsr*>(pUnoTblCrsr))->GetSelRing())->GetRingContainer())
            {
                lcl_ChkPaMBoth( m_aUnoCrsrEntries, nNode, nCntnt, rPaM);
            }
        }
    }
}

void CntntIdxStoreImpl::RestoreUnoCrsrs(updater_t& rUpdater)
{
    for (const PaMEntry& aEntry : m_aUnoCrsrEntries)
    {
        rUpdater(aEntry.m_pPaM->GetBound(!aEntry.m_isMark), aEntry.m_nCntnt);
    }
}

void CntntIdxStoreImpl::SaveShellCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt)
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
                    lcl_ChkPaMBoth( m_aShellCrsrEntries, nNode, nCntnt, *_pStkCrsr);
                } while ( (_pStkCrsr != 0 ) &&
                    ((_pStkCrsr = _pStkCrsr->GetNext()) != static_cast<SwCrsrShell*>(&rCurShell)->GetStkCrsr()) );

            for(SwPaM& rPaM : (static_cast<SwCrsrShell*>(&rCurShell)->_GetCrsr())->GetRingContainer())
            {
                lcl_ChkPaMBoth( m_aShellCrsrEntries, nNode, nCntnt, rPaM);
            }
        }
    }
}

void CntntIdxStoreImpl::RestoreShellCrsrs(updater_t& rUpdater)
{
    for (const PaMEntry& aEntry : m_aShellCrsrEntries)
    {
        rUpdater(aEntry.m_pPaM->GetBound(aEntry.m_isMark), aEntry.m_nCntnt);
    }
}

namespace sw { namespace mark {
    std::shared_ptr<CntntIdxStore> CntntIdxStore::Create()
    {
       return std::make_shared<CntntIdxStoreImpl>();
    }
}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
