/*************************************************************************
 *
 *  $RCSfile: contnr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:08 $
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

#define private     public
#define protected   public

#ifndef _LIMITS_H
#include <limits.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _SOLAR_H
#include <solar.h>
#endif

#ifndef _IMPCONT_HXX
#include <impcont.hxx>
#endif

#ifndef _CONTNR_HXX
#include <contnr.hxx>
#endif

#ifndef _DEBUG_HXX
#include <debug.hxx>
#endif

// -----------------------------------------------------------------------

DBG_NAME( CBlock );
DBG_NAME( Container );

/*************************************************************************
|*
|*    DbgCheckCBlock()
|*
|*    Beschreibung      Pruefung eines CBlock fuer Debug-Utilities
|*    Ersterstellung    MI 30.01.92
|*    Letzte Aenderung  TH 24.01.96
|*
*************************************************************************/

#ifdef DBG_UTIL
const char* DbgCheckCBlock( const void* pBlock )
{
    CBlock* p = (CBlock*)pBlock;

    if ( p->nCount > p->nSize )
        return "nCount > nSize";

    if ( p->nSize && !p->pNodes )
        return "nSize > 0 && pNodes == NULL";

    return NULL;
}
#endif

/*************************************************************************
|*
|*    CBlock::CBlock()
|*
|*    Beschreibung      Construktor des Verwaltungsblocks
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

CBlock::CBlock( USHORT nInitSize, CBlock* _pPrev, CBlock* _pNext )
{
    DBG_CTOR( CBlock, DbgCheckCBlock );

    pPrev   = _pPrev;
    pNext   = _pNext;
    nSize   = nInitSize;
    nCount  = 0;

    // Datenpuffer anlegen
    pNodes = (void**)new PVOID[nSize];
}

/*************************************************************************
|*
|*    CBlock::CBlock()
|*
|*    Beschreibung      Construktor des Verwaltungsblocks
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

CBlock::CBlock( USHORT _nSize, CBlock* _pPrev )
{
    DBG_CTOR( CBlock, DbgCheckCBlock );
    DBG_ASSERT( _nSize, "CBlock::CBlock(): nSize == 0" );

    pPrev   = _pPrev;
    pNext   = NULL;
    nSize   = _nSize;
    nCount  = _nSize;

    // Datenpuffer anlegen und initialisieren
    pNodes = (void**)new PVOID[nSize];
    memset( pNodes, 0, nSize*sizeof(PVOID) );
}

/*************************************************************************
|*
|*    CBlock::CBlock()
|*
|*    Beschreibung      Copy-Construktor des Verwaltungsblocks
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

CBlock::CBlock( const CBlock& r, CBlock* _pPrev )
{
    DBG_CTOR( CBlock, DbgCheckCBlock );
    DBG_CHKOBJ( &r, CBlock, DbgCheckCBlock );

    pPrev   = _pPrev;
    pNext   = NULL;
    nSize   = r.nSize;
    nCount  = r.nCount;

    // Datenpuffer anlegen und Daten kopieren
    pNodes = (void**)new PVOID[nSize];
    memcpy( pNodes, r.pNodes, nCount*sizeof(PVOID) );
}

/*************************************************************************
|*
|*    CBlock::~CBlock()
|*
|*    Beschreibung      Destruktor des Verwaltungsblocks
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

inline CBlock::~CBlock()
{
    DBG_DTOR( CBlock, DbgCheckCBlock );

    // Daten loeschen
    delete pNodes;
}

/*************************************************************************
|*
|*    CBlock::Insert()
|*
|*    Beschreibung      Fuegt einen Pointer ein
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void CBlock::Insert( void* p, USHORT nIndex, USHORT nReSize )
{
    DBG_CHKTHIS( CBlock, DbgCheckCBlock );
    DBG_ASSERT( nIndex <= nCount, "CBlock::Insert(): Index > nCount" );

    // Muss Block realokiert werden
    if ( nCount == nSize )
    {
        // Neue Daten anlegen
        nSize += nReSize;
        void** pNewNodes = (void**)new PVOID[nSize];

        // Wird angehaengt
        if ( nCount == nIndex )
        {
            // Daten kopieren
            memcpy( pNewNodes, pNodes, nCount*sizeof(PVOID) );
        }
        else
        {
            // Daten kopieren
            memcpy( pNewNodes, pNodes, nIndex*sizeof(PVOID) );
            memcpy( pNewNodes + nIndex + 1,
                    pNodes + nIndex,
                    (nCount-nIndex)*sizeof(PVOID) );
        }

        // Alte Daten loeschen und neue setzen
        delete pNodes;
        pNodes = pNewNodes;
    }
    else
    {
        if ( nIndex < nCount )
        {
            memmove( pNodes + nIndex + 1,
                     pNodes + nIndex,
                     (nCount-nIndex)*sizeof(PVOID) );
        }
    }

    // Neuen Pointer setzen und Elementgroesse erhoehen
    pNodes[nIndex] = p;
    nCount++;
}

/*************************************************************************
|*
|*    CBlock::Split()
|*
|*    Beschreibung      Fuegt einen Pointer ein und splittet den Block
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

CBlock* CBlock::Split( void* p, USHORT nIndex, USHORT nReSize )
{
    DBG_CHKTHIS( CBlock, DbgCheckCBlock );

    USHORT  nNewSize;
    USHORT  nMiddle;
    CBlock* pNewBlock;

    nMiddle = nCount/2;

    if ( ( nIndex == nCount ) || ( nIndex == 0 ) )
        nNewSize = nReSize;
    else
    {
        // Der aktuelle Block wird in der Mitte geteilt
        nNewSize = (nCount+1) / 2;

        if ( nNewSize < nReSize )
            nNewSize = nReSize;
        else
        {
            // Neue Groesse muss ein vielfaches von Resize sein
            if ( nNewSize % nReSize )
                nNewSize += nReSize - (nNewSize % nReSize);
            else
                nNewSize += nReSize;
        }
    }

    // Vor oder hinter dem aktuellem Block einfuegen?
    if ( nIndex > nMiddle )
    {
        // Neuen Split-Block anlegen und hinter dem aktuellem Block einfuegen
        pNewBlock = new CBlock( nNewSize, this, pNext );

        if ( pNext )
            pNext->pPrev = pNewBlock;
        pNext = pNewBlock;

        if ( nIndex == nCount )
        {
            // Neuen Pointer einfuegen
            pNewBlock->pNodes[0] = p;
            pNewBlock->nCount = 1;
        }
        else
        {
            nIndex -= nMiddle;
            // Alles von Mitte bis Index kopieren
            if ( nIndex )
                memcpy( pNewBlock->pNodes, pNodes+nMiddle, nIndex*sizeof(PVOID) );

            // Neuen Pointer einfuegen
            pNewBlock->pNodes[nIndex] = p;

            // Alles von Mitte bis Ende hinter Index kopieren
            memcpy( pNewBlock->pNodes+nIndex+1,
                    pNodes+nMiddle+nIndex,
                    (nCount-nMiddle-nIndex) * sizeof(PVOID) );

            pNewBlock->nCount = (nCount-nMiddle+1);
            nCount = nMiddle;

            // Den aktuellen Datenbereich auch halbieren
            if ( nSize != nNewSize )
            {
                void** pNewNodes = (void**)new PVOID[nNewSize];
                memcpy( pNewNodes, pNodes, nCount*sizeof(PVOID) );
                delete pNodes;
                pNodes = pNewNodes;
                nSize = nNewSize;
            }
        }
    }
    else
    {
        // Neuen Split-Block anlegen und vor dem aktuellem Block einfuegen
        pNewBlock = new CBlock( nNewSize, pPrev, this );

        if ( pPrev )
            pPrev->pNext = pNewBlock;
        pPrev = pNewBlock;

        if ( nIndex == 0 )
        {
            // Neuen Pointer einfuegen
            pNewBlock->pNodes[0] = p;
            pNewBlock->nCount = 1;
        }
        else
        {
            // Alles von Anfang bis Index kopieren
            memcpy( pNewBlock->pNodes, pNodes, nIndex*sizeof(PVOID) );

            // Neuen Pointer einfuegen
            pNewBlock->pNodes[nIndex] = p;

            // Alles von Index bis Mitte hinter Index kopieren
            if ( nIndex != nMiddle )
            {
                memcpy( pNewBlock->pNodes+nIndex+1,
                        pNodes+nIndex,
                        (nMiddle-nIndex) * sizeof(PVOID) );
            }

            pNewBlock->nCount = nMiddle+1;
            nCount -= nMiddle;

            // Die zweite Haelfte in einen neuen Block kopieren
            if ( nSize != nNewSize )
            {
                void** pNewNodes = (void**)new PVOID[nNewSize];
                memcpy( pNewNodes, pNodes+nMiddle, nCount*sizeof(PVOID) );
                delete pNodes;
                pNodes = pNewNodes;
                nSize = nNewSize;
            }
            else
                memmove( pNodes, pNodes+nMiddle, nCount*sizeof(PVOID) );
        }
    }

    // Neu angelegten Block zurueckgeben, da gegebenfalls die Blockpointer
    // im Container angepast werden koennen
    return pNewBlock;
}

/*************************************************************************
|*
|*    CBlock::Remove()
|*
|*    Beschreibung      Entfernt einen Pointer
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* CBlock::Remove( USHORT nIndex, USHORT nReSize )
{
    DBG_CHKTHIS( CBlock, DbgCheckCBlock );

    // Alten Pointer sichern
    void* pOld = pNodes[nIndex];

    // 1 Element weniger
    nCount--;

    // Block verkleinern (wenn Reallokationsgroesse um 4 unterschritten wird)
    if ( nCount == (nSize-nReSize-4) )
    {
        // Neue Daten anlegen
        nSize -= nReSize;
        void** pNewNodes = (void**)new PVOID[nSize];

        // Wird letzter Eintrag geloescht
        if ( nIndex == nCount )
        {
            // Daten kopieren
            memcpy( pNewNodes, pNodes, nCount*sizeof(PVOID) );
        }
        else
        {
            // Daten kopieren
            memcpy( pNewNodes, pNodes, nIndex*sizeof(PVOID) );
            memcpy( pNewNodes + nIndex, pNodes + nIndex+1,
                    (nCount-nIndex)*sizeof(PVOID) );
        }

        // Alte Daten loeschen und neue setzen
        delete pNodes;
        pNodes = pNewNodes;
    }
    else
    {
        // Wenn nicht das letzte Element, dann zusammenschieben
        if ( nIndex < nCount )
        {
            memmove( pNodes + nIndex, pNodes + nIndex + 1,
                     (nCount-nIndex)*sizeof(PVOID) );
        }
    }

    // Alten Pointer zurueckgeben
    return pOld;
}

/*************************************************************************
|*
|*    CBlock::Replace()
|*
|*    Beschreibung      Ersetzt einen Pointer
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

inline void* CBlock::Replace( void* p, USHORT nIndex )
{
    DBG_CHKTHIS( CBlock, DbgCheckCBlock );

    // Alten Pointer sichern, neuen setzen und alten zurueckgeben
    void* pOld = pNodes[nIndex];
    pNodes[nIndex] = p;
    return pOld;
}

/*************************************************************************
|*
|*    CBlock::GetObjectPtr()
|*
|*    Beschreibung      Gibt einen Pointer auf den Pointer aus dem Block
|*                      zurueck
|*    Ersterstellung    TH 26.01.93
|*    Letzte Aenderung  TH 26.01.93
|*
*************************************************************************/

