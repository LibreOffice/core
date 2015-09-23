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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tools/link.hxx>
#include <rsctree.hxx>


NameNode::NameNode()
{
    pLeft = pRight = NULL;
}

NameNode::~NameNode()
{
}

void NameNode::EnumNodes( Link<const NameNode&,void> aLink ) const
{
    if( Left() )
        Left()->EnumNodes( aLink );
    aLink.Call( *this );
    if( Right() )
        Right()->EnumNodes( aLink );
}

NameNode * NameNode::ChangeDLListBTree( NameNode * pList )
{
    NameNode * pMiddle;
    NameNode * pTmp;
    sal_uInt32 nEle, i;

    if( pList )
    {
        while( pList->Left() )
            pList = pList->Left();
        pTmp = pList;

        for( nEle = 0; pTmp->Right(); nEle++ )
            pTmp = pTmp->Right();

        pMiddle = pList;
        if( nEle / 2 )
        {
            for( i = 0; i < (nEle / 2); i++ )
            {
                pMiddle = pMiddle->Right();
            }
        }
        else
        {
            pList = nullptr;
        }
        if( NULL != (pTmp = pMiddle->Left()) )  // set right pointer to NULL
            pTmp->pRight = nullptr;

        // set left pointer to NULL
        NameNode * pRightNode = pMiddle->Right();
        if (pRightNode)
            pRightNode->pLeft = nullptr;

        pMiddle->pLeft = ChangeDLListBTree( pList );
        pMiddle->pRight = ChangeDLListBTree( pRightNode );

        return pMiddle;
    }
    return pList;
}

NameNode * NameNode::ChangeBTreeDLList()
{
    NameNode * pList;
    NameNode * pLL_RN;    // right node of left list

    if( Right() )
    {
        pList = Right()->ChangeBTreeDLList();
        pRight = pList;
        pList->pLeft = this;
    }
    pList = this;
    if( Left() )
    {
        pLL_RN = pList = Left()->ChangeBTreeDLList();

        while( pLL_RN->Right() )
            pLL_RN = pLL_RN->Right();

        pLeft = pLL_RN;
        pLL_RN->pRight = this;
    }
    return pList;
}

NameNode * NameNode::Remove( NameNode * pRemove )
{
    NameNode * pRoot = this;
    NameNode * pParent = SearchParent( pRemove );

    if( pParent )
    {
        if( pParent->Left() &&
            (EQUAL == pRemove->Compare( pParent->Left() ) ) )
        {
            pParent->pLeft = pRemove->Left();
            if( pRemove->Right() )
                pParent->Insert( pRemove->Right() );
        }
        else if( pParent->Right() &&
                 (EQUAL == pRemove->Compare( pParent->Right() ) ) )
        {
            pParent->pRight = pRemove->Right();
            if( pRemove->Left() )
                pParent->Insert( pRemove->Left() );
        }
    }
    else if( EQUAL == this->Compare( pRemove ) )
    {
        if( Right() )
        {
            pRoot = Right();
            if( Left() )
                Right()->Insert( Left() );
        }
        else
        {
            pRoot = Left();
        }
    }
    pRemove->pLeft = pRemove->pRight = NULL;

    return pRoot;
}


COMPARE NameNode::Compare( const NameNode * pCompare ) const
{
    if( reinterpret_cast<sal_uIntPtr>(this) < reinterpret_cast<sal_uIntPtr>(pCompare) )
        return LESS;
    else if( reinterpret_cast<sal_uIntPtr>(this) > reinterpret_cast<sal_uIntPtr>(pCompare) )
        return GREATER;
    else
        return EQUAL;
}

COMPARE NameNode::Compare( const void * pCompare ) const
{
    if( reinterpret_cast<sal_uIntPtr>(this) < reinterpret_cast<sal_uIntPtr>(pCompare) )
        return LESS;
    else if( reinterpret_cast<sal_uIntPtr>(this) > reinterpret_cast<sal_uIntPtr>(pCompare) )
        return GREATER;
    else
        return EQUAL;
}

// search for a parent node.
// return a pointer to the parent node if found.
// otherwise return 0.
NameNode* NameNode::SearchParent( const NameNode * pSearch ) const
{
    int nCmp = Compare( pSearch );

    if( nCmp == GREATER )
    {
        if( Left() )
        {
            if( Left()->Compare( pSearch ) == EQUAL )
                return const_cast<NameNode *>(this);
            return Left()->SearchParent( pSearch );
        }
    }
    else if( nCmp == LESS )
    {
        if( Right() )
        {
            if( Right()->Compare( pSearch ) == EQUAL )
                return const_cast<NameNode *>(this);
            return Right()->SearchParent( pSearch );
        }
    }
    return nullptr;
}

