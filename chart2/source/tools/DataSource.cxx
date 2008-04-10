/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataSource.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

DataSource::DataSource(
    const Sequence< Reference< chart2::data::XDataSequence > > & rSequences )
{
    m_aDataSeq.realloc( rSequences.getLength());
    sal_Int32 i;
    for(i=rSequences.getLength(); --i;)
    {
        m_aDataSeq[i].set( new LabeledDataSequence( rSequences[i] ));
    }
}

DataSource::DataSource(
    const Sequence< Reference< chart2::data::XDataSequence > > & rSequences,
    const Sequence< Reference< chart2::data::XDataSequence > > & rLabel )
{
    m_aDataSeq.realloc( rSequences.getLength());
    for( sal_Int32 i=0; i<rSequences.getLength(); ++i )
    {
        if( i < rLabel.getLength())
            m_aDataSeq[i].set( new LabeledDataSequence( rSequences[i], rLabel[i] ));
        else
            m_aDataSeq[i].set( new LabeledDataSequence( rSequences[i],
                                                        Reference< chart2::data::XDataSequence >() ));
    }
}

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
