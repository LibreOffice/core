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

PRJNAME=dtrans
TARGET=generic
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO
COMP1TYPELIST=dtrans

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""
# ------------------------------------------------------------------

SLOFILES= \
        $(SLO)$/generic_clipboard.obj \
        $(SLO)$/clipboardmanager.obj \
        $(SLO)$/dtrans.obj

SHL1TARGET= dtrans

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 

SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET)
SHL1OBJS=		$(SLOFILES)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------
.ENDIF # L10N_framework

.INCLUDE :	target.mk

ALLTAR : $(MISC)/dtrans.component

$(MISC)/dtrans.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dtrans.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dtrans.component
