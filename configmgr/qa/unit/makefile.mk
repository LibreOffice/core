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
# $Revision: 1.3 $
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
PRJNAME := configmgr
TARGET := qa_unit

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)$/performance.obj $(SLO)$/threading.obj $(SLO)$/ubootstrap.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(CPPUNITLIB) $(SALLIB)
SHL1VERSIONMAP = export.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk

ALLTAR: test

$(MISC)$/$(TARGET).rdb .ERRREMOVE:
    $(COPY) $(SOLARBINDIR)$/types.rdb $@
    regcomp -register -r $@ -c $(subst,$/,/ $(DLLDEST)$/configmgr2.uno$(DLLPOST))
    regcomp -register -r $@ -c $(subst,$/,/ $(SOLARLIBDIR)/sax.uno$(DLLPOST))
    regcomp -register -r $@ -c $(subst,$/,/ $(SOLARLIBDIR)/stocservices.uno$(DLLPOST))
    regcomp -register -r $@ -c $(subst,$/,/ $(SOLARLIBDIR)/streams.uno$(DLLPOST))

test .PHONY: $(SHL1TARGETN) $(MISC)$/$(TARGET).rdb
    testshl2 $(SHL1TARGETN) -forward "$(MISC)$/$(TARGET).rdb#$(PWD)$/$(MISC)$/$(TARGET).registry"
