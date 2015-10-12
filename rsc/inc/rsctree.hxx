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
#ifndef INCLUDED_RSC_INC_RSCTREE_HXX
#define INCLUDED_RSC_INC_RSCTREE_HXX

#include <tools/link.hxx>
#include <rsctools.hxx>

class NameNode
{
    void                SubOrderTree( NameNode * pOrderNode );

protected:
    NameNode*     pLeft;    // left subtree
    NameNode*     pRight;   // right subtree

                        // pCmp ist Zeiger auf Namen
            NameNode*   Search( const void * pCmp ) const;

                         // convert a double linked list into a binary tree
            NameNode*   ChangeDLListBTree( NameNode * pList );

                        NameNode();
    virtual             ~NameNode();

                        // convert a binary tree in a double linked list
                        NameNode* ChangeBTreeDLList();

public:
            void        EnumNodes( Link<const NameNode&,void> aLink ) const;
            NameNode*   Left() const { return pLeft; }
            NameNode*   Right() const{ return pRight; }
            NameNode*   Search( const NameNode * pName ) const;
                        // insert a new node in the b-tree
            bool        Insert( NameNode * pTN, sal_uInt32 * nDepth );
            bool        Insert( NameNode* pTN );
    virtual COMPARE     Compare( const NameNode * ) const;
    virtual COMPARE     Compare( const void * ) const;
            NameNode*   SearchParent( const NameNode * ) const;
                        // returns the new root
            NameNode*   Remove( NameNode * );
            void        OrderTree();
};

class IdNode : public NameNode
{
    virtual COMPARE Compare( const NameNode * ) const override;
    virtual COMPARE Compare( const void * ) const override;
protected:
    using NameNode::Search;

public:

    IdNode*         Search( sal_uInt32 nTypName ) const;
    virtual sal_uInt32  GetId() const;
};

class StringNode : public NameNode
{
    virtual COMPARE Compare( const NameNode * ) const override;
    virtual COMPARE Compare( const void * ) const override;

protected:
    using NameNode::Search;

    OString m_aName;

public:
    StringNode() {}
    StringNode(const OString& rStr) { m_aName = rStr; }

    StringNode*     Search( const char * ) const;
    OString    GetName() const { return m_aName; }
};

#endif // INCLUDED_RSC_INC_RSCTREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
