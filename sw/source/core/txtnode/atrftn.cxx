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

#include <doc.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <swfont.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <ftnfrm.hxx>
#include <ndindex.hxx>
#include <fmtftntx.hxx>
#include <section.hxx>
#include <switerator.hxx>

namespace {
    
    
    
    
    
    static std::set<sal_uInt16> lcl_GetUsedFtnRefNumbers(SwDoc &rDoc,
                                                         SwTxtFtn *pExclude,
                                                         std::vector<SwTxtFtn*> &rInvalid)
    {
        std::set<sal_uInt16> aArr;
        SwFtnIdxs& ftnIdxs = rDoc.GetFtnIdxs();

        rInvalid.clear();

        for( sal_uInt16 n = 0; n < ftnIdxs.size(); ++n )
        {
            SwTxtFtn* pTxtFtn = ftnIdxs[ n ];
            if ( pTxtFtn != pExclude )
            {
                if ( USHRT_MAX == pTxtFtn->GetSeqRefNo() )
                {
                    rInvalid.push_back(pTxtFtn);
                }
                else
                {
                    aArr.insert( pTxtFtn->GetSeqRefNo() );
                }
            }
        }
        return aArr;
    }

    
    
    
    
    static bool lcl_IsRefNumAvailable(std::set<sal_uInt16> &rUsedNums,
                                         sal_uInt16 requested)
    {
        if ( USHRT_MAX == requested )
            return false;  
        if ( rUsedNums.count(requested) )
            return false;  
        return true;
    }

    
    
    
    
    static std::vector<sal_uInt16> lcl_GetUnusedSeqRefNums(std::set<sal_uInt16> &rUsedNums,
                                                           size_t numRequired)
    {
        std::vector<sal_uInt16> unusedNums;
        sal_uInt16 newNum = 0;
        std::set<sal_uInt16>::iterator it;
        
        for( it = rUsedNums.begin(); it != rUsedNums.end(); ++it )
        {
            while ( newNum < *it )
            {
                unusedNums.push_back( newNum++ );
                if ( unusedNums.size() >= numRequired )
                    return unusedNums;
            }
            newNum++;
        }
        
        while ( unusedNums.size() < numRequired )
        {
            unusedNums.push_back( newNum++ );
        }

        return unusedNums;
    }

}

/*************************************************************************
|*
|*    class SwFmtFtn
|*
*************************************************************************/


SwFmtFtn::SwFmtFtn( bool bEndNote )
    : SfxPoolItem( RES_TXTATR_FTN ),
    m_pTxtAttr( 0 ),
    m_nNumber( 0 ),
    m_bEndNote( bEndNote )
{
}


bool SwFmtFtn::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return m_nNumber  == ((SwFmtFtn&)rAttr).m_nNumber &&
           m_aNumber  == ((SwFmtFtn&)rAttr).m_aNumber &&
           m_bEndNote == ((SwFmtFtn&)rAttr).m_bEndNote;
}


SfxPoolItem* SwFmtFtn::Clone( SfxItemPool* ) const
{
    SwFmtFtn* pNew  = new SwFmtFtn;
    pNew->m_aNumber = m_aNumber;
    pNew->m_nNumber = m_nNumber;
    pNew->m_bEndNote = m_bEndNote;
    return pNew;
}

void SwFmtFtn::SetEndNote( bool b )
{
    if ( b != m_bEndNote )
    {
        if ( GetTxtFtn() )
        {
            GetTxtFtn()->DelFrms(0);
        }
        m_bEndNote = b;
    }
}

SwFmtFtn::~SwFmtFtn()
{
}