inline void** CBlock::GetObjectPtr( USHORT nIndex )
{
    DBG_CHKTHIS( CBlock, DbgCheckCBlock );

    return &(pNodes[nIndex]);
}

/*************************************************************************
|*
|*    CBlock::SetSize()
|*
|*    Beschreibung      Aendert die Groesse des Blocks
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void CBlock::SetSize( USHORT nNewSize )
{
    DBG_CHKTHIS( CBlock, DbgCheckCBlock );
    DBG_ASSERT( nNewSize, "CBlock::SetSize(): nNewSize == 0" );

    // Unterscheidet sich die Groesse
    if ( nNewSize != nCount )
    {
        // Array erweitern
        void** pNewNodes = new PVOID[nNewSize];

        // Alte Tabelle in die Neue kopieren
        if ( nNewSize < nCount )
            memcpy( pNewNodes, pNodes, nNewSize*sizeof(PVOID) );
        else
        {
            memcpy( pNewNodes, pNodes, nCount*sizeof(PVOID) );

            // Array mit 0 initialisieren
            memset( pNewNodes+nCount, 0, (nNewSize-nCount)*sizeof(PVOID) );
        }

        // Altes Array loeschen und neue Werte setzen
        nSize  = nNewSize;
        nCount = nSize;
        delete pNodes;
        pNodes = pNewNodes;
    }
}

//------------------------------------------------------------------------

/*************************************************************************
|*
|*    DbgCheckContainer()
|*
|*    Beschreibung      Pruefung eines Container fuer Debug-Utilities
|*    Ersterstellung    MI 30.01.92
|*    Letzte Aenderung  TH 24.01.96
|*
*************************************************************************/

