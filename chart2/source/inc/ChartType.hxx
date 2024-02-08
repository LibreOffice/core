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
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <rtl/ref.hxx>

#include <vector>
#include "charttoolsdllapi.hxx"

namespace chart
{
class BaseCoordinateSystem;
class DataSeries;
class ModifyEventForwarder;

enum
{
    PROP_PIECHARTTYPE_USE_RINGS,
    PROP_PIECHARTTYPE_3DRELATIVEHEIGHT,
    PROP_PIECHARTTYPE_SUBTYPE // none, of-bar, of-pie
};



namespace impl
{
typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::chart2::XChartType,
        css::chart2::XDataSeriesContainer,
        css::util::XCloneable,
        css::util::XModifyBroadcaster,
        css::util::XModifyListener >
    ChartType_Base;
}

class UNLESS_MERGELIBS(OOO_DLLPUBLIC_CHARTTOOLS) ChartType :
    public impl::ChartType_Base,
    public ::property::OPropertySet
{
public:
    explicit ChartType();
    virtual ~ChartType() override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()

    explicit ChartType( const ChartType & rOther );

    // ____ XChartType ____
    // still abstract ! implement !
    virtual OUString SAL_CALL getChartType() override = 0;
    virtual css::uno::Reference< css::chart2::XCoordinateSystem > SAL_CALL
        createCoordinateSystem( ::sal_Int32 DimensionCount ) final override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedMandatoryRoles() override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedOptionalRoles() override;
    virtual OUString SAL_CALL getRoleOfSequenceForSeriesLabel() override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedPropertyRoles() override;

    // ____ XDataSeriesContainer ____
    virtual void SAL_CALL addDataSeries(
        const css::uno::Reference< css::chart2::XDataSeries >& aDataSeries ) override;
    virtual void SAL_CALL removeDataSeries(
        const css::uno::Reference< css::chart2::XDataSeries >& aDataSeries ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::XDataSeries > > SAL_CALL getDataSeries() override;
    virtual void SAL_CALL setDataSeries(
        const css::uno::Sequence< css::uno::Reference< css::chart2::XDataSeries > >& aDataSeries ) override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
        
    virtual rtl::Reference<ChartType> cloneChartType() const = 0;

    void addDataSeries(
        const rtl::Reference< ::chart::DataSeries >& aDataSeries );
    void removeDataSeries(
        const rtl::Reference< ::chart::DataSeries >& aDataSeries );
    void setDataSeries(
        const std::vector< rtl::Reference< ::chart::DataSeries > >& aDataSeries );
    const std::vector< rtl::Reference< ::chart::DataSeries > > & getDataSeries2() const { return m_aDataSeries; }

    virtual rtl::Reference< ::chart::BaseCoordinateSystem >
        createCoordinateSystem2( sal_Int32 DimensionCount );

protected:

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    void fireModifyEvent();

    // ____ OPropertySet ____
    virtual void GetDefaultValue( sal_Int32 nHandle, css::uno::Any& rAny ) const override;
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;

private:
    void impl_addDataSeriesWithoutNotification(
        const rtl::Reference< ::chart::DataSeries >& aDataSeries );

private:
    typedef
        std::vector< rtl::Reference< ::chart::DataSeries > >  tDataSeriesContainerType;

    // --- mutable members: the following members need mutex guard ---

    tDataSeriesContainerType  m_aDataSeries;

    bool  m_bNotifyChanges;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
