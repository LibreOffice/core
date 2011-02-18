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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_soltools.hxx"

#include "hashtbl.hxx"
#include <string.h>

// -------------------------------------------------------------
// class HashItem
//
class HashItem
{
    enum ETag { TAG_EMPTY, TAG_USED, TAG_DELETED };

    void*   m_pObject;
    ETag    m_Tag;
    char*   m_Key;

public:
    HashItem() { m_Tag = TAG_EMPTY; m_Key = NULL; m_pObject = NULL; }
    ~HashItem() { delete [] m_Key; }

    bool IsDeleted() const
    {   return m_Tag == TAG_DELETED; }

    bool IsEmpty() const
    {   return m_Tag == TAG_DELETED || m_Tag == TAG_EMPTY; }

    bool IsFree() const
    {   return m_Tag == TAG_EMPTY; }

    bool IsUsed() const
    {   return m_Tag == TAG_USED; }

    void Delete()
    { m_Tag = TAG_DELETED; delete [] m_Key; m_Key = new char[ 1 ]; m_Key[ 0 ] = 0; m_pObject = NULL; }

    const char *GetKey() const
    { return m_Key; }

    void* GetObject() const
    { return m_pObject; }

    void SetObject(const char * Key, void *pObject)
    { m_Tag = TAG_USED; delete [] m_Key; m_Key = new char[ strlen( Key ) + 1 ]; strcpy( m_Key, Key ); m_pObject = pObject; }
};

#define MIN(a,b) (a)<(b)?(a):(b)
#define MAX(a,b) (a)>(b)?(a):(b)

// -------------------------------------------------------------
// class HashTable
//

/*static*/ double HashTable::m_defMaxLoadFactor = 0.5;
/*static*/ double HashTable::m_defDefGrowFactor = 2.0;

HashTable::HashTable(unsigned long lSize, bool bOwner, double dMaxLoadFactor, double dGrowFactor)
{
    m_lSize          = lSize;
    m_bOwner         = bOwner;
    m_lElem          = 0;
    m_dMaxLoadFactor = MAX(0.5,MIN(1.0,dMaxLoadFactor));  // 0.5 ... 1.0
    m_dGrowFactor    = MAX(2.0,MIN(5.0,dGrowFactor));     // 1.3 ... 5.0
    m_pData          = new HashItem [lSize];
}

HashTable::~HashTable()
{
    // Wenn die HashTable der Owner der Objecte ist,
    // müssen die Destruktoren separat gerufen werden.
    // Dies geschieht über die virtuelle Methode OnDeleteObject()
    //
    // Problem: Virtuelle Funktionen sind im Destructor nicht virtuell!!
    //          Der Code muß deshalb ins Macro

    // Speicher für HashItems freigeben
    delete [] m_pData;
}

void* HashTable::GetObjectAt(unsigned long lPos) const
// Gibt Objekt zurück, wenn es eines gibt, sonst NULL;
{
    HashItem *pItem = &m_pData[lPos];

    return pItem->IsUsed() ? pItem->GetObject() : NULL;
}

void HashTable::OnDeleteObject(void*)
{
}

