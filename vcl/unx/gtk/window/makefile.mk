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
# $Revision: 1.12 $
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

PRJNAME=vcl
TARGET=gtkwin
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# workaround for makedepend hang
MKDEPENDSOLVER=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="unx"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="unx"

.IF "$(ENABLE_GTK)" != ""

PKGCONFIG_MODULES=gtk+-2.0
.IF "$(ENABLE_DBUS)" != ""
CDEFS+=-DENABLE_DBUS
PKGCONFIG_MODULES+= dbus-glib-1
.ENDIF
.INCLUDE : pkg_config.mk

.IF "$(COM)" == "C52"
NOOPTFILES=$(SLO)$/gtkframe.obj
.ENDIF

SLOFILES=\
            $(SLO)$/gtkframe.obj				\
            $(SLO)$/gtkobject.obj
EXCEPTIONSFILES=$(SLO)$/gtkframe.obj
.ELSE # "$(ENABLE_GTK)" != ""

dummy:
    @echo GTK disabled - nothing to build
.ENDIF
.ENDIF		# "$(GUIBASE)"!="unx"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk
