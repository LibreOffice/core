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

#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include "charttoolsdllapi.hxx"

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::chart2 { class XDataSeries; }
namespace com::sun::star::chart2::data { class XDataProvider; }
namespace com::sun::star::chart2::data { class XDataSequence; }
namespace com::sun::star::chart2::data { class XDataSource; }
namespace com::sun::star::chart2::data { class XLabeledDataSequence; }
namespace com::sun::star::uno { template <typename > class Sequence; }

namespace chart::StatisticsHelper
{
    /** Calculates 1/n * sum (x_i - x_mean)^2.

        @see http://mathworld.wolfram.com/Variance.html
     */
    OOO_DLLPUBLIC_CHARTTOOLS double getVariance( const css::uno::Sequence< double > & rData );

    // square root of the variance
    OOO_DLLPUBLIC_CHARTTOOLS double getStandardDeviation( const css::uno::Sequence< double > & rData );

    // also called "Standard deviation of the mean (SDOM)"
    OOO_DLLPUBLIC_CHARTTOOLS double getStandardError( const css::uno::Sequence< double > & rData );

    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference< css::chart2::data::XLabeledDataSequence >
        getErrorLabeledDataSequenceFromDataSource(
            const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource,
            bool bPositiveValue,
            bool bYError = true );

    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference< css::chart2::data::XDataSequence >
        getErrorDataSequenceFromDataSource(
            const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource,
            bool bPositiveValue,
            bool bYError = true );

    OOO_DLLPUBLIC_CHARTTOOLS double getErrorFromDataSource(
        const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource,
        sal_Int32 nIndex,
        bool bPositiveValue,
        bool bYError = true );

    OOO_DLLPUBLIC_CHARTTOOLS void setErrorDataSequence(
        const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource,
        const css::uno::Reference< css::chart2::data::XDataProvider > & xDataProvider,
        const OUString & rNewRange,
        bool bPositiveValue,
        bool bYError = true,
        OUString const * pXMLRange = nullptr );

    /// @return the newly created or existing error bar object
    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference< css::beans::XPropertySet >
        addErrorBars(
            const css::uno::Reference< css::chart2::XDataSeries > & xDataSeries,
            sal_Int32 nStyle,
            bool bYError = true );

    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference< css::beans::XPropertySet >
        getErrorBars(
            const css::uno::Reference< css::chart2::XDataSeries > & xDataSeries,
            bool bYError = true );

    OOO_DLLPUBLIC_CHARTTOOLS bool hasErrorBars(
        const css::uno::Reference< css::chart2::XDataSeries > & xDataSeries,
        bool bYError = true );

    OOO_DLLPUBLIC_CHARTTOOLS void removeErrorBars(
        const css::uno::Reference< css::chart2::XDataSeries > & xDataSeries,
        bool bYError = true );

    OOO_DLLPUBLIC_CHARTTOOLS bool usesErrorBarRanges(
        const css::uno::Reference< css::chart2::XDataSeries > & xDataSeries,
        bool bYError = true );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
