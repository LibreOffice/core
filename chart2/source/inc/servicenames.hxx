/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#define CHART_ACCESSIBLE_TEXT_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.AccessibleTextComponent")
#define CHART_ACCESSIBLE_TEXT_SERVICE_NAME ::rtl::OUString::createFromAscii("com.sun.star.accessibility.AccessibleTextComponent")

#define CHART_RENDERER_SERVICE_IMPLEMENTATION_NAME ::rtl::OUString::createFromAscii("com.sun.star.comp.chart2.ChartRenderer")

//.............................................................................
} //namespace chart
//.............................................................................

#endif
