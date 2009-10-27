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
#include "ChartModelHelper.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

ChartData::ChartData( const Reference< uno::XComponentContext > & xContext ) :
        m_xContext( xContext ),
        m_xDataProvider( 0 ),
        m_xInternalDataProvider( 0 )
{}

ChartData::~ChartData()
{}

void ChartData::setDataProvider(
    const Reference< chart2::data::XDataProvider > & xDataProvider ) throw()
{
    m_xDataProvider.set( xDataProvider );
    m_xInternalDataProvider.clear();
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
        m_xInternalDataProvider = ChartModelHelper::createInternalDataProvider( xChartDoc );
    else
        m_xInternalDataProvider = ChartModelHelper::createInternalDataProvider();

    m_xDataProvider.set( m_xInternalDataProvider );
    return true;
}

bool ChartData::hasInternalData() const
{
    return (m_xDataProvider.is() && m_xInternalDataProvider.is());
}

bool ChartData::createDefaultData() throw()
{
    if( hasInternalData() )
    {
        uno::Reference< lang::XInitialization > xIni(m_xInternalDataProvider,uno::UNO_QUERY);
        if ( xIni.is() )
        {
            uno::Sequence< uno::Any > aArgs(1);
            beans::NamedValue aParam(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CreateDefaultData")),uno::makeAny(sal_True));
            aArgs[0] <<= aParam;
            xIni->initialize(aArgs);
            return true;
        }
    }
    return false;
}

} //  namespace chart
