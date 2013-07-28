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
#ifndef _CHART_DATASERIES_HXX
#define _CHART_DATASERIES_HXX

// UNO types
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

// helper classes
#include "ServiceMacros.hxx"
#include <cppuhelper/implbase8.hxx>
#include <comphelper/uno3.hxx>
#include <osl/mutex.hxx>

// STL
#include <vector>
#include <map>

#include "MutexContainer.hxx"
#include "OPropertySet.hxx"

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper8<
        ::com::sun::star::chart2::XDataSeries,
        ::com::sun::star::chart2::data::XDataSink,
        ::com::sun::star::chart2::data::XDataSource,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::chart2::XRegressionCurveContainer,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener >
    DataSeries_Base;
}

class DataSeries :
    public MutexContainer,
    public impl::DataSeries_Base,
    public ::property::OPropertySet
{
public:
    explicit DataSeries(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataSeries();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( DataSeries )
    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    explicit DataSeries( const DataSeries & rOther );

    // late initialization to call after copy-constructing
    void Init( const DataSeries & rOther );

    // ____ XDataSeries ____
    /// @see ::com::sun::star::chart2::XDataSeries
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        SAL_CALL getDataPointByIndex( sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resetDataPoint( sal_Int32 nIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resetAllDataPoints()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataSink ____
    /// @see ::com::sun::star::chart2::data::XDataSink
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > >& aData )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataSource ____
    /// @see ::com::sun::star::chart2::data::XDataSource
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);
    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    /// @see ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    // ____ XRegressionCurveContainer ____
    /// @see ::com::sun::star::chart2::XRegressionCurveContainer
    virtual void SAL_CALL addRegressionCurve(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >& aRegressionCurve )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRegressionCurve(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >& aRegressionCurve )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XRegressionCurve > > SAL_CALL getRegressionCurves()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRegressionCurves(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XRegressionCurve > >& aRegressionCurves )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent();
    using OPropertySet::disposing;

    void fireModifyEvent();

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;
    typedef ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > > tDataSequenceContainer;
    tDataSequenceContainer        m_aDataSequences;

    typedef ::std::map< sal_Int32,
        ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > > tDataPointAttributeContainer;
    tDataPointAttributeContainer  m_aAttributedDataPoints;

    typedef
        ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve > >
        tRegressionCurveContainerType;
    tRegressionCurveContainerType m_aRegressionCurves;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
};

}  // namespace chart

// _CHART_DATASERIES_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
