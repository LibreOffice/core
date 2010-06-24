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

TARGET := test_tdmanager

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

SLOFILES = $(SLO)$/testtdmanager.obj

SHL1TARGET = testtdmanager.uno
SHL1OBJS = $(SLOFILES)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1IMPLIB = itesttdmanager
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR: test

$(MISC)$/$(TARGET)$/%.rdb : %.idl
    - rm $@
    - $(MKDIR) $(MISC)$/$(TARGET)
    idlc -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -C -cid -we $<
    regmerge $@ /UCR $(subst,.rdb,.urd $@)

IDL_FILES = \
    types.idl \
    types2_incomp.idl \
    types3_incomp.idl \
    types4_incomp.idl \
    types5_incomp.idl \
    types5.idl \
    types6_incomp.idl

RDB_FILES = $(foreach,i,$(subst,.idl,.rdb $(IDL_FILES)) $(MISC)$/$(TARGET)$/$i)

$(SLOFILES): $(RDB_FILES)

test .PHONY: $(SHL1TARGETN) $(RDB_FILES)
    uno -c test.tdmanager.impl -l $(subst,$/,/ $(SHL1TARGETN)) \
    -ro $(subst,$/,/ $(SOLARBINDIR)$/udkapi_doc.rdb) \
        -- $(subst,$/,/ $(SOLARBINDIR)$/types_doc.rdb) \
           $(subst,$/,/ $(RDB_FILES))
