/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bplustree.hxx>

#include <cassert>

/** B+ tree node implementation.

It has to be able to act as an internal node, as well as the leaf node.
*/
template < class Key, class Value >
struct BPlusTreeNode
{
    /// The tree node order
    static const int sOrder = 7; /// TODO find out the optimal value here :-)

    /// The number of children / data entries.
    int m_nUsed;

    /// The B+ tree has the data only in leaves, so we have to distinguish between internal nodes and leaves.
    bool m_bIsInternal : 1;

    /// Keys for this node (meaning intervals in case of internal nodes, real keys otherwise)
    Key m_pKeys[ sOrder ];

    union {
        /// Internal node, contains only pointers to other nodes
        BPlusTreeNode m_pChildren[ sOrder ];

        /// Leaf node, contains data.
        Value m_pValues[ sOrder ];
    };

    /// Pointer to the next node (valid only for the leaf nodes).
    BPlusTreeNode *m_pNext;

    BPlusTreeNode() : m_nUsed( 0 ), m_pNext( NULL ) {}
};

template < class Key, class Value >
BPlusTree< Key, Value >::BPlusTree()
    : m_pRoot( new BPlusTreeNode< Key, Value > )
{
}

template < class Key, class Value >
BPlusTree< Key, Value >::~BPlusTree()
{
    // TODO
}

template < class Key, class Value >
Key BPlusTree< Key, Value >::Count() const
{
    // TODO
}

template < class Key, class Value >
void BPlusTree< Key, Value >::Insert( const Value& rValue, Key nPos )
{
    // TODO
}

template < class Key, class Value >
void BPlusTree< Key, Value >::Remove( Key nPos, Key nNumber )
{
    // TODO
}

template < class Key, class Value >
void BPlusTree< Key, Value >::Move( Key nFrom, Key nTo )
{
    // TODO
}

template < class Key, class Value >
void BPlusTree< Key, Value >::Replace( Key nPos, const Value& rValue )
{
    // TODO
}

template < class Key, class Value >
const Value& BPlusTree< Key, Value >::operator[]( Key nPos ) const
{
    assert( m_pRoot->m_nUsed > 0 );

    BPlusTreeNode< Key, Value > *pNode = m_pRoot;

    // recursion is nice for the alg. description, but for implementation, we
    // want to unwind it
    while ( pNode->m_bIsInternal )
    {
        for ( int i = 0; i < pNode->m_nUsed - 1 ; ++i )
        {
            if ( nPos < pNode->m_pKeys[ i ] )
            {
                pNode = pNode->m_pChildren[ i ];
                break;
            }
        }
        pNode = pNode->m_pChildren[ pNode->m_nUsed - 1 ];
    }

    // now we have the leaf node
    for ( int i = 0; i < pNode->m_nUsed; ++i )
    {
        if ( nPos == pNode->m_pKeys[ i ] )
            return pNode->m_pValues[ i ];
    }

    // we do not allow not finding a value so far
    assert( false );
}

template < class Key, class Value >
void BPlusTree< Key, Value >::ForEach( FnForEach fn, void* pArgs )
{
    // TODO
}

template < class Key, class Value >
void BPlusTree< Key, Value >::ForEach( Key nStart, Key nEnd, FnForEach fn, void* pArgs )
{
    // TODO
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
