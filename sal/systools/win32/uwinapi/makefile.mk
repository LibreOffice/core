#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
PRJNAME=sal
TARGET=uwinapi

USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Targets ----------------------------------


.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
CFLAGSCXX+=-Wno-unused-parameter -Wno-return-type
.ENDIF

SLOFILES=\
        $(SLO)$/snprintf.obj\
        $(SLO)$/snwprintf.obj

SHL1TARGET=$(TARGET)
SHL1IMPLIB=$(SHL1TARGET)
SHL1DEF=$(MISC)/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1DEPN=\
        $(DEF1EXPORTFILE)\
        makefile.mk

SHL1OBJS=$(SLOFILES)

#No default libraries
STDSHL=

.IF "$(COM)"=="GCC"
SHL1STDLIBS=\
        -lmingw32 \
        $(MINGW_LIBGCC)
MINGWSSTDOBJ=
MINGWSSTDENDOBJ=
.ENDIF

SHL1STDLIBS+=\
        $(KERNEL32LIB)\
        $(USER32LIB)\
        $(ADVAPI32LIB)\
        $(VERSIONLIB)\
        $(LIBCMT)\
        $(SHLWAPILIB)


.ENDIF

.INCLUDE : target.mk
