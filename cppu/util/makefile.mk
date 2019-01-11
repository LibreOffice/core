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

PRJNAME=cppu
TARGET=cppu
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

.IF "$(OS)" != "WNT" && "$(OS)" != "OS2" && "$(OS)" != "MACOSX"
UNIXVERSIONNAMES=UDK
.ENDIF


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : ../source/helper/purpenv/export.mk

# --- Files --------------------------------------------------------

SHL1LIBS= \
    $(SLB)$/cppu_typelib.lib	\
    $(SLB)$/cppu_uno.lib		\
    $(SLB)$/cppu_threadpool.lib	\
    $(SLB)$/cppu_cppu.lib

.IF "$(GUI)" == "WNT" || "$(GUI)"=="OS2"
SHL1TARGET=$(TARGET)$(UDK_MAJOR)
.ELSE
SHL1TARGET= uno_$(TARGET)
.ENDIF

SHL1STDLIBS = $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=i$(TARGET)
.IF "$(OS)"!="FREEBSD"
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
.ENDIF

SHL1VERSIONMAP=$(TARGET).map
SHL1RPATH=URELIB

DEF1NAME=$(SHL1TARGET)


SHL2TARGET  := $(NAMEpurpenv_helper)
DEF2NAME    := $(SHL2TARGET)
.IF "$(GUI)$(COM)"=="WNTGCC"
SHL2VERSIONMAP:=uno_purpenvhelper$(COMID).map
.ELIF "$(GUI)$(COM)"=="WNTMSC"
SHL2VERSIONMAP:=$(SHL2TARGET)$(CPU).map
.ELIF "$(GUI)"=="OS2"
SHL2VERSIONMAP:=uno_purpenvhelperwntgcc.map
.ELSE
SHL2VERSIONMAP:=$(SHL2TARGET).map
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
SHL2DEF     := $(MISC)$/$(SHL2TARGET).def
SHL2IMPLIB  := i$(SHL2TARGET)
SHL2STDLIBS := $(CPPULIB) $(SALLIB)
SHL2RPATH   := URELIB
SHL2OBJS    := \
    $(SLO)$/helper_purpenv_Environment.obj 	\
    $(SLO)$/helper_purpenv_Mapping.obj      \
    $(SLO)$/helper_purpenv_Proxy.obj


# --- Targets ------------------------------------------------------

.PHONY: ALLTAR


ALLTAR:   $(SHL2TARGETN)
    $(MAKE) $(MAKECMDGOALS) -f extra.mk


.INCLUDE :	target.mk


$(SHL2TARGETN): $(SHL1TARGETN)

