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

#include <osl/diagnose.h>
#include "stgavl.hxx"
#include <assert.h>

StgAvlNode::StgAvlNode()
{
    pLeft = pRight = NULL;
    nBalance = nId = 0;
}

StgAvlNode::~StgAvlNode()
{
    delete pLeft;
    delete pRight;
}

StgAvlNode* StgAvlNode::Find( StgAvlNode* pFind )
{
    if ( pFind )
    {
        StgAvlNode* p = this;
        while( p )
        {
            short nRes = p->Compare( pFind );
            if( !nRes )
                return p;
            else p = ( nRes < 0 ) ? p->pLeft : p->pRight;
        }
    }
    return NULL;
}

// find point to add node to AVL tree and returns
// +/0/- for >/=/< previous

short StgAvlNode::Locate
    ( StgAvlNode* pFind,
      StgAvlNode** pPivot, StgAvlNode **pParent, StgAvlNode** pPrev )
{
    short nRes = 0;
    StgAvlNode* pCur = this;

    OSL_ENSURE( pPivot && pParent && pPrev, "The pointers may not be NULL!" );
    *pParent = *pPrev = NULL;
    *pPivot = this;

    // search tree for insertion point
    if ( pFind )
    {
        while( pCur != NULL )
        {
            // check for pPivot
            if( pCur->nBalance != 0 )
                *pPivot = pCur, *pParent = *pPrev;
            // save pPrev location and see what direction to go
            *pPrev = pCur;
            nRes = pCur->Compare( pFind );
            if( nRes == 0 )
                break;
            else pCur = ( nRes < 0 ) ? pCur->pLeft : pCur->pRight;
        }
    }

    return nRes;
}

// adjust balance factors in AVL tree from pivot down.
// Returns delta balance.

short StgAvlNode::Adjust( StgAvlNode** pHeavy, StgAvlNode* pNew )
{
    StgAvlNode* pCur = this;
    short nDelta;
    // no traversing
    OSL_ENSURE( pHeavy && pNew, "The pointers is not allowed to be NULL!" );
    if( pCur == pNew || !pNew )
        return nBalance;

    short nRes = Compare( pNew );
    if( nRes > 0 )
    {
        *pHeavy = pCur = pRight;
        nDelta = -1;
    }
    else
    {
        *pHeavy = pCur = pLeft;
        nDelta = 1;
    }
    nBalance = 0;
    while( pCur != pNew )
    {
        nRes = pCur->Compare( pNew );
        if( nRes > 0 )
        {
            // height of right increases by 1
            pCur->nBalance = -1;
            pCur = pCur->pRight;
        }
        else
        {
            // height of left increases by 1
            pCur->nBalance = 1;
            pCur = pCur->pLeft;
        }
    }
    nBalance = nBalance + nDelta;
    return nDelta;
}

// perform LL rotation and return new root

StgAvlNode* StgAvlNode::RotLL()
{
    assert(pLeft && "The pointer is not allowed to be NULL!");
    StgAvlNode *pHeavy = pLeft;
    pLeft = pHeavy->pRight;
    pHeavy->pRight = this;
    pHeavy->nBalance = nBalance = 0;
    return pHeavy;
}

// perform LR rotation and return new root

StgAvlNode* StgAvlNode::RotLR()
{
    assert(pLeft && pLeft->pRight && "The pointer is not allowed to be NULL!");
    StgAvlNode* pHeavy = pLeft;
    StgAvlNode* pNewRoot = pHeavy->pRight;

    pHeavy->pRight = pNewRoot->pLeft;
    pLeft = pNewRoot->pRight;
    pNewRoot->pLeft = pHeavy;
    pNewRoot->pRight = this;

    switch( pNewRoot->nBalance )
    {
        case 1:     // LR( b )
            nBalance = -1;
            pHeavy->nBalance = 0;
            break;
        case -1:    // LR( c )
            pHeavy->nBalance = 1;
            nBalance = 0;
            break;
        case 0:     // LR( a )
            nBalance = 0;
            pHeavy->nBalance = 0;
            break;
    }
    pNewRoot->nBalance = 0;
    return pNewRoot;
}

// perform RR rotation and return new root
StgAvlNode* StgAvlNode::RotRR()
{
    assert(pRight && "The pointer is not allowed to be NULL!" );
    StgAvlNode* pHeavy = pRight;
    pRight = pHeavy->pLeft;
    pHeavy->pLeft = this;
    nBalance = pHeavy->nBalance = 0;
    return pHeavy;
}

// perform the RL rotation and return the new root
StgAvlNode* StgAvlNode::RotRL()
{
    assert(pRight && pRight->pLeft && "The pointer is not allowed to be NULL!");
    StgAvlNode* pHeavy = pRight;
    StgAvlNode* pNewRoot = pHeavy->pLeft;
    pHeavy->pLeft = pNewRoot->pRight;
    pRight = pNewRoot->pLeft;
    pNewRoot->pRight = pHeavy;
    pNewRoot->pLeft = this;
    switch( pNewRoot->nBalance )
    {
        case -1:    // RL( b )
            nBalance = 1;
            pHeavy->nBalance = 0;
            break;
        case 1:     // RL( c )
            pHeavy->nBalance = -1;
            nBalance = 0;
            break;
        case 0:     // RL( a )
            nBalance = 0;
            pHeavy->nBalance = 0;
            break;
    }
    pNewRoot->nBalance = 0;
    return pNewRoot;
}