#ifdef DBG_UTIL
const char* DbgCheckContainer( const void* pCont )
{
    Container* p = (Container*)pCont;

    if ( p->nCount && (!p->pFirstBlock || !p->pLastBlock || !p->pCurBlock) )
        return "nCount > 0 but no CBlocks";

    return NULL;
}
#endif

/*************************************************************************
|*
|*    ImpCopyContainer()
|*
|*    Beschreibung      Kopiert alle Daten des Containers
|*    Ersterstellung    TH 24.01.96
|*    Letzte Aenderung  TH 24.01.96
|*
*************************************************************************/

static void ImpCopyContainer( Container* pCont1, const Container* pCont2 )
{
    // Werte vom uebergebenen Container uebernehmen
    pCont1->nCount     = pCont2->nCount;
    pCont1->nCurIndex  = pCont2->nCurIndex;
    pCont1->nInitSize  = pCont2->nInitSize;
    pCont1->nReSize    = pCont2->nReSize;
    pCont1->nBlockSize = pCont2->nBlockSize;

    // Alle Bloecke kopieren
    if ( pCont2->nCount )
    {
        CBlock* pBlock1;
        CBlock* pBlock2;
        CBlock* pTempBlock;

        // Erstmal ersten Block kopieren
        pBlock2 = pCont2->pFirstBlock;
        pCont1->pFirstBlock = new CBlock( *pBlock2, NULL );
        // Ist erster Block der Current-Block, dann Current-Block setzen
        if ( pBlock2 == pCont2->pCurBlock )
            pCont1->pCurBlock = pCont1->pFirstBlock;
        pBlock1 = pCont1->pFirstBlock;
        pBlock2 = pBlock2->GetNextBlock();
        while ( pBlock2 )
        {
            // Neuen Block anlegen und aus der uebergebenen Liste kopieren
            pTempBlock = new CBlock( *pBlock2, pBlock1 );
            pBlock1->SetNextBlock( pTempBlock );
            pBlock1 = pTempBlock;

            // Current-Block beruecksichtigen
            if ( pBlock2 == pCont2->pCurBlock )
                pCont1->pCurBlock = pBlock1;

            // Auf naechsten Block weitersetzen
            pBlock2 = pBlock2->GetNextBlock();
        }

        // Letzten Block setzen
        pCont1->pLastBlock = pBlock1;
    }
    else
    {
        pCont1->pFirstBlock = NULL;
        pCont1->pLastBlock  = NULL;
        pCont1->pCurBlock   = NULL;
    }
}

