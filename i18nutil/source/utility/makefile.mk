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

PRJNAME=i18nutil
TARGET=i18nutil
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
    $(SLO)$/casefolding.obj

SLOFILES= \
    $(EXCEPTIONSFILES) \
    $(SLO)$/unicode.obj \
    $(SLO)$/widthfolding.obj \
    $(SLO)$/oneToOneMapping.obj

# Unicode utilities 
.IF "$(GUI)" == "OS2"
SHL1TARGET=		$(TARGET)
.ELSE
SHL1TARGET=		$(TARGET)$(COMID)
.ENDIF
SHL1IMPLIB=		i$(TARGET)

DEF1DEPN=		$(MISC)$/$(SHL1TARGET).flt
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)
DEFLIB1NAME=	$(SHL1TARGET)

SHL1OBJS=		$(SLOFILES)

LIB1TARGET=	$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=$(SHL1OBJS)

SHL1STDLIBS= \
        $(CPPULIB) \
        $(SALLIB)

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo CLEAR_THE_FILE > $@
    @echo __CT >> $@

