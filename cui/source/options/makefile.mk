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


PRJ=..$/..
PRJNAME=cui
TARGET=options
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_LAYOUT)" == "TRUE"
CFLAGS+= -DENABLE_LAYOUT=1 -I../$(PRJ)/layout/inc -I../$(PRJ)/layout/$(INPATH)/inc
.ENDIF # ENABLE_LAYOUT == TRUE

# --- Files --------------------------------------------------------

SRS1NAME=options
SRC1FILES =  \
        connpooloptions.src \
        dbregister.src \
        doclinkdialog.src \
        fontsubs.src \
        internationaloptions.src \
        optaccessibility.src \
        optasian.src \
        optchart.src \
        optcolor.src \
        optctl.src \
        optdict.src \
        optfltr.src \
        optgdlg.src \
        optgenrl.src \
        opthtml.src \
        optimprove.src \
        optinet2.src \
        optjava.src \
        optjsearch.src \
        optlingu.src \
        optmemory.src \
        optpath.src \
        optsave.src \
        optupdt.src \
        readonlyimage.src \
        securityoptions.src \
        treeopt.src \
        webconninfo.src \


SLOFILES+=\
        $(SLO)$/cfgchart.obj \
        $(SLO)$/connpoolconfig.obj \
        $(SLO)$/connpooloptions.obj \
        $(SLO)$/connpoolsettings.obj \
        $(SLO)$/cuisrchdlg.obj \
        $(SLO)$/dbregister.obj \
        $(SLO)$/dbregisterednamesconfig.obj \
        $(SLO)$/dbregistersettings.obj \
        $(SLO)$/doclinkdialog.obj \
        $(SLO)$/fontsubs.obj \
        $(SLO)$/internationaloptions.obj \
        $(SLO)$/optaccessibility.obj \
        $(SLO)$/optasian.obj \
        $(SLO)$/optchart.obj \
        $(SLO)$/optcolor.obj \
        $(SLO)$/optctl.obj \
        $(SLO)$/optdict.obj \
        $(SLO)$/optfltr.obj \
        $(SLO)$/optgdlg.obj \
        $(SLO)$/optgenrl.obj \
        $(SLO)$/optHeaderTabListbox.obj \
        $(SLO)$/opthtml.obj \
        $(SLO)$/optimprove.obj \
        $(SLO)$/optimprove2.obj \
        $(SLO)$/optinet2.obj \
        $(SLO)$/optjava.obj \
        $(SLO)$/optjsearch.obj \
        $(SLO)$/optlingu.obj \
        $(SLO)$/optmemory.obj \
        $(SLO)$/optpath.obj \
        $(SLO)$/optsave.obj \
        $(SLO)$/optupdt.obj \
        $(SLO)$/radiobtnbox.obj \
        $(SLO)$/readonlyimage.obj \
        $(SLO)$/sdbcdriverenum.obj \
        $(SLO)$/securityoptions.obj \
        $(SLO)$/treeopt.obj \
        $(SLO)$/webconninfo.obj \


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
