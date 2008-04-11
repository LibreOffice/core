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
# $Revision: 1.6 $
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
PRJ=..$/..$/..

PRJNAME=bridges
TARGET=gcc3_uno
LIBTARGET=no
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(COM)$(OS)$(CPU)" == "GCCSOLARISS"

.IF "$(cppu_no_leak)" == ""
CFLAGS += -DLEAK_STATIC_DATA
.ENDIF

CFLAGSNOOPT=-O0

NOOPTFILES = \
    $(SLO)$/uno2cpp.obj	\
    $(SLO)$/cpp2uno.obj

SLOFILES= \
    $(SLO)$/except.obj		\
    $(SLO)$/cpp2uno.obj		\
    $(SLO)$/uno2cpp.obj


SHL1TARGET=$(TARGET)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=..$/..$/bridge_exports.map
SHL1RPATH=URELIB

SHL1OBJS= $(SLOFILES)
SHL1LIBS =$(SLB)$/cpp_uno_shared.lib

SHL1STDLIBS= \
    $(CPPULIB) \
    $(SALLIB)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

