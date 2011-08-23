/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <vector>
#include <map>

#include "stylepool.hxx"
#include <bf_svtools/itemiter.hxx>
#include <bf_svtools/itempool.hxx>


using namespace boost;

namespace binfilter
{
    // A "Node" represents a subset of inserted SfxItemSets
    // The root node represents the empty set
    // The other nodes contain a SfxPoolItem and represents an item set which contains their
    // pool item and the pool items of their parents.
    class Node
    {
        std::vector<Node*> mChildren; // child nodes, create by findChildNode(..)
        std::vector< StylePool::SfxItemSet_Pointer_t > aItemSet; // shared pointer an inserted item set or nul
        const SfxPoolItem *pItem;   // my pool item
        Node *pUpper;               // if I'm a child node that's my parent node
    public:
        Node() : pItem( 0 ), pUpper( 0 ) {} // root node Ctor
        Node( const SfxPoolItem& rItem, Node* pParent ) : // child node Ctor
            pItem( rItem.Clone() ), pUpper( pParent ){}
        ~Node();
        bool hasItemSet() const { return 0 < aItemSet.size(); }
        const StylePool::SfxItemSet_Pointer_t getItemSet() const { return aItemSet[aItemSet.size()-1]; }
        void setItemSet( const SfxItemSet& rSet ){ aItemSet.push_back( StylePool::SfxItemSet_Pointer_t( rSet.Clone() ) ); }
        Node* findChildNode( const SfxPoolItem& rItem );
        Node* nextItemSet( Node* pLast );
        const SfxPoolItem& getPoolItem() const { return *pItem; }
    };

    Node* Node::findChildNode( const SfxPoolItem& rItem )
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
        pNextNode = new Node( rItem, pNextNode );
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
    */

    Node* Node::nextItemSet( Node* pLast )
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
            pNext = *aIter;
            if( pNext->hasItemSet() ) // any child with item set?
                return pNext;
            pNext = pNext->nextItemSet( 0 ); // 0 => downstairs only
            if( pNext )
                return pNext;
            ++aIter;
        }
        // Searching upstairs
        if( pLast && pUpper )
            pNext = pUpper->nextItemSet( this );
        return pNext;
    }

    Node::~Node()
    {
        std::vector<Node*>::iterator aIter = mChildren.begin();
        while( aIter != mChildren.end() )
        {
            delete *aIter;
            ++aIter;
        }
        delete pItem;
    }

    class Iterator : public IStylePoolIteratorAccess
    {
        std::map< const SfxItemSet*, Node >& rRoot;
        std::map< const SfxItemSet*, Node >::iterator pCurrNode;
        Node* pNode;
    public:
        Iterator( std::map< const SfxItemSet*, Node >& rR ) 
            : rRoot( rR ), pCurrNode( rR.begin() ), pNode(0) {}
        virtual StylePool::SfxItemSet_Pointer_t getNext();
        virtual ::rtl::OUString getName();
    };

    StylePool::SfxItemSet_Pointer_t Iterator::getNext()
    {
        StylePool::SfxItemSet_Pointer_t pReturn;
        while( pNode || pCurrNode != rRoot.end() )
        {
            if( !pNode )
            {
                pNode = &pCurrNode->second;
                ++pCurrNode;
                if( pNode->hasItemSet() )
                    return pNode->getItemSet();
            }
            pNode = pNode->nextItemSet( pNode );
            if( pNode && pNode->hasItemSet() )
                return pNode->getItemSet();
        }
        return pReturn;
    }

    ::rtl::OUString Iterator::getName()
    {
        ::rtl::OUString aString;
        if( pNode && pNode->hasItemSet() )
            aString = StylePool::nameOf( pNode->getItemSet() );
        return aString;
    }


/* This static method creates a unique name from a shared pointer to a SfxItemSet
   The name is the memory address of the SfxItemSet itself. */

::rtl::OUString StylePool::nameOf( SfxItemSet_Pointer_t pSet )
{
    return ::rtl::OUString::valueOf( reinterpret_cast<sal_IntPtr>( pSet.get() ), 16 );
}

// class StylePoolImpl organized a tree-structure where every node represents a SfxItemSet.
// The insertItemSet method adds a SfxItemSet into the tree if necessary and returns a shared_ptr
// to a copy of the SfxItemSet.
// The aRoot-Node represents an empty SfxItemSet.

class StylePoolImpl
{
private:
    std::map< const SfxItemSet*, Node > aRoot;
    sal_Int32 nCount;
public:
    StylePoolImpl() : nCount(0) {}
    StylePool::SfxItemSet_Pointer_t insertItemSet( const SfxItemSet& rSet );
    IStylePoolIteratorAccess* createIterator();
    sal_Int32 getCount() const { return nCount; }
};

StylePool::SfxItemSet_Pointer_t StylePoolImpl::insertItemSet( const SfxItemSet& rSet )
{
    bool bNonPoolable = false;
    Node* pCurNode = &aRoot[ rSet.GetParent() ];
    SfxItemIter aIter( rSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    // Every SfxPoolItem in the SfxItemSet causes a step deeper into the tree,
    // a complete empty SfxItemSet would stay at the root node.
    while( pItem )
    {
        if( !rSet.GetPool()->IsItemFlag(pItem->Which(), SFX_ITEM_POOLABLE ) )
            bNonPoolable = true;
        pCurNode = pCurNode->findChildNode( *pItem );
        pItem = aIter.NextItem();
    }
    // Every leaf node represents an inserted item set, but "non-leaf" nodes represents subsets
    // of inserted itemsets.
    // These nodes could have but does not need to have a shared_ptr to a item set.
    if( !pCurNode->hasItemSet() )
    {
        pCurNode->setItemSet( rSet );
        bNonPoolable = false; // to avoid a double insertion
        ++nCount;
    }
    // If rSet contains at least one non poolable item, a new itemset has to be inserted
    if( bNonPoolable )
        pCurNode->setItemSet( rSet );
#ifdef DEBUG
    {
        sal_Int32 nCheck = -1;
        sal_Int32 nNo = -1;
        IStylePoolIteratorAccess* pIter = createIterator();
        StylePool::SfxItemSet_Pointer_t pTemp;
        do
        {
            ++nCheck;
            pTemp = pIter->getNext();
            if( pCurNode->hasItemSet() && pTemp.get() == pCurNode->getItemSet().get() )
            {
                ::rtl::OUString aStr = pIter->getName();
                nNo = nCheck;
            }
        } while( pTemp.get() );
        DBG_ASSERT( nCount == nCheck, "Wrong counting");
        delete pIter;
    }
#endif
    return pCurNode->getItemSet();
}

IStylePoolIteratorAccess* StylePoolImpl::createIterator()
{ return new Iterator( aRoot ); }

// Ctor, Dtor and redirected methods of class StylePool, nearly inline ;-)

StylePool::SfxItemSet_Pointer_t StylePool::insertItemSet( const SfxItemSet& rSet )
{ return pImpl->insertItemSet( rSet ); }

IStylePoolIteratorAccess* StylePool::createIterator()
{ return pImpl->createIterator(); }

sal_Int32 StylePool::getCount() const
{ return pImpl->getCount(); }

StylePool::~StylePool() { delete pImpl; }

// End of class StylePool
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
