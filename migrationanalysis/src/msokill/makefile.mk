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



PRJ=..$/..

PRJNAME		=	migrationanalysis
TARGET		=	msokill
LIBTARGET	=	NO
TARGETTYPE	=	CUI

ENABLE_EXCEPTIONS	=	TRUE

# --- Settings ------------------------------------------------------------

.INCLUDE : settings.mk

# --- Allgemein -----------------------------------------------------------

OBJFILES= 	$(OBJ)$/msokill.obj \
            $(OBJ)$/StdAfx.obj

# --- Targets ------------------------------------------------------

UWINAPILIB=
LIBSALCPPRT=

APP1NOSAL=		TRUE
APP1TARGET=		msokill

#APP1STDLIB
STDLIB1=msi.lib\
    shell32.lib\
    oleaut32.lib\
    gdi32.lib\
    comdlg32.lib\
    advapi32.lib\
    comctl32.lib\
    shlwapi.lib\
    oledlg.lib\
    ole32.lib\
    uuid.lib\
    oleacc.lib\
    winspool.lib\

APP1OBJS=		$(OBJFILES)

PAW_RES_DIR:=$(BIN)$/ProAnalysisWizard$/Resources
PAW_RES_EXE:=$(PAW_RES_DIR)$/$(APP1TARGET).exe

# --- setup --------------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(PAW_RES_EXE)

$(PAW_RES_EXE) : $(BIN)$/$$(@:f)
    -$(MKDIRHIER) $(@:d)
    $(COPY) $(BIN)$/$(@:f) $@

