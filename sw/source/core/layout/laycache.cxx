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

#include <editeng/formatbreakitem.hxx>
#include <sal/log.hxx>
#include <doc.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docstat.hxx>
#include <docary.hxx>
#include <fmtpdsc.hxx>
#include <laycache.hxx>
#include "layhelp.hxx"
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <swtable.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <sectfrm.hxx>
#include <fmtcntnt.hxx>
#include <pagedesc.hxx>
#include <frmtool.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <viewopt.hxx>
#include <flyfrm.hxx>
#include <sortedobjs.hxx>
#include <ndindex.hxx>
#include <node.hxx>

#include <limits>
#include <set>

using namespace ::com::sun::star;

SwLayoutCache::SwLayoutCache() : nLockCount( 0 ) {}

/*
 *  Reading and writing of the layout cache.
 *  The layout cache is not necessary, but it improves
 *  the performance and reduces the text flow during
 *  the formatting.
 *  The layout cache contains the index of the paragraphs/tables
 *  at the top of every page, so it's possible to create
 *  the right count of pages and to distribute the document content
 *  to this pages before the formatting starts.
 */

void SwLayoutCache::Read( SvStream &rStream )
{
    if( !pImpl )
    {
        pImpl.reset( new SwLayCacheImpl );
        if( !pImpl->Read( rStream ) )
        {
            pImpl.reset();
        }
    }
}

void SwLayCacheImpl::Insert( sal_uInt16 nType, sal_uLong nIndex, sal_Int32 nOffset )
{
    aType.push_back( nType );
    mIndices.push_back( nIndex );
    aOffset.push_back( nOffset );
}

bool SwLayCacheImpl::Read( SvStream& rStream )
{
    SwLayCacheIoImpl aIo( rStream, false );
    if( aIo.GetMajorVersion() > SW_LAYCACHE_IO_VERSION_MAJOR )
        return false;

    // Due to an evil bug in the layout cache (#102759#), we cannot trust the
    // sizes of fly frames which have been written using the "old" layout cache.
    // This flag should indicate that we do not want to trust the width and
    // height of fly frames
    bUseFlyCache = aIo.GetMinorVersion() >= 1;

    aIo.OpenRec( SW_LAYCACHE_IO_REC_PAGES );
    aIo.OpenFlagRec();
    aIo.CloseFlagRec();
    while( aIo.BytesLeft() && !aIo.HasError() )
    {
        sal_uInt32 nIndex(0), nOffset(0);

        switch( aIo.Peek() )
        {
        case SW_LAYCACHE_IO_REC_PARA:
        {
            aIo.OpenRec( SW_LAYCACHE_IO_REC_PARA );
            sal_uInt8 cFlags = aIo.OpenFlagRec();
            aIo.GetStream().ReadUInt32( nIndex );
            if( (cFlags & 0x01) != 0 )
                aIo.GetStream().ReadUInt32( nOffset );
            else
                nOffset = COMPLETE_STRING;
            aIo.CloseFlagRec();
            Insert( SW_LAYCACHE_IO_REC_PARA, nIndex, static_cast<sal_Int32>(nOffset) );
            aIo.CloseRec();
            break;
        }
        case SW_LAYCACHE_IO_REC_TABLE:
            aIo.OpenRec( SW_LAYCACHE_IO_REC_TABLE );
            aIo.OpenFlagRec();
            aIo.GetStream().ReadUInt32( nIndex )
                           .ReadUInt32( nOffset );
            Insert( SW_LAYCACHE_IO_REC_TABLE, nIndex, static_cast<sal_Int32>(nOffset) );
            aIo.CloseFlagRec();
            aIo.CloseRec();
            break;
        case SW_LAYCACHE_IO_REC_FLY:
        {
            aIo.OpenRec( SW_LAYCACHE_IO_REC_FLY );
            aIo.OpenFlagRec();
            aIo.CloseFlagRec();
            sal_Int32 nX(0), nY(0), nW(0), nH(0);
            sal_uInt16 nPgNum(0);
            aIo.GetStream().ReadUInt16( nPgNum ).ReadUInt32( nIndex )
                   .ReadInt32( nX ).ReadInt32( nY ).ReadInt32( nW ).ReadInt32( nH );
            m_FlyCache.emplace_back( nPgNum, nIndex, nX, nY, nW, nH );
            aIo.CloseRec();
            break;
        }
        default:
            aIo.SkipRec();
            break;
        }
    }
    aIo.CloseRec();

    return !aIo.HasError();
}

/** writes the index (more precise: the difference between
 * the index and the first index of the document content)
 * of the first paragraph/table at the top of every page.
 * If at the top of a page is the rest of a paragraph/table
 * from the bottom of the previous page, the character/row
 * number is stored, too.
 * The position, size and page number of the text frames
 * are stored, too
 */
