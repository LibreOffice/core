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


#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/awt/tree/XMutableTreeNode.hpp>
#include "treecontrolpeer.hxx"
#include <comphelper/processfactory.hxx>

#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <svtools/treelistbox.hxx>
#include <svtools/treelistentry.hxx>
#include <svtools/viewdataentry.hxx>
#include <svtools/svlbitm.hxx>

#include <map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt::tree;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::graphic;

struct LockGuard
{
public:
    explicit LockGuard( sal_Int32& rLock )
    : mrLock( rLock )
    {
        rLock++;
    }

    ~LockGuard()
    {
        mrLock--;
    }

    sal_Int32& mrLock;
};




class ImplContextGraphicItem : public SvLBoxContextBmp
{
public:
    ImplContextGraphicItem( SvTreeListEntry* pEntry,sal_uInt16 nFlags,Image& rI1,Image& rI2, bool bExpanded)
        : SvLBoxContextBmp(pEntry, nFlags, rI1, rI2, bExpanded) {}

    OUString msExpandedGraphicURL;
    OUString msCollapsedGraphicURL;
};



class UnoTreeListBoxImpl : public SvTreeListBox
{
public:
    UnoTreeListBoxImpl( TreeControlPeer* pPeer, vcl::Window* pParent, WinBits nWinStyle );
    virtual ~UnoTreeListBoxImpl();
    virtual void dispose() SAL_OVERRIDE;

    sal_uInt32 insert( SvTreeListEntry* pEntry,SvTreeListEntry* pParent,sal_uLong nPos=TREELIST_APPEND );

    virtual void    RequestingChildren( SvTreeListEntry* pParent ) SAL_OVERRIDE;

    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& ) SAL_OVERRIDE;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) SAL_OVERRIDE;

    DECL_LINK(OnSelectionChangeHdl, void *);
    DECL_LINK(OnExpandingHdl, void *);
    DECL_LINK(OnExpandedHdl, void *);

private:
    rtl::Reference< TreeControlPeer > mxPeer;
};



class UnoTreeListItem : public SvLBoxString
{
public:
    explicit        UnoTreeListItem( SvTreeListEntry* );
                    UnoTreeListItem();
    virtual         ~UnoTreeListItem();
    void            InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* ) SAL_OVERRIDE;
    void            SetImage( const Image& rImage );
    OUString        GetGraphicURL() const { return maGraphicURL;}
    void            SetGraphicURL( const OUString& rGraphicURL );
    virtual void    Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                          const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) SAL_OVERRIDE;
    SvLBoxItem*     Create() const SAL_OVERRIDE;
    void            Clone( SvLBoxItem* pSource ) SAL_OVERRIDE;

private:
    OUString        maGraphicURL;
    Image           maImage;
};



class UnoTreeListEntry : public SvTreeListEntry
{
public:
    UnoTreeListEntry( const Reference< XTreeNode >& xNode, TreeControlPeer* pPeer );
    virtual ~UnoTreeListEntry();

    Reference< XTreeNode > mxNode;
    TreeControlPeer* mpPeer;
};

TreeControlPeer::TreeControlPeer()
    : maSelectionListeners( *this )
    , maTreeExpansionListeners( *this )
    , maTreeEditListeners( *this )
    , mbIsRootDisplayed(false)
    , mpTreeImpl( 0 )
    , mnEditLock( 0 )
    , mpTreeNodeMap( 0 )
{
}



TreeControlPeer::~TreeControlPeer()
{
    if( mpTreeImpl )
        mpTreeImpl->Clear();
    delete mpTreeNodeMap;
}



void TreeControlPeer::addEntry( UnoTreeListEntry* pEntry )
{
    if( pEntry && pEntry->mxNode.is() )
    {
        if( !mpTreeNodeMap )
        {
            mpTreeNodeMap = new TreeNodeMap();
        }

        (*mpTreeNodeMap)[ pEntry->mxNode ] = pEntry;
    }
}



void TreeControlPeer::removeEntry( UnoTreeListEntry* pEntry )
{
    if( mpTreeNodeMap && pEntry && pEntry->mxNode.is() )
    {
        TreeNodeMap::iterator aIter( mpTreeNodeMap->find( pEntry->mxNode ) );
        if( aIter != mpTreeNodeMap->end() )
        {
            mpTreeNodeMap->erase( aIter );
        }
    }
}



UnoTreeListEntry* TreeControlPeer::getEntry( const Reference< XTreeNode >& xNode, bool bThrow /* = true */ ) throw( IllegalArgumentException )
{
    if( mpTreeNodeMap )
    {
        TreeNodeMap::iterator aIter( mpTreeNodeMap->find( xNode ) );
        if( aIter != mpTreeNodeMap->end() )
            return (*aIter).second;
    }

    if( bThrow )
        throw IllegalArgumentException();

    return 0;
}



vcl::Window* TreeControlPeer::createVclControl( vcl::Window* pParent, sal_Int64 nWinStyle )
{
    mpTreeImpl = VclPtr<UnoTreeListBoxImpl>::Create( this, pParent, nWinStyle );
    return mpTreeImpl;
}



/** called from the UnoTreeListBoxImpl when it gets deleted */
void TreeControlPeer::disposeControl()
{
    delete mpTreeNodeMap;
    mpTreeNodeMap = 0;
    mpTreeImpl = 0;
}



void TreeControlPeer::SetWindow( const VclPtr< vcl::Window > &pWindow )
{
    VCLXWindow::SetWindow( pWindow );
}



