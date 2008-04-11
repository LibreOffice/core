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
# $Revision: 1.4 $
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