void SwFmtFtn::GetFtnText( OUString& rStr ) const
{
    if( m_pTxtAttr->GetStartNode() )
    {
        SwNodeIndex aIdx( *m_pTxtAttr->GetStartNode(), 1 );
        SwCntntNode* pCNd = aIdx.GetNode().GetTxtNode();
        if( !pCNd )
            pCNd = aIdx.GetNodes().GoNext( &aIdx );

        if( pCNd->IsTxtNode() ) {
            rStr = ((SwTxtNode*)pCNd)->GetExpandTxt();

            ++aIdx;
            while ( !aIdx.GetNode().IsEndNode() ) {
                if ( aIdx.GetNode().IsTxtNode() )
                    rStr += "  " + ((SwTxtNode*)(aIdx.GetNode().GetTxtNode()))->GetExpandTxt();
                ++aIdx;
            }
        }
    }
}

    
OUString SwFmtFtn::GetViewNumStr( const SwDoc& rDoc, sal_Bool bInclStrings ) const
{
    OUString sRet( GetNumStr() );
    if( sRet.isEmpty() )
    {
        
        
        bool bMakeNum = true;
        const SwSectionNode* pSectNd = m_pTxtAttr
                    ? SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr( *m_pTxtAttr )
                    : 0;

        if( pSectNd )
        {
            const SwFmtFtnEndAtTxtEnd& rFtnEnd = (SwFmtFtnEndAtTxtEnd&)
                pSectNd->GetSection().GetFmt()->GetFmtAttr(
                                IsEndNote() ?
                                static_cast<sal_uInt16>(RES_END_AT_TXTEND) :
                                static_cast<sal_uInt16>(RES_FTN_AT_TXTEND) );

            if( FTNEND_ATTXTEND_OWNNUMANDFMT == rFtnEnd.GetValue() )
            {
                bMakeNum = false;
                sRet = rFtnEnd.GetSwNumType().GetNumStr( GetNumber() );
                if( bInclStrings )
                {
                    sRet = rFtnEnd.GetPrefix() + sRet + rFtnEnd.GetSuffix();
                }
            }
        }

        if( bMakeNum )
        {
            const SwEndNoteInfo* pInfo;
            if( IsEndNote() )
                pInfo = &rDoc.GetEndNoteInfo();
            else
                pInfo = &rDoc.GetFtnInfo();
            sRet = pInfo->aFmt.GetNumStr( GetNumber() );
            if( bInclStrings )
            {
                sRet = pInfo->GetPrefix() + sRet + pInfo->GetSuffix();
            }
        }
    }
    return sRet;
}

/*************************************************************************
 *                      class SwTxt/FmtFnt
 *************************************************************************/

SwTxtFtn::SwTxtFtn( SwFmtFtn& rAttr, sal_Int32 nStartPos )
    : SwTxtAttr( rAttr, nStartPos )
    , m_pStartNode( 0 )
    , m_pTxtNode( 0 )
    , m_nSeqNo( USHRT_MAX )
{
    rAttr.m_pTxtAttr = this;
    SetHasDummyChar(true);
}


SwTxtFtn::~SwTxtFtn()
{
    SetStartNode( 0 );
}



void SwTxtFtn::SetStartNode( const SwNodeIndex *pNewNode, sal_Bool bDelNode )
{
    if( pNewNode )
    {
        if ( !m_pStartNode )
        {
            m_pStartNode = new SwNodeIndex( *pNewNode );
        }
        else
        {
            *m_pStartNode = *pNewNode;
        }
    }
    else if ( m_pStartNode )
    {
        
        
        
        SwDoc* pDoc;
        if ( m_pTxtNode )
        {
            pDoc = m_pTxtNode->GetDoc();
        }
        else
        {
            
            
            
            
            
            pDoc = m_pStartNode->GetNodes().GetDoc();
        }

        
        
        
        if( !pDoc->IsInDtor() )
        {
            if( bDelNode )
            {
                
                
                pDoc->DeleteSection( &m_pStartNode->GetNode() );
            }
            else
                
                
                
                DelFrms( 0 );
        }
        DELETEZ( m_pStartNode );

        
        for( sal_uInt16 n = 0; n < pDoc->GetFtnIdxs().size(); ++n )
            if( this == pDoc->GetFtnIdxs()[n] )
            {
                pDoc->GetFtnIdxs().erase( pDoc->GetFtnIdxs().begin() + n );
                
                if( !pDoc->IsInDtor() && n < pDoc->GetFtnIdxs().size() )
                {
                    SwNodeIndex aTmp( pDoc->GetFtnIdxs()[n]->GetTxtNode() );
                    pDoc->GetFtnIdxs().UpdateFtn( aTmp );
                }
                break;
            }
    }
}


