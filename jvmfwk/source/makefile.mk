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
# $Revision: 1.14 $
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

PRJ = ..
PRJNAME = jvmfwk
FRAMEWORKLIB=jvmfwk
TARGET = $(FRAMEWORKLIB)
ENABLE_EXCEPTIONS = TRUE

.IF "$(OS)" != "WNT" && "$(GUI)"!="OS2"
UNIXVERSIONNAMES = UDK
.ENDIF # WNT

.INCLUDE: settings.mk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

UNOUCROUT = $(OUT)$/inc

SLOFILES = \
    $(SLO)$/framework.obj \
    $(SLO)$/libxmlutil.obj \
    $(SLO)$/fwkutil.obj \
    $(SLO)$/elements.obj \
    $(SLO)$/fwkbase.obj


#LIB1TARGET=$(SLB)$/$(FRAMEWORKLIB).lib

.IF "$(UNIXVERSIONNAMES)" == ""
SHL1TARGET = $(FRAMEWORKLIB)$(UDK_MAJOR)
.ELSE # UNIXVERSIONNAMES
SHL1TARGET = $(FRAMEWORKLIB)
.ENDIF # UNIXVERSIONNAMES

#SHL1TARGET=$(FRAMEWORKLIB)
SHL1DEPN=
SHL1IMPLIB = i$(FRAMEWORKLIB)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB) $(SALHELPERLIB) $(LIBXML2LIB)
SHL1RPATH = URELIB

.IF "$(OS)" == "WNT"
SHL1STDLIBS += $(ADVAPI32LIB)
.ENDIF # WNT

SHL1VERSIONMAP = framework.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

.IF "$(GUI)"=="UNX"
RCFILE=$(BIN)$/jvmfwk3rc
.ELIF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
RCFILE=$(BIN)$/jvmfwk3.ini
.END


.INCLUDE: target.mk
$(RCFILE): jvmfwk3rc
    -$(COPY) $< $@

ALLTAR: \
    $(RCFILE)

