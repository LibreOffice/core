#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJNAME=soltools
TARGET=make_makedepend
PRJ=..
TARGETTYPE=CUI
LIBTARGET=NO
# noadjust here to have dependencies over there
noadjust=TRUE
nodep=true
ENABLE_EXCEPTIONS=TRUE

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE : settings.mk

.IF "$(CROSS_COMPILING)"=="YES"

all:
# nothing

.ENDIF

LIBSALCPPRT=
UWINAPILIB=

CDEFS+=-DNO_X11 -DXP_PC -DHW_THREADS  

.IF "$(COM)" == "MSC"
# C4100: unreferenced formal parameter
# C4131: uses old-style declarator
# C4242: conversion from 'int' to 'char', possible loss of data
# C4706: assignment within conditional expression
CDEFS+=-wd4100 -wd4131 -wd4242 -wd4706
.ENDIF

OBJFILES=  \
        $(OBJ)$/cppsetup.obj \
        $(OBJ)$/ifparser.obj \
        $(OBJ)$/include.obj \
        $(OBJ)$/main.obj \
        $(OBJ)$/parse.obj \
        $(OBJ)$/pr.obj \
        $(OBJ)$/collectdircontent.obj \
        $(NULL)

APP1TARGET=makedepend
APP1OBJS=$(OBJFILES)
APP1RPATH=NONE

.IF "$(COM)"=="MSC"
APP1STDLIBS+=msvcprt.lib
.ENDIF                        # "$(COM)"=="MSC"

.IF "$(OS)$(COM)"=="SOLARISC52"
#APP1STDLIBS+=-lstlport
APP1STDLIBS+=-lCstd
.ENDIF


.INCLUDE : target.mk


