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

PRJNAME=automation
TARGET=automation
TARGETTYPE=GUI

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=                 \
    $(SLB)$/server.lib     \
    $(SLB)$/simplecm.lib   \
    $(SLB)$/communi.lib

# --- sts library ---------------------------------------------------

SHL1TARGET= sts$(DLLPOSTFIX)
SHL1IMPLIB= ists

SHL1STDLIBS= \
            $(TOOLSLIB) \
            $(UNOTOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)       \
            $(VCLLIB) \
            $(BASICLIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(COMPHELPERLIB) \
            $(SOTLIB)


.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        $(ADVAPI32LIB)	\
        $(GDI32LIB)
.ENDIF

SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=       $(SLB)$/$(TARGET).lib

DEF1NAME        =$(SHL1TARGET)
DEF1DEPN        =       \
    $(MISC)$/$(SHL1TARGET).flt

DEFLIB1NAME     =$(TARGET)
DEF1DES         =TestToolServer

# --- simple communication library (no vcl) ---------------------------------------------------

SHL2TARGET= simplecm$(DLLPOSTFIX)
SHL2IMPLIB= isimplecm

SHL2STDLIBS= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)


SHL2DEF=        $(MISC)$/$(SHL2TARGET).def
SHL2LIBS=       $(SLB)$/simplecm.lib 

DEF2NAME        =$(SHL2TARGET)
DEF2DEPN        =       \
    $(MISC)$/$(SHL2TARGET).flt

DEFLIB2NAME     =simplecm
DEF2DES         =SimpleCommunication

# --- communication library ---------------------------------------------------

SHL3TARGET= communi$(DLLPOSTFIX)
SHL3IMPLIB= icommuni

SHL3STDLIBS= \
            $(TOOLSLIB) \
            $(SVLLIB)       \
            $(VCLLIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(VOSLIB) \
            $(SIMPLECMLIB)


.IF "$(GUI)"=="WNT"
SHL3STDLIBS+= \
        $(ADVAPI32LIB)	\
        $(GDI32LIB)
.ENDIF

SHL3DEF=        $(MISC)$/$(SHL3TARGET).def
SHL3LIBS=       $(SLB)$/communi.lib 
SHL3DEPN=$(SHL2TARGETN)

DEF3NAME        =$(SHL3TARGET)
DEF3DEPN        =       \
    $(MISC)$/$(SHL3TARGET).flt

DEFLIB3NAME     =communi
DEF3DES         =Communication


# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

# --- Basic-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo Sbi >> $@
    @echo exception >> $@
    @echo bad_alloc >> $@
    @echo __CT >> $@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo Sbi >> $@
    @echo exception >> $@
    @echo bad_alloc >> $@
    @echo __CT >> $@

$(MISC)$/$(SHL3TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo Sbi >> $@
    @echo exception >> $@
    @echo bad_alloc >> $@
    @echo __CT >> $@


# ------------------------------------------------------------------------

