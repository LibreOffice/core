/*************************************************************************
 *
 *  $RCSfile: hashtbl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:11 $
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

#include <tlgen.hxx>
#include "hashtbl.hxx"

// -------------------------------------------------------------
// class HashItem
//
class HashItem
{
    enum ETag { TAG_EMPTY, TAG_USED, TAG_DELETED };

    void*   m_pObject;
    ETag    m_Tag;
    String  m_Key;

public:
    HashItem() { m_Tag = TAG_EMPTY; m_pObject = NULL; }

    BOOL IsDeleted() const
    {   return m_Tag == TAG_DELETED; }

    BOOL IsEmpty() const
    {   return m_Tag == TAG_DELETED || m_Tag == TAG_EMPTY; }

    BOOL IsFree() const
    {   return m_Tag == TAG_EMPTY; }

    BOOL IsUsed() const
    {   return m_Tag == TAG_USED; }

    void Delete()
    { m_Tag = TAG_DELETED; m_Key = ""; m_pObject = NULL; }

    String const& GetKey() const
    { return m_Key; }

    void* GetObject() const
    { return m_pObject; }

    void SetObject(String const Key, void *pObject)
    { m_Tag = TAG_USED; m_Key = Key; m_pObject = pObject; }
};

// #define MIN(a,b) (a)<(b)?(a):(b)
// #define MAX(a,b) (a)>(b)?(a):(b)

// -------------------------------------------------------------
// class HashTable
//

/*static*/ double HashTable::m_defMaxLoadFactor = 0.8;
/*static*/ double HashTable::m_defDefGrowFactor = 2.0;

HashTable::HashTable(ULONG lSize, BOOL bOwner, double dMaxLoadFactor, double dGrowFactor)
{
    m_lSize          = lSize;
    m_bOwner         = bOwner;
    m_lElem          = 0;
    m_dMaxLoadFactor = max(0.5,min(1.0,dMaxLoadFactor));  // 0.5 ... 1.0
    m_dGrowFactor    = max(1.3,(5.0,dGrowFactor));     // 1.3 ... 5.0
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
        for (ULONG i=0; i<GetSize(); i++)
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

void* HashTable::GetObjectAt(ULONG lPos) const
// Gibt Objekt zurück, wenn es eines gibt, sonst NULL;
{
    DBG_ASSERT(lPos<m_lSize, "HashTable::GetObjectAt()");

    HashItem *pItem = &m_pData[lPos];

    return pItem->IsUsed() ? pItem->GetObject() : NULL;
}

void HashTable::OnDeleteObject(void*)
{
    DBG_ERROR("HashTable::OnDeleteObject(void*) nicht überladen")
}

ULONG HashTable::Hash(String const& Key) const
{
    /*
    ULONG lHash = 0;
    ULONG i,n;

    for (i=0,n=Key.Len(); i<n; i++)
    {
        lHash *= 256L;
        lHash += (ULONG)(USHORT)Key.GetStr()[i];
        lHash %= m_lSize;
    }
    return lHash;
    */

    // Hashfunktion von P.J. Weinberger
    // aus dem "Drachenbuch" von Aho/Sethi/Ullman
    ULONG i,n;
    ULONG h = 0;
    ULONG g = 0;

    for (i=0,n=Key.Len(); i<n; i++)
    {
        h = (h<<4) + (ULONG)(USHORT)Key.GetStr()[i];
        g = h & 0xf0000000;

        if (g != 0)
        {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }

    return h % m_lSize;
}

ULONG HashTable::DHash(String const& Key, ULONG lOldHash) const
{
    ULONG lHash = lOldHash;
    ULONG i,n;

    for (i=0,n=Key.Len(); i<n; i++)
    {
        lHash *= 256L;
        lHash += (ULONG)(USHORT)Key.GetStr()[i];
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

ULONG HashTable::Probe(ULONG lPos) const
// gibt den Folgewert von lPos zurück
{
    lPos++; if (lPos==m_lSize) lPos=0;
    return lPos;
}

BOOL HashTable::IsFull() const
{
    return m_lElem>=m_lSize;
}

BOOL HashTable::Insert(String const& Key, void* pObject)
// pre:  Key ist nicht im Dictionary enthalten, sonst return FALSE
//       Dictionary ist nicht voll, sonst return FALSE
// post: pObject ist unter Key im Dictionary; m_nElem wurde erhöht
{
    SmartGrow();

    if (IsFull())
    {
        DBG_ERROR("HashTable::Insert() is full");
        return FALSE;
    }

    if (FindPos(Key) != NULL )
        return FALSE;

    ULONG     lPos  = Hash(Key);
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

        return TRUE;
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

        return TRUE;
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
    return TRUE;
}

HashItem* HashTable::FindPos(String const& Key) const
// sucht den Key; gibt Refrenz auf den Eintrag (gefunden)
// oder NULL (nicht gefunden) zurück
//
// pre:  -
// post: -
{
    // first hashing
    //
    ULONG     lPos  = Hash(Key);
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
            ULONG n      = 0;
            BOOL  bFound = FALSE;
            BOOL  bEnd   = FALSE;

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

void* HashTable::Find(String const& Key) const
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

void* HashTable::Delete(String const& Key)
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

    ULONG     lOldSize = m_lSize;              // alte Daten sichern
    HashItem* pOldData = m_pData;

    m_lSize = ULONG (m_dGrowFactor * m_lSize); // neue Größe
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
    for (ULONG i=0; i<lOldSize; i++)
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
    return FindValidObject(TRUE /* forward */);
}

void* HashTableIterator::GetLast()
{
    m_lAt = m_aTable.GetSize() -1;
    return FindValidObject(FALSE /* backward */);
}

void* HashTableIterator::GetNext()
{
    if (m_lAt+1 >= m_aTable.GetSize())
        return NULL;

    m_lAt++;
    return FindValidObject(TRUE /* forward */);
}

void* HashTableIterator::GetPrev()
{
    if (m_lAt <= 0)
        return NULL;

    m_lAt--;
    return FindValidObject(FALSE /* backward */);
}

void* HashTableIterator::FindValidObject(BOOL bForward)
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

#ifdef DBG_UTIL

    if (pObject == NULL)
    {
        DBG_ASSERT(bForward ? m_lAt == m_aTable.GetSize() -1 : m_lAt == 0,
            "HashTableIterator::FindValidObject()");
    }

#endif

    return pObject;
}
