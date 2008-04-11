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
# $Revision: 1.5 $
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


PRJ=..
PRJNAME=libegg
TARGET=eggtray
TARGETTYPE=GUI

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE
C_RESTRICTIONFLAGS*=-xc99=no_lib

# --- Settings ----------------------------------

.INCLUDE : settings.mk

.IF "$(ENABLE_GTK)"!=""

PKGCONFIG_MODULES=gtk+-2.0 gdk-2.0
.INCLUDE: pkg_config.mk
CFLAGS+=$(PKGCONFIG_CFLAGS)

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/eggtrayicon.obj

SHL1STDLIBS+=$(PKGCONFIG_LIBS)
.IF "$(PKGCONFIG_ROOT)"!=""
SHL1SONAME+=-z nodefs
SHL1NOCHECK=TRUE
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""


SHL1TARGET= 	eggtray$(DLLPOSTFIX)
SHL1LIBS=       $(SLB)$/eggtray.lib

.ENDIF #	"$(ENABLE_GTK)"!=""

# --- Targets ----------------------------------

.INCLUDE : target.mk
