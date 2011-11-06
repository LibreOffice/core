/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "DataSource.hxx"
#include "LabeledDataSequence.hxx"

using ::rtl::OUString;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.DataSource" ));
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
    throw (uno::RuntimeException)
{
    return m_aDataSeq;
}

// ____ XDataSink ____
void SAL_CALL DataSource::setData( const Sequence< Reference< chart2::data::XLabeledDataSequence > >& aData )
    throw (uno::RuntimeException)
{
    m_aDataSeq = aData;
}

// ================================================================================

Sequence< OUString > DataSource::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.data.DataSource" ));
    return aServices;
}

// ================================================================================

APPHELPER_XSERVICEINFO_IMPL( DataSource, lcl_aServiceName );

} // namespace chart
