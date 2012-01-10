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



PRJ=..

PRJNAME=soldep
TARGET=bootstrpdt
ENABLE_EXCEPTIONS=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# fixme, code is not yet 64 bit clean
.IF "$(OS)$(CPU)"=="LINUXX" || ("$(COM)"=="C52" && "$(CPU)"=="U")
all:
    @echo nothing to do
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
                  $(SLO)$/prj.obj

SLOFILES=\
          $(SLO)$/command.obj \
          $(SLO)$/dep.obj \
          $(SLO)$/minormk.obj \
          $(SLO)$/prodmap.obj \
          $(SLO)$/appdef.obj \
           $(SLO)$/hashtbl.obj		\
           $(SLO)$/sstring.obj		\
          $(SLO)$/prj.obj


SHL1TARGET	=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB	=$(TARGET)
SHL1LIBS	=$(SLB)$/$(TARGET).lib
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1DEPN	=$(SHL1LIBS)
SHL1STDLIBS=\
            $(TOOLSLIB) \
            $(VOSLIB)			\
            $(SALLIB)   

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME	=$(TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo __CT >> $@
