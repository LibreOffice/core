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



PRJ=..$/..$/

PRJNAME=salhelper
TARGET=dynloader
TARGET1=samplelib
TARGETTYPE=CUI
LIBTARGET=NO

NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE
BOOTSTRAP_SERVICE=FALSE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---


#RTTI on
.IF "$(OS)" == "WNT"
CFLAGS+= -GR
.ENDIF


# UNOTYPES= com.sun.star.lang.XInitialization \
#---------------------------------------------------------------------------
# Build the test library which is loaded by the 
# RealDynamicLoader

SLOFILES= \
        $(SLO)$/samplelib.obj

LIB1TARGET=$(SLB)$/$(TARGET1).lib
LIB1OBJFILES= \
        $(SLO)$/samplelib.obj


SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET1)
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME =$(TARGET1)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt

#DEF1EXPORTFILE=	exports.dxp

# ------------------------------------------------------------------------------

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)

APP1OBJS=	$(OBJ)$/loader.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)

#APP1LIBS=	$(LB)$/isalhelper.lib

.IF "$(OS)" == "WNT"
APP1STDLIBS+=   $(LB)$/isalhelper.lib
.ELSE
APP1STDLIBS+=   -lsalhelper$(UDK_MAJOR)$(COM)
.ENDIF

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo __CT>>$@


