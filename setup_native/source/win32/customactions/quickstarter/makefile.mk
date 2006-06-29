#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: ihi $ $Date: 2006-06-29 11:17:20 $
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
TARGET=quickstarter
TARGET1=sdqsmsi
TARGET2=qslnkmsi

# --- Settings -----------------------------------------------------

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE

.INCLUDE : settings.mk

CFLAGS+=-D_STLP_USE_STATIC_LIB
UWINAPILIB=

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="WNT"

STDSHL += \
    advapi32.lib\
    shell32.lib\
    msi.lib\
    $(LIBSTLPORTST)								


SHL1OBJS =	$(SLO)$/shutdown_quickstart.obj \
            $(SLO)$/quickstarter.obj

SHL1TARGET = $(TARGET1)
SHL1IMPLIB = i$(TARGET1)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1BASE = 0x1c000000
DEF1NAME=$(SHL1TARGET)
SHL1DEPN=$(SHL1OBJS)
DEF1EXPORTFILE=$(TARGET1).dxp

# --- Files --------------------------------------------------------

SHL2OBJS =	$(SLO)$/remove_quickstart_link.obj \
            $(SLO)$/quickstarter.obj

SHL2TARGET = $(TARGET2)
SHL2IMPLIB = i$(TARGET2)

SHL2DEF = $(MISC)$/$(SHL2TARGET).def
SHL2BASE = 0x1c000000
DEF2NAME=$(SHL2TARGET)
SHL2DEPN=$(SHL1OBJS)
DEF2EXPORTFILE=$(TARGET2).dxp

.ENDIF

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------

