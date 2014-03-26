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
#ifndef _RSCTREE_HXX
#define _RSCTREE_HXX

#include <tools/link.hxx>
#include <rsctools.hxx>

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
            bool        Insert( NameNode * pTN, sal_uInt32 * nDepth );
            bool        Insert( NameNode* pTN );
    virtual COMPARE     Compare( const NameNode * ) const;
    virtual COMPARE     Compare( const void * ) const;
            NameNode*   SearchParent( const NameNode * ) const;
                        // return ist neue Root
            NameNode*   Remove( NameNode * );
            void        OrderTree();
};

class IdNode : public NameNode
{
    virtual COMPARE Compare( const NameNode * ) const SAL_OVERRIDE;
    virtual COMPARE Compare( const void * ) const SAL_OVERRIDE;
protected:
    using NameNode::Search;

public:

    IdNode*         Search( sal_uInt32 nTypName ) const;
    virtual sal_uInt32  GetId() const;
};

class StringNode : public NameNode
{
    virtual COMPARE Compare( const NameNode * ) const SAL_OVERRIDE;
    virtual COMPARE Compare( const void * ) const SAL_OVERRIDE;

protected:
    using NameNode::Search;

    OString m_aName;

public:
    StringNode() {}
    StringNode(const OString& rStr) { m_aName = rStr; }

    StringNode*     Search( const char * ) const;
    OString    GetName() const { return m_aName; }
};

#endif // _RSCTREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
