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

#include <com/sun/star/awt/tree/XMutableTreeDataModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#include <toolkit/helper/servicenames.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::tree;
using namespace ::com::sun::star::lang;

namespace {

    enum broadcast_type { nodes_changed, nodes_inserted, nodes_removed, structure_changed };

class MutableTreeNode;
class MutableTreeDataModel;

typedef rtl::Reference< MutableTreeNode > MutableTreeNodeRef;
typedef std::vector< MutableTreeNodeRef > TreeNodeVector;
typedef rtl::Reference< MutableTreeDataModel > MutableTreeDataModelRef;

static void implThrowIllegalArgumentException() throw( IllegalArgumentException )
{
    throw IllegalArgumentException();
}

class MutableTreeDataModel : public ::cppu::WeakAggImplHelper2< XMutableTreeDataModel, XServiceInfo >,
                             public MutexAndBroadcastHelper
{
public:
    MutableTreeDataModel();
    virtual ~MutableTreeDataModel();

    void broadcast( broadcast_type eType, const Reference< XTreeNode >& xParentNode, const Reference< XTreeNode >* pNodes, sal_Int32 nNodes );

    // XMutableTreeDataModel
    virtual css::uno::Reference< css::awt::tree::XMutableTreeNode > SAL_CALL createNode( const css::uno::Any& DisplayValue, sal_Bool ChildrenOnDemand ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRoot( const css::uno::Reference< css::awt::tree::XMutableTreeNode >& RootNode ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XTreeDataModel
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getRoot(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addTreeDataModelListener( const css::uno::Reference< css::awt::tree::XTreeDataModelListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTreeDataModelListener( const css::uno::Reference< css::awt::tree::XTreeDataModelListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) throw (RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException, std::exception) override;

private:
    bool mbDisposed;
    Reference< XTreeNode > mxRootNode;
};

class MutableTreeNode: public ::cppu::WeakAggImplHelper2< XMutableTreeNode, XServiceInfo >
{
    friend class MutableTreeDataModel;

public:
    MutableTreeNode( const MutableTreeDataModelRef& xModel, const Any& rValue, bool bChildrenOnDemand );
    virtual ~MutableTreeNode();

    void setParent( MutableTreeNode* pParent );
    void broadcast_changes();
    void broadcast_changes(const Reference< XTreeNode >& xNode, bool bNew);

    // XMutableTreeNode
    virtual css::uno::Any SAL_CALL getDataValue() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDataValue( const css::uno::Any& _datavalue ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL appendChild( const css::uno::Reference< css::awt::tree::XMutableTreeNode >& ChildNode ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertChildByIndex( ::sal_Int32 Index, const css::uno::Reference< css::awt::tree::XMutableTreeNode >& ChildNode ) throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChildByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHasChildrenOnDemand( sal_Bool ChildrenOnDemand ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDisplayValue( const css::uno::Any& Value ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setNodeGraphicURL( const OUString& URL ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setExpandedGraphicURL( const OUString& URL ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCollapsedGraphicURL( const OUString& URL ) throw (css::uno::RuntimeException, std::exception) override;

    // XTreeNode
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getChildAt( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getIndex( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasChildrenOnDemand(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getDisplayValue(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getNodeGraphicURL(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getExpandedGraphicURL(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getCollapsedGraphicURL(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException, std::exception) override;

    static MutableTreeNode* getImplementation( const Reference< XTreeNode >& xNode, bool bThrows ) throw (IllegalArgumentException);
    static Reference< XTreeNode > getReference( MutableTreeNode* pNode )
    {
        return Reference< XTreeNode >( pNode );
    }

private:
    TreeNodeVector  maChildren;
    Any maDisplayValue;
    Any maDataValue;
    bool mbHasChildrenOnDemand;
    ::osl::Mutex maMutex;
    MutableTreeNode* mpParent;
    MutableTreeDataModelRef mxModel;
    OUString maNodeGraphicURL;
    OUString maExpandedGraphicURL;
    OUString maCollapsedGraphicURL;
    bool mbIsInserted;
};

MutableTreeDataModel::MutableTreeDataModel()
: mbDisposed( false )
{
}

MutableTreeDataModel::~MutableTreeDataModel()
{
}

void MutableTreeDataModel::broadcast( broadcast_type eType, const Reference< XTreeNode >& xParentNode, const Reference< XTreeNode >* pNodes, sal_Int32 nNodes )
{
    ::cppu::OInterfaceContainerHelper* pIter = BrdcstHelper.getContainer( cppu::UnoType<XTreeDataModelListener>::get() );
    if( pIter )
    {
        Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
        const Sequence< Reference< XTreeNode > > aNodes( pNodes, nNodes );
        TreeDataModelEvent aEvent( xSource, aNodes, xParentNode );

        ::cppu::OInterfaceIteratorHelper aListIter(*pIter);
        while(aListIter.hasMoreElements())
        {
            XTreeDataModelListener* pListener = static_cast<XTreeDataModelListener*>(aListIter.next());
            switch( eType )
            {
            case nodes_changed:     pListener->treeNodesChanged(aEvent); break;
            case nodes_inserted:    pListener->treeNodesInserted(aEvent); break;
            case nodes_removed:     pListener->treeNodesRemoved(aEvent); break;
            case structure_changed: pListener->treeStructureChanged(aEvent); break;
            }
        }
    }
}

Reference< XMutableTreeNode > SAL_CALL MutableTreeDataModel::createNode( const Any& aValue, sal_Bool bChildrenOnDemand ) throw (RuntimeException, std::exception)
{
    return new MutableTreeNode( this, aValue, bChildrenOnDemand );
}

void SAL_CALL MutableTreeDataModel::setRoot( const Reference< XMutableTreeNode >& xNode ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    if( !xNode.is() )
        throw IllegalArgumentException();

    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if( xNode != mxRootNode )
    {
        if( mxRootNode.is() )
        {
            MutableTreeNodeRef xOldImpl( dynamic_cast< MutableTreeNode* >( mxRootNode.get() ) );
            if( xOldImpl.is() )
                xOldImpl->mbIsInserted = false;
        }

        MutableTreeNodeRef xImpl( dynamic_cast< MutableTreeNode* >( xNode.get() ) );
        if( !xImpl.is() || xImpl->mbIsInserted )
            throw IllegalArgumentException();

        xImpl->mbIsInserted = true;
        mxRootNode.set(xImpl.get());

        Reference< XTreeNode > xParentNode;
        broadcast( structure_changed, xParentNode, &mxRootNode, 1 );
    }
}

Reference< XTreeNode > SAL_CALL MutableTreeDataModel::getRoot(  ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return mxRootNode;
}

void SAL_CALL MutableTreeDataModel::addTreeDataModelListener( const Reference< XTreeDataModelListener >& xListener ) throw (RuntimeException, std::exception)
{
    BrdcstHelper.addListener( cppu::UnoType<XTreeDataModelListener>::get(), xListener );
}

void SAL_CALL MutableTreeDataModel::removeTreeDataModelListener( const Reference< XTreeDataModelListener >& xListener ) throw (RuntimeException, std::exception)
{
    BrdcstHelper.removeListener( cppu::UnoType<XTreeDataModelListener>::get(), xListener );
}

void SAL_CALL MutableTreeDataModel::dispose() throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( !mbDisposed )
    {
        mbDisposed = true;
        css::lang::EventObject aEvent;
        aEvent.Source.set( static_cast< ::cppu::OWeakObject* >( this ) );
        BrdcstHelper.aLC.disposeAndClear( aEvent );
    }
}

void SAL_CALL MutableTreeDataModel::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException, std::exception)
{
    BrdcstHelper.addListener( cppu::UnoType<XEventListener>::get(), xListener );
}

void SAL_CALL MutableTreeDataModel::removeEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException, std::exception)
{
    BrdcstHelper.removeListener( cppu::UnoType<XEventListener>::get(), xListener );
}

OUString SAL_CALL MutableTreeDataModel::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return OUString( "toolkit.MutableTreeDataModel" );
}

sal_Bool SAL_CALL MutableTreeDataModel::supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL MutableTreeDataModel::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    Sequence<OUString> aSeq { "com.sun.star.awt.tree.MutableTreeDataModel" };
    return aSeq;
}

MutableTreeNode::MutableTreeNode( const MutableTreeDataModelRef& xModel, const Any& rValue, bool bChildrenOnDemand )
: maDisplayValue( rValue )
, mbHasChildrenOnDemand( bChildrenOnDemand )
, mpParent( nullptr )
, mxModel( xModel )
, mbIsInserted( false )
{
}

MutableTreeNode::~MutableTreeNode()
{
    TreeNodeVector::iterator aIter( maChildren.begin() );
    while( aIter != maChildren.end() )
        (*aIter++)->setParent(nullptr);
}

void MutableTreeNode::setParent( MutableTreeNode* pParent )
{
    mpParent = pParent;
}

MutableTreeNode* MutableTreeNode::getImplementation( const Reference< XTreeNode >& xNode, bool bThrows ) throw (IllegalArgumentException)
{
    MutableTreeNode* pImpl = dynamic_cast< MutableTreeNode* >( xNode.get() );
    if( bThrows && !pImpl )
        implThrowIllegalArgumentException();

    return pImpl;
}

void MutableTreeNode::broadcast_changes()
{
    if( mxModel.is() )
    {
        Reference< XTreeNode > xParent( getReference( mpParent ) );
        Reference< XTreeNode > xNode( getReference( this ) );
        mxModel->broadcast( nodes_changed, xParent, &xNode, 1 );
    }
}

void MutableTreeNode::broadcast_changes(const Reference< XTreeNode >& xNode, bool bNew)
{
    if( mxModel.is() )
    {
        Reference< XTreeNode > xParent( getReference( this ) );
        mxModel->broadcast( bNew ? nodes_inserted : nodes_removed, xParent, &xNode, 1 );
    }
}

Any SAL_CALL MutableTreeNode::getDataValue() throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    return maDataValue;
}

void SAL_CALL MutableTreeNode::setDataValue( const Any& _datavalue ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    maDataValue = _datavalue;
}

void SAL_CALL MutableTreeNode::appendChild( const Reference< XMutableTreeNode >& xChildNode ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    Reference< XTreeNode > xNode( xChildNode.get() );
    MutableTreeNodeRef xImpl( dynamic_cast< MutableTreeNode* >( xNode.get() ) );

    if( !xImpl.is() || xImpl->mbIsInserted || (this == xImpl.get()) )
        throw IllegalArgumentException();

    maChildren.push_back( xImpl );
    xImpl->setParent(this);
    xImpl->mbIsInserted = true;

    broadcast_changes( xNode, true );
}

void SAL_CALL MutableTreeNode::insertChildByIndex( sal_Int32 nChildIndex, const Reference< XMutableTreeNode >& xChildNode ) throw (IllegalArgumentException, IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );

    if( (nChildIndex < 0) || (nChildIndex > (sal_Int32)maChildren.size()) )
        throw IndexOutOfBoundsException();

    Reference< XTreeNode > xNode( xChildNode.get() );
    MutableTreeNodeRef xImpl( dynamic_cast< MutableTreeNode* >( xNode.get() ) );
    if( !xImpl.is() || xImpl->mbIsInserted || (this == xImpl.get()) )
        throw IllegalArgumentException();

    xImpl->mbIsInserted = true;

    TreeNodeVector::iterator aIter( maChildren.begin() );
    while( (nChildIndex-- > 0) && (aIter != maChildren.end()) )
        ++aIter;

    maChildren.insert( aIter, xImpl );
    xImpl->setParent( this );

    broadcast_changes( xNode, true );
}

void SAL_CALL MutableTreeNode::removeChildByIndex( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );

    if( (nChildIndex < 0) || (nChildIndex >= (sal_Int32)maChildren.size()) )
        throw IndexOutOfBoundsException();

    MutableTreeNodeRef xImpl;

    TreeNodeVector::iterator aIter( maChildren.begin() );
    std::advance(aIter, nChildIndex);

    xImpl = (*aIter);
    maChildren.erase( aIter );

    if( !xImpl.is() )
        throw IndexOutOfBoundsException();

    xImpl->setParent(nullptr);
    xImpl->mbIsInserted = false;

    broadcast_changes( getReference( xImpl.get() ), false );
}

void SAL_CALL MutableTreeNode::setHasChildrenOnDemand( sal_Bool bChildrenOnDemand ) throw (RuntimeException, std::exception)
{
    bool bChanged;

    {
        ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
        bChanged = mbHasChildrenOnDemand != bool(bChildrenOnDemand);
        mbHasChildrenOnDemand = bChildrenOnDemand;
    }

    if( bChanged )
        broadcast_changes();
}

void SAL_CALL MutableTreeNode::setDisplayValue( const Any& aValue ) throw (RuntimeException, std::exception)
{
    {
        ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
        maDisplayValue = aValue;
    }

    broadcast_changes();
}

void SAL_CALL MutableTreeNode::setNodeGraphicURL( const OUString& rURL ) throw (RuntimeException, std::exception)
{
    bool bChanged;

    {
        ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
        bChanged = maNodeGraphicURL != rURL;
        maNodeGraphicURL = rURL;
    }

    if( bChanged )
        broadcast_changes();
}

void SAL_CALL MutableTreeNode::setExpandedGraphicURL( const OUString& rURL ) throw (RuntimeException, std::exception)
{
    bool bChanged;

    {
        ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
        bChanged = maExpandedGraphicURL != rURL;
        maExpandedGraphicURL = rURL;
    }

    if( bChanged )
        broadcast_changes();
}

void SAL_CALL MutableTreeNode::setCollapsedGraphicURL( const OUString& rURL ) throw (RuntimeException, std::exception)
{
    bool bChanged;

    {
        ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
        bChanged = maCollapsedGraphicURL != rURL;
        maCollapsedGraphicURL = rURL;
    }

    if( bChanged )
        broadcast_changes();
}

Reference< XTreeNode > SAL_CALL MutableTreeNode::getChildAt( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException,RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );

    if( (nChildIndex < 0) || (nChildIndex >= (sal_Int32)maChildren.size()) )
        throw IndexOutOfBoundsException();
    return getReference( maChildren[nChildIndex].get() );
}

sal_Int32 SAL_CALL MutableTreeNode::getChildCount(  ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    return (sal_Int32)maChildren.size();
}

Reference< XTreeNode > SAL_CALL MutableTreeNode::getParent(  ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    return getReference( mpParent );
}

sal_Int32 SAL_CALL MutableTreeNode::getIndex( const Reference< XTreeNode >& xNode ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );

    MutableTreeNodeRef xImpl( MutableTreeNode::getImplementation( xNode, false ) );
    if( xImpl.is() )
    {
        sal_Int32 nChildCount = maChildren.size();
        while( nChildCount-- )
        {
            if( maChildren[nChildCount] == xImpl )
                return nChildCount;
        }
    }

    return -1;
}

sal_Bool SAL_CALL MutableTreeNode::hasChildrenOnDemand(  ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    return mbHasChildrenOnDemand;
}

Any SAL_CALL MutableTreeNode::getDisplayValue(  ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    return maDisplayValue;
}

OUString SAL_CALL MutableTreeNode::getNodeGraphicURL(  ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    return maNodeGraphicURL;
}

OUString SAL_CALL MutableTreeNode::getExpandedGraphicURL(  ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    return maExpandedGraphicURL;
}

OUString SAL_CALL MutableTreeNode::getCollapsedGraphicURL(  ) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( maMutex );
    return maCollapsedGraphicURL;
}

OUString SAL_CALL MutableTreeNode::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return OUString( "toolkit.MutableTreeNode" );
}

sal_Bool SAL_CALL MutableTreeNode::supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL MutableTreeNode::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    Sequence<OUString> aSeq { "com.sun.star.awt.tree.MutableTreeNode" };
    return aSeq;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_MutableTreeDataModel_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new MutableTreeDataModel());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
