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

PRJNAME=idl
TARGET=idl

TARGETTYPE=CUI

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------------

LIB1TARGET= $(LB)$/$(TARGET).lib
LIB1FILES = $(LB)$/prj.lib		 \
            $(LB)$/objects.lib	 \
            $(LB)$/cmptools.lib

APP1TARGET= svidl
APP1RPATH=NONE

APP1STDLIBS=	\
    $(TOOLSLIB)	\
    $(SALLIB)

APP1LIBS= $(LB)$/$(TARGET).lib
.IF "$(GUI)" != "UNX"
.IF "$(COM)"!="GCC"
APP1OBJS=	$(OBJ)$/svidl.obj	\
            $(OBJ)$/command.obj
.ENDIF
.ENDIF

.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD"
APP1STDLIBS+=-lcrypt
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

