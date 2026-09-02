/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
using namespace cpo::uno;
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
    virtual css::uno::Reference< css::awt::tree::XMutableTreeNode > createNode( const cpo::uno::Any& DisplayValue, bool ChildrenOnDemand ) override;
    virtual void setRoot( const css::uno::Reference< css::awt::tree::XMutableTreeNode >& RootNode ) override;

    // XTreeDataModel
    virtual css::uno::Reference< css::awt::tree::XTreeNode > getRoot(  ) override;
    virtual void addTreeDataModelListener( const css::uno::Reference< css::awt::tree::XTreeDataModelListener >& Listener ) override;
    virtual void removeTreeDataModelListener( const css::uno::Reference< css::awt::tree::XTreeDataModelListener >& Listener ) override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const Reference< XEventListener >& xListener ) override;
    virtual void removeEventListener( const Reference< XEventListener >& aListener ) override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > getSupportedServiceNames(  ) override;

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
    virtual cpo::uno::Any getDataValue() override;
    virtual void setDataValue( const cpo::uno::Any& _datavalue ) override;
    virtual void appendChild( const css::uno::Reference< css::awt::tree::XMutableTreeNode >& ChildNode ) override;
    virtual void insertChildByIndex( ::sal_Int32 Index, const css::uno::Reference< css::awt::tree::XMutableTreeNode >& ChildNode ) override;
    virtual void removeChildByIndex( ::sal_Int32 Index ) override;
    virtual void setHasChildrenOnDemand( bool ChildrenOnDemand ) override;
    virtual void setDisplayValue( const cpo::uno::Any& Value ) override;
    virtual void setNodeGraphicURL( const OUString& URL ) override;
    virtual void setExpandedGraphicURL( const OUString& URL ) override;
    virtual void setCollapsedGraphicURL( const OUString& URL ) override;

    // XTreeNode
    virtual css::uno::Reference< css::awt::tree::XTreeNode > getChildAt( ::sal_Int32 Index ) override;
    virtual ::sal_Int32 getChildCount(  ) override;
    virtual css::uno::Reference< css::awt::tree::XTreeNode > getParent(  ) override;
    virtual ::sal_Int32 getIndex( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual bool hasChildrenOnDemand(  ) override;
    virtual cpo::uno::Any getDisplayValue(  ) override;
    virtual OUString getNodeGraphicURL(  ) override;
    virtual OUString getExpandedGraphicURL(  ) override;
    virtual OUString getCollapsedGraphicURL(  ) override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > getSupportedServiceNames(  ) override;

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

Reference< XMutableTreeNode > MutableTreeDataModel::createNode( const Any& aValue, bool bChildrenOnDemand )
{
    return new MutableTreeNode( this, aValue, bChildrenOnDemand );
}

void MutableTreeDataModel::setRoot( const Reference< XMutableTreeNode >& xNode )
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
    mxRootNode = std::move(xImpl);

    Reference< XTreeNode > xParentNode;
    broadcastImpl( aGuard, structure_changed, xParentNode, mxRootNode );
}

Reference< XTreeNode > MutableTreeDataModel::getRoot(  )
{
    std::unique_lock aGuard( m_aMutex );
    return mxRootNode;
}

void MutableTreeDataModel::addTreeDataModelListener( const Reference< XTreeDataModelListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maTreeDataModelListeners.addInterface( aGuard, xListener );
}

void MutableTreeDataModel::removeTreeDataModelListener( const Reference< XTreeDataModelListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maTreeDataModelListeners.removeInterface( aGuard, xListener );
}

void MutableTreeDataModel::dispose()
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

void MutableTreeDataModel::addEventListener( const Reference< XEventListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maEventListeners.addInterface( aGuard, xListener );
}

void MutableTreeDataModel::removeEventListener( const Reference< XEventListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maEventListeners.removeInterface( aGuard, xListener );
}

OUString MutableTreeDataModel::getImplementationName(  )
{
    return u"toolkit.MutableTreeDataModel"_ustr;
}

bool MutableTreeDataModel::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > MutableTreeDataModel::getSupportedServiceNames(  )
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

Any MutableTreeNode::getDataValue()
{
    std::scoped_lock aGuard( maMutex );
    return maDataValue;
}

void MutableTreeNode::setDataValue( const Any& _datavalue )
{
    std::scoped_lock aGuard( maMutex );
    maDataValue = _datavalue;
}

void MutableTreeNode::appendChild( const Reference< XMutableTreeNode >& xChildNode )
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

void MutableTreeNode::insertChildByIndex( sal_Int32 nChildIndex, const Reference< XMutableTreeNode >& xChildNode )
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

void MutableTreeNode::removeChildByIndex( sal_Int32 nChildIndex )
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

void MutableTreeNode::setHasChildrenOnDemand( bool bChildrenOnDemand )
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

void MutableTreeNode::setDisplayValue( const Any& aValue )
{
    {
        std::scoped_lock aGuard( maMutex );
        maDisplayValue = aValue;
    }

    broadcast_changes();
}

void MutableTreeNode::setNodeGraphicURL( const OUString& rURL )
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

void MutableTreeNode::setExpandedGraphicURL( const OUString& rURL )
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

void MutableTreeNode::setCollapsedGraphicURL( const OUString& rURL )
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

Reference< XTreeNode > MutableTreeNode::getChildAt( sal_Int32 nChildIndex )
{
    std::scoped_lock aGuard( maMutex );

    if( (nChildIndex < 0) || (o3tl::make_unsigned(nChildIndex) >= maChildren.size()) )
        throw IndexOutOfBoundsException();
    return maChildren[nChildIndex];
}

sal_Int32 MutableTreeNode::getChildCount(  )
{
    std::scoped_lock aGuard( maMutex );
    return static_cast<sal_Int32>(maChildren.size());
}

Reference< XTreeNode > MutableTreeNode::getParent(  )
{
    std::scoped_lock aGuard( maMutex );
    return mpParent;
}

sal_Int32 MutableTreeNode::getIndex( const Reference< XTreeNode >& xNode )
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

bool MutableTreeNode::hasChildrenOnDemand(  )
{
    std::scoped_lock aGuard( maMutex );
    return mbHasChildrenOnDemand;
}

Any MutableTreeNode::getDisplayValue(  )
{
    std::scoped_lock aGuard( maMutex );
    return maDisplayValue;
}

OUString MutableTreeNode::getNodeGraphicURL(  )
{
    std::scoped_lock aGuard( maMutex );
    return maNodeGraphicURL;
}

OUString MutableTreeNode::getExpandedGraphicURL(  )
{
    std::scoped_lock aGuard( maMutex );
    return maExpandedGraphicURL;
}

OUString MutableTreeNode::getCollapsedGraphicURL(  )
{
    std::scoped_lock aGuard( maMutex );
    return maCollapsedGraphicURL;
}

OUString MutableTreeNode::getImplementationName(  )
{
    return u"toolkit.MutableTreeNode"_ustr;
}

bool MutableTreeNode::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > MutableTreeNode::getSupportedServiceNames(  )
{
    Sequence<OUString> aSeq { u"com.sun.star.awt.tree.MutableTreeNode"_ustr };
    return aSeq;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_MutableTreeDataModel_get_implementation(
    css::uno::XComponentContext *,
    cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new MutableTreeDataModel());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
