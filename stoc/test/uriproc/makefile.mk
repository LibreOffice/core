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
# $Revision: 1.9 $
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
TARGET := test_uriproc

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

UNOTYPES = \
    com.sun.star.beans.XPropertySet \
    com.sun.star.lang.XComponent \
    com.sun.star.lang.XMultiComponentFactory \
    com.sun.star.uno.XComponentContext \
    com.sun.star.uri.ExternalUriReferenceTranslator \
    com.sun.star.uri.UriReferenceFactory \
    com.sun.star.uri.VndSunStarPkgUrlReferenceFactory \
    com.sun.star.uri.XExternalUriReferenceTranslator \
    com.sun.star.uri.XUriReference \
    com.sun.star.uri.XUriReferenceFactory \
    com.sun.star.uri.XVndSunStarPkgUrlReferenceFactory \
    com.sun.star.uri.XVndSunStarScriptUrlReference \
    com.sun.star.util.XMacroExpander

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)$/test_uriproc.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(CPPUNITLIB) $(SALLIB) $(TESTSHL2LIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk

ALLTAR: test

$(BIN)$/$(TARGET).rdb .ERRREMOVE:
    $(COPY) $(SOLARBINDIR)$/types.rdb $@
    regcomp -register -r $@ -c $(subst,$/,/ $(DLLDEST)$/stocservices.uno$(DLLPOST))

test .PHONY: $(SHL1TARGETN) $(BIN)$/$(TARGET).rdb
    testshl2 $(SHL1TARGETN) -forward $(BIN)$/$(TARGET).rdb
