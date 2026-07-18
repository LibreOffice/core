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
#pragma once

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart/XDateCategories.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

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
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    rtl::Reference<ChartModel> getChartModel() const;

private:
    // ____ XDateCategories ____
    virtual cpo::uno::Sequence< double > getDateCategories() override;
    virtual void setDateCategories( const cpo::uno::Sequence< double >& rDates ) override;

    // ____ XAnyDescriptionAccess ____
    virtual cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >
        getAnyRowDescriptions() override;
    virtual void setAnyRowDescriptions(
        const cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >& aRowDescriptions ) override;
    virtual cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >
        getAnyColumnDescriptions() override;
    virtual void setAnyColumnDescriptions(
        const cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >& aColumnDescriptions ) override;

    // ____ XComplexDescriptionAccess (base of XAnyDescriptionAccess) ____
    virtual cpo::uno::Sequence< cpo::uno::Sequence< OUString > >
        getComplexRowDescriptions() override;
    virtual void setComplexRowDescriptions(
        const cpo::uno::Sequence< cpo::uno::Sequence< OUString > >& aRowDescriptions ) override;
    virtual cpo::uno::Sequence< cpo::uno::Sequence< OUString > >
        getComplexColumnDescriptions() override;
    virtual void setComplexColumnDescriptions(
        const cpo::uno::Sequence< cpo::uno::Sequence< OUString > >& aColumnDescriptions ) override;

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual cpo::uno::Sequence< cpo::uno::Sequence< double > > getData() override;
    virtual void setData( const cpo::uno::Sequence< cpo::uno::Sequence< double > >& aData ) override;
    virtual cpo::uno::Sequence< OUString > getRowDescriptions() override;
    virtual void setRowDescriptions( const cpo::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual cpo::uno::Sequence< OUString > getColumnDescriptions() override;
    virtual void setColumnDescriptions( const cpo::uno::Sequence< OUString >& aColumnDescriptions ) override;

    // ____ XChartData (base of XChartDataArray) ____
    virtual void addChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void removeChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double getNotANumber() override;
    virtual bool isNotANumber( double nNumber ) override;

    // ____ XComponent ____
    virtual void dispose() override;
    virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // ____ XEventListener ____
    virtual void disposing( const css::lang::EventObject& Source ) override;

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
