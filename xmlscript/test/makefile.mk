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

PRJNAME=xmlscript
TARGET=imexp
TARGETTYPE=GUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CXXFILES= \
        imexp.cxx

OBJFILES= \
        $(OBJ)$/imexp.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES)
APP1STDLIBS= \
        $(TOOLSLIB)			\
        $(SOTLIB)			\
        $(SVTOOLLIB)			\
        $(COMPHELPERLIB)		\
        $(VCLLIB)			\
        $(CPPULIB)			\
        $(CPPUHELPERLIB)		\
        $(SALLIB)			\
        $(XMLSCRIPTLIB)

APP1DEF=	$(MISC)$/imexp.def

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


# ------------------------------------------------------------------
# Windows
# ------------------------------------------------------------------

.IF "$(GUI)" == "WIN"

$(MISC)$/imexp.def: makefile.mk
    echo  NAME			imexp			>$@
    echo  DESCRIPTION	'XML dialog im-/ exporter'     	>>$@
    echo  EXETYPE		WINDOWS 			>>$@
    echo  STUB		'winSTUB.EXE'           	>>$@
    echo  PROTMODE						>>$@
    echo  CODE		PRELOAD MOVEABLE DISCARDABLE	>>$@
    echo  DATA		PRELOAD MOVEABLE MULTIPLE	>>$@
    echo  HEAPSIZE		8192				>>$@
    echo  STACKSIZE 	32768				>>$@

.ENDIF
