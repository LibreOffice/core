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

PRJNAME=fpicker
TARGET=fps_gnome.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE=

# ------------------------------------------------------------------

.IF "$(ENABLE_GTK)" != "TRUE"

dummy:
    @echo "Nothing to build. GUIBASE == $(GUIBASE), WITH_WIDGETSET == $(WITH_WIDGETSET)"

.ELSE # we build for GNOME

PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE : pkg_config.mk

# check gtk version
GTK_TWO_FOUR:=$(shell @$(PKG_CONFIG) --exists 'gtk+-2.0 >= 2.4.0' && echo ok)
.IF "$(GTK_TWO_FOUR)" != "ok"

dummy:
    @echo "Cannot build gtk filepicker because" 
    @$(PKGCONFIG) --print-errors --exists 'gtk+-2.0 >= 2.4.0'

.ELSE

CFLAGS+= $(WIDGETSET_CFLAGS)
CFLAGS+= $(PKGCONFIG_CFLAGS)

# --- Files --------------------------------------------------------

SLOFILES =\
        $(SLO)$/SalGtkPicker.obj				\
        $(SLO)$/SalGtkFilePicker.obj			\
        $(SLO)$/SalGtkFolderPicker.obj			\
        $(SLO)$/resourceprovider.obj			\
        $(SLO)$/FPentry.obj

SHL1NOCHECK=TRUE
SHL1TARGET=	$(TARGET)
SHL1OBJS=	$(SLOFILES)
SHL1STDLIBS=\
    $(VCLLIB) \
    $(TOOLSLIB) \
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(PKGCONFIG_LIBS)

.IF "$(OS)"=="SOLARIS"
LINKFLAGSDEFS=
.ENDIF # "$(OS)"=="SOLARIS"

SHL1VERSIONMAP=exports.map
DEF1NAME=$(SHL1TARGET)

.ENDIF # GTK_TWO_FOUR
.ENDIF # "$(GUIBASE)" != "unx" || "$(WITH_WIDGETSET)" != "gnome"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
