/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <tools/gen.hxx>
#include <tools/debug.hxx>
#include <soldep/hashtbl.hxx>

// -------------------------------------------------------------
// class HashItem
//
class HashItem
{
    enum ETag { TAG_EMPTY, TAG_USED, TAG_DELETED };

    void*   m_pObject;
    ETag    m_Tag;
    ByteString  m_Key;

public:
    HashItem() { m_Tag = TAG_EMPTY; m_pObject = NULL; }

    sal_Bool IsDeleted() const
    {   return m_Tag == TAG_DELETED; }

    sal_Bool IsEmpty() const
    {   return m_Tag == TAG_DELETED || m_Tag == TAG_EMPTY; }

    sal_Bool IsFree() const
    {   return m_Tag == TAG_EMPTY; }

    sal_Bool IsUsed() const
    {   return m_Tag == TAG_USED; }

    void Delete()
    { m_Tag = TAG_DELETED; m_Key = ""; m_pObject = NULL; }

    ByteString const& GetKey() const
    { return m_Key; }

    void* GetObject() const
    { return m_pObject; }

    void SetObject(ByteString const Key, void *pObject)
    { m_Tag = TAG_USED; m_Key = Key; m_pObject = pObject; }
};

#define MIN(a,b) (a)<(b)?(a):(b)
#define MAX(a,b) (a)>(b)?(a):(b)

// -------------------------------------------------------------
// class HashTable
//

/*static*/ double HashTable::m_defMaxLoadFactor = 0.8;
/*static*/ double HashTable::m_defDefGrowFactor = 2.0;

HashTable::HashTable(sal_uIntPtr lSize, sal_Bool bOwner, double dMaxLoadFactor, double dGrowFactor)
{
    m_lSize          = lSize;
    m_bOwner         = bOwner;
    m_lElem          = 0;
    m_dMaxLoadFactor = MAX(0.5,MIN(1.0,dMaxLoadFactor));  // 0.5 ... 1.0
    m_dGrowFactor    = MAX(1.3,MIN(5.0,dGrowFactor));     // 1.3 ... 5.0
    m_pData          = new HashItem [lSize];

// Statistik
#ifdef DBG_UTIL
    m_aStatistic.m_lSingleHash = 0;
    m_aStatistic.m_lDoubleHash = 0;
    m_aStatistic.m_lProbe      = 0;
#endif
}

HashTable::~HashTable()
{
    // Wenn die HashTable der Owner der Objecte ist,
    // müssen die Destruktoren separat gerufen werden.
    // Dies geschieht über die virtuelle Methode OnDeleteObject()
    //
    // Problem: Virtuelle Funktionen sind im Destructor nicht virtuell!!
    //          Der Code muß deshalb ins Macro

    /*
    if (m_bOwner)
    {
        for (sal_uIntPtr i=0; i<GetSize(); i++)
        {
            void *pObject = GetObjectAt(i);

            if (pObject != NULL)
                OnDeleteObject(pObject());
        }
    }
    */

    // Speicher für HashItems freigeben
    delete [] m_pData;
}

void* HashTable::GetObjectAt(sal_uIntPtr lPos) const
// Gibt Objekt zurück, wenn es eines gibt, sonst NULL;
{
    DBG_ASSERT(lPos<m_lSize, "HashTable::GetObjectAt()");

    HashItem *pItem = &m_pData[lPos];

    return pItem->IsUsed() ? pItem->GetObject() : NULL;
}

void HashTable::OnDeleteObject(void*)
{
    DBG_ERROR("HashTable::OnDeleteObject(void*) nicht überladen");
}