UnoTreeListEntry* TreeControlPeer::createEntry( const Reference< XTreeNode >& xNode, UnoTreeListEntry* pParent, sal_uLong nPos /* = TREELIST_APPEND */ )
{
    UnoTreeListEntry* pEntry = 0;
    if( mpTreeImpl )
    {
        Image aImage;
        pEntry = new UnoTreeListEntry( xNode, this );
        {
            std::unique_ptr<ImplContextGraphicItem> pContextBmp(
                new ImplContextGraphicItem(pEntry, 0, aImage, aImage, true));

            pEntry->AddItem(std::move(pContextBmp));
        }

        std::unique_ptr<UnoTreeListItem> pUnoItem(new UnoTreeListItem(pEntry));

        if( !xNode->getNodeGraphicURL().isEmpty() )
        {
            pUnoItem->SetGraphicURL( xNode->getNodeGraphicURL() );
            Image aNodeImage;
            loadImage( xNode->getNodeGraphicURL(), aNodeImage );
            pUnoItem->SetImage( aNodeImage );
            mpTreeImpl->AdjustEntryHeight( aNodeImage );
        }

        pEntry->AddItem(std::move(pUnoItem));

        mpTreeImpl->insert( pEntry, pParent, nPos );

        if( !msDefaultExpandedGraphicURL.isEmpty() )
            mpTreeImpl->SetExpandedEntryBmp( pEntry, maDefaultExpandedImage );

        if( !msDefaultCollapsedGraphicURL.isEmpty() )
            mpTreeImpl->SetCollapsedEntryBmp( pEntry, maDefaultCollapsedImage );

        updateEntry( pEntry );
    }
    return pEntry;
}



bool TreeControlPeer::updateEntry( UnoTreeListEntry* pEntry )
{
    bool bChanged = false;
    if( pEntry && pEntry->mxNode.is() && mpTreeImpl )
    {
        const OUString aValue( getEntryString( pEntry->mxNode->getDisplayValue() ) );
        UnoTreeListItem* pUnoItem = dynamic_cast< UnoTreeListItem* >( &pEntry->GetItem( 1 ) );
        if( pUnoItem )
        {
            if( aValue != pUnoItem->GetText() )
            {
                pUnoItem->SetText( aValue );
                bChanged = true;
            }

            if( pUnoItem->GetGraphicURL() != pEntry->mxNode->getNodeGraphicURL() )
            {
                Image aImage;
                if( loadImage( pEntry->mxNode->getNodeGraphicURL(), aImage ) )
                {
                    pUnoItem->SetGraphicURL( pEntry->mxNode->getNodeGraphicURL() );
                    pUnoItem->SetImage( aImage );
                    mpTreeImpl->AdjustEntryHeight( aImage );
                    bChanged = true;
                }
            }
        }

        if( bool(pEntry->mxNode->hasChildrenOnDemand()) != pEntry->HasChildrenOnDemand() )
        {
            pEntry->EnableChildrenOnDemand( pEntry->mxNode->hasChildrenOnDemand() );
            bChanged = true;
        }

        ImplContextGraphicItem* pContextGraphicItem = dynamic_cast< ImplContextGraphicItem* >( &pEntry->GetItem( 0 ) );
        if( pContextGraphicItem )
        {
            if( pContextGraphicItem->msExpandedGraphicURL != pEntry->mxNode->getExpandedGraphicURL() )
            {
                Image aImage;
                if( loadImage( pEntry->mxNode->getExpandedGraphicURL(), aImage ) )
                {
                    pContextGraphicItem->msExpandedGraphicURL = pEntry->mxNode->getExpandedGraphicURL();
                    mpTreeImpl->SetExpandedEntryBmp( pEntry, aImage );
                    bChanged = true;
                }
            }
            if( pContextGraphicItem->msCollapsedGraphicURL != pEntry->mxNode->getCollapsedGraphicURL() )
            {
                Image aImage;
                if( loadImage( pEntry->mxNode->getCollapsedGraphicURL(), aImage ) )
                {
                    pContextGraphicItem->msCollapsedGraphicURL = pEntry->mxNode->getCollapsedGraphicURL();
                    mpTreeImpl->SetCollapsedEntryBmp( pEntry, aImage );
                    bChanged = true;
                }
            }
        }

        if( bChanged )
            mpTreeImpl->GetModel()->InvalidateEntry( pEntry );
    }

    return bChanged;
}



void TreeControlPeer::onSelectionChanged()
{
    Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    EventObject aEvent( xSource );
    maSelectionListeners.selectionChanged( aEvent );
}



void TreeControlPeer::onRequestChildNodes( const Reference< XTreeNode >& xNode )
{
    try
    {
        Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
        TreeExpansionEvent aEvent( xSource, xNode );
        maTreeExpansionListeners.requestChildNodes( aEvent );
    }
    catch( Exception& )
    {
    }
}



bool TreeControlPeer::onExpanding( const Reference< XTreeNode >& xNode, bool bExpanding )
{
    try
    {
        Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
        TreeExpansionEvent aEvent( xSource, xNode );
        if( bExpanding )
        {
            maTreeExpansionListeners.treeExpanding( aEvent );
        }
        else
        {
            maTreeExpansionListeners.treeCollapsing( aEvent );
        }
    }
    catch( Exception& )
    {
        return false;
    }
    return true;
}



void TreeControlPeer::onExpanded( const Reference< XTreeNode >& xNode, bool bExpanding )
{
    try
    {
        Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
        TreeExpansionEvent aEvent( xSource, xNode );

        if( bExpanding )
        {
            maTreeExpansionListeners.treeExpanded( aEvent );
        }
        else
        {
            maTreeExpansionListeners.treeCollapsed( aEvent );
        }
    }
    catch( Exception& )
    {
    }
}



void TreeControlPeer::fillTree( UnoTreeListBoxImpl& rTree, const Reference< XTreeDataModel >& xDataModel )
{
    rTree.Clear();

    if( xDataModel.is() )
    {
        Reference< XTreeNode > xRootNode( xDataModel->getRoot() );
        if( xRootNode.is() )
        {
            if( mbIsRootDisplayed )
            {
                addNode( rTree, xRootNode, 0 );
            }
            else
            {
                const sal_Int32 nChildCount = xRootNode->getChildCount();
                for( sal_Int32 nChild = 0; nChild < nChildCount; nChild++ )
                    addNode( rTree, xRootNode->getChildAt( nChild ), 0 );
            }
        }
    }
}



