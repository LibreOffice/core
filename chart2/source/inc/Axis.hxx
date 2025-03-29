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
#pragma once

#include "OPropertySet.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include "ModifyListenerHelper.hxx"
#include "PropertyHelper.hxx"

namespace chart
{
class GridProperties;
class Title;

namespace impl
{
typedef ::cppu::WeakImplHelper<
        css::chart2::XAxis,
        css::chart2::XTitled,
        css::lang::XServiceInfo,
        css::util::XCloneable,
        css::util::XModifyBroadcaster,
        css::util::XModifyListener >
    Axis_Base;
}

class Axis final :
    public impl::Axis_Base,
    public ::property::OPropertySet
{
public:
    explicit Axis();
    virtual ~Axis() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

private:
    explicit Axis( const Axis & rOther );

    // late initialization to call after copy-constructing
    void Init();

    // ____ OPropertySet ____
    virtual void GetDefaultValue( sal_Int32 nHandle, css::uno::Any& rAny ) const override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

public:
    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

    // ____ XAxis ____
    virtual void SAL_CALL setScaleData( const css::chart2::ScaleData& rScaleData ) override;
    virtual css::chart2::ScaleData SAL_CALL getScaleData() override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getGridProperties() override;
    virtual css::uno::Sequence< css::uno::Reference< css::beans::XPropertySet > > SAL_CALL getSubGridProperties() override;
    virtual css::uno::Sequence< css::uno::Reference< css::beans::XPropertySet > > SAL_CALL getSubTickProperties() override;

    // ____ XTitled ____
    virtual css::uno::Reference< css::chart2::XTitle > SAL_CALL getTitleObject() override;
    virtual void SAL_CALL setTitleObject(
        const css::uno::Reference< css::chart2::XTitle >& Title ) override;

    // ____ XCloneable ____
    // Note: the coordinate systems are not cloned!
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    rtl::Reference< ::chart::Title > getTitleObject2() const;
    void setTitleObject( const rtl::Reference< ::chart::Title >& xNewTitle );

    rtl::Reference< ::chart::GridProperties > getGridProperties2();
    std::vector< rtl::Reference< ::chart::GridProperties > > getSubGridProperties2();

private:
    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    void fireModifyEvent();

    void AllocateSubGrids();

    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;

    css::chart2::ScaleData             m_aScaleData;

    rtl::Reference< ::chart::GridProperties >     m_xGrid;

    std::vector< rtl::Reference< ::chart::GridProperties > >     m_aSubGridProperties;

    rtl::Reference< ::chart::Title >          m_xTitle;
};

const ::chart::tPropertyValueMap &  StaticAxisDefaults();

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
