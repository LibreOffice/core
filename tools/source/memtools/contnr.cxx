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


#include <limits.h>

#include <string.h>

#include <stdio.h>
#include <tools/solar.h>
#include <impcont.hxx>
#include <tools/contnr.hxx>
#include <tools/debug.hxx>

// -----------------------------------------------------------------------

DBG_NAME( CBlock )
DBG_NAME( Container )

/*************************************************************************
|*
|*    DbgCheckCBlock()
|*
|*    Beschreibung      Pruefung eines CBlock fuer Debug-Utilities
|*
*************************************************************************/

#ifdef DBG_UTIL
const char* CBlock::DbgCheckCBlock( const void* pBlock )
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
|*
*************************************************************************/

CBlock::CBlock( sal_uInt16 nInitSize, CBlock* _pPrev, CBlock* _pNext )
{
    DBG_CTOR( CBlock, DbgCheckCBlock );

    pPrev   = _pPrev;
    pNext   = _pNext;
    nSize   = nInitSize;
    nCount  = 0;

    // Datenpuffer anlegen
    pNodes = new PVOID[nSize];
}

/*************************************************************************
|*
|*    CBlock::CBlock()
|*
|*    Beschreibung      Construktor des Verwaltungsblocks
|*
*************************************************************************/

CBlock::CBlock( sal_uInt16 _nSize, CBlock* _pPrev )
{
    DBG_CTOR( CBlock, DbgCheckCBlock );
    DBG_ASSERT( _nSize, "CBlock::CBlock(): nSize == 0" );

    pPrev   = _pPrev;
    pNext   = NULL;
    nSize   = _nSize;
    nCount  = _nSize;

    // Datenpuffer anlegen und initialisieren
    pNodes = new PVOID[nSize];
    memset( pNodes, 0, nSize*sizeof(PVOID) );
}

/*************************************************************************
|*
|*    CBlock::CBlock()
|*
|*    Beschreibung      Copy-Construktor des Verwaltungsblocks
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
    pNodes = new PVOID[nSize];
    memcpy( pNodes, r.pNodes, nCount*sizeof(PVOID) );
}

/*************************************************************************
|*
|*    CBlock::~CBlock()
|*
|*    Beschreibung      Destruktor des Verwaltungsblocks
|*
*************************************************************************/

inline CBlock::~CBlock()
{
    DBG_DTOR( CBlock, DbgCheckCBlock );

    // Daten loeschen
    delete[] pNodes;
}

/*************************************************************************
|*
|*    CBlock::Insert()
|*
|*    Beschreibung      Fuegt einen Pointer ein
|*
*************************************************************************/

