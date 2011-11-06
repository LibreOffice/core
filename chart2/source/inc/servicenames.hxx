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
