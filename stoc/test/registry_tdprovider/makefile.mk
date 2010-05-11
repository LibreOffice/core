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

PRJ := ..$/..
PRJNAME := stoc

TARGET := test_registry_tdprovider

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

SLOFILES = $(SLO)$/testregistrytdprovider.obj

SHL1TARGET = testregistrytdprovider.uno
SHL1OBJS = $(SLOFILES)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1IMPLIB = itestregistrytdprovider
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR: test

$(MISC)$/$(TARGET)$/all.rdb: types.idl
    - rm $@
    - $(MKDIR) $(MISC)$/$(TARGET)
    idlc -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -C -cid -we $<
    regmerge $@ /UCR $(MISC)$/$(TARGET)$/types.urd
    regmerge $@ / $(SOLARBINDIR)$/types.rdb

$(SLOFILES): $(MISC)$/$(TARGET)$/all.rdb

test .PHONY: $(SHL1TARGETN) $(MISC)$/$(TARGET)$/all.rdb
    uno -c test.registrytdprovider.impl -l $(subst,$/,/ $(SHL1TARGETN)) \
        -ro $(subst,$/,/ $(MISC)$/$(TARGET)$/all.rdb)
