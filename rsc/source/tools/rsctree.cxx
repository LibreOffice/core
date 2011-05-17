/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_rsc.hxx"
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Programmabh�ngige Includes.
#include <tools/link.hxx>
#include <rsctree.hxx>

/****************** C O D E **********************************************/

/****************** B i N o d e ******************************************/
/*************************************************************************
|*
|*    BiNode::BiNode()
|*
*************************************************************************/
BiNode::BiNode(){
    pLeft = pRight = NULL;
}

/*************************************************************************
|*
|*    BiNode::~BiNode()
|*
*************************************************************************/
BiNode::~BiNode(){
}

/*************************************************************************
|*
|*    BiNode::EnumNodes()
|*
*************************************************************************/
void BiNode::EnumNodes( Link aLink ) const{
    if( Left() )
        Left()->EnumNodes( aLink );
    aLink.Call( (BiNode *)this );
    if( Right() )
        Right()->EnumNodes( aLink );
}

/*************************************************************************
|*
|*    BiNode::ChangeDLListBTree()
|*
*************************************************************************/
BiNode * BiNode::ChangeDLListBTree( BiNode * pList ){
    BiNode * pRightNode;
    BiNode * pMiddle;
    BiNode * pTmp;
    sal_uInt32 nEle, i;

    if( pList ){
        while( pList->Left() )
            pList = pList->Left();
        pTmp = pList;
        for( nEle = 0; pTmp->Right(); nEle++ )
            pTmp = pTmp->Right();
        pMiddle = pList;
        if( nEle / 2 )
            for( i = 0; i < (nEle / 2); i++ )
                pMiddle = pMiddle->Right();
        else
            pList = (BiNode *)0;

        if( NULL != (pTmp = pMiddle->Left()) )  // rechten Zeiger auf Null
            pTmp->pRight = (BiNode *)0;

        // linken Zeiger auf Null
        if( NULL != (pRightNode = pMiddle->Right()) )
            pRightNode->pLeft = (BiNode *)0;

        pMiddle->pLeft = ChangeDLListBTree( pList );
        pMiddle->pRight = ChangeDLListBTree( pRightNode );

        return( pMiddle );
    }
    return( pList );
}

/*************************************************************************
|*
|*    BiNode::ChangeBTreeDLList()
|*
*************************************************************************/
BiNode * BiNode::ChangeBTreeDLList(){
    BiNode * pList;
    BiNode * pLL_RN;    // linke Liste rechter Knoten

    if( Right() ){
        pList = Right()->ChangeBTreeDLList();
        pRight = pList;
        pList->pLeft = this;
    }
    pList = this;
    if( Left() ){
        pLL_RN = pList = Left()->ChangeBTreeDLList();
        while( pLL_RN->Right() )
            pLL_RN = pLL_RN->Right();
        pLeft = pLL_RN;
        pLL_RN->pRight = this;
    }
    return( pList );
}

/****************** N a m e N o d e **************************************/
/*************************************************************************
|*
|*    NameNode::Remove()
|*
*************************************************************************/
NameNode * NameNode::Remove( NameNode * pRemove ){
    NameNode * pRoot = this;
    NameNode * pParent = SearchParent( pRemove );

    if( pParent ){
        if( pParent->Left()
          && (EQUAL == pRemove->Compare( pParent->Left() ) ) ){
            pParent->pLeft = pRemove->Left();
            if( pRemove->Right() )
                pParent->Insert( pRemove->Right() );
        }
        else if( pParent->Right()
          && (EQUAL == pRemove->Compare( pParent->Right() ) ) ){
            pParent->pRight = pRemove->Right();
            if( pRemove->Left() )
                pParent->Insert( pRemove->Left() );
        }
    }
    else if( EQUAL == this->Compare( pRemove ) ){
        if( Right() ){
            pRoot = Right();
            if( Left() )
                Right()->Insert( Left() );
        }
        else{
            pRoot = Left();
        }
    }
    pRemove->pLeft = pRemove->pRight = NULL;

    return pRoot;
}


