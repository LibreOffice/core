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

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.

.ELSE


INCPRE += $(MISC)$/$(TARGET)$/inc

APP1TARGET = $(TARGET)_any
APP1OBJS = $(SLO)$/test_any.obj $(SLO)$/main.obj
APP1STDLIBS = $(CPPULIB) $(GTESTLIB) $(TESTSHL2LIB) $(SALLIB)
APP1RPATH = NONE
APP1TEST = enabled

APP2TARGET = $(TARGET)_unotype
APP2OBJS = $(SLO)$/test_unotype.obj $(SLO)$/main.obj
APP2STDLIBS = $(CPPULIB) $(GTESTLIB) $(TESTSHL2LIB) $(SALLIB)
APP2RPATH = NONE
APP2TEST = enabled

APP3TARGET = $(TARGET)_reference
APP3OBJS = $(SLO)$/test_reference.obj $(SLO)$/main.obj
APP3STDLIBS = $(CPPULIB) $(GTESTLIB) $(TESTSHL2LIB) $(SALLIB)
APP3RPATH = NONE
APP3TEST = enabled

APP4TARGET = $(TARGET)_recursion
APP4OBJS = $(SLO)$/test_recursion.obj $(SLO)$/main.obj
APP4STDLIBS = $(CPPULIB) $(GTESTLIB) $(TESTSHL2LIB) $(SALLIB)
APP4RPATH = NONE
APP4TEST = enabled

.INCLUDE: target.mk

$(APP1OBJS): $(MISC)$/$(TARGET).cppumaker.flag
$(APP2OBJS): $(MISC)$/$(TARGET).cppumaker.flag
$(APP3OBJS): $(MISC)$/$(TARGET).cppumaker.flag
$(APP4OBJS): $(MISC)$/$(TARGET).cppumaker.flag

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

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
