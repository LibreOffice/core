/*************************************************************************
 *
 *  $RCSfile: rsctree.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:56 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/tools/rsctree.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.11  2000/09/17 12:51:12  willem.vandorp
    OpenOffice header added.

    Revision 1.10  2000/07/26 17:13:24  willem.vandorp
    Headers/footers replaced

    Revision 1.9  2000/07/11 17:17:55  th
    Unicode

    Revision 1.8  1997/08/27 18:17:52  MM
    neue Headerstruktur

**************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Programmabh„ngige Includes.
#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif

#ifndef _RSCTREE_HXX
#include <rsctree.hxx>
#endif

/****************** C O D E **********************************************/

/****************** B i N o d e ******************************************/
/*************************************************************************
|*
|*    BiNode::BiNode()
|*
|*    Beschreibung      NAME.DOC
|*    Ersterstellung    MM 07.02.91
|*    Letzte Aenderung  MM 07.02.91
|*
*************************************************************************/
BiNode::BiNode(){
    pLeft = pRight = NULL;
}

/*************************************************************************
|*
|*    BiNode::~BiNode()
|*
|*    Beschreibung
|*    Ersterstellung    MM 07.02.91
|*    Letzte Aenderung  MM 07.02.91
|*
*************************************************************************/
BiNode::~BiNode(){
}