void SwLayoutCache::Write( SvStream &rStream, const SwDoc& rDoc )
{
    if( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() ) // the layout itself ..
    {
        SwLayCacheIoImpl aIo( rStream, true );
        // We want to save the relative index, so we need the index
        // of the first content
        sal_uLong nStartOfContent = rDoc.GetNodes().GetEndOfContent().
                                StartOfSectionNode()->GetIndex();
        // The first page..
        SwPageFrame* pPage = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->Lower()));

        aIo.OpenRec( SW_LAYCACHE_IO_REC_PAGES );
        aIo.OpenFlagRec( 0, 0 );
        aIo.CloseFlagRec();
        while( pPage )
        {
            if( pPage->GetPrev() )
            {
                SwLayoutFrame* pLay = pPage->FindBodyCont();
                SwFrame* pTmp = pLay ? pLay->ContainsAny() : nullptr;
                // We are only interested in paragraph or table frames,
                // a section frames contains paragraphs/tables.
                if( pTmp && pTmp->IsSctFrame() )
                    pTmp = static_cast<SwSectionFrame*>(pTmp)->ContainsAny();

                if( pTmp ) // any content
                {
                    if( pTmp->IsTextFrame() )
                    {
                        SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(pTmp));
                        assert(!pFrame->GetMergedPara());
                        sal_uLong nNdIdx = pFrame->GetTextNodeFirst()->GetIndex();
                        if( nNdIdx > nStartOfContent )
                        {
                            /*  Open Paragraph Record */
                            aIo.OpenRec( SW_LAYCACHE_IO_REC_PARA );
                            bool bFollow = static_cast<SwTextFrame*>(pTmp)->IsFollow();
                            aIo.OpenFlagRec( bFollow ? 0x01 : 0x00,
                                            bFollow ? 8 : 4 );
                            nNdIdx -= nStartOfContent;
                            aIo.GetStream().WriteUInt32( nNdIdx );
                            if( bFollow )
                                aIo.GetStream().WriteUInt32( sal_Int32(static_cast<SwTextFrame*>(pTmp)->GetOfst()) );
                            aIo.CloseFlagRec();
                            /*  Close Paragraph Record */
                            aIo.CloseRec();
                        }
                    }
                    else if( pTmp->IsTabFrame() )
                    {
                        SwTabFrame* pTab = static_cast<SwTabFrame*>(pTmp);
                        sal_uLong nOfst = COMPLETE_STRING;
                        if( pTab->IsFollow() )
                        {
                            // If the table is a follow, we have to look for the
                            // master and to count all rows to get the row number
                            nOfst = 0;
                            if( pTab->IsFollow() )
                                pTab = pTab->FindMaster( true );
                            while( pTab != pTmp )
                            {
                                SwFrame* pSub = pTab->Lower();
                                while( pSub )
                                {
                                    ++nOfst;
                                    pSub = pSub->GetNext();
                                }
                                pTab = pTab->GetFollow();
                                assert(pTab && "Table follow without master");
                            }
                        }
                        while (true)
                        {
                            sal_uLong nNdIdx =
                                    pTab->GetTable()->GetTableNode()->GetIndex();
                            if( nNdIdx > nStartOfContent )
                            {
                                /* Open Table Record */
                                aIo.OpenRec( SW_LAYCACHE_IO_REC_TABLE );
                                aIo.OpenFlagRec( 0, 8 );
                                nNdIdx -= nStartOfContent;
                                aIo.GetStream().WriteUInt32( nNdIdx )
                                               .WriteUInt32( nOfst );
                                aIo.CloseFlagRec();
                                /* Close Table Record  */
                                aIo.CloseRec();
                            }
                            // If the table has a follow on the next page,
                            // we know already the row number and store this
                            // immediately.
                            if( pTab->GetFollow() )
                            {
                                if( nOfst == sal_uLong(COMPLETE_STRING) )
                                    nOfst = 0;
                                do
                                {
                                    SwFrame* pSub = pTab->Lower();
                                    while( pSub )
                                    {
                                        ++nOfst;
                                        pSub = pSub->GetNext();
                                    }
                                    pTab = pTab->GetFollow();
                                    SwPageFrame *pTabPage = pTab->FindPageFrame();
                                    if( pTabPage != pPage )
                                    {
                                        OSL_ENSURE( pPage->GetPhyPageNum() <
                                                pTabPage->GetPhyPageNum(),
                                                "Looping Tableframes" );
                                        pPage = pTabPage;
                                        break;
                                    }
                                } while ( pTab->GetFollow() );
                            }
                            else
                                break;
                        }
                    }
                }
            }
            if( pPage->GetSortedObjs() )
            {
                SwSortedObjs &rObjs = *pPage->GetSortedObjs();
                for (SwAnchoredObject* pAnchoredObj : rObjs)
                {
                    if (SwFlyFrame *pFly = dynamic_cast<SwFlyFrame*>(pAnchoredObj))
                    {
                        if( pFly->getFrameArea().Left() != FAR_AWAY &&
                            !pFly->GetAnchorFrame()->FindFooterOrHeader() )
                        {
                            const SwContact *pC =
                                    ::GetUserCall(pAnchoredObj->GetDrawObj());
                            if( pC )
                            {
                                sal_uInt32 nOrdNum = pAnchoredObj->GetDrawObj()->GetOrdNum();
                                sal_uInt16 nPageNum = pPage->GetPhyPageNum();
                                /* Open Fly Record */
                                aIo.OpenRec( SW_LAYCACHE_IO_REC_FLY );
                                aIo.OpenFlagRec( 0, 0 );
                                aIo.CloseFlagRec();
                                const SwRect& rRct = pFly->getFrameArea();
                                sal_Int32 nX = rRct.Left() - pPage->getFrameArea().Left();
                                sal_Int32 nY = rRct.Top() - pPage->getFrameArea().Top();
                                aIo.GetStream().WriteUInt16( nPageNum ).WriteUInt32( nOrdNum )
                                               .WriteInt32( nX ).WriteInt32( nY )
                                               .WriteInt32( rRct.Width() )
                                               .WriteInt32( rRct.Height() );
                                /* Close Fly Record  */
                                aIo.CloseRec();
                            }
                        }
                    }
                }
            }
            pPage = static_cast<SwPageFrame*>(pPage->GetNext());
        }
        aIo.CloseRec();
    }
}

