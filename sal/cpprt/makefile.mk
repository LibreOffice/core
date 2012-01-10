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

PRJNAME=SAL
TARGET=salcpprt

ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

SLOFILES =	\
    $(SLO)$/operators_new_delete.obj


.IF "$(OS)" != "SOLARIS"

# build as archive
LIB1TARGET=$(LB)$/$(TARGET).lib
LIB1ARCHIV=$(LB)$/lib$(TARGET).a
LIB1OBJFILES=$(SLOFILES)

.ELSE  # SOLARIS

# build as shared library (interposer needed for -Bdirect)
LINKFLAGS+= -z interpose

SHL1TARGET=	$(TARGET)
SHL1IMPLIB=	i$(TARGET)

SHL1STDLIBS=$(SALLIB)
SHL1OBJS=	$(SLOFILES)

.ENDIF # SOLARIS

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

