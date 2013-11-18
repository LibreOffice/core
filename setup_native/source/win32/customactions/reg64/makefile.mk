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
TARGET=reg64msi

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE 

.INCLUDE : settings.mk

#Disable precompiled header
CDEFS+=-Dnot_used_define_to_disable_pch

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="WNT"

UWINAPILIB=

SLOFILES = \
    $(SLO)$/reg64.obj

STDSHL += \
    $(KERNEL32LIB)\
    $(USER32LIB)\
    $(ADVAPI32LIB)\
    $(SHELL32LIB)\
    $(MSILIB)\
    $(SHLWAPILIB)\
    
#	msvcprt.lib 							

SHL1TARGET = $(TARGET)
SHL1IMPLIB = i$(TARGET)

SHL1OBJS=$(SLOFILES) 
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1DEPN = $(SLB)$/$(TARGET).lib
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF


# --- mapimailer --------------------------------------------------------

#TARGETTYPE=CUI

#OBJFILES=   $(OBJ)$/reg64.obj

#APP1TARGET=reg64
#APP1OBJS=$(OBJFILES)
#APP1STDLIBS=$(KERNEL32LIB)\
#	$(ADVAPI32LIB)\
#	$(MSILIB)\
#   $(SHELL32LIB)\
#    msvcprt.lib\
#    $(OLE32LIB)\
#    $(COMCTL32LIB)\
#    $(UUIDLIB)

            
#APP1DEF=$(MISC)$/$(APP1TARGET).def 

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk
INCLUDE!:=$(subst,/stl, $(INCLUDE))

# -------------------------------------------------------------------------


