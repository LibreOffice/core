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

#include "DataSeries.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <rtl/ref.hxx>
#include <vector>

namespace chart
{
class DataSource;

/** offers tooling to interpret different data sources in a structural
     and chart-type-dependent way.
  */
struct InterpretedData
{
    std::vector< std::vector< rtl::Reference<::chart::DataSeries> > > Series;
    css::uno::Reference< css::chart2::data::XLabeledDataSequence > Categories;
};

/** offers tooling to interpret different data sources in a structural
    and chart-type-dependent way.
 */
class SAL_DLLPUBLIC_RTTI DataInterpreter : public ::cppu::WeakImplHelper<
        css::lang::XServiceInfo >
{
public:
    explicit DataInterpreter();
    virtual ~DataInterpreter() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // convenience methods
    static  OUString GetRole( const css::uno::Reference< css::chart2::data::XDataSequence > & xSeq );

    static void SetRole(
        const css::uno::Reference< css::chart2::data::XDataSequence > & xSeq,
        const OUString & rRole );

    static css::uno::Any GetProperty(
        const css::uno::Sequence<css::beans::PropertyValue > & aArguments,
        std::u16string_view rName );

    static bool HasCategories(
        const css::uno::Sequence< css::beans::PropertyValue > & rArguments,
        const std::vector< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > & rData );

    static bool UseCategoriesAsX(
        const css::uno::Sequence< css::beans::PropertyValue > & rArguments );

    static std::vector<css::uno::Reference< css::chart2::data::XLabeledDataSequence >> getDataSequences(
        const css::uno::Reference< css::chart2::data::XDataSource >& xSource);

    // ____ DataInterpreter ____
    /** Interprets the given data.

        @param xSource
            the data source.

        @param aArguments
            Arguments that tell the template how to slice the given
            range.  The properties should be defined in a separate
            service.

            <p>For standard parameters that may be used, see the
            service StandardDiagramCreationParameters.
            </p>

        @param aSeriesToReUse
            use all the data series given here for the result before
            creating new ones.
     */
    virtual InterpretedData interpretDataSource(
        const css::uno::Reference< css::chart2::data::XDataSource >& xSource,
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments,
        const std::vector< rtl::Reference< ::chart::DataSeries > >& aSeriesToReUse );

    /** Re-interprets the data given in <code>aInterpretedData</code>
        while keeping the number of data series and the categories.
     */
    virtual InterpretedData reinterpretDataSeries(
        const InterpretedData& aInterpretedData );

    /** parses the given data and states, if a
        reinterpretDataSeries() call can be done
        without data loss.

        @return
            `TRUE`, if the data given in
            <code>aInterpretedData</code> has a similar structure than
            the one required is used as output of the data interpreter.
     */
    virtual bool isDataCompatible(
        const InterpretedData& aInterpretedData );

    /** Try to reverse the operation done in
        interpretDataSource().

        <p>In case <code>aInterpretedData</code> is the result of
        interpretDataSource()( <code>xSource</code> ),
        the result of this method should be <code>xSource</code>.</p>
     */
    static rtl::Reference< ::chart::DataSource > mergeInterpretedData(
        const InterpretedData& aInterpretedData );

    /** Get chart information that is specific to a particular chart
        type, by key.

        @param sKey
            name of the piece of data to retrieve.

        <p>Supported key strings:</p>
        <ul>
            <li><tt>"stock variant"</tt>: stock chart variant,
            with 0 = neither Open Values nor volume, 1 = Open Values,
            2 = volume, 3 = both. Valid for candlestick charts.</li>
        </ul>

        @return
            The value requested, or nothing if not present.
    */
    virtual css::uno::Any getChartTypeSpecificData(
        const OUString& sKey );
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
