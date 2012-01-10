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

PRJNAME=soltools
TARGET=checkdll
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------


.IF "$(GUI)"=="UNX"
LIBSALCPPRT=$(0)
APP1TARGET	= 	checkdll
APP1OBJS	=	$(OBJ)$/checkdll.obj
DEPOBJFILES	=	$(APP1OBJS) 
.IF "$(OS)"!="FREEBSD" && "$(OS)"!="MACOSX" && "$(OS)"!="NETBSD"
STDLIB += -ldl
.ENDIF
.IF "$(OS)"=="NETBSD"
APP1STDLIBS	+= -Wl,--whole-archive -lgcc -Wl,--no-whole-archive
.ENDIF
.ENDIF # "$(GUI)"=="UNX"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
