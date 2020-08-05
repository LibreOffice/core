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

#include <OPropertySet.hxx>
#include <MutexContainer.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper
    < css::lang::XServiceInfo,
      css::chart2::XCoordinateSystem,
      css::chart2::XChartTypeContainer,
      css::util::XCloneable,
      css::util::XModifyBroadcaster,
      css::util::XModifyListener >
    BaseCoordinateSystem_Base;
}

class BaseCoordinateSystem :
        public impl::BaseCoordinateSystem_Base,
        public MutexContainer,
        public ::property::OPropertySet
{
public:
    BaseCoordinateSystem( sal_Int32 nDimensionCount );
    explicit BaseCoordinateSystem( const BaseCoordinateSystem & rSource );
    virtual ~BaseCoordinateSystem() override;

    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const override;

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ XCoordinateSystem ____
    virtual ::sal_Int32 SAL_CALL getDimension() override;
    virtual void SAL_CALL setAxisByDimension(
        ::sal_Int32 nDimension,
        const css::uno::Reference< css::chart2::XAxis >& xAxis,
        ::sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::chart2::XAxis > SAL_CALL getAxisByDimension(
        ::sal_Int32 nDimension, ::sal_Int32 nIndex ) override;
    virtual ::sal_Int32 SAL_CALL getMaximumAxisIndexByDimension( ::sal_Int32 nDimension ) override;

    // ____ XChartTypeContainer ____
    virtual void SAL_CALL addChartType(
        const css::uno::Reference< css::chart2::XChartType >& aChartType ) override;
    virtual void SAL_CALL removeChartType(
        const css::uno::Reference< css::chart2::XChartType >& aChartType ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::XChartType > > SAL_CALL getChartTypes() override;
    virtual void SAL_CALL setChartTypes(
        const css::uno::Sequence< css::uno::Reference< css::chart2::XChartType > >& aChartTypes ) override;

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

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    void fireModifyEvent();

protected:
    css::uno::Reference< css::util::XModifyListener > m_xModifyEventForwarder;

private:
    sal_Int32                                             m_nDimensionCount;
    typedef std::vector< std::vector< css::uno::Reference< css::chart2::XAxis > > > tAxisVecVecType;
    tAxisVecVecType m_aAllAxis; //outer sequence is the dimension; inner sequence is the axis index that indicates main or secondary axis
    std::vector< css::uno::Reference< css::chart2::XChartType > >          m_aChartTypes;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
