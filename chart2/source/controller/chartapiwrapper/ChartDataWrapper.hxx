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
#ifndef CHART_CHARTDATAWRAPPER_HXX
#define CHART_CHARTDATAWRAPPER_HXX

#include "ServiceMacros.hxx"
#include "MutexContainer.hxx"
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart/XDateCategories.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <boost/shared_ptr.hpp>

namespace chart
{
namespace wrapper
{

class Chart2ModelContact;
struct lcl_Operator;

class ChartDataWrapper : public MutexContainer, public
    ::cppu::WeakImplHelper5<
    com::sun::star::chart2::XAnyDescriptionAccess,
    com::sun::star::chart::XDateCategories,
    com::sun::star::lang::XServiceInfo,
    com::sun::star::lang::XEventListener,
    com::sun::star::lang::XComponent >
{
public:
    ChartDataWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    ChartDataWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact
        , const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartData >& xNewData );
    virtual ~ChartDataWrapper();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XDateCategories ____
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getDateCategories() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDateCategories( const ::com::sun::star::uno::Sequence< double >& rDates ) throw (::com::sun::star::uno::RuntimeException);

    // ____ XAnyDescriptionAccess ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > SAL_CALL
        getAnyRowDescriptions() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAnyRowDescriptions(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& aRowDescriptions )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > SAL_CALL
        getAnyColumnDescriptions() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAnyColumnDescriptions(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& aColumnDescriptions )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XComplexDescriptionAccess (base of XAnyDescriptionAccess) ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > SAL_CALL
        getComplexRowDescriptions() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setComplexRowDescriptions(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< OUString > >& aRowDescriptions )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > SAL_CALL
        getComplexColumnDescriptions() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setComplexColumnDescriptions(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< OUString > >& aColumnDescriptions )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence<
        double > > SAL_CALL getData()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence<
                                   ::com::sun::star::uno::Sequence<
                                   double > >& aData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        OUString > SAL_CALL getRowDescriptions()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowDescriptions( const ::com::sun::star::uno::Sequence<
                                              OUString >& aRowDescriptions )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        OUString > SAL_CALL getColumnDescriptions()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColumnDescriptions( const ::com::sun::star::uno::Sequence<
                                                 OUString >& aColumnDescriptions )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener( const ::com::sun::star::uno::Reference<
                                                           ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChartDataChangeEventListener( const ::com::sun::star::uno::Reference<
                                                              ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getNotANumber()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    void fireChartDataChangeEvent( ::com::sun::star::chart::ChartDataChangeEvent& aEvent );

private: //methods
    void switchToInternalDataProvider();
    void initDataAccess();
    void applyData( lcl_Operator& rDataOperator );

private: //member
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XAnyDescriptionAccess > m_xDataAccess;

    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::cppu::OInterfaceContainerHelper           m_aEventListenerContainer;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHARTDATAWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
