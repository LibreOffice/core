/*************************************************************************
 *
 *  $RCSfile: stgavl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:56:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "stgavl.hxx"
#pragma hdrstop

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
    short nRes;
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
    nBalance += nDelta;
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

StgAvlNode* StgAvlNode::Rem( StgAvlNode** p, StgAvlNode* pDel, BOOL bPtrs )
{
    if( *p )
    {
        StgAvlNode* pCur = *p;
        short nRes = bPtrs ? ( pCur == pDel ) : pCur->Compare( pDel );
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

void StgAvlNode::Enum( short& n )
{
    if( this )
    {
        if( pLeft )
            pLeft->Enum( n );
        nId = n++;
        if( pRight )
            pRight->Enum( n );
    }
}

// Add node to AVL tree.
// Return FALSE if the element already exists.

BOOL StgAvlNode::Insert( StgAvlNode** pRoot, StgAvlNode* pIns )
{
    StgAvlNode* pPivot, *pHeavy, *pNewRoot, *pParent, *pPrev;
    // special case - empty tree
    if( *pRoot == NULL )
    {
        *pRoot = pIns;
        return TRUE;
    }
    // find insertion point and return if already present
    short nRes = (*pRoot)->Locate( pIns, &pPivot, &pParent, &pPrev );
    if( !nRes )
        return FALSE;
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
    return TRUE;
}

// Remove node from tree. Returns TRUE is found and removed.
// Actually delete if bDel

BOOL StgAvlNode::Remove( StgAvlNode** pRoot, StgAvlNode* pDel, BOOL bDel )
{
    // special case - empty tree
    if( *pRoot == NULL )
        return FALSE;
    // delete the element
    pDel = Rem( pRoot, pDel, FALSE );
    if( pDel )
    {
        if( bDel )
            delete pDel;
        // Rebalance the tree the hard way
        // OS 22.09.95: Auf MD's Wunsch auskommentiert wg. Absturz
/*      StgAvlNode* pNew = NULL;
        while( *pRoot )
        {
            StgAvlNode* p = Rem( pRoot, *pRoot, FALSE );
            Insert( &pNew, p );
        }
        *pRoot = pNew;*/
        return TRUE;
    }
    else
        return FALSE;
}

// Move node to a different tree. Returns TRUE is found and moved. This routine
// may be called when the key has changed.

BOOL StgAvlNode::Move
    ( StgAvlNode** pRoot1, StgAvlNode** pRoot2, StgAvlNode* pMove )
{
    // special case - empty tree
    if( *pRoot1 == NULL )
        return FALSE;
    pMove = Rem( pRoot1, pMove, FALSE );
    if( pMove )
        return Insert( pRoot2, pMove );
    else
        return FALSE;
}

////////////////////////// class AvlIterator /////////////////////////

// The iterator walks through a tree one entry by one.

StgAvlIterator::StgAvlIterator( StgAvlNode* p )
{
    pRoot = p;
    nCount = 0;
    if( p )
        p->Enum( nCount );
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

