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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _SWCACHE_HXX
#include <swcache.hxx>
#endif
namespace binfilter {

/*N*/ SV_IMPL_PTRARR(SwCacheObjArr,SwCacheObj*);

#ifndef DBG_UTIL
#define INCREMENT( nVar )
#else
#define INCREMENT( nVar )	++nVar
#endif

/*************************************************************************
|*
|*	SwCache::Check()
|*
|*	Ersterstellung		MA 23. Mar. 94
|*	Letzte Aenderung	MA 23. Mar. 94
|*
|*************************************************************************/

#ifdef DBG_UTIL

/*N*/ void SwCache::Check()
/*N*/ {
/*N*/ 	if ( !pRealFirst )
/*N*/ 		return;
/*N*/ 
/*N*/ 	//Konsistenspruefung.
/*N*/ 	ASSERT( !pLast->GetNext(), "Last but not last." );
/*N*/ 	ASSERT( !pRealFirst->GetPrev(), "First but not first." );
/*N*/ 	USHORT nCnt = 0;
/*N*/ 	BOOL bFirstFound = FALSE;
/*N*/ 	SwCacheObj *pObj = pRealFirst;
/*N*/ 	SwCacheObj *pRekursive = pObj;
/*N*/ 	while ( pObj )
/*N*/ 	{
/*N*/ 		//Das Objekt muss auch auf dem Rueckwaertsweg gefunden werden.
/*N*/ 		SwCacheObj *pTmp = pLast;
/*N*/ 		while ( pTmp && pTmp != pObj )
/*N*/ 			pTmp = pTmp->GetPrev();
/*N*/ 		ASSERT( pTmp, "Objekt not found." );
/*N*/ 
/*N*/ 		++nCnt;
/*N*/ 		if ( pObj == pFirst )
/*N*/ 			bFirstFound = TRUE;
/*N*/ 		if ( !pObj->GetNext() )
/*N*/ 			ASSERT( pObj == pLast, "Last not Found." );
/*N*/ 		pObj = pObj->GetNext();
/*N*/ 		ASSERT( pObj != pRekursive, "Recursion in SwCache." );
/*N*/ 	}
/*N*/ 	ASSERT( bFirstFound, "First not Found." );
/*N*/ 	ASSERT( (nCnt + aFreePositions.Count()) == Count(), "Lost Chain." );
/*N*/ 	if ( Count() == nCurMax )
/*N*/ 		ASSERT( (nCurMax - nCnt) == aFreePositions.Count(), "Lost FreePositions." );
/*N*/ }
#endif

#if defined(DBG_UTIL) && defined(MADEBUG)
#define CHECK Check();
#else
#define CHECK
#endif

/*************************************************************************
|*
|*	SwCache::SwCache(), ~SwCache()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 15. Mar. 94
|*
|*************************************************************************/


/*N*/ SwCache::SwCache( const USHORT nInitSize, const USHORT nGrowSize
/*N*/ #ifdef DBG_UTIL
/*N*/ 	, const ByteString &rNm
/*N*/ #endif
/*N*/ 	) :
/*N*/ 	SwCacheObjArr( (BYTE)nInitSize, (BYTE)nGrowSize ),
/*N*/ 	aFreePositions( 5, 5 ),
/*N*/ 	nMax( nInitSize ),
/*N*/ 	nCurMax( nInitSize ),
/*N*/ 	pRealFirst( 0 ),
/*N*/ 	pFirst( 0 ),
/*N*/ 	pLast( 0 )
/*N*/ #ifdef DBG_UTIL
/*N*/ 	, aName( rNm ),
/*N*/ 	nAppend( 0 ),
/*N*/ 	nInsertFree( 0 ),
/*N*/ 	nReplace( 0 ),
/*N*/ 	nGetSuccess( 0 ),
/*N*/ 	nGetFail( 0 ),
/*N*/ 	nToTop( 0 ),
/*N*/ 	nDelete( 0 ),
/*N*/ 	nGetSeek( 0 ),
/*N*/ 	nAverageSeekCnt( 0 ),
/*N*/ 	nFlushCnt( 0 ),
/*N*/ 	nFlushedObjects( 0 ),
/*N*/ 	nIncreaseMax( 0 ),
/*N*/ 	nDecreaseMax( 0 )
/*N*/ #endif
/*N*/ {
/*N*/ }

#ifdef DBG_UTIL


/*N*/ SwCache::~SwCache()
/*N*/ {
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ #ifndef MAC
/*N*/ 	{
/*N*/ 		ByteString sOut( aName ); sOut += '\n';
/*N*/ 		(( sOut += "Anzahl neuer Eintraege:				" )
/*N*/ 					+= ByteString::CreateFromInt32( nAppend ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Insert auf freie Plaetze:	" )
/*N*/ 					+= ByteString::CreateFromInt32( nInsertFree ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Ersetzungen:					" )
/*N*/ 					+= ByteString::CreateFromInt32( nReplace ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Erfolgreicher Get's:			" )
/*N*/ 					+= ByteString::CreateFromInt32( nGetSuccess ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Fehlgeschlagener Get's:		" )
/*N*/ 					+= ByteString::CreateFromInt32( nGetFail ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Umsortierungen (LRU):		" )
/*N*/ 					+= ByteString::CreateFromInt32( nToTop ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Loeschungen:					" )
/*N*/ 					+= ByteString::CreateFromInt32( nDelete ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Get's ohne Index:			" )
/*N*/ 					+= ByteString::CreateFromInt32( nGetSeek ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Seek fuer Get ohne Index:	" )
/*N*/ 					+= ByteString::CreateFromInt32( nAverageSeekCnt ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Flush-Aufrufe:				" )
/*N*/ 					+= ByteString::CreateFromInt32( nFlushCnt ))+= '\n';
/*N*/ 		(( sOut += "Anzahl geflush'ter Objekte:			" )
/*N*/ 					+= ByteString::CreateFromInt32( nFlushedObjects ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Cache-Erweiterungen:			" )
/*N*/ 					+= ByteString::CreateFromInt32( nIncreaseMax ))+= '\n';
/*N*/ 		(( sOut += "Anzahl Cache-Verkleinerungen:		" )
/*N*/ 					+= ByteString::CreateFromInt32( nDecreaseMax ))+= '\n';
/*N*/ 
/*N*/ 		DBG_ERROR( sOut.GetBuffer() );
/*N*/ 	}
/*N*/ 	Check();
/*N*/ #endif
/*N*/ #endif
/*N*/ }
#endif

/*************************************************************************
|*
|*	SwCache::Flush()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 15. Mar. 94
|*
|*************************************************************************/


/*N*/ void SwCache::Flush( const BYTE nPercent )
/*N*/ {
/*N*/ 	ASSERT( nPercent == 100, "SwCache::Flush() arbeitet nur 100%'ig" );
/*N*/ 
/*N*/ 	INCREMENT( nFlushCnt );
/*N*/ 	SwCacheObj *pObj = pRealFirst;
/*N*/ 	pRealFirst = pFirst = pLast = 0;
/*N*/ 	SwCacheObj *pTmp;
/*N*/ 	while ( pObj )
/*N*/ 	{
/*N*/ #ifdef DBG_UTIL
/*N*/ 		if ( pObj->IsLocked() )
/*N*/ 		{
/*?*/ 			ASSERT( TRUE, "Flushing locked objects." );
/*?*/ 			if ( !pRealFirst )
/*?*/ 			{
/*?*/ 				pRealFirst = pFirst = pLast = pObj;
/*?*/ 				pTmp = pObj->GetNext();
/*?*/ 				pObj->SetNext( 0 ); pObj->SetPrev( 0 );
/*?*/ 				pObj = pTmp;
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{	pLast->SetNext( pObj );
/*?*/ 				pObj->SetPrev( pLast );
/*?*/ 				pLast = pObj;
/*?*/ 				pTmp = pObj->GetNext();
/*?*/ 				pObj->SetNext( 0 );
/*?*/ 				pObj = pTmp;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ #endif
/*N*/ 		{
/*N*/ 			pTmp = (SwCacheObj*)pObj;
/*N*/ 			pObj = pTmp->GetNext();
/*N*/ 			aFreePositions.Insert( pTmp->GetCachePos(), aFreePositions.Count() );
/*N*/ 			*(pData + pTmp->GetCachePos()) = (void*)0;
/*N*/ 			delete pTmp;
/*N*/ 			INCREMENT( nFlushedObjects );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwCache::ToTop()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 24. Apr. 95
|*
|*************************************************************************/


/*N*/ void SwCache::ToTop( SwCacheObj *pObj )
/*N*/ {
/*N*/ 	INCREMENT( nToTop );
/*N*/ 
/*N*/ 	//Objekt aus der LRU-Kette ausschneiden und am Anfang einfuegen.
/*N*/ 	if ( pRealFirst == pObj )	//pFirst wurde vom Aufrufer geprueft!
/*N*/ 	{	CHECK;
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !pRealFirst )
/*N*/ 	{	//Der erste wird eingetragen.
/*?*/ 		ASSERT( !pFirst && !pLast, "First not first." );
/*?*/ 		pRealFirst = pFirst = pLast = pObj;
/*?*/ 		CHECK;
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	//Ausschneiden.
/*N*/ 	if ( pObj == pLast )
/*N*/ 	{
/*N*/ 		ASSERT( pObj->GetPrev(), "Last but no Prev." );
/*N*/ 		pLast = pObj->GetPrev();
/*N*/ 		pLast->SetNext( 0 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( pObj->GetNext() )
/*N*/ 			pObj->GetNext()->SetPrev( pObj->GetPrev() );
/*N*/ 		if ( pObj->GetPrev() )
/*N*/ 			pObj->GetPrev()->SetNext( pObj->GetNext() );
/*N*/ 	}
/*N*/ 
/*N*/ 	//Am (virtuellen) Anfang einfuegen.
/*N*/ 	if ( pRealFirst == pFirst )
/*N*/ 	{
/*N*/ 		pRealFirst->SetPrev( pObj );
/*N*/ 		pObj->SetNext( pRealFirst );
/*N*/ 		pObj->SetPrev( 0 );
/*N*/ 		pRealFirst = pFirst = pObj;
/*N*/ 		CHECK;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ASSERT( pFirst, "ToTop, First ist not RealFirst an Empty." );
/*N*/ 
/*N*/ 		if ( pFirst->GetPrev() )
/*N*/ 		{
/*N*/ 			pFirst->GetPrev()->SetNext( pObj );
/*N*/ 			pObj->SetPrev( pFirst->GetPrev() );
/*N*/ 		}
/*N*/ 		else
/*?*/ 			pObj->SetPrev( 0 );
/*N*/ 		pFirst->SetPrev( pObj );
/*N*/ 		pObj->SetNext( pFirst );
/*N*/ 		pFirst = pObj;
/*N*/ 		CHECK;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwCache::Get()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 22. Aug. 94
|*
|*************************************************************************/


/*N*/ SwCacheObj *SwCache::Get( const void *pOwner, const USHORT nIndex,
/*N*/ 						  const BOOL bToTop )
/*N*/ {
/*N*/ 	SwCacheObj *pRet;
/*N*/ 	if ( 0 != (pRet = nIndex < Count() ? operator[]( nIndex ) : 0) )
/*N*/ 	{
/*N*/ 		if ( !pRet->IsOwner( pOwner ) )
/*N*/ 			pRet = 0;
/*N*/ 		else if ( bToTop && pRet != pFirst )
/*N*/ 			ToTop( pRet );
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 		if ( pRet )
/*N*/ 			++nGetSuccess;
/*N*/ 		else
/*N*/ 			++nGetFail;
/*N*/ #endif
/*N*/ 
/*N*/ 	return pRet;
/*N*/ }



/*N*/ SwCacheObj *SwCache::Get( const void *pOwner, const BOOL bToTop )
/*N*/ {
/*N*/ 	SwCacheObj *pRet = pRealFirst;
/*N*/ 	while ( pRet && !pRet->IsOwner( pOwner ) )
/*N*/ 	{
/*N*/ 		INCREMENT( nAverageSeekCnt );
/*N*/ 		pRet = pRet->GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bToTop && pRet && pRet != pFirst )
/*N*/ 		ToTop( pRet );
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if ( pRet )
/*N*/ 		++nGetSuccess;
/*N*/ 	else
/*N*/ 		++nGetFail;
/*N*/ 	++nGetSeek;
/*N*/ #endif
/*N*/ 	return pRet;
/*N*/ }

/*************************************************************************
|*
|*	SwCache::Delete()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 15. Mar. 94
|*
|*************************************************************************/


/*N*/ void SwCache::DeleteObj( SwCacheObj *pObj )
/*N*/ {
/*N*/ 	CHECK;
/*N*/ 	ASSERT( !pObj->IsLocked(), "SwCache::Delete: Object ist Locked." );
/*N*/ 	if ( pObj->IsLocked() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if ( pFirst == pObj )
/*N*/ 	{
/*N*/ 		if ( pFirst->GetNext() )
/*N*/ 			pFirst = pFirst->GetNext();
/*N*/ 		else
/*N*/ 			pFirst = pFirst->GetPrev();
/*N*/ 	}
/*N*/ 	if ( pRealFirst == pObj )
/*N*/ 		pRealFirst = pRealFirst->GetNext();
/*N*/ 	if ( pLast == pObj )
/*N*/ 		pLast = pLast->GetPrev();
/*N*/ 	if ( pObj->GetPrev() )
/*N*/ 		pObj->GetPrev()->SetNext( pObj->GetNext() );
/*N*/ 	if ( pObj->GetNext() )
/*N*/ 		pObj->GetNext()->SetPrev( pObj->GetPrev() );
/*N*/ 
/*N*/ 	aFreePositions.Insert( pObj->GetCachePos(), aFreePositions.Count() );
/*N*/ 	*(pData + pObj->GetCachePos()) = (void*)0;
/*N*/ 	delete pObj;
/*N*/ 
/*N*/ 	CHECK;
/*N*/ 	if ( Count() > nCurMax &&
/*N*/ 		 (nCurMax <= (Count() - aFreePositions.Count())) )
/*N*/ 	{
/*N*/ 		//Falls moeglich wieder verkleinern, dazu muessen allerdings ausreichend
/*N*/ 		//Freie Positionen bereitstehen.
/*N*/ 		//Unangenehmer Nebeneffekt ist, das die Positionen verschoben werden
/*N*/ 		//muessen, und die Eigentuemer der Objekte diese wahrscheinlich nicht
/*N*/ 		//wiederfinden werden.
/*?*/ 		for ( USHORT i = 0; i < Count(); ++i )
/*?*/ 		{
/*?*/ 			SwCacheObj *pObj = operator[](i);
/*?*/ 			if ( !pObj )
/*?*/ 			{	SwCacheObjArr::Remove( i, 1 );
/*?*/ 				--i;
/*?*/ 			}
/*?*/ 			else
/*?*/ 				pObj->SetCachePos( i );
/*?*/ 		}
/*?*/ 		aFreePositions.Remove( 0, aFreePositions.Count() );
/*N*/ 	}
/*N*/ 	CHECK;
/*N*/ }

/*


void SwCache::Delete( const void *pOwner, const USHORT nIndex )
{
    INCREMENT( nDelete );
    SwCacheObj *pObj;
    if ( 0 != (pObj = Get( pOwner, nIndex, FALSE )) )
        DeleteObj( pObj );
}
*/



/*N*/ void SwCache::Delete( const void *pOwner )
/*N*/ {
/*N*/ 	INCREMENT( nDelete );
/*N*/ 	SwCacheObj *pObj;
/*N*/ 	if ( 0 != (pObj = Get( pOwner, BOOL(FALSE) )) )
/*N*/ 		DeleteObj( pObj );
/*N*/ }


/*************************************************************************
|*
|*	SwCache::Insert()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 20. Sep. 94
|*
|*************************************************************************/


/*N*/ BOOL SwCache::Insert( SwCacheObj *pNew )
/*N*/ {
/*N*/ 	CHECK;
/*N*/ 	ASSERT( !pNew->GetPrev() && !pNew->GetNext(), "New but not new." );
/*N*/ 
/*N*/ 	USHORT nPos;//Wird hinter den if's zum setzen am Obj benutzt.
/*N*/ 	if ( Count() < nCurMax )
/*N*/ 	{
/*N*/ 		//Es ist noch Platz frei, also einfach einfuegen.
/*N*/ 		INCREMENT( nAppend );
/*N*/ 		nPos = Count();
/*N*/ 		SwCacheObjArr::C40_INSERT( SwCacheObj, pNew, nPos );
/*N*/ 	}
/*N*/ 	else if ( aFreePositions.Count() )
/*N*/ 	{
/*N*/ 		//Es exitieren Platzhalter, also den letzten benutzen.
/*N*/ 		INCREMENT( nInsertFree );
/*N*/ 		const USHORT nFreePos = aFreePositions.Count() - 1;
/*N*/ 		nPos = aFreePositions[ nFreePos ];
/*N*/ 		*(pData + nPos) = pNew;
/*N*/ 		aFreePositions.Remove( nFreePos );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		INCREMENT( nReplace );
/*N*/ 		//Der letzte des LRU fliegt raus.
/*N*/ 		SwCacheObj *pObj = pLast;
/*N*/ 
/*N*/ 		while ( pObj && pObj->IsLocked() )
/*N*/ 			pObj = pObj->GetPrev();
/*N*/ 		if ( !pObj )
/*N*/ 		{
/*N*/ 			ASSERT( FALSE, "Cache overflow." );
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 
/*N*/ 		nPos = pObj->GetCachePos();
/*N*/ 		if ( pObj == pLast )
/*N*/ 		{	ASSERT( pObj->GetPrev(), "Last but no Prev" );
/*N*/ 			pLast = pObj->GetPrev();
/*N*/ 			pLast->SetNext( 0 );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if ( pObj->GetPrev() )
/*N*/ 				pObj->GetPrev()->SetNext( pObj->GetNext() );
/*N*/ 			if ( pObj->GetNext() )
/*N*/ 				pObj->GetNext()->SetPrev( pObj->GetPrev() );
/*N*/ 		}
/*N*/ 		delete pObj;
/*N*/ 		*(pData + nPos) = pNew;
/*N*/ 	}
/*N*/ 	pNew->SetCachePos( nPos );
/*N*/ 
/*N*/ 	//Anstelle von ToTop, einfach als pFirst einfuegen.
/*N*/ //	ToTop( nPos );
/*N*/ 	if ( pFirst )
/*N*/ 	{
/*N*/ 		if ( pFirst->GetPrev() )
/*N*/ 		{	pFirst->GetPrev()->SetNext( pNew );
/*N*/ 			pNew->SetPrev( pFirst->GetPrev() );
/*N*/ 		}
/*N*/ 		pFirst->SetPrev( pNew );
/*N*/ 		pNew->SetNext( pFirst );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	ASSERT( !pLast, "Last but no First." );
/*N*/ 		pLast = pNew;
/*N*/ 	}
/*N*/ 	if ( pFirst == pRealFirst )
/*N*/ 		pRealFirst = pNew;
/*N*/ 	pFirst = pNew;
/*N*/ 
/*N*/ 	CHECK;
/*N*/ 	return TRUE;
/*N*/ }

/*************************************************************************
|*
|*	SwCache::SetLRUOfst()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 15. Mar. 94
|*
|*************************************************************************/


/*N*/ void SwCache::SetLRUOfst( const USHORT nOfst )
/*N*/ {
/*N*/ 	if ( !pRealFirst || ((Count() - aFreePositions.Count()) < nOfst) )
/*N*/ 		return;
/*N*/ 
/*N*/ 	CHECK;
/*N*/ 	pFirst = pRealFirst;
/*N*/ 	for ( USHORT i = 0; i < Count() && i < nOfst; ++i )
/*N*/ 	{
/*N*/ 		if ( pFirst->GetNext() && pFirst->GetNext()->GetNext() )
/*N*/ 			pFirst = pFirst->GetNext();
/*N*/ 		else
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	CHECK;
/*N*/ }

/*************************************************************************
|*
|*	SwCacheObj::SwCacheObj()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 24. Nov. 95
|*
|*************************************************************************/


/*N*/ SwCacheObj::SwCacheObj( const void *pOwn ) :
/*N*/ 	nLock( 0 ),
/*N*/ 	nCachePos( USHRT_MAX ),
/*N*/ 	pNext( 0 ),
/*N*/ 	pPrev( 0 ),
/*N*/ 	pOwner( pOwn )
/*N*/ {
/*N*/ }



/*N*/ SwCacheObj::~SwCacheObj()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	SwCacheObj::SetLock(), Unlock()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 15. Mar. 94
|*
|*************************************************************************/

#ifdef DBG_UTIL



/*N*/ void SwCacheObj::Lock()
/*N*/ {
/*N*/ 	ASSERT( nLock < UCHAR_MAX, "To many Locks for CacheObject." );
/*N*/ 	++nLock;
/*N*/ }



/*N*/ void SwCacheObj::Unlock()
/*N*/ {
/*N*/ 	ASSERT( nLock, "No more Locks available." );
/*N*/ 	--nLock;
/*N*/ }
#endif

/*************************************************************************
|*
|*	SwCacheAccess::Get()
|*
|*	Ersterstellung		MA 15. Mar. 94
|*	Letzte Aenderung	MA 04. Apr. 95
|*
|*************************************************************************/


/*N*/ void SwCacheAccess::_Get()
/*N*/ {
/*N*/ 	ASSERT( !pObj, "SwCacheAcces Obj already available." );
/*N*/ 
/*N*/ 	pObj = NewObj();
/*N*/ 	if ( !rCache.Insert( pObj ) )
/*N*/ 	{
/*?*/ 		delete pObj;
/*?*/ 		pObj = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pObj->Lock();
/*N*/ }

/*************************************************************************
|*
|*	SwCacheAccess::IsAvailable()
|*
|*	Ersterstellung		MA 23. Mar. 94
|*	Letzte Aenderung	MA 23. Mar. 94
|*
|*************************************************************************/







}
