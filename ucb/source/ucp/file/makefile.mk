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
TARGET=ucpfile
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# Version
UCPFILE_MAJOR=1

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

SLOFILES=\
    $(SLO)$/prov.obj      \
    $(SLO)$/bc.obj        \
    $(SLO)$/shell.obj     \
    $(SLO)$/filtask.obj   \
    $(SLO)$/filrow.obj    \
    $(SLO)$/filrset.obj   \
    $(SLO)$/filid.obj     \
    $(SLO)$/filnot.obj    \
    $(SLO)$/filprp.obj    \
    $(SLO)$/filinpstr.obj \
    $(SLO)$/filstr.obj    \
    $(SLO)$/filcmd.obj    \
    $(SLO)$/filglob.obj   \
    $(SLO)$/filinsreq.obj \
    $(SLO)$/filrec.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

SHL1TARGET=$(TARGET)$(UCPFILE_MAJOR)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(UCBHELPERLIB)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

.IF "$(GUI)" == "OS2"
DEF1EXPORTFILE=exports2.dxp
.ENDIF

DEF1NAME=$(SHL1TARGET)
.ENDIF # L10N_framework

.INCLUDE: target.mk


ALLTAR : $(MISC)/ucpfile1.component

$(MISC)/ucpfile1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpfile1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpfile1.component
