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

PRJNAME=embeddedobj
TARGET=embobj
#LIBTARGET=NO
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

LIB1OBJFILES= \
        $(SLO)$/miscobj.obj\
        $(SLO)$/specialobject.obj\
        $(SLO)$/persistence.obj\
        $(SLO)$/embedobj.obj\
        $(SLO)$/inplaceobj.obj\
        $(SLO)$/visobj.obj\
        $(SLO)$/dummyobject.obj\
        $(SLO)$/xcreator.obj\
        $(SLO)$/xfactory.obj\
        $(SLO)$/register.obj\
        $(SLO)$/docholder.obj\
        $(SLO)$/intercept.obj

LIB1TARGET=$(SLB)$/$(TARGET).lib

SHL1TARGET= $(TARGET)

SHL1STDLIBS=\
    $(SALLIB)\
    $(CPPULIB)\
    $(COMPHELPERLIB)\
    $(CPPUHELPERLIB)

SHL1DEPN=
SHL1IMPLIB= i$(TARGET)

SHL1LIBS=	$(LIB1TARGET)

SHL1DEF= $(MISC)$/$(SHL1TARGET).def

DEF1NAME= $(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(LIB1TARGET)

DEFLIB1NAME=$(TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo __CT				>>$@


ALLTAR : $(MISC)/embobj.component

$(MISC)/embobj.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        embobj.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt embobj.component
