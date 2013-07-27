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



PRJ=..$/..$/..$/..
PRJNAME=shell
TARGET=propsheets
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE
CDEFS+=-U_WIN32_WINNT -D_WIN32_WINNT=0x0501 -U_WIN32_IE -D_WIN32_IE=0x501
.IF "$(COM)"!="GCC"
CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CFLAGS_X64+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE
CFLAGS_X64+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS_X64+=-D_WIN32_WINNT=0x0501 -D_WIN32_IE=0x501
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/propsheets.obj\
    $(SLO)$/listviewbuilder.obj\
    $(SLO)$/document_statistic.obj

.IF "$(BUILD_X64)"!=""
SLOFILES_X64=$(SLO_X64)$/propsheets.obj\
    $(SLO_X64)$/listviewbuilder.obj\
    $(SLO_X64)$/document_statistic.obj
.ENDIF # "$(BUILD_X64)"!=""

# --- Targets ------------------------------------------------------

.INCLUDE :	set_wntx64.mk
.INCLUDE :	target.mk
INCLUDE!:=$(subst,/stl, $(INCLUDE))

.INCLUDE :	tg_wntx64.mk