#ifdef DBG_UTIL
bool SwLayoutCache::CompareLayout( const SwDoc& rDoc ) const
{
    if( !pImpl )
        return true;
    const SwRootFrame *pRootFrame = rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    if( pRootFrame )
    {
        size_t nIndex = 0;
        sal_uLong nStartOfContent = rDoc.GetNodes().GetEndOfContent().
                                StartOfSectionNode()->GetIndex();
        const SwPageFrame* pPage = static_cast<const SwPageFrame*>(pRootFrame->Lower());
        if( pPage )
            pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
        while( pPage )
        {
            if( nIndex >= pImpl->size() )
                return false;

            const SwLayoutFrame* pLay = pPage->FindBodyCont();
            const SwFrame* pTmp = pLay ? pLay->ContainsAny() : nullptr;
            if( pTmp && pTmp->IsSctFrame() )
                pTmp = static_cast<const SwSectionFrame*>(pTmp)->ContainsAny();
            if( pTmp )
            {
                if( pTmp->IsTextFrame() )
                {

                    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(pTmp));
                    assert(!pFrame->GetMergedPara());
                    sal_uLong nNdIdx = pFrame->GetTextNodeFirst()->GetIndex();
                    if( nNdIdx > nStartOfContent )
                    {
                        bool bFollow = static_cast<const SwTextFrame*>(pTmp)->IsFollow();
                        nNdIdx -= nStartOfContent;
                        if( pImpl->GetBreakIndex( nIndex ) != nNdIdx ||
                            SW_LAYCACHE_IO_REC_PARA !=
                            pImpl->GetBreakType( nIndex ) ||
                            (bFollow
                              ? sal_Int32(static_cast<const SwTextFrame*>(pTmp)->GetOfst())
                              : COMPLETE_STRING) != pImpl->GetBreakOfst(nIndex))
                        {
                            return false;
                        }
                        ++nIndex;
                    }
                }
                else if( pTmp->IsTabFrame() )
                {
                    const SwTabFrame* pTab = static_cast<const SwTabFrame*>(pTmp);
                    sal_Int32 nOfst = COMPLETE_STRING;
                    if( pTab->IsFollow() )
                    {
                        nOfst = 0;
                        if( pTab->IsFollow() )
                            pTab = pTab->FindMaster( true );
                        while( pTab != pTmp )
                        {
                            const SwFrame* pSub = pTab->Lower();
                            while( pSub )
                            {
                                ++nOfst;
                                pSub = pSub->GetNext();
                            }
                            pTab = pTab->GetFollow();
                        }
                    }
                    do
                    {
                        sal_uLong nNdIdx =
                                pTab->GetTable()->GetTableNode()->GetIndex();
                        if( nNdIdx > nStartOfContent )
                        {
                            nNdIdx -= nStartOfContent;
                            if( pImpl->GetBreakIndex( nIndex ) != nNdIdx ||
                                SW_LAYCACHE_IO_REC_TABLE !=
                                pImpl->GetBreakType( nIndex ) ||
                               nOfst != pImpl->GetBreakOfst( nIndex ) )
                            {
                                return false;
                            }
                            ++nIndex;
                        }
                        if( pTab->GetFollow() )
                        {
                            if( nOfst == COMPLETE_STRING )
                                nOfst = 0;
                            do
                            {
                                const SwFrame* pSub = pTab->Lower();
                                while( pSub )
                                {
                                    ++nOfst;
                                    pSub = pSub->GetNext();
                                }
                                pTab = pTab->GetFollow();
                                const SwPageFrame *pTabPage = pTab->FindPageFrame();
                                if( pTabPage != pPage )
                                {
                                    pPage = pTabPage;
                                    break;
                                }
                            } while ( pTab->GetFollow() );
                        }
                        else
                            break;
                    } while( pTab );
                }
            }
            pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
        }
    }
    return true;
}
#endif

void SwLayoutCache::ClearImpl()
{
    if( !IsLocked() )
    {
        pImpl.reset();
    }
}

SwLayoutCache::~SwLayoutCache()
{
    OSL_ENSURE( !nLockCount, "Deleting a locked SwLayoutCache!?" );
}

/// helper class to create not nested section frames for nested sections.
SwActualSection::SwActualSection( SwActualSection *pUp,
                                  SwSectionFrame    *pSect,
                                  SwSectionNode   *pNd ) :
    pUpper( pUp ),
    pSectFrame( pSect ),
    pSectNode( pNd )
{
    if ( !pSectNode )
    {
        const SwNodeIndex *pIndex = pSect->GetFormat()->GetContent().GetContentIdx();
        pSectNode = pIndex->GetNode().FindSectionNode();
    }
}

