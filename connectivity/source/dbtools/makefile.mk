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
PRJNAME=connectivity
TARGET=dbt
USE_LDUMP2=TRUE

TARGETTYPE=CUI
USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/dbtools.pmk
.INCLUDE :  $(PRJ)$/version.mk

LDUMP=ldump2.exe

# --- Library -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
        $(SLB)$/sdbcx.lib		\
        $(SLB)$/commontools.lib \
        $(SLB)$/sql.lib			\
        $(SLB)$/simpledbt.lib   \
        $(SLB)$/cnr.lib

# --- dynamic library ---------------------------

SHL1TARGET=	$(DBTOOLS_TARGET)$(DLLPOSTFIX)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
    $(JVMACCESSLIB)				\
    $(UNOTOOLSLIB)              \
    $(TOOLSLIB)                 \
    $(COMPHELPERLIB)            \
    $(I18NISOLANGLIB)

# NETBSD: somewhere we have to instantiate the static data members.
# NETBSD-1.2.1 doesn't know about weak symbols so the default mechanism for GCC won't work.
# SCO and MACOSX: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="NETBSD" || "$(OS)"=="SCO" || "$(OS)$(COM)"=="OS2GCC" || "$(OS)"=="MACOSX"
SHL1STDLIBS+=$(UCBHELPERLIB)
.ENDIF

#SHL1DEPN=
SHL1IMPLIB=	idbtools

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(LIB1TARGET)
            
DEFLIB1NAME=$(TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- filter file ------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo _TI				>$@
    @echo _real				>>$@


ALLTAR : $(MISC)/dbtools.component

$(MISC)/dbtools.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dbtools.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dbtools.component
