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

#ifndef INCLUDED_SOT_SOURCE_SDSTOR_STGAVL_HXX
#define INCLUDED_SOT_SOURCE_SDSTOR_STGAVL_HXX

// This is an abstract base class for nodes.
// Especially, the compare function must be implemented.

class StgAvlNode
{
    friend class StgAvlIterator;
private:
    short Locate( StgAvlNode*, StgAvlNode**, StgAvlNode**, StgAvlNode** );
    short Adjust( StgAvlNode**, StgAvlNode* );
    StgAvlNode* RotLL();
    StgAvlNode* RotLR();
    StgAvlNode* RotRR();
    StgAvlNode* RotRL();
    void   StgEnum( short& );
    static StgAvlNode* Rem( StgAvlNode**, StgAvlNode*, bool );
protected:
    short m_nId;                          // iterator ID
    short m_nBalance;                     // indicates tree balance
    StgAvlNode* m_pLeft, *m_pRight;         // leaves
    StgAvlNode();
public:
    virtual ~StgAvlNode();
    StgAvlNode* Find( StgAvlNode* );
    static bool Insert( StgAvlNode**, StgAvlNode* );
    static bool Remove( StgAvlNode**, StgAvlNode*, bool bDel = true );
    virtual short Compare( const StgAvlNode* ) const = 0;
};

// The iterator class provides single stepping through an AVL tree.

class StgAvlIterator {
    StgAvlNode* m_pRoot;                  // root entry (parent)
    short       m_nCount;                 // tree size
    short       m_nCur;                   // current element
    StgAvlNode* Find( short );
public:
    explicit StgAvlIterator( StgAvlNode* );
    StgAvlNode* First();
    StgAvlNode* Next();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