// search for a node.
// return a pointer to the node if found.
// otherwise return 0.
NameNode* NameNode::Search( const NameNode * pSearch ) const
{
    int nCmp = Compare( pSearch );

    if( nCmp == GREATER )
    {
        if( Left() )
            return Left()->Search( pSearch );
    }
    else if( nCmp == LESS )
    {
        if( Right() )
            return Right()->Search( pSearch );
    }
    else
        return const_cast<NameNode *>(this);

    return NULL;
}

// search for a node.
// return a pointer to the node if found.
// otherwise return 0.
NameNode* NameNode::Search( const void * pSearch ) const
{
    int nCmp = Compare( pSearch );

    if( nCmp == GREATER )
    {
        if( Left() )
            return Left()->Search( pSearch );
    }
    else if( nCmp == LESS )
    {
        if( Right() )
            return Right()->Search( pSearch );
    }
    else
        return const_cast<NameNode *>(this);

    return NULL;
}

// A node is inserted into the tree
// If a node with the same name already exists, then returns false
// otherwise, returns true, In any case, the node will be inserted
bool NameNode::Insert( NameNode * pTN, sal_uInt32* pnDepth )
{
    bool bRet = true;
    int nCmp = Compare( pTN );

    *pnDepth += 1;
    if( nCmp == GREATER )
    {
        if( Left() )
            bRet =  Left()->Insert( pTN, pnDepth );
        else
            pLeft = pTN;
    }
    else
    {
        if( Right() )
            bRet = Right()->Insert( pTN, pnDepth );
        else
            pRight = pTN;

        if( nCmp == EQUAL )
            bRet = false;
    }
    return bRet;
}

// insert a node in the tree.
// if the node with the same name is in, return false and no insert.
// if not return true.
bool NameNode::Insert( NameNode * pTN )
{
    sal_uInt32  nDepth = 0;
    bool        bRet;

    bRet = Insert( pTN, &nDepth );
    if( bRet )
    {
        if( nDepth > 20 )
        {
            if( Left() )
                pLeft =  ChangeDLListBTree(  Left()->ChangeBTreeDLList() );
            if( Right() )
                pRight = ChangeDLListBTree( Right()->ChangeBTreeDLList() );
        }
    }

    return bRet;
}

void NameNode::OrderTree()
{
    NameNode * pTmpLeft = static_cast<NameNode *>(Left());
    NameNode * pTmpRight = static_cast<NameNode *>(Right());

    pLeft = NULL;
    pRight = NULL;
    SubOrderTree( pTmpLeft );
    SubOrderTree( pTmpRight );
}

void NameNode::SubOrderTree( NameNode * pOrderNode )
{
    if( pOrderNode )
    {
        NameNode * pTmpLeft = static_cast<NameNode *>(pOrderNode->Left());
        NameNode * pTmpRight = static_cast<NameNode *>(pOrderNode->Right());
        pOrderNode->pLeft = NULL;
        pOrderNode->pRight = NULL;
        Insert( pOrderNode );
        SubOrderTree( pTmpLeft );
        SubOrderTree( pTmpRight );
    }
}

IdNode * IdNode::Search( sal_uInt32 nTypeName ) const
{
    return static_cast<IdNode *>(NameNode::Search( static_cast<const void *>(&nTypeName) ));
}

COMPARE IdNode::Compare( const NameNode * pSearch ) const
{
    if( GetId() < (sal_uInt32)(static_cast<const IdNode *>(pSearch)->GetId()) )
        return LESS;
    else if( GetId() > (sal_uInt32)(static_cast<const IdNode *>(pSearch)->GetId()) )
        return GREATER;
    else
        return EQUAL;
}

// pSearch is a pointer to sal_uInt32
COMPARE IdNode::Compare( const void * pSearch ) const
{
    if( GetId() < *static_cast<const sal_uInt32 *>(pSearch) )
        return LESS;
    else if( GetId() > *static_cast<const sal_uInt32 *>(pSearch) )
        return GREATER;
    else
        return EQUAL;
}

sal_uInt32 IdNode::GetId() const
{
    return 0xFFFFFFFF;
}

StringNode * StringNode::Search( const char * pSearch ) const
{
    return static_cast<StringNode *>(NameNode::Search( static_cast<const void *>(pSearch) ));
}

COMPARE StringNode::Compare( const NameNode * pSearch ) const
{
    int nCmp = strcmp( m_aName.getStr(),
                       static_cast<const StringNode *>(pSearch)->m_aName.getStr() );
    if( nCmp < 0 )
        return LESS;
    else if( nCmp > 0 )
        return GREATER;
    else
        return EQUAL;
}

// pSearch is a pointer to const char *
COMPARE StringNode::Compare( const void * pSearch ) const
{
    int nCmp = strcmp( m_aName.getStr(), static_cast<const char *>(pSearch) );

    if( nCmp < 0 )
        return LESS;
    else if( nCmp > 0 )
        return GREATER;
    else
        return EQUAL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