void TreeControlPeer::addNode( UnoTreeListBoxImpl& rTree, const Reference< XTreeNode >& xNode, UnoTreeListEntry* pParentEntry )
{
    if( xNode.is() )
    {
        UnoTreeListEntry* pEntry = createEntry( xNode, pParentEntry );
        const sal_Int32 nChildCount = xNode->getChildCount();
        for( sal_Int32 nChild = 0; nChild < nChildCount; nChild++ )
            addNode( rTree, xNode->getChildAt( nChild ), pEntry );
    }
}



UnoTreeListBoxImpl& TreeControlPeer::getTreeListBoxOrThrow() const throw (RuntimeException )
{
    if( !mpTreeImpl )
        throw DisposedException();
    return *mpTreeImpl;
}



void TreeControlPeer::ChangeNodesSelection( const Any& rSelection, bool bSelect, bool bSetSelection ) throw( RuntimeException, IllegalArgumentException )
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

    Reference< XTreeNode > xTempNode;

    const Reference< XTreeNode > *pNodes = 0;
    sal_Int32 nCount = 0;

    if( rSelection.hasValue() )
    {
        switch( rSelection.getValueTypeClass() )
        {
        case TypeClass_INTERFACE:
            {
                rSelection >>= xTempNode;
                if( xTempNode.is() )
                {
                    nCount = 1;
                    pNodes = &xTempNode;
                }
                break;
            }
        case TypeClass_SEQUENCE:
            {
                if( rSelection.getValueType() == cppu::UnoType<Sequence< Reference< XTreeNode > >>::get() )
                {
                    const Sequence< Reference< XTreeNode > >& rSeq( *static_cast<const Sequence< Reference< XTreeNode > > *>(rSelection.getValue()) );
                    nCount = rSeq.getLength();
                    if( nCount )
                        pNodes = rSeq.getConstArray();
                }
                break;
            }
        default:
            break;
        }

        if( nCount == 0 )
            throw IllegalArgumentException();
    }

    if( bSetSelection )
        rTree.SelectAll( false );

    if( pNodes && nCount )
    {
        while( nCount-- )
        {
            UnoTreeListEntry* pEntry = getEntry( *pNodes++ );
            rTree.Select( pEntry, bSelect );
        }
    }
}


// ::com::sun::star::view::XSelectionSupplier


sal_Bool SAL_CALL TreeControlPeer::select( const Any& rSelection ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ChangeNodesSelection( rSelection, true, true );
    return sal_True;
}



Any SAL_CALL TreeControlPeer::getSelection() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

    Any aRet;

    sal_uLong nSelectionCount = rTree.GetSelectionCount();
    if( nSelectionCount == 1 )
    {
        UnoTreeListEntry* pEntry = dynamic_cast< UnoTreeListEntry* >( rTree.FirstSelected() );
        if( pEntry && pEntry->mxNode.is() )
            aRet <<= pEntry->mxNode;
    }
    else if( nSelectionCount > 1 )
    {
        Sequence< Reference< XTreeNode > > aSelection( nSelectionCount );
        Reference< XTreeNode >* pNodes = aSelection.getArray();
        UnoTreeListEntry* pEntry = dynamic_cast< UnoTreeListEntry* >( rTree.FirstSelected() );
        while( pEntry && nSelectionCount )
        {
            *pNodes++ = pEntry->mxNode;
            pEntry = dynamic_cast< UnoTreeListEntry* >( rTree.NextSelected( pEntry ) );
            --nSelectionCount;
        }

        OSL_ASSERT( (pEntry == 0) && (nSelectionCount == 0) );
        aRet <<= aSelection;
    }

    return aRet;
}



void SAL_CALL TreeControlPeer::addSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener ) throw (RuntimeException, std::exception)
{
    maSelectionListeners.addInterface( xListener );
}



void SAL_CALL TreeControlPeer::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener ) throw (RuntimeException, std::exception)
{
    maSelectionListeners.addInterface( xListener );
}


// ::com::sun::star::view::XMultiSelectionSupplier


sal_Bool SAL_CALL TreeControlPeer::addSelection( const Any& rSelection ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    ChangeNodesSelection( rSelection, true, false );
    return sal_True;
}



void SAL_CALL TreeControlPeer::removeSelection( const Any& rSelection ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    ChangeNodesSelection( rSelection, false, false );
}



void SAL_CALL TreeControlPeer::clearSelection() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    getTreeListBoxOrThrow().SelectAll( false );
}



sal_Int32 SAL_CALL TreeControlPeer::getSelectionCount() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return getTreeListBoxOrThrow().GetSelectionCount();
}



class TreeSelectionEnumeration : public ::cppu::WeakImplHelper< XEnumeration >
{
public:
    explicit TreeSelectionEnumeration( std::list< Any >& rSelection );
    virtual sal_Bool SAL_CALL hasMoreElements() throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Any SAL_CALL nextElement() throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception) SAL_OVERRIDE;

    std::list< Any > maSelection;
    std::list< Any >::iterator maIter;
};



TreeSelectionEnumeration::TreeSelectionEnumeration( std::list< Any >& rSelection )
{
    maSelection.swap( rSelection );
    maIter = maSelection.begin();
}



sal_Bool SAL_CALL TreeSelectionEnumeration::hasMoreElements() throw (RuntimeException, std::exception)
{
    return maIter != maSelection.end();
}



Any SAL_CALL TreeSelectionEnumeration::nextElement() throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    if( maIter == maSelection.end() )
        throw NoSuchElementException();

    return (*maIter++);
}



