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

#include <fmtftn.hxx>

#include <doc.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
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
#include <calbck.hxx>
#include <hints.hxx>

namespace {
    /// Get a sorted list of the used footnote reference numbers.
    /// @param[in]  rDoc     The active document.
    /// @param[in]  pExclude A footnote whose reference number should be excluded from the set.
    /// @param[out] rUsedRef The set of used reference numbers.
    /// @param[out] rInvalid  A returned list of all items that had an invalid reference number.
    void lcl_FillUsedFootnoteRefNumbers(SwDoc &rDoc,
                                         SwTextFootnote const *pExclude,
                                         std::set<sal_uInt16> &rUsedRef,
                                         std::vector<SwTextFootnote*> &rInvalid)
    {
        SwFootnoteIdxs& ftnIdxs = rDoc.GetFootnoteIdxs();

        rInvalid.clear();

        for( size_t n = 0; n < ftnIdxs.size(); ++n )
        {
            SwTextFootnote* pTextFootnote = ftnIdxs[ n ];
            if ( pTextFootnote != pExclude )
            {
                if ( USHRT_MAX == pTextFootnote->GetSeqRefNo() )
                {
                    rInvalid.push_back(pTextFootnote);
                }
                else
                {
                    rUsedRef.insert( pTextFootnote->GetSeqRefNo() );
                }
            }
        }
    }

    /// Check whether a requested reference number is available.
    /// @param[in] rUsedNums Set of used reference numbers.
    /// @param[in] requested The requested reference number.
    /// @returns true if the number is available, false if not.
    bool lcl_IsRefNumAvailable(std::set<sal_uInt16> const &rUsedNums,
                                         sal_uInt16 requested)
    {
        if ( USHRT_MAX == requested )
            return false;  // Invalid sequence number.
        if ( rUsedNums.count(requested) )
            return false;  // Number already used.
        return true;
    }

    /// Get the first few unused sequential reference numbers.
    /// @param[out] rLowestUnusedNums The lowest unused sequential reference numbers.
    /// @param[in] rUsedNums   The set of used sequential reference numbers.
    /// @param[in] numRequired The number of reference number required.
    void lcl_FillUnusedSeqRefNums(std::vector<sal_uInt16> &rLowestUnusedNums,
                                         const std::set<sal_uInt16> &rUsedNums,
                                         size_t numRequired)
    {
        if (!numRequired)
            return;

        rLowestUnusedNums.reserve(numRequired);
        sal_uInt16 newNum = 0;
        std::set<sal_uInt16>::iterator it;
        //Start by using numbers from gaps in rUsedNums
        for( it = rUsedNums.begin(); it != rUsedNums.end(); ++it )
        {
            while ( newNum < *it )
            {
                rLowestUnusedNums.push_back( newNum++ );
                if ( --numRequired == 0)
                    return;
            }
            newNum++;
        }
        //Filled in all gaps. Fill the rest of the list with new numbers.
        do
        {
            rLowestUnusedNums.push_back( newNum++ );
        }
        while ( --numRequired > 0 );
    }

}

SwFormatFootnote::SwFormatFootnote( bool bEndNote )
    : SfxPoolItem( RES_TXTATR_FTN )
    , SwModify(nullptr)
    , m_pTextAttr(nullptr)
    , m_nNumber(0)
    , m_nNumberRLHidden(0)
    , m_bEndNote(bEndNote)
{
}

bool SwFormatFootnote::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return m_nNumber  == static_cast<const SwFormatFootnote&>(rAttr).m_nNumber &&
        //FIXME?
           m_aNumber  == static_cast<const SwFormatFootnote&>(rAttr).m_aNumber &&
           m_bEndNote == static_cast<const SwFormatFootnote&>(rAttr).m_bEndNote;
}

SfxPoolItem* SwFormatFootnote::Clone( SfxItemPool* ) const
{
    SwFormatFootnote* pNew  = new SwFormatFootnote;
    pNew->m_aNumber = m_aNumber;
    pNew->m_nNumber = m_nNumber;
    pNew->m_nNumberRLHidden = m_nNumberRLHidden;
    pNew->m_bEndNote = m_bEndNote;
    return pNew;
}

