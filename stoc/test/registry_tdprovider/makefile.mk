#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 08:30:08 $
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

TARGET := test_registry_tdprovider

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

SLOFILES = $(SLO)$/testregistrytdprovider.obj

SHL1TARGET = testregistrytdprovider.uno
SHL1OBJS = $(SLOFILES)
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1IMPLIB = itestregistrytdprovider
DEF1NAME = $(SHL1TARGET)

.IF "$(OS)$(CPU)$(COMNAME)" == "LINUXIgcc3"
SHL1VERSIONMAP = testregistrytdprovider.LINUXIgcc3.map
.ELSE
SHL1VERSIONMAP = testregistrytdprovider.map
.ENDIF

.INCLUDE: target.mk

ALLTAR: test

$(MISC)$/$(TARGET)$/all.rdb: types.idl
    - rm $@
    - $(MKDIR) $(MISC)$/$(TARGET)
    idlc -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -C -cid -we $<
    regmerge $@ /UCR $(MISC)$/$(TARGET)$/types.urd
    regmerge $@ / $(SOLARBINDIR)$/types.rdb

$(SLOFILES): $(MISC)$/$(TARGET)$/all.rdb

test .PHONY: $(SHL1TARGETN) $(MISC)$/$(TARGET)$/all.rdb
    uno -c test.registrytdprovider.impl -l $(subst,$/,/ $(SHL1TARGETN)) \
        -ro $(subst,$/,/ $(MISC)$/$(TARGET)$/all.rdb)
