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

PRJNAME=basctl
TARGET=basicide
VISIBILITY_HIDDEN=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=ide_pch
PROJECTPCHSOURCE=ide_pch

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(basicdebug)" != "" || "$(BASICDEBUG)" != ""
CDEFS+=-DBASICDEBUG
.ENDIF


# --- Allgemein ----------------------------------------------------------

EXCEPTIONSFILES=$(SLO)$/basicrenderable.obj  \
                $(SLO)$/scriptdocument.obj  \
                $(SLO)$/basicbox.obj	\
                $(SLO)$/basidesh.obj	\
                $(SLO)$/basides1.obj	\
                $(SLO)$/basides2.obj	\
                $(SLO)$/basides3.obj	\
                $(SLO)$/baside2.obj		\
                $(SLO)$/baside3.obj		\
                $(SLO)$/basobj2.obj		\
                $(SLO)$/basobj3.obj		\
                $(SLO)$/bastypes.obj	\
                $(SLO)$/bastype2.obj	\
                $(SLO)$/bastype3.obj	\
                $(SLO)$/iderdll.obj		\
                $(SLO)$/macrodlg.obj	\
                $(SLO)$/moduldlg.obj	\
                $(SLO)$/moduldl2.obj	\
                $(SLO)$/unomodel.obj	\
                $(SLO)$/register.obj	\
                $(SLO)$/basdoc.obj	    \
                $(SLO)$/tbxctl.obj		\
                $(SLO)$/basidectrlr.obj	\
                $(SLO)$/localizationmgr.obj \
                $(SLO)$/doceventnotifier.obj \
                $(SLO)$/docsignature.obj \
                $(SLO)$/documentenumeration.obj

SLOFILES =  $(EXCEPTIONSFILES) \
            $(SLO)$/baside2b.obj	\
            $(SLO)$/brkdlg.obj	\
            $(SLO)$/objdlg.obj	\

SRS1NAME=$(TARGET)
SRC1FILES=	basidesh.src macrodlg.src moptions.src moduldlg.src objdlg.src brkdlg.src basicprint.src

.INCLUDE :  target.mk

$(INCCOM)$/dllname.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
        echo \#define DLL_NAME \"libbasctl$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
        echo \#define DLL_NAME \"basctl$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ENDIF

$(SLO)$/basiclib.obj : $(INCCOM)$/dllname.hxx

$(INCCOM)$/basicide.hrc: basidesh.hrc
    @-$(COPY) basidesh.hrc $@

