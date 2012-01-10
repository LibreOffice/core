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
CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)/test_propertysetmixin.obj
SHL1VERSIONMAP = test.map
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(CPPUNITLIB) $(SALLIB)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1RPATH = NONE
DEF1NAME = $(SHL1TARGET)

SHL2TARGET = $(TARGET).uno
SHL2OBJS = $(SLO)/comp_propertysetmixin.obj
SHL2VERSIONMAP = $(SOLARENV)/src/component.map
SHL2STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL2IMPLIB = i$(SHL2TARGET)
SH21RPATH = NONE
DEF2NAME = $(SHL2TARGET)

SLOFILES = $(SHL1OBJS) $(SHL2OBJS)

JAVAFILES = JavaSupplier.java
JARFILES = java_uno.jar juh.jar jurt.jar ridl.jar

.INCLUDE: target.mk

ALLTAR: test

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

test .PHONY: $(SHL1TARGETN) $(SHL2TARGETN) $(MISC)/$(TARGET)/$(TARGET).uno.jar \
        $(MISC)/$(TARGET)/types.rdb $(MISC)/$(TARGET)/services.rdb
    $(CPPUNITTESTER) $(SHL1TARGETN) \
        '-env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/udkapi.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/types.rdb' \
        '-env:UNO_SERVICES=$(my_file)$(SOLARXMLDIR)/ure/services.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb'\
        -env:URE_INTERNAL_LIB_DIR=$(my_file)$(SOLARSHAREDBIN) \
        -env:URE_INTERNAL_JAVA_DIR=$(my_file)$(SOLARBINDIR) \
        -env:OOO_INBUILD_SHAREDLIB_DIR=$(my_file)$(PWD)/$(DLLDEST) \
        -env:OOO_INBUILD_JAR_DIR=$(my_file)$(PWD)/$(MISC)/$(TARGET)

.END
