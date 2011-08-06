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

PRJ := ..$/..$/..
PRJNAME := testtools
TARGET := bridgetest_idl

.INCLUDE: settings.mk

.IF "$(CROSS_COMPILING)"=="YES"
all:
    @echo Nothing done when cross-compiling
.ENDIF

.INCLUDE: target.mk

.IF "$(L10N_framework)"==""

ALLTAR: $(MISC)$/$(TARGET).cppumaker.done $(MISC)$/$(TARGET).javamaker.done

$(MISC)$/$(TARGET).cppumaker.done: $(BIN)$/bridgetest.rdb
    $(CPPUMAKER) -O$(INCCOM) -BUCR $< -X$(SOLARBINDIR)/types.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).javamaker.done: $(BIN)$/bridgetest.rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD -X$(SOLARBINDIR)/types.rdb $<
    $(TOUCH) $@

$(BIN)$/bridgetest.rdb: bridgetest.idl
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<
    - $(RM) $@
    $(REGMERGE) $@ /UCR $(MISC)$/$(TARGET)$/bridgetest.urd

.IF "$(GUI)" == "WNT"
.IF "$(COM)" != "GCC"

CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF

ALLTAR: $(MISC)$/$(TARGET).cppumaker.done \
    $(MISC)$/$(TARGET).javamaker.done \
    $(BIN)$/cli_types_bridgetest.dll 

$(BIN)$/cli_types_bridgetest.dll: $(BIN)$/bridgetest.rdb
    $(CLIMAKER) $(CLIMAKERFLAGS) --out $@ -r $(SOLARBINDIR)$/cli_uretypes.dll \
        -X $(SOLARBINDIR)$/types.rdb $< 
    $(TOUCH) $@

.ENDIF
.ENDIF # GUI, WNT
.ENDIF # L10N_framework

