#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 08:32:31 $
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

TARGET := test_tdmanager

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

SLOFILES = $(SLO)$/testtdmanager.obj

SHL1TARGET = testtdmanager.uno
SHL1OBJS = $(SLOFILES)
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1IMPLIB = itesttdmanager
DEF1NAME = $(SHL1TARGET)

.IF "$(OS)$(CPU)$(COMNAME)" == "LINUXIgcc3"
SHL1VERSIONMAP = testtdmanager.LINUXIgcc3.map
.ELSE
SHL1VERSIONMAP = testtdmanager.map
.ENDIF

.INCLUDE: target.mk

ALLTAR: test

$(MISC)$/$(TARGET)$/%.rdb : %.idl
    - rm $@
    - $(MKDIR) $(MISC)$/$(TARGET)
    idlc -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -C -cid -we $<
    regmerge $@ /UCR $(subst,.rdb,.urd $@)

IDL_FILES = \
    types.idl \
    types2_incomp.idl \
    types3_incomp.idl \
    types4_incomp.idl \
    types5_incomp.idl \
    types5.idl \
    types6_incomp.idl

RDB_FILES = $(foreach,i,$(subst,.idl,.rdb $(IDL_FILES)) $(MISC)$/$(TARGET)$/$i)

$(SLOFILES): $(RDB_FILES)

test .PHONY: $(SHL1TARGETN) $(RDB_FILES)
    uno -c test.tdmanager.impl -l $(subst,$/,/ $(SHL1TARGETN)) \
    -ro $(subst,$/,/ $(SOLARBINDIR)$/udkapi_doc.rdb) \
        -- $(subst,$/,/ $(SOLARBINDIR)$/types_doc.rdb) \
           $(subst,$/,/ $(RDB_FILES))