/*************************************************************************
|*
|*    Container::Container()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

Container::Container( USHORT _nBlockSize, USHORT _nInitSize, USHORT _nReSize )
{
    DBG_CTOR( Container, DbgCheckContainer );

    // BlockSize muss mindestens 4 sein und kleiner als 64 KB
    if ( _nBlockSize < 4 )
        nBlockSize = 4;
    else
    {
        if ( _nBlockSize < CONTAINER_MAXBLOCKSIZE )
            nBlockSize = _nBlockSize;
        else
            nBlockSize = CONTAINER_MAXBLOCKSIZE;
    }

    // ReSize muss mindestens 2 sein und kleiner als BlockSize
    if ( _nReSize >= nBlockSize )
        nReSize = nBlockSize;
    else
    {
        if ( _nReSize < 2 )
            nReSize = 2;
        else
            nReSize = _nReSize;

        // BlockSize muss ein vielfaches der Resizegroesse sein
        if ( nBlockSize % nReSize )
            nBlockSize -= nReSize - (nBlockSize % nReSize);
    }

    // InitSize muss groesser gleich ReSize sein und kleiner als BlockSize
    if ( _nInitSize <= nReSize )
        nInitSize = nReSize;
    else
    {
        if ( _nInitSize >= nBlockSize )
            nInitSize = nBlockSize;
        else
        {
            nInitSize = _nInitSize;

            // InitSize muss ein vielfaches der Resizegroesse sein
            if ( nInitSize % nReSize )
                nInitSize -= nReSize - (nInitSize % nReSize);
        }
    }

    // Werte initialisieren
    pFirstBlock = NULL;
    pLastBlock  = NULL;
    pCurBlock   = NULL;
    nCount      = 0;
    nCurIndex   = 0;
}

/*************************************************************************
|*
|*    Container::Container()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

Container::Container( ULONG nSize )
{
    DBG_CTOR( Container, DbgCheckContainer );

    nCount     = nSize;
    nCurIndex  = 0;
    nBlockSize = CONTAINER_MAXBLOCKSIZE;
    nInitSize  = 1;
    nReSize    = 1;

    if ( !nSize )
    {
        pFirstBlock = NULL;
        pLastBlock  = NULL;
        pCurBlock   = NULL;
    }
    else
    {
        // Muss mehr als ein Block angelegt werden
        if ( nSize <= nBlockSize )
        {
            pFirstBlock = new CBlock( (USHORT)nSize, NULL );
            pLastBlock = pFirstBlock;
        }
        else
        {
            CBlock* pBlock1;
            CBlock* pBlock2;

            pFirstBlock = new CBlock( nBlockSize, NULL );
            pBlock1 = pFirstBlock;
            nSize -= nBlockSize;

            // Solange die Blockgroesse ueberschritten wird, neue Bloecke anlegen
            while ( nSize > nBlockSize )
            {
                pBlock2 = new CBlock( nBlockSize, pBlock1 );
                pBlock1->SetNextBlock( pBlock2 );
                pBlock1 = pBlock2;
                nSize -= nBlockSize;
            }

            pLastBlock = new CBlock( (USHORT)nSize, pBlock1 );
            pBlock1->SetNextBlock( pLastBlock );
        }

        pCurBlock  = pFirstBlock;
    }
}

/*************************************************************************
|*
|*    Container::Container()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

Container::Container( const Container& r )
{
    DBG_CTOR( Container, DbgCheckContainer );

    // Daten kopieren
    ImpCopyContainer( this, &r );
}

/*************************************************************************
|*
|*    Container::~Container()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

Container::~Container()
{
    DBG_DTOR( Container, DbgCheckContainer );

    // Alle Bloecke loeschen
    CBlock* pBlock = pFirstBlock;
    while ( pBlock )
    {
        CBlock* pTemp = pBlock->GetNextBlock();
        delete pBlock;
        pBlock = pTemp;
    }
}

/*************************************************************************
|*
|*    Container::ImpInsert()
|*
|*    Beschreibung      Interne Methode zum Einfuegen eines Pointers
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  DV 01.07.97
|*
*************************************************************************/

