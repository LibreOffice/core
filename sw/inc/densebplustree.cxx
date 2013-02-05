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
#include <cstdio>
#include <cstring>

#include <vector>

using namespace std;

/** The tree node order

This affects how big are the metadata and data nodes; the higher the value,
the flatter the structure is.  It is necessary to find a good compromise
between fragmentation (too low value) and not being too flat (too high value).

50 seems to be a good value so far, but we can change it easily if necessary.
*/
static const int sOrder = 50;

/** Minimum fill of a node.

Nodes except the rightmost ones will never have less than this number.
*/
static const int sMinFill = ( sOrder / 2 ) - 1;

/** B+ tree node implementation.

It has to be able to act as an internal node, as well as the leaf node.
*/
template < class Key, class Value >
struct DBPTreeNode
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
        DBPTreeNode* m_pChildren[ sOrder ];

        /// Leaf node, contains data.
        Value m_pValues[ sOrder ];
    };

    /// Pointer to the next node (valid only for the leaf nodes).
    DBPTreeNode *m_pNext;

    DBPTreeNode() : m_nUsed( 0 ), m_bIsInternal( false ), m_pNext( NULL ) {}

    /// Insert the value (for leaf nodes only).
    void insert( int nWhere, const Value& rValue )
    {
        assert( !m_bIsInternal );
        assert( nWhere <= m_nUsed );
        assert( nWhere < sOrder );

        for ( int i = m_nUsed; i > nWhere; --i )
            m_pValues[ i ] = m_pValues[ i - 1 ];

        m_pValues[ nWhere ] = rValue;
        ++m_nUsed;
    }

    /// Insert a new child node (for internal nodes only).
    void insert( int nWhere, int nOffset, DBPTreeNode* pNewNode )
    {
        assert( m_bIsInternal );
        assert( nWhere <= m_nUsed );
        assert( nWhere < sOrder );
        assert( nWhere > 0 ); // we always add the node to the right when splitting

        for ( int i = m_nUsed; i > nWhere; --i )
        {
            m_pChildren[ i ] = m_pChildren[ i - 1 ];
            m_pKeys[ i - 1 ] = m_pKeys[ i - 2 ];
        }

        m_pChildren[ nWhere ] = pNewNode;
        if ( nWhere - 2 >= 0 )
            m_pKeys[ nWhere - 1 ] = m_pKeys[ nWhere - 2 ] + nOffset;
        else
            m_pKeys[ nWhere - 1 ] = nOffset;

        ++m_nUsed;
    }

    /// Remove the given amount of content (regardless of the node type).
    void remove( int nWhere, int nCount )
    {
        assert( nWhere < m_nUsed );
        assert( nCount > 0 );
        assert( nWhere + nCount <= m_nUsed );

        if ( m_bIsInternal )
        {
            for ( int i = nWhere; i < m_nUsed - nCount; ++i )
                m_pChildren[ i ] = m_pChildren[ i + nCount ];

            for ( int i = nWhere - 1; i < m_nUsed - nCount - 1; ++i )
                m_pKeys[ i ] = m_pKeys[ i + nCount ];
        }
        else
        {
            for ( int i = nWhere; i < m_nUsed - nCount; ++i )
                m_pValues[ i ] = m_pValues[ i + nCount ];
        }

        m_nUsed -= nCount;
    }

    /** Split node, and make the original one smaller.

        @return relative key shift of the node.
        @param bIsAppend in case we are appending, we deliberately keep most of the data untouched, creating as empty node as possible
    */
    int copyFromSplitNode( DBPTreeNode *pNode, bool bIsAppend )
    {
        assert( sOrder > 2 );
        assert( pNode->m_nUsed == sOrder );

        // we optimize for the case of appending
        // it is expected that we first create the entire structure (so want
        // it to be dense from the space point of view), but when performing
        // later, the distribution has to be 'fair', because the access is
        // more or less random
        int nHowMuchKeep = bIsAppend? sOrder - 2: sOrder / 2;

        int offset = 0;

        m_bIsInternal = pNode->m_bIsInternal;
        if ( m_bIsInternal )
        {
            for ( int i = nHowMuchKeep; i < pNode->m_nUsed; ++i )
                m_pChildren[ i - nHowMuchKeep ] = pNode->m_pChildren[ i ];

            // we have to 'relativize' the keys
            offset = pNode->m_pKeys[ nHowMuchKeep - 1 ];
            for ( int i = nHowMuchKeep; i < pNode->m_nUsed - 1; ++i )
                m_pKeys[ i - nHowMuchKeep ] = pNode->m_pKeys[ i ] - offset;
        }
        else
        {
            for ( int i = nHowMuchKeep; i < pNode->m_nUsed; ++i )
                m_pValues[ i - nHowMuchKeep ] = pNode->m_pValues[ i ];

            offset = nHowMuchKeep;
        }

        m_nUsed = pNode->m_nUsed - nHowMuchKeep;
        pNode->m_nUsed = nHowMuchKeep;

        m_pNext = pNode->m_pNext;
        pNode->m_pNext = this;

        return offset;
    }
};