void SwFormatFootnote::Modify(SfxPoolItem const* pOld, SfxPoolItem const* pNew)
{
    NotifyClients(pOld, pNew);
    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached UNO object
        SetXFootnote(css::uno::Reference<css::text::XFootnote>(nullptr));
    }
}

void SwFormatFootnote::InvalidateFootnote()
{
    SwPtrMsgPoolItem const item(RES_REMOVE_UNO_OBJECT,
            &static_cast<SwModify&>(*this)); // cast to base class (void*)
    NotifyClients(&item, &item);
}

void SwFormatFootnote::SetEndNote( bool b )
{
    if ( b != m_bEndNote )
    {
        if ( GetTextFootnote() )
        {
            GetTextFootnote()->DelFrames(nullptr);
        }
        m_bEndNote = b;
    }
}

SwFormatFootnote::~SwFormatFootnote()
{
}

OUString SwFormatFootnote::GetFootnoteText(SwRootFrame const& rLayout) const
{
    OUStringBuffer buf;
    if( m_pTextAttr->GetStartNode() )
    {
        SwNodeIndex aIdx( *m_pTextAttr->GetStartNode(), 1 );
        SwContentNode* pCNd = aIdx.GetNode().GetTextNode();
        if( !pCNd )
            pCNd = aIdx.GetNodes().GoNext( &aIdx );

        if( pCNd->IsTextNode() ) {
            buf.append(static_cast<SwTextNode*>(pCNd)->GetExpandText(&rLayout));

            ++aIdx;
            while ( !aIdx.GetNode().IsEndNode() ) {
                if ( aIdx.GetNode().IsTextNode() )
                {
                    buf.append("  ");
                    buf.append(aIdx.GetNode().GetTextNode()->GetExpandText(&rLayout));
                }
                ++aIdx;
            }
        }
    }
    return buf.makeStringAndClear();
}

/// return the view string of the foot/endnote
OUString SwFormatFootnote::GetViewNumStr(const SwDoc& rDoc,
        SwRootFrame const*const pLayout, bool bInclStrings) const
{
    OUString sRet( GetNumStr() );
    if( sRet.isEmpty() )
    {
        // in this case the number is needed, get it via SwDoc's FootnoteInfo
        bool bMakeNum = true;
        const SwSectionNode* pSectNd = m_pTextAttr
                    ? SwUpdFootnoteEndNtAtEnd::FindSectNdWithEndAttr( *m_pTextAttr )
                    : nullptr;
        sal_uInt16 const nNumber(pLayout && pLayout->IsHideRedlines()
                ? GetNumberRLHidden()
                : GetNumber());

        if( pSectNd )
        {
            const SwFormatFootnoteEndAtTextEnd& rFootnoteEnd = static_cast<const SwFormatFootnoteEndAtTextEnd&>(
                pSectNd->GetSection().GetFormat()->GetFormatAttr(
                                IsEndNote() ?
                                static_cast<sal_uInt16>(RES_END_AT_TXTEND) :
                                static_cast<sal_uInt16>(RES_FTN_AT_TXTEND) ) );

            if( FTNEND_ATTXTEND_OWNNUMANDFMT == rFootnoteEnd.GetValue() )
            {
                bMakeNum = false;
                sRet = rFootnoteEnd.GetSwNumType().GetNumStr( nNumber );
                if( bInclStrings )
                {
                    sRet = rFootnoteEnd.GetPrefix() + sRet + rFootnoteEnd.GetSuffix();
                }
            }
        }

        if( bMakeNum )
        {
            const SwEndNoteInfo* pInfo;
            if( IsEndNote() )
                pInfo = &rDoc.GetEndNoteInfo();
            else
                pInfo = &rDoc.GetFootnoteInfo();
            sRet = pInfo->aFormat.GetNumStr( nNumber );
            if( bInclStrings )
            {
                sRet = pInfo->GetPrefix() + sRet + pInfo->GetSuffix();
            }
        }
    }
    return sRet;
}

