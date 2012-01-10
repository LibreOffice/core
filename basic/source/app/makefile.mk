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
TARGET=app

LIBTARGET = NO

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk

# --- Common ------------------------------------------------------------

OBJFILES = \
    $(OBJ)$/ttbasic.obj	\
    $(OBJ)$/basicrt.obj	\
    $(OBJ)$/processw.obj    \
    $(OBJ)$/process.obj     \
    $(OBJ)$/brkpnts.obj     \
    $(OBJ)$/mybasic.obj     \
    $(OBJ)$/status.obj      \
    $(OBJ)$/printer.obj     \
    $(OBJ)$/appwin.obj      \
    $(OBJ)$/appedit.obj     \
    $(OBJ)$/appbased.obj    \
    $(OBJ)$/apperror.obj    \
    $(OBJ)$/textedit.obj    \
    $(OBJ)$/msgedit.obj     \
    $(OBJ)$/dialogs.obj     \

EXCEPTIONSFILES = \
    $(OBJ)$/app.obj \
    $(OBJ)$/printer.obj     \
    $(OBJ)$/process.obj

.IF "$(GUI)" == "WNT"
EXCEPTIONSFILES += \
    $(OBJ)$/process.obj
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES = \
    basic.src				\
    ttmsg.src				\
    basmsg.src				\
    svtmsg.src				\
    testtool.src

LIB1TARGET=$(LB)$/app.lib
LIB1ARCHIV=$(LB)$/libapp.a
LIB1OBJFILES = \
        $(OBJ)$/basicrt.obj	\
        $(OBJ)$/processw.obj     \
        $(OBJ)$/process.obj      \
        $(OBJ)$/brkpnts.obj      \
        $(OBJ)$/app.obj          \
        $(OBJ)$/mybasic.obj	\
        $(OBJ)$/status.obj       \
        $(OBJ)$/printer.obj      \
        $(OBJ)$/appwin.obj       \
        $(OBJ)$/appedit.obj      \
        $(OBJ)$/appbased.obj     \
        $(OBJ)$/apperror.obj     \
        $(OBJ)$/textedit.obj     \
        $(OBJ)$/msgedit.obj      \
        $(OBJ)$/dialogs.obj      \
        $(OBJ)$/sbintern.obj

# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk

$(OBJ)$/dialogs.obj : $(INCCOM)$/_version.h

