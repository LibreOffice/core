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

PRJ := ..
PRJNAME := cppu
TARGET := qa

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

CFLAGSCXX+=$(CPPUNIT_CFLAGS)

DLLPRE=# no leading "lib" on .so files

INCPRE+=$(MISC)$/$(TARGET)$/inc

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

$(SHL1OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(SHL2OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(SHL3OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(SHL4OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(MISC)$/$(TARGET).cppumaker.flag: $(MISC)$/$(TARGET).rdb
    - $(MKDIRHIER) $(MISC)$/$(TARGET)$/inc
    $(CPPUMAKER) -O$(MISC)$/$(TARGET)$/inc -BUCR -C $< \
        $(SOLARBINDIR)$/udkapi.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).rdb: $(MISC)$/$(TARGET)$/types.urd
    - $(RM) $@
    $(REGMERGE) $@ /UCR $<

$(MISC)$/$(TARGET)$/types.urd: types.idl
    - $(MKDIR) $(MISC)$/$(TARGET)
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<

.INCLUDE : _cppunit.mk
