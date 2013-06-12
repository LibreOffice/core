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



PRJ=..$/..$/..

PRJNAME=sal
TARGET=kill
LIBTARGET=NO
DYNAMIC_CRT=
ENABLE_EXCEPTIONS=TRUE
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(COM)"=="GCC"
CDEFS=-D_WIN32_WINNT=0x0501
.ENDIF

.IF "$(USE_SYSTEM_STL)" != "YES" && "$(PRODUCT)" = ""
CDEFS+=-D_DEBUG
.ENDIF

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

UWINAPILIB=

OBJFILES=\
    $(OBJ)$/kill.obj
    
APP1NOSAL=TRUE
APP1OBJS=$(OBJFILES)
APP1TARGET=$(TARGET)

STDLIB1=


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
