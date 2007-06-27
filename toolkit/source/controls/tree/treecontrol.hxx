/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treecontrol.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 12:22:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef TOOLKIT_TREE_CONTROL_HXX
#define TOOLKIT_TREE_CONTROL_HXX

#ifndef _COM_SUN_STAR_AWT_TREE_XTREECONTROL_HPP_
#include <com/sun/star/awt/tree/XTreeControl.hpp>
#endif

#ifndef _TOOLKIT_HELPER_UNOCONTROLS_HXX_
#include <toolkit/controls/unocontrols.hxx>
#endif
#ifndef _TOOLKIT_AWT_UNOCONTROLMODEL_HXX_
#include <toolkit/controls/unocontrolmodel.hxx>
#endif
#ifndef _TOOLKIT_HELPER_SERVICENAMES_HXX_
#include <toolkit/helper/servicenames.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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
    UnoTreeModel();
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
class UnoTreeControl : public ::cppu::ImplInheritanceHelper1< UnoControlBase, ::com::sun::star::awt::tree::XTreeControl >
{
public:
    UnoTreeControl();
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
