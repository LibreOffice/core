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
#ifndef INCLUDED_CHART2_SOURCE_INC_DATASOURCE_HXX
#define INCLUDED_CHART2_SOURCE_INC_DATASOURCE_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

namespace chart
{

class DataSource : public
    ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::chart2::data::XDataSource,
        css::chart2::data::XDataSink >
{
public:
    explicit DataSource( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    explicit DataSource(
        const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > & rSequences );

    virtual ~DataSource();

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

protected:
    // ____ XDataSource ____
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > SAL_CALL
        getDataSequences()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XDataSink ____
    virtual void SAL_CALL setData( const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > >& aData )
        throw (css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > >
        m_aDataSeq;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_DATASOURCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
