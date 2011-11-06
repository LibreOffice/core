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



#ifndef _STGAVL_HXX
#define _STGAVL_HXX

#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif

// This class must be overloaded to define real, living nodes.
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
    static StgAvlNode* Rem( StgAvlNode**, StgAvlNode*, sal_Bool );
protected:
    short nId;                          // iterator ID
    short nBalance;                     // indicates tree balance
    StgAvlNode* pLeft, *pRight;         // leaves
    StgAvlNode();
public:
    virtual ~StgAvlNode();
    StgAvlNode* Find( StgAvlNode* );
    static sal_Bool Insert( StgAvlNode**, StgAvlNode* );
    static sal_Bool Remove( StgAvlNode**, StgAvlNode*, sal_Bool bDel = sal_True );
    static sal_Bool Move( StgAvlNode**, StgAvlNode**, StgAvlNode* );
    virtual short Compare( const StgAvlNode* ) const = 0;
};

// The iterator class provides single stepping through an AVL tree.

class StgAvlIterator {
    StgAvlNode* pRoot;                  // root entry (parent)
    short       nCount;                 // tree size
    short       nCur;                   // current element
    StgAvlNode* Find( short );
public:
    StgAvlIterator( StgAvlNode* );
    StgAvlNode* First();
    StgAvlNode* Last();
    StgAvlNode* Next();
    StgAvlNode* Prev();
};

#endif