SwTextFootnote::SwTextFootnote( SwFormatFootnote& rAttr, sal_Int32 nStartPos )
    : SwTextAttr( rAttr, nStartPos )
    , m_pTextNode( nullptr )
    , m_nSeqNo( USHRT_MAX )
{
    rAttr.m_pTextAttr = this;
    SetHasDummyChar(true);
}

SwTextFootnote::~SwTextFootnote()
{
    SetStartNode( nullptr );
}

void SwTextFootnote::SetStartNode( const SwNodeIndex *pNewNode, bool bDelNode )
{
    if( pNewNode )
    {
        if ( !m_pStartNode )
        {
            m_pStartNode.reset(new SwNodeIndex(*pNewNode));
        }
        else
        {
            *m_pStartNode = *pNewNode;
        }
    }
    else if ( m_pStartNode )
    {
        // need to do 2 things:
        // 1) unregister footnotes at their pages
        // 2) delete the footnote section in the Inserts of the nodes-array
        SwDoc* pDoc;
        if ( m_pTextNode )
        {
            pDoc = m_pTextNode->GetDoc();
        }
        else
        {
            //JP 27.01.97: the sw3-Reader creates a StartNode but the
            //             attribute isn't anchored in the TextNode yet.
            //             If it is deleted (e.g. Insert File with footnote
            //             inside fly frame), the content must also be deleted.
            pDoc = m_pStartNode->GetNodes().GetDoc();
        }

        // If called from ~SwDoc(), must not delete the footnote nodes,
        // and not necessary to delete the footnote frames.
        if( !pDoc->IsInDtor() )
        {
            if( bDelNode )
            {
                // 2) delete the section for the footnote nodes
                // it's possible that the Inserts have already been deleted (how???)
                pDoc->getIDocumentContentOperations().DeleteSection( &m_pStartNode->GetNode() );
            }
            else
                // If the nodes are not deleted, their frames must be removed
                // from the page (deleted), there is nothing else that deletes
                // them (particularly not Undo)
                DelFrames( nullptr );
        }
        m_pStartNode.reset();

        // remove the footnote from the SwDoc's array
        for( size_t n = 0; n < pDoc->GetFootnoteIdxs().size(); ++n )
            if( this == pDoc->GetFootnoteIdxs()[n] )
            {
                pDoc->GetFootnoteIdxs().erase( pDoc->GetFootnoteIdxs().begin() + n );
                // if necessary, update following footnotes
                if( !pDoc->IsInDtor() && n < pDoc->GetFootnoteIdxs().size() )
                {
                    SwNodeIndex aTmp( pDoc->GetFootnoteIdxs()[n]->GetTextNode() );
                    pDoc->GetFootnoteIdxs().UpdateFootnote( aTmp );
                }
                break;
            }
    }
}

void SwTextFootnote::SetNumber(const sal_uInt16 nNewNum,
        sal_uInt16 const nNumberRLHidden, const OUString &sNumStr)
{
    SwFormatFootnote& rFootnote = const_cast<SwFormatFootnote&>(GetFootnote());

    rFootnote.m_aNumber = sNumStr;
    if ( sNumStr.isEmpty() )
    {
        rFootnote.m_nNumber = nNewNum;
        rFootnote.m_nNumberRLHidden = nNumberRLHidden;
    }
    InvalidateNumberInLayout();
}

void SwTextFootnote::InvalidateNumberInLayout()
{
    assert(m_pTextNode);
    SwFormatFootnote const& rFootnote(GetFootnote());
    SwNodes &rNodes = m_pTextNode->GetDoc()->GetNodes();
    m_pTextNode->ModifyNotification( nullptr, &rFootnote );
    if ( m_pStartNode )
    {
        // must iterate over all TextNodes because of footnotes on other pages
        sal_uLong nSttIdx = m_pStartNode->GetIndex() + 1;
        sal_uLong nEndIdx = m_pStartNode->GetNode().EndOfSectionIndex();
        for( ; nSttIdx < nEndIdx; ++nSttIdx )
        {
            SwNode* pNd;
            if( ( pNd = rNodes[ nSttIdx ] )->IsTextNode() )
                static_cast<SwTextNode*>(pNd)->ModifyNotification( nullptr, &rFootnote );
        }
    }
}

