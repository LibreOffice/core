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


#include <treecontrol.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/awt/tree/XTreeDataModel.hpp>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/property.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt::tree;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;

namespace toolkit
{
//  ----------------------------------------------------
//  class UnoTreeModel
//  ----------------------------------------------------
UnoTreeModel::UnoTreeModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
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

UnoTreeModel::UnoTreeModel( const UnoTreeModel& rModel )
: UnoControlModel( rModel )
{
}

UnoControlModel* UnoTreeModel::Clone() const
{
    return new UnoTreeModel( *this );
}

OUString UnoTreeModel::getServiceName() throw(RuntimeException)
{
    return OUString::createFromAscii( szServiceName_TreeControlModel );
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
        return Any( Reference< XTreeDataModel >( 0 ) );
    case BASEPROPERTY_TREE_EDITABLE:
    case BASEPROPERTY_TREE_INVOKESSTOPNODEEDITING:
        return Any( sal_False );
    case BASEPROPERTY_TREE_ROOTDISPLAYED:
    case BASEPROPERTY_TREE_SHOWSROOTHANDLES:
    case BASEPROPERTY_TREE_SHOWSHANDLES:
        return Any( sal_True );
    case BASEPROPERTY_DEFAULTCONTROL:
        return uno::makeAny( ::rtl::OUString::createFromAscii( szServiceName_TreeControl ) );
    default:
        return UnoControlModel::ImplGetDefaultValue( nPropId );
    }
}

