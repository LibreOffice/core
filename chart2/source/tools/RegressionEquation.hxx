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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/chart2/XTitle.hpp>

#include <MutexContainer.hxx>
#include <OPropertySet.hxx>

#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::util::XCloneable,
        css::util::XModifyBroadcaster,
        css::util::XModifyListener,
        css::chart2::XTitle >
    RegressionEquation_Base;
}

class RegressionEquation final :
        public MutexContainer,
        public impl::RegressionEquation_Base,
        public ::property::OPropertySet
{
public:
    explicit RegressionEquation();
    virtual ~RegressionEquation() override;

    virtual OUString SAL_CALL
        getImplementationName()
        override;
    virtual sal_Bool SAL_CALL
        supportsService( const OUString& ServiceName )
        override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()

private:
    explicit RegressionEquation( const RegressionEquation & rOther );

    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const override;

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    // ____ XTitle ____
    virtual css::uno::Sequence<
        css::uno::Reference< css::chart2::XFormattedString > > SAL_CALL getText() override;
    virtual void SAL_CALL setText( const css::uno::Sequence<
                                   css::uno::Reference<
                                   css::chart2::XFormattedString > >& Strings ) override;

    using ::cppu::OPropertySetHelper::disposing;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;

    void fireModifyEvent();

    css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > > m_aStrings;

    css::uno::Reference< css::util::XModifyListener > m_xModifyEventForwarder;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
