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

// UNO types
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

// helper classes
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include "ModifyListenerHelper.hxx"
#include "PropertyHelper.hxx"

// STL
#include <vector>
#include <map>

#include "OPropertySet.hxx"

namespace com::sun::star::beans { class XPropertySet; }

namespace chart
{
class LabeledDataSequence;
class RegressionCurveModel;

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

class DataSeries
    final
    : public impl::DataSeries_Base
    , public ::property::OPropertySet
{
public:
    explicit DataSeries();
    virtual ~DataSeries() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

    explicit DataSeries( const DataSeries & rOther );

    // ____ XDataSeries ____
    /// @see css::chart2::XDataSeries
    virtual css::uno::Reference< css::beans::XPropertySet >
        SAL_CALL getDataPointByIndex( sal_Int32 nIndex ) override;
    virtual void SAL_CALL resetDataPoint( sal_Int32 nIndex ) override;
    virtual void SAL_CALL resetAllDataPoints() override;

    // ____ XDataSink ____
    /// @see css::chart2::data::XDataSink
    virtual void SAL_CALL setData( const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > >& aData ) override;

    // ____ XDataSource ____
    /// @see css::chart2::data::XDataSource
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences() override;

    // ____ OPropertySet ____
    virtual void GetDefaultValue( sal_Int32 nHandle, css::uno::Any& rAny ) const override;
    virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const css::uno::Any& rValue ) override;

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    /// @see css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    // ____ XRegressionCurveContainer ____
    /// @see css::chart2::XRegressionCurveContainer
    virtual void SAL_CALL addRegressionCurve(
        const css::uno::Reference< css::chart2::XRegressionCurve >& aRegressionCurve ) override;
    virtual void SAL_CALL removeRegressionCurve(
        const css::uno::Reference< css::chart2::XRegressionCurve >& aRegressionCurve ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::XRegressionCurve > > SAL_CALL getRegressionCurves() override;
    virtual void SAL_CALL setRegressionCurves(
        const css::uno::Sequence< css::uno::Reference< css::chart2::XRegressionCurve > >& aRegressionCurves ) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    typedef std::vector< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > tDataSequenceContainer;

    void setData( const tDataSequenceContainer& aData );
    const tDataSequenceContainer & getDataSequences2() const { return m_aDataSequences; }
    void addDataSequence(css::uno::Reference<css::chart2::data::XLabeledDataSequence> const& rSequence);

    typedef
        std::vector< rtl::Reference< ::chart::RegressionCurveModel > >
        tRegressionCurveContainerType;

    const tRegressionCurveContainerType & getRegressionCurves2() const { return m_aRegressionCurves; }

    /** Get the label of a series (e.g. for the legend)

        @param rLabelSequenceRole
            The data sequence contained in xSeries that has this role will be used
            to take its label.
     */
    OUString getLabelForRole( const OUString & rLabelSequenceRole );

    bool hasUnhiddenData();

    bool hasPointOwnColor(
        sal_Int32 nPointIndex
        , const css::uno::Reference< css::beans::XPropertySet >& xDataPointProperties //may be NULL this is just for performance
         );

    // returns true if AttributedDataPoints contains nPointIndex and the
    // property Color is DEFAULT
    bool hasPointOwnProperties( sal_Int32 nPointIndex );

private:

    // late initialization to call after copy-constructing
    void Init( const DataSeries & rOther );

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

    tDataSequenceContainer        m_aDataSequences;

    typedef std::map< sal_Int32,
        css::uno::Reference< css::beans::XPropertySet > > tDataPointAttributeContainer;
    tDataPointAttributeContainer  m_aAttributedDataPoints;

    tRegressionCurveContainerType m_aRegressionCurves;

    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;
};

const tPropertyValueMap & StaticDataSeriesDefaults();

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
