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
PRJNAME=unotools
TARGET=utl
TARGETTYPE=CUI
USE_LDUMP2=TRUE

USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk

# --- Library -----------------------------------

LIB1TARGET=$(SLB)$/untools.lib
LIB1FILES=\
        $(SLB)$/i18n.lib \
        $(SLB)$/misc.lib \
        $(SLB)$/streaming.lib \
        $(SLB)$/config.lib \
        $(SLB)$/ucbhelp.lib \
        $(SLB)$/procfact.lib \
        $(SLB)$/property.lib \
        $(SLB)$/accessibility.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1IMPLIB=iutl
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
        $(SALHELPERLIB) \
        $(COMPHELPERLIB) \
        $(UCBHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(I18NISOLANGLIB) \
        $(TOOLSLIB) \
        $(VOSLIB) \
        $(SALLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1DEPN=$(LIB1TARGET)

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME     =untools
DEF1DES         =unotools

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo CLEAR_THE_FILE		> $@
    @echo _TI					>> $@
    @echo _real					>> $@
    @echo NodeValueAccessor			>> $@
    @echo SubNodeAccess				>> $@
    @echo UpdateFromConfig			>> $@
    @echo UpdateToConfig				>> $@
        @echo _Impl >> $@


ALLTAR : $(MISC)/utl.component

$(MISC)/utl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        utl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt utl.component
