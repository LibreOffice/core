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

PRJNAME=store
TARGET=store
TARGETTYPE=CUI

USE_LDUMP2=TRUE
USE_DEFFILE=TRUE

NO_BSYMBOLIC=TRUE
NO_DEFAULT_STL=TRUE

.IF "$(OS)" != "MACOSX"
UNIXVERSIONNAMES=UDK
.ENDIF

# --- Settings ---

.INCLUDE : settings.mk

# --- Debug-Library ---

.IF "$(debug)" != ""

LIB1TARGET=	$(LB)$/$(TARGET)dbg.lib
LIB1ARCHIV=	$(LB)$/lib$(TARGET)dbg.a
LIB1FILES=	$(LB)$/store.lib

.ENDIF # debug

# --- Shared-Library ---

SHL1TARGET= 	$(TARGET)
SHL1IMPLIB= 	istore

SHL1VERSIONMAP=	$(TARGET).map

SHL1STDLIBS=	$(SALLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/store.lib
SHL1RPATH=  URELIB

# --- Def-File ---

DEF1NAME=	$(SHL1TARGET)
DEF1DES=Store

# --- Targets ---

.INCLUDE : target.mk