sal_uIntPtr HashTable::Hash(ByteString const& Key) const
{
    /*
    sal_uIntPtr lHash = 0;
    sal_uIntPtr i,n;

    for (i=0,n=Key.Len(); i<n; i++)
    {
        lHash *= 256L;
        lHash += (sal_uIntPtr)(sal_uInt16)Key.GetStr()[i];
        lHash %= m_lSize;
    }
    return lHash;
    */

    // Hashfunktion von P.J. Weinberger
    // aus dem "Drachenbuch" von Aho/Sethi/Ullman
    sal_uIntPtr i,n;
    sal_uIntPtr h = 0;
    sal_uIntPtr g = 0;

    for (i=0,n=Key.Len(); i<n; i++)
    {
        h = (h<<4) + (sal_uIntPtr)(sal_uInt16)Key.GetBuffer()[i];
        g = h & 0xf0000000;

        if (g != 0)
        {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }

    return h % m_lSize;
}

sal_uIntPtr HashTable::DHash(ByteString const& Key, sal_uIntPtr lOldHash) const
{
    sal_uIntPtr lHash = lOldHash;
    sal_uIntPtr i,n;

    for (i=0,n=Key.Len(); i<n; i++)
    {
        lHash *= 256L;
        lHash += (sal_uIntPtr)(sal_uInt16)Key.GetBuffer()[i];
        lHash %= m_lSize;
    }
    return lHash;

/*    return
        (
            lHash
        +   (char)Key.GetStr()[0] * 256
        +   (char)Key.GetStr()[Key.Len()-1]
        +   1
        )
        % m_lSize;
*/
}

sal_uIntPtr HashTable::Probe(sal_uIntPtr lPos) const
// gibt den Folgewert von lPos zurück
{
    lPos++; if (lPos==m_lSize) lPos=0;
    return lPos;
}

sal_Bool HashTable::IsFull() const
{
    return m_lElem>=m_lSize;
}

sal_Bool HashTable::Insert(ByteString const& Key, void* pObject)
// pre:  Key ist nicht im Dictionary enthalten, sonst return FALSE
//       Dictionary ist nicht voll, sonst return FALSE
// post: pObject ist unter Key im Dictionary; m_nElem wurde erhöht
{
    SmartGrow();

    if (IsFull())
    {
        DBG_ERROR("HashTable::Insert() is full");
        return sal_False;
    }

    if (FindPos(Key) != NULL )
        return sal_False;

    sal_uIntPtr     lPos  = Hash(Key);
    HashItem *pItem = &m_pData[lPos];

    // first hashing
    //
    if (pItem->IsEmpty())
    {
        pItem->SetObject(Key, pObject);
        m_lElem++;

        #ifdef DBG_UTIL
        m_aStatistic.m_lSingleHash++;
        #endif

        return sal_True;
    }

    // double hashing
    //
    lPos  = DHash(Key,lPos);
    pItem = &m_pData[lPos];

    if (pItem->IsEmpty())
    {
        pItem->SetObject(Key, pObject);
        m_lElem++;

        #ifdef DBG_UTIL
        m_aStatistic.m_lDoubleHash++;
        #endif

        return sal_True;
    }

    // linear probing
    //
    do
    {
        #ifdef DBG_UTIL
        m_aStatistic.m_lProbe++;
        #endif

        lPos  = Probe(lPos);
        pItem = &m_pData[lPos];
    }
    while(!pItem->IsEmpty());

    pItem->SetObject(Key, pObject);
    m_lElem++;
    return sal_True;
}

HashItem* HashTable::FindPos(ByteString const& Key) const
// sucht den Key; gibt Refrenz auf den Eintrag (gefunden)
// oder NULL (nicht gefunden) zurück
//
// pre:  -
// post: -
{
    // first hashing
    //
    sal_uIntPtr     lPos  = Hash(Key);
    HashItem *pItem = &m_pData[lPos];

    if (pItem->IsUsed()
    &&  pItem->GetKey() == Key)
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
        &&  pItem->GetKey() == Key)
        {
            return pItem;
        }

        // linear probing
        //
        if (pItem->IsDeleted() || pItem->IsUsed())
        {
            sal_uIntPtr n      = 0;
            sal_Bool  bFound = sal_False;
            sal_Bool  bEnd   = sal_False;

            do
            {
                n++;
                lPos   = Probe(lPos);
                pItem  = &m_pData[lPos];

                bFound =  pItem->IsUsed()
                       && pItem->GetKey() == Key;

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

void* HashTable::Find(ByteString const& Key) const
// Gibt Verweis des Objektes zurück, das unter Key abgespeichert ist,
// oder NULL wenn nicht vorhanden.
//
// pre:  -
// post: -
{
    HashItem *pItem = FindPos(Key);

    if (pItem != NULL
    &&  pItem->GetKey() == Key)
        return pItem->GetObject();
    else
        return NULL;
}

void* HashTable::Delete(ByteString const& Key)
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
    &&  pItem->GetKey() == Key)
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

    sal_uIntPtr     lOldSize = m_lSize;              // alte Daten sichern
    HashItem* pOldData = m_pData;

    m_lSize = sal_uIntPtr (m_dGrowFactor * m_lSize); // neue Größe
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
    for (sal_uIntPtr i=0; i<lOldSize; i++)
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
    return FindValidObject(sal_True /* forward */);
}

void* HashTableIterator::GetLast()
{
    m_lAt = m_aTable.GetSize() -1;
    return FindValidObject(sal_False /* backward */);
}

void* HashTableIterator::GetNext()
{
    if (m_lAt+1 >= m_aTable.GetSize())
        return NULL;

    m_lAt++;
    return FindValidObject(sal_True /* forward */);
}

void* HashTableIterator::GetPrev()
{
    if (m_lAt <= 0)
        return NULL;

    m_lAt--;
    return FindValidObject(sal_False /* backward */);
}

void* HashTableIterator::FindValidObject(sal_Bool bForward)
// Sucht nach einem vorhandenen Objekt ab der aktuellen
// Position.
//
// pre:  ab inkl. m_lAt soll die Suche beginnen
// post: if not found then
//          if bForward == sal_True then
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

#ifdef DBG_UTIL

    if (pObject == NULL)
    {
        DBG_ASSERT(bForward ? m_lAt == m_aTable.GetSize() -1 : m_lAt == 0,
            "HashTableIterator::FindValidObject()");
    }

#endif

    return pObject;
}
