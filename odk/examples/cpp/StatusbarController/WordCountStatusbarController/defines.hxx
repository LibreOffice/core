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

#ifndef GENERAL_DEFINES_HXX
#define GENERAL_DEFINES_HXX

#include "macros.hxx"

#ifndef STATUSBAR_CONTROLLER_IMPL_NAME
#error  You must define STATUSBAR_CONTROLLER_IMPL_NAME
#endif

#ifndef STATUSBAR_CONTROLLER_SERVICE_NAME
#error  You must define STATUSBAR_CONTROLLER_SERVICE_NAME
#endif

#ifndef PROTOCOL_HANDLER_IMPL_NAME
#error  You must define PROTOCOL_HANDLER_IMPL_NAME
#endif

#ifndef PROTOCOL_HANDLER_SERVICE_NAME
#error  You must define PROTOCOL_HANDLER_SERVICE_NAME
#endif

#ifndef PROTOCOL_HANDLER_URL_PROTOCOL
#error  You must define PROTOCOL_HANDLER_URL_PROTOCOL
#endif

#define OUSTR_COMMAND_UNO_WORDCOUNTDLG  C2U(".uno:WordCountDialog")
#define OUSTR_COMMAND_STATUSWORDCOUNT   C2U(PROTOCOL_HANDLER_URL_PROTOCOL ":StatusWordCount" )


#define OUSTR_SERVICENAME_BREAKITERATOR             C2U("com.sun.star.i18n.BreakIterator")
#define OUSTR_SERVICENAME_URLTRANSFORMER            C2U("com.sun.star.util.URLTransformer")
#define OUSTR_SERVICENAME_PROTOCOLHANDLER           C2U("com.sun.star.frame.ProtocolHandler")
#define OUSTR_SERVICENAME_MODULEMANAGER             C2U("com.sun.star.frame.ModuleManager")
#define OUSTR_SERVICENAME_CALLBACK                  C2U("com.sun.star.awt.AsyncCallback")

// OpenOffice.org Modules
#define STR_MODULE_CHARTDOCUMENT            "com.sun.star.chart2.ChartDocument"
#define STR_MODULE_DRAWINGDOCUMENT          "com.sun.star.drawing.DrawingDocument"
#define STR_MODULE_FORMULAPROPERTIES        "com.sun.star.formula.FormulaProperties"
#define STR_MODULE_BIBLIOGRAPHY             "com.sun.star.frame.Bibliography"
#define STR_MODULE_STARTMODULE              "com.sun.star.frame.StartModule"
#define STR_MODULE_PRESENTATIONDOCUMENT     "com.sun.star.presentation.PresentationDocument"
#define STR_MODULE_REPORTDEFINITION         "com.sun.star.report.ReportDefinition"
#define STR_MODULE_BASICIDE                 "com.sun.star.script.BasicIDE"
#define STR_MODULE_DATASOURCEBROWSER        "com.sun.star.sdb.DataSourceBrowser"
#define STR_MODULE_FORMDESIGN               "com.sun.star.sdb.FormDesign"
#define STR_MODULE_OFFICEDATABASEDOCUMENT   "com.sun.star.sdb.OfficeDatabaseDocument"
#define STR_MODULE_QUERYDESIGN              "com.sun.star.sdb.QueryDesign"
#define STR_MODULE_RELATIONDESIGN           "com.sun.star.sdb.RelationDesign"
#define STR_MODULE_TABLEDATAVIEW            "com.sun.star.sdb.TableDataView"
#define STR_MODULE_TABLEDESIGN              "com.sun.star.sdb.TableDesign"
#define STR_MODULE_TEXTREPORTDESIGN         "com.sun.star.sdb.TextReportDesign"
#define STR_MODULE_VIEWDESIGN               "com.sun.star.sdb.ViewDesign"
#define STR_MODULE_SPREADSHEETDOCUMENT      "com.sun.star.sheet.SpreadsheetDocument"
#define STR_MODULE_GLOBALDOCUMENT           "com.sun.star.text.GlobalDocument"
#define STR_MODULE_TEXTDOCUMENT             "com.sun.star.text.TextDocument"
#define STR_MODULE_WEBDOCUMENT              "com.sun.star.text.WebDocument"
#define STR_MODULE_XMLFORMDOCUMENT          "com.sun.star.xforms.XMLFormDocument"

#endif
