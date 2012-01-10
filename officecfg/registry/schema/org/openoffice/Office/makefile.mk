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


PRJ=..$/..$/..$/..$/..

PRJNAME=officecfg
TARGET=schema_ooOffice
PACKAGE=org.openoffice.Office

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCSFILES= \
    Accelerators.xcs \
    Addons.xcs \
    Calc.xcs \
    CalcAddIns.xcs \
    Canvas.xcs \
    Chart.xcs \
    Commands.xcs \
    Common.xcs \
    Compatibility.xcs \
    DataAccess.xcs \
    Draw.xcs \
    Events.xcs \
    Embedding.xcs \
    ExtensionDependencies.xcs \
    ExtensionManager.xcs \
    Impress.xcs \
    Java.xcs \
    Jobs.xcs \
    Labels.xcs \
    Linguistic.xcs \
    Logging.xcs \
    Math.xcs \
    OptionsDialog.xcs \
    ProtocolHandler.xcs \
    SFX.xcs \
    Substitution.xcs \
    TableWizard.xcs \
    UI.xcs \
    Views.xcs \
    Writer.xcs \
    WriterWeb.xcs \
    Scripting.xcs \
    Security.xcs \
    WebWizard.xcs \
    Recovery.xcs \
    FormWizard.xcs \
    ExtendedColorScheme.xcs \
    TypeDetection.xcs \
    TabBrowse.xcs \
    Paths.xcs \
    Histories.xcs \
    OOoImprovement.xcs

.INCLUDE :  target.mk

