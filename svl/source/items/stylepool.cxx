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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <vector>
#include <map>

#include <svl/stylepool.hxx>
#include <svl/itemiter.hxx>
#include <svl/itempool.hxx>


using namespace boost;

namespace {
    // A "Node" represents a subset of inserted SfxItemSets
    // The root node represents the empty set
    // The other nodes contain a SfxPoolItem and represents an item set which contains their
    // pool item and the pool items of their parents.
    class Node
    {
        std::vector<Node*> mChildren; // child nodes, create by findChildNode(..)
        // container of shared pointers of inserted item sets; for non-poolable
        // items more than one item set is needed
        std::vector< StylePool::SfxItemSet_Pointer_t > maItemSet;
        const SfxPoolItem *mpItem;   // my pool item
        Node *mpUpper;               // if I'm a child node that's my parent node
        // #i86923#
        const bool mbIsItemIgnorable;
    public:
        // #i86923#
        Node() // root node Ctor
            : mChildren(),
              maItemSet(),
              mpItem( 0 ),
              mpUpper( 0 ),
              mbIsItemIgnorable( false )
        {}
        Node( const SfxPoolItem& rItem, Node* pParent, const bool bIgnorable ) // child node Ctor
            : mChildren(),
              maItemSet(),
              mpItem( rItem.Clone() ),
              mpUpper( pParent ),
              mbIsItemIgnorable( bIgnorable )
        {}
        ~Node();
        // #i86923#
        bool hasItemSet( const bool bCheckUsage ) const;
        // #i87808#
        const StylePool::SfxItemSet_Pointer_t getItemSet() const
        {
            return maItemSet.back();
        }
        const StylePool::SfxItemSet_Pointer_t getUsedOrLastAddedItemSet() const;
        void setItemSet( const SfxItemSet& rSet ){ maItemSet.push_back( StylePool::SfxItemSet_Pointer_t( rSet.Clone() ) ); }
        // #i86923#
        Node* findChildNode( const SfxPoolItem& rItem,
                             const bool bIsItemIgnorable = false );
        Node* nextItemSet( Node* pLast,
                           const bool bSkipUnusedItemSet,
                           const bool bSkipIgnorable );
        const SfxPoolItem& getPoolItem() const { return *mpItem; }
        // #i86923#
        bool hasIgnorableChildren( const bool bCheckUsage ) const;
        const StylePool::SfxItemSet_Pointer_t getItemSetOfIgnorableChild(
                                        const bool bSkipUnusedItemSets ) const;
    };

    // #i87808#
    const StylePool::SfxItemSet_Pointer_t Node::getUsedOrLastAddedItemSet() const
    {
        std::vector< StylePool::SfxItemSet_Pointer_t >::const_reverse_iterator aIter;

        for ( aIter = maItemSet.rbegin(); aIter != maItemSet.rend(); ++aIter )
        {
            if ( (*aIter).use_count() > 1 )
            {
                return *aIter;
            }
        }

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
                std::vector< StylePool::SfxItemSet_Pointer_t >::const_reverse_iterator aIter;

                for ( aIter = maItemSet.rbegin(); aIter != maItemSet.rend(); ++aIter )
                {
                    if ( (*aIter).use_count() > 1 )
                    {
                        bHasItemSet = true;
                        break;
                    }
                }
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
        Node* pNextNode = this;
        std::vector<Node*>::iterator aIter = mChildren.begin();
        while( aIter != mChildren.end() )
        {
            if( rItem.Which() == (*aIter)->getPoolItem().Which() &&
                rItem == (*aIter)->getPoolItem() )
                return *aIter;
            ++aIter;
        }
        // #i86923#
        pNextNode = new Node( rItem, pNextNode, bIsItemIgnorable );
        mChildren.push_back( pNextNode );
        return pNextNode;
    }

