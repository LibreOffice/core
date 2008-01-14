#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: ihi $ $Date: 2008-01-14 15:57:15 $
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

PRJNAME=shell
TARGET=macbe

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

COMP1TYPELIST=$(TARGET)

# --- Settings ---

.INCLUDE : settings.mk

DLLPRE =

# --- Files ---

.IF "$(OS)" != "MACOSX"

dummy:
        @echo "Nothing to build for OS $(OS)."

.ELSE

OBJCXXFLAGS=-x objective-c++ -fobjc-exceptions
CFLAGSCXX+=$(OBJCXXFLAGS)

SLOFILES= \
    $(SLO)$/macbecdef.obj \
    $(SLO)$/macbackend.obj \
    $(SLO)$/macbelayer.obj

SHL1TARGET=$(TARGET)1.uno
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS= \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
    -framework Cocoa -framework SystemConfiguration

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

# --- Targets ---

.INCLUDE : target.mk

.ENDIF # "$(OS)" != "MACOSX"
