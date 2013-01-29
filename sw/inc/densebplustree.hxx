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

#ifndef SW_BPLUSTREE_HXX
#define SW_BPLUSTREE_HXX

#include <tools/solar.h>
#include <osl/diagnose.h>
#include <swdllapi.h>

#include <stack>

template < class Key, class Value > struct DenseBPlusTreeNode;

/** Dense B+ tree implementation (to replace the original BigPtrArray).

This structure is a modification of a B+ tree, see eg. wikipedia:
http://en.wikipedia.org/wiki/B%2B_tree
for the B+ tree implementation.

The problem with 'classic' B+ tree is that it is designed for key/value access
where the key typically is not a sequence of numbers.  Consequently, it does
not easily allow inserting in a sense that 'the rest of the values shift to
the right' - but that is the operation that we need to do effectively.

We do a small modification to the B+ tree implementation to make it 'dense' -
the keys are supposed to be a sequence, and if you insert a value, all shifts
to the right.  The trick to achieve it is that the values that are stored in
the internal nodes are not absolute, but are relative; so whatever is in the
parents is propagated to the children.  That way, shifting half of the tree by
1 to the right after insertion is just a matter of adding 1 to the appropriate
parent.

As part of the removal of BigPtrArray (and consequent refactor of the related
code), this structur is supposed to be a drop-in replacement, with some of
the functionality templatized for easier use.

Key is sal_uLong in the BigPtrArray implementation.
Value is supposed to be SwNodePtr initially.
*/
template < class Key, class Value >
class SW_DLLPUBLIC DenseBPlusTree
{
public:
    /// Callback function to be called during ForEach.
    typedef bool (*FnForEach)( const Value&, void* pArgs );

public:
    DenseBPlusTree();
    ~DenseBPlusTree();

    /// Number of elements.
    Key Count() const;

    /// Insert entry at the specified position.
    void Insert( const Value& rValue, Key nPos );

    /// Remove nNumber entries starting at the position nPos.
    void Remove( Key nPos, Key nNumber = 1 );

    /// Insert the value of 'nFrom' to the position 'nTo', and remove the original value.
    void Move( Key nFrom, Key nTo );

    /// Exchange the value on position pos with the new one.
    void Replace( Key nPos, const Value& rValue );

    /// Field access.
    const Value& operator[]( Key nPos ) const;

    /// Traverse over the entire data, and call fn on the data.
    void ForEach( FnForEach fn, void* pArgs = NULL );

    /// Traverse over the specified range, and call fn on the data.
    void ForEach( Key nStart, Key nEnd, FnForEach fn, void* pArgs = NULL );

private:
    typedef DenseBPlusTreeNode< Key, Value > TreeNode;

    /// We need to know the exact path from the root to the leaf, including the indexes for various operations
    struct NodeWithIndex {
        TreeNode *pNode;
        Key nIndex;

        NodeWithIndex( TreeNode *p, Key n ) : pNode( p ), nIndex( n ) {}
    };

    /// Root of the tree.
    TreeNode *m_pRoot;

    /** Search for the leaf node containing nPos.

        @return the leaf node containing nPos
        @param pParents stack of parents of the returned tree node so that we can traverse it back to the root
    */
    NodeWithIndex searchLeaf( Key nPos, std::stack< NodeWithIndex > *pParents = NULL );
};

#endif // SW_BPLUSTREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
