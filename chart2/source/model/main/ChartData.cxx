/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartData.cxx,v $
 * $Revision: 1.3 $
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

#include "ChartData.hxx"
#include "InternalDataProvider.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

ChartData::ChartData( const Reference< uno::XComponentContext > & xContext ) :
        m_xContext( xContext ),
        m_pInternalDataProvider( 0 )
{}

ChartData::~ChartData()
{}

void ChartData::setDataProvider(
    const Reference< chart2::data::XDataProvider > & xDataProvider ) throw()
{
    m_xDataProvider.set( xDataProvider );
    m_pInternalDataProvider = 0;
}

Reference< chart2::data::XDataProvider > ChartData::getDataProvider() const throw()
{
    return m_xDataProvider;
}

bool ChartData::createInternalData(
    bool bCloneOldData, const Reference< chart2::XChartDocument > & xChartDoc ) throw()
{
    if( hasInternalData() )
        return false;

    if( bCloneOldData )
        m_pInternalDataProvider = new InternalDataProvider( xChartDoc );
    else
        m_pInternalDataProvider = new InternalDataProvider();

    m_xDataProvider.set( m_pInternalDataProvider );
    return true;
}

bool ChartData::hasInternalData() const
{
    return (m_xDataProvider.is() && m_pInternalDataProvider != 0);
}

bool ChartData::createDefaultData() throw()
{
    if( hasInternalData() )
    {
        m_pInternalDataProvider->createDefaultData();
        return true;
    }
    return false;
}

} //  namespace chart
