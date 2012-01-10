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

PRJNAME=	salhelper
TARGET=		rtti
TARGET1=samplelibrtti
LIBTARGET=NO
TARGETTYPE=CUI


ENABLE_EXCEPTIONS=TRUE

USE_DEFFILE=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

#RTTI on
.IF "$(OS)" == "WNT"
CFLAGS+= -GR
.ENDIF

SLOFILES=	\
        $(SLO)$/samplelibrtti.obj

LIB1TARGET=$(SLB)$/$(TARGET1).lib
LIB1OBJFILES= \
        $(SLO)$/samplelibrtti.obj

SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)


SHL1DEPN=
SHL1IMPLIB=	i$(TARGET1)
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1EXPORTFILE=	exports.dxp

DEF1NAME=	$(SHL1TARGET)

.IF "$(OS)$(CPU)"=="SOLARISS"
SHL1VERSIONMAP=	sols.map
.ELIF "$(OS)$(CPU)"=="LINUXI"
SHL1VERSIONMAP= lngi.map
.ELIF "$(OS)$(CPU)$(COMNAME)" == "GCCFREEBSDIgcc2"
SHL1VERSIONMAP= gcc2_freebsd_intel.map
.ELIF "$(OS)$(CPU)$(COMNAME)" == "GCCFREEBSDIgcc3"
SHL1VERSIONMAP= gcc3_freebsd_intel.map
.ENDIF


# ------------------------------------------------------------------

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)

APP1OBJS=	$(OBJ)$/rttitest.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)

.IF "$(OS)" == "WNT"
APP1STDLIBS+=	$(LB)$/isamplelibrtti.lib
.ELSE
APP1STDLIBS+=	-lsamplelibrtti
.ENDIF 

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

