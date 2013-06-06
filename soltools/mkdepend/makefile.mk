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



PRJNAME=soltools
TARGET=make_makedepend
PRJ=..
TARGETTYPE=CUI
LIBTARGET=NO
# noadjust here to have dependencies over there
noadjust=TRUE
nodep=true
ENABLE_EXCEPTIONS=TRUE

# "mkdepend" is written in K&R style C. Modern compilers will generate
# lots of warning. There is no point in cleaning this up, so we just
# ignore warnings
EXTERNAL_WARNINGS_NOT_ERRORS=TRUE

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE : settings.mk

LIBSALCPPRT=
UWINAPILIB=

CDEFS+=-DNO_X11 -DXP_PC -DHW_THREADS  

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

.IF "$(OS)"=="SOLARIS"
#APP1STDLIBS+=-lstlport
APP1STDLIBS+=-lCstd
.ENDIF


.INCLUDE : target.mk


