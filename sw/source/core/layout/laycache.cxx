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
#include <doc.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docstat.hxx>
#include <docary.hxx>
#include <fmtpdsc.hxx>
#include <laycache.hxx>
#include <layhelp.hxx>
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
#include "viewopt.hxx"
#include <flyfrm.hxx>
#include <sortedobjs.hxx>

#include <set>

using namespace ::com::sun::star;

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
        pImpl = new SwLayCacheImpl;
        if( !pImpl->Read( rStream ) )
        {
            delete pImpl;
            pImpl = nullptr;
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
            Insert( SW_LAYCACHE_IO_REC_PARA, nIndex, (sal_Int32)nOffset );
            aIo.CloseRec( SW_LAYCACHE_IO_REC_PARA );
            break;
        }
        case SW_LAYCACHE_IO_REC_TABLE:
            aIo.OpenRec( SW_LAYCACHE_IO_REC_TABLE );
            aIo.OpenFlagRec();
            aIo.GetStream().ReadUInt32( nIndex )
                           .ReadUInt32( nOffset );
            Insert( SW_LAYCACHE_IO_REC_TABLE, nIndex, (sal_Int32)nOffset );
            aIo.CloseFlagRec();
            aIo.CloseRec( SW_LAYCACHE_IO_REC_TABLE );
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
            m_FlyCache.push_back(SwFlyCache( nPgNum, nIndex, nX, nY, nW, nH ));
            aIo.CloseRec( SW_LAYCACHE_IO_REC_FLY );
            break;
        }
        default:
            aIo.SkipRec();
            break;
        }
    }
    aIo.CloseRec( SW_LAYCACHE_IO_REC_PAGES );

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
        SwPageFrm* pPage = const_cast<SwPageFrm*>(static_cast<const SwPageFrm*>(rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->Lower()));

        aIo.OpenRec( SW_LAYCACHE_IO_REC_PAGES );
        aIo.OpenFlagRec( 0, 0 );
        aIo.CloseFlagRec();
        while( pPage )
        {
            if( pPage->GetPrev() )
            {
                SwLayoutFrm* pLay = pPage->FindBodyCont();
                SwFrm* pTmp = pLay ? pLay->ContainsAny() : nullptr;
                // We are only interested in paragraph or table frames,
                // a section frames contains paragraphs/tables.
                if( pTmp && pTmp->IsSctFrm() )
                    pTmp = static_cast<SwSectionFrm*>(pTmp)->ContainsAny();

                if( pTmp ) // any content
                {
                    if( pTmp->IsTextFrm() )
                    {
                        sal_uLong nNdIdx = static_cast<SwTextFrm*>(pTmp)->GetNode()->GetIndex();
                        if( nNdIdx > nStartOfContent )
                        {
                            /*  Open Paragraph Record */
                            aIo.OpenRec( SW_LAYCACHE_IO_REC_PARA );
                            bool bFollow = static_cast<SwTextFrm*>(pTmp)->IsFollow();
                            aIo.OpenFlagRec( bFollow ? 0x01 : 0x00,
                                            bFollow ? 8 : 4 );
                            nNdIdx -= nStartOfContent;
                            aIo.GetStream().WriteUInt32( nNdIdx );
                            if( bFollow )
                                aIo.GetStream().WriteUInt32( static_cast<SwTextFrm*>(pTmp)->GetOfst() );
                            aIo.CloseFlagRec();
                            /*  Close Paragraph Record */
                            aIo.CloseRec( SW_LAYCACHE_IO_REC_PARA );
                        }
                    }
                    else if( pTmp->IsTabFrm() )
                    {
                        SwTabFrm* pTab = static_cast<SwTabFrm*>(pTmp);
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
                                SwFrm* pSub = pTab->Lower();
                                while( pSub )
                                {
                                    ++nOfst;
                                    pSub = pSub->GetNext();
                                }
                                pTab = pTab->GetFollow();
                                OSL_ENSURE( pTab, "Table follow without master" );
                            }
                        }
                        do
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
                                aIo.CloseRec( SW_LAYCACHE_IO_REC_TABLE );
                            }
                            // If the table has a follow on the next page,
                            // we know already the row number and store this
                            // immediately.
                            if( pTab->GetFollow() )
                            {
                                if( nOfst == (sal_uLong) COMPLETE_STRING )
                                    nOfst = 0;
                                do
                                {
                                    SwFrm* pSub = pTab->Lower();
                                    while( pSub )
                                    {
                                        ++nOfst;
                                        pSub = pSub->GetNext();
                                    }
                                    pTab = pTab->GetFollow();
                                    SwPageFrm *pTabPage = pTab->FindPageFrm();
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
                        } while( pTab );
                    }
                }
            }
            if( pPage->GetSortedObjs() )
            {
                SwSortedObjs &rObjs = *pPage->GetSortedObjs();
                for ( size_t i = 0; i < rObjs.size(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = rObjs[i];
                    if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
                    {
                        SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                        if( pFly->Frm().Left() != FAR_AWAY &&
                            !pFly->GetAnchorFrm()->FindFooterOrHeader() )
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
                                SwRect &rRct = pFly->Frm();
                                sal_Int32 nX = rRct.Left() - pPage->Frm().Left();
                                sal_Int32 nY = rRct.Top() - pPage->Frm().Top();
                                aIo.GetStream().WriteUInt16( nPageNum ).WriteUInt32( nOrdNum )
                                               .WriteInt32( nX ).WriteInt32( nY )
                                               .WriteInt32( rRct.Width() )
                                               .WriteInt32( rRct.Height() );
                                /* Close Fly Record  */
                                aIo.CloseRec( SW_LAYCACHE_IO_REC_FLY );
                            }
                        }
                    }
                }
            }
            pPage = static_cast<SwPageFrm*>(pPage->GetNext());
        }
        aIo.CloseRec( SW_LAYCACHE_IO_REC_PAGES );
    }
}

