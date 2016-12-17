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
PRJNAME=avmedia
TARGET=avmedia
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Resources ---------------------------------

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res	
RESLIB1SRSFILES=			\
    $(SRS)$/viewer.srs		\
    $(SRS)$/framework.srs

# --- Files -------------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
    $(SLB)$/viewer.lib		\
    $(SLB)$/framework.lib	

# ==========================================================================

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=$(UNOTOOLSLIB) $(TOOLSLIB) $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB) $(VCLLIB) $(COMPHELPERLIB) $(SVTOOLLIB) $(SVLLIB) $(SFXLIB) $(VOSLIB)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib

DEF1NAME=$(SHL1TARGET)
DEF1DEPN=$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET)
DEF1DES=Avmedia
DEFLIB1NAME	=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

.IF "$(depend)"==""
$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo _Impl>$@
    @echo WEP>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
    @echo CT>>$@
.ENDIF

ALLTAR : $(MISC)/avmedia.component

$(MISC)/avmedia.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        avmedia.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt avmedia.component