void Container::ImpInsert( void* p, CBlock* pBlock, USHORT nIndex )
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    if ( !nCount )
    {
        if ( !pBlock )
        {
            pFirstBlock = new CBlock( nInitSize, NULL, NULL );
            pLastBlock  = pFirstBlock;
            pCurBlock   = pFirstBlock;
        }
        pFirstBlock->Insert( p, nIndex, nReSize );
    }
    else
    {
        // Ist im Block die maximale Blockgroesse erreicht,
        // dann neuen Block anlegen
        if ( pBlock->Count() == nBlockSize )
        {
            // Block auftrennen
            CBlock* pNewBlock = pBlock->Split( p, nIndex, nReSize );

            // Wurde Block dahinter angehaegnt
            if ( pBlock->pNext == pNewBlock )
            {
                // Gegebenenfalls LastBlock anpassen
                if ( pBlock == pLastBlock )
                    pLastBlock = pNewBlock;

                // Current-Position nachfuehren
                if ( pBlock == pCurBlock )
                {
                    if ( pBlock->nCount <= nCurIndex )
                    {
                        if ( nIndex <= nCurIndex )
                            nCurIndex++;
                        pCurBlock  = pNewBlock;
                        nCurIndex -= pBlock->nCount;
                    }
                }
            }
            else
            {
                // Gegebenenfalls FirstBlock anpassen
                if ( pBlock == pFirstBlock )
                    pFirstBlock = pNewBlock;

                // Current-Position nachfuehren
                if ( pBlock == pCurBlock )
                {
                    if ( nIndex <= nCurIndex )
                        nCurIndex++;
                    if ( pNewBlock->nCount <= nCurIndex )
                        nCurIndex -= pNewBlock->nCount;
                    else
                        pCurBlock = pNewBlock;
                }
            }
        }
        else
        {
            // Sonst reicht normales einfuegen in den Block
            pBlock->Insert( p, nIndex, nReSize );

            // Current-Position nachfuehren
            if ( (pBlock == pCurBlock) && (nIndex <= nCurIndex) )
                nCurIndex++;
        }
    }

    // Ein neues Item im Container
    nCount++;
}

