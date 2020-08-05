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
#include <com/sun/star/chart2/XDataInterpreter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace chart
{

class DataInterpreter : public ::cppu::WeakImplHelper<
        css::chart2::XDataInterpreter,
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
        const OUString & rName );

    static bool HasCategories(
        const css::uno::Sequence< css::beans::PropertyValue > & rArguments,
        const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > & rData );

    static bool UseCategoriesAsX(
        const css::uno::Sequence< css::beans::PropertyValue > & rArguments );

protected:
    // ____ XDataInterpreter ____
    virtual css::chart2::InterpretedData SAL_CALL interpretDataSource(
        const css::uno::Reference< css::chart2::data::XDataSource >& xSource,
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments,
        const css::uno::Sequence< css::uno::Reference< css::chart2::XDataSeries > >& aSeriesToReUse ) override;
    virtual css::chart2::InterpretedData SAL_CALL reinterpretDataSeries(
        const css::chart2::InterpretedData& aInterpretedData ) override;
    virtual sal_Bool SAL_CALL isDataCompatible(
        const css::chart2::InterpretedData& aInterpretedData ) override;
    virtual css::uno::Reference< css::chart2::data::XDataSource > SAL_CALL mergeInterpretedData(
        const css::chart2::InterpretedData& aInterpretedData ) override;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
