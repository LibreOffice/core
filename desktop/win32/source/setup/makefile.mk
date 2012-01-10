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

PRJNAME		=	desktop
TARGET		=	loader2
LIBTARGET	=	NO
DYNAMIC_CRT	=
TARGETTYPE	=	GUI

ENABLE_EXCEPTIONS	=	TRUE

# --- Settings ------------------------------------------------------------

.INCLUDE : settings.mk

# --- Allgemein -----------------------------------------------------------

INCPRE+=$(MFC_INCLUDE)

.IF "$(USE_STLP_DEBUG)"!=""
CDEFS+=-D_DEBUG
.ENDIF # "$(USE_STLP_DEBUG)"!=""

RCFILES=	$(RES)$/$(TARGET).rc

ULFFILES=	setup.ulf

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

OBJFILES= 	$(OBJ)$/setup_main.obj \
            $(OBJ)$/setup_a.obj \
            $(OBJ)$/setup_w.obj

# --- Targets ------------------------------------------------------
# Generate the native Windows resource file
# using lngconvex.exe 

UWINAPILIB=		$(0)
LIBSALCPPRT=	$(0)

APP1NOSAL=		TRUE
APP1TARGET=		loader2

APP1STDLIBS=	$(GDI32LIB) $(ADVAPI32LIB) $(SHELL32LIB) $(MSILIB)
.IF "$(COM)"!="GCC"
APP1STDLIBS+=	libcmt.lib
.ENDIF
APP1OBJS=		$(OBJFILES)

APP1DEF=		$(MISC)$/$(TARGET).def

APP1RES=		$(RES)$/$(TARGET).res
APP1NOSVRES=	$(RES)$/$(TARGET).res

# --- setup --------------------------------------------------------------

.INCLUDE :  target.mk

$(RCFILES) : $(ULFDIR)$/setup.ulf makefile.mk rcfooter.txt rcheader.txt rctmpl.txt
    $(LNGCONVEX) -ulf $(ULFDIR)$/setup.ulf -rc $(RCFILES) -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt

