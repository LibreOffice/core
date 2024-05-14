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
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/ref.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <mutex>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::tree;
using namespace ::com::sun::star::lang;

namespace {

    enum broadcast_type { nodes_changed, nodes_inserted, nodes_removed, structure_changed };

class MutableTreeNode;
class MutableTreeDataModel;

typedef std::vector< rtl::Reference< MutableTreeNode > > TreeNodeVector;

class MutableTreeDataModel : public ::cppu::WeakImplHelper< XMutableTreeDataModel, XServiceInfo >
{
public:
    MutableTreeDataModel();

    void broadcast( broadcast_type eType, const Reference< XTreeNode >& xParentNode, const Reference< XTreeNode >& rNode );

    // XMutableTreeDataModel
    virtual css::uno::Reference< css::awt::tree::XMutableTreeNode > SAL_CALL createNode( const css::uno::Any& DisplayValue, sal_Bool ChildrenOnDemand ) override;
    virtual void SAL_CALL setRoot( const css::uno::Reference< css::awt::tree::XMutableTreeNode >& RootNode ) override;

    // XTreeDataModel
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getRoot(  ) override;
    virtual void SAL_CALL addTreeDataModelListener( const css::uno::Reference< css::awt::tree::XTreeDataModelListener >& Listener ) override;
    virtual void SAL_CALL removeTreeDataModelListener( const css::uno::Reference< css::awt::tree::XTreeDataModelListener >& Listener ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

private:
    void broadcastImpl( std::unique_lock<std::mutex>& rGuard, broadcast_type eType, const Reference< XTreeNode >& xParentNode, const Reference< XTreeNode >& rNode );

    std::mutex m_aMutex;
    comphelper::OInterfaceContainerHelper4<XTreeDataModelListener> maTreeDataModelListeners;
    comphelper::OInterfaceContainerHelper4<XEventListener> maEventListeners;
    bool mbDisposed;
    rtl::Reference< MutableTreeNode > mxRootNode;
};

class MutableTreeNode: public ::cppu::WeakImplHelper< XMutableTreeNode, XServiceInfo >
{
    friend class MutableTreeDataModel;

public:
    MutableTreeNode( rtl::Reference< MutableTreeDataModel > xModel, Any aValue, bool bChildrenOnDemand );
    virtual ~MutableTreeNode() override;

    void setParent( MutableTreeNode* pParent );
    void broadcast_changes();
    void broadcast_changes(std::unique_lock<std::mutex> & rLock,
            const Reference< XTreeNode >& xNode, bool bNew);

    // XMutableTreeNode
    virtual css::uno::Any SAL_CALL getDataValue() override;
    virtual void SAL_CALL setDataValue( const css::uno::Any& _datavalue ) override;
    virtual void SAL_CALL appendChild( const css::uno::Reference< css::awt::tree::XMutableTreeNode >& ChildNode ) override;
    virtual void SAL_CALL insertChildByIndex( ::sal_Int32 Index, const css::uno::Reference< css::awt::tree::XMutableTreeNode >& ChildNode ) override;
    virtual void SAL_CALL removeChildByIndex( ::sal_Int32 Index ) override;
    virtual void SAL_CALL setHasChildrenOnDemand( sal_Bool ChildrenOnDemand ) override;
    virtual void SAL_CALL setDisplayValue( const css::uno::Any& Value ) override;
    virtual void SAL_CALL setNodeGraphicURL( const OUString& URL ) override;
    virtual void SAL_CALL setExpandedGraphicURL( const OUString& URL ) override;
    virtual void SAL_CALL setCollapsedGraphicURL( const OUString& URL ) override;

