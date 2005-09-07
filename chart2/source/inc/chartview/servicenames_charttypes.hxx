/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servicenames_charttypes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:49:46 $
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

#define CHART2_VIEW_BARCHART_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.BarChart_Impl")
#define CHART2_VIEW_BARCHART_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.BarChart")

#define CHART2_VIEW_PIECHART_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.PieChart_Impl")
#define CHART2_VIEW_PIECHART_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.PieChart")

#define CHART2_VIEW_AREACHART_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.AreaChart_Impl")
#define CHART2_VIEW_AREACHART_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.AreaChart")

//.............................................................................
} //namespace chart
//.............................................................................
#endif
