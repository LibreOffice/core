/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>
#include <bf_svx/brkitem.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docstat.hxx>
#include <docary.hxx>
#include <fmtpdsc.hxx>
#include <laycache.hxx>
#include <layhelp.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <ndindex.hxx>
#include <sectfrm.hxx>
#include <fmtcntnt.hxx>
#include <pagedesc.hxx>
#include <frmtool.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <flyfrm.hxx>

#include <set>
namespace binfilter {

/*N*/ SV_IMPL_PTRARR( SwPageFlyCache, SwFlyCachePtr )

/*-----------------28.5.2001 10:06------------------
 *  Reading and writing of the layout cache.
 *  The layout cache is not necessary, but it improves
 *  the performance and reduces the text flow during
 *  the formatting.
 *  The layout cache contains the index of the paragraphs/tables
 *  at the top of every page, so it's possible to create
 *  the right count of pages and to distribute the document content
 *  to this pages before the formatting starts.
 *--------------------------------------------------*/

/*N*/ void SwLayoutCache::Read( SvStream &rStream )
/*N*/ {
/*N*/     if( !pImpl )
/*N*/ 	{
/*N*/         pImpl = new SwLayCacheImpl;
/*N*/ 		if( !pImpl->Read( rStream ) )
/*N*/ 		{
/*?*/ 			delete pImpl;
/*?*/ 			pImpl = 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SwLayCacheImpl::Insert( USHORT nType, ULONG nIndex, xub_StrLen nOffset )
/*N*/ {
/*N*/ 	aType.Insert( nType, aType.Count() );
/*N*/ 	SvULongs::Insert( nIndex, SvULongs::Count() );
/*N*/ 	aOffset.Insert( nOffset, aOffset.Count() );
/*N*/ }

/*N*/ BOOL SwLayCacheImpl::Read( SvStream& rStream )
/*N*/ {
/*N*/ 	SwLayCacheIoImpl aIo( rStream, FALSE );
/*N*/ 	if( aIo.GetMajorVersion() > SW_LAYCACHE_IO_VERSION_MAJOR )
/*?*/ 		return FALSE;
/*N*/ 
/*N*/     // Due to an evil bug in the layout cache (#102759#), we cannot trust the
/*N*/     // sizes of fly frames which have been written using the "old" layout cache.
/*N*/     // This flag should indicate that we do not want to trust the width and
/*N*/     // height of fly frames
/*N*/     bUseFlyCache = aIo.GetMinorVersion() >= 1;
/*N*/ 
/*N*/ 	BYTE cFlags;
/*N*/ 	UINT32 nIndex, nOffset;
/*N*/ 
/*N*/ 	aIo.OpenRec( SW_LAYCACHE_IO_REC_PAGES );
/*N*/ 	aIo.OpenFlagRec();
/*N*/ 	aIo.CloseFlagRec();
/*N*/ 	while( aIo.BytesLeft() && !aIo.HasError() )
/*N*/ 	{
/*N*/ 		switch( aIo.Peek() )
/*N*/ 		{
/*N*/ 		case SW_LAYCACHE_IO_REC_PARA:
/*N*/ 			aIo.OpenRec( SW_LAYCACHE_IO_REC_PARA );
/*N*/ 			cFlags = aIo.OpenFlagRec();
/*N*/ 			aIo.GetStream() >> nIndex;
/*N*/ 			if( (cFlags & 0x01) != 0 )
/*N*/ 				aIo.GetStream() >> nOffset;
/*N*/ 			else
/*N*/ 				nOffset = STRING_LEN;
/*N*/ 			aIo.CloseFlagRec();
/*N*/ 			Insert( SW_LAYCACHE_IO_REC_PARA, nIndex, (xub_StrLen)nOffset );
/*N*/ 			aIo.CloseRec( SW_LAYCACHE_IO_REC_PARA );
/*N*/ 			break;
/*N*/ 		case SW_LAYCACHE_IO_REC_TABLE:
/*?*/ 			aIo.OpenRec( SW_LAYCACHE_IO_REC_TABLE );
/*?*/ 			aIo.OpenFlagRec();
/*?*/             aIo.GetStream() >> nIndex
/*?*/                             >> nOffset;
/*?*/ 			Insert( SW_LAYCACHE_IO_REC_TABLE, nIndex, (xub_StrLen)nOffset );
/*?*/ 			aIo.CloseFlagRec();
/*?*/ 			aIo.CloseRec( SW_LAYCACHE_IO_REC_TABLE );
/*?*/ 			break;
/*N*/         case SW_LAYCACHE_IO_REC_FLY:
/*N*/         {
/*N*/             aIo.OpenRec( SW_LAYCACHE_IO_REC_FLY );
/*N*/ 			aIo.OpenFlagRec();
/*N*/ 			aIo.CloseFlagRec();
/*N*/             long nX, nY, nW, nH;
/*N*/             USHORT nPgNum;
/*N*/             aIo.GetStream() >> nPgNum >> nIndex
/*N*/                     >> nX >> nY >> nW >> nH;
/*N*/             SwFlyCache* pFly = new SwFlyCache( nPgNum, nIndex, nX, nY, nW, nH );
/*N*/             aFlyCache.Insert( pFly, aFlyCache.Count() );
/*N*/             aIo.CloseRec( SW_LAYCACHE_IO_REC_FLY );
/*N*/ 			break;
/*N*/         }
/*N*/ 		default:
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 aIo.SkipRec();
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aIo.CloseRec( SW_LAYCACHE_IO_REC_PAGES );
/*N*/ 
/*N*/ 	return !aIo.HasError();
/*N*/ }

/*-----------------28.5.2001 10:19------------------
 * SwLayoutCache::Write(..)
 * writes the index (more precise: the difference between
 * the index and the first index of the document content)
 * of the first paragraph/table at the top of every page.
 * If at the top of a page is the rest of a paragraph/table
 * from the bottom of the previous page, the character/row
 * number is stored, too.
 * The position, size and page number of the text frames
 * are stored, too
 * --------------------------------------------------*/

/*N*/ void SwLayoutCache::Write( SvStream &rStream, const SwDoc& rDoc )
/*N*/ {
/*N*/     if( rDoc.GetRootFrm() ) // the layout itself ..
/*N*/     {
/*N*/ 		SwLayCacheIoImpl aIo( rStream, TRUE );
/*N*/         // We want to save the relative index, so we need the index
/*N*/         // of the first content
/*N*/         ULONG nStartOfContent = rDoc.GetNodes().GetEndOfContent().
/*N*/                                 FindStartNode()->GetIndex();
/*N*/         // The first page..
/*N*/         SwPageFrm* pPage = (SwPageFrm*)rDoc.GetRootFrm()->Lower();
/*N*/ 
/*N*/ 		aIo.OpenRec( SW_LAYCACHE_IO_REC_PAGES );
/*N*/ 		aIo.OpenFlagRec( 0, 0 );
/*N*/ 		aIo.CloseFlagRec();
/*N*/         while( pPage )
/*N*/         {
/*N*/             if( pPage->GetPrev() )
/*N*/             {
/*N*/                 SwLayoutFrm* pLay = pPage->FindBodyCont();
/*N*/                 SwFrm* pTmp = pLay ? pLay->ContainsAny() : NULL;
/*N*/                 // We are only interested in paragraph or table frames,
/*N*/                 // a section frames contains paragraphs/tables.
/*N*/                 if( pTmp && pTmp->IsSctFrm() )
/*?*/                     pTmp = ((SwSectionFrm*)pTmp)->ContainsAny();
/*N*/ 
/*N*/                 if( pTmp ) // any content
/*N*/                 {
/*N*/                     if( pTmp->IsTxtFrm() )
/*N*/                     {
/*N*/                         ULONG nNdIdx = ((SwTxtFrm*)pTmp)->GetNode()->GetIndex();
/*N*/                         if( nNdIdx > nStartOfContent )
/*N*/                         {
/*N*/                             /*  Open Paragraph Record */
/*N*/                             aIo.OpenRec( SW_LAYCACHE_IO_REC_PARA );
/*N*/                             BOOL bFollow = ((SwTxtFrm*)pTmp)->IsFollow();
/*N*/                             aIo.OpenFlagRec( bFollow ? 0x01 : 0x00,
/*N*/                                             bFollow ? 8 : 4 );
/*N*/                             nNdIdx -= nStartOfContent;
/*N*/                             aIo.GetStream() << static_cast<sal_uInt32>(nNdIdx);
/*N*/                             if( bFollow )
/*?*/                                 aIo.GetStream() << static_cast<sal_uInt32>(((SwTxtFrm*)pTmp)->GetOfst());
/*N*/                             aIo.CloseFlagRec();
/*N*/                             /*  Close Paragraph Record */
/*N*/                             aIo.CloseRec( SW_LAYCACHE_IO_REC_PARA );
/*N*/                         }
/*N*/                     }
/*N*/                     else if( pTmp->IsTabFrm() )
/*N*/                     {
/*N*/                         SwTabFrm* pTab = (SwTabFrm*)pTmp;
/*N*/                         ULONG nOfst = STRING_LEN;
/*N*/                         if( pTab->IsFollow() )
/*N*/                         {
/*N*/                             // If the table is a follow, we have to look for the
/*N*/                             // master and to count all rows to get the row number
/*N*/                             nOfst = 0;
/*N*/                             while( pTab->IsFollow() )
/*N*/                                 pTab = pTab->FindMaster();
/*N*/                             while( pTab != pTmp )
/*N*/                             {
/*N*/                                 SwFrm* pSub = pTab->Lower();
/*N*/                                 while( pSub )
/*N*/                                 {
/*N*/                                     ++nOfst;
/*N*/                                     pSub = pSub->GetNext();
/*N*/                                 }
/*N*/                                 pTab = pTab->GetFollow();
/*N*/                                 ASSERT( pTab, "Table follow without master" );
/*N*/                             }
/*N*/                         }
/*N*/                         do
/*N*/                         {
/*N*/                             ULONG nNdIdx =
/*N*/                                     pTab->GetTable()->GetTableNode()->GetIndex();
/*N*/                             if( nNdIdx > nStartOfContent )
/*N*/                             {
/*N*/                                 /* Open Table Record */
/*N*/                                 aIo.OpenRec( SW_LAYCACHE_IO_REC_TABLE );
/*N*/                                 aIo.OpenFlagRec( 0, 8 );
/*N*/                                 nNdIdx -= nStartOfContent;
/*N*/                                 aIo.GetStream() << static_cast<sal_uInt32>(nNdIdx)
/*N*/                                                 << static_cast<sal_uInt32>(nOfst);
/*N*/                                 aIo.CloseFlagRec();
/*N*/                                 /* Close Table Record  */
/*N*/                                 aIo.CloseRec( SW_LAYCACHE_IO_REC_TABLE );
/*N*/                             }
/*N*/                             // If the table has a follow on the next page,
/*N*/                             // we know already the row number and store this
/*N*/                             // immediately.
/*N*/                             if( pTab->GetFollow() )
/*N*/                             {
/*N*/                                 if( nOfst == STRING_LEN )
/*?*/                                     nOfst = 0;
/*N*/                                 do
/*N*/                                 {
/*N*/                                     SwFrm* pSub = pTab->Lower();
/*N*/                                     while( pSub )
/*N*/                                     {
/*N*/                                         ++nOfst;
/*N*/                                         pSub = pSub->GetNext();
/*N*/                                     }
/*N*/                                     pTab = pTab->GetFollow();
/*N*/                                     SwPageFrm *pTabPage = pTab->FindPageFrm();
/*N*/                                     if( pTabPage != pPage )
/*N*/                                     {
/*N*/                                         ASSERT( pPage->GetPhyPageNum() <
/*N*/                                                 pTabPage->GetPhyPageNum(),
/*N*/                                                 "Looping Tableframes" );
/*N*/                                         pPage = pTabPage;
/*N*/                                         break;
/*N*/                                     }
/*?*/                                 } while ( pTab->GetFollow() );
/*N*/                             }
/*N*/                             else
/*N*/                                 break;
/*N*/                         } while( pTab );
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/             if( pPage->GetSortedObjs() )
/*N*/             {
/*?*/                 SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
/*?*/                 for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*?*/                 {
/*?*/                     SdrObject *pO = rObjs[i];
/*?*/                     if ( pO->IsWriterFlyFrame() )
/*?*/                     {
/*?*/                         SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*?*/                         if( pFly->Frm().Left() != WEIT_WECH &&
/*?*/                             !pFly->GetAnchor()->FindFooterOrHeader() )
/*?*/                         {
/*?*/                             const SwContact *pC = (SwContact*)GetUserCall(pO);
/*?*/                             if( pC )
/*?*/                             {
/*?*/                                 sal_uInt32 nOrdNum = pO->GetOrdNum();
/*?*/                                 USHORT nPageNum = pPage->GetPhyPageNum();
/*?*/                                 /* Open Fly Record */
/*?*/                                 aIo.OpenRec( SW_LAYCACHE_IO_REC_FLY );
/*?*/                                 aIo.OpenFlagRec( 0, 0 );
/*?*/                                 aIo.CloseFlagRec();
/*?*/                                 SwRect &rRct = pFly->Frm();
/*?*/                                 sal_Int32 nX = rRct.Left() - pPage->Frm().Left();
/*?*/                                 sal_Int32 nY = rRct.Top() - pPage->Frm().Top();
/*?*/                                 aIo.GetStream() << nPageNum << nOrdNum
/*?*/                                                 << nX << nY << rRct.Width()
/*?*/                                                 << rRct.Height();
/*?*/                                 /* Close Fly Record  */
/*?*/                                 aIo.CloseRec( SW_LAYCACHE_IO_REC_FLY );
/*?*/                             }
/*?*/                         }
/*?*/                     }
/*?*/                 }
/*N*/             }
/*N*/             pPage = (SwPageFrm*)pPage->GetNext();
/*N*/         }
/*N*/ 		aIo.CloseRec( SW_LAYCACHE_IO_REC_PAGES );
/*N*/     }
/*N*/ }


/*N*/ void SwLayoutCache::ClearImpl()
/*N*/ {
/*N*/     if( !IsLocked() )
/*N*/     {
/*N*/         delete pImpl;
/*N*/         pImpl = 0;
/*N*/     }
/*N*/ }


/*N*/ SwLayoutCache::~SwLayoutCache()
/*N*/ {
/*N*/     ASSERT( !nLockCount, "Deleting a locked SwLayoutCache!?" );
/*N*/     delete pImpl;
/*N*/ }

/*-----------------28.5.2001 10:47------------------
 * SwActualSection,
 *  a help class to create not nested section frames
 *  for nested sections.
 * --------------------------------------------------*/

/*N*/ SwActualSection::SwActualSection( SwActualSection *pUp,
/*N*/ 								  SwSectionFrm	  *pSect,
/*N*/ 								  SwSectionNode	  *pNd ) :
/*N*/ 	pUpper( pUp ),
/*N*/ 	pSectFrm( pSect ),
/*N*/ 	pSectNode( pNd )
/*N*/ {
/*N*/ 	if ( !pSectNode )
/*N*/ 	{
/*?*/ 		const SwNodeIndex *pIndex = pSect->GetFmt()->GetCntnt().GetCntntIdx();
/*?*/ 		pSectNode = pSect->GetFmt()->GetDoc()->GetNodes()[*pIndex]->
/*?*/ 															FindSectionNode();
/*N*/ 	}
/*N*/ }

/*-----------------28.5.2001 11:09------------------
 * SwLayHelper
 *  is the helper class, which utilizes the layout cache information
 *  to distribute the document content to the rigth pages.
 * It's used by the _InsertCnt(..)-function.
 * If there's no layout cache, the distibution to the pages is more
 * a guess, but a guess with statistical background.
 * --------------------------------------------------*/

/*N*/ SwLayHelper::SwLayHelper( SwDoc *pD, SwFrm* &rpF, SwFrm* &rpP, SwPageFrm* &rpPg,
/*N*/             SwLayoutFrm* &rpL, SwActualSection* &rpA, BOOL &rB,
/*N*/             ULONG nNodeIndex, BOOL bCache )
/*N*/     : rpFrm( rpF ), rpPrv( rpP ), rpPage( rpPg ), rpLay( rpL ),
/*N*/       rpActualSection( rpA ), rbBreakAfter(rB), pDoc(pD), nMaxParaPerPage( 25 ),
/*N*/       nParagraphCnt( bCache ? 0 : USHRT_MAX ), bFirst( bCache )
/*N*/ {
/*N*/     pImpl = pDoc->GetLayoutCache() ? pDoc->GetLayoutCache()->LockImpl() : NULL;
/*N*/     if( pImpl )
/*N*/     {
/*?*/         nMaxParaPerPage = 1000;
/*?*/         nStartOfContent = pDoc->GetNodes().GetEndOfContent().FindStartNode()
/*?*/                           ->GetIndex();
/*?*/         nNodeIndex -= nStartOfContent;
/*?*/         nIndex = 0;
/*?*/         nFlyIdx = 0;
/*?*/         while( nIndex < pImpl->Count() && (*pImpl)[ nIndex ] < nNodeIndex )
/*?*/             ++nIndex;
/*?*/         if( nIndex >= pImpl->Count() )
/*?*/         {
/*?*/             pDoc->GetLayoutCache()->UnlockImpl();
/*?*/             pImpl = NULL;
/*?*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         nIndex = USHRT_MAX;
/*N*/         nStartOfContent = ULONG_MAX;
/*N*/     }
/*N*/ }

/*N*/ SwLayHelper::~SwLayHelper()
/*N*/ {
/*N*/     if( pImpl )
/*N*/     {
/*?*/         ASSERT( pDoc && pDoc->GetLayoutCache(), "Missing layoutcache" );
/*?*/         pDoc->GetLayoutCache()->UnlockImpl();
/*N*/     }
/*N*/ }

/*-----------------23.5.2001 16:40------------------
 * SwLayHelper::CalcPageCount() does not really calculate the page count,
 * it returns the page count value from the layout cache, if available,
 * otherwise it estimates the page count.
 * --------------------------------------------------*/

/*N*/ ULONG SwLayHelper::CalcPageCount()
/*N*/ {
/*N*/     ULONG nPgCount;
/*N*/     SwLayCacheImpl *pCache = pDoc->GetLayoutCache() ?
/*N*/                              pDoc->GetLayoutCache()->LockImpl() : NULL;
/*N*/     if( pCache )
/*N*/     {
/*?*/         nPgCount = pCache->Count() + 1;
/*?*/         pDoc->GetLayoutCache()->UnlockImpl();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/ 		nPgCount = pDoc->GetDocStat().nPage;
/*N*/         if ( nPgCount <= 10 ) // no page insertion for less than 10 pages
/*N*/ 			nPgCount = 0;
/*N*/ 		ULONG nNdCount = pDoc->GetDocStat().nPara;
/*N*/ 		if ( nNdCount <= 1 )
/*N*/ 		{
/*N*/             //Estimates the number of paragraphs.
/*N*/ 			ULONG nTmp = pDoc->GetNodes().GetEndOfContent().GetIndex() -
/*N*/ 						pDoc->GetNodes().GetEndOfExtras().GetIndex();
/*N*/             //Tables have a little overhead..
/*N*/ 			nTmp -= pDoc->GetTblFrmFmts()->Count() * 25;
/*N*/             //Fly frames, too ..
/*N*/ 			nTmp -= (pDoc->GetNodes().GetEndOfAutotext().GetIndex() -
/*N*/ 					   pDoc->GetNodes().GetEndOfInserts().GetIndex()) / 3 * 5;
/*N*/ 			if ( nTmp > 0 )
/*N*/ 				nNdCount = nTmp;
/*N*/ 		}
/*N*/         if ( nNdCount > 100 ) // no estimation below this value
/*N*/ 		{
/*N*/ 			if ( nPgCount > 0 )
/*N*/ 				nMaxParaPerPage = nNdCount / nPgCount;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				nMaxParaPerPage = Max( ULONG(20),
/*N*/ 									   ULONG(20 + nNdCount / 1000 * 3) );
/*N*/ #ifdef PM2
/*N*/ 				const ULONG nMax = 49;
/*N*/ #elif MAC
/*N*/ 				const ULONG nMax = 56;
/*N*/ #elif UNIX
/*N*/ 				const ULONG nMax = 57;
/*N*/ #else
/*N*/ 				const ULONG nMax = 53;
/*N*/ #endif
/*N*/ 				nMaxParaPerPage = Min( nMaxParaPerPage, nMax );
/*N*/ 				nPgCount = nNdCount / nMaxParaPerPage;
/*N*/ 			}
/*N*/             if ( nNdCount < 1000 )
/*N*/                 nPgCount = 0;// no progress bar for small documents
/*N*/ 			if ( pDoc->IsBrowseMode() )
/*?*/ 				nMaxParaPerPage *= 6;
/*N*/ 		}
/*N*/     }
/*N*/     return nPgCount;
/*N*/ }

/*-----------------23.5.2001 16:44------------------
 * SwLayHelper::CheckInsertPage()
 * inserts a page and return TRUE, if
 * - the break after flag is set
 * - the actual content wants a break before
 * - the maximum count of paragraph/rows is reached
 *
 * The break after flag is set, if the actual content
 * wants a break after.
 * --------------------------------------------------*/

/*N*/ BOOL SwLayHelper::CheckInsertPage()
/*N*/ {
/*N*/ 	FASTBOOL bEnd = 0 == rpPage->GetNext();
/*N*/     const SwAttrSet *pAttr = rpFrm->GetAttrSet();
/*N*/ 	const SvxFmtBreakItem &rBrk = pAttr->GetBreak();
/*N*/ 	const SwFmtPageDesc &rDesc = pAttr->GetPageDesc();
/*N*/ 	const SwPageDesc *pDesc = rDesc.GetPageDesc();
/*N*/ 
/*N*/ 	BOOL bBrk = nParagraphCnt > nMaxParaPerPage || rbBreakAfter;
/*N*/ 	rbBreakAfter = rBrk.GetBreak() == SVX_BREAK_PAGE_AFTER ||
/*N*/ 				   rBrk.GetBreak() == SVX_BREAK_PAGE_BOTH;
/*N*/ 	if ( !bBrk )
/*N*/ 		bBrk = rBrk.GetBreak() == SVX_BREAK_PAGE_BEFORE ||
/*N*/ 			   rBrk.GetBreak() == SVX_BREAK_PAGE_BOTH;
/*N*/ 
/*N*/ 	if ( bBrk || pDesc )
/*N*/ 	{
/*N*/ 		USHORT nPgNum = 0;
/*N*/ 		if ( !pDesc )
/*N*/ 			pDesc = rpPage->GetPageDesc()->GetFollow();
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if ( 0 != (nPgNum = rDesc.GetNumOffset()) )
/*N*/ 				((SwRootFrm*)rpPage->GetUpper())->SetVirtPageNum(TRUE);
/*N*/ 		}
/*N*/ 		BOOL bOdd = !rpPage->OnRightPage();
/*N*/ 		BOOL bInsertEmpty = FALSE;
/*N*/ 		if( nPgNum && bOdd != ( ( nPgNum % 2 ) != 0 ) )
/*N*/ 		{
/*N*/ 			bOdd = !bOdd;
/*N*/ 			bInsertEmpty = TRUE;
/*N*/ 		}
/*N*/ 		::binfilter::InsertNewPage( (SwPageDesc&)*pDesc, rpPage->GetUpper(),
/*N*/ 						 bOdd, bInsertEmpty, FALSE, rpPage->GetNext() );
/*N*/ 		if ( bEnd )
/*N*/ 		{
/*N*/ 			ASSERT( rpPage->GetNext(), "Keine neue Seite?" );
/*N*/ 			do
/*N*/ 			{	rpPage = (SwPageFrm*)rpPage->GetNext();
/*N*/ 			} while ( rpPage->GetNext() );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			ASSERT( rpPage->GetNext(), "Keine neue Seite?" );
/*?*/ 			rpPage = (SwPageFrm*)rpPage->GetNext();
/*?*/ 			if ( rpPage->IsEmptyPage() )
/*?*/ 			{
/*?*/ 				ASSERT( rpPage->GetNext(), "Keine neue Seite?" );
/*?*/ 				rpPage = (SwPageFrm*)rpPage->GetNext();
/*?*/ 			}
/*N*/ 		}
/*N*/ 		rpLay = rpPage->FindBodyCont();
/*N*/ 		while( rpLay->Lower() )
/*N*/ 			rpLay = (SwLayoutFrm*)rpLay->Lower();
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*-----------------28.5.2001 11:31------------------
 * SwLayHelper::CheckInsert
 *  is the entry point for the _InsertCnt-function.
 *  The document content index is checked either it is
 *  in the layout cache either it's time to insert a page
 *  cause the maximal estimation of content per page is reached.
 *  A really big table or long paragraph may contains more than
 *  one page, in this case the needed count of pages will inserted.
 * --------------------------------------------------*/

/*N*/ BOOL SwLayHelper::CheckInsert( ULONG nNodeIndex )
/*N*/ {
/*N*/     BOOL bRet = FALSE;
/*N*/     BOOL bLongTab = FALSE;
/*N*/     ULONG nMaxRowPerPage;
/*N*/     nNodeIndex -= nStartOfContent;
/*N*/     USHORT nRows;
/*N*/     if( rpFrm->IsTabFrm() )
/*N*/     {
/*N*/         //Inside a table counts every row as a paragraph
/*N*/         SwFrm *pLow = ((SwTabFrm*)rpFrm)->Lower();
/*N*/         nRows = 0;
/*N*/         do
/*N*/         {
/*N*/             ++nRows;
/*N*/             pLow = pLow->GetNext();
/*N*/         } while ( pLow );
/*N*/         nParagraphCnt += nRows;
/*N*/         if( !pImpl && nParagraphCnt > nMaxParaPerPage + 10 )
/*N*/         {
/*N*/             // OD 09.04.2003 #108698# - improve heuristics:
/*N*/             // Assume that a table, which has more than three times the quantity
/*N*/             // of maximal paragraphs per page rows, consists of rows, which have
/*N*/             // the height of a normal paragraph. Thus, allow as much rows per page
/*N*/             // as much paragraphs are allowed.
/*N*/             if ( nRows > ( 3*nMaxParaPerPage ) )
/*N*/             {
/*N*/                 nMaxRowPerPage = nMaxParaPerPage;
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/					SwFrm *pTmp = ((SwTabFrm*)rpFrm)->Lower();
/*N*/					if( pTmp->GetNext() )
/*N*/					pTmp = pTmp->GetNext();
/*N*/					pTmp = ((SwRowFrm*)pTmp)->Lower();
/*N*/					USHORT nCnt = 0;
/*N*/					do
/*N*/					{
/*N*/						++nCnt;
/*N*/						pTmp = pTmp->GetNext();
/*N*/					} while( pTmp );
/*N*/					nMaxRowPerPage = Max( ULONG(2), nMaxParaPerPage / nCnt );
/*N*/			 }
/*N*/             bLongTab = TRUE;
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/         ++nParagraphCnt;
/*N*/     if( bFirst && pImpl && nIndex < pImpl->Count() &&
/*N*/         pImpl->GetBreakIndex( nIndex ) == nNodeIndex &&
/*N*/         ( pImpl->GetBreakOfst( nIndex ) < STRING_LEN ||
/*N*/           ( ++nIndex < pImpl->Count() &&
/*N*/           pImpl->GetBreakIndex( nIndex ) == nNodeIndex ) ) )
/*?*/         bFirst = FALSE;
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/     ULONG nBreakIndex = ( pImpl && nIndex < pImpl->Count() ) ?
/*N*/                         pImpl->GetBreakIndex(nIndex) : 0xffff;
/*N*/ #endif
/*N*/     // OD 09.04.2003 #108698# - always split a big tables.
/*N*/     if ( !bFirst ||
/*N*/          ( rpFrm->IsTabFrm() && bLongTab )
/*N*/        )
/*N*/     {
/*N*/         ULONG nRowCount = 0;
/*N*/         do
/*N*/         {
/*N*/             if( pImpl || bLongTab )
/*N*/             {
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/                 ULONG nBrkIndex = ( pImpl && nIndex < pImpl->Count() ) ?
/*N*/                         pImpl->GetBreakIndex(nIndex) : 0xffff;
/*N*/ #endif
/*N*/                 xub_StrLen nOfst = STRING_LEN;
/*N*/                 USHORT nType = SW_LAYCACHE_IO_REC_PAGES;
/*N*/                 if( bLongTab )
/*N*/                 {
/*N*/                     rbBreakAfter = sal_True;
/*N*/                     nOfst = nRowCount + nMaxRowPerPage;
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*?*/                     while( nIndex < pImpl->Count() &&
/*?*/                            pImpl->GetBreakIndex(nIndex) < nNodeIndex)
/*?*/                         ++nIndex;
/*?*/                     if( nIndex < pImpl->Count() &&
/*?*/                         pImpl->GetBreakIndex(nIndex) == nNodeIndex )
/*?*/                     {
/*?*/                         nType = pImpl->GetBreakType( nIndex );
/*?*/                         nOfst = pImpl->GetBreakOfst( nIndex++ );
/*?*/                         rbBreakAfter = sal_True;
/*?*/                     }
/*N*/                 }
/*N*/ 
/*N*/                 if( nOfst < STRING_LEN )
/*N*/                 {
/*N*/                     sal_Bool bSplit = sal_False;
/*N*/                     sal_Bool bRepeat;
/*N*/                     if( !bLongTab && rpFrm->IsTxtFrm() &&
/*N*/                         SW_LAYCACHE_IO_REC_PARA == nType &&
/*N*/                         nOfst<((SwTxtFrm*)rpFrm)->GetTxtNode()->GetTxt().Len() )
/*?*/                         bSplit = sal_True;
/*N*/                     else if( rpFrm->IsTabFrm() && nRowCount < nOfst &&
/*N*/                              ( bLongTab || SW_LAYCACHE_IO_REC_TABLE == nType ) )
/*N*/                     {
/*N*/                         bRepeat = ((SwTabFrm*)rpFrm)->
/*N*/                                   GetTable()->IsHeadlineRepeat();
/*N*/                         bSplit = nOfst < nRows;
/*N*/                         bLongTab = bLongTab && bSplit;
/*N*/                     }
/*N*/                     if( bSplit )
/*N*/                     {
/*N*/                         rpFrm->InsertBehind( rpLay, rpPrv );
/*N*/                         rpFrm->Frm().Pos() = rpLay->Frm().Pos();
/*N*/                         rpFrm->Frm().Pos().Y() += 1;
/*N*/                         rpPrv = rpFrm;
/*N*/                         if( rpFrm->IsTabFrm() )
/*N*/                         {
/*N*/                             SwTabFrm* pTab = (SwTabFrm*)rpFrm;
/*N*/                             SwFrm *pRow = pTab->Lower();
/*N*/                             SwTabFrm *pFoll = new SwTabFrm( *pTab );
/*N*/ 
/*N*/                             SwFrm *pPrv;
/*N*/                             if( bRepeat )
/*N*/                             {
/*N*/                                 bDontCreateObjects = TRUE; //frmtool
/*N*/                                 SwRowFrm *pHeadline = new SwRowFrm(
/*N*/                                         *pTab->GetTable()->GetTabLines()[0] );
/*N*/                                 pHeadline->InsertBefore( pFoll, 0 );
/*N*/                                 bDontCreateObjects = FALSE;
/*N*/                                 pPrv = pFoll->Lower();
/*N*/                                 ++nRows;
/*N*/                             }
/*N*/                             else
/*?*/                                 pPrv = 0;
/*N*/                             while( pRow && nRowCount < nOfst )
/*N*/                             {
/*N*/                                 pRow = pRow->GetNext();
/*N*/                                 ++nRowCount;
/*N*/                             }
/*N*/                             while ( pRow )
/*N*/                             {
/*N*/                                 SwFrm* pNxt = pRow->GetNext();
/*N*/                                 pRow->Remove();
/*N*/                                 pRow->InsertBehind( pFoll, pPrv );
/*N*/                                 pPrv = pRow;
/*N*/                                 pRow = pNxt;
/*N*/                             }
/*N*/                             rpFrm = pFoll;
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*?*/                             SwTxtFrm *pNew = new SwTxtFrm( ((SwTxtFrm*)rpFrm)->
/*?*/                                                            GetTxtNode() );
/*?*/                             pNew->_SetIsFollow( sal_True );
/*?*/                             pNew->ManipOfst( nOfst );
/*?*/                             pNew->SetFollow( ((SwTxtFrm*)rpFrm)->GetFollow() );
/*?*/                             ((SwTxtFrm*)rpFrm)->SetFollow( pNew );
/*?*/                             rpFrm = pNew;
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/ 
/*N*/             SwPageFrm* pLastPage = rpPage;
/*N*/             if( CheckInsertPage() )
/*N*/             {
/*N*/                 _CheckFlyCache( pLastPage );
/*N*/                 if( rpPrv && rpPrv->IsTxtFrm() && !rpPrv->GetValidSizeFlag() )
/*N*/                     rpPrv->Frm().Height( rpPrv->GetUpper()->Prt().Height() );
/*N*/ 
/*N*/                 bRet = TRUE;
/*N*/                 rpPrv = 0;
/*N*/                 nParagraphCnt = 0;
/*N*/ 
/*N*/                 if ( rpActualSection )
/*N*/                 {
/*N*/                     //Hatte der SectionFrm ueberhaupt Inhalt? Wenn
/*N*/                     //nicht kann er gleich umgehaengt werden.
/*N*/                     SwSectionFrm *pSct;
/*N*/                     BOOL bInit = FALSE;
/*N*/                     if ( !rpActualSection->GetSectionFrm()->ContainsCntnt())
/*N*/                     {
/*N*/                         pSct = rpActualSection->GetSectionFrm();
/*N*/                         pSct->Remove();
/*N*/                     }
/*N*/                     else
/*N*/                     {
/*N*/                         pSct = new SwSectionFrm(
/*N*/                             *rpActualSection->GetSectionFrm(), FALSE );
/*N*/                         rpActualSection->GetSectionFrm()->SimpleFormat();
/*N*/                         bInit = TRUE;
/*N*/                     }
/*N*/                     rpActualSection->SetSectionFrm( pSct );
/*N*/                     pSct->InsertBehind( rpLay, 0 );
/*N*/                     if( bInit )
/*N*/                         pSct->Init();
/*N*/                     pSct->Frm().Pos() = rpLay->Frm().Pos();
/*N*/                     pSct->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.
/*N*/ 
/*N*/                     rpLay = pSct;
/*N*/                     if ( rpLay->Lower() && rpLay->Lower()->IsLayoutFrm() )
/*?*/                         rpLay = rpLay->GetNextLayoutLeaf();
/*N*/                 }
/*N*/             }
/*N*/         } while( bLongTab || ( pImpl && nIndex < pImpl->Count() &&
/*N*/                  (*pImpl)[ nIndex ] == nNodeIndex ) );
/*N*/     }
/*N*/     bFirst = FALSE;
/*N*/     return bRet;
/*N*/ }

/*N*/ struct SdrObjectCompare
/*N*/ {
/*N*/   bool operator()( const SdrObject* pF1, const SdrObject* pF2 ) const
/*N*/   {
/*N*/     return pF1->GetOrdNum() < pF2->GetOrdNum();
/*N*/   }
/*N*/ };

/*N*/ struct FlyCacheCompare
/*N*/ {
/*N*/   bool operator()( const SwFlyCache* pC1, const SwFlyCache* pC2 ) const
/*N*/   {
/*N*/     return pC1->nOrdNum < pC2->nOrdNum;
/*N*/   }
/*N*/ };

 /*-----------------28.6.2001 14:40------------------
  * SwLayHelper::_CheckFlyCache(..)
  * If a new page is inserted, the last page is analysed.
  * If there are text frames with default position, the fly cache
  * is checked, if these frames are stored in the cache.
  * --------------------------------------------------*/

/*N*/ void SwLayHelper::_CheckFlyCache( SwPageFrm* pPage )
/*N*/ {
/*N*/     if( !pImpl || !pPage )
/*N*/         return;
/*N*/     USHORT nFlyCount = pImpl->GetFlyCount();
/*N*/     // Any text frames at the page, fly cache avaiable?
/*N*/     if( pPage->GetSortedObjs() && nFlyIdx < nFlyCount )
/*N*/     {
/*N*/         SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
/*N*/         USHORT nPgNum = pPage->GetPhyPageNum();

/*

        //
        // NOTE: This code assumes that all objects have already been
        // inserted into the drawing layout, so that the cached objects
        // can be identified by their ordnum. Unfortunately this function
        // is called with page n if page n+1 has been inserted. Thus
        // not all the objects have been inserted and the ordnums cannot
        // be used to identify the objects.
        //

        for ( USHORT i = 0; i < rObjs.Count(); ++i )  // check objects
        {
            SdrObject *pO = rObjs[i];
            if ( pO->IsWriterFlyFrame() )  // a text frame?
            {
                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                if( pFly->Frm().Left() == WEIT_WECH && pFly->GetAnchor() &&
                    !pFly->GetAnchor()->FindFooterOrHeader() )
                {   // Only frame with default position and not in header/footer
                    const SwContact *pC = (SwContact*)GetUserCall(pO);
                    if( pC )
                    {
                        ULONG nOrdNum = pO->GetOrdNum(); // the Id
                        SwFlyCache* pFlyC;
                        while( nFlyIdx < nFlyCount && ( pFlyC = pImpl->
                               GetFlyCache(nFlyIdx) )->nPageNum < nPgNum)
                            ++nFlyIdx;
                        if( nFlyIdx < nFlyCount &&
                            pFlyC->nPageNum == nPgNum )
                        {
                            USHORT nIdx = nFlyIdx;
                            while( nIdx < nFlyCount && ( pFlyC = pImpl->
                                   GetFlyCache( nIdx ) )->nPageNum == nPgNum &&
                                   pFlyC->nOrdNum != nOrdNum )
                                ++nIdx;
                            if( nIdx < nFlyCount && pFlyC->nPageNum == nPgNum &&
                                pFlyC->nOrdNum == nOrdNum )
                            {   // we get the stored information
                                pFly->Frm().Pos().X() = pFlyC->Left() +
                                                        pPage->Frm().Left();
                                pFly->Frm().Pos().Y() = pFlyC->Top() +
                                                        pPage->Frm().Top();
                                pFly->Frm().Width( pFlyC->Width() );
                                pFly->Frm().Height( pFlyC->Height() );
                            }
                        }
                    }
                }
            }
        }
 */

        //
        // NOTE: Here we do not use the absolute ordnums but
        // relative ordnums for the objects on this page.

        // skip fly frames from pages before the current page
/*N*/         SwFlyCache* pFlyC;
/*N*/         while( nFlyIdx < nFlyCount && ( pFlyC = pImpl->
/*N*/                GetFlyCache(nFlyIdx) )->nPageNum < nPgNum)
/*N*/             ++nFlyIdx;
/*N*/ 
/*N*/         // sort cached objects on this page by ordnum
/*N*/         std::set< const SwFlyCache*, FlyCacheCompare > aFlyCacheSet;
/*N*/         USHORT nIdx = nFlyIdx;
/*N*/ 
/*N*/         while( nIdx < nFlyCount && ( pFlyC = pImpl->
/*N*/                GetFlyCache( nIdx ) )->nPageNum == nPgNum )
/*N*/         {
/*N*/             aFlyCacheSet.insert( pFlyC );
/*N*/             ++nIdx;
/*N*/         }
/*N*/ 
/*N*/         // sort objects on this page by ordnum
/*N*/         std::set< const SdrObject*, SdrObjectCompare > aFlySet;
/*N*/         for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/         {
/*N*/             SdrObject* pO = rObjs[i];
/*N*/             if ( pO->IsWriterFlyFrame() )  // a text frame?
/*N*/             {
/*N*/                 SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/                 if( pFly->GetAnchor() &&
/*N*/                     !pFly->GetAnchor()->FindFooterOrHeader() )
/*N*/                 {
/*N*/                     const SwContact *pC = (SwContact*)GetUserCall(pO);
/*N*/                     if( pC )
/*N*/                     {
/*N*/                         aFlySet.insert( pO );
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/         if ( aFlyCacheSet.size() == aFlySet.size() )
/*N*/         {
/*N*/             std::set< const SwFlyCache*, FlyCacheCompare >::iterator aFlyCacheSetIt =
/*N*/                     aFlyCacheSet.begin();
/*N*/             std::set< const SdrObject*, SdrObjectCompare >::iterator aFlySetIt =
/*N*/                     aFlySet.begin();
/*N*/ 
/*N*/             while ( aFlyCacheSetIt != aFlyCacheSet.end() )
/*N*/             {
/*N*/                 const SwFlyCache* pFlyC = *aFlyCacheSetIt;
/*N*/                 SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)*aFlySetIt)->GetFlyFrm();
/*N*/ 
/*N*/                 if ( pFly->Frm().Left() == WEIT_WECH )
/*N*/                 {
/*N*/                     // we get the stored information
/*N*/                     pFly->Frm().Pos().X() = pFlyC->Left() +
/*N*/                                             pPage->Frm().Left();
/*N*/                     pFly->Frm().Pos().Y() = pFlyC->Top() +
/*N*/                                             pPage->Frm().Top();
/*N*/                     if ( pImpl->IsUseFlyCache() )
/*N*/                     {
/*N*/                         pFly->Frm().Width( pFlyC->Width() );
/*N*/                         pFly->Frm().Height( pFlyC->Height() );
/*N*/                     }
/*N*/                 }
/*N*/ 
/*N*/                 ++aFlyCacheSetIt;
/*N*/                 ++aFlySetIt;
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ }

/*-----------------28.6.2001 14:48------------------
 * SwLayHelper::CheckPageFlyCache(..)
 * looks for the given text frame in the fly cache and sets
 * the position and size, if possible.
 * The fly cache is sorted by pages and we start searching with the given page.
 * If we found the page number in the fly cache, we set
 * the rpPage parameter to the right page, if possible.
 * --------------------------------------------------*/

/*N*/ BOOL SwLayHelper::CheckPageFlyCache( SwPageFrm* &rpPage, SwFlyFrm* pFly )
/*N*/ {
/*N*/     if( !pFly->GetAnchor() || !pFly->GetVirtDrawObj() ||
/*N*/         pFly->GetAnchor()->FindFooterOrHeader() )
/*N*/         return FALSE;
/*N*/     BOOL bRet = FALSE;
/*N*/     SwDoc* pDoc = rpPage->GetFmt()->GetDoc();
/*N*/     SwLayCacheImpl *pCache = pDoc->GetLayoutCache() ?
/*N*/                              pDoc->GetLayoutCache()->LockImpl() : NULL;
/*N*/     if( pCache )
/*N*/     {
/*?*/         USHORT nPgNum = rpPage->GetPhyPageNum();
/*?*/         USHORT nIdx = 0;
/*?*/         USHORT nCnt = pCache->GetFlyCount();
/*?*/         ULONG nOrdNum = pFly->GetVirtDrawObj()->GetOrdNum();
/*?*/         SwFlyCache* pFlyC;
/*?*/ 
/*?*/         // skip fly frames from pages before the current page
/*?*/         while( nIdx < nCnt &&
/*?*/                nPgNum > (pFlyC = pCache->GetFlyCache( nIdx ))->nPageNum )
/*?*/             ++nIdx;
/*?*/ 
/*?*/         while( nIdx < nCnt &&
/*?*/                nOrdNum != (pFlyC = pCache->GetFlyCache( nIdx ))->nOrdNum )
/*?*/             ++nIdx;
/*?*/         if( nIdx < nCnt )
/*?*/         {
/*?*/             SwPageFrm *pPage = rpPage;
/*?*/             while( pPage && pPage->GetPhyPageNum() < pFlyC->nPageNum )
/*?*/                 pPage = (SwPageFrm*)pPage->GetNext();
/*?*/             if( pPage )
/*?*/             {
/*?*/                 rpPage = pPage;
/*?*/                 pFly->Frm().Pos().X() = pFlyC->Left() + pPage->Frm().Left();
/*?*/                 pFly->Frm().Pos().Y() = pFlyC->Top() + pPage->Frm().Top();
/*?*/                 if ( pCache->IsUseFlyCache() )
/*?*/                 {
/*?*/                     pFly->Frm().Width( pFlyC->Width() );
/*?*/                     pFly->Frm().Height( pFlyC->Height() );
/*?*/                 }
/*?*/                 bRet = TRUE;
/*?*/             }
/*?*/         }
/*?*/         pDoc->GetLayoutCache()->UnlockImpl();
/*N*/     }
/*N*/     return bRet;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SwLayCacheIoImpl::SwLayCacheIoImpl( SvStream& rStrm, BOOL bWrtMd ) :
/*N*/ 	pStream( &rStrm ),
/*N*/ 	nMajorVersion(SW_LAYCACHE_IO_VERSION_MAJOR),
/*N*/ 	nMinorVersion(SW_LAYCACHE_IO_VERSION_MINOR),
/*N*/ 	bWriteMode( bWrtMd ),
/*N*/ 	bError( FALSE  )
/*N*/ {
/*N*/ 	if( bWriteMode )
/*N*/ 		*pStream << nMajorVersion
/*N*/ 				 <<	nMinorVersion;
/*N*/ 
/*N*/ 	else
/*N*/ 		*pStream >> nMajorVersion
/*N*/ 				 >>	nMinorVersion;
/*N*/ }

/*N*/ BOOL SwLayCacheIoImpl::OpenRec( BYTE cType )
/*N*/ {
/*N*/ 	BOOL bRes = TRUE;
/*N*/ 	UINT16 nLvl = aRecTypes.Count();
/*N*/ 	ASSERT( nLvl == aRecSizes.Count(), "OpenRec: Level" );
/*N*/ 	UINT32 nPos = pStream->Tell();
/*N*/ 	if( bWriteMode )
/*N*/ 	{
/*N*/ 		aRecTypes.Insert( cType, nLvl );
/*N*/ 		aRecSizes.Insert( nPos, nLvl );
/*N*/ 		*pStream << (UINT32) 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		UINT32 nVal;
/*N*/ 		*pStream >> nVal;
/*N*/ 		BYTE cRecTyp = (BYTE)nVal;
/*N*/ 		aRecTypes.Insert( cRecTyp, nLvl );
/*N*/ 		sal_uInt32 nSize = nVal >> 8;
/*N*/ 		aRecSizes.Insert( nPos + nSize, nLvl );
/*N*/ 		if( !nVal || cRecTyp != cType ||
/*N*/ 			pStream->GetErrorCode() != SVSTREAM_OK || pStream->IsEof() )
/*N*/ 		{
/*?*/ 			ASSERT( nVal, "OpenRec: Record-Header is 0" );
/*?*/ 			ASSERT( cRecTyp == cType,
/*?*/ 					"OpenRec: Wrong Record Type" );
/*?*/ 			aRecTypes[nLvl] = 0;
/*?*/ 			aRecSizes[nLvl] = pStream->Tell();
/*?*/ 			bRes = sal_False;
/*?*/ 			bError = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRes;
/*N*/ }

// Close record

/*N*/ BOOL SwLayCacheIoImpl::CloseRec( BYTE cType )
/*N*/ {
/*N*/ 	BOOL bRes = TRUE;
/*N*/ 	UINT16 nLvl = aRecTypes.Count();
/*N*/ 	ASSERT( nLvl == aRecSizes.Count(), "CloseRec: wrong Level" );
/*N*/ 	ASSERT( nLvl, "CloseRec: no levels" );
/*N*/ 	if( nLvl )
/*N*/ 	{
/*N*/ 		nLvl--;
/*N*/ 		ASSERT( cType == aRecTypes[nLvl],
/*N*/ 				"CloseRec: Wrong Block-Header" );
/*N*/ 		UINT32 nPos = pStream->Tell();
/*N*/ 		if( bWriteMode )
/*N*/ 		{
/*N*/ 			UINT32 nBgn = aRecSizes[nLvl];
/*N*/ 			pStream->Seek( nBgn );
/*N*/ 			UINT32 nSize = nPos - nBgn;
/*N*/ 			UINT32 nVal = ( nSize << 8 ) | aRecTypes[nLvl];
/*N*/ 			*pStream << nVal;
/*N*/ 			pStream->Seek( nPos );
/*N*/ 			if( pStream->GetError() != SVSTREAM_OK )
/*?*/ 			 	bRes = FALSE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			UINT32 n = aRecSizes[nLvl];
/*N*/ 			ASSERT( n >= nPos, "CloseRec: to much data read" );
/*N*/ 			if( n != nPos )
/*N*/ 			{
/*?*/ 				pStream->Seek( n );
/*?*/ 				if( n < nPos )
/*?*/ 				 	bRes = FALSE;
/*N*/ 			}
/*N*/ 			if( pStream->GetErrorCode() != SVSTREAM_OK )
/*?*/ 				bRes = FALSE;
/*N*/ 		}
/*N*/ 
/*N*/ 		aRecTypes.Remove( nLvl, 1 );
/*N*/ 		aRecSizes.Remove( nLvl, 1 );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !bRes )
/*?*/ 		bError = TRUE;
/*N*/ 
/*N*/ 	return bRes;
/*N*/ }

/*N*/ UINT32 SwLayCacheIoImpl::BytesLeft()
/*N*/ {
/*N*/ 	UINT16 nLvl = aRecSizes.Count();
/*N*/ 	UINT32 n = 0;
/*N*/ 	if( !bError && nLvl )
/*N*/ 	{
/*N*/ 		UINT32 nEndPos = aRecSizes[ nLvl-1 ];
/*N*/ 		UINT32 nPos = pStream->Tell();
/*N*/ 		if( nEndPos > nPos )
/*N*/ 			n = nEndPos - nPos;
/*N*/ 	}
/*N*/ 
/*N*/ 	return n;
/*N*/ }

/*N*/ BYTE SwLayCacheIoImpl::Peek()
/*N*/ {
/*N*/ 	BYTE c = 0;
/*N*/ 	if( !bError )
/*N*/ 	{
/*N*/ 		UINT32 nPos = pStream->Tell();
/*N*/ 		*pStream >> c;
/*N*/ 		pStream->Seek( nPos );
/*N*/ 		if( pStream->GetErrorCode() != SVSTREAM_OK )
/*N*/ 		{
/*?*/ 			c = 0;
/*?*/ 			bError = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return c;
/*N*/ }


/*N*/ BYTE SwLayCacheIoImpl::OpenFlagRec()
/*N*/ {
/*N*/ 	ASSERT( !bWriteMode, "OpenFlagRec illegal in write  mode" );
/*N*/ 	BYTE cFlags;
/*N*/ 	*pStream >> cFlags;
/*N*/ 	nFlagRecEnd = pStream->Tell() + ( cFlags & 0x0F );
/*N*/ 	return (cFlags >> 4);
/*N*/ }

/*N*/ void SwLayCacheIoImpl::OpenFlagRec( BYTE nFlags, BYTE nLen )
/*N*/ {
/*N*/ 	ASSERT( bWriteMode, "OpenFlagRec illegal in read  mode" );
/*N*/ 	ASSERT( (nFlags & 0xF0) == 0, "illegal flags set" );
/*N*/ 	ASSERT( nLen < 16, "wrong flag record length" );
/*N*/ 	BYTE cFlags = (nFlags << 4) + nLen;
/*N*/ 	*pStream << cFlags;
/*N*/ 	nFlagRecEnd = pStream->Tell() + nLen;
/*N*/ }

/*N*/ void SwLayCacheIoImpl::CloseFlagRec()
/*N*/ {
/*N*/ 	if( bWriteMode )
/*N*/ 	{
/*N*/ 		ASSERT( pStream->Tell() == nFlagRecEnd, "Wrong amount of data written" );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ASSERT( pStream->Tell() <= nFlagRecEnd, "To many data read" );
/*N*/ 		if( pStream->Tell() != nFlagRecEnd )
/*?*/ 			pStream->Seek( nFlagRecEnd );
/*N*/ 	}
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
