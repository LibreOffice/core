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



#ifndef _HASHTBL_HXX
#define _HASHTBL_HXX

#include <tlgen.hxx>

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

    ULONG Hash(String const& Key) const;
    ULONG DHash(String const& Key, ULONG lHash) const;
    ULONG Probe(ULONG lPos) const;

    HashItem* FindPos(String const& Key) const;
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

    ~HashTable();

    BOOL  IsFull() const;
    ULONG GetSize() const { return m_lSize; }

    void* Find   (String const& Key) const;
    BOOL  Insert (String const& Key, void* pObject);
    void* Delete (String const& Key);
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
        { return (ObjType) HashTable::Find(String(Key)); }              \
                                                                        \
        BOOL Insert (KeyType const& Key, ObjType Object)                \
        { return HashTable::Insert(String(Key), (void*) Object); }      \
                                                                        \
        ObjType  Delete (KeyType const&Key)                             \
        { return (ObjType) HashTable::Delete (String(Key)); }           \
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