void SwTxtFtn::SetNumber( const sal_uInt16 nNewNum, const OUString &sNumStr )
{
    SwFmtFtn& rFtn = (SwFmtFtn&)GetFtn();

    rFtn.m_aNumber = sNumStr;
    if ( sNumStr.isEmpty() )
    {
        rFtn.m_nNumber = nNewNum;
    }

    OSL_ENSURE( m_pTxtNode, "SwTxtFtn: where is my TxtNode?" );
    SwNodes &rNodes = m_pTxtNode->GetDoc()->GetNodes();
    m_pTxtNode->ModifyNotification( 0, &rFtn );
    if ( m_pStartNode )
    {
        
        SwNode* pNd;
        sal_uLong nSttIdx = m_pStartNode->GetIndex() + 1;
        sal_uLong nEndIdx = m_pStartNode->GetNode().EndOfSectionIndex();
        for( ; nSttIdx < nEndIdx; ++nSttIdx )
        {
            
            if( ( pNd = rNodes[ nSttIdx ] )->IsTxtNode() )
                ((SwTxtNode*)pNd)->ModifyNotification( 0, &rFtn );
        }
    }
}


void SwTxtFtn::CopyFtn(
    SwTxtFtn & rDest,
    SwTxtNode & rDestNode ) const
{
    if (m_pStartNode && !rDest.GetStartNode())
    {
        
        
        rDest.MakeNewTextSection( rDestNode.GetNodes() );
    }
    if (m_pStartNode && rDest.GetStartNode())
    {
        
        SwDoc *const pDstDoc = rDestNode.GetDoc();
        SwNodes &rDstNodes = pDstDoc->GetNodes();

        
        SwNodeRange aRg( *m_pStartNode, 1,
                    *m_pStartNode->GetNode().EndOfSectionNode() );

        
        
        SwNodeIndex aStart( *(rDest.GetStartNode()) );
        SwNodeIndex aEnd( *aStart.GetNode().EndOfSectionNode() );
        sal_uLong  nDestLen = aEnd.GetIndex() - aStart.GetIndex() - 1;

        m_pTxtNode->GetDoc()->CopyWithFlyInFly( aRg, 0, aEnd, NULL, sal_True );

        
        
        
        
        ++aStart;
        rDstNodes.Delete( aStart, nDestLen );
    }

    
    if( !GetFtn().m_aNumber.isEmpty() )
    {
        const_cast<SwFmtFtn &>(rDest.GetFtn()).m_aNumber = GetFtn().m_aNumber;
    }
}


    
void SwTxtFtn::MakeNewTextSection( SwNodes& rNodes )
{
    if ( m_pStartNode )
        return;

    
    SwTxtFmtColl *pFmtColl;
    const SwEndNoteInfo* pInfo;
    sal_uInt16 nPoolId;

    if( GetFtn().IsEndNote() )
    {
        pInfo = &rNodes.GetDoc()->GetEndNoteInfo();
        nPoolId = RES_POOLCOLL_ENDNOTE;
    }
    else
    {
        pInfo = &rNodes.GetDoc()->GetFtnInfo();
        nPoolId = RES_POOLCOLL_FOOTNOTE;
    }

    if( 0 == (pFmtColl = pInfo->GetFtnTxtColl() ) )
        pFmtColl = rNodes.GetDoc()->GetTxtCollFromPool( nPoolId );

    SwStartNode* pSttNd = rNodes.MakeTextSection( SwNodeIndex( rNodes.GetEndOfInserts() ),
                                        SwFootnoteStartNode, pFmtColl );
    m_pStartNode = new SwNodeIndex( *pSttNd );
}


