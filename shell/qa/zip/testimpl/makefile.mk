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


PRJ=../../..

PRJNAME=shell
TARGET=qa_zipimpl
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
.IF "$(OS)" == "WNT"
    NO_DEFAULT_STL=TRUE
    USE_STLP_DEBUG=
.ENDIF
USE_DEFFILE=TRUE
# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
UWINAPILIB =

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1RPATH = NONE
.IF "$(OS)" == "WNT"
    SHL1STDLIBS = msvcprt.lib 
.ENDIF
SHL1LIBS = $(SOLARLIBDIR)$/zlib.lib $(SLB)$/zipfile.lib   
SLOFILES=$(SLO)$/testzipimpl.obj
SHL1TARGET = $(TARGET)
DEF1NAME=$(SHL1TARGET)
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.IF "$(OS)" == "WNT"
    INCLUDE!:=$(subst,/stl, $(INCLUDE))
.ENDIF

