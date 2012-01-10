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

PRJNAME=	sal
TARGET=		unloading
TARGET1=samplelib1
TARGET2=samplelib2
#LIBTARGET=NO
TARGETTYPE=CUI
COMP1TYPELIST=$(TARGET1)

NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE

USE_DEFFILE=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

CPPUMAKERFLAGS =

.IF "$(BOOTSTRAP_SERVICE)" == "TRUE"
UNOUCROUT=	$(OUT)$/inc$/bootstrap
INCPRE+=	$(OUT)$/inc$/bootstrap
.ELSE
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF
UNOUCROUT=	$(OUT)$/inc$/light
INCPRE+=	$(OUT)$/inc$/light
.ENDIF


#SLOFILES=	\
#		$(SLO)$/samplelib1.obj	\
#		$(SLO)$/samplelib2.obj

LIB1TARGET=$(SLB)$/$(TARGET1).lib
LIB1OBJFILES= \
        $(SLO)$/samplelib1.obj

SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)



SHL1DEPN=
#SHL1IMPLIB=	i$(TARGET1)
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1EXPORTFILE=	exports.dxp
DEF1NAME=	$(SHL1TARGET)
#-------------------------------------------------------

LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2OBJFILES= \
        $(SLO)$/samplelib2.obj

SHL2TARGET=	$(TARGET2)

SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2DEPN=
SHL2LIBS=	$(SLB)$/$(TARGET2).lib
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2EXPORTFILE=	exports.dxp
DEF2NAME=	$(SHL2TARGET)

# ------------------------------------------------------------------

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)

APP1OBJS=	$(OBJ)$/unloadTest.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

