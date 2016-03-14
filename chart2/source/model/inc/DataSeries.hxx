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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_INC_DATASERIES_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_INC_DATASERIES_HXX

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
#include <cppuhelper/implbase.hxx>
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
typedef ::cppu::WeakImplHelper<
        css::chart2::XDataSeries,
        css::chart2::data::XDataSink,
        css::chart2::data::XDataSource,
        css::lang::XServiceInfo,
        css::chart2::XRegressionCurveContainer,
        css::util::XCloneable,
        css::util::XModifyBroadcaster,
        css::util::XModifyListener >
    DataSeries_Base;
}

class DataSeries :
    public MutexContainer,
    public impl::DataSeries_Base,
    public ::property::OPropertySet
{
public:
    explicit DataSeries( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~DataSeries();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    explicit DataSeries( const DataSeries & rOther );

    // late initialization to call after copy-constructing
    void Init( const DataSeries & rOther );

    // ____ XDataSeries ____
    /// @see css::chart2::XDataSeries
    virtual css::uno::Reference< css::beans::XPropertySet >
        SAL_CALL getDataPointByIndex( sal_Int32 nIndex )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL resetDataPoint( sal_Int32 nIndex )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL resetAllDataPoints()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XDataSink ____
    /// @see css::chart2::data::XDataSink
    virtual void SAL_CALL setData( const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > >& aData )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XDataSource ____
    /// @see css::chart2::data::XDataSource
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException) override;
    virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const css::uno::Any& rValue )
        throw (css::uno::Exception, std::exception) override;

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    /// @see css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    // ____ XRegressionCurveContainer ____
    /// @see css::chart2::XRegressionCurveContainer
    virtual void SAL_CALL addRegressionCurve(
        const css::uno::Reference< css::chart2::XRegressionCurve >& aRegressionCurve )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeRegressionCurve(
        const css::uno::Reference< css::chart2::XRegressionCurve >& aRegressionCurve )
        throw (css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::XRegressionCurve > > SAL_CALL getRegressionCurves()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRegressionCurves(
        const css::uno::Sequence< css::uno::Reference< css::chart2::XRegressionCurve > >& aRegressionCurves )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    void fireModifyEvent();

private:
    css::uno::Reference< css::uno::XComponentContext >  m_xContext;
    typedef ::std::vector< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > tDataSequenceContainer;
    tDataSequenceContainer        m_aDataSequences;

    typedef ::std::map< sal_Int32,
        css::uno::Reference< css::beans::XPropertySet > > tDataPointAttributeContainer;
    tDataPointAttributeContainer  m_aAttributedDataPoints;

    typedef
        ::std::vector< css::uno::Reference< css::chart2::XRegressionCurve > >
        tRegressionCurveContainerType;
    tRegressionCurveContainerType m_aRegressionCurves;

    css::uno::Reference< css::util::XModifyListener > m_xModifyEventForwarder;
};

}  // namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_INC_DATASERIES_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
