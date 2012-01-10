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



UCP_VERSION=1
UCP_NAME=odma


PRJ=..$/..$/..
PRJNAME=ucb

TARGET=ucp$(UCP_NAME)
TARGET2=s$(UCP_NAME)

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

TARGET2TYPE=CUI
LIBTARGET=NO

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# --- General -----------------------------------------------------

SLOFILES=\
    $(SLO)$/odma_lib.obj    		\
    $(SLO)$/odma_services.obj    	\
    $(SLO)$/odma_provider.obj    	\
    $(SLO)$/odma_content.obj     	\
    $(SLO)$/odma_resultset.obj		\
    $(SLO)$/odma_datasupplier.obj	\
    $(SLO)$/odma_inputstream.obj	\
    $(SLO)$/odma_contentcaps.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCP_VERSION)
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)	\
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

# --- SODMA executable -------------------------------------------------
OBJFILES= $(OBJ)$/odma_main.obj 

APP2TARGET= $(TARGET2)
APP2OBJS=   $(OBJFILES)
APP2STDLIBS=$(SALLIB)			\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)

DEF2DES=UCB ODMA URL converter

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

