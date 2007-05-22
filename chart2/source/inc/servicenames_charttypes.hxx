/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servicenames_charttypes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:25:49 $
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
#ifndef _CHART2_SERVICENAMES_CHARTTYPES_HXX
#define _CHART2_SERVICENAMES_CHARTTYPES_HXX

//.............................................................................
namespace chart
{
//.............................................................................

#define CHART2_SERVICE_NAME_CHARTTYPE_AREA ::rtl::OUString::createFromAscii("com.sun.star.chart2.AreaChartType")
#define CHART2_SERVICE_NAME_CHARTTYPE_BAR ::rtl::OUString::createFromAscii("com.sun.star.chart2.BarChartType")
#define CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ::rtl::OUString::createFromAscii("com.sun.star.chart2.ColumnChartType")
#define CHART2_SERVICE_NAME_CHARTTYPE_LINE ::rtl::OUString::createFromAscii("com.sun.star.chart2.LineChartType")
#define CHART2_SERVICE_NAME_CHARTTYPE_SCATTER ::rtl::OUString::createFromAscii("com.sun.star.chart2.ScatterChartType")
#define CHART2_SERVICE_NAME_CHARTTYPE_PIE ::rtl::OUString::createFromAscii("com.sun.star.chart2.PieChartType")
#define CHART2_SERVICE_NAME_CHARTTYPE_NET ::rtl::OUString::createFromAscii("com.sun.star.chart2.NetChartType")
#define CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK ::rtl::OUString::createFromAscii("com.sun.star.chart2.CandleStickChartType")

//.............................................................................
} //namespace chart
//.............................................................................
#endif
