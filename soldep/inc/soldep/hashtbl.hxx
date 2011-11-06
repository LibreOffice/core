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
    sal_uIntPtr     m_lSize;
    sal_uIntPtr     m_lElem;
    HashItem *m_pData;
    double    m_dMaxLoadFactor;
    double    m_dGrowFactor;
    sal_Bool      m_bOwner;

    sal_uIntPtr Hash(ByteString const& Key) const;
    sal_uIntPtr DHash(ByteString const& Key, sal_uIntPtr lHash) const;
    sal_uIntPtr Probe(sal_uIntPtr lPos) const;

    HashItem* FindPos(ByteString const& Key) const;
    void      SmartGrow();
    double    CalcLoadFactor() const;

// Statistik
#ifdef DBG_UTIL
private:
    struct
    {
        sal_uIntPtr m_lSingleHash;
        sal_uIntPtr m_lDoubleHash;
        sal_uIntPtr m_lProbe;
    }
        m_aStatistic;
#endif

protected:
    friend class HashTableIterator;

    virtual void OnDeleteObject(void* pObject);

    void* GetObjectAt(sal_uIntPtr lPos) const;

// Default-Werte
public:
    static double m_defMaxLoadFactor;
    static double m_defDefGrowFactor;

public:
    HashTable
    (
        sal_uIntPtr lSize,
        sal_Bool    bOwner,
        double  dMaxLoadFactor = HashTable::m_defMaxLoadFactor /* 0.8 */,
        double  dGrowFactor = HashTable::m_defDefGrowFactor /* 2.0 */
    );

    virtual ~HashTable();

    sal_Bool  IsFull() const;
    sal_uIntPtr GetSize() const { return m_lSize; }

    void* Find   (ByteString const& Key) const;
    sal_Bool  Insert (ByteString const& Key, void* pObject);
    void* Delete (ByteString const& Key);
};

// ADT hash table iterator
//
// Invariante: 0 <= m_lAt < m_aTable.GetCount()
//
class HashTableIterator
{
    sal_uIntPtr          m_lAt;
    HashTable const& m_aTable;

    void* FindValidObject(sal_Bool bForward);

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
            sal_uIntPtr lSize,                                              \
            double  dMaxLoadFactor = HashTable::m_defMaxLoadFactor,     \
            double  dGrowFactor = HashTable::m_defDefGrowFactor         \
        )                                                               \
        : HashTable(lSize,Owner,dMaxLoadFactor,dGrowFactor) {}          \
                                                                        \
        ObjType  Find (KeyType const& Key) const                        \
        { return (ObjType) HashTable::Find(ByteString(Key)); }              \
                                                                        \
        using HashTable::Insert;                                        \
        sal_Bool Insert (KeyType const& Key, ObjType Object)                \
        { return HashTable::Insert(ByteString(Key), (void*) Object); }      \
                                                                        \
        ObjType  Delete (KeyType const&Key)                             \
        { return (ObjType) HashTable::Delete (ByteString(Key)); }           \
    };

// HashTable OHNE Owner-Verhalten
#define DECLARE_HASHTABLE(ClassName,KeyType,ObjType)                 \
    DECLARE_HASHTABLE_INTERN(ClassName,sal_False,KeyType,ObjType)

// HashTable MIT Owner-Verhalten
#define DECLARE_HASHTABLE_OWNER(ClassName,KeyType,ObjType)           \
    DECLARE_HASHTABLE_INTERN(ClassName##2,sal_True,KeyType,ObjType)      \
    class ClassName : public ClassName##2                            \
    {                                                                \
    protected:                                                       \
        virtual void OnDeleteObject(void* pObject);                  \
    public:                                                          \
        ClassName                                                    \
        (                                                            \
            sal_uIntPtr lSize,                                           \
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
        for (sal_uIntPtr i=0; i<GetSize(); i++)                            \
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
