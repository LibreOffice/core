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


#include "treecontrol.hxx"

#include <com/sun/star/awt/tree/XTreeControl.hpp>
#include <com/sun/star/awt/tree/XTreeDataModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/property.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/implbase.hxx>

#include <helper/unopropertyarrayhelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::tree;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;

namespace toolkit
{


UnoTreeModel::UnoTreeModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory )
    :UnoControlModel( i_factory )
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
    ImplRegisterProperty( BASEPROPERTY_FILLCOLOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TREE_SELECTIONTYPE );
    ImplRegisterProperty( BASEPROPERTY_TREE_EDITABLE );
    ImplRegisterProperty( BASEPROPERTY_TREE_DATAMODEL );
    ImplRegisterProperty( BASEPROPERTY_TREE_ROOTDISPLAYED );
    ImplRegisterProperty( BASEPROPERTY_TREE_SHOWSHANDLES );
    ImplRegisterProperty( BASEPROPERTY_TREE_SHOWSROOTHANDLES );
    ImplRegisterProperty( BASEPROPERTY_ROW_HEIGHT );
    ImplRegisterProperty( BASEPROPERTY_TREE_INVOKESSTOPNODEEDITING );
    ImplRegisterProperty( BASEPROPERTY_HIDEINACTIVESELECTION );
}

rtl::Reference<UnoControlModel> UnoTreeModel::Clone() const
{
    return new UnoTreeModel( *this );
}

OUString UnoTreeModel::getServiceName()
{
    return "com.sun.star.awt.tree.TreeControlModel";
}

Any UnoTreeModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    switch( nPropId )
    {
    case BASEPROPERTY_TREE_SELECTIONTYPE:
        return Any( SelectionType_NONE );
    case BASEPROPERTY_ROW_HEIGHT:
        return Any( sal_Int32( 0 ) );
    case BASEPROPERTY_TREE_DATAMODEL:
        return Any( Reference< XTreeDataModel >( nullptr ) );
    case BASEPROPERTY_TREE_EDITABLE:
    case BASEPROPERTY_TREE_INVOKESSTOPNODEEDITING:
        return Any( false );
    case BASEPROPERTY_TREE_ROOTDISPLAYED:
    case BASEPROPERTY_TREE_SHOWSROOTHANDLES:
    case BASEPROPERTY_TREE_SHOWSHANDLES:
        return Any( true );
    case BASEPROPERTY_DEFAULTCONTROL:
        return uno::makeAny( OUString( "com.sun.star.awt.tree.TreeControl" ) );
    default:
        return UnoControlModel::ImplGetDefaultValue( nPropId );
    }
}

::cppu::IPropertyArrayHelper& UnoTreeModel::getInfoHelper()
{
    static UnoPropertyArrayHelper aHelper( ImplGetPropertyIds() );
    return aHelper;
}

// XMultiPropertySet
Reference< XPropertySetInfo > UnoTreeModel::getPropertySetInfo(  )
{
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

}

namespace {

typedef ::cppu::ImplInheritanceHelper< UnoControlBase, css::awt::tree::XTreeControl > UnoTreeControl_Base;
class UnoTreeControl : public UnoTreeControl_Base
{
public:
    UnoTreeControl();
    OUString GetComponentServiceName() override;

    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

    // css::awt::XControl
    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const css::uno::Any& xSelection ) override;
    virtual css::uno::Any SAL_CALL getSelection(  ) override;
    virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

    // css::view::XMultiSelectionSupplier
    virtual sal_Bool SAL_CALL addSelection( const css::uno::Any& Selection ) override;
    virtual void SAL_CALL removeSelection( const css::uno::Any& Selection ) override;
    virtual void SAL_CALL clearSelection(  ) override;
    virtual ::sal_Int32 SAL_CALL getSelectionCount(  ) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSelectionEnumeration(  ) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createReverseSelectionEnumeration(  ) override;

