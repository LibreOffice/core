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

PRJNAME          :=cppu
TARGET           :=cppu
ENABLE_EXCEPTIONS:=TRUE
NO_BSYMBOLIC     :=TRUE
USE_DEFFILE      :=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : ../source/helper/purpenv/export.mk

# --- Files --------------------------------------------------------

SHL3TARGET  := unsafe_uno_uno
SHL3IMPLIB  := i$(SHL3TARGET)
SHL3STDLIBS := $(purpenv_helper_LIB) $(SALLIB) 
SHL3OBJS    := $(SLO)$/UnsafeBridge.obj
.IF "$(GUI)"=="OS2"
SHL3VERSIONMAP=unsafe_os2.map
SHL3DEF=$(MISC)$/$(SHL3TARGET).def
DEF3NAME=$(SHL3TARGET)
.ELSE
SHL3DEF     := empty.def
.ENDIF
SHL3OBJS    := $(SLO)$/UnsafeBridge.obj
SHL3RPATH   := URELIB

SHL4TARGET  := affine_uno_uno
SHL4IMPLIB  := i$(SHL4TARGET)
SHL4STDLIBS := $(purpenv_helper_LIB) $(SALLIB) 
SHL4OBJS    := $(SLO)$/AffineBridge.obj
.IF "$(GUI)"=="OS2"
SHL4VERSIONMAP=unsafe_os2.map
SHL4DEF=$(MISC)$/$(SHL4TARGET).def
DEF4NAME=$(SHL4TARGET)
.ELSE
SHL4DEF     := empty.def
.ENDIF
SHL4OBJS    := $(SLO)$/AffineBridge.obj
SHL4RPATH   := URELIB


SHL5TARGET  := log_uno_uno
SHL5IMPLIB  := i$(SHL5TARGET)
SHL5STDLIBS := $(purpenv_helper_LIB) $(SALLIB) $(CPPULIB)
SHL5OBJS    := $(SLO)$/LogBridge.obj
                
.IF "$(GUI)"=="OS2"
SHL5VERSIONMAP=unsafe_os2.map
SHL5DEF=$(MISC)$/$(SHL5TARGET).def
DEF5NAME=$(SHL5TARGET)
.ELSE
SHL5DEF     := empty.def
.ENDIF
SHL5RPATH   := URELIB

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

