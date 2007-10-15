#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: vg $ $Date: 2007-10-15 11:57:04 $
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

PRJ=..$/..$/..

PRJNAME=bridges
TARGET=gcc3_uno
LIBTARGET=no
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(COM)$(OS)$(CPU)$(COMNAME)" == "GCCLINUX3gcc3"

.IF "$(cppu_no_leak)" == ""
CFLAGS += -DLEAK_STATIC_DATA
.ENDIF

CFLAGSNOOPT=-O0

SLOFILES= \
    $(SLO)$/except.obj		\
    $(SLO)$/cpp2uno.obj		\
    $(SLO)$/uno2cpp.obj

SHL1TARGET= $(TARGET)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=..$/..$/bridge_exports.map
SHL1RPATH=URELIB

SHL1OBJS= \
    $(SLO)$/except.obj		\
    $(SLO)$/cpp2uno.obj		\
    $(SLO)$/uno2cpp.obj

SHL1STDLIBS= \
    $(CPPULIB)			\
    $(SALLIB)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

