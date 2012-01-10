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



PRJ=..

PRJNAME=idlc
TARGET=idlc
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE
INCPRE=$(MISC)

.IF "$(GUI)" == "OS2"
STL_OS2_BUILDING=1
.ENDIF

# --- Settings -----------------------------------------------------


.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CXXFILES=   \
            wrap_scanner.cxx \
            wrap_parser.cxx	\
            idlcmain.cxx	\
            idlc.cxx	\
            idlccompile.cxx	\
            idlcproduce.cxx	\
            errorhandler.cxx	\
            options.cxx	\
            fehelper.cxx	\
            astdeclaration.cxx \
            astscope.cxx \
            aststack.cxx \
            astdump.cxx \
            astinterface.cxx \
            aststruct.cxx \
            aststructinstance.cxx \
            astoperation.cxx \
            astconstant.cxx \
            astenum.cxx \
            astarray.cxx \
            astunion.cxx \
            astexpression.cxx \
            astservice.cxx

YACCTARGET=$(MISC)$/parser.cxx
YACCFILES=parser.y

OBJFILES=   \
            $(OBJ)$/wrap_scanner.obj	\
            $(OBJ)$/wrap_parser.obj	\
            $(OBJ)$/idlcmain.obj	\
            $(OBJ)$/idlc.obj	\
            $(OBJ)$/idlccompile.obj	\
            $(OBJ)$/idlcproduce.obj	\
            $(OBJ)$/errorhandler.obj	\
            $(OBJ)$/options.obj	\
            $(OBJ)$/fehelper.obj	\
            $(OBJ)$/astdeclaration.obj	\
            $(OBJ)$/astscope.obj	\
            $(OBJ)$/aststack.obj	\
            $(OBJ)$/astdump.obj	\
            $(OBJ)$/astinterface.obj	\
            $(OBJ)$/aststruct.obj	\
            $(OBJ)$/aststructinstance.obj \
            $(OBJ)$/astoperation.obj	\
            $(OBJ)$/astconstant.obj	\
            $(OBJ)$/astenum.obj	\
            $(OBJ)$/astarray.obj	\
            $(OBJ)$/astunion.obj	\
            $(OBJ)$/astexpression.obj \
            $(OBJ)$/astservice.obj

APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJFILES)

APP1RPATH=SDK

APP1STDLIBS = \
    $(REGLIB) \
    $(SALLIB) \
    $(SALHELPERLIB)

# --- Targets ------------------------------------------------------

.IF "$(debug)" == ""
YACCFLAGS+=-l
.ELSE
YACCFLAGS+=-v
.ENDIF
    
.INCLUDE :  target.mk

$(MISC)$/stripped_scanner.ll : scanner.ll
    tr -d "\015" < scanner.ll > $(MISC)$/stripped_scanner.ll

$(MISC)$/scanner.cxx:	$(MISC)$/stripped_scanner.ll
    flex -o$(MISC)$/scanner.cxx $(MISC)$/stripped_scanner.ll

$(OBJ)$/wrap_parser.obj: $(MISC)$/parser.cxx
$(OBJ)$/wrap_scanner.obj: $(MISC)$/scanner.cxx
