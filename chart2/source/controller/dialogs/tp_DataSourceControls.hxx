/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_DataSourceControls.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:45:40 $
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
#ifndef CHART2_TP_DATASOURCECONTROLS_HXX
#define CHART2_TP_DATASOURCECONTROLS_HXX

#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <com/sun/star/chart2/XChartType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XLABELEDDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#endif

// header for class SvTabListBox
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif
// header for class SvTreeListBox
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

namespace chart
{

class SeriesEntry : public ::SvLBoxEntry
{
public:
    virtual ~SeriesEntry();

    /// the corresponding data series
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >
        m_xDataSeries;

    /// the chart type that contains the series (via XDataSeriesContainer)
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >
        m_xChartType;
};

class SeriesListBox : public ::SvTreeListBox
{
public:
    explicit SeriesListBox( Window* pParent, const ResId & rResId );
    ~SeriesListBox( );

    virtual SvLBoxEntry* CreateEntry() const;
};

} //  namespace chart

// CHART2_TP_DATASOURCECONTROLS_HXX
#endif