template < class Key, class Value >
DenseBPlusTree< Key, Value >::DenseBPlusTree()
    : m_pRoot( new DBPTreeNode< Key, Value > ),
      m_pLastLeaf( m_pRoot ),
      m_nCount( 0 ),
      m_nDepth( 1 )
{
    assert( sMinFill > 0 ); // just to be sure ;-)
}

template < class Key, class Value >
DenseBPlusTree< Key, Value >::~DenseBPlusTree()
{
    // TODO
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::Insert( const Value& rValue, Key nPos )
{
    NodeWithIndex pParents[ m_nDepth ];
    int nParentsLength = 0;
    NodeWithIndex aLeaf( m_pLastLeaf, m_pLastLeaf->m_nUsed );

    // if we are lucky, we just append, otherwise do the full job
    if ( nPos != m_nCount || m_pLastLeaf->m_nUsed == sOrder )
        aLeaf = findLeaf( nPos, pParents, nParentsLength );

    if ( aLeaf.pNode->m_nUsed < sOrder )
    {
        // there's still space in the current node
        aLeaf.pNode->insert( aLeaf.nIndex, rValue );
        shiftNodes( pParents, nParentsLength, 1 );
    }
    else
    {
        NodeWithIndex pNewParents[ m_nDepth ];
        int nNewParentsLength;
        DBPTreeNode< Key, Value > *pNewLeaf = splitNode( aLeaf.pNode, nPos == m_nCount, pParents, nParentsLength, pNewParents, nNewParentsLength );

        if ( aLeaf.nIndex <= aLeaf.pNode->m_nUsed )
            aLeaf.pNode->insert( aLeaf.nIndex, rValue );
        else
        {
            pNewLeaf->insert( aLeaf.nIndex - aLeaf.pNode->m_nUsed, rValue );
            ++pNewParents[ nNewParentsLength - 1 ].nIndex;
        }

        shiftNodes( pNewParents, nNewParentsLength, 1 );
    }

    ++m_nCount;
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::Remove( Key nPos, Key nNumber )
{
    assert( nNumber > 0 );
    assert( nPos + nNumber <= m_nCount );

    NodeWithIndex pParents[ m_nDepth ];
    int nParentsLength = 0;
    NodeWithIndex aLeaf = findLeaf( nPos, pParents, nParentsLength );

    if ( aLeaf.pNode->m_nUsed - nNumber >= sMinFill )
    {
        aLeaf.pNode->remove( aLeaf.nIndex, nNumber );
        shiftNodes( pParents, nParentsLength, -nNumber );
    }
    else
    {
        // let's find the first node that we are not removing, and use the
        // m_pNext chains to delete everything in between on every level
        NodeWithIndex pAfterParents[ m_nDepth ];
        int nAfterParentsLength = 0;
        NodeWithIndex aAfter = findLeaf( nPos + nNumber, pAfterParents, nAfterParentsLength );

        // we do the operation the same way on every level, regardless it is a
        // leaf, or an internal node
        pParents[ nParentsLength ] = aLeaf;
        pAfterParents[ nAfterParentsLength ] = aAfter;

        // remove it
        assert( nParentsLength == nAfterParentsLength );
        removeBetween( pParents, pAfterParents, nParentsLength + 1 );

        // update indexes
        shiftNodes( pParents, nParentsLength, aAfter.nIndex - nNumber );

        // FIXME we have to create a function that walks up the parents to do
        // this right even in the case nIndex == m_nUsed
        if ( pParents[ nParentsLength - 1 ].nIndex < pParents[ nParentsLength - 1 ].pNode->m_nUsed - 1 )
            ++pParents[ nParentsLength - 1 ].nIndex;
        shiftNodes( pParents, nParentsLength, -aAfter.nIndex );
    }

    m_nCount -= nNumber;
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

    NodeWithIndex aLeaf = findLeaf( nPos );

    aLeaf.pNode->m_pValues[ aLeaf.nIndex ] = rValue;
}

template < class Key, class Value >
const Value& DenseBPlusTree< Key, Value >::operator[]( Key nPos ) const
{
    assert( m_pRoot->m_nUsed > 0 );

    NodeWithIndex aLeaf = findLeaf( nPos );

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
void DenseBPlusTree< Key, Value >::dump() const
{
    printf( "======================\nCount: %d\n", Count() );
    vector< DBPTreeNode< Key, Value >* > aLifo;
    aLifo.push_back( m_pRoot );

    while ( !aLifo.empty() )
    {
        DBPTreeNode< Key, Value > *pNode = aLifo.front();
        aLifo.erase( aLifo.begin() );

        if ( pNode->m_bIsInternal )
        {
            printf( "internal node: %p\nkeys: ", pNode );
            for ( int i = 0; i < pNode->m_nUsed - 1; ++i )
                printf( "%d, ", pNode->m_pKeys[ i ] );

            printf( "\nchildren: " );
            for ( int i = 0; i < pNode->m_nUsed; ++i )
            {
                printf( "%p, ", pNode->m_pChildren[ i ] );
                aLifo.push_back( pNode->m_pChildren[ i ] );
            }
            printf( "\n\n" );
        }
        else
        {
            printf( "leaf node: %p\nvalues: ", pNode );
            for ( int i = 0; i < pNode->m_nUsed; ++i )
                printf( "%d, ", pNode->m_pValues[ i ] );
            printf( "\n\n" );
        }
    }
}

template < class Key, class Value >
typename DenseBPlusTree< Key, Value >::NodeWithIndex DenseBPlusTree< Key, Value >::findLeaf( Key nPos, NodeWithIndex pParents[], int &rParentsLength )
{
    DBPTreeNode< Key, Value > *pNode = m_pRoot;
    rParentsLength = 0;

    // traverse from the root to the leaves
    while ( pNode->m_bIsInternal )
    {
        int i;
        if ( pNode->m_nUsed < 2 || nPos < pNode->m_pKeys[ 0 ] )  // nPos too small, we continue leftmost
            i = 0;
        else if ( pNode->m_pKeys[ pNode->m_nUsed - 2 ] <= nPos ) // nPos is too big, continue rightmost
            i = pNode->m_nUsed - 1;
        else
        {
            // binary search, the values are ordered
            i = 1;
            int max = pNode->m_nUsed - 2;
            while ( i < max )
            {
                int pivot = i + ( max - i ) / 2;
                if ( pNode->m_pKeys[ pivot ] <= nPos )
                    i = pivot + 1;
                else
                    max = pivot;
            }
        }

        // m_pKeys in children are relative
        if ( i > 0 )
            nPos -= pNode->m_pKeys[ i - 1 ];

        if ( pParents )
            pParents[ rParentsLength++ ] = NodeWithIndex( pNode, i );

        pNode = pNode->m_pChildren[ i ];
    }

    return NodeWithIndex( pNode, nPos );
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::shiftNodes( const NodeWithIndex pParents[], int nParentsLength, int nHowMuch )
{
    for ( int p = nParentsLength - 1; p >= 0; --p )
    {
        const NodeWithIndex &rNode = pParents[ p ];
        for ( int i = rNode.nIndex; i < rNode.pNode->m_nUsed - 1; ++i )
            rNode.pNode->m_pKeys[ i ] += nHowMuch;
    }
}

template < class Key, class Value >
DBPTreeNode< Key, Value >* DenseBPlusTree< Key, Value >::splitNode( DBPTreeNode< Key, Value > *pNode, bool bIsAppend, const NodeWithIndex pParents[], int nParentsLength, NodeWithIndex pNewParents[], int &rNewParentsLength )
{
    assert( pNode->m_nUsed == sOrder );

    DBPTreeNode< Key, Value > *pNewNode = new DBPTreeNode< Key, Value >;
    int offset = pNewNode->copyFromSplitNode( pNode, bIsAppend );

    // update the last leaf if necessary
    if ( pNode == m_pLastLeaf )
        m_pLastLeaf = pNewNode;

    if ( nParentsLength == 0 )
    {
        // we have to create a new root
        DBPTreeNode< Key, Value > *pNewRoot = new DBPTreeNode< Key, Value >;
        pNewRoot->m_bIsInternal = true;
        pNewRoot->m_pChildren[ 0 ] = m_pRoot;
        pNewRoot->m_nUsed = 1;

        m_pRoot = pNewRoot;
        ++m_nDepth;

        m_pRoot->insert( 1, offset, pNewNode );

        pNewParents[ 0 ] = NodeWithIndex( m_pRoot, 0 );
        rNewParentsLength = 1;
    }
    else
    {
        NodeWithIndex aParent = pParents[ nParentsLength - 1 ];

        if ( aParent.pNode->m_nUsed < sOrder )
        {
            aParent.pNode->insert( aParent.nIndex + 1, offset, pNewNode );

            memcpy( pNewParents, pParents, sizeof( pParents[ 0 ] ) * ( nParentsLength - 1 ) );
            rNewParentsLength = nParentsLength;
            pNewParents[ rNewParentsLength - 1 ] = aParent;
        }
        else
        {
            DBPTreeNode< Key, Value > *pNewParent = splitNode( aParent.pNode, bIsAppend, pParents, nParentsLength - 1, pNewParents, rNewParentsLength );

            if ( aParent.nIndex <= aParent.pNode->m_nUsed )
            {
                aParent.pNode->insert( aParent.nIndex + 1, offset, pNewNode );
                pNewParents[ rNewParentsLength++ ] = aParent;
            }
            else
            {
                pNewParent->insert( aParent.nIndex - aParent.pNode->m_nUsed + 1, offset, pNewNode );
                pNewParents[ rNewParentsLength++ ] = NodeWithIndex( pNewParent, aParent.nIndex - aParent.pNode->m_nUsed + 1 );
            }
        }
    }

    return pNewNode;
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::removeBetween( const NodeWithIndex pFrom[], const NodeWithIndex pTo[], int nLength )
{
    for ( int p = 0; p < nLength; ++p )
    {
        const NodeWithIndex &rLeaf = pFrom[ p ];
        const NodeWithIndex &rAfter = pTo[ p ];

        if ( rLeaf.pNode == rAfter.pNode )
        {
            // we need to keep parents of the 'from' branch too
            if ( rLeaf.pNode->m_bIsInternal )
            {
                if ( rAfter.nIndex - rLeaf.nIndex - 1 > 0 )
                    rLeaf.pNode->remove( rLeaf.nIndex + 1, rAfter.nIndex - rLeaf.nIndex - 1 );
            }
            else
                rLeaf.pNode->remove( rLeaf.nIndex, rAfter.nIndex - rLeaf.nIndex );
        }
        else
        {
            // remove rest of the content of the node where the deletion starts
            rLeaf.pNode->remove( rLeaf.nIndex, rLeaf.pNode->m_nUsed - rLeaf.nIndex );

            // delete all nodes between from and to on the given level
            for ( DBPTreeNode< Key, Value > *pNode = rLeaf.pNode->m_pNext; pNode != rAfter.pNode; )
            {
                DBPTreeNode< Key, Value > *pToDelete = pNode;
                pNode = pNode->m_pNext;
                delete pToDelete;
            }

            // remove the remaining data in the node after the deleted range
            if ( rAfter.nIndex > 0 )
                rAfter.pNode->remove( 0, rAfter.nIndex );

            // reconnect
            rLeaf.pNode->m_pNext = rAfter.pNode;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
