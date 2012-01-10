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
PRJNAME=ucb
TARGET=ucpftp
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# Version
UCPFTP_MAJOR=1


# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# --- General -----------------------------------------------------
.IF "$(L10N_framework)"==""
# first target ( shared library )

SLOFILES1=\
    $(SLO)$/ftpservices.obj  \
    $(SLO)$/ftpcontentprovider.obj  \
    $(SLO)$/ftpcontent.obj   \
    $(SLO)$/ftpcontentidentifier.obj   \
    $(SLO)$/ftpcontentcaps.obj \
    $(SLO)$/ftpdynresultset.obj  \
    $(SLO)$/ftpresultsetbase.obj \
    $(SLO)$/ftpresultsetI.obj \
    $(SLO)$/ftploaderthread.obj  \
    $(SLO)$/ftpinpstr.obj	\
    $(SLO)$/ftpdirp.obj     \
    $(SLO)$/ftpcfunc.obj     \
    $(SLO)$/ftpurl.obj     \
    $(SLO)$/ftpintreq.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES1)

# --- Shared-Library 1 ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCPFTP_MAJOR)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(UCBHELPERLIB) \
    $(CURLLIB)

.IF "$(GUI)" == "OS2"
SHL1STDLIBS+=ssl.lib crypto.lib libz.lib
.ENDIF

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS= \
    $(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

.ENDIF # L10N_framework
.INCLUDE: target.mk














ALLTAR : $(MISC)/ucpftp1.component

$(MISC)/ucpftp1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpftp1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpftp1.component
