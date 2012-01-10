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



# @@@ UCP Version - Increase, if your UCP libraray becomes incompatible.
UCP_VERSION=1

# @@@ Name for your UCP. Will become part of the library name (See below).
UCP_NAME=myucp

# @@@ Relative path to project root.
PRJ=..$/..

# @@@ Name of the project your UCP code recides it.
PRJNAME=ucbhelper

TARGET=ucp$(UCP_NAME)$(UCP_VERSION).uno

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk
.INCLUDE :	../../version.mk

# --- General -----------------------------------------------------

# no "lib" prefix
DLLPRE =

# @@@ Adjust template file names. Add own files here.
SLOFILES=\
    $(SLO)$/myucp_services.obj    	\
    $(SLO)$/myucp_provider.obj    	\
    $(SLO)$/myucp_content.obj     	\
    $(SLO)$/myucp_contentcaps.obj   \
    $(SLO)$/myucp_resultset.obj    	\
    $(SLO)$/myucp_datasupplier.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# @@@ Add additional libs here.
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB) \
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

