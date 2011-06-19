#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 3.
#
#
#     GNU Lesser General Public License Version 3
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
TARGET=thidxmsi

.IF "$(GUI)"=="WNT"

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE

.INCLUDE : settings.mk

CFLAGS+=-D_STLP_USE_STATIC_LIB

# --- Files --------------------------------------------------------

UWINAPILIB=

SLOFILES = \
    $(SLO)$/thesaurus.obj

SHL1STDLIBS=	kernel32.lib\
                user32.lib\
                advapi32.lib\
                shell32.lib\
                msi.lib

SHL1LIBS = $(SLB)$/$(TARGET).lib

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
