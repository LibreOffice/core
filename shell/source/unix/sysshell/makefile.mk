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
# $Revision: 1.11 $
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

PRJNAME=shell
TARGET=sysshell
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(OS)" != "MACOSX"

LIB1OBJFILES=$(SLO)$/systemshell.obj
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1ARCHIV=$(SLB)$/lib$(TARGET).a

SLOFILES=$(SLO)$/recently_used_file.obj \
         $(SLO)$/recently_used_file_handler.obj

SHL1TARGET=recentfile
.IF "$(GUI)" == "OS2"
SHL1IMPLIB=i$(TARGET)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
DEF1NAME=       $(SHL1TARGET)

.ELSE
#SHL1IMPLIB=
.ENDIF

SHL1STDLIBS=$(EXPATASCII3RDLIB)\
    $(SALLIB)\
    $(CPPULIB)\
    $(CPPUHELPERLIB)\
    $(COMPHELPERLIB)

SHL1LIBS=$(SLB)$/xmlparser.lib
SHL1OBJS=$(SLOFILES)
SHL1VERSIONMAP=recfile.map

.ELSE

dummy:
    @echo "Nothing to build for MACOSX"

.ENDIF # MACOSX
# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
