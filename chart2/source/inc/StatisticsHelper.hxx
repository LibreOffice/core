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

#include <config_options.h>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::chart2::data { class XDataProvider; }
namespace com::sun::star::chart2::data { class XDataSequence; }
namespace com::sun::star::chart2::data { class XDataSource; }
namespace com::sun::star::chart2::data { class XLabeledDataSequence; }
namespace com::sun::star::uno { template <typename > class Sequence; }
namespace chart { class DataSeries; }

namespace chart::StatisticsHelper
{
    /** Calculates 1/n * sum (x_i - x_mean)^2.

        @see http://mathworld.wolfram.com/Variance.html
     */
    double getVariance( const css::uno::Sequence< double > & rData );

    // square root of the variance
    double getStandardDeviation( const css::uno::Sequence< double > & rData );

    // also called "Standard deviation of the mean (SDOM)"
    double getStandardError( const css::uno::Sequence< double > & rData );

    css::uno::Reference< css::chart2::data::XLabeledDataSequence >
        getErrorLabeledDataSequenceFromDataSource(
            const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource,
            bool bPositiveValue,
            bool bYError = true );

    css::uno::Reference< css::chart2::data::XDataSequence >
        getErrorDataSequenceFromDataSource(
            const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource,
            bool bPositiveValue,
            bool bYError = true );

    double getErrorFromDataSource(
        const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource,
        sal_Int32 nIndex,
        bool bPositiveValue,
        bool bYError = true );

    void setErrorDataSequence(
        const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource,
        const css::uno::Reference< css::chart2::data::XDataProvider > & xDataProvider,
        const OUString & rNewRange,
        bool bPositiveValue,
        bool bYError = true,
        OUString const * pXMLRange = nullptr );

    /// @return the newly created or existing error bar object
    css::uno::Reference< css::beans::XPropertySet >
        addErrorBars(
            const rtl::Reference< ::chart::DataSeries > & xDataSeries,
            sal_Int32 nStyle,
            bool bYError = true );

    css::uno::Reference< css::beans::XPropertySet >
        getErrorBars(
            const rtl::Reference< ::chart::DataSeries > & xDataSeries,
            bool bYError = true );

    bool hasErrorBars(
        const rtl::Reference< ::chart::DataSeries > & xDataSeries,
        bool bYError = true );

    void removeErrorBars(
        const rtl::Reference< ::chart::DataSeries > & xDataSeries,
        bool bYError = true );

    bool usesErrorBarRanges(
        const rtl::Reference< ::chart::DataSeries > & xDataSeries,
        bool bYError = true );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
