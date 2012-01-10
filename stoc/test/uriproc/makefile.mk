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

CFLAGSCXX += $(CPPUNIT_CFLAGS)
DLLPRE = # no leading "lib" on .so files

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)/test_uriproc.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(CPPUNITLIB) $(SALLIB)
SHL1VERSIONMAP = version.map
SHL1RPATH = NONE
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

ALLTAR: test

test .PHONY: $(SHL1TARGETN) $(MISC)/$(TARGET)/services.rdb
    $(CPPUNITTESTER) $(SHL1TARGETN) \
        -env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/udkapi.rdb \
        -env:UNO_SERVICES=$(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb \
        -env:URE_INTERNAL_LIB_DIR=$(my_file)$(PWD)/$(DLLDEST)

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

.END