/*************************************************************************
|*
|*    Container::Insert()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void Container::Insert( void* p )
{
    ImpInsert( p, pCurBlock, nCurIndex );
}

/*************************************************************************
|*
|*    Container::Insert()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void Container::Insert( void* p, ULONG nIndex )
{
    if ( nCount <= nIndex )
    {
        if ( pLastBlock )
            ImpInsert( p, pLastBlock, pLastBlock->Count() );
        else
            ImpInsert( p, NULL, 0 );
    }
    else
    {
        // Block suchen
        CBlock* pTemp = pFirstBlock;
        while ( pTemp->Count() < nIndex )
        {
            nIndex -= pTemp->Count();
            pTemp   = pTemp->GetNextBlock();
        }

        ImpInsert( p, pTemp, (USHORT)nIndex );
    }
}

/*************************************************************************
|*
|*    Container::Insert()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void Container::Insert( void* pNew, void* pOld )
{
    ULONG nIndex = GetPos( pOld );
    if ( nIndex != CONTAINER_ENTRY_NOTFOUND )
        Insert( pNew, nIndex );
}

/*************************************************************************
|*
|*    Container::ImpRemove()
|*
|*    Beschreibung      Interne Methode zum Entfernen eines Pointers
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::ImpRemove( CBlock* pBlock, USHORT nIndex )
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    void* pOld;

    // Ist Liste danach leer
    if ( nCount == 1 )
    {
        // Block und CurIndex zuruecksetzen
        pOld = pBlock->GetObject( nIndex );
        pBlock->Reset();
        nCurIndex = 0;
    }
    else
    {
        // Ist Block nach Remove leer
        if ( pBlock->Count() == 1 )
        {
            // dann Block entfernen und Block-Pointer umsetzen
            if ( pBlock->GetPrevBlock() )
                (pBlock->GetPrevBlock())->SetNextBlock( pBlock->GetNextBlock() );
            else
                pFirstBlock = pBlock->GetNextBlock();

            if ( pBlock->GetNextBlock() )
                (pBlock->GetNextBlock())->SetPrevBlock( pBlock->GetPrevBlock() );
            else
                pLastBlock = pBlock->GetPrevBlock();

            // Current-Position nachfuehren
            if ( pBlock == pCurBlock )
            {
                if ( pBlock->GetNextBlock() )
                {
                    pCurBlock = pBlock->GetNextBlock();
                    nCurIndex = 0;
                }
                else
                {
                    pCurBlock = pBlock->GetPrevBlock();
                    nCurIndex = pCurBlock->Count()-1;
                }
            }

            pOld = pBlock->GetObject( nIndex );
            delete pBlock;
        }
        else
        {
            // Sonst Item aus dem Block entfernen
            pOld = pBlock->Remove( nIndex, nReSize );

            // Current-Position nachfuehren
            if ( (pBlock == pCurBlock) &&
                 ((nIndex < nCurIndex) || ((nCurIndex == pBlock->Count()) && nCurIndex)) )
                nCurIndex--;
        }
    }

    // Jetzt gibt es ein Item weniger
    nCount--;

    // Und den Pointer zurueckgeben, der entfernt wurde
    return pOld;
}

/*************************************************************************
|*
|*    Container::Remove()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::Remove()
{
    // Wenn kein Item vorhanden ist, NULL zurueckgeben
    if ( !nCount )
        return NULL;
    else
        return ImpRemove( pCurBlock, nCurIndex );
}

/*************************************************************************
|*
|*    Container::Remove()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::Remove( ULONG nIndex )
{
    // Ist Index nicht innerhalb des Containers, dann NULL zurueckgeben
    if ( nCount <= nIndex )
        return NULL;
    else
    {
        // Block suchen
        CBlock* pTemp = pFirstBlock;
        while ( pTemp->Count() <= nIndex )
        {
            nIndex -= pTemp->Count();
            pTemp   = pTemp->GetNextBlock();
        }

        return ImpRemove( pTemp, (USHORT)nIndex );
    }
}

/*************************************************************************
|*
|*    Container::Replace()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::Replace( void* p )
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    if ( !nCount )
        return NULL;
    else
        return pCurBlock->Replace( p, nCurIndex );
}

/*************************************************************************
|*
|*    Container::Replace()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::Replace( void* p, ULONG nIndex )
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Ist Index nicht innerhalb des Containers, dann NULL zurueckgeben
    if ( nCount <= nIndex )
        return NULL;
    else
    {
        // Block suchen
        CBlock* pTemp = pFirstBlock;
        while ( pTemp->Count() <= nIndex )
        {
            nIndex -= pTemp->Count();
            pTemp   = pTemp->GetNextBlock();
        }

        return pTemp->Replace( p, (USHORT)nIndex );
    }
}

/*************************************************************************
|*
|*    Container::SetSize()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void Container::SetSize( ULONG nNewSize )
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    if ( nNewSize )
    {
        // Unterscheiden sich die Groessen
        if ( nNewSize != nCount )
        {
            CBlock* pTemp;
            ULONG   nTemp;

            // Wird verkleinert
            if ( nNewSize < nCount )
            {
                pTemp = pFirstBlock;
                nTemp = 0;
                while ( (nTemp+pTemp->Count()) < nNewSize )
                {
                    nTemp += pTemp->Count();
                    pTemp  = pTemp->GetNextBlock();
                }

                // Alle folgenden Bloecke loeschen
                BOOL    bLast = FALSE;
                CBlock* pDelNext;
                CBlock* pDelBlock = pTemp->GetNextBlock();
                while ( pDelBlock )
                {
                    // Muss CurrentBlock umgesetzt werden
                    if ( pDelBlock == pCurBlock )
                        bLast = TRUE;
                    pDelNext = pDelBlock->GetNextBlock();
                    delete pDelBlock;
                    pDelBlock = pDelNext;
                }

                // Block in der Groesse anpassen, oder bei Groesse 0 loeschen
                if ( nNewSize > nTemp )
                {
                    pLastBlock = pTemp;
                    pTemp->SetNextBlock( NULL );
                    pTemp->SetSize( (USHORT)(nNewSize-nTemp) );
                }
                else
                {
                    pLastBlock = pTemp->GetPrevBlock();
                    pLastBlock->SetNextBlock( NULL );
                    delete pTemp;
                }

                nCount = nNewSize;
                if ( bLast )
                {
                    pCurBlock = pLastBlock;
                    nCurIndex = pCurBlock->Count()-1;
                }
            }
            else
            {
                // Auf den letzen Puffer setzen
                pTemp = pLastBlock;
                nTemp = nNewSize - nCount;

                if ( !pTemp )
                {
                    // Muss mehr als ein Block angelegt werden
                    if ( nNewSize <= nBlockSize )
                    {
                        pFirstBlock = new CBlock( (USHORT)nNewSize, NULL );
                        pLastBlock = pFirstBlock;
                    }
                    else
                    {
                        CBlock* pBlock1;
                        CBlock* pBlock2;

                        pFirstBlock = new CBlock( nBlockSize, NULL );
                        pBlock1 = pFirstBlock;
                        nNewSize -= nBlockSize;

                        // Solange die Blockgroesse ueberschritten wird, neue Bloecke anlegen
                        while ( nNewSize > nBlockSize )
                        {
                            pBlock2 = new CBlock( nBlockSize, pBlock1 );
                            pBlock1->SetNextBlock( pBlock2 );
                            pBlock1 = pBlock2;
                            nNewSize -= nBlockSize;
                        }

                        pLastBlock = new CBlock( (USHORT)nNewSize, pBlock1 );
                        pBlock1->SetNextBlock( pLastBlock );
                    }

                    pCurBlock  = pFirstBlock;
                }
                // Reicht es, den letzen Puffer in der Groesse anzupassen
                else if ( (nTemp+pTemp->Count()) <= nBlockSize )
                    pTemp->SetSize( (USHORT)(nTemp+pTemp->Count()) );
                else
                {
                    // Puffer auf max. Blockgroesse setzen
                    nTemp -= nBlockSize - pTemp->GetSize();
                    pTemp->SetSize( nBlockSize );

                    CBlock* pTemp2;
                    // Solange die Blockgroesse ueberschritten wird,
                    // neue Bloecke anlegen
                    while ( nTemp > nBlockSize )
                    {
                        pTemp2 = new CBlock( nBlockSize, pTemp );
                        pTemp->SetNextBlock( pTemp2 );
                        pTemp = pTemp2;
                        nTemp -= nBlockSize;
                    }

                    // Den letzten Block anlegen
                    if ( nTemp )
                    {
                        pLastBlock = new CBlock( (USHORT)nTemp, pTemp );
                        pTemp->SetNextBlock( pLastBlock );
                    }
                    else
                        pLastBlock = pTemp;
                }

                nCount = nNewSize;
            }
        }
    }
    else
        Clear();
}

/*************************************************************************
|*
|*    Container::Clear()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void Container::Clear()
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Erst alle Bloecke loeschen
    CBlock* pBlock = pFirstBlock;
    while ( pBlock )
    {
        CBlock* pTemp = pBlock->GetNextBlock();
        delete pBlock;
        pBlock = pTemp;
    }

    // Werte zuruecksetzen
    pFirstBlock = NULL;
    pLastBlock  = NULL;
    pCurBlock   = NULL;
    nCount      = 0;
    nCurIndex   = 0;
}

/*************************************************************************
|*
|*    Container::GetCurObject()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::GetCurObject() const
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // NULL, wenn Container leer
    if ( !nCount )
        return NULL;
    else
        return pCurBlock->GetObject( nCurIndex );
}

/*************************************************************************
|*
|*    Container::GetCurPos()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

ULONG Container::GetCurPos() const
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // CONTAINER_ENTRY_NOTFOUND, wenn Container leer
    if ( !nCount )
        return CONTAINER_ENTRY_NOTFOUND;
    else
    {
        // Block suchen
        CBlock* pTemp = pFirstBlock;
        ULONG   nTemp = 0;
        while ( pTemp != pCurBlock )
        {
            nTemp += pTemp->Count();
            pTemp  = pTemp->GetNextBlock();
        }

        return nTemp+nCurIndex;
    }
}

/*************************************************************************
|*
|*    Container::GetObjectPtr()
|*
|*    Beschreibung      Interne Methode fuer Referenz-Container
|*    Ersterstellung    TH 26.01.93
|*    Letzte Aenderung  TH 26.01.93
|*
*************************************************************************/

