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

#include <svl/stylepool.hxx>
#include <svl/itemiter.hxx>
#include <svl/itempool.hxx>
#include <tools/debug.hxx>
#include <algorithm>
#include <map>
#include <memory>
#include <vector>

namespace {
    /** A "Node" represents a subset of inserted SfxItemSets
     * The root node represents the empty set
     * The other nodes contain a SfxPoolItem and represents an item set which contains their
     * pool item and the pool items of their parents.
     */
    class Node
    {
        std::vector<std::unique_ptr<Node>> mChildren; // child nodes, create by findChildNode(..)
        // container of shared pointers of inserted item sets; for non-poolable
        // items more than one item set is needed
        std::vector< std::shared_ptr<SfxItemSet> > maItemSet;
        std::unique_ptr<const SfxPoolItem> mpItem;   // my pool item
        Node *mpUpper;               // if I'm a child node that's my parent node
        // #i86923#
        const bool mbIsItemIgnorable;
    public:
        // #i86923#
        Node() // root node Ctor
            : mChildren(),
              maItemSet(),
              mpUpper( nullptr ),
              mbIsItemIgnorable( false )
        {}
        Node( const SfxPoolItem& rItem, Node* pParent, const bool bIgnorable ) // child node Ctor
            : mChildren(),
              maItemSet(),
              mpItem( rItem.Clone() ),
              mpUpper( pParent ),
              mbIsItemIgnorable( bIgnorable )
        {}
        // #i86923#
        bool hasItemSet( const bool bCheckUsage ) const;
        // #i87808#
        std::shared_ptr<SfxItemSet> const & getItemSet() const
        {
            return maItemSet.back();
        }
        std::shared_ptr<SfxItemSet> const & getUsedOrLastAddedItemSet() const;
        void setItemSet( const SfxItemSet& rSet ){ maItemSet.push_back( std::shared_ptr<SfxItemSet>( rSet.Clone() ) ); }
        // #i86923#
        Node* findChildNode( const SfxPoolItem& rItem,
                             const bool bIsItemIgnorable );
        Node* nextItemSet( Node const * pLast,
                           const bool bSkipUnusedItemSet,
                           const bool bSkipIgnorable );
        // #i86923#
        bool hasIgnorableChildren( const bool bCheckUsage ) const;
        const std::shared_ptr<SfxItemSet> getItemSetOfIgnorableChild(
                                        const bool bSkipUnusedItemSets ) const;
    };

    // #i87808#
    std::shared_ptr<SfxItemSet> const & Node::getUsedOrLastAddedItemSet() const
    {
        auto aIter = std::find_if(maItemSet.rbegin(), maItemSet.rend(),
            [](const std::shared_ptr<SfxItemSet>& rxItemSet) { return rxItemSet.use_count() > 1; });

        if (aIter != maItemSet.rend())
            return *aIter;

        return maItemSet.back();
    }

    // #i86923#
    bool Node::hasItemSet( const bool bCheckUsage ) const
    {
        bool bHasItemSet = false;

        if ( !maItemSet.empty())
        {
            if ( bCheckUsage )
            {
                bHasItemSet = std::any_of(maItemSet.rbegin(), maItemSet.rend(),
                    [](const std::shared_ptr<SfxItemSet>& rxItemSet) { return rxItemSet.use_count() > 1; });
            }
            else
            {
                bHasItemSet = true;
            }
        }
        return bHasItemSet;
    }

    // #i86923#
    Node* Node::findChildNode( const SfxPoolItem& rItem,
                               const bool bIsItemIgnorable )
    {
        for( auto const & rChild : mChildren )
        {
            if( rItem.Which() == rChild->mpItem->Which() &&
                rItem == *rChild->mpItem )
                return rChild.get();
        }
        // #i86923#
        auto pNextNode = new Node( rItem, this, bIsItemIgnorable );
        mChildren.emplace_back( pNextNode );
        return pNextNode;
    }

