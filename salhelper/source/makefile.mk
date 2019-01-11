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

PRJNAME=salhelper
TARGET=salhelper

ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
USE_DEFFILE=TRUE

.IF "$(OS)" != "WNT" && "$(GUI)"!="OS2" && "$(OS)" != "MACOSX"
UNIXVERSIONNAMES=UDK
.ENDIF # WNT

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=	\
    $(SLO)$/condition.obj \
    $(SLO)$/dynload.obj \
    $(SLO)$/simplereferenceobject.obj

.IF "$(GUI)" == "WNT"
SHL1TARGET=	$(TARGET)$(UDK_MAJOR)$(COMID)
.ELIF "$(GUI)" == "OS2"
SHL1TARGET=	salhelp$(UDK_MAJOR)
.ELSE
SHL1TARGET=	uno_$(TARGET)$(COMID)
.ENDIF

SHL1STDLIBS=$(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=  URELIB

DEF1NAME=	$(SHL1TARGET)

.IF "$(COMNAME)"=="msci"
SHL1VERSIONMAP=msci.map
.ELIF "$(COMNAME)"=="mscx"
SHL1VERSIONMAP=mscx.map
.ELIF "$(GUI)"=="OS2"
SHL1VERSIONMAP=gcc3os2.map
.ELIF "$(COMNAME)"=="sunpro5"
SHL1VERSIONMAP=sols.map
.ELIF "$(COMNAME)"=="gcc3"
SHL1VERSIONMAP=gcc3.map
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

