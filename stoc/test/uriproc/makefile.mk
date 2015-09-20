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



.IF "$(OOO_SUBSEQUENT_TESTS)" == ""
nothing .PHONY:
.ELSE

PRJ := ../..
PRJNAME := stoc
TARGET := test_uriproc

ENABLE_EXCEPTIONS := TRUE

my_components = stocservices

.INCLUDE: settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.

.ELSE

DLLPRE = # no leading "lib" on .so files

APP1TARGET = $(TARGET)
APP1OBJS = $(SLO)/test_uriproc.obj
APP1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(GTESTLIB) $(SALLIB)
APP1RPATH = NONE
# this is a custom test, can't use APP1TARGET_run so disable it here:
APP1TEST = disabled

SLOFILES = $(APP1OBJS)

.INCLUDE: target.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

ALLTAR: test

test .PHONY: $(SHL1TARGETN) $(MISC)/$(TARGET)/services.rdb
    $(COMMAND_ECHO) $(AUGMENT_LIBRARY_PATH_LOCAL) \
    UNO_TYPES=$(my_file)$(SOLARBINDIR)/udkapi.rdb \
    UNO_SERVICES=$(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb \
    URE_INTERNAL_LIB_DIR=$(my_file)$(PWD)/$(DLLDEST) \
    $(APP1TARGETN) --gtest_output="xml:$(BIN)/$(APP1TARGET)_result.xml"

$(MISC)/$(TARGET)/services.rdb .ERRREMOVE: $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/$(TARGET)/services.input \
        $(my_components:^"$(MISC)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(PWD)/$(MISC)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/$(TARGET)/services.input

$(MISC)/$(TARGET)/services.input:
    $(MKDIRHIER) $(@:d)
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"

.ENDIF # "$(OOO_SUBSEQUENT_TESTS)" == ""
