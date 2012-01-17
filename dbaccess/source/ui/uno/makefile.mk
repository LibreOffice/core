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
PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=uiuno

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        dbinteraction.src \
        copytablewizard.src

# ... object files ............................
# workaround for issue http://qa.openoffice.org/issues/show_bug.cgi?id=102305 Linux specific
.IF "$(COM)" == "GCC"
NOOPTFILES=\
        $(SLO)$/copytablewizard.obj
.ENDIF

SLOFILES=	\
        $(SLO)$/copytablewizard.obj \
        $(SLO)$/composerdialogs.obj	\
        $(SLO)$/unosqlmessage.obj	\
        $(SLO)$/unoadmin.obj	\
        $(SLO)$/admindlg.obj	\
        $(SLO)$/TableFilterDlg.obj	\
        $(SLO)$/AdvancedSettingsDlg.obj	\
        $(SLO)$/unoDirectSql.obj	\
        $(SLO)$/DBTypeWizDlg.obj	\
        $(SLO)$/DBTypeWizDlgSetup.obj	\
        $(SLO)$/UserSettingsDlg.obj \
        $(SLO)$/ColumnModel.obj	\
        $(SLO)$/ColumnControl.obj	\
        $(SLO)$/ColumnPeer.obj	\
        $(SLO)$/dbinteraction.obj \
        $(SLO)$/textconnectionsettings_uno.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
