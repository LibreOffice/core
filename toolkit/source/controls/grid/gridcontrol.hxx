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

#ifndef TOOLKIT_GRID_CONTROL_HXX
#define TOOLKIT_GRID_CONTROL_HXX

#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/XGridRowSelection.hpp>

#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/sequence.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <boost/scoped_ptr.hpp>

namespace toolkit
{

class GridEventForwarder;

// ===================================================================
// = UnoGridModel
// ===================================================================
class UnoGridModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

public:
    UnoGridModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    UnoGridModel( const UnoGridModel& rModel );

    UnoControlModel* Clone() const;

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoGridModel, UnoControlModel, szServiceName_GridControlModel )
};


// ===================================================================
// = UnoGridControl
// ===================================================================
typedef ::cppu::ImplInheritanceHelper2  <   UnoControlBase
                                        ,   ::com::sun::star::awt::grid::XGridControl
                                        ,   ::com::sun::star::awt::grid::XGridRowSelection
                                        >   UnoGridControl_Base;
class UnoGridControl : public UnoGridControl_Base
{
public:
    UnoGridControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ::rtl::OUString             GetComponentServiceName();

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::grid::XGridControl
    virtual ::sal_Int32 SAL_CALL getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCurrentColumn(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCurrentRow(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL goToCell( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::util::VetoException);

    // ::com::sun::star::awt::grid::XGridRowSelection
    virtual void SAL_CALL selectRow( ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException );
    virtual void SAL_CALL selectAllRows() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deselectRow( ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException );
    virtual void SAL_CALL deselectAllRows() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL getSelectedRows() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasSelectedRows() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isRowSelected(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoGridControl, UnoControlBase, szServiceName_GridControl )

    using UnoControl::getPeer;

protected:
    ~UnoGridControl();

private:
    SelectionListenerMultiplexer                m_aSelectionListeners;
    ::boost::scoped_ptr< GridEventForwarder >   m_pEventForwarder;
};

} // toolkit

#endif // _TOOLKIT_TREE_CONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