namespace {

bool sanityCheckLayoutCache(SwLayCacheImpl const& rCache,
        SwNodes const& rNodes, sal_uLong nNodeIndex)
{
    auto const nStartOfContent(rNodes.GetEndOfContent().StartOfSectionNode()->GetIndex());
    nNodeIndex -= nStartOfContent;
    auto const nMaxIndex(rNodes.GetEndOfContent().GetIndex() - nStartOfContent);
    for (size_t nIndex = 0; nIndex < rCache.size(); ++nIndex)
    {
        auto const nBreakIndex(rCache.GetBreakIndex(nIndex));
        if (nBreakIndex < nNodeIndex || nMaxIndex <= nBreakIndex)
        {
            SAL_WARN("sw.layout",
                "invalid node index in layout-cache: " << nBreakIndex);
            return false;
        }
        auto const nBreakType(rCache.GetBreakType(nIndex));
        switch (nBreakType)
        {
            case SW_LAYCACHE_IO_REC_PARA:
                if (!rNodes[nBreakIndex + nStartOfContent]->IsTextNode())
                {
                    SAL_WARN("sw.layout",
                        "invalid node of type 'P' in layout-cache");
                    return false;
                }
                break;
            case SW_LAYCACHE_IO_REC_TABLE:
                if (!rNodes[nBreakIndex + nStartOfContent]->IsTableNode())
                {
                    SAL_WARN("sw.layout",
                        "invalid node of type 'T' in layout-cache");
                    return false;
                }
                break;
            default:
                assert(false); // Read shouldn't have inserted that
        }
    }
    return true;
}

} // namespace

/** helper class, which utilizes the layout cache information
 *  to distribute the document content to the right pages.
 * It's used by the InsertCnt_(..)-function.
 * If there's no layout cache, the distribution to the pages is more
 * a guess, but a guess with statistical background.
 */
SwLayHelper::SwLayHelper( SwDoc *pD, SwFrame* &rpF, SwFrame* &rpP, SwPageFrame* &rpPg,
                          SwLayoutFrame* &rpL, std::unique_ptr<SwActualSection> &rpA,
                          sal_uLong nNodeIndex, bool bCache )
    : mrpFrame( rpF )
    , mrpPrv( rpP )
    , mrpPage( rpPg )
    , mrpLay( rpL )
    , mrpActualSection( rpA )
    , mbBreakAfter(false)
    , mpDoc(pD)
    , mnMaxParaPerPage( 25 )
    , mnParagraphCnt( bCache ? 0 : USHRT_MAX )
    , mnFlyIdx( 0 )
    , mbFirst( bCache )
{
    mpImpl = mpDoc->GetLayoutCache() ? mpDoc->GetLayoutCache()->LockImpl() : nullptr;
    if( mpImpl )
    {
        SwNodes const& rNodes(mpDoc->GetNodes());
        if (sanityCheckLayoutCache(*mpImpl, rNodes, nNodeIndex))
        {
            mnIndex = 0;
            mnStartOfContent = rNodes.GetEndOfContent().StartOfSectionNode()->GetIndex();
            mnMaxParaPerPage = 1000;
        }
        else
        {
            mpDoc->GetLayoutCache()->UnlockImpl();
            mpImpl = nullptr;
            mnIndex = std::numeric_limits<size_t>::max();
            mnStartOfContent = USHRT_MAX;
        }
    }
    else
    {
        mnIndex = std::numeric_limits<size_t>::max();
        mnStartOfContent = ULONG_MAX;
    }
}

SwLayHelper::~SwLayHelper()
{
    if( mpImpl )
    {
        OSL_ENSURE( mpDoc && mpDoc->GetLayoutCache(), "Missing layoutcache" );
        mpDoc->GetLayoutCache()->UnlockImpl();
    }
}

/** Does NOT really calculate the page count,
 * it returns the page count value from the layout cache, if available,
 * otherwise it estimates the page count.
 */
sal_uLong SwLayHelper::CalcPageCount()
{
    sal_uLong nPgCount;
    SwLayCacheImpl *pCache = mpDoc->GetLayoutCache() ?
                             mpDoc->GetLayoutCache()->LockImpl() : nullptr;
    if( pCache )
    {
        nPgCount = pCache->size() + 1;
        mpDoc->GetLayoutCache()->UnlockImpl();
    }
    else
    {
        nPgCount = mpDoc->getIDocumentStatistics().GetDocStat().nPage;
        if ( nPgCount <= 10 ) // no page insertion for less than 10 pages
            nPgCount = 0;
        sal_uLong nNdCount = mpDoc->getIDocumentStatistics().GetDocStat().nPara;
        if ( nNdCount <= 1 )
        {
            //Estimates the number of paragraphs.
            sal_uLong nTmp = mpDoc->GetNodes().GetEndOfContent().GetIndex() -
                        mpDoc->GetNodes().GetEndOfExtras().GetIndex();
            //Tables have a little overhead..
            nTmp -= mpDoc->GetTableFrameFormats()->size() * 25;
            //Fly frames, too ..
            nTmp -= (mpDoc->GetNodes().GetEndOfAutotext().GetIndex() -
                       mpDoc->GetNodes().GetEndOfInserts().GetIndex()) / 3 * 5;
            if ( nTmp > 0 )
                nNdCount = nTmp;
        }
        if ( nNdCount > 100 ) // no estimation below this value
        {
            if ( nPgCount > 0 )
                mnMaxParaPerPage = nNdCount / nPgCount;
            else
            {
                mnMaxParaPerPage = std::max( sal_uLong(20),
                                       sal_uLong(20 + nNdCount / 1000 * 3) );
                const sal_uLong nMax = 53;
                mnMaxParaPerPage = std::min( mnMaxParaPerPage, nMax );
                nPgCount = nNdCount / mnMaxParaPerPage;
            }
            if ( nNdCount < 1000 )
                nPgCount = 0;// no progress bar for small documents
            SwViewShell *pSh = nullptr;
            if( mrpLay && mrpLay->getRootFrame() )
                pSh = mrpLay->getRootFrame()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                mnMaxParaPerPage *= 6;
        }
    }
    return nPgCount;
}

