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

PRJNAME=ucb
TARGET=ucbdemo
TARGETTYPE=GUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

OBJFILES=\
    $(OBJ)$/srcharg.obj \
    $(OBJ)$/ucbdemo.obj

#SRSFILES= $(SRS)$/ucbdemo.srs

#
# UCBDEMO
#
APP1TARGET=	ucbdemo
APP1OBJS=\
    $(OBJ)$/srcharg.obj \
    $(OBJ)$/ucbdemo.obj
#APP1RES=	$(RES)$/ucbdemo.res

.IF "$(COMPHELPERLIB)"==""

.IF "$(GUI)" == "UNX"
COMPHELPERLIB=-licomphelp2
.ENDIF # unx

.IF "$(GUI)"=="WNT"
COMPHELPERLIB=icomphelp2.lib
.ENDIF # wnt

.ENDIF

APP1STDLIBS=\
    $(SALLIB) \
    $(VOSLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
    $(TOOLSLIB) \
    $(SVTOOLLIB) \
    $(VCLLIB) \
    $(UCBHELPERLIB)

# --- Targets ---

.INCLUDE : target.mk

