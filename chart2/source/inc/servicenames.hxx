/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servicenames.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:14:30 $
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
#ifndef _CHART_SERVICENAMES_HXX
#define _CHART_SERVICENAMES_HXX

//.............................................................................
namespace chart
{
//.............................................................................

#define CHART_MODEL_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.ChartModel")
#define CHART_MODEL_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.ChartDocument")
//@todo create your own service containing the service com.sun.star.document.OfficeDocument

#define CHART_CONTROLLER_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.ChartController")
#define CHART_CONTROLLER_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.ChartController")
//@todo create your own service containing the service com.sun.star.frame.Controller

#define CHART_VIEW_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.ChartView")
#define CHART_VIEW_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.ChartView")

#define CHART_FRAMELOADER_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.ChartFrameLoader")
#define CHART_FRAMELOADER_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.frame.SynchronousFrameLoader")

#define CHART_WIZARD_DIALOG_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.WizardDialog")
#define CHART_WIZARD_DIALOG_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.WizardDialog")

#define CHART_TYPE_DIALOG_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.ChartTypeDialog")
#define CHART_TYPE_DIALOG_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.ChartTypeDialog")

// wrapper for old UNO API (com.sun.star.chart)
#define CHART_CHARTAPIWRAPPER_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.ChartDocumentWrapper")
#define CHART_CHARTAPIWRAPPER_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.ChartDocumentWrapper")

// accessibility
#define CHART2_ACCESSIBLE_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.AccessibleChartView")
#define CHART2_ACCESSIBLE_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.chart2.AccessibleChartView")

#define CHART_ACCESSIBLE_TEXT_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.AccessibleTextComponent")
#define CHART_ACCESSIBLE_TEXT_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.accessibility.AccessibleTextComponent")

#define CHART_RENDERER_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.ChartRenderer")

//.............................................................................
} //namespace chart
//.............................................................................

#endif