    /* Find the next node which has a SfxItemSet.
       The input parameter pLast has a sophisticated meaning:
       downstairs only:
       pLast == 0 => scan your children and their children
                     but neither your parents neither your siblings
       downstairs and upstairs:
       pLast == this => scan your children, their children,
                        the children of your parent behind you, and so on
       partial downstairs and upstairs
       pLast != 0 && pLast != this => scan your children behind the given children,
                        the children of your parent behind you and so on.

       OD 2008-03-11 #i86923#
       introduce parameters <bSkipUnusedItemSets> and <bSkipIgnorable>
       and its handling.
    */
    Node* Node::nextItemSet( Node* pLast,
                             const bool bSkipUnusedItemSets,
                             const bool bSkipIgnorable )
    {
        // Searching downstairs
        std::vector<Node*>::iterator aIter = mChildren.begin();
        // For pLast == 0 and pLast == this all children are of interest
        // for another pLast the search starts behind pLast...
        if( pLast && pLast != this )
        {
            aIter = std::find( mChildren.begin(), mChildren.end(), pLast );
            if( aIter != mChildren.end() )
                ++aIter;
        }
        Node *pNext = 0;
        while( aIter != mChildren.end() )
        {
            // #i86923#
            if ( bSkipIgnorable && (*aIter)->mbIsItemIgnorable )
            {
                ++aIter;
                continue;
            }
            pNext = *aIter;
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
            pNext = pNext->nextItemSet( 0, bSkipUnusedItemSets, bSkipIgnorable ); // 0 => downstairs only
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
        bool bHasIgnorableChildren( false );

        std::vector<Node*>::const_iterator aIter = mChildren.begin();
        while( aIter != mChildren.end() && !bHasIgnorableChildren )
        {
            Node* pChild = *aIter;
            if ( pChild->mbIsItemIgnorable )
            {
                bHasIgnorableChildren =
                    !bCheckUsage ||
                    ( pChild->hasItemSet( bCheckUsage /* == true */ ) ||
                      pChild->hasIgnorableChildren( bCheckUsage /* == true */ ) );
            }
            ++aIter;
        }

        return bHasIgnorableChildren;
    }

    const StylePool::SfxItemSet_Pointer_t Node::getItemSetOfIgnorableChild(
                                        const bool bSkipUnusedItemSets ) const
    {
        DBG_ASSERT( hasIgnorableChildren( bSkipUnusedItemSets ),
                    "<Node::getItemSetOfIgnorableChild> - node has no ignorable children" );

        std::vector<Node*>::const_iterator aIter = mChildren.begin();
        while( aIter != mChildren.end() )
        {
            Node* pChild = *aIter;
            if ( pChild->mbIsItemIgnorable )
            {
                if ( pChild->hasItemSet( bSkipUnusedItemSets ) )
                {
                    return pChild->getUsedOrLastAddedItemSet();
                }
                else
                {
                    pChild = pChild->nextItemSet( 0, bSkipUnusedItemSets, false );
                    if ( pChild )
                    {
                        return pChild->getUsedOrLastAddedItemSet();
                    }
                }
            }
            ++aIter;
        }

        StylePool::SfxItemSet_Pointer_t pReturn;
        return pReturn;
    }

    Node::~Node()
    {
        std::vector<Node*>::iterator aIter = mChildren.begin();
        while( aIter != mChildren.end() )
        {
            delete *aIter;
            ++aIter;
        }
        delete mpItem;
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
              mpNode(0),
              mbSkipUnusedItemSets( bSkipUnusedItemSets ),
              mbSkipIgnorable( bSkipIgnorable )
        {}
        virtual StylePool::SfxItemSet_Pointer_t getNext();
        virtual OUString getName();
    };

    StylePool::SfxItemSet_Pointer_t Iterator::getNext()
    {
        StylePool::SfxItemSet_Pointer_t pReturn;
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

    OUString Iterator::getName()
    {
        OUString aString;
        if( mpNode && mpNode->hasItemSet( false ) )
        {
            aString = StylePool::nameOf( mpNode->getUsedOrLastAddedItemSet() );
        }
        return aString;
    }

}

/* This static method creates a unique name from a shared pointer to a SfxItemSet
   The name is the memory address of the SfxItemSet itself. */

OUString StylePool::nameOf( SfxItemSet_Pointer_t pSet )
{
    return OUString::number( reinterpret_cast<sal_IntPtr>( pSet.get() ), 16 );
}

// class StylePoolImpl organized a tree-structure where every node represents a SfxItemSet.
// The insertItemSet method adds a SfxItemSet into the tree if necessary and returns a shared_ptr
// to a copy of the SfxItemSet.
// The aRoot-Node represents an empty SfxItemSet.

class StylePoolImpl
{
private:
    std::map< const SfxItemSet*, Node > maRoot;
    sal_Int32 mnCount;
    // #i86923#
    SfxItemSet* mpIgnorableItems;
public:
    // #i86923#
    explicit StylePoolImpl( SfxItemSet* pIgnorableItems = 0 )
        : maRoot(),
          mnCount(0),
          mpIgnorableItems( pIgnorableItems != 0
                            ? pIgnorableItems->Clone( sal_False )
                            : 0 )
    {
        DBG_ASSERT( !pIgnorableItems || !pIgnorableItems->Count(),
                    "<StylePoolImpl::StylePoolImpl(..)> - misusage: item set for ignorable item should be empty. Please correct usage." );
        DBG_ASSERT( !mpIgnorableItems || !mpIgnorableItems->Count(),
                    "<StylePoolImpl::StylePoolImpl(..)> - <SfxItemSet::Clone( sal_False )> does not work as excepted - <mpIgnorableItems> is not empty. Please inform OD." );
    }

    ~StylePoolImpl()
    {
        delete mpIgnorableItems;
    }

    StylePool::SfxItemSet_Pointer_t insertItemSet( const SfxItemSet& rSet );

    // #i86923#
    IStylePoolIteratorAccess* createIterator( bool bSkipUnusedItemSets = false,
                                              bool bSkipIgnorableItems = false );
    sal_Int32 getCount() const { return mnCount; }
};

StylePool::SfxItemSet_Pointer_t StylePoolImpl::insertItemSet( const SfxItemSet& rSet )
{
    bool bNonPoolable = false;
    Node* pCurNode = &maRoot[ rSet.GetParent() ];
    SfxItemIter aIter( rSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    // Every SfxPoolItem in the SfxItemSet causes a step deeper into the tree,
    // a complete empty SfxItemSet would stay at the root node.
    // #i86923# insert ignorable items to the tree leaves.
    std::auto_ptr<SfxItemSet> pFoundIgnorableItems;
    if ( mpIgnorableItems )
    {
        pFoundIgnorableItems.reset( new SfxItemSet( *mpIgnorableItems ) );
    }
    while( pItem )
    {
        if( !rSet.GetPool()->IsItemFlag(pItem->Which(), SFX_ITEM_POOLABLE ) )
            bNonPoolable = true;
        if ( !pFoundIgnorableItems.get() ||
             ( pFoundIgnorableItems.get() &&
               pFoundIgnorableItems->Put( *pItem ) == 0 ) )
        {
            pCurNode = pCurNode->findChildNode( *pItem );
        }
        pItem = aIter.NextItem();
    }
    if ( pFoundIgnorableItems.get() &&
         pFoundIgnorableItems->Count() > 0 )
    {
        SfxItemIter aIgnorableItemsIter( *pFoundIgnorableItems );
        pItem = aIgnorableItemsIter.GetCurItem();
        while( pItem )
        {
            if( !rSet.GetPool()->IsItemFlag(pItem->Which(), SFX_ITEM_POOLABLE ) )
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
        ++mnCount;
    }
    // If rSet contains at least one non poolable item, a new itemset has to be inserted
    if( bNonPoolable )
        pCurNode->setItemSet( rSet );
#ifdef DEBUG
    {
        sal_Int32 nCheck = -1;
        IStylePoolIteratorAccess* pIter = createIterator();
        StylePool::SfxItemSet_Pointer_t pTemp;
        do
        {
            ++nCheck;
            pTemp = pIter->getNext();
        } while( pTemp.get() );
        DBG_ASSERT( mnCount == nCheck, "Wrong counting");
        delete pIter;
    }
#endif
    return pCurNode->getItemSet();
}

// #i86923#
IStylePoolIteratorAccess* StylePoolImpl::createIterator( bool bSkipUnusedItemSets,
                                                         bool bSkipIgnorableItems )
{
    return new Iterator( maRoot, bSkipUnusedItemSets, bSkipIgnorableItems );
}
// Ctor, Dtor and redirected methods of class StylePool, nearly inline ;-)

// #i86923#
StylePool::StylePool( SfxItemSet* pIgnorableItems )
    : pImpl( new StylePoolImpl( pIgnorableItems ) )
{}

StylePool::SfxItemSet_Pointer_t StylePool::insertItemSet( const SfxItemSet& rSet )
{ return pImpl->insertItemSet( rSet ); }

// #i86923#
IStylePoolIteratorAccess* StylePool::createIterator( const bool bSkipUnusedItemSets,
                                                     const bool bSkipIgnorableItems )
{
    return pImpl->createIterator( bSkipUnusedItemSets, bSkipIgnorableItems );
}

sal_Int32 StylePool::getCount() const
{ return pImpl->getCount(); }

StylePool::~StylePool() { delete pImpl; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
