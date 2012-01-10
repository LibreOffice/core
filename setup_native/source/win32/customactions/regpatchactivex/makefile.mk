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
TARGET=regpatchactivex
USE_DEFFILE=TRUE

.IF "$(GUI)"=="WNT"

# --- Settings -----------------------------------------------------

# NO_DEFAULT_STL=TRUE
ENABLE_EXCEPTIONS=TRUE

.INCLUDE : settings.mk

STDSHL=
# SOLARINC!:=$(SOLARINC:s/stl//)

# --- Files --------------------------------------------------------

INCPRE+=.\Include
.DIRCACHE=NO
# CFLAGS+=-E

SLOFILES =	$(SLO)$/regpatchactivex.obj

.IF "$(COM)"=="GCC"
SHL1STDLIBS += -lstdc++
.IF "$(MINGW_GCCLIB_EH)"=="YES"
SHL1STDLIBS += -lgcc_eh
.ENDIF
SHL1STDLIBS += -lgcc -lmingw32 -lmoldname -lmsvcrt
.ELSE
SHL1STDLIBS=
.ENDIF

SHL1STDLIBS+=	$(KERNEL32LIB)\
                $(USER32LIB)\
                $(ADVAPI32LIB)\
                $(SHELL32LIB)\
                $(MSILIB)
.IF "$(COM)"!="GCC"
SHL1STDLIBS+=	libcmt.lib
.ENDIF


SHL1LIBS = $(SLB)$/$(TARGET).lib 

#SHL1TARGET = $(TARGET)$(DLLPOSTFIX)
SHL1TARGET = $(TARGET)
SHL1IMPLIB = i$(TARGET)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1DEPN = $(SLB)$/$(TARGET).lib
SHL1BASE = 0x1c000000
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------


.ENDIF

