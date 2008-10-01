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
# $Revision: 1.4.12.1 $
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

PRJ := ..$/..$/..
PRJNAME := bridges
TARGET := sunpro5_uno
ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

.IF "$(COM)" == "C52" && "$(CPU)" == "U"

SHL1TARGET = $(TARGET)
SHL1OBJS = \
    $(SLO)$/callvirtualmethod.obj \
    $(SLO)$/cpp2uno.obj \
    $(SLO)$/exceptions.obj \
    $(SLO)$/fp.obj \
    $(SLO)$/isdirectreturntype.obj \
    $(SLO)$/uno2cpp.obj \
    $(SLO)$/vtableslotcall.obj
SHL1LIBS = $(SLB)$/cpp_uno_shared.lib
SHL1STDLIBS = $(CPPULIB) $(SALLIB)
SHL1RPATH = URELIB
SHL1VERSIONMAP = ..$/..$/bridge_exports.map
SHL1IMPLIB= i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.ENDIF

.INCLUDE: target.mk

$(SLO)$/%.obj: %.s
    CC -m64 -KPIC -c -o $(SLO)$/$(@:b).o $<
    touch $@