    /**
     * Find the next node which has a SfxItemSet.
     * The input parameter pLast has a sophisticated meaning:
     * downstairs only:
     * pLast == 0 => scan your children and their children
     *               but neither your parents neither your siblings
     * downstairs and upstairs:
     * pLast == this => scan your children, their children,
     *                  the children of your parent behind you, and so on
     * partial downstairs and upstairs
     *  pLast != 0 && pLast != this => scan your children behind the given children,
     *                 the children of your parent behind you and so on.
     *
     * OD 2008-03-11 #i86923#
     * introduce parameters <bSkipUnusedItemSets> and <bSkipIgnorable>
     * and its handling.
     */
    Node* Node::nextItemSet( Node const * pLast,
                             const bool bSkipUnusedItemSets,
                             const bool bSkipIgnorable )
    {
        // Searching downstairs
        auto aIter = mChildren.begin();
        // For pLast == 0 and pLast == this all children are of interest
        // for another pLast the search starts behind pLast...
        if( pLast && pLast != this )
        {
            aIter = std::find_if( mChildren.begin(), mChildren.end(),
                                  [&] (std::unique_ptr<Node> const &p) { return p.get() == pLast; });
            if( aIter != mChildren.end() )
                ++aIter;
        }
        Node *pNext = nullptr;
        while( aIter != mChildren.end() )
        {
            // #i86923#
            if ( bSkipIgnorable && (*aIter)->mbIsItemIgnorable )
            {
                ++aIter;
                continue;
            }
            pNext = aIter->get();
            // #i86923#
            if ( pNext->hasItemSet( bSkipUnusedItemSets ) )
            {
                return pNext;
            }
            if ( bSkipIgnorable &&
                 pNext->hasIgnorableChildren( bSkipUnusedItemSets ) )
            {
                return pNext;
            }
            pNext = pNext->nextItemSet( nullptr, bSkipUnusedItemSets, bSkipIgnorable ); // 0 => downstairs only
            if( pNext )
                return pNext;
            ++aIter;
        }
        // Searching upstairs
        if( pLast && mpUpper )
        {
            // #i86923#
            pNext = mpUpper->nextItemSet( this, bSkipUnusedItemSets, bSkipIgnorable );
        }
        return pNext;
    }

    // #i86923#
    bool Node::hasIgnorableChildren( const bool bCheckUsage ) const
    {
        return std::any_of(mChildren.begin(), mChildren.end(),
            [&bCheckUsage](const std::unique_ptr<Node>& rxChild) {
                Node* pChild = rxChild.get();
                return pChild->mbIsItemIgnorable &&
                    (!bCheckUsage ||
                     ( pChild->hasItemSet( bCheckUsage /* == true */ ) ||
                       pChild->hasIgnorableChildren( bCheckUsage /* == true */ ) ));
            });
    }

    const std::shared_ptr<SfxItemSet> Node::getItemSetOfIgnorableChild(
                                        const bool bSkipUnusedItemSets ) const
    {
        DBG_ASSERT( hasIgnorableChildren( bSkipUnusedItemSets ),
                    "<Node::getItemSetOfIgnorableChild> - node has no ignorable children" );

        for( const auto& rxChild : mChildren )
        {
            Node* pChild = rxChild.get();
            if ( pChild->mbIsItemIgnorable )
            {
                if ( pChild->hasItemSet( bSkipUnusedItemSets ) )
                {
                    return pChild->getUsedOrLastAddedItemSet();
                }
                else
                {
                    pChild = pChild->nextItemSet( nullptr, bSkipUnusedItemSets, false );
                    if ( pChild )
                    {
                        return pChild->getUsedOrLastAddedItemSet();
                    }
                }
            }
        }

        std::shared_ptr<SfxItemSet> pReturn;
        return pReturn;
    }

