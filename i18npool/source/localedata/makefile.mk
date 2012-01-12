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


PRJNAME=i18npool
TARGET=locale

TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)" == "SOLARIS"
# the xmlparser build breaks in this header file
# 
#      /opt/SUNWspro/WS6U1/include/CC/Cstd/./limits
#
# which defines a class with member functions called min() and max().
# the build breaks because in solar.h, there is something like this
# 
#      #define max(x,y) ((x) < (y) ? (y) : (x))
#      #define min(x,y) ((x) < (y) ? (x) : (y))
#
# so the only choice is to prevent "CC/Cstd/./limits" from being 
# included:
CDEFS+=-D__STD_LIMITS
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=   $(SLO)$/localedata.obj

OBJFILES   = \
        $(OBJ)$/saxparser.obj \
        $(OBJ)$/LocaleNode.obj \
        $(OBJ)$/filewriter.obj

APP1TARGET = saxparser
APP1RPATH = NONE

APP1OBJS   = $(OBJFILES)

APP1STDLIBS = \
        $(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB)

DEPOBJFILES = \
        $(OBJ)$/saxparser.obj \
        $(OBJ)$/LocaleNode.obj \
        $(OBJ)$/filewriter.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

