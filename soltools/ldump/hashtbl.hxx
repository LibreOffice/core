/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hashtbl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:28:12 $
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

// ADT hash table
//
// Invariante:
//    1. m_lElem < m_lSize
//    2. die Elemente in m_Array wurden double-hashed erzeugt
//
class HashItem;

class HashTable
{
    unsigned long m_lSize;
    unsigned long m_lElem;
    HashItem *m_pData;
    double    m_dMaxLoadFactor;
    double    m_dGrowFactor;
    bool      m_bOwner;

    unsigned long Hash(const char *cKey) const;
    unsigned long DHash(const char *cKey , unsigned long lHash) const;
    unsigned long Probe(unsigned long lPos) const;

    HashItem* FindPos(const char *cKey) const;
    void      SmartGrow();
    double    CalcLoadFactor() const;

protected:
    friend class HashTableIterator;

    virtual void OnDeleteObject(void* pObject);

    void* GetObjectAt(unsigned long lPos) const;

// Default-Werte
public:
    static double m_defMaxLoadFactor;
    static double m_defDefGrowFactor;

public:
    HashTable
    (
        unsigned long   lSize,
        bool    bOwner,
        double  dMaxLoadFactor = HashTable::m_defMaxLoadFactor /* 0.8 */,
        double  dGrowFactor = HashTable::m_defDefGrowFactor /* 2.0 */
    );

    ~HashTable();

    bool  IsFull() const;
    unsigned long GetSize() const { return m_lSize; }

    void* Find   (const char *cKey ) const;
    bool  Insert (const char *cKey , void* pObject);
    void* Delete (const char *cKey);
};

// ADT hash table iterator
//
// Invariante: 0 <= m_lAt < m_aTable.GetCount()
//
class HashTableIterator
{
    unsigned long    m_lAt;
    HashTable const& m_aTable;

    void* FindValidObject(bool bForward);

protected:
    void* GetFirst(); // Interation _ohne_ Sortierung
    void* GetNext();
    void* GetLast();
    void* GetPrev();

public:
    HashTableIterator(HashTable const&);
};

// typsichere Makros ---------------------------------------------------

#define DECLARE_HASHTABLE_INTERN(ClassName,Owner,KeyType,ObjType)    \
    class ClassName : public HashTable                               \
    {                                                                \
    public:                                                          \
        ClassName                                                    \
        (                                                            \
            unsigned long lSize,                                     \
            double  dMaxLoadFactor = HashTable::m_defMaxLoadFactor,  \
            double  dGrowFactor = HashTable::m_defDefGrowFactor      \
        )                                                            \
        : HashTable(lSize,Owner,dMaxLoadFactor,dGrowFactor) {}       \
                                                                     \
        ObjType  Find (KeyType const& Key) const                     \
        { return (ObjType) HashTable::Find((char *) Key); }          \
                                                                     \
        bool Insert (KeyType const& Key, ObjType Object)             \
        { return HashTable::Insert((char *) Key, (void*) Object); }  \
                                                                     \
        ObjType  Delete (KeyType const&Key)                          \
        { return (ObjType) HashTable::Delete ((char *) Key); }       \
    };

// HashTable OHNE Owner-Verhalten
#define DECLARE_HASHTABLE(ClassName,KeyType,ObjType)                 \
    DECLARE_HASHTABLE_INTERN(ClassName,false,KeyType,ObjType)

// HashTable MIT Owner-Verhalten
#define DECLARE_HASHTABLE_OWNER(ClassName,KeyType,ObjType)           \
    DECLARE_HASHTABLE_INTERN(ClassName##2,true,KeyType,ObjType)      \
    class ClassName : public ClassName##2                            \
    {                                                                \
    protected:                                                       \
        virtual void OnDeleteObject(void* pObject);                  \
    public:                                                          \
        ClassName                                                    \
        (                                                            \
            unsigned long   lSize,                                   \
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
        for (unsigned long i=0; i<GetSize(); i++)                    \
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

