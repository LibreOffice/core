/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sot.hxx"


#include "stgavl.hxx"

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
    StgAvlNode* p = this;
    while( p )
    {
        short nRes = p->Compare( pFind );
        if( !nRes )
            return p;
        else p = ( nRes < 0 ) ? p->pLeft : p->pRight;
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
    *pParent = *pPrev = NULL;
    *pPivot = this;

    // search tree for insertion point

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
    return( nRes );
}

// adjust balance factors in AVL tree from pivot down.
// Returns delta balance.

short StgAvlNode::Adjust( StgAvlNode** pHeavy, StgAvlNode* pNew )
{
    StgAvlNode* pCur = this;
    short nDelta;
    // no traversing
    if( pCur == pNew )
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
    StgAvlNode *pHeavy = pLeft;
    pLeft = pHeavy->pRight;
    pHeavy->pRight = this;
    pHeavy->nBalance = nBalance = 0;
    return pHeavy;
}

// perform LR rotation and return new root

StgAvlNode* StgAvlNode::RotLR()
{

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
    StgAvlNode* pHeavy = pRight;
    pRight = pHeavy->pLeft;
    pHeavy->pLeft = this;
    nBalance = pHeavy->nBalance = 0;
    return pHeavy;
}

// perform the RL rotation and return the new root

StgAvlNode* StgAvlNode::RotRL()
{
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

StgAvlNode* StgAvlNode::Rem( StgAvlNode** p, StgAvlNode* pDel, sal_Bool bPtrs )
{
    if( *p )
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
    if( this )
    {
        if( pLeft )
            pLeft->StgEnum( n );
        nId = n++;
        if( pRight )
            pRight->StgEnum( n );
    }
}

// Add node to AVL tree.
// Return sal_False if the element already exists.

sal_Bool StgAvlNode::Insert( StgAvlNode** pRoot, StgAvlNode* pIns )
{
    StgAvlNode* pPivot, *pHeavy, *pNewRoot, *pParent, *pPrev;
    // special case - empty tree
    if( *pRoot == NULL )
    {
        *pRoot = pIns;
        return sal_True;
    }
    // find insertion point and return if already present
    short nRes = (*pRoot)->Locate( pIns, &pPivot, &pParent, &pPrev );
    if( !nRes )
        return sal_False;
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
    return sal_True;
}

// Remove node from tree. Returns sal_True is found and removed.
// Actually delete if bDel

sal_Bool StgAvlNode::Remove( StgAvlNode** pRoot, StgAvlNode* pDel, sal_Bool bDel )
{
    // special case - empty tree
    if( *pRoot == NULL )
        return sal_False;
    // delete the element
    pDel = Rem( pRoot, pDel, sal_False );
    if( pDel )
    {
        if( bDel )
            delete pDel;
        // Rebalance the tree the hard way
        // OS 22.09.95: Auf MD's Wunsch auskommentiert wg. Absturz
/*      StgAvlNode* pNew = NULL;
        while( *pRoot )
        {
            StgAvlNode* p = Rem( pRoot, *pRoot, sal_False );
            Insert( &pNew, p );
        }
        *pRoot = pNew;*/
        return sal_True;
    }
    else
        return sal_False;
}

// Move node to a different tree. Returns sal_True is found and moved. This routine
// may be called when the key has changed.

sal_Bool StgAvlNode::Move
    ( StgAvlNode** pRoot1, StgAvlNode** pRoot2, StgAvlNode* pMove )
{
    // special case - empty tree
    if( *pRoot1 == NULL )
        return sal_False;
    pMove = Rem( pRoot1, pMove, sal_False );
    if( pMove )
        return Insert( pRoot2, pMove );
    else
        return sal_False;
}

////////////////////////// class AvlIterator /////////////////////////

// The iterator walks through a tree one entry by one.

StgAvlIterator::StgAvlIterator( StgAvlNode* p )
{
    pRoot = p;
    nCount = 0;
    if( p )
        p->StgEnum( nCount );
}

StgAvlNode* StgAvlIterator::Find( short n )
{
    StgAvlNode* p = pRoot;
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
    nCur = -1;
    return Next();
}

StgAvlNode* StgAvlIterator::Last()
{
    nCur = nCount;
    return Prev();
}

StgAvlNode* StgAvlIterator::Next()
{
    return Find( ++nCur );
}

StgAvlNode* StgAvlIterator::Prev()
{
    return Find( --nCur );
}

