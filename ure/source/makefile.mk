#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 10:55:03 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ := ..
PRJNAME := ure
TARGET := $(PRJNAME)

.INCLUDE: settings.mk

.IF "$(OS)" == "WNT"
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
    - $(MKDIR) $(@:d)
    + $(COPY) $< $@

$(BIN)$/ure$/THIRDPARTYLICENSEREADME.html: THIRDPARTYLICENSEREADME.html
    - $(MKDIR) $(@:d)
    + $(COPY) $< $@

$(BIN)$/ure$/jvmfwk3$(MY_RC): jvmfwk3$(MY_RC)
    - $(MKDIR) $(@:d)
    + $(COPY) $< $@

$(BIN)$/ure$/uno$(MY_RC): uno$(MY_RC)
    - $(MKDIR) $(@:d)
    + $(COPY) $< $@

.IF "$(OS)" != "WNT"

ALLTAR: $(BIN)$/ure$/startup.sh

$(BIN)$/ure$/startup.sh: startup.sh
    - $(MKDIR) $(@:d)
    + $(COPY) $< $@

.ENDIF
