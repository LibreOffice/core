/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rsctree.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:45:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _RSCTREE_HXX
#define _RSCTREE_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif

/****************** C L A S S E S ****************************************/
class BiNode
{
protected:
    BiNode*     pLeft;    // left subtree
    BiNode*     pRight;   // right subtree

public:

                         // Wandelt eine doppelt verkettete Liste in
                         // einen binaeren Baum um
            BiNode *    ChangeDLListBTree( BiNode * pList );

                        BiNode();
    virtual             ~BiNode();


                        // Wandelt einen binaeren Baum in eine doppelt
                        // verkettete Liste um
                        BiNode* ChangeBTreeDLList();

            BiNode *    Left() const { return pLeft  ; };
            BiNode *    Right() const{ return pRight ; };
            void        EnumNodes( Link aLink ) const;
};

/*************************************************************************/
class NameNode : public BiNode
{
    void                SubOrderTree( NameNode * pOrderNode );

protected:
                        // pCmp ist Zeiger auf Namen
            NameNode*   Search( const void * pCmp ) const;

public:
            NameNode*   Left() const { return (NameNode *)pLeft  ; };
            NameNode*   Right() const{ return (NameNode *)pRight ; };
            NameNode*   Search( const NameNode * pName ) const;
                        // insert a new node in the b-tree
            BOOL        Insert( NameNode * pTN, sal_uInt32 * nDepth );
            BOOL        Insert( NameNode* pTN );
    virtual COMPARE     Compare( const NameNode * ) const;
    virtual COMPARE     Compare( const void * ) const;
            NameNode*   SearchParent( const NameNode * ) const;
                        // return ist neue Root
            NameNode*   Remove( NameNode * );
            void        OrderTree();
            BOOL        IsOrderTree() const;

};

/*************************************************************************/
class IdNode : public NameNode
{
    virtual COMPARE Compare( const NameNode * ) const;
    virtual COMPARE Compare( const void * ) const;
protected:
    using NameNode::Search;

public:

    IdNode*         Search( sal_uInt32 nTypName ) const;
    virtual sal_uInt32  GetId() const;
};

/*************************************************************************/
class StringNode : public NameNode
{
    virtual COMPARE Compare( const NameNode * ) const;
    virtual COMPARE Compare( const void * ) const;

protected:
    using NameNode::Search;

    ByteString      aName;

public:
                    StringNode(){};
                    StringNode( const ByteString & rStr ) { aName = rStr; }

    StringNode*     Search( const char * ) const;
    ByteString      GetName() const { return aName; }
};

#endif // _RSCTREE_HXX