/*************************************************************************
|*
|*    NameNode::Compare
|*
*************************************************************************/
COMPARE NameNode::Compare( const NameNode * pCompare ) const{
    if( (long)this < (long)pCompare )
        return LESS;
    else if( (long)this > (long)pCompare )
        return GREATER;
    else
        return EQUAL;
}

COMPARE NameNode::Compare( const void * pCompare ) const{
    if( (long)this < (long)pCompare )
        return LESS;
    else if( (long)this > (long)pCompare )
        return GREATER;
    else
        return EQUAL;
}

/*************************************************************************
|*
|*    NameNode::SearchParent
|*
*************************************************************************/
NameNode* NameNode::SearchParent( const NameNode * pSearch ) const{
// search for a parent node.
// return a pointer to the parent node if found.
// otherwise return 0.
    int nCmp = Compare( pSearch );

    if( nCmp == GREATER ){
        if( Left() ){
            if( ((NameNode *)Left())->Compare( pSearch ) == EQUAL )
                return (NameNode *)this;
            return ((NameNode *)Left())->SearchParent( pSearch );
        };
    }
    else if( nCmp == LESS ){
        if( Right() ){
            if( ((NameNode *)Right())->Compare( pSearch ) == EQUAL )
                return (NameNode *)this;
            return ((NameNode *)Right())->SearchParent( pSearch );
        }
    };
    return( (NameNode *)NULL );
}

/*************************************************************************
|*
|*    NameNode::Search
|*
*************************************************************************/
NameNode* NameNode::Search( const NameNode * pSearch ) const{
// search for a node.
// return a pointer to the node if found.
// otherwise return 0.
    int nCmp = Compare( pSearch );

    if( nCmp == GREATER ){
        if( Left() )
            return ((NameNode *)Left())->Search( pSearch );
    }
    else if( nCmp == LESS ){
        if( Right() )
            return ((NameNode *)Right())->Search( pSearch );
    }
    else
        return( (NameNode *)this );

    return( NULL );
}

NameNode* NameNode::Search( const void * pSearch ) const{
// search for a node.
// return a pointer to the node if found.
// otherwise return 0.
    int nCmp = Compare( pSearch );

    if( nCmp == GREATER ){
        if( Left() )
            return ((NameNode *)Left())->Search( pSearch );
    }
    else if( nCmp == LESS ){
        if( Right() )
            return ((NameNode *)Right())->Search( pSearch );
    }
    else
        return( (NameNode *)this );

    return( NULL );
}

/*************************************************************************
|*
|*    NameNode::Insert()
|*
*************************************************************************/
sal_Bool NameNode::Insert( NameNode * pTN, sal_uInt32* pnDepth ){
// Ein Knoten wird in den Baum eingefuegt
// Gibt es einen Knoten mit dem gleichen Namen, dann return sal_False
// sonst return sal_True. Der Knoten wird auf jeden Fall eingefuegt.

    sal_Bool bRet = sal_True;
    int nCmp = Compare( pTN );

    *pnDepth += 1;
    if( nCmp == GREATER ){
        if( Left() )
            bRet =  ((NameNode *)Left())->Insert( pTN, pnDepth );
        else
            pLeft = pTN;
    }
    else{
        if( Right() )
            bRet = ((NameNode *)Right())->Insert( pTN, pnDepth );
        else
            pRight = pTN;
        if( nCmp == EQUAL )
            bRet = sal_False;
    };
    return( bRet );
}

/*************************************************************************
|*
|*    NameNode::Insert()
|*
*************************************************************************/
sal_Bool NameNode::Insert( NameNode * pTN ){
// insert a node in the tree.
// if the node with the same name is in, return sal_False and no insert.
// if not return true.
    sal_uInt32  nDepth = 0;
    sal_Bool        bRet;

    bRet = Insert( pTN, &nDepth );
    if( bRet ){
        if( nDepth > 20 ){
            if( Left() )
                pLeft =  ChangeDLListBTree(  Left()->ChangeBTreeDLList() );
            if( Right() )
                pRight = ChangeDLListBTree( Right()->ChangeBTreeDLList() );
        }
    }

    return( bRet );
}