/**
 * inserts a page and return true, if
 * - the break after flag is set
 * - the actual content wants a break before
 * - the maximum count of paragraph/rows is reached
 *
 * The break after flag is set, if the actual content
 * wants a break after.
 */
bool SwLayHelper::CheckInsertPage()
{
    bool bEnd = nullptr == mrpPage->GetNext();
    const SvxFormatBreakItem& rBrk = mrpFrame->GetBreakItem();
    const SwFormatPageDesc& rDesc = mrpFrame->GetPageDescItem();
    // #118195# Do not evaluate page description if frame
    // is a follow frame!
    const SwPageDesc* pDesc = mrpFrame->IsFlowFrame() &&
                              SwFlowFrame::CastFlowFrame( mrpFrame )->IsFollow() ?
                              nullptr :
                              rDesc.GetPageDesc();

    bool bBrk = mnParagraphCnt > mnMaxParaPerPage || mbBreakAfter;
    mbBreakAfter = rBrk.GetBreak() == SvxBreak::PageAfter ||
                   rBrk.GetBreak() == SvxBreak::PageBoth;
    if ( !bBrk )
        bBrk = rBrk.GetBreak() == SvxBreak::PageBefore ||
               rBrk.GetBreak() == SvxBreak::PageBoth;

    if ( bBrk || pDesc )
    {
        ::boost::optional<sal_uInt16> oPgNum;
        if ( !pDesc )
        {
            pDesc = mrpPage->GetPageDesc()->GetFollow();

            SwFormatPageDesc aFollowDesc( pDesc );
            oPgNum = aFollowDesc.GetNumOffset();
            if ( oPgNum )
                static_cast<SwRootFrame*>(mrpPage->GetUpper())->SetVirtPageNum(true);
        }
        else
        {
            oPgNum = rDesc.GetNumOffset();
            if ( oPgNum )
                static_cast<SwRootFrame*>(mrpPage->GetUpper())->SetVirtPageNum(true);
        }
        bool bNextPageOdd = !mrpPage->OnRightPage();
        bool bInsertEmpty = false;
        if( oPgNum && bNextPageOdd != ( ( oPgNum.get() % 2 ) != 0 ) )
        {
            bNextPageOdd = !bNextPageOdd;
            bInsertEmpty = true;
        }
        // If the page style is changing, we'll have a first page.
        bool bNextPageFirst = pDesc != mrpPage->GetPageDesc();
        ::InsertNewPage( const_cast<SwPageDesc&>(*pDesc), mrpPage->GetUpper(),
                         bNextPageOdd, bNextPageFirst, bInsertEmpty, false, mrpPage->GetNext() );
        if ( bEnd )
        {
            OSL_ENSURE( mrpPage->GetNext(), "No new page?" );
            do
            {   mrpPage = static_cast<SwPageFrame*>(mrpPage->GetNext());
            } while ( mrpPage->GetNext() );
        }
        else
        {
            OSL_ENSURE( mrpPage->GetNext(), "No new page?" );
            mrpPage = static_cast<SwPageFrame*>(mrpPage->GetNext());
            if ( mrpPage->IsEmptyPage() )
            {
                OSL_ENSURE( mrpPage->GetNext(), "No new page?" );
                mrpPage = static_cast<SwPageFrame*>(mrpPage->GetNext());
            }
        }
        mrpLay = mrpPage->FindBodyCont();
        while( mrpLay->Lower() )
            mrpLay = static_cast<SwLayoutFrame*>(mrpLay->Lower());
        return true;
    }
    return false;
}

/** entry point for the InsertCnt_-function.
 *  The document content index is checked either it is
 *  in the layout cache either it's time to insert a page
 *  cause the maximal estimation of content per page is reached.
 *  A really big table or long paragraph may contains more than
 *  one page, in this case the needed count of pages will inserted.
 */