// Remove a tree element. Return the removed element or NULL.

StgAvlNode* StgAvlNode::Rem( StgAvlNode** p, StgAvlNode* pDel, bool bPtrs )
{
    if( p && *p && pDel )
    {
        StgAvlNode* pCur = *p;
        short nRes = bPtrs ? short( pCur == pDel ) : short(pCur->Compare( pDel ));
        if( !nRes )
        {
            // Element found: remove
            if( !pCur->pRight )
            {
                *p = pCur->pLeft; pCur->pLeft = NULL;
            }
            else if( !pCur->pLeft )
            {
                *p = pCur->pRight; pCur->pRight = NULL;
            }
            else
            {
                // The damn element has two leaves. Get the
                // rightmost element of the left subtree (which
                // is lexically before this element) and replace
                // this element with the element found.
                StgAvlNode* last = pCur;
                StgAvlNode* l;
                for( l = pCur->pLeft;
                     l->pRight; last = l, l = l->pRight ) {}
                // remove the element from chain
                if( l == last->pRight )
                    last->pRight = l->pLeft;
                else
                    last->pLeft = l->pLeft;
                // perform the replacement
                l->pLeft = pCur->pLeft;
                l->pRight = pCur->pRight;
                *p = l;
                // delete the element
                pCur->pLeft = pCur->pRight = NULL;
            }
            return pCur;
        }
        else
        {
            if( nRes < 0 )
                return Rem( &pCur->pLeft, pDel, bPtrs );
            else
                return Rem( &pCur->pRight, pDel, bPtrs );
        }
    }
    return NULL;
}

// Enumerate the tree for later iteration

void StgAvlNode::StgEnum( short& n )
{
    if( pLeft )
        pLeft->StgEnum( n );
    nId = n++;
    if( pRight )
        pRight->StgEnum( n );
}

// Add node to AVL tree.
// Return false if the element already exists.

bool StgAvlNode::Insert( StgAvlNode** pRoot, StgAvlNode* pIns )
{
    StgAvlNode* pPivot, *pHeavy, *pNewRoot, *pParent, *pPrev;
    if ( !pRoot )
        return false;

    // special case - empty tree
    if( *pRoot == NULL )
    {
        *pRoot = pIns;
        return true;
    }
    // find insertion point and return if already present
    short nRes = (*pRoot)->Locate( pIns, &pPivot, &pParent, &pPrev );
    if( !nRes )
        return false;

    assert(pPivot && pPrev && "The pointers may not be NULL!");

    // add new node
    if( nRes < 0 )
        pPrev->pLeft = pIns;
    else
        pPrev->pRight = pIns;
    // rebalance tree
    short nDelta = pPivot->Adjust( &pHeavy, pIns );
    if( pPivot->nBalance >= 2 || pPivot->nBalance <= -2 )
    {
        pHeavy = ( nDelta < 0 ) ? pPivot->pRight : pPivot->pLeft;
        // left imbalance
        if( nDelta > 0 )
            if( pHeavy->nBalance == 1 )
                pNewRoot = pPivot->RotLL();
            else
                pNewRoot = pPivot->RotLR();
        // right imbalance
        else if( pHeavy->nBalance == -1 )
            pNewRoot = pPivot->RotRR();
        else
            pNewRoot = pPivot->RotRL();
        // relink balanced subtree
        if( pParent == NULL )
            *pRoot = pNewRoot;
        else if( pPivot == pParent->pLeft )
            pParent->pLeft = pNewRoot;
        else if( pPivot == pParent->pRight )
            pParent->pRight = pNewRoot;
    }
    return true;
}

// Remove node from tree. Returns true is found and removed.
// Actually delete if bDel

bool StgAvlNode::Remove( StgAvlNode** pRoot, StgAvlNode* pDel, bool bDel )
{
    if ( !pRoot )
        return false;

    // special case - empty tree
    if( *pRoot == NULL )
        return false;
    // delete the element
    pDel = Rem( pRoot, pDel, false );
    if( pDel )
    {
        if( bDel )
            delete pDel;
        // Rebalance the tree the hard way
        // OS 22.09.95: Auf MD's Wunsch auskommentiert wg. Absturz
/*      StgAvlNode* pNew = NULL;
        while( *pRoot )
        {
            StgAvlNode* p = Rem( pRoot, *pRoot, false );
            Insert( &pNew, p );
        }
        *pRoot = pNew;*/
        return true;
    }
    else
        return false;
}

// Move node to a different tree. Returns true is found and moved. This routine
// may be called when the key has changed.



////////////////////////// class AvlIterator

// The iterator walks through a tree one entry by one.

StgAvlIterator::StgAvlIterator( StgAvlNode* p )
{
    m_pRoot = p;
    m_nCount = 0;
    m_nCur = 0;
    if( p )
        p->StgEnum( m_nCount );
}

StgAvlNode* StgAvlIterator::Find( short n )
{
    StgAvlNode* p = m_pRoot;
    while( p )
    {
        if( n == p->nId )
            break;
        else p = ( n < p->nId ) ? p->pLeft : p->pRight;
    }
    return p;
}

StgAvlNode* StgAvlIterator::First()
{
    m_nCur = -1;
    return Next();
}

StgAvlNode* StgAvlIterator::Next()
{
    return Find( ++m_nCur );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
