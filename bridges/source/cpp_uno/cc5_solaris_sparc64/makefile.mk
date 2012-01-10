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



PRJ := ..$/..$/..
PRJNAME := bridges
TARGET := sunpro5_uno
ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

.IF "$(COM)" == "C52" && "$(CPU)" == "U"

SHL1TARGET = $(TARGET)
SHL1OBJS = \
    $(SLO)$/callvirtualmethod.obj \
    $(SLO)$/cpp2uno.obj \
    $(SLO)$/exceptions.obj \
    $(SLO)$/fp.obj \
    $(SLO)$/isdirectreturntype.obj \
    $(SLO)$/uno2cpp.obj \
    $(SLO)$/vtableslotcall.obj
SHL1LIBS = $(SLB)$/cpp_uno_shared.lib
SHL1STDLIBS = $(CPPULIB) $(SALLIB)
SHL1RPATH = URELIB
SHL1VERSIONMAP = ..$/..$/bridge_exports.map
SHL1IMPLIB= i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.ENDIF

.INCLUDE: target.mk

$(SLO)$/%.obj: %.s
    CC -m64 -KPIC -c -o $(SLO)$/$(@:b).o $<
    touch $@
