/*************************************************************************
 *
 *  $RCSfile: iprcache.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:36 $
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

#include <string.h>

#include "iprcache.hxx"
#include "misc.hxx"

#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

//#define IPR_DEF_CACHE_SIZE        503
#define IPR_DEF_CACHE_MAX       375
#define IPR_DEF_CACHE_MAXINPUT  200

#ifdef DBG_STATISTIC

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

//#define IPR_CACHE_SIZE        nTblSize
#define IPR_CACHE_MAX       nMax
#define IPR_CACHE_MAXINPUT  nMaxInput

#else

//#define IPR_CACHE_SIZE        IPR_DEF_CACHE_SIZE
#define IPR_CACHE_MAX       IPR_DEF_CACHE_MAX
#define IPR_CACHE_MAXINPUT  IPR_DEF_CACHE_MAXINPUT

#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

FlushListener::FlushListener( Flushable *pFO )
{
    SetFlushObj( pFO );
}

FlushListener::~FlushListener()
{
}

void FlushListener::SetDicList( Reference<XDictionaryList> &xDL )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (xDicList != xDL)
    {
        if (xDicList.is())
            xDicList->removeDictionaryListEventListener( this );

        xDicList = xDL;
        if (xDicList.is())
            xDicList->addDictionaryListEventListener( this, FALSE );
    }
}

void SAL_CALL FlushListener::disposing( const EventObject& rSource )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (xDicList.is()  &&  rSource.Source == xDicList)
    {
        xDicList->removeDictionaryListEventListener( this );
        xDicList = NULL;    //! release reference
    }
}

void SAL_CALL FlushListener::processDictionaryListEvent(
    const DictionaryListEvent& rDicListEvent )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (rDicListEvent.Source == xDicList)
    {
        INT16 nEvt = rDicListEvent.nCondensedEvent;
        BOOL bFlush =      ( nEvt & DictionaryListEventFlags::ADD_NEG_ENTRY )
                        || ( nEvt & DictionaryListEventFlags::DEL_POS_ENTRY )
                        || ( nEvt & DictionaryListEventFlags::ACTIVATE_NEG_DIC )
                        || ( nEvt & DictionaryListEventFlags::DEACTIVATE_POS_DIC );

        DBG_ASSERT( pFlushObj, "lng : missing object (NULL pointer)" );
        if (bFlush && pFlushObj != NULL)
            pFlushObj->Flush();
    }
}

///////////////////////////////////////////////////////////////////////////

class IPRCachedWord
{
    String           aWord;
    IPRCachedWord   *pNext;
    IPRCachedWord   *pPrev;
    IPRCachedWord   *pFollow;
    INT16            nLanguage;
    ULONG            nFound;

    // don't allow to use copy-constructor and assignment-operator
    IPRCachedWord(const IPRCachedWord &);
    IPRCachedWord & operator = (const IPRCachedWord &);

public:
    IPRCachedWord( const String& rWord, IPRCachedWord* pFollow, INT16 nLang )
        : aWord( rWord ), pPrev( NULL ), pFollow( pFollow ),
          nLanguage( nLang ), nFound( 0 ) {}
    ~IPRCachedWord(){}

    const String&   GetWord()                       { return aWord; }
    void            SetWord( const String& aNew )   { aWord = aNew; }

    USHORT          GetLang()                       { return nLanguage; }
    void            SetLang( INT16 nNew )           { nLanguage = nNew; }

    IPRCachedWord*  GetNext()                       { return pNext; }
    void            SetNext( IPRCachedWord* pNew )  { pNext = pNew; }

    IPRCachedWord*  GetPrev()                       { return pPrev; }
    void            SetPrev( IPRCachedWord* pNew )  { pPrev = pNew; }

    IPRCachedWord*  GetFollow()                     { return pFollow; }
    void            SetFollow( IPRCachedWord* pNew ){ pFollow = pNew; }

    void            IncFound()                      { ++nFound; }
    ULONG           GetFound()                      { return nFound; }
    void            SetFound( ULONG nNew )          { nFound = nNew; }
};

///////////////////////////////////////////////////////////////////////////

IPRSpellCache::IPRSpellCache( ULONG nSize ) :
    ppHash      ( NULL ),
    pFirst      ( NULL ),
    pLast       ( NULL ),
    nIndex      ( 0 ),
    nCount      ( 0 ),
    nInputPos   ( 0 ),
    nInputValue ( 0 ),
    nTblSize    ( nSize )
#ifdef DBG_STATISTIC
    ,nMax       ( IPR_DEF_CACHE_MAX ),
    nMaxInput   ( IPR_DEF_CACHE_MAXINPUT ),
    nFound      ( 0 ),
    nLost       ( 0 )
#endif
{
    xDicList = GetDictionaryList();

    pFlushLstnr = new FlushListener( this );
    xFlushLstnr = pFlushLstnr;
    pFlushLstnr->SetDicList( xDicList );    //! after reference is established
}

IPRSpellCache::~IPRSpellCache()
{
    MutexGuard  aGuard( GetLinguMutex() );

    pFlushLstnr->SetDicList( Reference< XDictionaryList >() );

#ifdef DBG_STATISTIC
    // Binary File oeffnen
    String aOutTmp( String::CreateFromAscii( "iprcache.stk" ) )
    SvFileStream aOut( aOutTmp, STREAM_STD_WRITE );

    if( aOut.IsOpen() && !aOut.GetError() && ppHash )
    {
        ByteString aStr( "Gefunden: ");
        aStr += nFound;
        aStr += "   Verloren: ";
        aStr += nLost;
        ULONG nSumSum = 0;
        aOut << aStr.GetBuffer() << endl;
        for( ULONG i = 0; i < nTblSize; ++i )
        {
            aStr = "Index: ";
            aStr += i;
            aStr += "    Tiefe: ";
            ULONG nDeep = 0;
            ULONG nSum = 0;
            IPRCachedWord* pTmp = *( ppHash + i );
            while( pTmp )
            {
                ++nDeep;
                nSum += pTmp->GetFound();
                pTmp = pTmp->GetNext();
            }
            aStr += nDeep;
            aStr += "  Anzahl: ";
            aStr += nSum;
            nSumSum += nSum;
            aOut << aStr.GetBuffer() << endl;
            pTmp = *( ppHash + i );
            aStr = "                 Found: ";
            while( pTmp )
            {
                aStr += pTmp->GetFound();
                aStr += "  ";
                pTmp = pTmp->GetNext();
            }
            aOut << aStr.GetBuffer() << endl;
        }
        aStr = "Summe: ";
        aStr += nSumSum;
        aOut << aStr.GetBuffer() << endl;
    }
#endif

    while( pFirst )
    {
        pLast = pFirst->GetNext();
        delete pFirst;
        pFirst = pLast;
    }
    delete ppHash;
}

void IPRSpellCache::Flush()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if( ppHash )
    {
        while( pFirst )
        {
            pLast = pFirst->GetNext();
            delete pFirst;
            pFirst = pLast;
        }
        delete ppHash;
        ppHash = NULL;
        nIndex = 0;
        nCount = 0;
        nInputPos = 0;
        nInputValue = 0;
#ifdef DBG_STATISTIC
        nFound = 0;
        nLost = 0;
#endif
    }
}

BOOL IPRSpellCache::CheckWord( const String& rWord, INT16 nLang, BOOL bAllLang )
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRet = FALSE;
    // Hash-Index-Berechnung
    nIndex = 0;
    const sal_Unicode* pp = rWord.GetBuffer();
    while( *pp )
        nIndex = nIndex << 1 ^ *pp++;
    nIndex %= nTblSize;

    if( ppHash )
    {
        pRun = *(ppHash + nIndex);

        if( pRun && !( bRet = (rWord == pRun->GetWord() &&
            (nLang == pRun->GetLang() || bAllLang)) ) )
        {
            IPRCachedWord* pTmp = pRun->GetNext();
            while( pTmp && !( bRet = ( rWord == pTmp->GetWord() &&
                (nLang == pRun->GetLang() || bAllLang) ) ) )
            {
                pRun = pTmp;
                pTmp = pTmp->GetNext();
            }
            if ( bRet )
            {   // Gefunden: Umsortieren in der Hash-Liste
                pRun->SetNext( pTmp->GetNext() );
                pTmp->SetNext( *( ppHash + nIndex ) );
                *( ppHash + nIndex ) = pTmp;
                pRun = pTmp;
            }
        }
           if( bRet )
        {
            if ( pRun->GetPrev() )
            {   // Wenn wir noch nicht erster sind, werden wir es jetzt:
                if ( ( pRun->GetFound() <= nInputValue ) &&
                         ( ++nInputPos > IPR_CACHE_MAXINPUT )
                    || ( pInput == pRun ) && !( pInput = pRun->GetFollow() ) )

                {   // Wenn die Input-Stelle am Maximum anlangt, erhoehen
                    ++nInputValue; // wir den InputValue und gehen wieder
                    nInputPos = 0; // an den Anfang
                    pInput = pFirst;
                }
                IPRCachedWord* pTmp = pRun->GetFollow();
                pRun->GetPrev()->SetFollow( pTmp ); //Unser Ex-Prev -> Ex-Follow
                pRun->SetFollow( pFirst );    // Wir selbst -> Ex-First
                pFirst->SetPrev( pRun );      // Wir selbst <- Ex-First
                if( pTmp )
                    pTmp->SetPrev( pRun->GetPrev() ); // Ex-Prev <- Ex-Follow
                else
                    pLast = pRun->GetPrev(); // falls wir letzter waren
                pRun->SetPrev( NULL );  // Erste haben keinen Prev
                pFirst = pRun;          // Wir sind Erster!
            }
            pRun->IncFound(); // Mitzaehlen, wie oft wiedergefunden
        }
    }
    return bRet;
}

void IPRSpellCache::AddWord( const String& rWord, INT16 nLang )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if( !ppHash )
    {
        ppHash = new  IPRCachedWord* [ nTblSize ];
        memset( (void *)ppHash, 0, ( sizeof( IPRCachedWord* ) * nTblSize ) );
    }
    IPRCachedWord* pTmp;
    if( nCount == IPR_CACHE_MAX-1 )
    {
        ULONG nDel = 0;
        pRun = pLast;  // Der letzte wird ueberschrieben
        const sal_Unicode* pp = pRun->GetWord().GetBuffer();
        while( *pp )
            nDel = nDel << 1 ^ *pp++;
        nDel %= nTblSize; // Hash-Index des letzten
        // Der letzte wird aus seiner alten Hash-Liste entfernt
        if( ( pTmp = *( ppHash + nDel ) ) == pRun )
            *( ppHash + nDel ) = pRun->GetNext();
        else
        {
            while( pTmp->GetNext() != pRun )
                pTmp = pTmp->GetNext();
            pTmp->SetNext( pRun->GetNext() );
        }
        pRun->SetWord( rWord ); // Ueberschreiben des alten Inhalts
        pRun->SetLang( nLang );
        pRun->SetFound( 0 );
    }
    else
    {
        ++nCount;
        pRun = new IPRCachedWord( rWord, pFirst, nLang );
        if( pFirst )
            pFirst->SetPrev( pRun );
        pFirst = pRun; // Ganz Neue kommen erstmal nach vorne
        if ( !pLast )
        {
            pLast = pRun;
            pInput = pRun;
        }
    }

    pRun->SetNext( *( ppHash + nIndex ) );  // In der Hash-Liste
    *(ppHash + nIndex ) = pRun;             // vorne einsortieren

    // In der LRU-Kette umsortieren ...
    if ( pRun != pInput && pRun != pInput->GetPrev() )
    {
        pTmp = pRun->GetPrev();
        IPRCachedWord* pFoll = pRun->GetFollow();
        // Entfernen aus der alten Position
        if( pTmp )
            pTmp->SetFollow( pFoll );
        else
            pFirst = pFoll; // wir waren erster
        if( pFoll )
            pFoll->SetPrev( pTmp );
        else
            pLast = pTmp; // wir waren letzter
        // Einfuegen vor pInput
        if( pTmp = pInput->GetPrev() )
            pTmp->SetFollow( pRun );
        else
            pFirst = pRun; // pInput war erster
        pRun->SetPrev( pTmp );
        pRun->SetFollow( pInput );
        pInput->SetPrev( pRun );
    }
    pInput = pRun; // pInput zeigt auf den zuletzt einsortierten
}

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

