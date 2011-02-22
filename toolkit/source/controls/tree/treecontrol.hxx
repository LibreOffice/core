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

#ifndef TOOLKIT_TREE_CONTROL_HXX
#define TOOLKIT_TREE_CONTROL_HXX

#include <com/sun/star/awt/tree/XTreeControl.hpp>
#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <cppuhelper/implbase1.hxx>

#include <toolkit/helper/listenermultiplexer.hxx>

namespace toolkit {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

// ===================================================================
// = UnoTreeModel
// ===================================================================
class UnoTreeModel : public UnoControlModel
{
protected:
    Any     ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
    UnoTreeModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    UnoTreeModel( const UnoTreeModel& rModel );

    UnoControlModel* Clone() const;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoTreeModel, UnoControlModel, szServiceName_TreeControlModel )
};


// ===================================================================
// = UnoTreeControl
// ===================================================================
typedef ::cppu::ImplInheritanceHelper1< UnoControlBase, ::com::sun::star::awt::tree::XTreeControl > UnoTreeControl_Base;
class UnoTreeControl : public UnoTreeControl_Base
{
public:
    UnoTreeControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ::rtl::OUString             GetComponentServiceName();

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::view::XSelectionSupplier
    virtual ::sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& xSelection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::view::XMultiSelectionSupplier
    virtual ::sal_Bool SAL_CALL addSelection( const ::com::sun::star::uno::Any& Selection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelection( const ::com::sun::star::uno::Any& Selection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearSelection(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getSelectionCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createSelectionEnumeration(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createReverseSelectionEnumeration(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTreeControl
    virtual ::rtl::OUString SAL_CALL getDefaultExpandedGraphicURL() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultExpandedGraphicURL( const ::rtl::OUString& _defaultexpandedgraphicurl ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDefaultCollapsedGraphicURL() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultCollapsedGraphicURL( const ::rtl::OUString& _defaultcollapsedgraphicurl ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isNodeExpanded( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isNodeCollapsed( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL makeNodeVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::awt::tree::ExpandVetoException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isNodeVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL expandNode( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::awt::tree::ExpandVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL collapseNode( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::awt::tree::ExpandVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addTreeExpansionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeExpansionListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTreeExpansionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeExpansionListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode > SAL_CALL getNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode > SAL_CALL getClosestNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getNodeRect( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isEditing(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL stopEditing(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancelEditing(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startEditingAtNode( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addTreeEditListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeEditListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTreeEditListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeEditListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoTreeControl, UnoControlBase, szServiceName_TreeControl )

    using UnoControl::getPeer;
private:
    TreeSelectionListenerMultiplexer maSelectionListeners;
    TreeExpansionListenerMultiplexer maTreeExpansionListeners;
    TreeEditListenerMultiplexer maTreeEditListeners;
};

} // toolkit

#endif // _TOOLKIT_TREE_CONTROL_HXX