bool SwLayHelper::CheckInsert( sal_uLong nNodeIndex )
{
    bool bRet = false;
    bool bLongTab = false;
    sal_uLong nMaxRowPerPage( 0 );
    nNodeIndex -= mnStartOfContent;
    sal_uInt16 nRows( 0 );
    if( mrpFrame->IsTabFrame() )
    {
        //Inside a table counts every row as a paragraph
        SwFrame *pLow = static_cast<SwTabFrame*>(mrpFrame)->Lower();
        nRows = 0;
        do
        {
            ++nRows;
            pLow = pLow->GetNext();
        } while ( pLow );
        mnParagraphCnt += nRows;
        if( !mpImpl && mnParagraphCnt > mnMaxParaPerPage + 10 )
        {
            // OD 09.04.2003 #108698# - improve heuristics:
            // Assume that a table, which has more than three times the quantity
            // of maximal paragraphs per page rows, consists of rows, which have
            // the height of a normal paragraph. Thus, allow as much rows per page
            // as much paragraphs are allowed.
            if ( nRows > ( 3*mnMaxParaPerPage ) )
            {
                nMaxRowPerPage = mnMaxParaPerPage;
            }
            else
            {
                SwFrame *pTmp = static_cast<SwTabFrame*>(mrpFrame)->Lower();
                if( pTmp->GetNext() )
                    pTmp = pTmp->GetNext();
                pTmp = static_cast<SwRowFrame*>(pTmp)->Lower();
                sal_uInt16 nCnt = 0;
                do
                {
                    ++nCnt;
                    pTmp = pTmp->GetNext();
                } while( pTmp );
                nMaxRowPerPage = std::max( sal_uLong(2), mnMaxParaPerPage / nCnt );
            }
            bLongTab = true;
        }
    }
    else
        ++mnParagraphCnt;
    if( mbFirst && mpImpl && mnIndex < mpImpl->size() &&
        mpImpl->GetBreakIndex( mnIndex ) == nNodeIndex &&
        ( mpImpl->GetBreakOfst( mnIndex ) < COMPLETE_STRING ||
          ( ++mnIndex < mpImpl->size() &&
          mpImpl->GetBreakIndex( mnIndex ) == nNodeIndex ) ) )
        mbFirst = false;
    // OD 09.04.2003 #108698# - always split a big tables.
    if ( !mbFirst ||
         ( mrpFrame->IsTabFrame() && bLongTab )
       )
    {
        sal_Int32 nRowCount = 0;
        do
        {
            if( mpImpl || bLongTab )
            {
                sal_Int32 nOfst = COMPLETE_STRING;
                sal_uInt16 nType = SW_LAYCACHE_IO_REC_PAGES;
                if( bLongTab )
                {
                    mbBreakAfter = true;
                    nOfst = static_cast<sal_Int32>(nRowCount + nMaxRowPerPage);
                }
                else
                {
                    while( mnIndex < mpImpl->size() &&
                           mpImpl->GetBreakIndex(mnIndex) < nNodeIndex)
                        ++mnIndex;
                    if( mnIndex < mpImpl->size() &&
                        mpImpl->GetBreakIndex(mnIndex) == nNodeIndex )
                    {
                        nType = mpImpl->GetBreakType( mnIndex );
                        nOfst = mpImpl->GetBreakOfst( mnIndex++ );
                        mbBreakAfter = true;
                    }
                }

                if( nOfst < COMPLETE_STRING )
                {
                    bool bSplit = false;
                    sal_uInt16 nRepeat( 0 );
                    if( !bLongTab && mrpFrame->IsTextFrame() &&
                        SW_LAYCACHE_IO_REC_PARA == nType &&
                        nOfst < static_cast<SwTextFrame*>(mrpFrame)->GetText().getLength())
                        bSplit = true;
                    else if( mrpFrame->IsTabFrame() && nRowCount < nOfst &&
                             ( bLongTab || SW_LAYCACHE_IO_REC_TABLE == nType ) )
                    {
                        nRepeat = static_cast<SwTabFrame*>(mrpFrame)->
                                  GetTable()->GetRowsToRepeat();
                        bSplit = nOfst < nRows && nRowCount + nRepeat < nOfst;
                        bLongTab = bLongTab && bSplit;
                    }
                    if( bSplit )
                    {
                        mrpFrame->InsertBehind( mrpLay, mrpPrv );

                        {
                            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*mrpFrame);
                            aFrm.Pos() = mrpLay->getFrameArea().Pos();
                            aFrm.Pos().AdjustY(1 );
                        }

                        mrpPrv = mrpFrame;
                        if( mrpFrame->IsTabFrame() )
                        {
                            SwTabFrame* pTab = static_cast<SwTabFrame*>(mrpFrame);
                            // #i33629#, #i29955#
                            ::RegistFlys( pTab->FindPageFrame(), pTab );
                            SwFrame *pRow = pTab->Lower();
                            SwTabFrame *pFoll = new SwTabFrame( *pTab );

                            SwFrame *pPrv;
                            if( nRepeat > 0 )
                            {
                                bDontCreateObjects = true; //frmtool

                                // Insert new headlines:
                                sal_uInt16 nRowIdx = 0;
                                SwRowFrame* pHeadline = nullptr;
                                while( nRowIdx < nRepeat )
                                {
                                    OSL_ENSURE( pTab->GetTable()->GetTabLines()[ nRowIdx ], "Table without rows?" );
                                    pHeadline =
                                        new SwRowFrame( *pTab->GetTable()->GetTabLines()[ nRowIdx ], pTab );
                                    pHeadline->SetRepeatedHeadline( true );
                                    pHeadline->InsertBefore( pFoll, nullptr );
                                    pHeadline->RegistFlys();

                                    ++nRowIdx;
                                }

                                bDontCreateObjects = false;
                                pPrv = pHeadline;
                                nRows = nRows + nRepeat;
                            }
                            else
                                pPrv = nullptr;
                            while( pRow && nRowCount < nOfst )
                            {
                                pRow = pRow->GetNext();
                                ++nRowCount;
                            }
                            while ( pRow )
                            {
                                SwFrame* pNxt = pRow->GetNext();
                                pRow->RemoveFromLayout();
                                pRow->InsertBehind( pFoll, pPrv );
                                pPrv = pRow;
                                pRow = pNxt;
                            }
                            mrpFrame = pFoll;
                        }
                        else
                        {
                            SwTextFrame *const pNew = static_cast<SwTextFrame*>(
                                static_cast<SwTextFrame*>(mrpFrame)
                                    ->GetTextNodeFirst()->MakeFrame(mrpFrame));
                            pNew->ManipOfst( TextFrameIndex(nOfst) );
                            pNew->SetFollow( static_cast<SwTextFrame*>(mrpFrame)->GetFollow() );
                            static_cast<SwTextFrame*>(mrpFrame)->SetFollow( pNew );
                            mrpFrame = pNew;
                        }
                    }
                }
            }

            SwPageFrame* pLastPage = mrpPage;
            if( CheckInsertPage() )
            {
                CheckFlyCache_( pLastPage );
                if( mrpPrv && mrpPrv->IsTextFrame() && !mrpPrv->isFrameAreaSizeValid() )
                {
                    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*mrpPrv);
                    aFrm.Height( mrpPrv->GetUpper()->getFramePrintArea().Height() );
                }

                bRet = true;
                mrpPrv = nullptr;
                mnParagraphCnt = 0;

                if ( mrpActualSection )
                {
                    //Did the SectionFrame even have a content? If not, we can
                    //directly put it somewhere else
                    SwSectionFrame *pSct;
                    bool bInit = false;
                    if ( !mrpActualSection->GetSectionFrame()->ContainsContent())
                    {
                        pSct = mrpActualSection->GetSectionFrame();
                        pSct->RemoveFromLayout();
                    }
                    else
                    {
                        pSct = new SwSectionFrame(
                            *mrpActualSection->GetSectionFrame(), false );
                        mrpActualSection->GetSectionFrame()->SimpleFormat();
                        bInit = true;
                    }
                    mrpActualSection->SetSectionFrame( pSct );
                    pSct->InsertBehind( mrpLay, nullptr );

                    if( bInit )
                    {
                        pSct->Init();
                    }

                    {
                        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pSct);
                        aFrm.Pos() = mrpLay->getFrameArea().Pos();
                        aFrm.Pos().AdjustY(1 ); //because of the notifications
                    }

                    mrpLay = pSct;
                    if ( mrpLay->Lower() && mrpLay->Lower()->IsLayoutFrame() )
                        mrpLay = mrpLay->GetNextLayoutLeaf();
                }
            }
        } while( bLongTab || ( mpImpl && mnIndex < mpImpl->size() &&
                 mpImpl->GetBreakIndex( mnIndex ) == nNodeIndex ) );
    }
    mbFirst = false;
    return bRet;
}

