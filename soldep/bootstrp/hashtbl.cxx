
/*************************************************************************
#*
#*    $Workfile:$
#*
#*    class Klassenname(n)
#*
#*    Beschreibung      DOKUNAME.DOC oder
#*                      Beschreibung des Moduls
#*
#*    Ersterstellung    XX  TT.MM.JJ
#*    Letzte Aenderung  $Author: vg $ $Date: 2007-10-22 14:41:55 $
#*    $Revision: 1.3 $
#*
#*    $Logfile:$
#*
#*    Copyright (c) 1989 - 2000, STAR DIVISION
#*
#*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/soldep/bootstrp/hashtbl.cxx,v 1.3 2007-10-22 14:41:55 vg Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2.6.1  2007/10/18 07:45:52  obo
      #150903# unxmacxi support

      Revision 1.2  2007/06/27 23:37:39  hr
      INTEGRATION: CWS vgbugs07 (1.1.22); FILE MERGED
      2007/06/04 13:35:21 vg 1.1.22.1: #i76605# Remove -I .../inc/module hack introduced by hedaburemove01

      Revision 1.1.22.1  2007/06/04 13:35:21  vg
      #i76605# Remove -I .../inc/module hack introduced by hedaburemove01

      Revision 1.1  2006/04/26 12:25:22  obo
      initial revision

      Revision 1.3  2001/09/14 13:14:56  nf
      Handling of invisible objects

      Revision 1.2  2000/11/02 10:31:12  hjs
      unicode first shot

      Revision 1.1  1999/06/11 17:02:54  hjs
      aus tools\workben


*************************************************************************/

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

HashTable::HashTable(ULONG lSize, BOOL bOwner, double dMaxLoadFactor, double dGrowFactor)
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

ULONG HashTable::Hash(ByteString const& Key) const
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
        h = (h<<4) + (ULONG)(USHORT)Key.GetBuffer()[i];
        g = h & 0xf0000000;

        if (g != 0)
        {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }

    return h % m_lSize;
}

ULONG HashTable::DHash(ByteString const& Key, ULONG lOldHash) const
{
    ULONG lHash = lOldHash;
    ULONG i,n;

    for (i=0,n=Key.Len(); i<n; i++)
    {
        lHash *= 256L;
        lHash += (ULONG)(USHORT)Key.GetBuffer()[i];
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

BOOL HashTable::Insert(ByteString const& Key, void* pObject)
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

HashItem* HashTable::FindPos(ByteString const& Key) const
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