Reference< XEnumeration > SAL_CALL TreeControlPeer::createSelectionEnumeration() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

    sal_uInt32 nSelectionCount = rTree.GetSelectionCount();
    std::list< Any > aSelection( nSelectionCount );

    UnoTreeListEntry* pEntry = dynamic_cast< UnoTreeListEntry* >( rTree.FirstSelected() );
    while( pEntry && nSelectionCount )
    {
        aSelection.push_back( Any( pEntry->mxNode ) );
        pEntry = dynamic_cast< UnoTreeListEntry* >( rTree.NextSelected( pEntry ) );
        --nSelectionCount;
    }

    OSL_ASSERT( (pEntry == 0) && (nSelectionCount == 0) );

    return Reference< XEnumeration >( new TreeSelectionEnumeration( aSelection ) );
}



Reference< XEnumeration > SAL_CALL TreeControlPeer::createReverseSelectionEnumeration() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

    sal_uInt32 nSelectionCount = rTree.GetSelectionCount();
    std::list< Any > aSelection;

    UnoTreeListEntry* pEntry = dynamic_cast< UnoTreeListEntry* >( rTree.FirstSelected() );
    while( pEntry && nSelectionCount )
    {
        aSelection.push_front( Any( pEntry->mxNode ) );
        pEntry = dynamic_cast< UnoTreeListEntry* >( rTree.NextSelected( pEntry ) );
        --nSelectionCount;
    }

    OSL_ASSERT( (pEntry == 0) && (nSelectionCount == 0) );

    return Reference< XEnumeration >( new TreeSelectionEnumeration( aSelection ) );
}


// ::com::sun::star::awt::XTreeControl


OUString SAL_CALL TreeControlPeer::getDefaultExpandedGraphicURL() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return msDefaultExpandedGraphicURL;
}



void SAL_CALL TreeControlPeer::setDefaultExpandedGraphicURL( const OUString& sDefaultExpandedGraphicURL ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if( msDefaultExpandedGraphicURL != sDefaultExpandedGraphicURL )
    {
        if( !sDefaultExpandedGraphicURL.isEmpty() )
            loadImage( sDefaultExpandedGraphicURL, maDefaultExpandedImage );
        else
            maDefaultExpandedImage = Image();

        UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

        SvTreeListEntry* pEntry = rTree.First();
        while( pEntry )
        {
            ImplContextGraphicItem* pContextGraphicItem = dynamic_cast< ImplContextGraphicItem* >( &pEntry->GetItem( 0 ) );
            if( pContextGraphicItem )
            {
                if( pContextGraphicItem->msExpandedGraphicURL.isEmpty() )
                    rTree.SetExpandedEntryBmp( pEntry, maDefaultExpandedImage );
            }
            pEntry = rTree.Next( pEntry );
        }

        msDefaultExpandedGraphicURL = sDefaultExpandedGraphicURL;
    }
}



OUString SAL_CALL TreeControlPeer::getDefaultCollapsedGraphicURL() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return msDefaultCollapsedGraphicURL;
}



void SAL_CALL TreeControlPeer::setDefaultCollapsedGraphicURL( const OUString& sDefaultCollapsedGraphicURL ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if( msDefaultCollapsedGraphicURL != sDefaultCollapsedGraphicURL )
    {
        if( !sDefaultCollapsedGraphicURL.isEmpty() )
            loadImage( sDefaultCollapsedGraphicURL, maDefaultCollapsedImage );
        else
            maDefaultCollapsedImage = Image();

        UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

        SvTreeListEntry* pEntry = rTree.First();
        while( pEntry )
        {
            ImplContextGraphicItem* pContextGraphicItem = dynamic_cast< ImplContextGraphicItem* >( &pEntry->GetItem( 0 ) );
            if( pContextGraphicItem )
            {
                if( pContextGraphicItem->msCollapsedGraphicURL.isEmpty() )
                    rTree.SetCollapsedEntryBmp( pEntry, maDefaultCollapsedImage );
            }
            pEntry = rTree.Next( pEntry );
        }

        msDefaultCollapsedGraphicURL = sDefaultCollapsedGraphicURL;
    }
}



sal_Bool SAL_CALL TreeControlPeer::isNodeExpanded( const Reference< XTreeNode >& xNode ) throw (RuntimeException, IllegalArgumentException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    UnoTreeListEntry* pEntry = getEntry( xNode );
    return pEntry && rTree.IsExpanded( pEntry );
}



sal_Bool SAL_CALL TreeControlPeer::isNodeCollapsed( const Reference< XTreeNode >& xNode ) throw (RuntimeException, IllegalArgumentException, std::exception)
{
    SolarMutexGuard aGuard;
    return !isNodeExpanded( xNode );
}



void SAL_CALL TreeControlPeer::makeNodeVisible( const Reference< XTreeNode >& xNode ) throw (RuntimeException, ExpandVetoException, IllegalArgumentException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    UnoTreeListEntry* pEntry = getEntry( xNode );
    if( pEntry )
        rTree.MakeVisible( pEntry );
}



sal_Bool SAL_CALL TreeControlPeer::isNodeVisible( const Reference< XTreeNode >& xNode ) throw (RuntimeException, IllegalArgumentException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    UnoTreeListEntry* pEntry = getEntry( xNode );
    return pEntry && rTree.IsEntryVisible( pEntry );
}



void SAL_CALL TreeControlPeer::expandNode( const Reference< XTreeNode >& xNode ) throw (RuntimeException, ExpandVetoException, IllegalArgumentException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    UnoTreeListEntry* pEntry = getEntry( xNode );
    if( pEntry )
        rTree.Expand( pEntry );
}



void SAL_CALL TreeControlPeer::collapseNode( const Reference< XTreeNode >& xNode ) throw (RuntimeException, ExpandVetoException, IllegalArgumentException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    UnoTreeListEntry* pEntry = getEntry( xNode );
    if( pEntry )
        rTree.Collapse( pEntry );
}



void SAL_CALL TreeControlPeer::addTreeExpansionListener( const Reference< XTreeExpansionListener >& xListener ) throw (RuntimeException, std::exception)
{
    maTreeExpansionListeners.addInterface( xListener );
}



void SAL_CALL TreeControlPeer::removeTreeExpansionListener( const Reference< XTreeExpansionListener >& xListener ) throw (RuntimeException, std::exception)
{
    maTreeExpansionListeners.removeInterface( xListener );
}



Reference< XTreeNode > SAL_CALL TreeControlPeer::getNodeForLocation( sal_Int32 x, sal_Int32 y ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

    Reference< XTreeNode > xNode;

    const Point aPos( x, y );
    UnoTreeListEntry* pEntry = dynamic_cast< UnoTreeListEntry* >( rTree.GetEntry( aPos, true ) );
    if( pEntry )
        xNode = pEntry->mxNode;

    return xNode;
}



Reference< XTreeNode > SAL_CALL TreeControlPeer::getClosestNodeForLocation( sal_Int32 x, sal_Int32 y ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

    Reference< XTreeNode > xNode;

    const Point aPos( x, y );
    UnoTreeListEntry* pEntry = dynamic_cast< UnoTreeListEntry* >( rTree.GetEntry( aPos, true ) );
    if( pEntry )
        xNode = pEntry->mxNode;

    return xNode;
}



awt::Rectangle SAL_CALL TreeControlPeer::getNodeRect( const Reference< XTreeNode >& i_Node ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    UnoTreeListEntry* pEntry = getEntry( i_Node );

    ::Rectangle aEntryRect( rTree.GetFocusRect( pEntry, rTree.GetEntryPosition( pEntry ).Y() ) );
    return VCLUnoHelper::ConvertToAWTRect( aEntryRect );
}



sal_Bool SAL_CALL TreeControlPeer::isEditing(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    return rTree.IsEditingActive();
}



sal_Bool SAL_CALL TreeControlPeer::stopEditing() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    if( rTree.IsEditingActive() )
    {
        rTree.EndEditing();
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}



void SAL_CALL TreeControlPeer::cancelEditing(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    rTree.EndEditing();
}



void SAL_CALL TreeControlPeer::startEditingAtNode( const Reference< XTreeNode >& xNode ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    UnoTreeListEntry* pEntry = getEntry( xNode );
    rTree.EditEntry( pEntry );
}

void SAL_CALL TreeControlPeer::addTreeEditListener( const Reference< XTreeEditListener >& xListener ) throw (RuntimeException, std::exception)
{
    maTreeEditListeners.addInterface( xListener );
}

void SAL_CALL TreeControlPeer::removeTreeEditListener( const Reference< XTreeEditListener >& xListener ) throw (RuntimeException, std::exception)
{
    maTreeEditListeners.removeInterface( xListener );
}

bool TreeControlPeer::onEditingEntry( UnoTreeListEntry* pEntry )
{
    if( mpTreeImpl && pEntry && pEntry->mxNode.is() && (maTreeEditListeners.getLength() > 0)  )
    {
        try
        {
            maTreeEditListeners.nodeEditing( pEntry->mxNode );
        }
        catch( VetoException& )
        {
            return false;
        }
        catch( Exception& )
        {
        }
    }
    return true;
}

bool TreeControlPeer::onEditedEntry( UnoTreeListEntry* pEntry, const OUString& rNewText )
{
    if( mpTreeImpl && pEntry && pEntry->mxNode.is() ) try
    {
        LockGuard aLockGuard( mnEditLock );
        if( maTreeEditListeners.getLength() > 0 )
        {
            maTreeEditListeners.nodeEdited( pEntry->mxNode, rNewText );
            return false;
        }
        else
        {
            Reference< XMutableTreeNode > xMutableNode( pEntry->mxNode, UNO_QUERY );
            if( xMutableNode.is() )
                xMutableNode->setDisplayValue( Any( rNewText ) );
            else
                return false;
        }

    }
    catch( Exception& )
    {
    }

    return true;
}


// ::com::sun::star::awt::tree::TreeDataModelListener


void SAL_CALL TreeControlPeer::treeNodesChanged( const ::com::sun::star::awt::tree::TreeDataModelEvent& rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mnEditLock != 0 )
        return;

    updateTree( rEvent, true );
}

void SAL_CALL TreeControlPeer::treeNodesInserted( const ::com::sun::star::awt::tree::TreeDataModelEvent& rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mnEditLock != 0 )
        return;

    updateTree( rEvent, true );
}

void SAL_CALL TreeControlPeer::treeNodesRemoved( const ::com::sun::star::awt::tree::TreeDataModelEvent& rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mnEditLock != 0 )
        return;

    updateTree( rEvent, true );
}

void SAL_CALL TreeControlPeer::treeStructureChanged( const ::com::sun::star::awt::tree::TreeDataModelEvent& rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mnEditLock != 0 )
        return;

    updateTree( rEvent, true );
}

void TreeControlPeer::updateTree( const ::com::sun::star::awt::tree::TreeDataModelEvent& rEvent, bool bRecursive )
{
    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

    Sequence< Reference< XTreeNode > > Nodes;
    Reference< XTreeNode > xNode( rEvent.ParentNode );
    if( !xNode.is() && Nodes.getLength() )
    {
        xNode = Nodes[0];
    }

    if( xNode.is() )
        updateNode( rTree, xNode, bRecursive );
}

void TreeControlPeer::updateNode( UnoTreeListBoxImpl& rTree, const Reference< XTreeNode >& xNode, bool bRecursive )
{
    if( xNode.is() )
    {
        UnoTreeListEntry* pNodeEntry = getEntry( xNode, false );

        if( !pNodeEntry )
        {
            Reference< XTreeNode > xParentNode( xNode->getParent() );
            UnoTreeListEntry* pParentEntry = 0;
            sal_uLong nChild = TREELIST_APPEND;

            if( xParentNode.is() )
            {
                pParentEntry = getEntry( xParentNode  );
                nChild = xParentNode->getIndex( xNode );
            }

            pNodeEntry = createEntry( xNode, pParentEntry, nChild );
        }

        if( bRecursive )
            updateChildNodes( rTree, xNode, pNodeEntry );
    }
}