struct SdrObjectCompare
{
  bool operator()( const SdrObject* pF1, const SdrObject* pF2 ) const
  {
    return pF1->GetOrdNum() < pF2->GetOrdNum();
  }
};

struct FlyCacheCompare
{
  bool operator()( const SwFlyCache* pC1, const SwFlyCache* pC2 ) const
  {
    return pC1->nOrdNum < pC2->nOrdNum;
  }
};

/**
 * If a new page is inserted, the last page is analysed.
 * If there are text frames with default position, the fly cache
 * is checked, if these frames are stored in the cache.
 */
void SwLayHelper::CheckFlyCache_( SwPageFrame* pPage )
{
    if( !mpImpl || !pPage )
        return;
    const size_t nFlyCount = mpImpl->GetFlyCount();
    // Any text frames at the page, fly cache available?
    if( pPage->GetSortedObjs() && mnFlyIdx < nFlyCount )
    {
        SwSortedObjs &rObjs = *pPage->GetSortedObjs();
        sal_uInt16 nPgNum = pPage->GetPhyPageNum();

        // NOTE: Here we do not use the absolute ordnums but
        // relative ordnums for the objects on this page.

        // skip fly frames from pages before the current page
        while( mnFlyIdx < nFlyCount &&
               mpImpl->GetFlyCache(mnFlyIdx).nPageNum < nPgNum )
            ++mnFlyIdx;

        // sort cached objects on this page by ordnum
        std::set< const SwFlyCache*, FlyCacheCompare > aFlyCacheSet;
        size_t nIdx = mnFlyIdx;

        SwFlyCache* pFlyC;
        while( nIdx < nFlyCount &&
               ( pFlyC = &mpImpl->GetFlyCache( nIdx ) )->nPageNum == nPgNum )
        {
            aFlyCacheSet.insert( pFlyC );
            ++nIdx;
        }

        // sort objects on this page by ordnum
        std::set< const SdrObject*, SdrObjectCompare > aFlySet;
        for (SwAnchoredObject* pAnchoredObj : rObjs)
        {
            if (SwFlyFrame *pFly = dynamic_cast<SwFlyFrame*>(pAnchoredObj))  // a text frame?
            {
                if( pFly->GetAnchorFrame() &&
                    !pFly->GetAnchorFrame()->FindFooterOrHeader() )
                {
                    const SwContact *pC = ::GetUserCall( pAnchoredObj->GetDrawObj() );
                    if( pC )
                    {
                        aFlySet.insert( pAnchoredObj->GetDrawObj() );
                    }
                }
            }
        }

        if ( aFlyCacheSet.size() == aFlySet.size() )
        {
            std::set< const SdrObject*, SdrObjectCompare >::iterator aFlySetIt =
                    aFlySet.begin();

            for ( const SwFlyCache* pFlyCache : aFlyCacheSet )
            {
                SwFlyFrame* pFly = const_cast<SwVirtFlyDrawObj*>(static_cast<const SwVirtFlyDrawObj*>(*aFlySetIt))->GetFlyFrame();

                if ( pFly->getFrameArea().Left() == FAR_AWAY )
                {
                    // we get the stored information
                    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pFly);
                    aFrm.Pos().setX( pFlyCache->Left() + pPage->getFrameArea().Left() );
                    aFrm.Pos().setY( pFlyCache->Top() + pPage->getFrameArea().Top() );

                    if ( mpImpl->IsUseFlyCache() )
                    {
                        aFrm.Width( pFlyCache->Width() );
                        aFrm.Height( pFlyCache->Height() );
                    }
                }

                ++aFlySetIt;
            }
        }
    }
}

