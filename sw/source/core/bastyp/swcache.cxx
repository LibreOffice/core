/*************************************************************************
 *
 *  $RCSfile: swcache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-06 09:44:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#include "errhdl.hxx"
#include "swcache.hxx"

#ifndef PRODUCT
    #include <stdlib.h>         // getenv()
    #include <stdio.h>          // sprintf()
#endif


SV_IMPL_PTRARR(SwCacheObjArr,SwCacheObj*);

#ifdef PRODUCT
#define INCREMENT( nVar )
#else
#define INCREMENT( nVar )   ++nVar
#endif

/*************************************************************************
|*
|*  SwCache::Check()
|*
|*  Ersterstellung      MA 23. Mar. 94
|*  Letzte Aenderung    MA 23. Mar. 94
|*
|*************************************************************************/

#ifndef PRODUCT

void SwCache::Check()
{
    if ( !pRealFirst )
        return;

    //Konsistenspruefung.
    ASSERT( !pLast->GetNext(), "Last but not last." );
    ASSERT( !pRealFirst->GetPrev(), "First but not first." );
    USHORT nCnt = 0;
    BOOL bFirstFound = FALSE;
    SwCacheObj *pObj = pRealFirst;
    SwCacheObj *pRekursive = pObj;
    while ( pObj )
    {
        //Das Objekt muss auch auf dem Rueckwaertsweg gefunden werden.
        SwCacheObj *pTmp = pLast;
        while ( pTmp && pTmp != pObj )
            pTmp = pTmp->GetPrev();
        ASSERT( pTmp, "Objekt not found." );

        ++nCnt;
        if ( pObj == pFirst )
            bFirstFound = TRUE;
        if ( !pObj->GetNext() )
            ASSERT( pObj == pLast, "Last not Found." );
        pObj = pObj->GetNext();
        ASSERT( pObj != pRekursive, "Recursion in SwCache." );
    }
    ASSERT( bFirstFound, "First not Found." );
    ASSERT( (nCnt + aFreePositions.Count()) == Count(), "Lost Chain." );
    if ( Count() == nCurMax )
        ASSERT( (nCurMax - nCnt) == aFreePositions.Count(), "Lost FreePositions." );
}
#endif

#if !defined(PRODUCT) && defined(MADEBUG)
#define CHECK Check();
#else
#define CHECK
#endif

/*************************************************************************
|*
|*  SwCache::SwCache(), ~SwCache()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 15. Mar. 94
|*
|*************************************************************************/


SwCache::SwCache( const USHORT nInitSize, const USHORT nGrowSize
#ifndef PRODUCT
    , const ByteString &rNm
#endif
    ) :
    SwCacheObjArr( (BYTE)nInitSize, (BYTE)nGrowSize ),
    aFreePositions( 5, 5 ),
    nMax( nInitSize ),
    nCurMax( nInitSize ),
    pRealFirst( 0 ),
    pFirst( 0 ),
    pLast( 0 )
#ifndef PRODUCT
    , aName( rNm ),
    nAppend( 0 ),
    nInsertFree( 0 ),
    nReplace( 0 ),
    nGetSuccess( 0 ),
    nGetFail( 0 ),
    nToTop( 0 ),
    nDelete( 0 ),
    nGetSeek( 0 ),
    nAverageSeekCnt( 0 ),
    nFlushCnt( 0 ),
    nFlushedObjects( 0 ),
    nIncreaseMax( 0 ),
    nDecreaseMax( 0 )
#endif
{
}

#ifndef PRODUCT



