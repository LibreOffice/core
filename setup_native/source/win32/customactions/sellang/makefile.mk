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
TARGET=sellangmsi

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE

.INCLUDE : settings.mk

.IF "$(GUI)"=="WNT" && "$(WINDOWS_SDK_HOME)"!=""

CFLAGS+=-D_STLP_USE_STATIC_LIB

# --- Files --------------------------------------------------------

UWINAPILIB=

SLOFILES = \
    $(SLO)$/sellang.obj

SHL1STDLIBS= \
    $(KERNEL32LIB)\
    $(USER32LIB)\
    $(ADVAPI32LIB)\
    $(SHELL32LIB)\
    $(MSILIB)

SHL1LIBS = $(SLB)$/$(TARGET).lib

SHL1TARGET = $(TARGET)
SHL1IMPLIB = i$(TARGET)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1DEPN = $(SLB)$/$(TARGET).lib
SHL1BASE = 0x1c000000
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------
