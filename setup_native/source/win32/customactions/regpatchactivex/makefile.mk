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
# $Revision: 1.7 $
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

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=regpatchactivex
USE_DEFFILE=TRUE

.IF "$(GUI)"=="WNT"

# --- Settings -----------------------------------------------------

# NO_DEFAULT_STL=TRUE
ENABLE_EXCEPTIONS=TRUE

.INCLUDE : settings.mk

STDSHL=
# SOLARINC!:=$(SOLARINC:s/stl//)

# --- Files --------------------------------------------------------

INCPRE+=.\Include
.DIRCACHE=NO
# CFLAGS+=-E

SLOFILES =	$(SLO)$/regpatchactivex.obj

.IF "$(COM)"=="GCC"
SHL1STDLIBS=	-lmingw32 -lstdc++ -lgcc -lmingwthrd -lmsvcrt
.ELSE
SHL1STDLIBS=
.ENDIF

SHL1STDLIBS+=	$(KERNEL32LIB)\
                $(USER32LIB)\
                $(ADVAPI32LIB)\
                $(SHELL32LIB)\
                $(MSILIB)
.IF "$(COM)"!="GCC"
SHL1STDLIBS+=	libcmt.lib
.ENDIF


SHL1LIBS = $(SLB)$/$(TARGET).lib 

#SHL1TARGET = $(TARGET)$(DLLPOSTFIX)
SHL1TARGET = $(TARGET)
SHL1IMPLIB = i$(TARGET)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1DEPN = $(SLB)$/$(TARGET).lib
SHL1BASE = 0x1c000000
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------


.ENDIF

