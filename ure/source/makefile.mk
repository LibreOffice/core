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
# $Revision: 1.6 $
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
PRJNAME := ure
TARGET := $(PRJNAME)

.INCLUDE: settings.mk

.IF "$(OS)" == "WNT" || "$(OS)" == "OS2"
MY_RC = .ini
.ELSE
MY_RC = rc
.ENDIF

ZIP1TARGET = uretest
ZIP1FLAGS = -r
ZIP1LIST = uretest

.INCLUDE: target.mk

ALLTAR: \
    $(BIN)$/ure$/README \
    $(BIN)$/ure$/THIRDPARTYLICENSEREADME.html \
    $(BIN)$/ure$/jvmfwk3$(MY_RC) \
    $(BIN)$/ure$/uno$(MY_RC)

$(BIN)$/ure$/README: README
    -$(MKDIR) $(@:d)
    $(COPY) $< $@

$(BIN)$/ure$/THIRDPARTYLICENSEREADME.html: THIRDPARTYLICENSEREADME.html
    - $(MKDIR) $(@:d)
    $(COPY) $< $@

$(BIN)$/ure$/jvmfwk3$(MY_RC): jvmfwk3$(MY_RC)
    -$(MKDIR) $(@:d)
    $(COPY) $< $@

$(BIN)$/ure$/uno$(MY_RC): uno$(MY_RC)
    -$(MKDIR) $(@:d)
    $(COPY) $< $@

.IF "$(OS)" != "WNT"

ALLTAR: $(BIN)$/ure$/startup.sh

$(BIN)$/ure$/startup.sh: startup.sh
    -$(MKDIR) $(@:d)
    $(COPY) $< $@

.ENDIF
