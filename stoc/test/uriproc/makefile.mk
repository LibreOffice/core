#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2004-06-17 11:44:59 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2003 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
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
    com.sun.star.uri.XVndSunStarScriptUrlReference

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
    + $(COPY) $(SOLARBINDIR)$/types.rdb $@
    regcomp -register -r $@ -c $(subst,$/,/ $(DLLDEST)$/uriproc.uno$(DLLPOST))

test .PHONY: $(SHL1TARGETN) $(BIN)$/$(TARGET).rdb
    testshl2 $(SHL1TARGETN) -forward $(BIN)$/$(TARGET).rdb
