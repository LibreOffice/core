#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 13:08:58 $
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
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(CPPUNITLIB) $(SALLIB)
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