void TreeControlPeer::updateChildNodes( UnoTreeListBoxImpl& rTree, const Reference< XTreeNode >& xParentNode, UnoTreeListEntry* pParentEntry )
{
    if( xParentNode.is() && pParentEntry )
    {
        UnoTreeListEntry* pCurrentChild = dynamic_cast< UnoTreeListEntry* >( rTree.FirstChild( pParentEntry ) );

        const sal_Int32 nChildCount = xParentNode->getChildCount();
        for( sal_Int32 nChild = 0; nChild < nChildCount; nChild++ )
        {
            Reference< XTreeNode > xNode( xParentNode->getChildAt( nChild ) );
            if( !pCurrentChild || ( pCurrentChild->mxNode != xNode ) )
            {
                UnoTreeListEntry* pNodeEntry = getEntry( xNode, false );
                if( pNodeEntry == 0 )
                {
                    // child node is not yet part of the tree, add it
                    pCurrentChild = createEntry( xNode, pParentEntry, nChild );
                }
                else if( pNodeEntry != pCurrentChild )
                {
                    // node is already part of the tree, but not on the correct position
                    rTree.GetModel()->Move( pNodeEntry, pParentEntry, nChild );
                    pCurrentChild = pNodeEntry;
                    updateEntry( pCurrentChild );
                }
            }
            else
            {
                // child node has entry and entry is equal to current entry,
                // so no structural changes happened
                updateEntry( pCurrentChild );
            }

            pCurrentChild = dynamic_cast< UnoTreeListEntry* >( SvTreeListBox::NextSibling( pCurrentChild ) );
        }

        // check if we have entries without nodes left, we need to remove them
        while( pCurrentChild )
        {
            UnoTreeListEntry* pNextChild = dynamic_cast< UnoTreeListEntry* >( SvTreeListBox::NextSibling( pCurrentChild ) );
            rTree.GetModel()->Remove( pCurrentChild );
            pCurrentChild = pNextChild;
        }
    }
}

OUString TreeControlPeer::getEntryString( const Any& rValue )
{
    OUString sValue;
    if( rValue.hasValue() )
    {
        switch( rValue.getValueTypeClass() )
        {
        case TypeClass_SHORT:
        case TypeClass_LONG:
            {
                sal_Int32 nValue = 0;
                if( rValue >>= nValue )
                    sValue = OUString::number( nValue );
                break;
            }
        case TypeClass_BYTE:
        case TypeClass_UNSIGNED_SHORT:
        case TypeClass_UNSIGNED_LONG:
            {
                sal_uInt32 nValue = 0;
                if( rValue >>= nValue )
                    sValue = OUString::number( nValue );
                break;
            }
        case TypeClass_HYPER:
            {
                sal_Int64 nValue = 0;
                if( rValue >>= nValue )
                    sValue = OUString::number( nValue );
                break;
            }
        case TypeClass_UNSIGNED_HYPER:
            {
                sal_uInt64 nValue = 0;
                if( rValue >>= nValue )
                    sValue = OUString::number( nValue );
                break;
            }
        case TypeClass_FLOAT:
        case TypeClass_DOUBLE:
            {
                double fValue = 0.0;
                if( rValue >>= fValue )
                    sValue = OUString::number( fValue );
                break;
            }
        case TypeClass_STRING:
            rValue >>= sValue;
            break;
    /*
        case TypeClass_INTERFACE:
            // @todo
            break;
        case TypeClass_SEQUENCE:
            {
                Sequence< Any > aValues;
                if( aValue >>= aValues )
                {
                    updateEntry( SvTreeListEntry& rEntry, aValues );
                    return;
                }
            }
            break;
    */
        default:
            break;
        }
    }
    return sValue;
}

// XEventListener
void SAL_CALL TreeControlPeer::disposing( const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    // model is disposed, so we clear our tree
    SolarMutexGuard aGuard;
    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
    rTree.Clear();
    mxDataModel.clear();
}

void TreeControlPeer::onChangeDataModel( UnoTreeListBoxImpl& rTree, const Reference< XTreeDataModel >& xDataModel )
{
    if( xDataModel.is() && (mxDataModel == xDataModel) )
        return; // do nothing

    Reference< XTreeDataModelListener > xListener( this );

    if( mxDataModel.is() )
        mxDataModel->removeTreeDataModelListener( xListener );

    mxDataModel = xDataModel;

    fillTree( rTree, mxDataModel );

    if( mxDataModel.is() )
        mxDataModel->addTreeDataModelListener( xListener );
}


// ::com::sun::star::awt::XLayoutConstrains


::com::sun::star::awt::Size TreeControlPeer::getMinimumSize() throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aSz;
/* todo
    MultiLineEdit* pEdit = (MultiLineEdit*) GetWindow();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcMinimumSize());
*/
    return aSz;
}

::com::sun::star::awt::Size TreeControlPeer::getPreferredSize() throw(RuntimeException, std::exception)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size TreeControlPeer::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aSz = rNewSize;
/* todo
    MultiLineEdit* pEdit = (MultiLineEdit*) GetWindow();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcAdjustedSize( VCLSize(rNewSize )));
*/
    return aSz;
}


// ::com::sun::star::awt::XVclWindowPeer


