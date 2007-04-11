/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hashtbl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:38:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _HASHTBL_HXX
#define _HASHTBL_HXX

#include <tools/gen.hxx>
#include <tools/string.hxx>

// ADT hash table
//
// Invariante:
//    1. m_lElem < m_lSize
//    2. die Elemente in m_Array wurden double-hashed erzeugt
//
class HashItem;

class HashTable
{
    ULONG     m_lSize;
    ULONG     m_lElem;
    HashItem *m_pData;
    double    m_dMaxLoadFactor;
    double    m_dGrowFactor;
    BOOL      m_bOwner;

    ULONG Hash(ByteString const& Key) const;
    ULONG DHash(ByteString const& Key, ULONG lHash) const;
    ULONG Probe(ULONG lPos) const;

    HashItem* FindPos(ByteString const& Key) const;
    void      SmartGrow();
    double    CalcLoadFactor() const;

// Statistik
#ifdef DBG_UTIL
private:
    struct
    {
        ULONG m_lSingleHash;
        ULONG m_lDoubleHash;
        ULONG m_lProbe;
    }
        m_aStatistic;
#endif

protected:
    friend class HashTableIterator;

    virtual void OnDeleteObject(void* pObject);

    void* GetObjectAt(ULONG lPos) const;

// Default-Werte
public:
    static double m_defMaxLoadFactor;
    static double m_defDefGrowFactor;

public:
    HashTable
    (
        ULONG   lSize,
        BOOL    bOwner,
        double  dMaxLoadFactor = HashTable::m_defMaxLoadFactor /* 0.8 */,
        double  dGrowFactor = HashTable::m_defDefGrowFactor /* 2.0 */
    );

    virtual ~HashTable();

    BOOL  IsFull() const;
    ULONG GetSize() const { return m_lSize; }

    void* Find   (ByteString const& Key) const;
    BOOL  Insert (ByteString const& Key, void* pObject);
    void* Delete (ByteString const& Key);
};

// ADT hash table iterator
//
// Invariante: 0 <= m_lAt < m_aTable.GetCount()
//
class HashTableIterator
{
    ULONG            m_lAt;
    HashTable const& m_aTable;

    void* FindValidObject(BOOL bForward);

protected:
    void* GetFirst(); // Interation _ohne_ Sortierung
    void* GetNext();
    void* GetLast();
    void* GetPrev();

public:
    HashTableIterator(HashTable const&);
};

// typsichere Makros ---------------------------------------------------

#define DECLARE_HASHTABLE_INTERN(ClassName,Owner,KeyType,ObjType)       \
    class ClassName : public HashTable                                  \
    {                                                                   \
    public:                                                             \
        ClassName                                                       \
        (                                                               \
            ULONG   lSize,                                              \
            double  dMaxLoadFactor = HashTable::m_defMaxLoadFactor,     \
            double  dGrowFactor = HashTable::m_defDefGrowFactor         \
        )                                                               \
        : HashTable(lSize,Owner,dMaxLoadFactor,dGrowFactor) {}          \
                                                                        \
        ObjType  Find (KeyType const& Key) const                        \
        { return (ObjType) HashTable::Find(ByteString(Key)); }              \
                                                                        \
        BOOL Insert (KeyType const& Key, ObjType Object)                \
        { return HashTable::Insert(ByteString(Key), (void*) Object); }      \
                                                                        \
        ObjType  Delete (KeyType const&Key)                             \
        { return (ObjType) HashTable::Delete (ByteString(Key)); }           \
    };

// HashTable OHNE Owner-Verhalten
#define DECLARE_HASHTABLE(ClassName,KeyType,ObjType)                 \
    DECLARE_HASHTABLE_INTERN(ClassName,FALSE,KeyType,ObjType)

// HashTable MIT Owner-Verhalten
#define DECLARE_HASHTABLE_OWNER(ClassName,KeyType,ObjType)           \
    DECLARE_HASHTABLE_INTERN(ClassName##2,TRUE,KeyType,ObjType)      \
    class ClassName : public ClassName##2                            \
    {                                                                \
    protected:                                                       \
        virtual void OnDeleteObject(void* pObject);                  \
    public:                                                          \
        ClassName                                                    \
        (                                                            \
            ULONG   lSize,                                           \
            double  dMaxLoadFactor = HashTable::m_defMaxLoadFactor,  \
            double  dGrowFactor = HashTable::m_defDefGrowFactor      \
        )                                                            \
        : ClassName##2(lSize,dMaxLoadFactor,dGrowFactor) {}          \
        ~ClassName();                                                \
    };

#define IMPLEMENT_HASHTABLE_OWNER(ClassName,KeyType,ObjType)         \
    void ClassName::OnDeleteObject(void* pObject)                    \
    { delete (ObjType) pObject; }                                    \
                                                                     \
    ClassName::~ClassName()                                          \
    {                                                                \
        for (ULONG i=0; i<GetSize(); i++)                            \
        {                                                            \
            void *pObject = GetObjectAt(i);                          \
            if (pObject != NULL)                                     \
                OnDeleteObject(pObject);                             \
        }                                                            \
    }

// Iterator-Makros --------------------------------------------------

#define DECLARE_HASHTABLE_ITERATOR(ClassName,ObjType)               \
    class ClassName : public HashTableIterator                      \
    {                                                               \
    public:                                                         \
        ClassName(HashTable const& aTable)                          \
        : HashTableIterator(aTable) {}                              \
                                                                    \
        ObjType GetFirst()                                          \
            { return (ObjType)HashTableIterator::GetFirst(); }      \
        ObjType GetNext()                                           \
            { return (ObjType)HashTableIterator::GetNext();  }      \
        ObjType GetLast()                                           \
            { return (ObjType)HashTableIterator::GetLast();  }      \
        ObjType GetPrev()                                           \
            { return (ObjType)HashTableIterator::GetPrev();  }      \
    };


#endif // _HASHTBL_HXX
