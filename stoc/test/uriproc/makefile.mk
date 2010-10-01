#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