SwLayCacheIoImpl::SwLayCacheIoImpl( SvStream& rStrm, bool bWrtMd ) :
    pStream( &rStrm ),
    nFlagRecEnd ( 0 ),
    nMajorVersion(SW_LAYCACHE_IO_VERSION_MAJOR),
    nMinorVersion(SW_LAYCACHE_IO_VERSION_MINOR),
    bWriteMode( bWrtMd ),
    bError( false  )
{
    if( bWriteMode )
        pStream->WriteUInt16( nMajorVersion )
                .WriteUInt16( nMinorVersion );

    else
        pStream->ReadUInt16( nMajorVersion )
                .ReadUInt16( nMinorVersion );
}

void SwLayCacheIoImpl::OpenRec( sal_uInt8 cType )
{
    sal_uInt32 nPos = pStream->Tell();
    if( bWriteMode )
    {
        aRecords.emplace_back(cType, nPos );
        pStream->WriteUInt32( 0 );
    }
    else
    {
        sal_uInt32 nVal(0);
        pStream->ReadUInt32( nVal );
        sal_uInt8 cRecTyp = static_cast<sal_uInt8>(nVal);
        if (!nVal || cRecTyp != cType || !pStream->good())
        {
            OSL_ENSURE( nVal, "OpenRec: Record-Header is 0" );
            OSL_ENSURE( cRecTyp == cType, "OpenRec: Wrong Record Type" );
            aRecords.emplace_back(0, pStream->Tell() );
            bError = true;
        }
        else
        {
            sal_uInt32 nSize = nVal >> 8;
            aRecords.emplace_back(cRecTyp, nPos+nSize );
        }
    }
}

// Close record
void SwLayCacheIoImpl::CloseRec()
{
    bool bRes = true;
    OSL_ENSURE( !aRecords.empty(), "CloseRec: no levels" );
    if( !aRecords.empty() )
    {
        sal_uInt32 nPos = pStream->Tell();
        if( bWriteMode )
        {
            sal_uInt32 nBgn = aRecords.back().size;
            pStream->Seek( nBgn );
            sal_uInt32 nSize = nPos - nBgn;
            sal_uInt32 nVal = ( nSize << 8 ) | aRecords.back().type;
            pStream->WriteUInt32( nVal );
            pStream->Seek( nPos );
            if( pStream->GetError() != ERRCODE_NONE )
                 bRes = false;
        }
        else
        {
            sal_uInt32 n = aRecords.back().size;
            OSL_ENSURE( n >= nPos, "CloseRec: to much data read" );
            if( n != nPos )
            {
                pStream->Seek( n );
                if( n < nPos )
                     bRes = false;
            }
            if( pStream->GetErrorCode() != ERRCODE_NONE )
                bRes = false;
        }
        aRecords.pop_back();
    }

    if( !bRes )
        bError = true;
}

sal_uInt32 SwLayCacheIoImpl::BytesLeft()
{
    sal_uInt32 n = 0;
    if( !bError && !aRecords.empty() )
    {
        sal_uInt32 nEndPos = aRecords.back().size;
        sal_uInt32 nPos = pStream->Tell();
        if( nEndPos > nPos )
            n = nEndPos - nPos;
    }
    return n;
}

sal_uInt8 SwLayCacheIoImpl::Peek()
{
    sal_uInt8 c(0);
    if( !bError )
    {
        sal_uInt32 nPos = pStream->Tell();
        pStream->ReadUChar( c );
        pStream->Seek( nPos );
        if( pStream->GetErrorCode() != ERRCODE_NONE )
        {
            c = 0;
            bError = true;
        }
    }
    return c;
}

void SwLayCacheIoImpl::SkipRec()
{
    sal_uInt8 c = Peek();
    OpenRec( c );
    pStream->Seek( aRecords.back().size );
    CloseRec();
}

sal_uInt8 SwLayCacheIoImpl::OpenFlagRec()
{
    OSL_ENSURE( !bWriteMode, "OpenFlagRec illegal in write  mode" );
    sal_uInt8 cFlags(0);
    pStream->ReadUChar( cFlags );
    nFlagRecEnd = pStream->Tell() + ( cFlags & 0x0F );
    return (cFlags >> 4);
}

void SwLayCacheIoImpl::OpenFlagRec( sal_uInt8 nFlags, sal_uInt8 nLen )
{
    OSL_ENSURE( bWriteMode, "OpenFlagRec illegal in read  mode" );
    OSL_ENSURE( (nFlags & 0xF0) == 0, "illegal flags set" );
    OSL_ENSURE( nLen < 16, "wrong flag record length" );
    sal_uInt8 cFlags = (nFlags << 4) + nLen;
    pStream->WriteUChar( cFlags );
    nFlagRecEnd = pStream->Tell() + nLen;
}

void SwLayCacheIoImpl::CloseFlagRec()
{
    if( bWriteMode )
    {
        OSL_ENSURE( pStream->Tell() == nFlagRecEnd, "Wrong amount of data written" );
    }
    else
    {
        OSL_ENSURE( pStream->Tell() <= nFlagRecEnd, "Too many data read" );
        if( pStream->Tell() != nFlagRecEnd )
            pStream->Seek( nFlagRecEnd );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
