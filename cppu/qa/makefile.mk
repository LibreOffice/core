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



PRJ := ..
PRJNAME := cppu
TARGET := qa

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

DLLPRE = # no leading "lib" on .so files

INCPRE += $(MISC)$/$(TARGET)$/inc

SHL1TARGET = $(TARGET)_any
SHL1OBJS = $(SLO)$/test_any.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(TESTSHL2LIB) $(SALLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SHL2TARGET = $(TARGET)_unotype
SHL2OBJS = $(SLO)$/test_unotype.obj
SHL2STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(TESTSHL2LIB) $(SALLIB)
SHL2VERSIONMAP = version.map
SHL2IMPLIB = i$(SHL2TARGET)
DEF2NAME = $(SHL2TARGET)

SHL3TARGET = $(TARGET)_reference
SHL3OBJS = $(SLO)$/test_reference.obj
SHL3STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(TESTSHL2LIB) $(SALLIB)
SHL3VERSIONMAP = version.map
SHL3IMPLIB = i$(SHL3TARGET)
DEF3NAME = $(SHL3TARGET)

SHL4TARGET = $(TARGET)_recursion
SHL4OBJS = $(SLO)$/test_recursion.obj
SHL4STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(TESTSHL2LIB) $(SALLIB)
SHL4VERSIONMAP = version.map
SHL4IMPLIB = i$(SHL4TARGET)
DEF4NAME = $(SHL4TARGET)

SLOFILES = $(SHL1OBJS) $(SHL2OBJS) $(SHL3OBJS) $(SHL4OBJS)

.INCLUDE: target.mk

ALLTAR: test

$(SHL1OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(MISC)$/$(TARGET).cppumaker.flag: $(MISC)$/$(TARGET).rdb
    - $(MKDIRHIER) $(MISC)$/$(TARGET)$/inc
    $(CPPUMAKER) -O$(MISC)$/$(TARGET)$/inc -BUCR -C $< \
        $(SOLARBINDIR)$/udkapi.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).rdb: $(MISC)$/$(TARGET)$/types.urd
    - rm $@
    $(REGMERGE) $@ /UCR $<

$(MISC)$/$(TARGET)$/types.urd: types.idl
    - $(MKDIR) $(MISC)$/$(TARGET)
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<

test .PHONY: $(SHL1TARGETN) $(SHL2TARGETN) $(SHL3TARGETN) $(SHL4TARGETN)
    $(TESTSHL2) $(SHL1TARGETN)
    $(TESTSHL2) $(SHL2TARGETN)
    $(TESTSHL2) $(SHL3TARGETN)
    $(TESTSHL2) $(SHL4TARGETN)
