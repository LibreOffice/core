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



PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=lngpckinsthlp


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE

.INCLUDE : settings.mk

#Disable precompiled header
CDEFS+=-Dnot_used_define_to_disable_pch

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="WNT"

.IF "$(USE_SYSTEM_STL)" != "YES" && "$(PRODUCT)" = ""
CDEFS+=-D_DEBUG
.ENDIF

UWINAPILIB=

SLOFILES =	$(SLO)$/lngpckinsthelper.obj \
            $(SLO)$/respintest.obj \
            $(SLO)$/checkrunningofficelanguagepack.obj

STDSHL+=	\
    $(ADVAPI32LIB) \
    $(MSILIB)      \
    $(SHELL32LIB)								

SHL1OBJS = $(SLOFILES)               \
    $(SLO)$/seterror.obj

.IF "$(COM)"=="GCC"
STDSHL+=	\
    $(KERNEL32LIB)\
    -lmsvcrt
.ENDIF

#SHL1LIBS = $(SLB)$/$(TARGET).lib 

SHL1OBJS = $(SLOFILES)              \
    $(SLO)$/registerextensions.obj  \
    $(SLO)$/seterror.obj

SHL1TARGET = $(TARGET)
SHL1IMPLIB = i$(TARGET)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1DEPN = $(SLB)$/$(TARGET).lib
SHL1BASE = 0x1c000000
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------