void** Container::GetObjectPtr( ULONG nIndex )
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Ist Index nicht innerhalb des Containers, dann NULL zurueckgeben
    if ( nCount <= nIndex )
        return NULL;
    else
    {
        // Block suchen
        CBlock* pTemp = pFirstBlock;
        while ( pTemp->Count() <= nIndex )
        {
            nIndex -= pTemp->Count();
            pTemp   = pTemp->GetNextBlock();
        }

        // Item innerhalb des gefundenen Blocks zurueckgeben
        return pTemp->GetObjectPtr( (USHORT)nIndex );
    }
}

/*************************************************************************
|*
|*    Container::GetObject()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::GetObject( ULONG nIndex ) const
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Ist Index nicht innerhalb des Containers, dann NULL zurueckgeben
    if ( nCount <= nIndex )
        return NULL;
    else
    {
        // Block suchen
        CBlock* pTemp = pFirstBlock;
        while ( pTemp->Count() <= nIndex )
        {
            nIndex -= pTemp->Count();
            pTemp   = pTemp->GetNextBlock();
        }

        // Item innerhalb des gefundenen Blocks zurueckgeben
        return pTemp->GetObject( (USHORT)nIndex );
    }
}

/*************************************************************************
|*
|*    Container::GetPos()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

ULONG Container::GetPos( const void* p ) const
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    void**  pNodes;
    CBlock* pTemp;
    ULONG   nTemp;
    USHORT  nBlockCount;
    USHORT  i;

    // Block suchen
    pTemp = pFirstBlock;
    nTemp = 0;
    while ( pTemp )
    {
        pNodes = pTemp->GetNodes();
        i = 0;
        nBlockCount = pTemp->Count();
        while ( i < nBlockCount )
        {
            if ( p == *pNodes )
                return nTemp+i;
            pNodes++;
            i++;
        }
        nTemp += nBlockCount;
        pTemp  = pTemp->GetNextBlock();
    }

    return CONTAINER_ENTRY_NOTFOUND;
}

/*************************************************************************
|*
|*    Container::GetPos()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 14.09.94
|*    Letzte Aenderung  TH 14.09.94
|*
*************************************************************************/

ULONG Container::GetPos( const void* p, ULONG nStartIndex,
                         BOOL bForward ) const
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Ist Index nicht innerhalb des Containers, dann NOTFOUND zurueckgeben
    if ( nCount <= nStartIndex )
        return CONTAINER_ENTRY_NOTFOUND;
    else
    {
        void**  pNodes;
        USHORT  nBlockCount;
        USHORT  i;

        // Block suchen
        CBlock* pTemp = pFirstBlock;
        ULONG   nTemp = 0;
        while ( nTemp+pTemp->Count() <= nStartIndex )
        {
            nTemp += pTemp->Count();
            pTemp  = pTemp->GetNextBlock();
        }

        // Jetzt den Pointer suchen
        if ( bForward )
        {
            // Alle Bloecke durchrsuchen
            i = (USHORT)(nStartIndex - nTemp);
            pNodes = pTemp->GetObjectPtr( i );
            do
            {
                nBlockCount = pTemp->Count();
                while ( i < nBlockCount )
                {
                    if ( p == *pNodes )
                        return nTemp+i;
                    pNodes++;
                    i++;
                }
                nTemp += nBlockCount;
                pTemp  = pTemp->GetNextBlock();
                if ( pTemp )
                {
                    i = 0;
                    pNodes = pTemp->GetNodes();
                }
                else
                    break;
            }
            while ( TRUE );
        }
        else
        {
            // Alle Bloecke durchrsuchen
            i = (USHORT)(nStartIndex-nTemp)+1;
            pNodes = pTemp->GetObjectPtr( i-1 );
            do
            {
                do
                {
                    if ( p == *pNodes )
                        return nTemp+i-1;
                    pNodes--;
                    i--;
                }
                while ( i );
                nTemp -= pTemp->Count();
                pTemp  = pTemp->GetPrevBlock();
                if ( pTemp )
                {
                    i = pTemp->Count();
                    // Leere Bloecke in der Kette darf es nicht geben. Nur
                    // wenn ein Block existiert, darf dieser leer sein
                    pNodes = pTemp->GetObjectPtr( i-1 );
                }
                else
                    break;
            }
            while ( TRUE );
        }
    }

    return CONTAINER_ENTRY_NOTFOUND;
}

/*************************************************************************
|*
|*    Container::Seek()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::Seek( ULONG nIndex )
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Ist der Container leer, dann NULL zurueckgeben
    if ( nCount <= nIndex )
        return NULL;
    else
    {
        // Block suchen
        CBlock* pTemp = pFirstBlock;
        while ( pTemp->Count() <= nIndex )
        {
            nIndex -= pTemp->Count();
            pTemp   = pTemp->GetNextBlock();
        }

        // Item innerhalb des gefundenen Blocks zurueckgeben
        pCurBlock = pTemp;
        nCurIndex = (USHORT)nIndex;
        return pCurBlock->GetObject( nCurIndex );
    }
}

/*************************************************************************
|*
|*    Container::First()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::First()
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Ist Container leer, dann NULL zurueckgeben
    if ( !nCount )
        return NULL;
    else
    {
        // Block und Index setzen und ersten Pointer zurueckgeben
        pCurBlock = pFirstBlock;
        nCurIndex = 0;
        return pCurBlock->GetObject( nCurIndex );
    }
}

/*************************************************************************
|*
|*    Container::Last()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::Last()
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Ist Container leer, dann NULL zurueckgeben
    if ( !nCount )
        return NULL;
    else
    {
        // Block und Index setzen und ersten Pointer zurueckgeben
        pCurBlock = pLastBlock;
        nCurIndex = pCurBlock->Count()-1;
        return pCurBlock->GetObject( nCurIndex );
    }
}

/*************************************************************************
|*
|*    Container::Next()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::Next()
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Ist Container leer, dann NULL zurueckgeben, ansonsten preufen ob
    // naechste Position noch im aktuellen Block ist. Falls nicht, dann
    // einen Block weiterschalten (geht ohne Gefahr, da leere Bloecke
    // nicht vorkommen duerfen, es sein denn, es ist der einzige).
    if ( !nCount )
        return NULL;
    else if ( (nCurIndex+1) < pCurBlock->Count() )
        return pCurBlock->GetObject( ++nCurIndex );
    else if ( pCurBlock->GetNextBlock() )
    {
        pCurBlock = pCurBlock->GetNextBlock();
        nCurIndex = 0;
        return pCurBlock->GetObject( nCurIndex );
    }
    else
        return NULL;
}

/*************************************************************************
|*
|*    Container::Prev()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

void* Container::Prev()
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Ist Container leer, dann NULL zurueckgeben, ansonsten preufen ob
    // vorherige Position noch im aktuellen Block ist. Falls nicht, dann
    // einen Block zurueckschalten (geht ohne Gefahr, da leere Bloecke
    // nicht vorkommen duerfen, es sein denn, es ist der einzige).
    if ( !nCount )
        return NULL;
    else if ( nCurIndex )
        return pCurBlock->GetObject( --nCurIndex );
    else if ( pCurBlock->GetPrevBlock() )
    {
        pCurBlock = pCurBlock->GetPrevBlock();
        nCurIndex = pCurBlock->Count() - 1;
        return pCurBlock->GetObject( nCurIndex );
    }
    else
        return NULL;
}

/*************************************************************************
|*
|*    Container::operator =()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

Container& Container::operator =( const Container& r )
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // Erst alle Bloecke loeschen
    CBlock* pBlock = pFirstBlock;
    while ( pBlock )
    {
        CBlock* pTemp = pBlock->GetNextBlock();
        delete pBlock;
        pBlock = pTemp;
    }

    // Daten kopieren
    ImpCopyContainer( this, &r );
    return *this;
}

/*************************************************************************
|*
|*    Container::operator ==()
|*
|*    Beschreibung      CONTNR.SDW
|*    Ersterstellung    TH 17.09.91
|*    Letzte Aenderung  TH 17.09.91
|*
*************************************************************************/

BOOL Container::operator ==( const Container& r ) const
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    if ( nCount != r.nCount )
        return FALSE;

    ULONG i = 0;
    while ( i < nCount )
    {
        if ( GetObject( i ) != r.GetObject( i ) )
            return FALSE;
        i++;
    }

    return TRUE;
}