unsigned long HashTable::Hash(const char *Key) const
{
    // Hashfunktion von P.J. Weinberger
    // aus dem "Drachenbuch" von Aho/Sethi/Ullman
    unsigned long i,n;
    unsigned long h = 0;
    unsigned long g = 0;

    for (i=0,n=strlen( Key ); i<n; i++)
    {
        h = (h<<4) + (unsigned long)(unsigned short)Key[i];
        g = h & 0xf0000000;

        if (g != 0)
        {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }

    return h % m_lSize;
}

unsigned long HashTable::DHash(const char* Key, unsigned long lOldHash) const
{
    unsigned long lHash = lOldHash;
    unsigned long i,n;

    for (i=0,n=strlen( Key ); i<n; i++)
    {
        lHash *= 256L;
        lHash += (unsigned long)(unsigned short)Key[i];
        lHash %= m_lSize;
    }
    return lHash;
}

unsigned long HashTable::Probe(unsigned long lPos) const
// gibt den Folgewert von lPos zurück
{
    lPos++; if (lPos==m_lSize) lPos=0;
    return lPos;
}

bool HashTable::IsFull() const
{
    return m_lElem>=m_lSize;
}

bool HashTable::Insert(const char * Key, void* pObject)
// pre:  Key ist nicht im Dictionary enthalten, sonst return FALSE
//       Dictionary ist nicht voll, sonst return FALSE
// post: pObject ist unter Key im Dictionary; m_nElem wurde erhöht
{
    SmartGrow();

    if (IsFull())
    {
        return false;
    }

    if (FindPos(Key) != NULL )
        return false;

    unsigned long     lPos  = Hash(Key);
    HashItem *pItem = &m_pData[lPos];

    // first hashing
    //
    if (pItem->IsEmpty())
    {
        pItem->SetObject(Key, pObject);
        m_lElem++;

        return true;
    }

    // double hashing
    //
    lPos  = DHash(Key,lPos);
    pItem = &m_pData[lPos];

    if (pItem->IsEmpty())
    {
        pItem->SetObject(Key, pObject);
        m_lElem++;

        return true;
    }

    // linear probing
    //
    do
    {
        lPos  = Probe(lPos);
        pItem = &m_pData[lPos];
    }
    while(!pItem->IsEmpty());

    pItem->SetObject(Key, pObject);
    m_lElem++;
    return true;
}

HashItem* HashTable::FindPos(const char * Key) const
// sucht den Key; gibt Refrenz auf den Eintrag (gefunden)
// oder NULL (nicht gefunden) zurück
//
// pre:  -
// post: -
{
    // first hashing
    //
    unsigned long     lPos  = Hash(Key);
    HashItem *pItem = &m_pData[lPos];

    if (pItem->IsUsed()
    &&  !(strcmp( pItem->GetKey(), Key )))
    {
        return pItem;
    }

    // double hashing
    //
    if (pItem->IsDeleted() || pItem->IsUsed())
    {
        lPos  = DHash(Key,lPos);
        pItem = &m_pData[lPos];

        if (pItem->IsUsed()
        &&  (!strcmp( pItem->GetKey(), Key)))
        {
            return pItem;
        }

        // linear probing
        //
        if (pItem->IsDeleted() || pItem->IsUsed())
        {
            unsigned long n      = 0;
            bool  bFound = false;
            bool  bEnd   = false;

            do
            {
                n++;
                lPos   = Probe(lPos);
                pItem  = &m_pData[lPos];

                bFound =  pItem->IsUsed()
                       && !( strcmp( pItem->GetKey(), Key ));

                bEnd = !(n<m_lSize || pItem->IsFree());
            }
            while(!bFound && !bEnd);

            return bFound ? pItem : NULL;
        }
    }

    // nicht gefunden
    //
    return NULL;
}

void* HashTable::Find(const char *Key) const
// Gibt Verweis des Objektes zurück, das unter Key abgespeichert ist,
// oder NULL wenn nicht vorhanden.
//
// pre:  -
// post: -
{
    HashItem *pItem = FindPos(Key);

    if (pItem != NULL
    && ( !strcmp( pItem->GetKey(), Key )))
        return pItem->GetObject();
    else
        return NULL;
}

void* HashTable::Delete( const char * Key)
// Löscht Objekt, das unter Key abgespeichert ist und gibt Verweis
// darauf zurück.
// Gibt NULL zurück, wenn Key nicht vorhanden ist.
//
// pre:  -
// post: Objekt ist nicht mehr enthalten; m_lElem dekrementiert
//       Wenn die HashTable der Owner ist, wurde das Object gelöscht
{
    HashItem *pItem = FindPos(Key);

    if (pItem != NULL
    &&  ( !strcmp( pItem->GetKey(), Key )))
    {
        void* pObject = pItem->GetObject();

        if (m_bOwner)
            OnDeleteObject(pObject);

        pItem->Delete();
        m_lElem--;
        return pObject;
    }
    else
    {
        return NULL;
    }
}

double HashTable::CalcLoadFactor() const
// prozentuale Belegung der Hashtabelle berechnen
{
    return double(m_lElem) / double(m_lSize);
}

void HashTable::SmartGrow()
// Achtung: da die Objekte umkopiert werden, darf die OnDeleteObject-Methode
//          nicht gerufen werden
{
    double dLoadFactor = CalcLoadFactor();

    if (dLoadFactor <= m_dMaxLoadFactor)
        return; // nothing to grow

    unsigned long     lOldSize = m_lSize;              // alte Daten sichern
    HashItem* pOldData = m_pData;

    m_lSize = (unsigned long) (m_dGrowFactor * m_lSize); // neue Größe
    m_pData = new HashItem[m_lSize];           // neue Daten holen

    // kein Speicher:
    // Zustand "Tabelle voll" wird in Insert abgefangen
    //
    if (m_pData == NULL)
    {
        m_lSize = lOldSize;
        m_pData = pOldData;
        return;
    }

    m_lElem = 0;                               // noch keine neuen Daten

    // Umkopieren der Daten
    //
    for (unsigned long i=0; i<lOldSize; i++)
    {
        HashItem *pItem = &pOldData[i];

        if (pItem->IsUsed())
            Insert(pItem->GetKey(),pItem->GetObject());
    }

    delete [] pOldData;
}

// Iterator ---------------------------------------------------------
//

HashTableIterator::HashTableIterator(HashTable const& aTable)
: m_aTable(aTable)
{
    m_lAt = 0;
}

void* HashTableIterator::GetFirst()
{
    m_lAt = 0;
    return FindValidObject(true /* forward */);
}

void* HashTableIterator::GetLast()
{
    m_lAt = m_aTable.GetSize() -1;
    return FindValidObject(false /* backward */);
}

void* HashTableIterator::GetNext()
{
    if (m_lAt+1 >= m_aTable.GetSize())
        return NULL;

    m_lAt++;
    return FindValidObject(true /* forward */);
}

void* HashTableIterator::GetPrev()
{
    if (m_lAt <= 0)
        return NULL;

    m_lAt--;
    return FindValidObject(false /* backward */);
}

void* HashTableIterator::FindValidObject(bool bForward)
// Sucht nach einem vorhandenen Objekt ab der aktuellen
// Position.
//
// pre:  ab inkl. m_lAt soll die Suche beginnen
// post: if not found then
//          if bForward == TRUE then
//              m_lAt == m_aTable.GetSize() -1
//          else
//              m_lAt == 0
//       else
//          m_lAt ist die gefundene Position
{
    void *pObject = m_aTable.GetObjectAt(m_lAt);

    if (pObject != NULL)
        return pObject;

    while (pObject == NULL
       && (bForward ? ((m_lAt+1) < m_aTable.GetSize())
                    :   m_lAt    > 0))
    {
        if (bForward)
            m_lAt++;
        else
            m_lAt--;

        pObject = m_aTable.GetObjectAt(m_lAt);
    }

    return pObject;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
