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

#ifndef INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDCONTROL_HXX
#define INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDCONTROL_HXX

#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/XGridRowSelection.hpp>

#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/sequence.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <memory>

namespace toolkit
{

class GridEventForwarder;


// = UnoGridModel

class UnoGridModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any ImplGetDefaultValue( sal_uInt16 nPropId ) const SAL_OVERRIDE;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() SAL_OVERRIDE;

public:
    explicit UnoGridModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_factory );
    UnoGridModel( const UnoGridModel& rModel );

    UnoControlModel* Clone() const SAL_OVERRIDE;

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("stardiv.Toolkit.GridControlModel"); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        auto s(UnoControlModel::getSupportedServiceNames());
        s.realloc(s.getLength() + 1);
        s[s.getLength() - 1] = "com.sun.star.awt.grid.UnoControlGridModel";
        return s;
    }
};



// = UnoGridControl

typedef ::cppu::ImplInheritanceHelper  <   UnoControlBase
                                        ,   ::com::sun::star::awt::grid::XGridControl
                                        ,   ::com::sun::star::awt::grid::XGridRowSelection
                                        >   UnoGridControl_Base;
class UnoGridControl : public UnoGridControl_Base
{
public:
    UnoGridControl();
    OUString             GetComponentServiceName() SAL_OVERRIDE;

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::XControl
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::grid::XGridControl
    virtual ::sal_Int32 SAL_CALL getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getCurrentColumn(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getCurrentRow(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL goToCell( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::util::VetoException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::awt::grid::XGridRowSelection
    virtual void SAL_CALL selectRow( ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL selectAllRows() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL deselectRow( ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL deselectAllRows() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL getSelectedRows() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasSelectedRows() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isRowSelected(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("stardiv.Toolkit.GridControl"); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        auto s(UnoControlBase::getSupportedServiceNames());
        s.realloc(s.getLength() + 1);
        s[s.getLength() - 1] = "com.sun.star.awt.grid.UnoControlGrid";
        return s;
    }

    using UnoControl::getPeer;

protected:
    virtual ~UnoGridControl();

private:
    SelectionListenerMultiplexer                m_aSelectionListeners;
    std::unique_ptr< GridEventForwarder >   m_pEventForwarder;
};

} // toolkit

#endif // _TOOLKIT_TREE_CONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
