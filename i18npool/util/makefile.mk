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

PRJNAME=i18npool
TARGET=i18npool.uno

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =
.INCLUDE :	$(PRJ)/version.mk

# --- Allgemein ----------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/defaultnumberingprovider.lib	\
        $(SLB)$/nativenumber.lib		\
        $(SLB)$/registerservices.lib		\
        $(SLB)$/numberformatcode.lib		\
        $(SLB)$/locale.lib			\
        $(SLB)$/indexentry.lib			\
        $(SLB)$/calendar.lib		\
        $(SLB)$/breakiterator.lib \
        $(SLB)$/transliterationImpl.lib \
        $(SLB)$/characterclassification.lib \
        $(SLB)$/collator.lib \
        $(SLB)$/inputchecker.lib \
        $(SLB)$/textconversion.lib \
        $(SLB)$/ordinalsuffix.lib

SHL1TARGET= $(TARGET)
# WNT needs implib name even if there is none
SHL1IMPLIB= i$(SHL1TARGET)

SHL1DEPN=	makefile.mk \
        $(SLB)/i18nisolang$(ISOLANG_MAJOR)$(COMID).lib
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1LIBS=$(LIB1TARGET)
SHL1STDLIBS= \
        $(I18NUTILLIB) \
        $(I18NISOLANGLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(ICUINLIB) \
        $(ICUUCLIB) \
        $(ICUDATALIB)

# --- Targets ------------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/i18npool.component

$(MISC)/i18npool.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        i18npool.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt i18npool.component
