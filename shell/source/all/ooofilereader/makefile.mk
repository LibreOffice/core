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
PRJNAME=shell
TARGET=ooofilereader
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(GUI)" != "OS2"
CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-D_WIN32_IE=0x501
CFLAGS_X64+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CFLAGS_X64+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS_X64+=-D_WIN32_IE=0x501
.ENDIF
.IF "$(GUI)" == "OS2"
CFLAGS+=-DISOLATION_AWARE_ENABLED -DXML_UNICODE -DXML_UNICODE_WCHAR_T -DUNICODE -D_UNICODE
.ENDIF

# --- Files --------------------------------------------------------


SLOFILES=$(SLO)$/basereader.obj\
         $(SLO)$/metainforeader.obj\
         $(SLO)$/contentreader.obj\
         $(SLO)$/simpletag.obj\
         $(SLO)$/keywordstag.obj\
         $(SLO)$/autostyletag.obj

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)
LIB1FILES=$(SLB)$/zipfile.lib\
          $(SLB)$/xmlparser.lib
          
.IF "$(BUILD_X64)"!=""
SLOFILES_X64=$(SLO_X64)$/basereader.obj\
         $(SLO_X64)$/metainforeader.obj\
         $(SLO_X64)$/contentreader.obj\
         $(SLO_X64)$/simpletag.obj\
         $(SLO_X64)$/keywordstag.obj\
         $(SLO_X64)$/autostyletag.obj

LIB1TARGET_X64=$(SLB_X64)$/$(TARGET).lib
LIB1OBJFILES_X64=$(SLOFILES_X64)
LIB1FILES_X64=$(SLB_X64)$/zipfile.lib\
          $(SLB_X64)$/xmlparser.lib
.ENDIF # "$(BUILD_X64)"!=""

# --- Targets ------------------------------------------------------

.INCLUDE :	set_wntx64.mk
.INCLUDE :	target.mk
.IF "$(OS)" == "WNT" 
INCLUDE!:=$(subst,/stl, $(INCLUDE))
 
.ENDIF
.INCLUDE :	tg_wntx64.mk