SwCache::~SwCache()
{
#ifndef MAC
    static USHORT nOpenMode = STREAM_WRITE | STREAM_TRUNC;
    String sExt(String::CreateFromAscii(".log"));
    utl::TempFile aTempFile(String::CreateFromAscii("swcache"), &sExt);
    SvFileStream aStream( aTempFile.GetFileName(), nOpenMode );
    nOpenMode = STREAM_WRITE;

    if( !aStream.GetError() )
    {
        aStream.Seek( STREAM_SEEK_TO_END );
        ByteString sOut( aName ); sOut += '\n';
        (( sOut += "Anzahl neuer Eintraege:             " ) += nAppend )+= '\n';
        (( sOut += "Anzahl Insert auf freie Plaetze:    " ) += nInsertFree )+= '\n';
        (( sOut += "Anzahl Ersetzungen:                 " ) += nReplace )+= '\n';
        (( sOut += "Anzahl Erfolgreicher Get's:         " ) += nGetSuccess )+= '\n';
        (( sOut += "Anzahl Fehlgeschlagener Get's:      " ) += nGetFail )+= '\n';
        (( sOut += "Anzahl Umsortierungen (LRU):        " ) += nToTop )+= '\n';
        (( sOut += "Anzahl Loeschungen:                 " ) += nDelete )+= '\n';
        (( sOut += "Anzahl Get's ohne Index:            " ) += nGetSeek )+= '\n';
        (( sOut += "Anzahl Seek fuer Get ohne Index:    " ) += nAverageSeekCnt )+= '\n';
        (( sOut += "Anzahl Flush-Aufrufe:               " ) += nFlushCnt )+= '\n';
        (( sOut += "Anzahl geflush'ter Objekte:         " ) += nFlushedObjects )+= '\n';
        (( sOut += "Anzahl Cache-Erweiterungen:         " ) += nIncreaseMax )+= '\n';
        (( sOut += "Anzahl Cache-Verkleinerungen:       " ) += nDecreaseMax )+= '\n';

        aStream << sOut.GetBuffer()
                << "-------------------------------------------------------"
                << endl;
    }
    Check();
#endif
}
#endif

/*************************************************************************
|*
|*  SwCache::Flush()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 15. Mar. 94
|*
|*************************************************************************/


void SwCache::Flush( const BYTE nPercent )
{
    ASSERT( nPercent == 100, "SwCache::Flush() arbeitet nur 100%'ig" );

    INCREMENT( nFlushCnt );
    SwCacheObj *pObj = pRealFirst;
    pRealFirst = pFirst = pLast = 0;
    SwCacheObj *pTmp;
    while ( pObj )
    {
#ifndef PRODUCT
        if ( pObj->IsLocked() )
        {
            ASSERT( TRUE, "Flushing locked objects." );
            if ( !pRealFirst )
            {
                pRealFirst = pFirst = pLast = pObj;
                pTmp = pObj->GetNext();
                pObj->SetNext( 0 ); pObj->SetPrev( 0 );
                pObj = pTmp;
            }
            else
            {   pLast->SetNext( pObj );
                pObj->SetPrev( pLast );
                pLast = pObj;
                pTmp = pObj->GetNext();
                pObj->SetNext( 0 );
                pObj = pTmp;
            }
        }
        else
#endif
        {
            pTmp = (SwCacheObj*)pObj;
            pObj = pTmp->GetNext();
            aFreePositions.Insert( pTmp->GetCachePos(), aFreePositions.Count() );
            *(pData + pTmp->GetCachePos()) = (void*)0;
            delete pTmp;
            INCREMENT( nFlushedObjects );
        }
    }
}

/*************************************************************************
|*
|*  SwCache::ToTop()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 24. Apr. 95
|*
|*************************************************************************/


void SwCache::ToTop( SwCacheObj *pObj )
{
    INCREMENT( nToTop );

    //Objekt aus der LRU-Kette ausschneiden und am Anfang einfuegen.
    if ( pRealFirst == pObj )   //pFirst wurde vom Aufrufer geprueft!
    {   CHECK;
        return;
    }

    if ( !pRealFirst )
    {   //Der erste wird eingetragen.
        ASSERT( !pFirst && !pLast, "First not first." );
        pRealFirst = pFirst = pLast = pObj;
        CHECK;
        return;
    }

    //Ausschneiden.
    if ( pObj == pLast )
    {
        ASSERT( pObj->GetPrev(), "Last but no Prev." );
        pLast = pObj->GetPrev();
        pLast->SetNext( 0 );
    }
    else
    {
        if ( pObj->GetNext() )
            pObj->GetNext()->SetPrev( pObj->GetPrev() );
        if ( pObj->GetPrev() )
            pObj->GetPrev()->SetNext( pObj->GetNext() );
    }

    //Am (virtuellen) Anfang einfuegen.
    if ( pRealFirst == pFirst )
    {
        pRealFirst->SetPrev( pObj );
        pObj->SetNext( pRealFirst );
        pObj->SetPrev( 0 );
        pRealFirst = pFirst = pObj;
        CHECK;
    }
    else
    {
        ASSERT( pFirst, "ToTop, First ist not RealFirst an Empty." );

        if ( pFirst->GetPrev() )
        {
            pFirst->GetPrev()->SetNext( pObj );
            pObj->SetPrev( pFirst->GetPrev() );
        }
        else
            pObj->SetPrev( 0 );
        pFirst->SetPrev( pObj );
        pObj->SetNext( pFirst );
        pFirst = pObj;
        CHECK;
    }
}

