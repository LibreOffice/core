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



PRJ := ..$/..
PRJNAME := codemaker
TARGET := test_codemaker_cppumaker

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

DLLPRE = # no leading "lib" on .so files

INCPRE += $(MISC)$/$(TARGET)$/inc

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)$/test_codemaker_cppumaker.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(SALLIB) $(TESTSHL2LIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk

ALLTAR: test

$(SHL1OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(MISC)$/$(TARGET).cppumaker.flag: $(BIN)$/cppumaker$(EXECPOST)
$(MISC)$/$(TARGET).cppumaker.flag: $(MISC)$/$(TARGET).rdb
    - $(MKDIRHIER) $(MISC)$/$(TARGET)$/inc
    $(AUGMENT_LIBRARY_PATH) $(BIN)$/cppumaker$(EXECPOST) \
        -O$(MISC)$/$(TARGET)$/inc -BUCR -C $< $(SOLARBINDIR)$/udkapi.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).rdb: $(MISC)$/$(TARGET)$/types.urd
    - rm $@
    $(REGMERGE) $@ /UCR $<

$(MISC)$/$(TARGET)$/types.urd: types.idl
    - $(MKDIR) $(MISC)$/$(TARGET)
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<

test .PHONY: $(SHL1TARGETN)
    $(AUGMENT_LIBRARY_PATH) testshl2 $<
