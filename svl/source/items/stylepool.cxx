/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    
    
    
    
    class Node
    {
        std::vector<Node*> mChildren; 
        
        
        std::vector< StylePool::SfxItemSet_Pointer_t > maItemSet;
        const SfxPoolItem *mpItem;   
        Node *mpUpper;               
        
        const bool mbIsItemIgnorable;
    public:
        
        Node() 
            : mChildren(),
              maItemSet(),
              mpItem( 0 ),
              mpUpper( 0 ),
              mbIsItemIgnorable( false )
        {}
        Node( const SfxPoolItem& rItem, Node* pParent, const bool bIgnorable ) 
            : mChildren(),
              maItemSet(),
              mpItem( rItem.Clone() ),
              mpUpper( pParent ),
              mbIsItemIgnorable( bIgnorable )
        {}
        ~Node();
        
        bool hasItemSet( const bool bCheckUsage ) const;
        
        const StylePool::SfxItemSet_Pointer_t getItemSet() const
        {
            return maItemSet.back();
        }
        const StylePool::SfxItemSet_Pointer_t getUsedOrLastAddedItemSet() const;
        void setItemSet( const SfxItemSet& rSet ){ maItemSet.push_back( StylePool::SfxItemSet_Pointer_t( rSet.Clone() ) ); }
        
        Node* findChildNode( const SfxPoolItem& rItem,
                             const bool bIsItemIgnorable = false );
        Node* nextItemSet( Node* pLast,
                           const bool bSkipUnusedItemSet,
                           const bool bSkipIgnorable );
        const SfxPoolItem& getPoolItem() const { return *mpItem; }
        
        bool hasIgnorableChildren( const bool bCheckUsage ) const;
        const StylePool::SfxItemSet_Pointer_t getItemSetOfIgnorableChild(
                                        const bool bSkipUnusedItemSets ) const;
    };

    
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
        
        std::vector<Node*>::iterator aIter = mChildren.begin();
        
        
        if( pLast && pLast != this )
        {
            aIter = std::find( mChildren.begin(), mChildren.end(), pLast );
            if( aIter != mChildren.end() )
                ++aIter;
        }
        Node *pNext = 0;
        while( aIter != mChildren.end() )
        {
            
            if ( bSkipIgnorable && (*aIter)->mbIsItemIgnorable )
            {
                ++aIter;
                continue;
            }
            pNext = *aIter;
            
            if ( pNext->hasItemSet( bSkipUnusedItemSets ) )
            {
                return pNext;
            }
            if ( bSkipIgnorable &&
                 pNext->hasIgnorableChildren( bSkipUnusedItemSets ) )
            {
                return pNext;
            }
            pNext = pNext->nextItemSet( 0, bSkipUnusedItemSets, bSkipIgnorable ); 
            if( pNext )
                return pNext;
            ++aIter;
        }
        
        if( pLast && mpUpper )
        {
            
            pNext = mpUpper->nextItemSet( this, bSkipUnusedItemSets, bSkipIgnorable );
        }
        return pNext;
    }

    
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
                
                if ( mpNode->hasItemSet( mbSkipUnusedItemSets ) )
                {
                    
                    return mpNode->getUsedOrLastAddedItemSet();
                }
            }
            
            mpNode = mpNode->nextItemSet( mpNode, mbSkipUnusedItemSets, mbSkipIgnorable );
            if ( mpNode && mpNode->hasItemSet( mbSkipUnusedItemSets ) )
            {
                
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






class StylePoolImpl
{
private:
    std::map< const SfxItemSet*, Node > maRoot;
    sal_Int32 mnCount;
    
    SfxItemSet* mpIgnorableItems;
public:
    
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
    
    
    
    if( !pCurNode->hasItemSet( false ) )
    {
        pCurNode->setItemSet( rSet );
        bNonPoolable = false; 
        ++mnCount;
    }
    
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


IStylePoolIteratorAccess* StylePoolImpl::createIterator( bool bSkipUnusedItemSets,
                                                         bool bSkipIgnorableItems )
{
    return new Iterator( maRoot, bSkipUnusedItemSets, bSkipIgnorableItems );
}



StylePool::StylePool( SfxItemSet* pIgnorableItems )
    : pImpl( new StylePoolImpl( pIgnorableItems ) )
{}

StylePool::SfxItemSet_Pointer_t StylePool::insertItemSet( const SfxItemSet& rSet )
{ return pImpl->insertItemSet( rSet ); }


IStylePoolIteratorAccess* StylePool::createIterator( const bool bSkipUnusedItemSets,
                                                     const bool bSkipIgnorableItems )
{
    return pImpl->createIterator( bSkipUnusedItemSets, bSkipIgnorableItems );
}

sal_Int32 StylePool::getCount() const
{ return pImpl->getCount(); }

StylePool::~StylePool() { delete pImpl; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
