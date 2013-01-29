/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <densebplustree.hxx>

#include <cassert>

/** The tree node order

This affects how big are the metadata and data nodes; the higher the value,
the flatter the structure is.  It is necessary to find a good compromise
between fragmentation (too low value) and not being too flat (too high value).

50 seems to be a good value so far, but we can change it easily.
*/
static const int sOrder = 7;

/** B+ tree node implementation.

It has to be able to act as an internal node, as well as the leaf node.
*/
template < class Key, class Value >
struct DenseBPlusTreeNode
{
    /// The number of children / data entries.
    int m_nUsed;

    /// The B+ tree has the data only in leaves, so we have to distinguish between internal nodes and leaves.
    bool m_bIsInternal : 1;

    /** Keys for this node.

        In the internal nodes, the appropriate values get incremented as we
        insert more and more, and parts of the tree are shifted to the right.

        The real index of a value (when we find it) is a sum of all the
        appropriate m_pKey's that lead to the value.

        In principle, the m_pKeys should always have 0 in m_pKeys[0], let's
        implicitly assume that, and not store it at all.
    */
    Key m_pKeys[ sOrder - 1 ];

    union {
        /// Internal node, contains only pointers to other nodes
        DenseBPlusTreeNode m_pChildren[ sOrder ];

        /// Leaf node, contains data.
        Value m_pValues[ sOrder ];
    };

    /// Pointer to the next node (valid only for the leaf nodes).
    DenseBPlusTreeNode *m_pNext;

    DenseBPlusTreeNode() : m_nUsed( 0 ), m_bIsInternal( false ), m_pNext( NULL ) {}
};

template < class Key, class Value >
DenseBPlusTree< Key, Value >::DenseBPlusTree()
    : m_pRoot( new TreeNode )
{
}

template < class Key, class Value >
DenseBPlusTree< Key, Value >::~DenseBPlusTree()
{
    // TODO
}

template < class Key, class Value >
Key DenseBPlusTree< Key, Value >::Count() const
{
    // TODO
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::Insert( const Value& rValue, Key nPos )
{
    // TODO
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::Remove( Key nPos, Key nNumber )
{
    // TODO
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::Move( Key nFrom, Key nTo )
{
    // TODO
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::Replace( Key nPos, const Value& rValue )
{
    assert( m_pRoot->m_nUsed > 0 );

    NodeWithIndex aLeaf = searchLeaf( nPos, NULL );

    aLeaf.pNode->m_pValues[ aLeaf.nIndex ] = rValue;
}

template < class Key, class Value >
const Value& DenseBPlusTree< Key, Value >::operator[]( Key nPos ) const
{
    assert( m_pRoot->m_nUsed > 0 );

    NodeWithIndex aLeaf = searchLeaf( nPos, NULL );

    return aLeaf.pNode->m_pValues[ aLeaf.nIndex ];
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::ForEach( FnForEach fn, void* pArgs )
{
    // TODO
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::ForEach( Key nStart, Key nEnd, FnForEach fn, void* pArgs )
{
    // TODO
}

template < class Key, class Value >
typename DenseBPlusTree< Key, Value >::NodeWithIndex DenseBPlusTree< Key, Value >::searchLeaf( Key nPos, std::stack< NodeWithIndex > *pParents )
{
    if ( m_pRoot->m_nUsed == 0 )
        return NodeWithIndex( m_pRoot, -1 );

    TreeNode *pNode = m_pRoot;

    // recursion is nice for the alg. description, but for implementation, we
    // want to unwind it
    while ( pNode->m_bIsInternal )
    {
        int i = 0;
        while ( i < pNode->m_nUsed - 1 && pNode->m_pKeys[ i ] <= nPos )
            ++i;

        // m_pKeys in children are relative
        if ( i > 0 )
            nPos -= pNode->m_pKeys[ i - 1 ];

        if ( pParents )
            pParents->push_back( NodeWithIndex( pNode, i ) );

        pNode = pNode->m_pChildren[ i ];
    }

    // now we have the leaf node, check that we are not out of bounds
    assert( nPos < pNode->m_nUsed );

    return NodeWithIndex( pNode, nPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
