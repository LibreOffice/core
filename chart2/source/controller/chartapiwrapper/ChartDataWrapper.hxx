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

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart/XDateCategories.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/EventObject.hpp>

#include <memory>

namespace chart
{
class ChartModel;

namespace wrapper
{

class Chart2ModelContact;
struct lcl_Operator;

class ChartDataWrapper final : public
    ::cppu::WeakImplHelper<
    css::chart2::XAnyDescriptionAccess,
    css::chart::XDateCategories,
    css::lang::XServiceInfo,
    css::lang::XEventListener,
    css::lang::XComponent >
{
public:
    explicit ChartDataWrapper(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);
    ChartDataWrapper(std::shared_ptr<Chart2ModelContact> spChart2ModelContact
        , const css::uno::Reference< css::chart::XChartData >& xNewData );
    virtual ~ChartDataWrapper() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    rtl::Reference<ChartModel> getChartModel() const;

private:
    // ____ XDateCategories ____
    virtual css::uno::Sequence< double > SAL_CALL getDateCategories() override;
    virtual void SAL_CALL setDateCategories( const css::uno::Sequence< double >& rDates ) override;

    // ____ XAnyDescriptionAccess ____
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL
        getAnyRowDescriptions() override;
    virtual void SAL_CALL setAnyRowDescriptions(
        const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aRowDescriptions ) override;
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL
        getAnyColumnDescriptions() override;
    virtual void SAL_CALL setAnyColumnDescriptions(
        const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aColumnDescriptions ) override;

    // ____ XComplexDescriptionAccess (base of XAnyDescriptionAccess) ____
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL
        getComplexRowDescriptions() override;
    virtual void SAL_CALL setComplexRowDescriptions(
        const css::uno::Sequence< css::uno::Sequence< OUString > >& aRowDescriptions ) override;
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL
        getComplexColumnDescriptions() override;
    virtual void SAL_CALL setComplexColumnDescriptions(
        const css::uno::Sequence< css::uno::Sequence< OUString > >& aColumnDescriptions ) override;

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL getData() override;
    virtual void SAL_CALL setData( const css::uno::Sequence< css::uno::Sequence< double > >& aData ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRowDescriptions() override;
    virtual void SAL_CALL setRowDescriptions( const css::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getColumnDescriptions() override;
    virtual void SAL_CALL setColumnDescriptions( const css::uno::Sequence< OUString >& aColumnDescriptions ) override;

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void SAL_CALL removeChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double SAL_CALL getNotANumber() override;
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber ) override;

    // ____ XComponent ____
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    void fireChartDataChangeEvent( css::chart::ChartDataChangeEvent& aEvent );

    void switchToInternalDataProvider();
    void initDataAccess();
    void applyData( lcl_Operator& rDataOperator );

    std::mutex m_aMutex;
    css::uno::Reference< css::chart2::XAnyDescriptionAccess > m_xDataAccess;
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_aEventListenerContainer;
};

} //  namespace chart::wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
