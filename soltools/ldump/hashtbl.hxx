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

    virtual ~HashTable();

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

    void operator =(HashTableIterator &); // not defined

    void* FindValidObject(bool bForward);

protected:
    void* GetFirst(); // Interation _ohne_ Sortierung
    void* GetNext();
    void* GetLast();
    void* GetPrev();

public:
    HashTableIterator(HashTable const&);
};

#endif // _HASHTBL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
