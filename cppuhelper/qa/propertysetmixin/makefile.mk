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

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.

.ELSE

PRJ := ../..
PRJNAME := cppuhelper

TARGET := qa_propertysetmixin
PACKAGE = test/cppuhelper/propertysetmixin/comp

ENABLE_EXCEPTIONS := TRUE

my_components = $(TARGET).cpp $(TARGET).java

.INCLUDE: settings.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

DLLPRE = # no leading "lib" on .so files
INCPRE += -I$(MISC)/$(TARGET)/inc

APP1TARGET = $(TARGET)
APP1OBJS = $(SLO)/test_propertysetmixin.obj
APP1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(GTESTLIB) $(SALLIB)
APP1RPATH = NONE
# this is a custom test, can't use APP1TARGET_run so disable it here:
APP1TEST = disabled

SHL2TARGET = $(TARGET).uno
SHL2OBJS = $(SLO)/comp_propertysetmixin.obj
SHL2VERSIONMAP = $(SOLARENV)/src/component.map
SHL2STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL2IMPLIB = i$(SHL2TARGET)
SH21RPATH = NONE
DEF2NAME = $(SHL2TARGET)

SLOFILES = $(APP1OBJS) $(SHL2OBJS)

JAVAFILES = JavaSupplier.java
JARFILES = java_uno.jar juh.jar jurt.jar ridl.jar


ALLTAR: test

.INCLUDE: target.mk

$(MISC)/$(TARGET)/types.urd: types.idl
    $(MKDIRHIER) $(@:d)
    $(IDLC) -O$(@:d) -I$(SOLARIDLDIR) -cid -we $<

$(MISC)/$(TARGET)/types.rdb: $(MISC)/$(TARGET)/types.urd
    - $(RM) $@
    $(REGMERGE) $@ /UCR $<

$(MISC)/$(TARGET)/cppumaker.flag: $(MISC)/$(TARGET)/types.rdb
    $(CPPUMAKER) -O$(MISC)/$(TARGET)/inc -BUCR -Gc \
        -X$(SOLARBINDIR)/udkapi.rdb $<
    $(TOUCH) $@

$(SLOFILES): $(MISC)/$(TARGET)/cppumaker.flag

$(MISC)/$(TARGET)/javamaker.flag: $(MISC)/$(TARGET)/types.rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD -Gc -X$(SOLARBINDIR)/udkapi.rdb $<
    $(TOUCH) $@

$(JAVATARGET): $(MISC)/$(TARGET)/javamaker.flag

$(MISC)/$(TARGET)/services.rdb .ERRREMOVE: $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/$(TARGET)/services.input \
        $(my_components:^"$(MISC)/$(TARGET)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(PWD)/$(MISC)/$(TARGET)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/$(TARGET)/services.input

$(MISC)/$(TARGET)/services.input:
    $(MKDIRHIER) $(@:d)
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@

$(MISC)/$(TARGET)/$(TARGET).cpp.component .ERRREMOVE: \
        $(SOLARENV)/bin/createcomponent.xslt $(TARGET).cpp.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_INBUILD_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt $(TARGET).cpp.component

$(MISC)/$(TARGET)/$(TARGET).java.component .ERRREMOVE: \
        $(SOLARENV)/bin/createcomponent.xslt $(TARGET).java.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_INBUILD_JAVA)$(TARGET).uno.jar' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt $(TARGET).java.component

$(MISC)/$(TARGET)/$(TARGET).uno.jar: $(JAVATARGET) \
        $(MISC)/$(TARGET)/javamaker.flag manifest
    jar cfm $@ manifest -C $(CLASSDIR) test/cppuhelper/propertysetmixin


test .PHONY: $(APP1TARGETN) $(SHL2TARGETN) $(MISC)/$(TARGET)/$(TARGET).uno.jar \
        $(MISC)/$(TARGET)/types.rdb $(MISC)/$(TARGET)/services.rdb
        $(COMMAND_ECHO) $(AUGMENT_LIBRARY_PATH_LOCAL) \
        UNO_TYPES='$(my_file)$(SOLARBINDIR)/udkapi.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/types.rdb' \
        UNO_SERVICES='$(my_file)$(SOLARXMLDIR)/ure/services.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb'\
        URE_INTERNAL_LIB_DIR=$(my_file)$(SOLARSHAREDBIN) \
        URE_INTERNAL_JAVA_DIR=$(my_file)$(SOLARBINDIR) \
        OOO_INBUILD_SHAREDLIB_DIR=$(my_file)$(PWD)/$(DLLDEST) \
        OOO_INBUILD_JAR_DIR=$(my_file)$(PWD)/$(MISC)/$(TARGET) \
        $(APP1TARGETN) --gtest_output="xml:$(BIN)/$(APP1TARGET)_result.xml"


.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"

.ENDIF # "$(OOO_SUBSEQUENT_TESTS)" == ""
