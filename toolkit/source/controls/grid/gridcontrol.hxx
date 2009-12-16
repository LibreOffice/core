/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gridcontrol.hxx,v $
 * $Revision: 1.3 $
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

#ifndef TOOLKIT_GRID_CONTROL_HXX
#define TOOLKIT_GRID_CONTROL_HXX

#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/view/SelectionType.hpp>
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
// = UnoGridModel
// ===================================================================
class UnoGridModel : public UnoControlModel
{
protected:
    Any     ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
    UnoGridModel();
    UnoGridModel( const UnoGridModel& rModel );

    UnoControlModel* Clone() const;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoGridModel, UnoControlModel, szServiceName_GridControlModel )
};


// ===================================================================
// = UnoGridControl
// ===================================================================
class UnoGridControl : public ::cppu::ImplInheritanceHelper1< UnoControlBase, ::com::sun::star::awt::grid::XGridControl >
{
public:
    UnoGridControl();
    ::rtl::OUString             GetComponentServiceName();

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::grid::XGridControl
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel > SAL_CALL getColumnModel() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColumnModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel > & model) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel > SAL_CALL getDataModel() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDataModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel > & model) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getItemIndexAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException);
    //virtual void SAL_CALL addMouseListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener > & listener) throw (::com::sun::star::uno::RuntimeException);
    //virtual void SAL_CALL removeMouseListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener > & listener) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::grid::XGridSelection

    virtual ::sal_Int32 SAL_CALL getMinSelectionIndex() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getMaxSelectionIndex() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertIndexIntervall(::sal_Int32 start, ::sal_Int32 length) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeIndexIntervall(::sal_Int32 start, ::sal_Int32 length) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL getSelection() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isSelectionEmpty() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isSelectedIndex(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectRow(::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoGridControl, UnoControlBase, szServiceName_GridControl )

    using UnoControl::getPeer;
private:
    ::com::sun::star::view::SelectionType mSelectionMode;
};

} // toolkit

#endif // _TOOLKIT_TREE_CONTROL_HXX
