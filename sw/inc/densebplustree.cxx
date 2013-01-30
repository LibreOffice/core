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
#include <vector>

using namespace std;

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

    /** Split node, and make the original one smaller.

        @return relative key shift of the node.
    */
    int copyFromSplitNode( DBPTreeNode *pNode )
    {
        assert( pNode->m_nUsed == sOrder );

        const int offset = sOrder / 2;
        int nKeyShift = 0;

        m_bIsInternal = pNode->m_bIsInternal;
        if ( m_bIsInternal )
        {
            for ( int i = 0; i < sOrder - offset; ++i )
                m_pChildren[ i ] = pNode->m_pChildren[ i + offset ];

            // we have to 'relativize' the keys
            nKeyShift = pNode->m_pKeys[ offset - 1 ];
            for ( int i = 0; i < sOrder - offset - 1; ++i )
                m_pKeys[ i ] = pNode->m_pKeys[ i + offset ] - nKeyShift;
        }
        else
        {
            for ( int i = 0; i < sOrder - offset; ++i )
                m_pValues[ i ] = pNode->m_pValues[ i + offset ];

            nKeyShift = offset;

            m_pNext = pNode->m_pNext;
            pNode->m_pNext = this;
        }

        m_nUsed = sOrder - offset;
        pNode->m_nUsed = offset;

        return nKeyShift;
    }
};

template < class Key, class Value >
DenseBPlusTree< Key, Value >::DenseBPlusTree()
    : m_pRoot( new DBPTreeNode< Key, Value > ),
      m_nCount( 0 )
{
}

template < class Key, class Value >
DenseBPlusTree< Key, Value >::~DenseBPlusTree()
{
    // TODO
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::Insert( const Value& rValue, Key nPos )
{
    stack< NodeWithIndex > aParents;
    NodeWithIndex aLeaf = findLeaf( nPos, &aParents );

    if ( aLeaf.pNode->m_nUsed < sOrder - 1 )
    {
        // there's still space in the current node
        aLeaf.pNode->insert( aLeaf.nIndex, rValue );
        shiftNodes( aParents, 1 );
    }
    else
    {
        stack< NodeWithIndex > aNewParents;
        DBPTreeNode< Key, Value > *pNewLeaf = splitNode( aLeaf.pNode, aParents, aNewParents );

        if ( aLeaf.nIndex < aLeaf.pNode->m_nUsed )
        {
            aLeaf.pNode->insert( aLeaf.nIndex, rValue );
            shiftNodes( aParents, 1 );
        }
        else
        {
            pNewLeaf->insert( aLeaf.nIndex - pNewLeaf->m_nUsed, rValue );
            shiftNodes( aNewParents, 1 );
        }
    }

    ++m_nCount;
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

    NodeWithIndex aLeaf = findLeaf( nPos, NULL );

    aLeaf.pNode->m_pValues[ aLeaf.nIndex ] = rValue;
}

template < class Key, class Value >
const Value& DenseBPlusTree< Key, Value >::operator[]( Key nPos ) const
{
    assert( m_pRoot->m_nUsed > 0 );

    NodeWithIndex aLeaf = findLeaf( nPos, NULL );

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
    vector< DBPTreeNode< Key, Value >* > aLifo;
    aLifo.push_back( m_pRoot );

    while ( !aLifo.empty() )
    {
        DBPTreeNode< Key, Value > *pNode = aLifo.front();
        aLifo.erase( aLifo.begin() );

        if ( pNode->m_bIsInternal )
        {
            printf( "internal node: %p\nchildren: ", pNode );
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
typename DenseBPlusTree< Key, Value >::NodeWithIndex DenseBPlusTree< Key, Value >::findLeaf( Key nPos, std::stack< NodeWithIndex > *pParents )
{
    DBPTreeNode< Key, Value > *pNode = m_pRoot;

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
            pParents->push( NodeWithIndex( pNode, i ) );

        pNode = pNode->m_pChildren[ i ];
    }

    return NodeWithIndex( pNode, nPos );
}

template < class Key, class Value >
void DenseBPlusTree< Key, Value >::shiftNodes( const std::stack< NodeWithIndex >& rParents, int nHowMuch )
{
    stack< NodeWithIndex > aParents( rParents );
    while ( !aParents.empty() )
    {
        NodeWithIndex aNode = aParents.top();
        aParents.pop();

        for ( int i = aNode.nIndex; i < aNode.pNode->m_nUsed - 1; ++i )
            aNode.pNode->m_pKeys[ i ] += nHowMuch;
    }
}

template < class Key, class Value >
DBPTreeNode< Key, Value >* DenseBPlusTree< Key, Value >::splitNode( DBPTreeNode< Key, Value > *pNode, const std::stack< NodeWithIndex > &rParents, std::stack< NodeWithIndex > &rNewParents )
{
#if 0
    stack< NodeWithIndex > aParents( rParents );

    struct Split {
        NodeWithIndex aOriginal;
        DBPTreeNode< Key, Value >* pNewNode;
        int nOffset;

        Split( NodeWithIndex &rOrig, DBPTreeNode< Key, Value > *pNew, int nOff ) : aOriginal( rOrig ), pNewNode( pNew ), nOffset( nOff ) {}
    };

    stack< Split > aSplit;

    while ( pNode->m_nUsed == sOrder )
    {
        DBPTreeNode< Key, Value > *pNewNode = new DBPTreeNode< Key, Value >;

        int offset = pNewNode->copyFromSplitNode( pNode );

        NodeWithIndex aNode = aParents.top();
        aParents.pop();

        aSplit.push( Split( aNode, pNewNode, offset ) );

        pNode = aNode.pNode;
    }

    // copy the common (not split) part of parents
    rNewParents = aParents;

    // create new root if we have split even that
    if ( rNewParents.empty() )
    {
        DBPTreeNode< Key, Value > *pNewNode = new DBPTreeNode< Key, Value >;
        pNewNode->m_bIsInternal = true;
        pNewNode->m_pChildren[ 0 ] = m_pRoot;
        pNewNode->m_nUsed = 1;

        m_pRoot = pNewNode;
        rNewParents.push( NodeWithIndex( m_pRoot, 1 ) );
    }

    DBPTreeNode< Key, Value > *pNewNode;
    while ( !aSplit.empty() )
    {
        Split aEntry = aSplit.top();
        pNewNode = aEntry.pNewNode;
        aSplit.pop();

        // insert the new node to the parent (there is enough space now)
        rNewParents.top().pNode->insert( rNewParents.top().nIndex, aEntry.nOffset, aSplit.pNewNode );

        if ( aEntry.aOriginal.nIndex < aEntry.aOriginal.pNode->m_nUsed )
            rNewParents.push( aEntry.aOriginal );
        else
            rNewParents.push( NodeWithIndex( pNewNode, aEntry.aOriginal.nIndex - aEntry.nOffset ) );
    }

    return pNewNode;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
