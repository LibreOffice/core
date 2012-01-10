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

PRJNAME=basic
TARGET=classes
LIBTARGET=NO

# --- Settings -----------------------------------------------------------

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

ALLTAR .SEQUENTIAL : \
        $(MISC)$/$(TARGET).don \
        $(MISC)$/$(TARGET).slo

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(OUT)$/inc -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

$(MISC)$/$(TARGET).slo : $(SLOTARGET)
        echo $@

# --- Allgemein -----------------------------------------------------------

SLOFILES=	\
    $(SLO)$/sb.obj       \
    $(SLO)$/sbxmod.obj	\
    $(SLO)$/image.obj	\
    $(SLO)$/sbintern.obj	\
    $(SLO)$/sbunoobj.obj	\
    $(SLO)$/propacc.obj	\
    $(SLO)$/disas.obj \
    $(SLO)$/errobject.obj \
    $(SLO)$/eventatt.obj

OBJFILES=	\
    $(OBJ)$/sbintern.obj

SRS1NAME=$(TARGET)
SRC1FILES=	sb.src

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1OBJFILES = $(SLOFILES)

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