    class Iterator : public IStylePoolIteratorAccess
    {
        std::map< const SfxItemSet*, Node >& mrRoot;
        std::map< const SfxItemSet*, Node >::iterator mpCurrNode;
        Node* mpNode;
        const bool mbSkipUnusedItemSets;
        const bool mbSkipIgnorable;
    public:
        // #i86923#
        Iterator( std::map< const SfxItemSet*, Node >& rR,
                  const bool bSkipUnusedItemSets,
                  const bool bSkipIgnorable )
            : mrRoot( rR ),
              mpCurrNode( rR.begin() ),
              mpNode(nullptr),
              mbSkipUnusedItemSets( bSkipUnusedItemSets ),
              mbSkipIgnorable( bSkipIgnorable )
        {}
        virtual std::shared_ptr<SfxItemSet> getNext() override;
    };

    std::shared_ptr<SfxItemSet> Iterator::getNext()
    {
        std::shared_ptr<SfxItemSet> pReturn;
        while( mpNode || mpCurrNode != mrRoot.end() )
        {
            if( !mpNode )
            {
                mpNode = &mpCurrNode->second;
                ++mpCurrNode;
                // #i86923#
                if ( mpNode->hasItemSet( mbSkipUnusedItemSets ) )
                {
                    // #i87808#
                    return mpNode->getUsedOrLastAddedItemSet();
                }
            }
            // #i86923#
            mpNode = mpNode->nextItemSet( mpNode, mbSkipUnusedItemSets, mbSkipIgnorable );
            if ( mpNode && mpNode->hasItemSet( mbSkipUnusedItemSets ) )
            {
                // #i87808#
                return mpNode->getUsedOrLastAddedItemSet();
            }
            if ( mbSkipIgnorable &&
                 mpNode && mpNode->hasIgnorableChildren( mbSkipUnusedItemSets ) )
            {
                return mpNode->getItemSetOfIgnorableChild( mbSkipUnusedItemSets );
            }
        }
        return pReturn;
    }

}

/**
 * This static method creates a unique name from a shared pointer to a SfxItemSet
 * The name is the memory address of the SfxItemSet itself.
 */
OUString StylePool::nameOf( const std::shared_ptr<SfxItemSet>& pSet )
{
    return OUString::number( reinterpret_cast<sal_IntPtr>( pSet.get() ), 16 );
}

/**
 * class StylePoolImpl organized a tree-structure where every node represents a SfxItemSet.
 * The insertItemSet method adds a SfxItemSet into the tree if necessary and returns a shared_ptr
 * to a copy of the SfxItemSet.
 * The aRoot-Node represents an empty SfxItemSet.
 */
class StylePoolImpl
{
private:
    std::map< const SfxItemSet*, Node > maRoot;
    // #i86923#
    std::unique_ptr<SfxItemSet> mpIgnorableItems;
#ifdef DEBUG
    sal_Int32 mnCount;
#endif
public:
    // #i86923#
    explicit StylePoolImpl( SfxItemSet const * pIgnorableItems )
        : maRoot(),
#ifdef DEBUG
          mnCount(0),
#endif
          mpIgnorableItems( pIgnorableItems != nullptr
                            ? pIgnorableItems->Clone( false )
                            : nullptr )
    {
        DBG_ASSERT( !pIgnorableItems || !pIgnorableItems->Count(),
                    "<StylePoolImpl::StylePoolImpl(..)> - misusage: item set for ignorable item should be empty. Please correct usage." );
        DBG_ASSERT( !mpIgnorableItems || !mpIgnorableItems->Count(),
                    "<StylePoolImpl::StylePoolImpl(..)> - <SfxItemSet::Clone( sal_False )> does not work as excepted - <mpIgnorableItems> is not empty." );
    }

    std::shared_ptr<SfxItemSet> insertItemSet( const SfxItemSet& rSet );

