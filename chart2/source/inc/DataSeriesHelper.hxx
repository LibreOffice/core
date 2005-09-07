/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeriesHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:39:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_DATASERIESHELPER_HXX
#define CHART2_DATASERIESHELPER_HXX

#ifndef _COM_SUN_STAR_CHART2_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/XDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASOURCE_HPP_
#include <com/sun/star/chart2/XDataSource.hpp>
#endif

namespace chart
{

namespace DataSeriesHelper
{
    /** Retrieves the data sequence in the given data source that matches the
        given role.  If more than one sequences match the role, the first match
        is returned.  If no sequence matches, an empty reference is returned.

        @param aSource
            The data source containing all data sequences to be searched through.

        @param aRole
            The role that is to be filtered out.
     */
::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSequence >
    getDataSequenceByRole( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::chart2::XDataSource > & xSource,
                           ::rtl::OUString aRole );

} //  namespace DataSeriesHelper
} //  namespace chart

// CHART2_DATASERIESHELPER_HXX
#endif
