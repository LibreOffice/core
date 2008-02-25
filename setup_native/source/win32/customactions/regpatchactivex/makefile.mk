#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:25:18 $
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

