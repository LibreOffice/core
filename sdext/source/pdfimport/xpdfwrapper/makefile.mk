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

PRJNAME=sdext
TARGET=xpdfimport
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

.IF "$(ENABLE_PDFIMPORT)" == "NO" || "$(SYSTEM_POPPLER)" != "YES"
@all:
    @echo "PDF Import extension disabled."
.ENDIF

CFLAGS += $(POPPLER_CFLAGS)

# --- Files --------------------------------------------------------
UWINAPILIB:=
APP1TARGET=$(TARGET)
APP1LIBSALCPPRT=
APP1OBJS= \
    $(OBJ)$/wrapper_gpl.obj $(OBJ)/pdfioutdev_gpl.obj $(OBJ)/pnghelper.obj

APP1STDLIBS+=$(ZLIB3RDLIB)

APP1STDLIBS+=$(POPPLER_LIBS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