void CBlock::Insert( void* p, sal_uInt16 nIndex, sal_uInt16 nReSize )
{
    DBG_CHKTHIS( CBlock, DbgCheckCBlock );
    DBG_ASSERT( nIndex <= nCount, "CBlock::Insert(): Index > nCount" );

    // Muss Block realokiert werden
    if ( nCount == nSize )
    {
        // Neue Daten anlegen
        nSize = nSize + nReSize;    // MSVC warns here if += is used
        void** pNewNodes = new PVOID[nSize];

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
        delete[] pNodes;
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
|*
*************************************************************************/

CBlock* CBlock::Split( void* p, sal_uInt16 nIndex, sal_uInt16 nReSize )
{
    DBG_CHKTHIS( CBlock, DbgCheckCBlock );

    sal_uInt16  nNewSize;
    sal_uInt16  nMiddle;
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
                nNewSize = nNewSize + nReSize;  // MSVC warns here if += is used
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
            nIndex = nIndex - nMiddle;  // MSVC warns here if += is used
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
                void** pNewNodes = new PVOID[nNewSize];
                memcpy( pNewNodes, pNodes, nCount*sizeof(PVOID) );
                delete[] pNodes;
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
            nCount = nCount - nMiddle;  // MSVC warns here if += is used

            // Die zweite Haelfte in einen neuen Block kopieren
            if ( nSize != nNewSize )
            {
                void** pNewNodes = new PVOID[nNewSize];
                memcpy( pNewNodes, pNodes+nMiddle, nCount*sizeof(PVOID) );
                delete[] pNodes;
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
|*
*************************************************************************/

void* CBlock::Remove( sal_uInt16 nIndex, sal_uInt16 nReSize )
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
        nSize = nSize - nReSize;    // MSVC warns here if += is used
        void** pNewNodes = new PVOID[nSize];

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
        delete[] pNodes;
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
|*
*************************************************************************/

inline void* CBlock::Replace( void* p, sal_uInt16 nIndex )
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
|*
*************************************************************************/

inline void** CBlock::GetObjectPtr( sal_uInt16 nIndex )
{
    DBG_CHKTHIS( CBlock, DbgCheckCBlock );

    return &(pNodes[nIndex]);
}

//------------------------------------------------------------------------

/*************************************************************************
|*
|*    DbgCheckContainer()
|*
|*    Beschreibung      Pruefung eines Container fuer Debug-Utilities
|*
*************************************************************************/

#ifdef DBG_UTIL
const char* Container::DbgCheckContainer( const void* pCont )
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
|*
*************************************************************************/

void Container::ImpCopyContainer( const Container* pCont2 )
{
    // Werte vom uebergebenen Container uebernehmen
    nCount     = pCont2->nCount;
    nCurIndex  = pCont2->nCurIndex;
    nInitSize  = pCont2->nInitSize;
    nReSize    = pCont2->nReSize;
    nBlockSize = pCont2->nBlockSize;

    // Alle Bloecke kopieren
    if ( pCont2->nCount )
    {
        CBlock* pBlock1;
        CBlock* pBlock2;
        CBlock* pTempBlock;

        // Erstmal ersten Block kopieren
        pBlock2 = pCont2->pFirstBlock;
        pFirstBlock = new CBlock( *pBlock2, NULL );
        // Ist erster Block der Current-Block, dann Current-Block setzen
        if ( pBlock2 == pCont2->pCurBlock )
            pCurBlock = pFirstBlock;
        pBlock1 = pFirstBlock;
        pBlock2 = pBlock2->GetNextBlock();
        while ( pBlock2 )
        {
            // Neuen Block anlegen und aus der uebergebenen Liste kopieren
            pTempBlock = new CBlock( *pBlock2, pBlock1 );
            pBlock1->SetNextBlock( pTempBlock );
            pBlock1 = pTempBlock;

            // Current-Block beruecksichtigen
            if ( pBlock2 == pCont2->pCurBlock )
                pCurBlock = pBlock1;

            // Auf naechsten Block weitersetzen
            pBlock2 = pBlock2->GetNextBlock();
        }

        // Letzten Block setzen
        pLastBlock = pBlock1;
    }
    else
    {
        pFirstBlock = NULL;
        pLastBlock  = NULL;
        pCurBlock   = NULL;
    }
}

/*************************************************************************
|*
|*    Container::Container()
|*
*************************************************************************/

Container::Container( sal_uInt16 _nBlockSize, sal_uInt16 _nInitSize, sal_uInt16 _nReSize )
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
*************************************************************************/

Container::Container( sal_uIntPtr nSize )
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
            pFirstBlock = new CBlock( (sal_uInt16)nSize, NULL );
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

            pLastBlock = new CBlock( (sal_uInt16)nSize, pBlock1 );
            pBlock1->SetNextBlock( pLastBlock );
        }

        pCurBlock  = pFirstBlock;
    }
}

/*************************************************************************
|*
|*    Container::Container()
|*
*************************************************************************/

Container::Container( const Container& r )
{
    DBG_CTOR( Container, DbgCheckContainer );

    // Daten kopieren
    ImpCopyContainer( &r );
}

/*************************************************************************
|*
|*    Container::~Container()
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
|*
*************************************************************************/

void Container::ImpInsert( void* p, CBlock* pBlock, sal_uInt16 nIndex )
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
                        nCurIndex = nCurIndex - pBlock->nCount; // MSVC warns here if += is used
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
                        nCurIndex = nCurIndex - pNewBlock->nCount;  // MSVC warns here if += is used
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
*************************************************************************/

void Container::Insert( void* p )
{
    ImpInsert( p, pCurBlock, nCurIndex );
}

/*************************************************************************
|*
|*    Container::Insert()
|*
*************************************************************************/

void Container::Insert( void* p, sal_uIntPtr nIndex )
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

        ImpInsert( p, pTemp, (sal_uInt16)nIndex );
    }
}

/*************************************************************************
|*
|*    Container::ImpRemove()
|*
|*    Beschreibung      Interne Methode zum Entfernen eines Pointers
|*
*************************************************************************/

void* Container::ImpRemove( CBlock* pBlock, sal_uInt16 nIndex )
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
*************************************************************************/

void* Container::Remove( sal_uIntPtr nIndex )
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

        return ImpRemove( pTemp, (sal_uInt16)nIndex );
    }
}

/*************************************************************************
|*
|*    Container::Replace()
|*
*************************************************************************/

void* Container::Replace( void* p, sal_uIntPtr nIndex )
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

        return pTemp->Replace( p, (sal_uInt16)nIndex );
    }
}

/*************************************************************************
|*
|*    Container::Clear()
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
*************************************************************************/

sal_uIntPtr Container::GetCurPos() const
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    // CONTAINER_ENTRY_NOTFOUND, wenn Container leer
    if ( !nCount )
        return CONTAINER_ENTRY_NOTFOUND;
    else
    {
        // Block suchen
        CBlock* pTemp = pFirstBlock;
        sal_uIntPtr nTemp = 0;
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
|*    Container::GetObject()
|*
*************************************************************************/

void* Container::GetObject( sal_uIntPtr nIndex ) const
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
        return pTemp->GetObject( (sal_uInt16)nIndex );
    }
}

/*************************************************************************
|*
|*    Container::GetPos()
|*
*************************************************************************/

sal_uIntPtr Container::GetPos( const void* p ) const
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    void**  pNodes;
    CBlock* pTemp;
    sal_uIntPtr nTemp;
    sal_uInt16  nBlockCount;
    sal_uInt16  i;

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
|*    Container::Seek()
|*
*************************************************************************/

void* Container::Seek( sal_uIntPtr nIndex )
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
        nCurIndex = (sal_uInt16)nIndex;
        return pCurBlock->GetObject( nCurIndex );
    }
}

/*************************************************************************
|*
|*    Container::First()
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
    ImpCopyContainer( &r );
    return *this;
}

/*************************************************************************
|*
|*    Container::operator ==()
|*
*************************************************************************/

sal_Bool Container::operator ==( const Container& r ) const
{
    DBG_CHKTHIS( Container, DbgCheckContainer );

    if ( nCount != r.nCount )
        return sal_False;

    sal_uIntPtr i = 0;
    while ( i < nCount )
    {
        if ( GetObject( i ) != r.GetObject( i ) )
            return sal_False;
        i++;
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
