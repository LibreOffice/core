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

#include <string.h>
#include <limits.h>
#include "bparr.hxx"
namespace binfilter {

// die Blockverwaltung waechst/schrumpft immer um 20 Bloecke, das sind dann
// immer ~ 20 * MAXENTRY == 20000 Eintraege
const USHORT nBlockGrowSize = 20;

#ifndef DBG_UTIL

#define CHECKIDX( p, n, i, c )

#else

#define CHECKIDX( p, n, i, c ) CheckIdx( p, n, i, c );

/*N*/ void CheckIdx( BlockInfo** ppInf, USHORT nBlock, ULONG nSize, USHORT nCur )
/*N*/ {
/*N*/ 	DBG_ASSERT( !nSize || nCur < nBlock, "BigPtrArray: CurIndex steht falsch" );
/*N*/ 
/*N*/ 	ULONG nIdx = 0;
/*N*/ 	for( USHORT nCnt = 0; nCnt < nBlock; ++nCnt, ++ppInf )
/*N*/ 	{
/*N*/ 		nIdx += (*ppInf)->nElem;
/*N*/ 		// Array mit Luecken darf es nicht geben
/*N*/ 		DBG_ASSERT( !nCnt || (*(ppInf-1))->nEnd + 1 == (*ppInf)->nStart,
/*N*/ 					"BigPtrArray: Luecke in der Verwaltung!" );
/*N*/ 	}
/*N*/ 
/*N*/ 	DBG_ASSERT( nIdx == nSize, "BigPtrArray: Anzahl ungueltig" );
/*N*/ }

#endif


/*N*/ BigPtrArray::BigPtrArray()
/*N*/ {
/*N*/ 	nBlock = nCur = 0;
/*N*/ 	nSize = 0;
/*N*/ 	nMaxBlock = nBlockGrowSize;		// == 20 * 1000 Eintraege
/*N*/ 	ppInf = new BlockInfo* [ nMaxBlock ];
/*N*/ }



/*N*/ BigPtrArray::~BigPtrArray()
/*N*/ {
/*N*/ 	if( nBlock )
/*N*/ 	{
/*N*/ 		BlockInfo** pp = ppInf;
/*N*/ 		for( USHORT n = 0; n < nBlock; ++n, ++pp )
/*N*/ 		{
/*N*/ 						delete[] (*pp)->pData;
/*N*/ 			delete    *pp;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	delete[] ppInf;
/*N*/ }

// Einfachst-Implementation, evtl. spaeter mal komplexer
#if 0
#endif

// Auch der Move ist schlicht. Optimieren ist hier wg. der
// Stueckelung des Feldes zwecklos!


// das Ende ist EXCLUSIV


/*N*/ void BigPtrArray::ForEach( ULONG nStart, ULONG nEnd,
/*N*/ 							FnForEach fn, void* pArgs )
/*N*/ {
/*N*/ 	if( nEnd > nSize )
/*N*/ 		nEnd = nSize;
/*N*/ 
/*N*/ 	if( nStart < nEnd )
/*N*/ 	{
/*N*/ 		USHORT cur = Index2Block( nStart );
/*N*/ 		BlockInfo** pp = ppInf + cur;
/*N*/ 		BlockInfo* p = *pp;
/*N*/ 		USHORT nElem = USHORT( nStart - p->nStart );
/*N*/ 		ElementPtr* pElem = p->pData + nElem;
/*N*/ 		nElem = p->nElem - nElem;
/*N*/ 		for(;;)
/*N*/ 		{
/*N*/ 			if( !(*fn)( *pElem++, pArgs ) || ++nStart >= nEnd )
/*N*/ 				break;
/*N*/ 
/*N*/ 			// naechstes Element
/*N*/ 			if( !--nElem )
/*N*/ 			{
/*N*/ 				// neuer Block
/*N*/ 				p = *++pp;
/*N*/ 				pElem = p->pData;
/*N*/ 				nElem = p->nElem;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ ElementPtr BigPtrArray::operator[]( ULONG idx ) const
/*N*/ {
/*N*/ 	// weil die Funktion eben doch nicht const ist:
/*N*/ 	DBG_ASSERT( idx < nSize, "operator[]: Index aussserhalb" );
/*N*/ 	BigPtrArray* pThis = (BigPtrArray*) this;
/*N*/ 	USHORT cur = Index2Block( idx );
/*N*/ 	BlockInfo* p = ppInf[ cur ];
/*N*/ 	pThis->nCur = cur;
/*N*/ 	return p->pData[ idx - p->nStart ];
/*N*/ }

///////////////////////////////////////////////////////////////////////////

// private Methoden

// Suchen des Blocks einer bestimmten Position
// Algorithmus:
// 1. Test, ob der letzte Block der gesuchte Block ist
// 2. Sonderfall: Index = 0?
// 3. Test der Nachbarbloecke

// 4. Binaere Suche



/*N*/ USHORT BigPtrArray::Index2Block( ULONG pos ) const
/*N*/ {
/*N*/ 	// zuletzt verwendeter Block?
/*N*/ 	BlockInfo* p = ppInf[ nCur ];
/*N*/ 	if( p->nStart <= pos && p->nEnd >= pos )
/*N*/ 		return nCur;
/*N*/ 	// Index = 0?
/*N*/ 	if( !pos )
/*N*/ 		return 0;
/*N*/ 	// Folgeblock?
/*N*/ 	if( nCur < ( nBlock - 1 ) )
/*N*/ 	{
/*N*/ 		p = ppInf[ nCur+1 ];
/*N*/ 		if( p->nStart <= pos && p->nEnd >= pos )
/*N*/ 			return nCur+1;
/*N*/ 	}
/*N*/ 	// vorangehender Block?
/*N*/ 	else if( pos < p->nStart && nCur > 0 )
/*N*/ 	{
/*N*/ 		p = ppInf[ nCur-1 ];
/*N*/ 		if( p->nStart <= pos && p->nEnd >= pos )
/*N*/ 			return nCur-1;
/*N*/ 	}
/*N*/ 	// Binaere Suche:
/*N*/ 	// Diese fuehrt immer zum Erfolg
/*N*/ 	USHORT lower = 0, upper = nBlock - 1;
/*N*/ 	USHORT cur = 0;
/*N*/ 	for(;;)
/*N*/ 	{
/*N*/ 		USHORT n = lower + ( upper - lower ) / 2;
/*N*/ 		cur = ( n == cur ) ? n+1 : n;
/*N*/ 		p = ppInf[ cur ];
/*N*/ 		if( p->nStart <= pos && p->nEnd >= pos )
/*N*/ 			return cur;
/*N*/ 		if( p->nStart > pos )
/*N*/ 			upper = cur;
/*N*/ 		else
/*N*/ 			lower = cur;
/*N*/ 	}
/*N*/ }


// Update aller Indexbereiche ab einer bestimmten Position

// pos bezeichnet den letzten korrekten Block

/*N*/ void BigPtrArray::UpdIndex( USHORT pos )
/*N*/ {
/*N*/ 	BlockInfo** pp = ppInf + pos;
/*N*/ 	ULONG idx = (*pp)->nEnd + 1;
/*N*/ 	BlockInfo* p;
/*N*/ 	while( ++pos < nBlock )
/*N*/ 	{
/*N*/ 		p = *++pp;
/*N*/ 		p->nStart = idx;
/*N*/ 		idx       += p->nElem;
/*N*/ 		p->nEnd   = idx - 1;
/*N*/ 	}
/*N*/ }

// Einrichten eines neuen Blocks an einer bestimmten Position

// Vorhandene Blocks werden nach hinten verschoben



/*N*/ BlockInfo* BigPtrArray::InsBlock( USHORT pos )
/*N*/ {
/*N*/ 	if( nBlock == nMaxBlock )
/*N*/ 	{
/*N*/ 		// dann sollte wir mal das Array erweitern
/*N*/ 		BlockInfo** ppNew = new BlockInfo* [ nMaxBlock + nBlockGrowSize ];
/*N*/ 		memcpy( ppNew, ppInf, nMaxBlock * sizeof( BlockInfo* ));
/*N*/ 		delete[] ppInf;
/*N*/ 		nMaxBlock += nBlockGrowSize;
/*N*/ 		ppInf = ppNew;
/*N*/ 	}
/*N*/ 	if( pos != nBlock )
/*N*/ 		memmove( ppInf + pos+1, ppInf + pos ,
/*N*/ 				 ( nBlock - pos ) * sizeof (BlockInfo*) );
/*N*/ 	++nBlock;
/*N*/ 	BlockInfo* p = new BlockInfo;
/*N*/ 	ppInf[ pos ] = p;
/*N*/ 
/*N*/ 	if( pos )
/*N*/ 		p->nStart = p->nEnd = ppInf[ pos-1 ]->nEnd + 1;
/*N*/ 	else
/*N*/ 		p->nStart = p->nEnd = 0;
/*N*/ 	p->nEnd--;	// keine Elemente
/*N*/ 	p->nElem = 0;
/*N*/ 	p->pData = new ElementPtr [ MAXENTRY ];
/*N*/ 	p->pBigArr = this;
/*N*/ 	return p;
/*N*/ }

/*N*/ void BigPtrArray::BlockDel( USHORT nDel )
/*N*/ {
/*N*/ 	nBlock -= nDel;
/*N*/ 	if( nMaxBlock - nBlock > nBlockGrowSize )
/*N*/ 	{
/*N*/ 		// dann koennen wir wieder schrumpfen
/*N*/ 		nDel = (( nBlock / nBlockGrowSize ) + 1 ) * nBlockGrowSize;
/*N*/ 		BlockInfo** ppNew = new BlockInfo* [ nDel ];
/*N*/ 		memcpy( ppNew, ppInf, nBlock * sizeof( BlockInfo* ));
/*N*/ 		delete[] ppInf;
/*N*/ 		ppInf = ppNew;
/*N*/ 		nMaxBlock = nDel;
/*N*/ 	}
/*N*/ }


/*N*/ void BigPtrArray::Insert( const ElementPtr& rElem, ULONG pos )
/*N*/ {
/*N*/ 	CHECKIDX( ppInf, nBlock, nSize, nCur );
/*N*/ 
/*N*/ 	BlockInfo* p;
/*N*/ 	USHORT cur;
/*N*/ 	if( !nSize )
/*N*/ 		// Sonderfall: erstes Element einfuegen
/*N*/ 		p = InsBlock( cur = 0 );
/*N*/ 	else if( pos == nSize )
/*N*/ 	{
/*N*/ 		// Sonderfall: Einfuegen am Ende
/*N*/ 		cur = nBlock - 1;
/*N*/ 		p = ppInf[ cur ];
/*N*/ 		if( p->nElem == MAXENTRY )
/*N*/ 			// Der letzte Block ist voll, neuen anlegen
/*?*/ 			p = InsBlock( ++cur );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Standardfall:
/*N*/ 		cur = Index2Block( pos );
/*N*/ 		p = ppInf[ cur ];
/*N*/ 	}
/*N*/ 	if( p->nElem == MAXENTRY )
/*N*/ 	{
/*N*/ 		// passt der letzte Eintrag in den naechsten Block?
/*N*/ 		BlockInfo* q;
/*N*/ 		if( cur < ( nBlock - 1 ) && ppInf[ cur+1 ]->nElem < MAXENTRY )
/*N*/ 		{
/*N*/ 			q = ppInf[ cur+1 ];
/*N*/ 			if( q->nElem )
/*N*/ 			{
/*N*/ 				register int nCount = q->nElem;
/*N*/ 				register ElementPtr *pFrom = q->pData + nCount,
/*N*/ 									*pTo = pFrom+1;
/*N*/ 				while( nCount-- )
/*N*/ 					++( *--pTo = *--pFrom )->nOffset;
/*N*/ 			}
/*N*/ 			q->nStart--;
/*N*/ 			q->nEnd--;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// Wenn er auch nicht in den Folgeblock passt, muss ein
/*N*/ 			// neuer Block eingefuegt werden
/*N*/ 			// erst mal bei Bedarf komprimieren
/*N*/ 
/*N*/ 			// wenn mehr als 50% "Luft" im Array ist, dann sollte man mal das
/*N*/ 			// Compress aufrufen
/*N*/ 			if( /*nBlock == nMaxBlock &&*/
/*N*/ 				nBlock > ( nSize / ( MAXENTRY / 2 ) ) &&
/*N*/ 				cur >= Compress() )
/*N*/ 			{
/*N*/ 				// es wurde vor der akt. Pos etwas verschoben und alle
/*N*/ 				// Pointer koennen ungueltig sein. Also das Insert
/*N*/ 				// nochmals aufsetzen
/*?*/ 				Insert( rElem, pos );
/*N*/ 				return ;
/*N*/ 			}
/*N*/ 
/*N*/ 			q = InsBlock( cur+1 );
/*N*/ 		}
/*N*/ 
/*N*/ 		// Eintrag passt nicht mehr. Dann muss Platz gemacht werden
/*N*/ 		ElementPtr pLast = p->pData[ MAXENTRY-1 ];
/*N*/ 		pLast->nOffset = 0;
/*N*/ 		pLast->pBlock = q;
/*N*/ 
/*N*/ 		q->pData[ 0 ] = pLast;
/*N*/ 		q->nElem++;
/*N*/ 		q->nEnd++;
/*N*/ 
/*N*/ 		p->nEnd--;
/*N*/ 		p->nElem--;
/*N*/ 	}
/*N*/ 	// Nun haben wir einen freien Block am Wickel: eintragen
/*N*/ 	pos -= p->nStart;
/*N*/ 	DBG_ASSERT( pos < MAXENTRY, "falsche Pos" );
/*N*/ 	if( pos != p->nElem )
/*N*/ 	{
/*N*/ 		register int nCount = p->nElem - USHORT(pos);
/*N*/ 		register ElementPtr *pFrom = p->pData + p->nElem,
/*N*/ 							*pTo = pFrom + 1;
/*N*/ 		while( nCount-- )
/*N*/ 			++( *--pTo = *--pFrom )->nOffset;
/*N*/ 	}
/*N*/ 	// Element eintragen und Indexe updaten
/*N*/ 	((ElementPtr&)rElem)->nOffset = USHORT(pos);
/*N*/ 	((ElementPtr&)rElem)->pBlock = p;
/*N*/ 	p->pData[ pos ] = rElem;
/*N*/ 	p->nEnd++;
/*N*/ 	p->nElem++;
/*N*/ 	nSize++;
/*N*/ 	if( cur != ( nBlock - 1 ) ) UpdIndex( cur );
/*N*/ 	nCur = cur;
/*N*/ 
/*N*/ 	CHECKIDX( ppInf, nBlock, nSize, nCur );
/*N*/ }

/*N*/ void BigPtrArray::Remove( ULONG pos, ULONG n )
/*N*/ {
/*N*/ 	CHECKIDX( ppInf, nBlock, nSize, nCur );
/*N*/ 
/*N*/ 	USHORT nBlkdel = 0;					// entfernte Bloecke
/*N*/ 	USHORT cur = Index2Block( pos );	// aktuelle Blocknr
/*N*/ 	USHORT nBlk1 = cur;                 // 1. behandelter Block
/*N*/ 	USHORT nBlk1del = USHRT_MAX;		// 1. entfernter Block
/*N*/ 	BlockInfo* p = ppInf[ cur ];
/*N*/ 	pos -= p->nStart;
/*N*/ 	ULONG nElem = n;
/*N*/ 	while( nElem )
/*N*/ 	{
/*N*/ 		USHORT nel = p->nElem - USHORT(pos);
/*N*/ 		if( ULONG(nel) > nElem )
/*N*/ 			nel = USHORT(nElem);
/*N*/ 		// Eventuell Elemente verschieben
/*N*/ 		if( ( pos + nel ) < ULONG(p->nElem) )
/*N*/ 		{
/*N*/ 			register ElementPtr *pTo = p->pData + pos,
/*N*/ 								*pFrom = pTo + nel;
/*N*/ 			register int nCount = p->nElem - nel - USHORT(pos);
/*N*/ 			while( nCount-- )
/*N*/ 			{
/*N*/ 				(*pTo++ = *pFrom++)->nOffset -= nel;
/*N*/ //				(*pTo++)->nOffset -= nel;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		p->nEnd -= nel;
/*N*/ 		p->nElem -= nel;
/*N*/ 		if( !p->nElem )
/*N*/ 		{
/*N*/ 			// eventuell Block ganz entfernen
/*N*/ 			delete[] p->pData;
/*N*/ 			nBlkdel++;
/*N*/ 			if( USHRT_MAX == nBlk1del )
/*N*/ 				nBlk1del = cur;
/*N*/ 		}
/*N*/ 		nElem -= nel;
/*N*/ 		if( !nElem )
/*N*/ 			break;
/*N*/ 		p = ppInf[ ++cur ];
/*N*/ 		pos = 0;
/*N*/ 	}
/*N*/ 	// Am Ende die Tabelle updaten, falls Bloecke geloescht waren
/*N*/ 	if( nBlkdel )
/*N*/ 	{
/*N*/ 		// loeschen sollte man immer !!
/*N*/ 		for( USHORT i = nBlk1del; i < ( nBlk1del + nBlkdel ); i++ )
/*N*/ 			delete ppInf[ i ];
/*N*/ 
/*N*/ 		if( ( nBlk1del + nBlkdel ) < nBlock )
/*N*/ 		{
/*N*/ 			memmove( ppInf + nBlk1del, ppInf + nBlk1del + nBlkdel,
/*N*/ 					 ( nBlock - nBlkdel - nBlk1del ) * sizeof( BlockInfo* ) );
/*N*/ 
/*N*/ 			// JP 19.07.95: nicht den ersten behandelten, sondern den davor!!
/*N*/ 			//				UpdateIdx updatet nur alle Nachfolgende!!
/*N*/ 			if( !nBlk1 )
/*N*/ 			{
/*N*/ 				p = ppInf[ 0 ];
/*N*/ 				p->nStart = 0;
/*N*/ 				p->nEnd = p->nElem-1;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				--nBlk1;
/*N*/ 		}
/*N*/ 		BlockDel( nBlkdel );			// es wurden Bloecke geloescht
/*N*/ 	}
/*N*/ 
/*N*/ 	nSize -= n;
/*N*/ 	if( nBlk1 != ( nBlock - 1 ) && nSize )
/*N*/ 		UpdIndex( nBlk1 );
/*N*/ 	nCur = nBlk1;
/*N*/ 
/*N*/ 	// wenn mehr als 50% "Luft" im Array ist, dann sollte man mal das
/*N*/ 	// Compress aufrufen
/*N*/ 	if( nBlock > ( nSize / ( MAXENTRY / 2 ) ) )
/*N*/ 		Compress();
/*N*/ 
/*N*/ 	CHECKIDX( ppInf, nBlock, nSize, nCur );
/*N*/ }


/*N*/ void BigPtrArray::Replace( ULONG idx, const ElementPtr& rElem)
/*N*/ {
/*N*/ 	// weil die Funktion eben doch nicht const ist:
/*N*/ 	DBG_ASSERT( idx < nSize, "Set: Index aussserhalb" );
/*N*/ 	BigPtrArray* pThis = (BigPtrArray*) this;
/*N*/ 	USHORT cur = Index2Block( idx );
/*N*/ 	BlockInfo* p = ppInf[ cur ];
/*N*/ 	pThis->nCur = cur;
/*N*/ 	((ElementPtr&)rElem)->nOffset = USHORT(idx - p->nStart);
/*N*/ 	((ElementPtr&)rElem)->pBlock = p;
/*N*/ 	p->pData[ idx - p->nStart ] = rElem;
/*N*/ }


// Index eines Eintrags heraussuchen
// Dies ist der selbe Code wie in ForEach() mit dem Unterschied,
// dass statt einer Funktion ein Vergleich durchgefuehrt wird.
// Man haette auch ForEach() nehmen koennen, spart aber durch
// den fehlenden Call etwas Zeit.



// Array komprimieren

/*N*/ USHORT BigPtrArray::Compress( short nMax )
/*N*/ {
/*N*/ 	CHECKIDX( ppInf, nBlock, nSize, nCur );
/*N*/ 
/*N*/ 	// Es wird von vorne nach hinten ueber das InfoBlock Array iteriert.
/*N*/ 	// Wenn zwischen durch Block gel”scht werden, dann mussen alle
/*N*/ 	// nachfolgenden verschoben werden. Dazu werden die Pointer pp und qq
/*N*/ 	// benutzt; wobei pp das "alte" Array, qq das "neue" Array ist.
/*N*/ 	BlockInfo** pp = ppInf, **qq = pp;
/*N*/ 	BlockInfo* p;
/*N*/ 	BlockInfo* pLast;					// letzter nicht voller Block
/*N*/ 	USHORT nLast = 0;					// fehlende Elemente
/*N*/ 	USHORT nBlkdel = 0;					// Anzahl der geloeschte Bloecke
/*N*/ 	USHORT nFirstChgPos = USHRT_MAX;	// ab welcher Pos gab es die 1. Aenderung?
/*N*/ 
/*N*/ 	// von Fuell-Prozenten auf uebrige Eintrage umrechnen
/*N*/ 	nMax = MAXENTRY - (long) MAXENTRY * nMax / 100;
/*N*/ 
/*N*/ 	for( USHORT cur = 0; cur < nBlock; ++cur )
/*N*/ 	{
/*N*/ 		p = *pp++;
/*N*/ 		USHORT n = p->nElem;
/*N*/ 		// Testen, ob der noch nicht volle Block so gelassen wird
/*N*/ 		// dies ist der Fall, wenn der aktuelle Block gesplittet
/*N*/ 		// werden muesste, der noch nicht volle Block aber bereits
/*N*/ 		// ueber dem uebergebenen Break-Wert voll ist. In diesem
/*N*/ 		// Fall wird von einer weiteren Fuellung (die ja wegen dem
/*N*/ 		// zweifachen memmove() zeitaufwendig ist) abgesehen.
/*N*/ 		if( nLast && ( n > nLast ) && ( nLast < nMax ) )
/*N*/ 			nLast = 0;
/*N*/ 		if( nLast )
/*N*/ 		{
/*N*/ 			if( USHRT_MAX == nFirstChgPos )
/*N*/ 				nFirstChgPos = cur;
/*N*/ 
/*N*/ 			// ein nicht voller Block vorhanden: auffuellen
/*N*/ 			if( n > nLast )
/*N*/ 				n = nLast;
/*N*/ 
/*N*/ 			// Elemente uebertragen, vom akt. in den letzten
/*N*/ 			register ElementPtr* pElem = pLast->pData + pLast->nElem;
/*N*/ 			register ElementPtr* pFrom = p->pData;
/*N*/ 			for( register int nCount = n, nOff = pLast->nElem;
/*N*/ 							nCount; --nCount, ++pElem )
/*N*/ 				*pElem = *pFrom++,
/*N*/ 					(*pElem)->pBlock = pLast,
/*N*/ 					(*pElem)->nOffset = nOff++;
/*N*/ 
/*N*/ 			// korrigieren
/*N*/ 			pLast->nElem += n;
/*N*/ 			nLast    -= n;
/*N*/ 			p->nElem -= n;
/*N*/ 
/*N*/ 			// Ist der aktuelle Block dadurch leer geworden?
/*N*/ 			if( !p->nElem )
/*N*/ 			{
/*N*/ 				// dann kann der entfernt werden
/*N*/ 				delete[] p->pData;
/*N*/ 				delete   p, p = 0;
/*N*/ 				++nBlkdel;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pElem = p->pData, pFrom = pElem + n;
/*N*/ 				register int nCount = p->nElem;
/*N*/ 				while( nCount-- )
/*N*/ 				{
/*N*/ 					(*pElem++ = *pFrom++)->nOffset -= n;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( p )		// die Blockinfo wurde nicht geloescht
/*N*/ 		{
/*N*/ 			*qq++ = p;		// dann setze sie an die richtige neue Position
/*N*/ 
/*N*/ 			// eventuell den letzten halbvollen Block festhalten
/*N*/ 			if( !nLast && p->nElem < MAXENTRY )
/*N*/ 			{
/*N*/ 				pLast = p;
/*N*/ 				nLast = MAXENTRY - p->nElem;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Bloecke geloescht wurden, ggfs. das BlockInfo Array verkuerzen
/*N*/ 	if( nBlkdel )
/*N*/ 		BlockDel( nBlkdel );
/*N*/ 
/*N*/ 	// Und neu durchindizieren
/*N*/ 	p = ppInf[ 0 ];
/*N*/ 	p->nEnd = p->nElem - 1;
/*N*/ 	UpdIndex( 0 );
/*N*/ 
/*N*/ 	if( nCur >= nFirstChgPos )
/*N*/ 		nCur = 0;
/*N*/ 
/*N*/ 	CHECKIDX( ppInf, nBlock, nSize, nCur );
/*N*/ 
/*N*/ 	return nFirstChgPos;
/*N*/ }


}
