#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: ihi $ $Date: 2008-04-24 16:26:54 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2008 by Sun Microsystems, Inc.
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

PRJ := ..$/..$/..
PRJNAME := bridges
TARGET := sunpro5_uno
ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

.IF "$(COM)" == "C52" && "$(CPUNAME)" == "SPARC64"

SHL1TARGET = $(TARGET)
SHL1OBJS = \
    $(SLO)$/callvirtualmethod.obj \
    $(SLO)$/cpp2uno.obj \
    $(SLO)$/doflushcode.obj \
    $(SLO)$/exceptions.obj \
    $(SLO)$/flushcode.obj \
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
