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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_CHARTDATAWRAPPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_CHARTDATAWRAPPER_HXX

#include "MutexContainer.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart/XDateCategories.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

namespace chart
{
namespace wrapper
{

class Chart2ModelContact;
struct lcl_Operator;

class ChartDataWrapper : public MutexContainer, public
    ::cppu::WeakImplHelper<
    css::chart2::XAnyDescriptionAccess,
    css::chart::XDateCategories,
    css::lang::XServiceInfo,
    css::lang::XEventListener,
    css::lang::XComponent >
{
public:
    explicit ChartDataWrapper( std::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    ChartDataWrapper( std::shared_ptr< Chart2ModelContact > spChart2ModelContact
        , const css::uno::Reference< css::chart::XChartData >& xNewData );
    virtual ~ChartDataWrapper();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

protected:
    // ____ XDateCategories ____
    virtual css::uno::Sequence< double > SAL_CALL getDateCategories() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDateCategories( const css::uno::Sequence< double >& rDates ) throw (css::uno::RuntimeException, std::exception) override;

    // ____ XAnyDescriptionAccess ____
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL
        getAnyRowDescriptions() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAnyRowDescriptions(
        const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aRowDescriptions )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL
        getAnyColumnDescriptions() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAnyColumnDescriptions(
        const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aColumnDescriptions )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XComplexDescriptionAccess (base of XAnyDescriptionAccess) ____
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL
        getComplexRowDescriptions() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setComplexRowDescriptions(
        const css::uno::Sequence< css::uno::Sequence< OUString > >& aRowDescriptions )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL
        getComplexColumnDescriptions() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setComplexColumnDescriptions(
        const css::uno::Sequence< css::uno::Sequence< OUString > >& aColumnDescriptions )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL getData()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setData( const css::uno::Sequence< css::uno::Sequence< double > >& aData )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRowDescriptions()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRowDescriptions( const css::uno::Sequence< OUString >& aRowDescriptions )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getColumnDescriptions()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColumnDescriptions( const css::uno::Sequence< OUString >& aColumnDescriptions )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getNotANumber()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    void fireChartDataChangeEvent( css::chart::ChartDataChangeEvent& aEvent );

private: //methods
    void switchToInternalDataProvider();
    void initDataAccess();
    void applyData( lcl_Operator& rDataOperator );

private: //member
    css::uno::Reference< css::chart2::XAnyDescriptionAccess > m_xDataAccess;

    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper2      m_aEventListenerContainer;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_CHARTDATAWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