    // #i86923#
    std::unique_ptr<IStylePoolIteratorAccess> createIterator( bool bSkipUnusedItemSets,
                                              bool bSkipIgnorableItems );
};


std::shared_ptr<SfxItemSet> StylePoolImpl::insertItemSet( const SfxItemSet& rSet )
{
    bool bNonPoolable = false;
    Node* pCurNode = &maRoot[ rSet.GetParent() ];
    SfxItemIter aIter( rSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    // Every SfxPoolItem in the SfxItemSet causes a step deeper into the tree,
    // a complete empty SfxItemSet would stay at the root node.
    // #i86923# insert ignorable items to the tree leaves.
    std::unique_ptr<SfxItemSet> xFoundIgnorableItems;
    if ( mpIgnorableItems )
    {
        xFoundIgnorableItems.reset( new SfxItemSet( *mpIgnorableItems ) );
    }
    while( pItem )
    {
        if( !rSet.GetPool()->IsItemPoolable(pItem->Which() ) )
            bNonPoolable = true;
        if (!xFoundIgnorableItems || (xFoundIgnorableItems->Put(*pItem) == nullptr))
        {
            pCurNode = pCurNode->findChildNode( *pItem, false );
        }
        pItem = aIter.NextItem();
    }
    if ( xFoundIgnorableItems.get() &&
         xFoundIgnorableItems->Count() > 0 )
    {
        SfxItemIter aIgnorableItemsIter( *xFoundIgnorableItems );
        pItem = aIgnorableItemsIter.GetCurItem();
        while( pItem )
        {
            if( !rSet.GetPool()->IsItemPoolable(pItem->Which() ) )
                bNonPoolable = true;
            pCurNode = pCurNode->findChildNode( *pItem, true );
            pItem = aIgnorableItemsIter.NextItem();
        }
    }
    // Every leaf node represents an inserted item set, but "non-leaf" nodes represents subsets
    // of inserted itemsets.
    // These nodes could have but does not need to have a shared_ptr to a item set.
    if( !pCurNode->hasItemSet( false ) )
    {
        pCurNode->setItemSet( rSet );
        bNonPoolable = false; // to avoid a double insertion
#ifdef DEBUG
        ++mnCount;
#endif
    }
    // If rSet contains at least one non poolable item, a new itemset has to be inserted
    if( bNonPoolable )
        pCurNode->setItemSet( rSet );
#ifdef DEBUG
    {
        sal_Int32 nCheck = -1;
        std::unique_ptr<IStylePoolIteratorAccess> pIter = createIterator(false,false);
        std::shared_ptr<SfxItemSet> pTemp;
        do
        {
            ++nCheck;
            pTemp = pIter->getNext();
        } while( pTemp.get() );
        DBG_ASSERT( mnCount == nCheck, "Wrong counting");
    }
#endif
    return pCurNode->getItemSet();
}

// #i86923#
std::unique_ptr<IStylePoolIteratorAccess> StylePoolImpl::createIterator( bool bSkipUnusedItemSets,
                                                         bool bSkipIgnorableItems )
{
    return std::make_unique<Iterator>( maRoot, bSkipUnusedItemSets, bSkipIgnorableItems );
}
// Ctor, Dtor and redirected methods of class StylePool, nearly inline ;-)

// #i86923#
StylePool::StylePool( SfxItemSet const * pIgnorableItems )
    : pImpl( new StylePoolImpl( pIgnorableItems ) )
{}

std::shared_ptr<SfxItemSet> StylePool::insertItemSet( const SfxItemSet& rSet )
{ return pImpl->insertItemSet( rSet ); }

// #i86923#
std::unique_ptr<IStylePoolIteratorAccess> StylePool::createIterator( const bool bSkipUnusedItemSets,
                                                     const bool bSkipIgnorableItems )
{
    return pImpl->createIterator( bSkipUnusedItemSets, bSkipIgnorableItems );
}

StylePool::~StylePool()
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