void TreeControlPeer::setProperty( const OUString& PropertyName, const Any& aValue) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

    switch( GetPropertyId( PropertyName ) )
    {
        case BASEPROPERTY_HIDEINACTIVESELECTION:
        {
            bool bEnabled = false;
            if ( aValue >>= bEnabled )
            {
                WinBits nStyle = rTree.GetStyle();
                if ( bEnabled )
                    nStyle |= WB_HIDESELECTION;
                else
                    nStyle &= ~WB_HIDESELECTION;
                rTree.SetStyle( nStyle );
            }
        }
        break;

        case BASEPROPERTY_TREE_SELECTIONTYPE:
        {
            SelectionType eSelectionType;
            if( aValue >>= eSelectionType )
            {
                SelectionMode eSelMode;
                switch( eSelectionType )
                {
                case SelectionType_SINGLE:  eSelMode = SINGLE_SELECTION; break;
                case SelectionType_RANGE:   eSelMode = RANGE_SELECTION; break;
                case SelectionType_MULTI:   eSelMode = MULTIPLE_SELECTION; break;
    //          case SelectionType_NONE:
                default:                    eSelMode = NO_SELECTION; break;
                }
                if( rTree.GetSelectionMode() != eSelMode )
                    rTree.SetSelectionMode( eSelMode );
            }
            break;
        }

        case BASEPROPERTY_TREE_DATAMODEL:
            onChangeDataModel( rTree, Reference< XTreeDataModel >( aValue, UNO_QUERY ) );
            break;
        case BASEPROPERTY_ROW_HEIGHT:
        {
            sal_Int32 nHeight = 0;
            if( aValue >>= nHeight )
                rTree.SetEntryHeight( (short)nHeight );
            break;
        }
        case BASEPROPERTY_TREE_EDITABLE:
        {
            bool bEnabled = false;
            if( aValue >>= bEnabled )
                rTree.EnableInplaceEditing( bEnabled );
            break;
        }
        case BASEPROPERTY_TREE_INVOKESSTOPNODEEDITING:
            break; // @todo
        case BASEPROPERTY_TREE_ROOTDISPLAYED:
        {
            bool bDisplayed = false;
            if( (aValue >>= bDisplayed) && ( bDisplayed != mbIsRootDisplayed) )
            {
                onChangeRootDisplayed(bDisplayed);
            }
            break;
        }
        case BASEPROPERTY_TREE_SHOWSHANDLES:
        {
            bool bEnabled = false;
            if( aValue >>= bEnabled )
            {
                WinBits nBits = rTree.GetStyle() & (~WB_HASLINES);
                if( bEnabled )
                    nBits |= WB_HASLINES;
                if( nBits != rTree.GetStyle() )
                    rTree.SetStyle( nBits );
            }
            break;
        }
        case BASEPROPERTY_TREE_SHOWSROOTHANDLES:
        {
            bool bEnabled = false;
            if( aValue >>= bEnabled )
            {
                WinBits nBits = rTree.GetStyle() & (~WB_HASLINESATROOT);
                if( bEnabled )
                    nBits |= WB_HASLINESATROOT;
                if( nBits != rTree.GetStyle() )
                    rTree.SetStyle( nBits );
            }
            break;
        }
        default:
        VCLXWindow::setProperty( PropertyName, aValue );
        break;
    }
}

Any TreeControlPeer::getProperty( const OUString& PropertyName ) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const sal_uInt16 nPropId = GetPropertyId( PropertyName );
    if( (nPropId >= BASEPROPERTY_TREE_START) && (nPropId <= BASEPROPERTY_TREE_END) )
    {
        UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();
        switch(nPropId)
        {
        case BASEPROPERTY_TREE_SELECTIONTYPE:
        {
            SelectionType eSelectionType;

            SelectionMode eSelMode = rTree.GetSelectionMode();
            switch( eSelMode )
            {
            case SINGLE_SELECTION:  eSelectionType = SelectionType_SINGLE; break;
            case RANGE_SELECTION:   eSelectionType = SelectionType_RANGE; break;
            case MULTIPLE_SELECTION:eSelectionType = SelectionType_MULTI; break;
//          case NO_SELECTION:
            default:                eSelectionType = SelectionType_NONE; break;
            }
            return Any( eSelectionType );
        }
        case BASEPROPERTY_ROW_HEIGHT:
            return Any( (sal_Int32)rTree.GetEntryHeight() );
        case BASEPROPERTY_TREE_DATAMODEL:
            return Any( mxDataModel );
        case BASEPROPERTY_TREE_EDITABLE:
            return Any( rTree.IsInplaceEditingEnabled() );
        case BASEPROPERTY_TREE_INVOKESSTOPNODEEDITING:
            return Any( sal_True ); // @todo
        case BASEPROPERTY_TREE_ROOTDISPLAYED:
            return Any( mbIsRootDisplayed );
        case BASEPROPERTY_TREE_SHOWSHANDLES:
            return Any( (rTree.GetStyle() & WB_HASLINES) != 0 );
        case BASEPROPERTY_TREE_SHOWSROOTHANDLES:
            return Any( (rTree.GetStyle() & WB_HASLINESATROOT) != 0 );
        }
    }
    return VCLXWindow::getProperty( PropertyName );
}

void TreeControlPeer::onChangeRootDisplayed( bool bIsRootDisplayed )
{
    if( mbIsRootDisplayed == bIsRootDisplayed )
        return;

    mbIsRootDisplayed = bIsRootDisplayed;

    UnoTreeListBoxImpl& rTree = getTreeListBoxOrThrow();

    if( rTree.GetEntryCount() == 0 )
        return;

    // todo
    fillTree( rTree, mxDataModel );
    if( mbIsRootDisplayed )
    {
    }
    else
    {
    }
}

bool TreeControlPeer::loadImage( const OUString& rURL, Image& rImage )
{
    if( !mxGraphicProvider.is() )
    {
        mxGraphicProvider = graphic::GraphicProvider::create(
            comphelper::getProcessComponentContext());
    }

    try
    {
        ::com::sun::star::beans::PropertyValues aProps( 1 );
        aProps[0].Name = "URL";
        aProps[0].Value <<= rURL;

        Reference< XGraphic > xGraphic( mxGraphicProvider->queryGraphic( aProps ) );

        Graphic aGraphic( xGraphic );
        rImage = Image(aGraphic.GetBitmapEx());
        return true;
    }
    catch( Exception& )
    {
    }

    return false;
}


// class UnoTreeListBoxImpl


