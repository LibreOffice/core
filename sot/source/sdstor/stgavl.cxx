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
    m_pLeft = m_pRight = NULL;
    m_nBalance = m_nId = 0;
}

StgAvlNode::~StgAvlNode()
{
    delete m_pLeft;
    delete m_pRight;
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
            else p = ( nRes < 0 ) ? p->m_pLeft : p->m_pRight;
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
            if( pCur->m_nBalance != 0 )
                *pPivot = pCur, *pParent = *pPrev;
            // save pPrev location and see what direction to go
            *pPrev = pCur;
            nRes = pCur->Compare( pFind );
            if( nRes == 0 )
                break;
            else pCur = ( nRes < 0 ) ? pCur->m_pLeft : pCur->m_pRight;
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
        return m_nBalance;

    short nRes = Compare( pNew );
    if( nRes > 0 )
    {
        *pHeavy = pCur = m_pRight;
        nDelta = -1;
    }
    else
    {
        *pHeavy = pCur = m_pLeft;
        nDelta = 1;
    }
    m_nBalance = 0;
    while( pCur != pNew )
    {
        nRes = pCur->Compare( pNew );
        if( nRes > 0 )
        {
            // height of right increases by 1
            pCur->m_nBalance = -1;
            pCur = pCur->m_pRight;
        }
        else
        {
            // height of left increases by 1
            pCur->m_nBalance = 1;
            pCur = pCur->m_pLeft;
        }
    }
    m_nBalance = m_nBalance + nDelta;
    return nDelta;
}

// perform LL rotation and return new root

StgAvlNode* StgAvlNode::RotLL()
{
    assert(m_pLeft && "The pointer is not allowed to be NULL!");
    StgAvlNode *pHeavy = m_pLeft;
    m_pLeft = pHeavy->m_pRight;
    pHeavy->m_pRight = this;
    pHeavy->m_nBalance = m_nBalance = 0;
    return pHeavy;
}

// perform LR rotation and return new root

StgAvlNode* StgAvlNode::RotLR()
{
    assert(m_pLeft && m_pLeft->m_pRight && "The pointer is not allowed to be NULL!");
    StgAvlNode* pHeavy = m_pLeft;
    StgAvlNode* pNewRoot = pHeavy->m_pRight;

    pHeavy->m_pRight = pNewRoot->m_pLeft;
    m_pLeft = pNewRoot->m_pRight;
    pNewRoot->m_pLeft = pHeavy;
    pNewRoot->m_pRight = this;

    switch( pNewRoot->m_nBalance )
    {
        case 1:     // LR( b )
            m_nBalance = -1;
            pHeavy->m_nBalance = 0;
            break;
        case -1:    // LR( c )
            pHeavy->m_nBalance = 1;
            m_nBalance = 0;
            break;
        case 0:     // LR( a )
            m_nBalance = 0;
            pHeavy->m_nBalance = 0;
            break;
    }
    pNewRoot->m_nBalance = 0;
    return pNewRoot;
}

// perform RR rotation and return new root
StgAvlNode* StgAvlNode::RotRR()
{
    assert(m_pRight && "The pointer is not allowed to be NULL!" );
    StgAvlNode* pHeavy = m_pRight;
    m_pRight = pHeavy->m_pLeft;
    pHeavy->m_pLeft = this;
    m_nBalance = pHeavy->m_nBalance = 0;
    return pHeavy;
}

// perform the RL rotation and return the new root
StgAvlNode* StgAvlNode::RotRL()
{
    assert(m_pRight && m_pRight->m_pLeft && "The pointer is not allowed to be NULL!");
    StgAvlNode* pHeavy = m_pRight;
    StgAvlNode* pNewRoot = pHeavy->m_pLeft;
    pHeavy->m_pLeft = pNewRoot->m_pRight;
    m_pRight = pNewRoot->m_pLeft;
    pNewRoot->m_pRight = pHeavy;
    pNewRoot->m_pLeft = this;
    switch( pNewRoot->m_nBalance )
    {
        case -1:    // RL( b )
            m_nBalance = 1;
            pHeavy->m_nBalance = 0;
            break;
        case 1:     // RL( c )
            pHeavy->m_nBalance = -1;
            m_nBalance = 0;
            break;
        case 0:     // RL( a )
            m_nBalance = 0;
            pHeavy->m_nBalance = 0;
            break;
    }
    pNewRoot->m_nBalance = 0;
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
            if( !pCur->m_pRight )
            {
                *p = pCur->m_pLeft; pCur->m_pLeft = NULL;
            }
            else if( !pCur->m_pLeft )
            {
                *p = pCur->m_pRight; pCur->m_pRight = NULL;
            }
            else
            {
                // The damn element has two leaves. Get the
                // rightmost element of the left subtree (which
                // is lexically before this element) and replace
                // this element with the element found.
                StgAvlNode* last = pCur;
                StgAvlNode* l;
                for( l = pCur->m_pLeft;
                     l->m_pRight; last = l, l = l->m_pRight ) {}
                // remove the element from chain
                if( l == last->m_pRight )
                    last->m_pRight = l->m_pLeft;
                else
                    last->m_pLeft = l->m_pLeft;
                // perform the replacement
                l->m_pLeft = pCur->m_pLeft;
                l->m_pRight = pCur->m_pRight;
                *p = l;
                // delete the element
                pCur->m_pLeft = pCur->m_pRight = NULL;
            }
            return pCur;
        }
        else
        {
            if( nRes < 0 )
                return Rem( &pCur->m_pLeft, pDel, bPtrs );
            else
                return Rem( &pCur->m_pRight, pDel, bPtrs );
        }
    }
    return NULL;
}

// Enumerate the tree for later iteration

void StgAvlNode::StgEnum( short& n )
{
    if( m_pLeft )
        m_pLeft->StgEnum( n );
    m_nId = n++;
    if( m_pRight )
        m_pRight->StgEnum( n );
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
        pPrev->m_pLeft = pIns;
    else
        pPrev->m_pRight = pIns;
    // rebalance tree
    short nDelta = pPivot->Adjust( &pHeavy, pIns );
    if( pPivot->m_nBalance >= 2 || pPivot->m_nBalance <= -2 )
    {
        pHeavy = ( nDelta < 0 ) ? pPivot->m_pRight : pPivot->m_pLeft;
        // left imbalance
        if( nDelta > 0 )
            if( pHeavy->m_nBalance == 1 )
                pNewRoot = pPivot->RotLL();
            else
                pNewRoot = pPivot->RotLR();
        // right imbalance
        else if( pHeavy->m_nBalance == -1 )
            pNewRoot = pPivot->RotRR();
        else
            pNewRoot = pPivot->RotRL();
        // relink balanced subtree
        if( pParent == NULL )
            *pRoot = pNewRoot;
        else if( pPivot == pParent->m_pLeft )
            pParent->m_pLeft = pNewRoot;
        else if( pPivot == pParent->m_pRight )
            pParent->m_pRight = pNewRoot;
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
        if( n == p->m_nId )
            break;
        else p = ( n < p->m_nId ) ? p->m_pLeft : p->m_pRight;
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