    // css::awt::XTreeControl
    virtual OUString SAL_CALL getDefaultExpandedGraphicURL() override;
    virtual void SAL_CALL setDefaultExpandedGraphicURL( const OUString& _defaultexpandedgraphicurl ) override;
    virtual OUString SAL_CALL getDefaultCollapsedGraphicURL() override;
    virtual void SAL_CALL setDefaultCollapsedGraphicURL( const OUString& _defaultcollapsedgraphicurl ) override;
    virtual sal_Bool SAL_CALL isNodeExpanded( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual sal_Bool SAL_CALL isNodeCollapsed( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void SAL_CALL makeNodeVisible( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual sal_Bool SAL_CALL isNodeVisible( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void SAL_CALL expandNode( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void SAL_CALL collapseNode( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void SAL_CALL addTreeExpansionListener( const css::uno::Reference< css::awt::tree::XTreeExpansionListener >& Listener ) override;
    virtual void SAL_CALL removeTreeExpansionListener( const css::uno::Reference< css::awt::tree::XTreeExpansionListener >& Listener ) override;
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) override;
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getClosestNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) override;
    virtual css::awt::Rectangle SAL_CALL getNodeRect( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual sal_Bool SAL_CALL isEditing(  ) override;
    virtual sal_Bool SAL_CALL stopEditing(  ) override;
    virtual void SAL_CALL cancelEditing(  ) override;
    virtual void SAL_CALL startEditingAtNode( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void SAL_CALL addTreeEditListener( const css::uno::Reference< css::awt::tree::XTreeEditListener >& Listener ) override;
    virtual void SAL_CALL removeTreeEditListener( const css::uno::Reference< css::awt::tree::XTreeEditListener >& Listener ) override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoTreeControl, UnoControlBase, "com.sun.star.awt.tree.TreeControl" )

    using UnoControl::getPeer;
private:
    TreeSelectionListenerMultiplexer maSelectionListeners;
    TreeExpansionListenerMultiplexer maTreeExpansionListeners;
    TreeEditListenerMultiplexer maTreeEditListeners;
};

UnoTreeControl::UnoTreeControl()
: maSelectionListeners( *this )
, maTreeExpansionListeners( *this )
, maTreeEditListeners( *this )
{
}

OUString UnoTreeControl::GetComponentServiceName()
{
    return "Tree";
}


// css::view::XSelectionSupplier


sal_Bool SAL_CALL UnoTreeControl::select( const Any& rSelection )
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->select( rSelection );
}


Any SAL_CALL UnoTreeControl::getSelection()
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getSelection();
}


void SAL_CALL UnoTreeControl::addSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener )
{
    maSelectionListeners.addInterface( xListener );
    if( getPeer().is() && (maSelectionListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // add it to the peer if this is the first listener added to that proxy
        Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->addSelectionChangeListener(&maSelectionListeners);
    }
}


void SAL_CALL UnoTreeControl::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener )
{
    if( getPeer().is() && (maSelectionListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // remove it from the peer if this is the last listener removed from that proxy
        Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->removeSelectionChangeListener(&maSelectionListeners);
    }
    maSelectionListeners.removeInterface( xListener );
}


// css::view::XMultiSelectionSupplier


sal_Bool SAL_CALL UnoTreeControl::addSelection( const Any& rSelection )
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->addSelection(rSelection);
}


void SAL_CALL UnoTreeControl::removeSelection( const Any& rSelection )
{
    Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->removeSelection(rSelection);
}


void SAL_CALL UnoTreeControl::clearSelection()
{
    Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->clearSelection();
}


sal_Int32 SAL_CALL UnoTreeControl::getSelectionCount()
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getSelectionCount();
}


Reference< XEnumeration > SAL_CALL UnoTreeControl::createSelectionEnumeration()
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->createSelectionEnumeration();
}


Reference< XEnumeration > SAL_CALL UnoTreeControl::createReverseSelectionEnumeration()
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->createReverseSelectionEnumeration();
}


// XTreeControl


OUString SAL_CALL UnoTreeControl::getDefaultExpandedGraphicURL()
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getDefaultExpandedGraphicURL();
}


void SAL_CALL UnoTreeControl::setDefaultExpandedGraphicURL( const OUString& _defaultexpansiongraphicurl )
{
    Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->setDefaultExpandedGraphicURL(_defaultexpansiongraphicurl);
}


OUString SAL_CALL UnoTreeControl::getDefaultCollapsedGraphicURL()
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getDefaultCollapsedGraphicURL();
}


void SAL_CALL UnoTreeControl::setDefaultCollapsedGraphicURL( const OUString& _defaultcollapsedgraphicurl )
{
    Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->setDefaultCollapsedGraphicURL(_defaultcollapsedgraphicurl);
}


sal_Bool SAL_CALL UnoTreeControl::isNodeExpanded( const Reference< XTreeNode >& xNode )
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->isNodeExpanded(xNode);
}


sal_Bool SAL_CALL UnoTreeControl::isNodeCollapsed( const Reference< XTreeNode >& xNode )
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->isNodeCollapsed(xNode);
}


void SAL_CALL UnoTreeControl::makeNodeVisible( const Reference< XTreeNode >& xNode )
{
    Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->makeNodeVisible(xNode);
}


sal_Bool SAL_CALL UnoTreeControl::isNodeVisible( const Reference< XTreeNode >& xNode )
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->isNodeVisible(xNode);
}


void SAL_CALL UnoTreeControl::expandNode( const Reference< XTreeNode >& xNode )
{
    Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->expandNode(xNode);
}


