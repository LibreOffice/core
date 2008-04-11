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
# $Revision: 1.17 $
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
TARGET=gconfbe

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

COMP1TYPELIST=$(TARGET)

# --- Settings ---

.INCLUDE : settings.mk

# no "lib" prefix
DLLPRE =
UCDSRCEXT = txt

.IF "$(ENABLE_LOCKDOWN)" == "YES"
CFLAGS+=-DENABLE_LOCKDOWN
.ENDIF

.IF "$(ENABLE_GNOMEVFS)"!=""
COMPILER_WARN_ALL=TRUE
PKGCONFIG_MODULES=gconf-2.0 gobject-2.0 ORBit-2.0 glib-2.0
.INCLUDE: pkg_config.mk

.IF "$(OS)" == "SOLARIS"
LINKFLAGS+=-z nodefs
.ENDIF          # "$(OS)" == "SOLARIS"

.IF "$(OS)" == "LINUX"
# hack for faked SO environment
CFLAGS+=-gdwarf-2
PKGCONFIG_LIBS!:=-Wl,--export-dynamic $(PKGCONFIG_LIBS)
.ENDIF

# --- Files ---


SLOFILES=\
    $(SLO)$/gconfbecdef.obj \
    $(SLO)$/gconfbackend.obj \
    $(SLO)$/gconflayer.obj
        
SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET)1.uno   
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=    \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)
        
SHL1STDLIBS+=$(PKGCONFIG_LIBS)

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF          # "$(ENABLE_GNOMEVFS)"!=""

# --- Targets ---

.INCLUDE : target.mk

ALLTAR : $(MISC)/$(TARGET)1-ucd.txt

$(MISC)/$(TARGET)1-ucd.txt : $$(@:b).$(UCDSRCEXT)
    cat $< | tr -d "\015" > $@

