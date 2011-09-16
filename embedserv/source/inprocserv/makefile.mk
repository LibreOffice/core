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

PRJ=..$/..
PRJNAME=embedserv
TARGET=inprocserv

use_shl_versions=

# --- Settings ----------------------------------
.INCLUDE : settings.mk

.IF "$(GUI)" == "WNT"

LIBTARGET=NO
USE_DEFFILE=YES
LIBCMT=libcmt.lib

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/dllentry.obj \
    $(SLO)$/advisesink.obj \
    $(SLO)$/inprocembobj.obj

SHL1TARGET=$(TARGET)
.IF "$(COM)"=="GCC"
SHL1STDLIBS += -lstdc++
.IF "$(MINGW_GCCLIB_EH)"=="YES"
SHL1STDLIBS += -lgcc_eh
.ENDIF
SHL1STDLIBS += -lgcc -lmingw32 -lmoldname -lmsvcrt
.ELSE
SHL1STDLIBS=
.ENDIF

SHL1STDLIBS+=\
    $(UUIDLIB)\
    $(OLE32LIB)\
    $(GDI32LIB)\
    $(ADVAPI32LIB)

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(TARGET).def

DEF1NAME= $(TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk

