#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: ihi $ $Date: 2007-11-23 13:58:12 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2007 by Sun Microsystems, Inc.
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
    regcomp -register -r $@ -c $(subst,$/,/ $(SOLARLIBDIR)/typeconverter.uno$(DLLPOST))
    regcomp -register -r $@ -c $(subst,$/,/ $(SOLARLIBDIR)/sax.uno$(DLLPOST))
    regcomp -register -r $@ -c $(subst,$/,/ $(SOLARLIBDIR)/simplereg.uno$(DLLPOST))
    regcomp -register -r $@ -c $(subst,$/,/ $(SOLARLIBDIR)/streams.uno$(DLLPOST))

test .PHONY: $(SHL1TARGETN) $(MISC)$/$(TARGET).rdb
    testshl2 $(SHL1TARGETN) -forward "$(MISC)$/$(TARGET).rdb#$(PWD)$/$(MISC)$/$(TARGET).registry"
