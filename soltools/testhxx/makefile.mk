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
PRJNAME := soltools
TARGET := testhxx
LIBTARGET := NO
ENABLE_EXCEPTIONS := TRUE

CAPTURE_COMMAND = echo
CAPTURE_OUTPUT = > $(MISC)$/testhxx.output && $(TOUCH) $(SLO)$/testhxx.obj

.INCLUDE: $(PRJ)$/util$/makefile.pmk
.INCLUDE: settings.mk

.IF "$(CROSS_COMPILING)"=="YES"
all:
    @echo Nothing done when cross-compiling
.ENDIF

SLOFILES = $(SLO)$/testhxx.obj

.INCLUDE: target.mk

ALLTAR: $(BIN)$/$(TARGET)

$(BIN)$/$(TARGET) .ERRREMOVE : $(MISC)$/testhxx.output create.pl
    $(PERL) -w create.pl < $(MISC)$/testhxx.output > $@
    chmod +x $@

$(MISC)$/testhxx.output: $(SLO)$/testhxx.obj
    $(TOUCH) $<
    $(TOUCH) $@