void SwTxtFtn::DelFrms( const SwFrm* pSib )
{
    
    OSL_ENSURE( m_pTxtNode, "SwTxtFtn: where is my TxtNode?" );
    if ( !m_pTxtNode )
        return;

    const SwRootFrm* pRoot = pSib ? pSib->getRootFrm() : 0;
    bool bFrmFnd = false;
    {
        SwIterator<SwCntntFrm,SwTxtNode> aIter( *m_pTxtNode );
        for( SwCntntFrm* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
        {
            if( pRoot != pFnd->getRootFrm() && pRoot )
                continue;
            SwPageFrm* pPage = pFnd->FindPageFrm();
            if( pPage )
            {
                pPage->RemoveFtn( pFnd, this );
                bFrmFnd = true;
            }
        }
    }
    
    
    if ( !bFrmFnd && m_pStartNode )
    {
        SwNodeIndex aIdx( *m_pStartNode );
        SwCntntNode* pCNd = m_pTxtNode->GetNodes().GoNext( &aIdx );
        if( pCNd )
        {
            SwIterator<SwCntntFrm,SwCntntNode> aIter( *pCNd );
            for( SwCntntFrm* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
            {
                if( pRoot != pFnd->getRootFrm() && pRoot )
                    continue;
                SwPageFrm* pPage = pFnd->FindPageFrm();

                SwFrm *pFrm = pFnd->GetUpper();
                while ( pFrm && !pFrm->IsFtnFrm() )
                    pFrm = pFrm->GetUpper();

                SwFtnFrm *pFtn = (SwFtnFrm*)pFrm;
                while ( pFtn && pFtn->GetMaster() )
                    pFtn = pFtn->GetMaster();
                OSL_ENSURE( pFtn->GetAttr() == this, "Ftn mismatch error." );

                while ( pFtn )
                {
                    SwFtnFrm *pFoll = pFtn->GetFollow();
                    pFtn->Cut();
                    delete pFtn;
                    pFtn = pFoll;
                }

                
                
                if ( pPage )
                    pPage->UpdateFtnNum();
            }
        }
    }
}



sal_uInt16 SwTxtFtn::SetSeqRefNo()
{
    if( !m_pTxtNode )
        return USHRT_MAX;

    SwDoc* pDoc = m_pTxtNode->GetDoc();
    if( pDoc->IsInReading() )
        return USHRT_MAX;

    std::vector<SwTxtFtn*> badRefNums;
    std::set<sal_uInt16> aUsedNums = ::lcl_GetUsedFtnRefNumbers(*pDoc, this, badRefNums);
    if ( ::lcl_IsRefNumAvailable(aUsedNums, m_nSeqNo) )
        return m_nSeqNo;
    std::vector<sal_uInt16> unused = ::lcl_GetUnusedSeqRefNums(aUsedNums, 1);
    return m_nSeqNo = unused[0];
}



void SwTxtFtn::SetUniqueSeqRefNo( SwDoc& rDoc )
{
    std::vector<SwTxtFtn*> badRefNums;
    std::set<sal_uInt16> aUsedNums = ::lcl_GetUsedFtnRefNumbers(rDoc, NULL, badRefNums);
    std::vector<sal_uInt16> unused = ::lcl_GetUnusedSeqRefNums(aUsedNums, badRefNums.size());

    for (size_t i = 0; i < badRefNums.size(); ++i)
    {
        badRefNums[i]->m_nSeqNo = unused[i];
    }
}

void SwTxtFtn::CheckCondColl()
{

    if( GetStartNode() )
        ((SwStartNode&)GetStartNode()->GetNode()).CheckSectionCondColl();

}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