void SwTextFootnote::CopyFootnote(
    SwTextFootnote & rDest,
    SwTextNode & rDestNode ) const
{
    if (m_pStartNode && !rDest.GetStartNode())
    {
        // dest missing node section? create it here!
        // (happens in SwTextNode::CopyText if pDest == this)
        rDest.MakeNewTextSection( rDestNode.GetNodes() );
    }
    if (m_pStartNode && rDest.GetStartNode())
    {
        // footnotes not necessarily in same document!
        SwDoc *const pDstDoc = rDestNode.GetDoc();
        SwNodes &rDstNodes = pDstDoc->GetNodes();

        // copy only the content of the section
        SwNodeRange aRg( *m_pStartNode, 1,
                    *m_pStartNode->GetNode().EndOfSectionNode() );

        // insert at the end of rDest, i.e., the nodes are appended.
        // nDestLen contains number of ContentNodes in rDest _before_ copy.
        SwNodeIndex aStart( *(rDest.GetStartNode()) );
        SwNodeIndex aEnd( *aStart.GetNode().EndOfSectionNode() );
        sal_uLong  nDestLen = aEnd.GetIndex() - aStart.GetIndex() - 1;

        m_pTextNode->GetDoc()->GetDocumentContentOperationsManager().CopyWithFlyInFly( aRg, 0, aEnd );

        // in case the destination section was not empty, delete the old nodes
        // before:   Src: SxxxE,  Dst: SnE
        // now:      Src: SxxxE,  Dst: SnxxxE
        // after:    Src: SxxxE,  Dst: SxxxE
        ++aStart;
        rDstNodes.Delete( aStart, nDestLen );
    }

    // also copy user defined number string
    if( !GetFootnote().m_aNumber.isEmpty() )
    {
        const_cast<SwFormatFootnote &>(rDest.GetFootnote()).m_aNumber = GetFootnote().m_aNumber;
    }
}

/// create a new nodes-array section for the footnote
void SwTextFootnote::MakeNewTextSection( SwNodes& rNodes )
{
    if ( m_pStartNode )
        return;

    // set the footnote style on the SwTextNode
    SwTextFormatColl *pFormatColl;
    const SwEndNoteInfo* pInfo;
    sal_uInt16 nPoolId;

    if( GetFootnote().IsEndNote() )
    {
        pInfo = &rNodes.GetDoc()->GetEndNoteInfo();
        nPoolId = RES_POOLCOLL_ENDNOTE;
    }
    else
    {
        pInfo = &rNodes.GetDoc()->GetFootnoteInfo();
        nPoolId = RES_POOLCOLL_FOOTNOTE;
    }

    if( nullptr == (pFormatColl = pInfo->GetFootnoteTextColl() ) )
        pFormatColl = rNodes.GetDoc()->getIDocumentStylePoolAccess().GetTextCollFromPool( nPoolId );

    SwStartNode* pSttNd = rNodes.MakeTextSection( SwNodeIndex( rNodes.GetEndOfInserts() ),
                                        SwFootnoteStartNode, pFormatColl );
    m_pStartNode.reset(new SwNodeIndex(*pSttNd));
}

