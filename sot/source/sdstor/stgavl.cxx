/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl/diagnose.h>
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




short StgAvlNode::Locate
    ( StgAvlNode* pFind,
      StgAvlNode** pPivot, StgAvlNode **pParent, StgAvlNode** pPrev )
{
    short nRes = 0;
    StgAvlNode* pCur = this;

    OSL_ENSURE( pPivot && pParent && pPrev, "The pointers may not be NULL!" );
    *pParent = *pPrev = NULL;
    *pPivot = this;

    
    if ( pFind )
    {
        while( pCur != NULL )
        {
            
            if( pCur->nBalance != 0 )
                *pPivot = pCur, *pParent = *pPrev;
            
            *pPrev = pCur;
            nRes = pCur->Compare( pFind );
            if( nRes == 0 )
                break;
            else pCur = ( nRes < 0 ) ? pCur->pLeft : pCur->pRight;
        }
    }

    return( nRes );
}




short StgAvlNode::Adjust( StgAvlNode** pHeavy, StgAvlNode* pNew )
{
    StgAvlNode* pCur = this;
    short nDelta;
    
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
            
            pCur->nBalance = -1;
            pCur = pCur->pRight;
        }
        else
        {
            
            pCur->nBalance = 1;
            pCur = pCur->pLeft;
        }
    }
    nBalance = nBalance + nDelta;
    return nDelta;
}



StgAvlNode* StgAvlNode::RotLL()
{
    OSL_ENSURE( pLeft, "The pointer is not allowed to be NULL!" );
    StgAvlNode *pHeavy = pLeft;
    pLeft = pHeavy->pRight;
    pHeavy->pRight = this;
    pHeavy->nBalance = nBalance = 0;
    return pHeavy;
}



StgAvlNode* StgAvlNode::RotLR()
{
    OSL_ENSURE( pLeft && pLeft->pRight, "The pointer is not allowed to be NULL!" );
    StgAvlNode* pHeavy = pLeft;
    StgAvlNode* pNewRoot = pHeavy->pRight;

    pHeavy->pRight = pNewRoot->pLeft;
    pLeft = pNewRoot->pRight;
    pNewRoot->pLeft = pHeavy;
    pNewRoot->pRight = this;

    switch( pNewRoot->nBalance )
    {
        case 1:     
            nBalance = -1;
            pHeavy->nBalance = 0;
            break;
        case -1:    
            pHeavy->nBalance = 1;
            nBalance = 0;
            break;
        case 0:     
            nBalance = 0;
            pHeavy->nBalance = 0;
            break;
    }
    pNewRoot->nBalance = 0;
    return pNewRoot;
}



StgAvlNode* StgAvlNode::RotRR()
{
    OSL_ENSURE( pRight, "The pointer is not allowed to be NULL!" );
    StgAvlNode* pHeavy = pRight;
    pRight = pHeavy->pLeft;
    pHeavy->pLeft = this;
    nBalance = pHeavy->nBalance = 0;
    return pHeavy;
}



StgAvlNode* StgAvlNode::RotRL()
{
    OSL_ENSURE( pRight && pRight->pLeft, "The pointer is not allowed to be NULL!" );
    StgAvlNode* pHeavy = pRight;
    StgAvlNode* pNewRoot = pHeavy->pLeft;
    pHeavy->pLeft = pNewRoot->pRight;
    pRight = pNewRoot->pLeft;
    pNewRoot->pRight = pHeavy;
    pNewRoot->pLeft = this;
    switch( pNewRoot->nBalance )
    {
        case -1:    
            nBalance = 1;
            pHeavy->nBalance = 0;
            break;
        case 1:     
            pHeavy->nBalance = -1;
            nBalance = 0;
            break;
        case 0:     
            nBalance = 0;
            pHeavy->nBalance = 0;
            break;
    }
    pNewRoot->nBalance = 0;
    return pNewRoot;
}



StgAvlNode* StgAvlNode::Rem( StgAvlNode** p, StgAvlNode* pDel, bool bPtrs )
{
    if( p && *p && pDel )
    {
        StgAvlNode* pCur = *p;
        short nRes = bPtrs ? short( pCur == pDel ) : short(pCur->Compare( pDel ));
        if( !nRes )
        {
            
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
                
                
                
                
                StgAvlNode* last = pCur;
                StgAvlNode* l;
                for( l = pCur->pLeft;
                     l->pRight; last = l, l = l->pRight ) {}
                
                if( l == last->pRight )
                    last->pRight = l->pLeft;
                else
                    last->pLeft = l->pLeft;
                
                l->pLeft = pCur->pLeft;
                l->pRight = pCur->pRight;
                *p = l;
                
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



void StgAvlNode::StgEnum( short& n )
{
    if( pLeft )
        pLeft->StgEnum( n );
    nId = n++;
    if( pRight )
        pRight->StgEnum( n );
}




bool StgAvlNode::Insert( StgAvlNode** pRoot, StgAvlNode* pIns )
{
    StgAvlNode* pPivot, *pHeavy, *pNewRoot, *pParent, *pPrev;
    if ( !pRoot )
        return false;

    
    if( *pRoot == NULL )
    {
        *pRoot = pIns;
        return true;
    }
    
    short nRes = (*pRoot)->Locate( pIns, &pPivot, &pParent, &pPrev );
    if( !nRes )
        return false;
    OSL_ENSURE( pPivot && pPrev, "The pointers may not be NULL!" );

    
    if( nRes < 0 )
        pPrev->pLeft = pIns;
    else
        pPrev->pRight = pIns;
    
    short nDelta = pPivot->Adjust( &pHeavy, pIns );
    if( pPivot->nBalance >= 2 || pPivot->nBalance <= -2 )
    {
        pHeavy = ( nDelta < 0 ) ? pPivot->pRight : pPivot->pLeft;
        
        if( nDelta > 0 )
            if( pHeavy->nBalance == 1 )
                pNewRoot = pPivot->RotLL();
            else
                pNewRoot = pPivot->RotLR();
        
        else if( pHeavy->nBalance == -1 )
            pNewRoot = pPivot->RotRR();
        else
            pNewRoot = pPivot->RotRL();
        
        if( pParent == NULL )
            *pRoot = pNewRoot;
        else if( pPivot == pParent->pLeft )
            pParent->pLeft = pNewRoot;
        else if( pPivot == pParent->pRight )
            pParent->pRight = pNewRoot;
    }
    return true;
}




bool StgAvlNode::Remove( StgAvlNode** pRoot, StgAvlNode* pDel, bool bDel )
{
    if ( !pRoot )
        return false;

    
    if( *pRoot == NULL )
        return false;
    
    pDel = Rem( pRoot, pDel, false );
    if( pDel )
    {
        if( bDel )
            delete pDel;
        
        
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




bool StgAvlNode::Move( StgAvlNode** pRoot1, StgAvlNode** pRoot2, StgAvlNode* pMove )
{
    if ( !pRoot1 )
        return false;

    
    if( *pRoot1 == NULL )
        return false;
    pMove = Rem( pRoot1, pMove, false );
    if( pMove )
        return Insert( pRoot2, pMove );
    else
        return false;
}





StgAvlIterator::StgAvlIterator( StgAvlNode* p )
{
    pRoot = p;
    nCount = 0;
    nCur = 0;
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

StgAvlNode* StgAvlIterator::Next()
{
    return Find( ++nCur );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
