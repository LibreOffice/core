#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************


PRJ=..$/..$/..$/..$/..$/..

PRJNAME=officecfg
TARGET=data_ooOUI
PACKAGE=org.openoffice.Office.UI

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------


LOCALIZEDFILES= \
    BasicIDECommands.xcu \
    BibliographyCommands.xcu \
    CalcCommands.xcu \
    ChartCommands.xcu \
    ChartWindowState.xcu \
    DbuCommands.xcu \
    BaseWindowState.xcu \
    WriterFormWindowState.xcu \
    WriterReportWindowState.xcu \
    DbQueryWindowState.xcu \
    DbTableWindowState.xcu \
    DbRelationWindowState.xcu \
    DbBrowserWindowState.xcu \
    DbTableDataWindowState.xcu \
    DrawImpressCommands.xcu \
    Effects.xcu \
    GenericCommands.xcu \
    MathCommands.xcu \
    StartModuleCommands.xcu \
    BasicIDEWindowState.xcu \
    CalcWindowState.xcu \
    DrawWindowState.xcu \
    ImpressWindowState.xcu \
    MathWindowState.xcu \
    StartModuleWindowState.xcu \
    WriterWindowState.xcu \
    XFormsWindowState.xcu \
    WriterGlobalWindowState.xcu \
    WriterWebWindowState.xcu \
    WriterCommands.xcu\
    GenericCategories.xcu\
    Sidebar.xcu

XCUFILES= \
    $(LOCALIZEDFILES) \
    Controller.xcu \
    Factories.xcu \
    GlobalSettings.xcu

.INCLUDE :  target.mk

