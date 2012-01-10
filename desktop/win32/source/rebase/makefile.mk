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

PRJNAME=desktop
TARGET=rebasegui
LIBTARGET=NO
TARGETTYPE=GUI
UWINAPILIB=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

OBJFILES = \
    $(OBJ)$/rebase.obj	\
    $(OBJ)$/rebasegui.obj

ULFFILES=	rebasegui.ulf

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

RCFILES=		$(RES)$/$(TARGET).rc

# --- Targets ------------------------------------------------------

APP1NOSAL=		TRUE
APP1TARGET=		rebaseoo

APP1STDLIBS=	$(SHELL32LIB) $(SOLARLIBDIR)$/pathutils-obj.obj
.IF "$(COM)"=="GCC"
    APP1STDLIBS+=$(PSDK_HOME)$/lib$/imagehlp.lib
.ELSE
    APP1STDLIBS+=imagehlp.lib
.ENDIF

APP1OBJS=		$(OBJ)$/rebase.obj

APP1RPATH=		BRAND

APP2NOSAL=		TRUE
APP2TARGET=		rebasegui 
APP2STDLIBS=	$(SHELL32LIB) $(SOLARLIBDIR)$/pathutils-obj.obj

APP2OBJS=		$(OBJ)$/rebasegui.obj

APP2DEF=		$(MISC)$/$(TARGET).def
APP2RPATH=		BRAND

APP2RES=		$(RES)$/$(TARGET).res
APP2NOSVRES=	$(RES)$/$(TARGET).res

# --- setup --------------------------------------------------------------

.INCLUDE :  target.mk

$(RCFILES) : $(ULFDIR)$/rebasegui.ulf makefile.mk rcfooter.txt rcheader.txt rctmpl.txt
    $(LNGCONVEX) -ulf $(ULFDIR)$/rebasegui.ulf -rc $(RCFILES) -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt
