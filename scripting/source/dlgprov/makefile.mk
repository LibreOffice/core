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

PRJNAME=scripting
TARGET=dlgprov
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

SLOFILES= \
        $(SLO)$/dlgprov.obj \
        $(SLO)$/DialogModelProvider.obj \
        $(SLO)$/dlgevtatt.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX).uno
SHL1IMPLIB=	i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
        $(SFX2LIB) \
        $(VCLLIB) \
        $(TOOLSLIB) \
        $(XMLSCRIPTLIB) \
        $(CPPUHELPERLIB) \
        $(COMPHELPERLIB) \
        $(UCBHELPERLIB) \
        $(VBAHELPERLIB)	\
        $(CPPULIB) \
        $(BASICLIB) \
        $(SALLIB)

SHL1DEPN=
SHL1LIBS=$(SLB)$/$(TARGET).lib

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

# ------------------------------------------------------------------

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@
 

ALLTAR : $(MISC)/dlgprov.component

$(MISC)/dlgprov.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dlgprov.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dlgprov.component
