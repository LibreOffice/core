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

#include "DataSource.hxx"
#include "LabeledDataSequence.hxx"
#include <cppuhelper/supportsservice.hxx>

using ::osl::MutexGuard;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;

namespace
{
static const char lcl_aServiceName[] = "com.sun.star.comp.chart.DataSource";
}  // anonymous namespace

namespace chart
{

DataSource::DataSource(
    const Reference< uno::XComponentContext > & /*xContext*/ )
{}

DataSource::DataSource(
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > & rSequences ) :
        m_aDataSeq( rSequences )
{}

DataSource::~DataSource()
{}

// ____ XDataSource ____
Sequence< Reference< chart2::data::XLabeledDataSequence > > SAL_CALL DataSource::getDataSequences()
    throw (uno::RuntimeException, std::exception)
{
    return m_aDataSeq;
}

// ____ XDataSink ____
void SAL_CALL DataSource::setData( const Sequence< Reference< chart2::data::XLabeledDataSequence > >& aData )
    throw (uno::RuntimeException, std::exception)
{
    m_aDataSeq = aData;
}

Sequence< OUString > DataSource::getSupportedServiceNames_Static()
{
    Sequence<OUString> aServices { "com.sun.star.chart2.data.DataSource" };
    return aServices;
}

OUString SAL_CALL DataSource::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString DataSource::getImplementationName_Static()
{
    return OUString(lcl_aServiceName);
}

sal_Bool SAL_CALL DataSource::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DataSource::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

} // namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart_DataSource_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::DataSource(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