/*************************************************************************
|*
|*  SwCache::Get()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 22. Aug. 94
|*
|*************************************************************************/


SwCacheObj *SwCache::Get( const void *pOwner, const USHORT nIndex,
                          const BOOL bToTop )
{
    SwCacheObj *pRet;
    if ( 0 != (pRet = nIndex < Count() ? operator[]( nIndex ) : 0) )
    {
        if ( !pRet->IsOwner( pOwner ) )
            pRet = 0;
        else if ( bToTop && pRet != pFirst )
            ToTop( pRet );
    }

#ifndef PRODUCT
        if ( pRet )
            ++nGetSuccess;
        else
            ++nGetFail;
#endif

    return pRet;
}



SwCacheObj *SwCache::Get( const void *pOwner, const BOOL bToTop )
{
    SwCacheObj *pRet = pRealFirst;
    while ( pRet && !pRet->IsOwner( pOwner ) )
    {
        INCREMENT( nAverageSeekCnt );
        pRet = pRet->GetNext();
    }

    if ( bToTop && pRet && pRet != pFirst )
        ToTop( pRet );

#ifndef PRODUCT
    if ( pRet )
        ++nGetSuccess;
    else
        ++nGetFail;
    ++nGetSeek;
#endif
    return pRet;
}

/*************************************************************************
|*
|*  SwCache::Delete()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 15. Mar. 94
|*
|*************************************************************************/


void SwCache::DeleteObj( SwCacheObj *pObj )
{
    CHECK;
    ASSERT( !pObj->IsLocked(), "SwCache::Delete: Object ist Locked." );
    if ( pObj->IsLocked() )
        return;

    if ( pFirst == pObj )
    {
        if ( pFirst->GetNext() )
            pFirst = pFirst->GetNext();
        else
            pFirst = pFirst->GetPrev();
    }
    if ( pRealFirst == pObj )
        pRealFirst = pRealFirst->GetNext();
    if ( pLast == pObj )
        pLast = pLast->GetPrev();
    if ( pObj->GetPrev() )
        pObj->GetPrev()->SetNext( pObj->GetNext() );
    if ( pObj->GetNext() )
        pObj->GetNext()->SetPrev( pObj->GetPrev() );

    aFreePositions.Insert( pObj->GetCachePos(), aFreePositions.Count() );
    *(pData + pObj->GetCachePos()) = (void*)0;
    delete pObj;

    CHECK;
    if ( Count() > nCurMax &&
         (nCurMax <= (Count() - aFreePositions.Count())) )
    {
        //Falls moeglich wieder verkleinern, dazu muessen allerdings ausreichend
        //Freie Positionen bereitstehen.
        //Unangenehmer Nebeneffekt ist, das die Positionen verschoben werden
        //muessen, und die Eigentuemer der Objekte diese wahrscheinlich nicht
        //wiederfinden werden.
        for ( USHORT i = 0; i < Count(); ++i )
        {
            SwCacheObj *pObj = operator[](i);
            if ( !pObj )
            {   SwCacheObjArr::Remove( i, 1 );
                --i;
            }
            else
                pObj->SetCachePos( i );
        }
        aFreePositions.Remove( 0, aFreePositions.Count() );
    }
    CHECK;
}

/*


void SwCache::Delete( const void *pOwner, const USHORT nIndex )
{
    INCREMENT( nDelete );
    SwCacheObj *pObj;
    if ( 0 != (pObj = Get( pOwner, nIndex, FALSE )) )
        DeleteObj( pObj );
}
*/



void SwCache::Delete( const void *pOwner )
{
    INCREMENT( nDelete );
    SwCacheObj *pObj;
    if ( 0 != (pObj = Get( pOwner, BOOL(FALSE) )) )
        DeleteObj( pObj );
}


/*************************************************************************
|*
|*  SwCache::Insert()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 20. Sep. 94
|*
|*************************************************************************/