/*************************************************************************
|*
|*    NameNode::OrderTree()
|*
*************************************************************************/
void NameNode::OrderTree(){
    NameNode * pTmpLeft = (NameNode *)Left();
    NameNode * pTmpRight = (NameNode *)Right();

    pLeft = NULL;
    pRight = NULL;
    SubOrderTree( pTmpLeft );
    SubOrderTree( pTmpRight );
}

void NameNode::SubOrderTree( NameNode * pOrderNode ){
    if( pOrderNode ){
        NameNode * pTmpLeft = (NameNode *)pOrderNode->Left();
        NameNode * pTmpRight = (NameNode *)pOrderNode->Right();
        pOrderNode->pLeft = NULL;
        pOrderNode->pRight = NULL;
        Insert( pOrderNode );
        SubOrderTree( pTmpLeft );
        SubOrderTree( pTmpRight );
    }
}

/*************************************************************************
|*
|*    NameNode::IdOrderTree()
|*
*************************************************************************/
class OrderCtrl {
    sal_Bool       bOrder;
    NameNode * pName;
    DECL_LINK( CallBackFunc, NameNode * );
public:
            OrderCtrl() { bOrder = sal_False; pName = NULL; }
    sal_Bool    IsOrder( const NameNode * pRoot )
    {
            bOrder = sal_True;
            pName  = NULL;
            pRoot->EnumNodes( LINK( this, OrderCtrl, CallBackFunc ) );
            return bOrder;
    };
};
IMPL_LINK_INLINE_START( OrderCtrl, CallBackFunc, NameNode *, pNext )
{
    if( pName && pName->Compare( pNext ) != LESS )
        bOrder = sal_False;
    pName = pNext;
    return 0;
}
IMPL_LINK_INLINE_END( OrderCtrl, CallBackFunc, NameNode *, pNext )

sal_Bool NameNode::IsOrderTree() const{
    OrderCtrl aOrder;

    return aOrder.IsOrder( this );
}

/****************** I d N o d e ******************************************/
/*************************************************************************
|*
|*    IdNode::Search()
|*
*************************************************************************/
IdNode * IdNode::Search( sal_uInt32 nTypeName ) const{
    return( (IdNode *)NameNode::Search( (const void *)&nTypeName ) );
}

/*************************************************************************
|*
|*    IdNode::Compare()
|*
*************************************************************************/
COMPARE IdNode::Compare( const NameNode * pSearch ) const
{
    if( GetId() < (sal_uInt32)(((const IdNode *)pSearch)->GetId()) )
        return LESS;
    else if( GetId() > (sal_uInt32)(((const IdNode *)pSearch)->GetId()) )
        return GREATER;
    else
        return EQUAL;
}

COMPARE IdNode::Compare( const void * pSearch ) const{
// pSearch ist ein Zeiger auf sal_uInt32

    if( GetId() < *((const sal_uInt32 *)pSearch) )
        return LESS;
    else if( GetId() > *((const sal_uInt32 *)pSearch) )
        return GREATER;
    else
        return EQUAL;
}

/*************************************************************************
|*
|*    IdNode::GetId()
|*
*************************************************************************/
sal_uInt32 IdNode::GetId() const
{
    return( 0xFFFFFFFF );
}

/*************************************************************************
|*
|*    StringNode::Search()
|*
*************************************************************************/
StringNode * StringNode::Search( const char * pSearch ) const{
    return (StringNode *)NameNode::Search( (const void *)pSearch );
}

/*************************************************************************
|*
|*    StringNode::Compare()
|*
*************************************************************************/
COMPARE StringNode::Compare( const NameNode * pSearch ) const
{
    int nCmp = strcmp( aName.GetBuffer(),
                       ((const StringNode *)pSearch)->aName.GetBuffer() );
    if( nCmp < 0 )
        return LESS;
    else if( nCmp > 0 )
        return GREATER;
    else
        return EQUAL;
}

COMPARE StringNode::Compare( const void * pSearch ) const
{
// pSearch ist ein Zeiger auf const char *
    int nCmp = strcmp( aName.GetBuffer(), (const char *)pSearch );

    if( nCmp < 0 )
        return LESS;
    else if( nCmp > 0 )
        return GREATER;
    else
        return EQUAL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
