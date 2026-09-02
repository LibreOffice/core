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

#ifndef INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDCONTROL_HXX
#define INCLUDED_TOOLKIT_SOURCE_CONTROLS_GRID_GRIDCONTROL_HXX

#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/XGridRowSelection.hpp>

#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <cppuhelper/implbase2.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <memory>

namespace toolkit
{

class GridEventForwarder;


// = UnoGridModel

class UnoGridModel : public UnoControlModel
{
protected:
    cpo::uno::Any ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

public:
    explicit UnoGridModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory );
    UnoGridModel( const UnoGridModel& rModel );

    rtl::Reference<UnoControlModel> Clone() const override;

    // css::lang::XComponent
    void dispose(  ) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString getServiceName() override;

    // OPropertySetHelper
    void setFastPropertyValue_NoBroadcast( std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;

    // XServiceInfo
    OUString getImplementationName() override
    { return u"stardiv.Toolkit.GridControlModel"_ustr; }

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override
    {
        auto s(UnoControlModel::getSupportedServiceNames());
        s.realloc(s.getLength() + 1);
        s.getArray()[s.getLength() - 1] = u"com.sun.star.awt.grid.UnoControlGridModel"_ustr;
        return s;
    }
};


// = UnoGridControl

typedef ::cppu::AggImplInheritanceHelper2  <   UnoControlBase
                                        ,   css::awt::grid::XGridControl
                                        ,   css::awt::grid::XGridRowSelection
                                        >   UnoGridControl_Base;
class UnoGridControl : public UnoGridControl_Base
{
public:
    UnoGridControl();
    OUString             GetComponentServiceName() const override;

    // css::lang::XComponent
    void dispose(  ) override;

    // css::awt::XControl
    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    bool setModel( const css::uno::Reference< css::awt::XControlModel >& rxModel ) override;

    // css::awt::grid::XGridControl
    virtual ::sal_Int32 getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) override;
    virtual ::sal_Int32 getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) override;
    virtual ::sal_Int32 getCurrentColumn(  ) override;
    virtual ::sal_Int32 getCurrentRow(  ) override;
    virtual void goToCell( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) override;

    // css::awt::grid::XGridRowSelection
    virtual void selectRow( ::sal_Int32 i_rowIndex ) override;
    virtual void selectAllRows() override;
    virtual void deselectRow( ::sal_Int32 i_rowIndex ) override;
    virtual void deselectAllRows() override;
    virtual cpo::uno::Sequence< ::sal_Int32 > getSelectedRows() override;
    virtual bool hasSelectedRows() override;
    virtual bool isRowSelected(::sal_Int32 index) override;
    virtual void addSelectionListener(const css::uno::Reference< css::awt::grid::XGridSelectionListener > & listener) override;
    virtual void removeSelectionListener(const css::uno::Reference< css::awt::grid::XGridSelectionListener > & listener) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override
    { return u"stardiv.Toolkit.GridControl"_ustr; }

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override
    {
        auto s(UnoControlBase::getSupportedServiceNames());
        s.realloc(s.getLength() + 1);
        s.getArray()[s.getLength() - 1] = u"com.sun.star.awt.grid.UnoControlGrid"_ustr;
        return s;
    }

    using UnoControl::getPeer;

protected:
    virtual ~UnoGridControl() override;

private:
    SelectionListenerMultiplexer                m_aSelectionListeners;
    std::unique_ptr< GridEventForwarder >   m_pEventForwarder;
};

} // toolkit

#endif // _TOOLKIT_TREE_CONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
