#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2007-10-09 15:19:37 $
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
PRJNAME := cppu
TARGET := qa

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

INCPRE += $(MISC)$/$(TARGET)$/inc

SHL1TARGET = $(TARGET)_any
SHL1OBJS = $(SLO)$/test_any.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(SALLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SHL2TARGET = $(TARGET)_unotype
SHL2OBJS = $(SLO)$/test_unotype.obj
SHL2STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(SALLIB)
SHL2VERSIONMAP = version.map
SHL2IMPLIB = i$(SHL2TARGET)
DEF2NAME = $(SHL2TARGET)

SHL3TARGET = $(TARGET)_reference
SHL3OBJS = $(SLO)$/test_reference.obj
SHL3STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(SALLIB)
SHL3VERSIONMAP = version.map
SHL3IMPLIB = i$(SHL3TARGET)
DEF3NAME = $(SHL3TARGET)

SLOFILES = $(SHL1OBJS) $(SHL2OBJS) $(SHL3OBJS)

.INCLUDE: target.mk

ALLTAR: test

$(SHL1OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(MISC)$/$(TARGET).cppumaker.flag: $(MISC)$/$(TARGET).rdb
    - $(MKDIRHIER) $(MISC)$/$(TARGET)$/inc
    $(CPPUMAKER) -O$(MISC)$/$(TARGET)$/inc -BUCR -C $< \
        $(SOLARBINDIR)$/udkapi.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).rdb: $(MISC)$/$(TARGET)$/types.urd
    - rm $@
    $(REGMERGE) $@ /UCR $<

$(MISC)$/$(TARGET)$/types.urd: types.idl
    - $(MKDIR) $(MISC)$/$(TARGET)
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<

test .PHONY: $(SHL1TARGETN) $(SHL2TARGETN) $(SHL3TARGETN)
    testshl2 $(SHL1TARGETN)
    testshl2 $(SHL2TARGETN)
    testshl2 $(SHL3TARGETN)
