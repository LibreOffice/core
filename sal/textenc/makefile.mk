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



PRJ = ..
PRJNAME = sal
TARGET = textenc

.INCLUDE: settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

SLOFILES = \
    $(SLO)$/context.obj \
    $(SLO)$/convertbig5hkscs.obj \
    $(SLO)$/converter.obj \
    $(SLO)$/converteuctw.obj \
    $(SLO)$/convertgb18030.obj \
    $(SLO)$/convertiso2022cn.obj \
    $(SLO)$/convertiso2022jp.obj \
    $(SLO)$/convertiso2022kr.obj \
    $(SLO)$/convertsinglebytetobmpunicode.obj \
    $(SLO)$/tcvtbyte.obj \
    $(SLO)$/tcvtmb.obj \
    $(SLO)$/tcvtutf7.obj \
    $(SLO)$/tcvtutf8.obj \
    $(SLO)$/tenchelp.obj \
    $(SLO)$/tencinfo.obj \
    $(SLO)$/textcvt.obj \
    $(SLO)$/textenc.obj \
    $(SLO)$/unichars.obj

OBJFILES = \
    $(OBJ)$/context.obj \
    $(OBJ)$/convertbig5hkscs.obj \
    $(OBJ)$/converter.obj \
    $(OBJ)$/converteuctw.obj \
    $(OBJ)$/convertgb18030.obj \
    $(OBJ)$/convertiso2022cn.obj \
    $(OBJ)$/convertiso2022jp.obj \
    $(OBJ)$/convertiso2022kr.obj \
    $(OBJ)$/convertsinglebytetobmpunicode.obj \
    $(OBJ)$/tcvtbyte.obj \
    $(OBJ)$/tcvtmb.obj \
    $(OBJ)$/tcvtutf7.obj \
    $(OBJ)$/tcvtutf8.obj \
    $(OBJ)$/tenchelp.obj \
    $(OBJ)$/tencinfo.obj \
    $(OBJ)$/textcvt.obj \
    $(OBJ)$/textenc.obj \
    $(OBJ)$/unichars.obj

# Optimization off on Solaris Intel due to internal compiler error; to be
# reevaluated after compiler upgrade:
.IF "$(OS)$(CPU)" == "SOLARISI"

NOOPTFILES = \
    $(OBJ)$/textenc.obj \
    $(SLO)$/textenc.obj

.ENDIF # OS, CPU, SOLARISI

.INCLUDE: target.mk