void SAL_CALL UnoTreeControl::collapseNode( const Reference< XTreeNode >& xNode )
{
    Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->collapseNode(xNode);
}


void SAL_CALL UnoTreeControl::addTreeExpansionListener( const Reference< XTreeExpansionListener >& xListener )
{
    maTreeExpansionListeners.addInterface( xListener );
    if( getPeer().is() && (maTreeExpansionListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // add it to the peer if this is the first listener added to that proxy
        Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->addTreeExpansionListener(&maTreeExpansionListeners);
    }
}


void SAL_CALL UnoTreeControl::removeTreeExpansionListener( const Reference< XTreeExpansionListener >& xListener )
{
    if( getPeer().is() && (maTreeExpansionListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // remove it from the peer if this is the last listener removed from that proxy
        Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->removeTreeExpansionListener(&maTreeExpansionListeners);
    }
    maTreeExpansionListeners.removeInterface( xListener );
}


Reference< XTreeNode > SAL_CALL UnoTreeControl::getNodeForLocation( sal_Int32 x, sal_Int32 y )
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getNodeForLocation(x,y);
}


Reference< XTreeNode > SAL_CALL UnoTreeControl::getClosestNodeForLocation( sal_Int32 x, sal_Int32 y )
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getClosestNodeForLocation(x,y);
}


awt::Rectangle SAL_CALL UnoTreeControl::getNodeRect( const Reference< XTreeNode >& Node )
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getNodeRect( Node );
}


sal_Bool SAL_CALL UnoTreeControl::isEditing(  )
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->isEditing();
}


sal_Bool SAL_CALL UnoTreeControl::stopEditing()
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->stopEditing();
}


void SAL_CALL UnoTreeControl::cancelEditing()
{
    Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->cancelEditing();
}


void SAL_CALL UnoTreeControl::startEditingAtNode( const Reference< XTreeNode >& xNode )
{
    Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->startEditingAtNode(xNode);
}


void SAL_CALL UnoTreeControl::addTreeEditListener( const Reference< XTreeEditListener >& xListener )
{
    maTreeEditListeners.addInterface( xListener );
    if( getPeer().is() && (maTreeEditListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // add it to the peer if this is the first listener added to that proxy
        Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->addTreeEditListener(&maTreeEditListeners);
    }
}


void SAL_CALL UnoTreeControl::removeTreeEditListener( const Reference< XTreeEditListener >& xListener )
{
    if( getPeer().is() && (maTreeEditListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // remove it from the peer if this is the last listener removed from that proxy
        Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->removeTreeEditListener(&maTreeEditListeners);
    }
    maTreeEditListeners.removeInterface( xListener );
}


// XComponent


void SAL_CALL UnoTreeControl::dispose(  )
{
    lang::EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >(this);
    maSelectionListeners.disposeAndClear( aEvt );
    maTreeExpansionListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoTreeControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer )
{
    UnoControlBase::createPeer( rxToolkit, rParentPeer );

    Reference< XTreeControl >  xTree( getPeer(), UNO_QUERY_THROW );
    if( maSelectionListeners.getLength() )
        xTree->addSelectionChangeListener( &maSelectionListeners );
    if( maTreeExpansionListeners.getLength() )
        xTree->addTreeExpansionListener( &maTreeExpansionListeners );
}

}

void SAL_CALL TreeEditListenerMultiplexer::nodeEditing( const Reference< XTreeNode >& Node )
{
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while( aIt.hasMoreElements() )
    {
        Reference<XTreeEditListener> xListener(aIt.next());
        try
        {
            xListener->nodeEditing( Node );
        }
        catch( const DisposedException& e )
        {
            OSL_ENSURE( e.Context.is(), "caught DisposedException with empty Context field" );
            if ( e.Context == xListener || !e.Context.is() )
                aIt.remove();
        }
        catch( const RuntimeException& )
        {
            DISPLAY_EXCEPTION( TreeEditListenerMultiplexer, nodeEditing )
        }
    }
}

void SAL_CALL TreeEditListenerMultiplexer::nodeEdited( const Reference< XTreeNode >& Node, const OUString& NewText )
{
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while( aIt.hasMoreElements() )
    {
        Reference<XTreeEditListener> xListener(aIt.next());
        try
        {
            xListener->nodeEdited( Node, NewText );
        }
        catch( const DisposedException& e )
        {
            OSL_ENSURE( e.Context.is(), "caught DisposedException with empty Context field" );
            if ( e.Context == xListener || !e.Context.is() )
                aIt.remove();
        }
        catch( const RuntimeException& )
        {
            DISPLAY_EXCEPTION( TreeEditListenerMultiplexer, nodeEdited )
        }
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_TreeControlModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::UnoTreeModel(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_TreeControl_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoTreeControl());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
