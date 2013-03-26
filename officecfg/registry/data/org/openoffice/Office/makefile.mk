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
TARGET=data_ooOffice
PACKAGE=org.openoffice.Office

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCUFILES= \
    Accelerators.xcu \
    Calc.xcu \
    Canvas.xcu \
    Common.xcu \
    Compatibility.xcu \
    DataAccess.xcu \
    Embedding.xcu \
    ExtensionDependencies.xcu \
    ExtensionManager.xcu \
    Impress.xcu	\
    Jobs.xcu \
    Labels.xcu \
    Linguistic.xcu \
    Logging.xcu \
    Math.xcu \
    ProtocolHandler.xcu \
    Security.xcu \
    Scripting.xcu \
    SFX.xcu \
    TableWizard.xcu \
    UI.xcu \
    Views.xcu \
    WebWizard.xcu \
    FormWizard.xcu \
    Writer.xcu \
    Paths.xcu \
    Histories.xcu

MODULEFILES= \
    Accelerators-macosx.xcu \
    Accelerators-unxwnt.xcu \
    Common-brand.xcu \
    Common-writer.xcu   \
    Common-calc.xcu   \
    Common-draw.xcu   \
    Common-impress.xcu   \
    Common-base.xcu   \
    Common-math.xcu   \
    Common-unx.xcu   \
    Common-unixdesktop.xcu \
    Common-gconflockdown.xcu \
    Common-macosx.xcu   \
    Common-wnt.xcu   \
    Common-UseOOoFileDialogs.xcu \
    Linguistic-ForceDefaultLanguage.xcu \
    Scripting-python.xcu   \
    Common-cjk.xcu   \
    Common-ctl.xcu   \
    Common-korea.xcu   \
    Paths-macosx.xcu    \
    Paths-unxwnt.xcu    \
    Paths-unixdesktop.xcu \
    Paths-internallibtextcatdata.xcu \
    Paths-externallibtextcatdata.xcu \
    Writer-cjk.xcu \
    Impress-ogltrans.xcu \
    Embedding-calc.xcu   \
    Embedding-chart.xcu   \
    Embedding-draw.xcu   \
    Embedding-impress.xcu   \
    Embedding-math.xcu   \
    Embedding-base.xcu   \
    Embedding-writer.xcu \
    UI-brand.xcu \
    Recovery-gconflockdown.xcu

LOCALIZEDFILES= \
    Accelerators.xcu \
    Common.xcu \
    SFX.xcu \
    DataAccess.xcu \
    TableWizard.xcu \
    UI.xcu \
    Embedding.xcu \
    WebWizard.xcu \
    FormWizard.xcu \
    Writer.xcu


.INCLUDE :  target.mk