/*************************************************************************
|*
|*    BiNode::EnumNodes()
|*
|*    Beschreibung
|*    Ersterstellung    MM 07.02.91
|*    Letzte Aenderung  MM 07.02.91
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
|*    Beschreibung
|*    Ersterstellung    MM 11.01.91
|*    Letzte Aenderung  MM 11.01.91
|*
*************************************************************************/
BiNode * BiNode::ChangeDLListBTree( BiNode * pList ){
    BiNode * pRightNode;
    BiNode * pMiddle;
    BiNode * pTmp;
    USHORT nEle, i;

    if( pList ){
        while( pList->Left() )
            pList = pList->Left();
        pTmp = pList;
        for( nEle = 0; pTmp->Right(); nEle++ )
            pTmp = pTmp->Right();
        pMiddle = pList;
        if( nEle / 2 )
            for( i = 0; i < (USHORT)(nEle / 2); i++ )
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
|*    Beschreibung
|*    Ersterstellung    MM 11.01.91
|*    Letzte Aenderung  MM 11.01.91
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
|*    Beschreibung
|*    Ersterstellung    MM 10.07.91
|*    Letzte Aenderung  MM 10.07.91
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
|*    Beschreibung
|*    Ersterstellung    MM 10.07.91
|*    Letzte Aenderung  MM 13.07.91
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
|*    Beschreibung
|*    Ersterstellung    MM 10.07.91
|*    Letzte Aenderung  MM 10.07.91
|*
*************************************************************************/
NameNode* NameNode::SearchParent( const NameNode * pSearch ) const{
// search for a parent node.
// return a pointer to the parent node if found.
// otherwise return 0.
    short nCmp = Compare( pSearch );

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
|*    Beschreibung
|*    Ersterstellung    MM 21.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
NameNode* NameNode::Search( const NameNode * pSearch ) const{
// search for a node.
// return a pointer to the node if found.
// otherwise return 0.
    short nCmp = Compare( pSearch );

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
    short nCmp = Compare( pSearch );

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
|*    Beschreibung      NAME.DOC
|*    Ersterstellung    MM 11.01.91
|*    Letzte Aenderung  MM 11.01.91
|*
*************************************************************************/
BOOL NameNode::Insert( NameNode * pTN, USHORT * pnDepth ){
// Ein Knoten wird in den Baum eingefuegt
// Gibt es einen Knoten mit dem gleichen Namen, dann return FALSE
// sonst return TRUE. Der Knoten wird auf jeden Fall eingefuegt.

    BOOL bRet = TRUE;
    short nCmp = Compare( pTN );

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
            bRet = FALSE;
    };
    return( bRet );
}

/*************************************************************************
|*
|*    NameNode::Insert()
|*
|*    Beschreibung      NAME.DOC
|*    Ersterstellung    MM 21.03.90
|*    Letzte Aenderung  MM 11.01.91
|*
*************************************************************************/
BOOL NameNode::Insert( NameNode * pTN ){
// insert a node in the tree.
// if the node with the same name is in, return FALSE and no insert.
// if not return true.
    USHORT  nDepth = 0;
    BOOL            bRet;

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
|*    Beschreibung
|*    Ersterstellung    MM 23.09.91
|*    Letzte Aenderung  MM 23.09.91
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
|*    Beschreibung
|*    Ersterstellung    MM 15.11.91
|*    Letzte Aenderung  MM 15.11.91
|*
*************************************************************************/
class OrderCtrl {
    BOOL       bOrder;
    NameNode * pName;
    DECL_LINK( CallBackFunc, NameNode * );
public:
            OrderCtrl() { bOrder = FALSE; pName = NULL; }
    BOOL    IsOrder( const NameNode * pRoot )
    {
            bOrder = TRUE;
            pName  = NULL;
            pRoot->EnumNodes( LINK( this, OrderCtrl, CallBackFunc ) );
            return bOrder;
    };
};
IMPL_LINK_INLINE_START( OrderCtrl, CallBackFunc, NameNode *, pNext )
{
    if( pName && pName->Compare( pNext ) != LESS )
        bOrder = FALSE;
    pName = pNext;
    return 0;
}
IMPL_LINK_INLINE_END( OrderCtrl, CallBackFunc, NameNode *, pNext )

BOOL NameNode::IsOrderTree() const{
    OrderCtrl aOrder;

    return aOrder.IsOrder( this );
}

/****************** I d N o d e ******************************************/
/*************************************************************************
|*
|*    IdNode::Search()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.11.91
|*    Letzte Aenderung  MM 06.11.91
|*
*************************************************************************/
IdNode * IdNode::Search( USHORT nTypeName ) const{
    return( (IdNode *)NameNode::Search( (const void *)&nTypeName ) );
}

/*************************************************************************
|*
|*    IdNode::Compare()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.11.91
|*    Letzte Aenderung  MM 06.11.91
|*
*************************************************************************/
COMPARE IdNode::Compare( const NameNode * pSearch ) const
{
    if( GetId() < (USHORT)(((const IdNode *)pSearch)->GetId()) )
        return LESS;
    else if( GetId() > (USHORT)(((const IdNode *)pSearch)->GetId()) )
        return GREATER;
    else
        return EQUAL;
}

COMPARE IdNode::Compare( const void * pSearch ) const{
// pSearch ist ein Zeiger auf USHORT

    if( GetId() < *((const USHORT *)pSearch) )
        return LESS;
    else if( GetId() > *((const USHORT *)pSearch) )
        return GREATER;
    else
        return EQUAL;
}

/*************************************************************************
|*
|*    IdNode::GetId()
|*
|*    Beschreibung
|*    Ersterstellung    MM 23.09.91
|*    Letzte Aenderung  MM 23.09.91
|*
*************************************************************************/
USHORT IdNode::GetId() const
{
    return( 0xFFFF );
}

/*************************************************************************
|*
|*    StringNode::Search()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.11.91
|*    Letzte Aenderung  MM 06.11.91
|*
*************************************************************************/
StringNode * StringNode::Search( const char * pSearch ) const{
    return (StringNode *)NameNode::Search( (const void *)pSearch );
}

/*************************************************************************
|*
|*    StringNode::Compare()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.11.91
|*    Letzte Aenderung  MM 06.11.91
|*
*************************************************************************/
COMPARE StringNode::Compare( const NameNode * pSearch ) const
{
    short nCmp = strcmp( aName.GetBuffer(),
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
    short nCmp = strcmp( aName.GetBuffer(), (const char *)pSearch );

    if( nCmp < 0 )
        return LESS;
    else if( nCmp > 0 )
        return GREATER;
    else
        return EQUAL;
}