    // XTreeNode
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getChildAt( ::sal_Int32 Index ) override;
    virtual ::sal_Int32 SAL_CALL getChildCount(  ) override;
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getParent(  ) override;
    virtual ::sal_Int32 SAL_CALL getIndex( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual sal_Bool SAL_CALL hasChildrenOnDemand(  ) override;
    virtual css::uno::Any SAL_CALL getDisplayValue(  ) override;
    virtual OUString SAL_CALL getNodeGraphicURL(  ) override;
    virtual OUString SAL_CALL getExpandedGraphicURL(  ) override;
    virtual OUString SAL_CALL getCollapsedGraphicURL(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

private:
    TreeNodeVector  maChildren;
    Any maDisplayValue;
    Any maDataValue;
    bool mbHasChildrenOnDemand;
    std::mutex maMutex;
    MutableTreeNode* mpParent;
    rtl::Reference< MutableTreeDataModel > mxModel;
    OUString maNodeGraphicURL;
    OUString maExpandedGraphicURL;
    OUString maCollapsedGraphicURL;
    bool mbIsInserted;
};

MutableTreeDataModel::MutableTreeDataModel()
: mbDisposed( false )
{
}

void MutableTreeDataModel::broadcast( broadcast_type eType, const Reference< XTreeNode >& xParentNode, const Reference< XTreeNode >& rNode )
{
    std::unique_lock aGuard(m_aMutex);
    broadcastImpl(aGuard, eType, xParentNode, rNode);
}

void MutableTreeDataModel::broadcastImpl( std::unique_lock<std::mutex>& rGuard, broadcast_type eType, const Reference< XTreeNode >& xParentNode, const Reference< XTreeNode >& rNode )
{
    if( !maTreeDataModelListeners.getLength(rGuard) )
        return;

    Reference< XInterface > xSource( getXWeak() );
    const Sequence< Reference< XTreeNode > > aNodes { rNode };
    TreeDataModelEvent aEvent( xSource, aNodes, xParentNode );

    comphelper::OInterfaceIteratorHelper4 aListIter(rGuard, maTreeDataModelListeners);
    rGuard.unlock();
    while(aListIter.hasMoreElements())
    {
        XTreeDataModelListener* pListener = aListIter.next().get();
        switch( eType )
        {
        case nodes_changed:     pListener->treeNodesChanged(aEvent); break;
        case nodes_inserted:    pListener->treeNodesInserted(aEvent); break;
        case nodes_removed:     pListener->treeNodesRemoved(aEvent); break;
        case structure_changed: pListener->treeStructureChanged(aEvent); break;
        }
    }
}

Reference< XMutableTreeNode > SAL_CALL MutableTreeDataModel::createNode( const Any& aValue, sal_Bool bChildrenOnDemand )
{
    return new MutableTreeNode( this, aValue, bChildrenOnDemand );
}

void SAL_CALL MutableTreeDataModel::setRoot( const Reference< XMutableTreeNode >& xNode )
{
    if( !xNode.is() )
        throw IllegalArgumentException();

    std::unique_lock aGuard( m_aMutex );
    if( xNode.get() == mxRootNode.get() )
        return;

    if( mxRootNode.is() )
        mxRootNode->mbIsInserted = false;

    rtl::Reference< MutableTreeNode > xImpl( dynamic_cast< MutableTreeNode* >( xNode.get() ) );
    if( !xImpl.is() || xImpl->mbIsInserted )
        throw IllegalArgumentException();

    xImpl->mbIsInserted = true;
    mxRootNode = xImpl;

    Reference< XTreeNode > xParentNode;
    broadcastImpl( aGuard, structure_changed, xParentNode, mxRootNode );
}

Reference< XTreeNode > SAL_CALL MutableTreeDataModel::getRoot(  )
{
    std::unique_lock aGuard( m_aMutex );
    return mxRootNode;
}

void SAL_CALL MutableTreeDataModel::addTreeDataModelListener( const Reference< XTreeDataModelListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maTreeDataModelListeners.addInterface( aGuard, xListener );
}

void SAL_CALL MutableTreeDataModel::removeTreeDataModelListener( const Reference< XTreeDataModelListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maTreeDataModelListeners.removeInterface( aGuard, xListener );
}

void SAL_CALL MutableTreeDataModel::dispose()
{
    std::unique_lock aGuard( m_aMutex );

    if( !mbDisposed )
    {
        mbDisposed = true;
        css::lang::EventObject aEvent;
        aEvent.Source.set( getXWeak() );
        maTreeDataModelListeners.disposeAndClear( aGuard, aEvent );
        maEventListeners.disposeAndClear( aGuard, aEvent );
    }
}

void SAL_CALL MutableTreeDataModel::addEventListener( const Reference< XEventListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maEventListeners.addInterface( aGuard, xListener );
}

void SAL_CALL MutableTreeDataModel::removeEventListener( const Reference< XEventListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maEventListeners.removeInterface( aGuard, xListener );
}

OUString SAL_CALL MutableTreeDataModel::getImplementationName(  )
{
    return u"toolkit.MutableTreeDataModel"_ustr;
}

sal_Bool SAL_CALL MutableTreeDataModel::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL MutableTreeDataModel::getSupportedServiceNames(  )
{
    Sequence<OUString> aSeq { u"com.sun.star.awt.tree.MutableTreeDataModel"_ustr };
    return aSeq;
}

MutableTreeNode::MutableTreeNode( rtl::Reference< MutableTreeDataModel > xModel, Any aValue, bool bChildrenOnDemand )
: maDisplayValue(std::move( aValue ))
, mbHasChildrenOnDemand( bChildrenOnDemand )
, mpParent( nullptr )
, mxModel(std::move( xModel ))
, mbIsInserted( false )
{
}

MutableTreeNode::~MutableTreeNode()
{
    for( auto& rChild : maChildren )
        rChild->setParent(nullptr);
}

void MutableTreeNode::setParent( MutableTreeNode* pParent )
{
    mpParent = pParent;
}

void MutableTreeNode::broadcast_changes()
{
    if( mxModel.is() )
    {
        mxModel->broadcast( nodes_changed, mpParent, this );
    }
}

void MutableTreeNode::broadcast_changes(std::unique_lock<std::mutex> & rLock,
        const Reference< XTreeNode >& xNode, bool const bNew)
{
    auto const xModel(mxModel);
    rLock.unlock();
    if (xModel.is())
    {
        xModel->broadcast(bNew ? nodes_inserted : nodes_removed, this, xNode);
    }
}

Any SAL_CALL MutableTreeNode::getDataValue()
{
    std::scoped_lock aGuard( maMutex );
    return maDataValue;
}

void SAL_CALL MutableTreeNode::setDataValue( const Any& _datavalue )
{
    std::scoped_lock aGuard( maMutex );
    maDataValue = _datavalue;
}

void SAL_CALL MutableTreeNode::appendChild( const Reference< XMutableTreeNode >& xChildNode )
{
    std::unique_lock aGuard( maMutex );
    rtl::Reference< MutableTreeNode > xImpl( dynamic_cast< MutableTreeNode* >( xChildNode.get() ) );

    if( !xImpl.is() || xImpl->mbIsInserted || (this == xImpl.get()) )
        throw IllegalArgumentException();

    maChildren.push_back( xImpl );
    xImpl->setParent(this);
    xImpl->mbIsInserted = true;

    broadcast_changes(aGuard, xChildNode, true);
}

void SAL_CALL MutableTreeNode::insertChildByIndex( sal_Int32 nChildIndex, const Reference< XMutableTreeNode >& xChildNode )
{
    std::unique_lock aGuard( maMutex );

    if( (nChildIndex < 0) || (o3tl::make_unsigned(nChildIndex) > maChildren.size()) )
        throw IndexOutOfBoundsException();

    rtl::Reference< MutableTreeNode > xImpl( dynamic_cast< MutableTreeNode* >( xChildNode.get() ) );
    if( !xImpl.is() || xImpl->mbIsInserted || (this == xImpl.get()) )
        throw IllegalArgumentException();

    xImpl->mbIsInserted = true;

    TreeNodeVector::iterator aIter( maChildren.begin() );
    std::advance(aIter, nChildIndex);

    maChildren.insert( aIter, xImpl );
    xImpl->setParent( this );

    broadcast_changes(aGuard, xChildNode, true);
}

void SAL_CALL MutableTreeNode::removeChildByIndex( sal_Int32 nChildIndex )
{
    std::unique_lock aGuard( maMutex );

    if( (nChildIndex < 0) || (o3tl::make_unsigned(nChildIndex) >= maChildren.size()) )
        throw IndexOutOfBoundsException();

    rtl::Reference< MutableTreeNode > xImpl;

    TreeNodeVector::iterator aIter( maChildren.begin() );
    std::advance(aIter, nChildIndex);

    xImpl = *aIter;
    maChildren.erase( aIter );

    if( !xImpl.is() )
        throw IndexOutOfBoundsException();

    xImpl->setParent(nullptr);
    xImpl->mbIsInserted = false;

    broadcast_changes(aGuard, xImpl, false);
}

void SAL_CALL MutableTreeNode::setHasChildrenOnDemand( sal_Bool bChildrenOnDemand )
{
    bool bChanged;

    {
        std::scoped_lock aGuard( maMutex );
        bChanged = mbHasChildrenOnDemand != bool(bChildrenOnDemand);
        mbHasChildrenOnDemand = bChildrenOnDemand;
    }

    if( bChanged )
        broadcast_changes();
}

void SAL_CALL MutableTreeNode::setDisplayValue( const Any& aValue )
{
    {
        std::scoped_lock aGuard( maMutex );
        maDisplayValue = aValue;
    }

    broadcast_changes();
}

void SAL_CALL MutableTreeNode::setNodeGraphicURL( const OUString& rURL )
{
    bool bChanged;

    {
        std::scoped_lock aGuard( maMutex );
        bChanged = maNodeGraphicURL != rURL;
        maNodeGraphicURL = rURL;
    }

    if( bChanged )
        broadcast_changes();
}

void SAL_CALL MutableTreeNode::setExpandedGraphicURL( const OUString& rURL )
{
    bool bChanged;

    {
        std::scoped_lock aGuard( maMutex );
        bChanged = maExpandedGraphicURL != rURL;
        maExpandedGraphicURL = rURL;
    }

    if( bChanged )
        broadcast_changes();
}

void SAL_CALL MutableTreeNode::setCollapsedGraphicURL( const OUString& rURL )
{
    bool bChanged;

    {
        std::scoped_lock aGuard( maMutex );
        bChanged = maCollapsedGraphicURL != rURL;
        maCollapsedGraphicURL = rURL;
    }

    if( bChanged )
        broadcast_changes();
}

Reference< XTreeNode > SAL_CALL MutableTreeNode::getChildAt( sal_Int32 nChildIndex )
{
    std::scoped_lock aGuard( maMutex );

    if( (nChildIndex < 0) || (o3tl::make_unsigned(nChildIndex) >= maChildren.size()) )
        throw IndexOutOfBoundsException();
    return maChildren[nChildIndex];
}

sal_Int32 SAL_CALL MutableTreeNode::getChildCount(  )
{
    std::scoped_lock aGuard( maMutex );
    return static_cast<sal_Int32>(maChildren.size());
}

Reference< XTreeNode > SAL_CALL MutableTreeNode::getParent(  )
{
    std::scoped_lock aGuard( maMutex );
    return mpParent;
}

sal_Int32 SAL_CALL MutableTreeNode::getIndex( const Reference< XTreeNode >& xNode )
{
    std::scoped_lock aGuard( maMutex );

    rtl::Reference< MutableTreeNode > xImpl( dynamic_cast< MutableTreeNode* >( xNode.get() ) );
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

sal_Bool SAL_CALL MutableTreeNode::hasChildrenOnDemand(  )
{
    std::scoped_lock aGuard( maMutex );
    return mbHasChildrenOnDemand;
}

Any SAL_CALL MutableTreeNode::getDisplayValue(  )
{
    std::scoped_lock aGuard( maMutex );
    return maDisplayValue;
}

OUString SAL_CALL MutableTreeNode::getNodeGraphicURL(  )
{
    std::scoped_lock aGuard( maMutex );
    return maNodeGraphicURL;
}

OUString SAL_CALL MutableTreeNode::getExpandedGraphicURL(  )
{
    std::scoped_lock aGuard( maMutex );
    return maExpandedGraphicURL;
}

OUString SAL_CALL MutableTreeNode::getCollapsedGraphicURL(  )
{
    std::scoped_lock aGuard( maMutex );
    return maCollapsedGraphicURL;
}

OUString SAL_CALL MutableTreeNode::getImplementationName(  )
{
    return u"toolkit.MutableTreeNode"_ustr;
}

sal_Bool SAL_CALL MutableTreeNode::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL MutableTreeNode::getSupportedServiceNames(  )
{
    Sequence<OUString> aSeq { u"com.sun.star.awt.tree.MutableTreeNode"_ustr };
    return aSeq;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_MutableTreeDataModel_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new MutableTreeDataModel());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