void SwTextFootnote::DelFrames(SwRootFrame const*const pRoot)
{
    // delete the FootnoteFrames from the pages
    OSL_ENSURE( m_pTextNode, "SwTextFootnote: where is my TextNode?" );
    if ( !m_pTextNode )
        return;

    bool bFrameFnd = false;
    {
        SwIterator<SwContentFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*m_pTextNode);
        for( SwContentFrame* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
        {
            if( pRoot != pFnd->getRootFrame() && pRoot )
                continue;
            SwPageFrame* pPage = pFnd->FindPageFrame();
            if( pPage )
            {
                // note: we have found the correct frame only if the footnote
                // was actually removed; in case this is called from
                // SwTextFrame::DestroyImpl(), then that frame isn't connected
                // to SwPageFrame any more, and RemoveFootnote on any follow
                // must not prevent the fall-back to the !bFrameFnd code.
                bFrameFnd = pPage->RemoveFootnote(pFnd, this);
            }
        }
    }
    //JP 13.05.97: if the layout is deleted before the footnotes are deleted,
    //             try to delete the footnote's frames by another way
    if ( !bFrameFnd && m_pStartNode )
    {
        SwNodeIndex aIdx( *m_pStartNode );
        SwContentNode* pCNd = m_pTextNode->GetNodes().GoNext( &aIdx );
        if( pCNd )
        {
            SwIterator<SwContentFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aIter(*pCNd);
            for( SwContentFrame* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
            {
                if( pRoot != pFnd->getRootFrame() && pRoot )
                    continue;
                SwPageFrame* pPage = pFnd->FindPageFrame();

                SwFrame *pFrame = pFnd->GetUpper();
                while ( pFrame && !pFrame->IsFootnoteFrame() )
                    pFrame = pFrame->GetUpper();

                SwFootnoteFrame *pFootnote = static_cast<SwFootnoteFrame*>(pFrame);
                while ( pFootnote && pFootnote->GetMaster() )
                    pFootnote = pFootnote->GetMaster();
                OSL_ENSURE( pFootnote->GetAttr() == this, "Footnote mismatch error." );

                while ( pFootnote )
                {
                    SwFootnoteFrame *pFoll = pFootnote->GetFollow();
                    pFootnote->Cut();
                    SwFrame::DestroyFrame(pFootnote);
                    pFootnote = pFoll;
                }

                // #i20556# During hiding of a section, the connection
                // to the layout is already lost. pPage may be 0:
                if ( pPage )
                    pPage->UpdateFootnoteNum();
            }
        }
    }
}

/// Set the sequence number for the current footnote.
/// @returns The new sequence number or USHRT_MAX if invalid.
void SwTextFootnote::SetSeqRefNo()
{
    if( !m_pTextNode )
        return;

    SwDoc* pDoc = m_pTextNode->GetDoc();
    if( pDoc->IsInReading() )
        return;

    std::set<sal_uInt16> aUsedNums;
    std::vector<SwTextFootnote*> badRefNums;
    ::lcl_FillUsedFootnoteRefNumbers(*pDoc, this, aUsedNums, badRefNums);
    if ( ::lcl_IsRefNumAvailable(aUsedNums, m_nSeqNo) )
        return;
    std::vector<sal_uInt16> unused;
    ::lcl_FillUnusedSeqRefNums(unused, aUsedNums, 1);
    m_nSeqNo = unused[0];
}

/// Set a unique sequential reference number for every footnote in the document.
/// @param[in] rDoc The document to be processed.
void SwTextFootnote::SetUniqueSeqRefNo( SwDoc& rDoc )
{
    std::set<sal_uInt16> aUsedNums;
    std::vector<SwTextFootnote*> badRefNums;
    ::lcl_FillUsedFootnoteRefNumbers(rDoc, nullptr, aUsedNums, badRefNums);
    std::vector<sal_uInt16> aUnused;
    ::lcl_FillUnusedSeqRefNums(aUnused, aUsedNums, badRefNums.size());

    for (size_t i = 0; i < badRefNums.size(); ++i)
    {
        badRefNums[i]->m_nSeqNo = aUnused[i];
    }
}

void SwTextFootnote::CheckCondColl()
{
//FEATURE::CONDCOLL
    if( GetStartNode() )
        static_cast<SwStartNode&>(GetStartNode()->GetNode()).CheckSectionCondColl();
//FEATURE::CONDCOLL
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