BOOL SwCache::Insert( SwCacheObj *pNew )
{
    CHECK;
    ASSERT( !pNew->GetPrev() && !pNew->GetNext(), "New but not new." );

    USHORT nPos;//Wird hinter den if's zum setzen am Obj benutzt.
    if ( Count() < nCurMax )
    {
        //Es ist noch Platz frei, also einfach einfuegen.
        INCREMENT( nAppend );
        nPos = Count();
        SwCacheObjArr::C40_INSERT( SwCacheObj, pNew, nPos );
    }
    else if ( aFreePositions.Count() )
    {
        //Es exitieren Platzhalter, also den letzten benutzen.
        INCREMENT( nInsertFree );
        const USHORT nFreePos = aFreePositions.Count() - 1;
        nPos = aFreePositions[ nFreePos ];
        *(pData + nPos) = pNew;
        aFreePositions.Remove( nFreePos );
    }
    else
    {
        INCREMENT( nReplace );
        //Der letzte des LRU fliegt raus.
        SwCacheObj *pObj = pLast;

        while ( pObj && pObj->IsLocked() )
            pObj = pObj->GetPrev();
        if ( !pObj )
        {
            ASSERT( FALSE, "Cache overflow." );
            return FALSE;
        }

        nPos = pObj->GetCachePos();
        if ( pObj == pLast )
        {   ASSERT( pObj->GetPrev(), "Last but no Prev" );
            pLast = pObj->GetPrev();
            pLast->SetNext( 0 );
        }
        else
        {
            if ( pObj->GetPrev() )
                pObj->GetPrev()->SetNext( pObj->GetNext() );
            if ( pObj->GetNext() )
                pObj->GetNext()->SetPrev( pObj->GetPrev() );
        }
        delete pObj;
        *(pData + nPos) = pNew;
    }
    pNew->SetCachePos( nPos );

    //Anstelle von ToTop, einfach als pFirst einfuegen.
//  ToTop( nPos );
    if ( pFirst )
    {
        if ( pFirst->GetPrev() )
        {   pFirst->GetPrev()->SetNext( pNew );
            pNew->SetPrev( pFirst->GetPrev() );
        }
        pFirst->SetPrev( pNew );
        pNew->SetNext( pFirst );
    }
    else
    {   ASSERT( !pLast, "Last but no First." );
        pLast = pNew;
    }
    if ( pFirst == pRealFirst )
        pRealFirst = pNew;
    pFirst = pNew;

    CHECK;
    return TRUE;
}

/*************************************************************************
|*
|*  SwCache::SetLRUOfst()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 15. Mar. 94
|*
|*************************************************************************/


void SwCache::SetLRUOfst( const USHORT nOfst )
{
    if ( !pRealFirst || ((Count() - aFreePositions.Count()) < nOfst) )
        return;

    CHECK;
    pFirst = pRealFirst;
    for ( USHORT i = 0; i < Count() && i < nOfst; ++i )
    {
        if ( pFirst->GetNext() && pFirst->GetNext()->GetNext() )
            pFirst = pFirst->GetNext();
        else
            break;
    }
    CHECK;
}

/*************************************************************************
|*
|*  SwCacheObj::SwCacheObj()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 24. Nov. 95
|*
|*************************************************************************/


SwCacheObj::SwCacheObj( const void *pOwn ) :
    nLock( 0 ),
    nCachePos( USHRT_MAX ),
    pNext( 0 ),
    pPrev( 0 ),
    pOwner( pOwn )
{
}



SwCacheObj::~SwCacheObj()
{
}

/*************************************************************************
|*
|*  SwCacheObj::SetLock(), Unlock()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 15. Mar. 94
|*
|*************************************************************************/

#ifndef PRODUCT



void SwCacheObj::Lock()
{
    ASSERT( nLock < UCHAR_MAX, "To many Locks for CacheObject." );
    ++nLock;
}



void SwCacheObj::Unlock()
{
    ASSERT( nLock, "No more Locks available." );
    --nLock;
}
#endif

/*************************************************************************
|*
|*  SwCacheAccess::Get()
|*
|*  Ersterstellung      MA 15. Mar. 94
|*  Letzte Aenderung    MA 04. Apr. 95
|*
|*************************************************************************/


void SwCacheAccess::_Get()
{
    ASSERT( !pObj, "SwCacheAcces Obj already available." );

    pObj = NewObj();
    if ( !rCache.Insert( pObj ) )
    {
        delete pObj;
        pObj = 0;
    }
    else
        pObj->Lock();
}

/*************************************************************************
|*
|*  SwCacheAccess::IsAvailable()
|*
|*  Ersterstellung      MA 23. Mar. 94
|*  Letzte Aenderung    MA 23. Mar. 94
|*
|*************************************************************************/


BOOL SwCacheAccess::IsAvailable() const
{
    return pObj != 0;
}





