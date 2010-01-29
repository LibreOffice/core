#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.7 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

.IF "$(OOO_SUBSEQUENT_TESTS)" == ""
nothing .PHONY:
.ELSE

PRJ := ../..
PRJNAME := cppuhelper

TARGET := qa_propertysetmixin
PACKAGE = test/cppuhelper/propertysetmixin/comp

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

DLLPRE = # no leading "lib" on .so files
INCPRE += -I$(MISC)/$(TARGET)/inc

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
    - $(MKDIR) $(@:d)
    $(IDLC) -O$(@:d) -I$(SOLARIDLDIR) -cid -we $<

$(MISC)/$(TARGET)/types.rdb: $(MISC)/$(TARGET)/types.urd
    - $(RM) $@
    $(REGMERGE) $@ /UCR $<

$(MISC)/$(TARGET)/uno.rdb: $(MISC)/$(TARGET)/types.rdb $(SHL2TARGETN) \
        $(MISC)/$(TARGET)/$(TARGET).uno.jar $(MISC)/$(TARGET)/bootstrap.rdb
    - $(MKDIR) $(@:d)
    $(COPY) $(SOLARBINDIR)/types.rdb $@
    $(REGMERGE) $@ / $(MISC)/$(TARGET)/types.rdb
    $(REGCOMP) -register -r $@ -wop -c javaloader.uno -c javavm.uno \
        -c reflection.uno -c stocservices.uno -c $(SHL2TARGETN)
    $(REGCOMP) -register -br $(MISC)/$(TARGET)/bootstrap.rdb -r $@ \
        -c $(my_file)$(PWD)/$(MISC)/$(TARGET)/$(TARGET).uno.jar \
        -env:URE_INTERNAL_JAVA_DIR=$(my_file)$(SOLARBINDIR)

$(MISC)/$(TARGET)/bootstrap.rdb:
    - $(MKDIR) $(@:d)
    $(COPY) $(SOLARBINDIR)/types.rdb $@
    $(REGCOMP) -register -r $@ -wop -c javaloader.uno -c javavm.uno \
        -c stocservices.uno

$(MISC)/$(TARGET)/cppumaker.flag: $(MISC)/$(TARGET)/types.rdb
    $(CPPUMAKER) -O$(MISC)/$(TARGET)/inc -BUCR -Gc \
        -X$(SOLARBINDIR)/types.rdb $<
    $(TOUCH) $@

$(SLOFILES): $(MISC)/$(TARGET)/cppumaker.flag

$(MISC)/$(TARGET)/javamaker.flag: $(MISC)/$(TARGET)/types.rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD -Gc -X$(SOLARBINDIR)/types.rdb $<
    $(TOUCH) $@

# The following dependency (to execute javac whenever javamaker has run) does
# not work reliably, see #i28827#:
$(JAVAFILES) $(JAVACLASSFILES): $(MISC)/$(TARGET)/javamaker.flag

$(MISC)/$(TARGET)/$(TARGET).uno.jar: $(JAVACLASSFILES) \
        $(MISC)/$(TARGET)/javamaker.flag manifest
    jar cfm $@ manifest -C $(CLASSDIR) test/cppuhelper/propertysetmixin

test .PHONY: $(SHL1TARGETN) $(MISC)/$(TARGET)/uno.rdb
    $(CPPUNITTESTER) $(SHL1TARGETN) \
        -env:URE_INTERNAL_JAVA_DIR=$(my_file)$(SOLARBINDIR) \
        -env:URE_INTERNAL_LIB_DIR=$(my_file)$(SOLARSHAREDBIN) \
        -env:arg-reg=$(MISC)/$(TARGET)/uno.rdb -env:arg-path=$(SOLARSHAREDBIN)

.END
