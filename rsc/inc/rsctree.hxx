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


#ifndef _RSCTREE_HXX
#define _RSCTREE_HXX

#include <tools/link.hxx>
#include <rsctools.hxx>

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
            sal_Bool        Insert( NameNode * pTN, sal_uInt32 * nDepth );
            sal_Bool        Insert( NameNode* pTN );
    virtual COMPARE     Compare( const NameNode * ) const;
    virtual COMPARE     Compare( const void * ) const;
            NameNode*   SearchParent( const NameNode * ) const;
                        // return ist neue Root
            NameNode*   Remove( NameNode * );
            void        OrderTree();
            sal_Bool        IsOrderTree() const;

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