::cppu::IPropertyArrayHelper& UnoTreeModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// XMultiPropertySet
Reference< XPropertySetInfo > UnoTreeModel::getPropertySetInfo(  ) throw(RuntimeException)
{
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoTreeControl
//  ----------------------------------------------------
UnoTreeControl::UnoTreeControl( const Reference< XMultiServiceFactory >& i_factory )
: UnoTreeControl_Base( i_factory )
, maSelectionListeners( *this )
, maTreeExpansionListeners( *this )
, maTreeEditListeners( *this )
{
}

OUString UnoTreeControl::GetComponentServiceName()
{
    return OUString("Tree");
}

// -------------------------------------------------------------------
// ::com::sun::star::view::XSelectionSupplier
// -------------------------------------------------------------------

sal_Bool SAL_CALL UnoTreeControl::select( const Any& rSelection ) throw (IllegalArgumentException, RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->select( rSelection );
}

// -------------------------------------------------------------------

Any SAL_CALL UnoTreeControl::getSelection() throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getSelection();
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::addSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener ) throw (RuntimeException)
{
    maSelectionListeners.addInterface( xListener );
    if( getPeer().is() && (maSelectionListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // add it to the peer if this is the first listener added to that proxy
        Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
        Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->addSelectionChangeListener(&maSelectionListeners);
    }
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener ) throw (RuntimeException)
{
    if( getPeer().is() && (maSelectionListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // remove it from the peer if this is the last listener removed from that proxy
        Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
        Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->removeSelectionChangeListener(&maSelectionListeners);
    }
    maSelectionListeners.removeInterface( xListener );
}

// -------------------------------------------------------------------
// ::com::sun::star::view::XMultiSelectionSupplier
// -------------------------------------------------------------------

sal_Bool SAL_CALL UnoTreeControl::addSelection( const Any& rSelection ) throw (IllegalArgumentException, RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->addSelection(rSelection);
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::removeSelection( const Any& rSelection ) throw (IllegalArgumentException, RuntimeException)
{
    Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
    Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->removeSelection(rSelection);
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::clearSelection() throw (RuntimeException)
{
    Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
    Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->clearSelection();
}

// -------------------------------------------------------------------

sal_Int32 SAL_CALL UnoTreeControl::getSelectionCount() throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getSelectionCount();
}

// -------------------------------------------------------------------

Reference< XEnumeration > SAL_CALL UnoTreeControl::createSelectionEnumeration() throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->createSelectionEnumeration();
}

// -------------------------------------------------------------------

Reference< XEnumeration > SAL_CALL UnoTreeControl::createReverseSelectionEnumeration() throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->createReverseSelectionEnumeration();
}

// --------------------------------------------------------------------
// XTreeControl
// --------------------------------------------------------------------

OUString SAL_CALL UnoTreeControl::getDefaultExpandedGraphicURL() throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getDefaultExpandedGraphicURL();
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::setDefaultExpandedGraphicURL( const OUString& _defaultexpansiongraphicurl ) throw (RuntimeException)
{
    Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
    Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->setDefaultExpandedGraphicURL(_defaultexpansiongraphicurl);
}

// -------------------------------------------------------------------

OUString SAL_CALL UnoTreeControl::getDefaultCollapsedGraphicURL() throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getDefaultCollapsedGraphicURL();
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::setDefaultCollapsedGraphicURL( const OUString& _defaultcollapsedgraphicurl ) throw (RuntimeException)
{
    Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
    Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->setDefaultCollapsedGraphicURL(_defaultcollapsedgraphicurl);
}

// -------------------------------------------------------------------

sal_Bool SAL_CALL UnoTreeControl::isNodeExpanded( const Reference< XTreeNode >& xNode ) throw (RuntimeException, IllegalArgumentException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->isNodeExpanded(xNode);
}

// -------------------------------------------------------------------

sal_Bool SAL_CALL UnoTreeControl::isNodeCollapsed( const Reference< XTreeNode >& xNode ) throw (RuntimeException, IllegalArgumentException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->isNodeCollapsed(xNode);
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::makeNodeVisible( const Reference< XTreeNode >& xNode ) throw (RuntimeException, ExpandVetoException, IllegalArgumentException)
{
    Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
    Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->makeNodeVisible(xNode);
}

// -------------------------------------------------------------------

sal_Bool SAL_CALL UnoTreeControl::isNodeVisible( const Reference< XTreeNode >& xNode ) throw (RuntimeException, IllegalArgumentException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->isNodeVisible(xNode);
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::expandNode( const Reference< XTreeNode >& xNode ) throw (RuntimeException, ExpandVetoException, IllegalArgumentException)
{
    Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
    Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->expandNode(xNode);
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::collapseNode( const Reference< XTreeNode >& xNode ) throw (RuntimeException, ExpandVetoException, IllegalArgumentException)
{
    Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
    Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->collapseNode(xNode);
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::addTreeExpansionListener( const Reference< XTreeExpansionListener >& xListener ) throw (RuntimeException)
{
    maTreeExpansionListeners.addInterface( xListener );
    if( getPeer().is() && (maTreeExpansionListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // add it to the peer if this is the first listener added to that proxy
        Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
        Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->addTreeExpansionListener(&maTreeExpansionListeners);
    }
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::removeTreeExpansionListener( const Reference< XTreeExpansionListener >& xListener ) throw (RuntimeException)
{
    if( getPeer().is() && (maTreeExpansionListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // remove it from the peer if this is the last listener removed from that proxy
        Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
        Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->removeTreeExpansionListener(&maTreeExpansionListeners);
    }
    maTreeExpansionListeners.removeInterface( xListener );
}

// -------------------------------------------------------------------

Reference< XTreeNode > SAL_CALL UnoTreeControl::getNodeForLocation( sal_Int32 x, sal_Int32 y ) throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getNodeForLocation(x,y);
}

// -------------------------------------------------------------------

Reference< XTreeNode > SAL_CALL UnoTreeControl::getClosestNodeForLocation( sal_Int32 x, sal_Int32 y ) throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getClosestNodeForLocation(x,y);
}

// -------------------------------------------------------------------

awt::Rectangle SAL_CALL UnoTreeControl::getNodeRect( const Reference< XTreeNode >& Node ) throw (IllegalArgumentException, RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->getNodeRect( Node );
}

// -------------------------------------------------------------------

sal_Bool SAL_CALL UnoTreeControl::isEditing(  ) throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->isEditing();
}

// -------------------------------------------------------------------

sal_Bool SAL_CALL UnoTreeControl::stopEditing() throw (RuntimeException)
{
    return Reference< XTreeControl >( getPeer(), UNO_QUERY_THROW )->stopEditing();
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::cancelEditing() throw (RuntimeException)
{
    Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
    Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->cancelEditing();
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::startEditingAtNode( const Reference< XTreeNode >& xNode ) throw (IllegalArgumentException, RuntimeException)
{
    Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
    Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->startEditingAtNode(xNode);
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::addTreeEditListener( const Reference< XTreeEditListener >& xListener ) throw (RuntimeException)
{
    maTreeEditListeners.addInterface( xListener );
    if( getPeer().is() && (maTreeEditListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // add it to the peer if this is the first listener added to that proxy
        Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
        Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->addTreeEditListener(&maTreeEditListeners);
    }
}

// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::removeTreeEditListener( const Reference< XTreeEditListener >& xListener ) throw (RuntimeException)
{
    if( getPeer().is() && (maTreeEditListeners.getLength() == 1) )
    {
        // maSelectionListeners acts as a proxy,
        // remove it from the peer if this is the last listener removed from that proxy
        Reference< XWindowPeer > xGcc3WorkaroundTemporary( getPeer());
        Reference< XTreeControl >( xGcc3WorkaroundTemporary, UNO_QUERY_THROW )->removeTreeEditListener(&maTreeEditListeners);
    }
    maTreeEditListeners.removeInterface( xListener );
}

// -------------------------------------------------------------------
// XComponent
// -------------------------------------------------------------------

void SAL_CALL UnoTreeControl::dispose(  ) throw(RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >(this);
    maSelectionListeners.disposeAndClear( aEvt );
    maTreeExpansionListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoTreeControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControlBase::createPeer( rxToolkit, rParentPeer );

    Reference< XTreeControl >  xTree( getPeer(), UNO_QUERY_THROW );
    if( maSelectionListeners.getLength() )
        xTree->addSelectionChangeListener( &maSelectionListeners );
    if( maTreeExpansionListeners.getLength() )
        xTree->addTreeExpansionListener( &maTreeExpansionListeners );
}

}

Reference< XInterface > SAL_CALL TreeControl_CreateInstance( const Reference< XMultiServiceFactory >& i_factory )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::UnoTreeControl( i_factory ) );
}

Reference< XInterface > SAL_CALL TreeControlModel_CreateInstance( const Reference< XMultiServiceFactory >& i_factory )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::UnoTreeModel( i_factory ) );
}

void SAL_CALL TreeEditListenerMultiplexer::nodeEditing( const Reference< XTreeNode >& Node ) throw (RuntimeException, ::com::sun::star::util::VetoException)
{
    ::cppu::OInterfaceIteratorHelper aIt( *this );
    while( aIt.hasMoreElements() )
    {
        Reference< XTreeEditListener > xListener(static_cast< XTreeEditListener* >( aIt.next() ) );
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
        catch( const RuntimeException& e )
        {
            (void)e;
            DISPLAY_EXCEPTION( TreeEditListenerMultiplexer, nodeEditing, e )
        }
    }
}

void SAL_CALL TreeEditListenerMultiplexer::nodeEdited( const Reference< XTreeNode >& Node, const OUString& NewText ) throw (RuntimeException)
{
    ::cppu::OInterfaceIteratorHelper aIt( *this );
    while( aIt.hasMoreElements() )
    {
        Reference< XTreeEditListener > xListener( static_cast< XTreeEditListener* >( aIt.next() ) );
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
        catch( const RuntimeException& e )
        {
            (void)e;
            DISPLAY_EXCEPTION( TreeEditListenerMultiplexer, nodeEdited, e )
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
