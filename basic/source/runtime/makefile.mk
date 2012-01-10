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

PRJNAME=basic
TARGET=runtime

ENABLE_EXCEPTIONS = TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk


# --- Allgemein -----------------------------------------------------------

SLOFILES=	\
    $(SLO)$/basrdll.obj	\
    $(SLO)$/comenumwrapper.obj	\
    $(SLO)$/inputbox.obj	\
    $(SLO)$/runtime.obj	\
    $(SLO)$/step0.obj	\
    $(SLO)$/step1.obj	\
    $(SLO)$/step2.obj	\
    $(SLO)$/iosys.obj	\
    $(SLO)$/stdobj.obj	\
    $(SLO)$/stdobj1.obj	\
    $(SLO)$/methods.obj	\
    $(SLO)$/methods1.obj	\
    $(SLO)$/props.obj	\
    $(SLO)$/ddectrl.obj	\
    $(SLO)$/dllmgr.obj \
    $(SLO)$/sbdiagnose.obj

.IF "$(GUI)$(COM)$(CPU)" == "WNTMSCI"
SLOFILES+=	$(SLO)$/wnt.obj
.ELIF "$(GUI)$(COM)$(CPU)" == "WNTGCCI"
SLOFILES+=	$(SLO)$/wnt-mingw.obj
.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

$(SLO)$/%.obj: %.s
#kendy: Cut'n'paste from bridges/source/cpp_uno/mingw_intel/makefile.mk
    $(CC) -c -o $(SLO)$/$(@:b).obj $<
    touch $@