UnoTreeListBoxImpl::UnoTreeListBoxImpl( TreeControlPeer* pPeer, vcl::Window* pParent, WinBits nWinStyle )
: SvTreeListBox( pParent, nWinStyle )
, mxPeer( pPeer )
{
    SetStyle( WB_BORDER | WB_HASLINES |WB_HASBUTTONS | WB_HASLINESATROOT | WB_HASBUTTONSATROOT | WB_HSCROLL );
    SetNodeDefaultImages();
    SetSelectHdl( LINK(this, UnoTreeListBoxImpl, OnSelectionChangeHdl) );
    SetDeselectHdl( LINK(this, UnoTreeListBoxImpl, OnSelectionChangeHdl) );

    SetExpandingHdl( LINK(this, UnoTreeListBoxImpl, OnExpandingHdl) );
    SetExpandedHdl( LINK(this, UnoTreeListBoxImpl, OnExpandedHdl) );

}



UnoTreeListBoxImpl::~UnoTreeListBoxImpl()
{
    disposeOnce();
}

void UnoTreeListBoxImpl::dispose()
{
    if( mxPeer.is() )
        mxPeer->disposeControl();
    SvTreeListBox::dispose();
}



IMPL_LINK_NOARG(UnoTreeListBoxImpl, OnSelectionChangeHdl)
{
    if( mxPeer.is() )
        mxPeer->onSelectionChanged();
    return 0;
}



IMPL_LINK_NOARG(UnoTreeListBoxImpl, OnExpandingHdl)
{
    UnoTreeListEntry* pEntry = dynamic_cast< UnoTreeListEntry* >( GetHdlEntry() );

    if( pEntry && mxPeer.is() )
    {
        return mxPeer->onExpanding( pEntry->mxNode, !IsExpanded( pEntry ) ) ? 1 : 0;
    }
    return 0;
}



IMPL_LINK_NOARG(UnoTreeListBoxImpl, OnExpandedHdl)
{
    UnoTreeListEntry* pEntry = dynamic_cast< UnoTreeListEntry* >( GetHdlEntry() );
    if( pEntry && mxPeer.is() )
    {
        mxPeer->onExpanded( pEntry->mxNode, IsExpanded( pEntry ) );
    }
    return 0;
}



sal_uInt32 UnoTreeListBoxImpl::insert( SvTreeListEntry* pEntry,SvTreeListEntry* pParent,sal_uLong nPos )
{
    if( pParent )
        return SvTreeListBox::Insert( pEntry, pParent, nPos );
    else
        return SvTreeListBox::Insert( pEntry, nPos );
}



void UnoTreeListBoxImpl::RequestingChildren( SvTreeListEntry* pParent )
{
    UnoTreeListEntry* pEntry = dynamic_cast< UnoTreeListEntry* >( pParent );
    if( pEntry && pEntry->mxNode.is() && mxPeer.is() )
        mxPeer->onRequestChildNodes( pEntry->mxNode );
}



bool UnoTreeListBoxImpl::EditingEntry( SvTreeListEntry* pEntry, Selection& )
{
    return mxPeer.is() && mxPeer->onEditingEntry( dynamic_cast< UnoTreeListEntry* >( pEntry ) );
}



bool UnoTreeListBoxImpl::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText )
{
    return mxPeer.is() && mxPeer->onEditedEntry( dynamic_cast< UnoTreeListEntry* >( pEntry ), rNewText );
}


// class UnoTreeListItem


UnoTreeListItem::UnoTreeListItem( SvTreeListEntry* pEntry )
: SvLBoxString(pEntry, 0, OUString())
{
}



UnoTreeListItem::UnoTreeListItem()
: SvLBoxString()
{
}



UnoTreeListItem::~UnoTreeListItem()
{
}



void UnoTreeListItem::Paint(
    const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext, const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    Point aPos(rPos);
    Size aSize(GetSize(&rDev, &rEntry));
    if (!!maImage)
    {
        rRenderContext.DrawImage(aPos, maImage, rDev.IsEnabled() ? DrawImageFlags::NONE : DrawImageFlags::Disable);
        int nWidth = maImage.GetSizePixel().Width() + 6;
        aPos.X() += nWidth;
        aSize.Width() -= nWidth;
    }
    rRenderContext.DrawText(Rectangle(aPos,aSize),maText, rDev.IsEnabled() ? DrawTextFlags::NONE : DrawTextFlags::Disable);
}



SvLBoxItem* UnoTreeListItem::Create() const
{
    return new UnoTreeListItem;
}



void UnoTreeListItem::Clone( SvLBoxItem* pSource )
{
    UnoTreeListItem* pSourceItem = dynamic_cast< UnoTreeListItem* >( pSource );
    if( pSourceItem )
    {
        maText = pSourceItem->maText;
        maImage = pSourceItem->maImage;
    }
}



void UnoTreeListItem::SetImage( const Image& rImage )
{
    maImage = rImage;
}






void UnoTreeListItem::SetGraphicURL( const OUString& rGraphicURL )
{
    maGraphicURL = rGraphicURL;
}



void UnoTreeListItem::InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    pViewData->maSize = maImage.GetSizePixel();

    const Size aTextSize(pView->GetTextWidth( maText ), pView->GetTextHeight());
    if( pViewData->maSize.Width() )
    {
        pViewData->maSize.Width() += 6 + aTextSize.Width();
        if( pViewData->maSize.Height() < aTextSize.Height() )
            pViewData->maSize.Height() = aTextSize.Height();
    }
    else
    {
        pViewData->maSize = aTextSize;
    }
}



UnoTreeListEntry::UnoTreeListEntry( const Reference< XTreeNode >& xNode, TreeControlPeer* pPeer )
: SvTreeListEntry()
, mxNode( xNode )
, mpPeer( pPeer )
{
    if( mpPeer )
        mpPeer->addEntry( this );
}



UnoTreeListEntry::~UnoTreeListEntry()
{
    if( mpPeer )
        mpPeer->removeEntry( this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