#ifdef DBG_UTIL
bool SwLayoutCache::CompareLayout( const SwDoc& rDoc ) const
{
    if( !pImpl )
        return true;
    const SwRootFrm *pRootFrm = rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    if( pRootFrm )
    {
        sal_uInt16 nIndex = 0;
        sal_uLong nStartOfContent = rDoc.GetNodes().GetEndOfContent().
                                StartOfSectionNode()->GetIndex();
        const SwPageFrm* pPage = static_cast<const SwPageFrm*>(pRootFrm->Lower());
        if( pPage )
            pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
        while( pPage )
        {
            if( nIndex >= pImpl->size() )
                return false;

            const SwLayoutFrm* pLay = pPage->FindBodyCont();
            const SwFrm* pTmp = pLay ? pLay->ContainsAny() : nullptr;
            if( pTmp && pTmp->IsSctFrm() )
                pTmp = static_cast<const SwSectionFrm*>(pTmp)->ContainsAny();
            if( pTmp )
            {
                if( pTmp->IsTextFrm() )
                {
                    sal_uLong nNdIdx = static_cast<const SwTextFrm*>(pTmp)->GetNode()->GetIndex();
                    if( nNdIdx > nStartOfContent )
                    {
                        bool bFollow = static_cast<const SwTextFrm*>(pTmp)->IsFollow();
                        nNdIdx -= nStartOfContent;
                        if( pImpl->GetBreakIndex( nIndex ) != nNdIdx ||
                            SW_LAYCACHE_IO_REC_PARA !=
                            pImpl->GetBreakType( nIndex ) ||
                            ( bFollow ? static_cast<const SwTextFrm*>(pTmp)->GetOfst()
                              : COMPLETE_STRING ) != pImpl->GetBreakOfst( nIndex ) )
                        {
                            return false;
                        }
                        ++nIndex;
                    }
                }
                else if( pTmp->IsTabFrm() )
                {
                    const SwTabFrm* pTab = static_cast<const SwTabFrm*>(pTmp);
                    sal_Int32 nOfst = COMPLETE_STRING;
                    if( pTab->IsFollow() )
                    {
                        nOfst = 0;
                        if( pTab->IsFollow() )
                            pTab = pTab->FindMaster( true );
                        while( pTab != pTmp )
                        {
                            const SwFrm* pSub = pTab->Lower();
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
                                const SwFrm* pSub = pTab->Lower();
                                while( pSub )
                                {
                                    ++nOfst;
                                    pSub = pSub->GetNext();
                                }
                                pTab = pTab->GetFollow();
                                const SwPageFrm *pTabPage = pTab->FindPageFrm();
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
            pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
        }
    }
    return true;
}
#endif

void SwLayoutCache::ClearImpl()
{
    if( !IsLocked() )
    {
        delete pImpl;
        pImpl = nullptr;
    }
}

SwLayoutCache::~SwLayoutCache()
{
    OSL_ENSURE( !nLockCount, "Deleting a locked SwLayoutCache!?" );
    delete pImpl;
}

/// helper class to create not nested section frames for nested sections.
SwActualSection::SwActualSection( SwActualSection *pUp,
                                  SwSectionFrm    *pSect,
                                  SwSectionNode   *pNd ) :
    pUpper( pUp ),
    pSectFrm( pSect ),
    pSectNode( pNd )
{
    if ( !pSectNode )
    {
        const SwNodeIndex *pIndex = pSect->GetFormat()->GetContent().GetContentIdx();
        pSectNode = pIndex->GetNode().FindSectionNode();
    }
}

/** helper class, which utilizes the layout cache information
 *  to distribute the document content to the right pages.
 * It's used by the _InsertCnt(..)-function.
 * If there's no layout cache, the distibution to the pages is more
 * a guess, but a guess with statistical background.
 */
SwLayHelper::SwLayHelper( SwDoc *pD, SwFrm* &rpF, SwFrm* &rpP, SwPageFrm* &rpPg,
                          SwLayoutFrm* &rpL, SwActualSection* &rpA, bool &rB,
                          sal_uLong nNodeIndex, bool bCache )
    : rpFrm( rpF )
    , rpPrv( rpP )
    , rpPage( rpPg )
    , rpLay( rpL )
    , rpActualSection( rpA )
    , rbBreakAfter(rB)
    , pDoc(pD)
    , nMaxParaPerPage( 25 )
    , nParagraphCnt( bCache ? 0 : USHRT_MAX )
    , nFlyIdx( 0 )
    , bFirst( bCache )
{
    pImpl = pDoc->GetLayoutCache() ? pDoc->GetLayoutCache()->LockImpl() : nullptr;
    if( pImpl )
    {
        nMaxParaPerPage = 1000;
        nStartOfContent = pDoc->GetNodes().GetEndOfContent().StartOfSectionNode()
                          ->GetIndex();
        nNodeIndex -= nStartOfContent;
        nIndex = 0;
        while( nIndex < pImpl->size() && pImpl->GetBreakIndex( nIndex ) < nNodeIndex )
        {
            ++nIndex;
        }
        if( nIndex >= pImpl->size() )
        {
            pDoc->GetLayoutCache()->UnlockImpl();
            pImpl = nullptr;
        }
    }
    else
    {
        nIndex = USHRT_MAX;
        nStartOfContent = ULONG_MAX;
    }
}

SwLayHelper::~SwLayHelper()
{
    if( pImpl )
    {
        OSL_ENSURE( pDoc && pDoc->GetLayoutCache(), "Missing layoutcache" );
        pDoc->GetLayoutCache()->UnlockImpl();
    }
}

/** Does NOT really calculate the page count,
 * it returns the page count value from the layout cache, if available,
 * otherwise it estimates the page count.
 */
sal_uLong SwLayHelper::CalcPageCount()
{
    sal_uLong nPgCount;
    SwLayCacheImpl *pCache = pDoc->GetLayoutCache() ?
                             pDoc->GetLayoutCache()->LockImpl() : nullptr;
    if( pCache )
    {
        nPgCount = pCache->size() + 1;
        pDoc->GetLayoutCache()->UnlockImpl();
    }
    else
    {
        nPgCount = pDoc->getIDocumentStatistics().GetDocStat().nPage;
        if ( nPgCount <= 10 ) // no page insertion for less than 10 pages
            nPgCount = 0;
        sal_uLong nNdCount = pDoc->getIDocumentStatistics().GetDocStat().nPara;
        if ( nNdCount <= 1 )
        {
            //Estimates the number of paragraphs.
            sal_uLong nTmp = pDoc->GetNodes().GetEndOfContent().GetIndex() -
                        pDoc->GetNodes().GetEndOfExtras().GetIndex();
            //Tables have a little overhead..
            nTmp -= pDoc->GetTableFrameFormats()->size() * 25;
            //Fly frames, too ..
            nTmp -= (pDoc->GetNodes().GetEndOfAutotext().GetIndex() -
                       pDoc->GetNodes().GetEndOfInserts().GetIndex()) / 3 * 5;
            if ( nTmp > 0 )
                nNdCount = nTmp;
        }
        if ( nNdCount > 100 ) // no estimation below this value
        {
            if ( nPgCount > 0 )
                nMaxParaPerPage = nNdCount / nPgCount;
            else
            {
                nMaxParaPerPage = std::max( sal_uLong(20),
                                       sal_uLong(20 + nNdCount / 1000 * 3) );
                const sal_uLong nMax = 53;
                nMaxParaPerPage = std::min( nMaxParaPerPage, nMax );
                nPgCount = nNdCount / nMaxParaPerPage;
            }
            if ( nNdCount < 1000 )
                nPgCount = 0;// no progress bar for small documents
            SwViewShell *pSh = nullptr;
            if( rpLay && rpLay->getRootFrm() )
                pSh = rpLay->getRootFrm()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                nMaxParaPerPage *= 6;
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
    bool bEnd = nullptr == rpPage->GetNext();
    const SwAttrSet* pAttr = rpFrm->GetAttrSet();
    const SvxFormatBreakItem& rBrk = pAttr->GetBreak();
    const SwFormatPageDesc& rDesc = pAttr->GetPageDesc();
    // #118195# Do not evaluate page description if frame
    // is a follow frame!
    const SwPageDesc* pDesc = rpFrm->IsFlowFrm() &&
                              SwFlowFrm::CastFlowFrm( rpFrm )->IsFollow() ?
                              nullptr :
                              rDesc.GetPageDesc();

    bool bBrk = nParagraphCnt > nMaxParaPerPage || rbBreakAfter;
    rbBreakAfter = rBrk.GetBreak() == SVX_BREAK_PAGE_AFTER ||
                   rBrk.GetBreak() == SVX_BREAK_PAGE_BOTH;
    if ( !bBrk )
        bBrk = rBrk.GetBreak() == SVX_BREAK_PAGE_BEFORE ||
               rBrk.GetBreak() == SVX_BREAK_PAGE_BOTH;

    if ( bBrk || pDesc )
    {
        ::boost::optional<sal_uInt16> oPgNum;
        if ( !pDesc )
        {
            pDesc = rpPage->GetPageDesc()->GetFollow();

            SwFormatPageDesc aFollowDesc( pDesc );
            oPgNum = aFollowDesc.GetNumOffset();
            if ( oPgNum )
                static_cast<SwRootFrm*>(rpPage->GetUpper())->SetVirtPageNum(true);
        }
        else
        {
            oPgNum = rDesc.GetNumOffset();
            if ( oPgNum )
                static_cast<SwRootFrm*>(rpPage->GetUpper())->SetVirtPageNum(true);
        }
        bool bNextPageOdd = !rpPage->OnRightPage();
        bool bInsertEmpty = false;
        if( oPgNum && bNextPageOdd != ( ( oPgNum.get() % 2 ) != 0 ) )
        {
            bNextPageOdd = !bNextPageOdd;
            bInsertEmpty = true;
        }
        // If the page style is changing, we'll have a first page.
        bool bNextPageFirst = pDesc != rpPage->GetPageDesc();
        ::InsertNewPage( (SwPageDesc&)*pDesc, rpPage->GetUpper(),
                         bNextPageOdd, bNextPageFirst, bInsertEmpty, false, rpPage->GetNext() );
        if ( bEnd )
        {
            OSL_ENSURE( rpPage->GetNext(), "No new page?" );
            do
            {   rpPage = static_cast<SwPageFrm*>(rpPage->GetNext());
            } while ( rpPage->GetNext() );
        }
        else
        {
            OSL_ENSURE( rpPage->GetNext(), "No new page?" );
            rpPage = static_cast<SwPageFrm*>(rpPage->GetNext());
            if ( rpPage->IsEmptyPage() )
            {
                OSL_ENSURE( rpPage->GetNext(), "No new page?" );
                rpPage = static_cast<SwPageFrm*>(rpPage->GetNext());
            }
        }
        rpLay = rpPage->FindBodyCont();
        while( rpLay->Lower() )
            rpLay = static_cast<SwLayoutFrm*>(rpLay->Lower());
        return true;
    }
    return false;
}

/** entry point for the _InsertCnt-function.
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
    nNodeIndex -= nStartOfContent;
    sal_uInt16 nRows( 0 );
    if( rpFrm->IsTabFrm() )
    {
        //Inside a table counts every row as a paragraph
        SwFrm *pLow = static_cast<SwTabFrm*>(rpFrm)->Lower();
        nRows = 0;
        do
        {
            ++nRows;
            pLow = pLow->GetNext();
        } while ( pLow );
        nParagraphCnt += nRows;
        if( !pImpl && nParagraphCnt > nMaxParaPerPage + 10 )
        {
            // OD 09.04.2003 #108698# - improve heuristics:
            // Assume that a table, which has more than three times the quantity
            // of maximal paragraphs per page rows, consists of rows, which have
            // the height of a normal paragraph. Thus, allow as much rows per page
            // as much paragraphs are allowed.
            if ( nRows > ( 3*nMaxParaPerPage ) )
            {
                nMaxRowPerPage = nMaxParaPerPage;
            }
            else
            {
                SwFrm *pTmp = static_cast<SwTabFrm*>(rpFrm)->Lower();
                if( pTmp->GetNext() )
                    pTmp = pTmp->GetNext();
                pTmp = static_cast<SwRowFrm*>(pTmp)->Lower();
                sal_uInt16 nCnt = 0;
                do
                {
                    ++nCnt;
                    pTmp = pTmp->GetNext();
                } while( pTmp );
                nMaxRowPerPage = std::max( sal_uLong(2), nMaxParaPerPage / nCnt );
            }
            bLongTab = true;
        }
    }
    else
        ++nParagraphCnt;
    if( bFirst && pImpl && nIndex < pImpl->size() &&
        pImpl->GetBreakIndex( nIndex ) == nNodeIndex &&
        ( pImpl->GetBreakOfst( nIndex ) < COMPLETE_STRING ||
          ( ++nIndex < pImpl->size() &&
          pImpl->GetBreakIndex( nIndex ) == nNodeIndex ) ) )
        bFirst = false;
#if OSL_DEBUG_LEVEL > 1
    sal_uLong nBreakIndex = ( pImpl && nIndex < pImpl->size() ) ?
                        pImpl->GetBreakIndex(nIndex) : 0xffff;
    (void)nBreakIndex;
#endif
    // OD 09.04.2003 #108698# - always split a big tables.
    if ( !bFirst ||
         ( rpFrm->IsTabFrm() && bLongTab )
       )
    {
        sal_Int32 nRowCount = 0;
        do
        {
            if( pImpl || bLongTab )
            {
#if OSL_DEBUG_LEVEL > 1
                sal_uLong nBrkIndex = ( pImpl && nIndex < pImpl->size() ) ?
                        pImpl->GetBreakIndex(nIndex) : 0xffff;
                (void)nBrkIndex;
#endif
                sal_Int32 nOfst = COMPLETE_STRING;
                sal_uInt16 nType = SW_LAYCACHE_IO_REC_PAGES;
                if( bLongTab )
                {
                    rbBreakAfter = true;
                    nOfst = static_cast<sal_Int32>(nRowCount + nMaxRowPerPage);
                }
                else
                {
                    while( nIndex < pImpl->size() &&
                           pImpl->GetBreakIndex(nIndex) < nNodeIndex)
                        ++nIndex;
                    if( nIndex < pImpl->size() &&
                        pImpl->GetBreakIndex(nIndex) == nNodeIndex )
                    {
                        nType = pImpl->GetBreakType( nIndex );
                        nOfst = pImpl->GetBreakOfst( nIndex++ );
                        rbBreakAfter = true;
                    }
                }

                if( nOfst < COMPLETE_STRING )
                {
                    bool bSplit = false;
                    sal_uInt16 nRepeat( 0 );
                    if( !bLongTab && rpFrm->IsTextFrm() &&
                        SW_LAYCACHE_IO_REC_PARA == nType &&
                        nOfst < static_cast<SwTextFrm*>(rpFrm)->GetTextNode()->GetText().getLength())
                        bSplit = true;
                    else if( rpFrm->IsTabFrm() && nRowCount < nOfst &&
                             ( bLongTab || SW_LAYCACHE_IO_REC_TABLE == nType ) )
                    {
                        nRepeat = static_cast<SwTabFrm*>(rpFrm)->
                                  GetTable()->GetRowsToRepeat();
                        bSplit = nOfst < nRows && nRowCount + nRepeat < nOfst;
                        bLongTab = bLongTab && bSplit;
                    }
                    if( bSplit )
                    {
                        rpFrm->InsertBehind( rpLay, rpPrv );
                        rpFrm->Frm().Pos() = rpLay->Frm().Pos();
                        rpFrm->Frm().Pos().Y() += 1;
                        rpPrv = rpFrm;
                        if( rpFrm->IsTabFrm() )
                        {
                            SwTabFrm* pTab = static_cast<SwTabFrm*>(rpFrm);
                            // #i33629#, #i29955#
                            ::RegistFlys( pTab->FindPageFrm(), pTab );
                            SwFrm *pRow = pTab->Lower();
                            SwTabFrm *pFoll = new SwTabFrm( *pTab );

                            SwFrm *pPrv;
                            if( nRepeat > 0 )
                            {
                                bDontCreateObjects = true; //frmtool

                                // Insert new headlines:
                                sal_uInt16 nRowIdx = 0;
                                SwRowFrm* pHeadline = nullptr;
                                while( nRowIdx < nRepeat )
                                {
                                    OSL_ENSURE( pTab->GetTable()->GetTabLines()[ nRowIdx ], "Table ohne Zeilen?" );
                                    pHeadline =
                                        new SwRowFrm( *pTab->GetTable()->GetTabLines()[ nRowIdx ], pTab );
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
                                SwFrm* pNxt = pRow->GetNext();
                                pRow->RemoveFromLayout();
                                pRow->InsertBehind( pFoll, pPrv );
                                pPrv = pRow;
                                pRow = pNxt;
                            }
                            rpFrm = pFoll;
                        }
                        else
                        {
                            SwTextFrm *const pNew = static_cast<SwTextFrm*>(
                                static_cast<SwTextFrm*>(rpFrm)
                                    ->GetTextNode()->MakeFrm(rpFrm));
                            pNew->ManipOfst( nOfst );
                            pNew->SetFollow( static_cast<SwTextFrm*>(rpFrm)->GetFollow() );
                            static_cast<SwTextFrm*>(rpFrm)->SetFollow( pNew );
                            rpFrm = pNew;
                        }
                    }
                }
            }

            SwPageFrm* pLastPage = rpPage;
            if( CheckInsertPage() )
            {
                _CheckFlyCache( pLastPage );
                if( rpPrv && rpPrv->IsTextFrm() && !rpPrv->GetValidSizeFlag() )
                    rpPrv->Frm().Height( rpPrv->GetUpper()->Prt().Height() );

                bRet = true;
                rpPrv = nullptr;
                nParagraphCnt = 0;

                if ( rpActualSection )
                {
                    //Did the SectionFrm even have a content? If not, we can
                    //directly put it somewhere else
                    SwSectionFrm *pSct;
                    bool bInit = false;
                    if ( !rpActualSection->GetSectionFrm()->ContainsContent())
                    {
                        pSct = rpActualSection->GetSectionFrm();
                        pSct->RemoveFromLayout();
                    }
                    else
                    {
                        pSct = new SwSectionFrm(
                            *rpActualSection->GetSectionFrm(), false );
                        rpActualSection->GetSectionFrm()->SimpleFormat();
                        bInit = true;
                    }
                    rpActualSection->SetSectionFrm( pSct );
                    pSct->InsertBehind( rpLay, nullptr );
                    if( bInit )
                        pSct->Init();
                    pSct->Frm().Pos() = rpLay->Frm().Pos();
                    pSct->Frm().Pos().Y() += 1; //because of the notifications

                    rpLay = pSct;
                    if ( rpLay->Lower() && rpLay->Lower()->IsLayoutFrm() )
                        rpLay = rpLay->GetNextLayoutLeaf();
                }
            }
        } while( bLongTab || ( pImpl && nIndex < pImpl->size() &&
                 pImpl->GetBreakIndex( nIndex ) == nNodeIndex ) );
    }
    bFirst = false;
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
void SwLayHelper::_CheckFlyCache( SwPageFrm* pPage )
{
    if( !pImpl || !pPage )
        return;
    const size_t nFlyCount = pImpl->GetFlyCount();
    // Any text frames at the page, fly cache available?
    if( pPage->GetSortedObjs() && nFlyIdx < nFlyCount )
    {
        SwSortedObjs &rObjs = *pPage->GetSortedObjs();
        sal_uInt16 nPgNum = pPage->GetPhyPageNum();

        // NOTE: Here we do not use the absolute ordnums but
        // relative ordnums for the objects on this page.

        // skip fly frames from pages before the current page
        while( nFlyIdx < nFlyCount &&
               pImpl->GetFlyCache(nFlyIdx).nPageNum < nPgNum )
            ++nFlyIdx;

        // sort cached objects on this page by ordnum
        std::set< const SwFlyCache*, FlyCacheCompare > aFlyCacheSet;
        size_t nIdx = nFlyIdx;

        SwFlyCache* pFlyC;
        while( nIdx < nFlyCount &&
               ( pFlyC = &pImpl->GetFlyCache( nIdx ) )->nPageNum == nPgNum )
        {
            aFlyCacheSet.insert( pFlyC );
            ++nIdx;
        }

        // sort objects on this page by ordnum
        std::set< const SdrObject*, SdrObjectCompare > aFlySet;
        for ( size_t i = 0; i < rObjs.size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = rObjs[i];
            if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )  // a text frame?
            {
                SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                if( pFly->GetAnchorFrm() &&
                    !pFly->GetAnchorFrm()->FindFooterOrHeader() )
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
            std::set< const SwFlyCache*, FlyCacheCompare >::iterator aFlyCacheSetIt =
                    aFlyCacheSet.begin();
            std::set< const SdrObject*, SdrObjectCompare >::iterator aFlySetIt =
                    aFlySet.begin();

            while ( aFlyCacheSetIt != aFlyCacheSet.end() )
            {
                const SwFlyCache* pFlyCache = *aFlyCacheSetIt;
                SwFlyFrm* pFly = const_cast<SwVirtFlyDrawObj*>(static_cast<const SwVirtFlyDrawObj*>(*aFlySetIt))->GetFlyFrm();

                if ( pFly->Frm().Left() == FAR_AWAY )
                {
                    // we get the stored information
                    pFly->Frm().Pos().X() = pFlyCache->Left() +
                                            pPage->Frm().Left();
                    pFly->Frm().Pos().Y() = pFlyCache->Top() +
                                            pPage->Frm().Top();
                    if ( pImpl->IsUseFlyCache() )
                    {
                        pFly->Frm().Width( pFlyCache->Width() );
                        pFly->Frm().Height( pFlyCache->Height() );
                    }
                }

                ++aFlyCacheSetIt;
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

bool SwLayCacheIoImpl::OpenRec( sal_uInt8 cType )
{
    bool bRes = true;
    sal_uInt32 nPos = pStream->Tell();
    if( bWriteMode )
    {
        aRecords.push_back( RecTypeSize(cType, nPos) );
        pStream->WriteUInt32( 0 );
    }
    else
    {
        sal_uInt32 nVal(0);
        pStream->ReadUInt32( nVal );
        sal_uInt8 cRecTyp = (sal_uInt8)nVal;
        if( !nVal || cRecTyp != cType ||
            pStream->GetErrorCode() != SVSTREAM_OK || pStream->IsEof() )
        {
            OSL_ENSURE( nVal, "OpenRec: Record-Header is 0" );
            OSL_ENSURE( cRecTyp == cType, "OpenRec: Wrong Record Type" );
            aRecords.push_back( RecTypeSize(0, pStream->Tell()) );
            bRes = false;
            bError = true;
        }
        else
        {
            sal_uInt32 nSize = nVal >> 8;
            aRecords.push_back( RecTypeSize(cRecTyp, nPos+nSize) );
        }
    }
    return bRes;
}

// Close record
bool SwLayCacheIoImpl::CloseRec( sal_uInt8 )
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
            if( pStream->GetError() != SVSTREAM_OK )
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
            if( pStream->GetErrorCode() != SVSTREAM_OK )
                bRes = false;
        }
        aRecords.pop_back();
    }

    if( !bRes )
        bError = true;

    return bRes;
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
        if( pStream->GetErrorCode() != SVSTREAM_OK )
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
    CloseRec( c );
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
        OSL_ENSURE( pStream->Tell() <= nFlagRecEnd, "To many data read" );
        if( pStream->Tell() != nFlagRecEnd )
            pStream->Seek( nFlagRecEnd );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
