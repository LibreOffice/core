#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
TARGET=gtka11y
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="unx"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="unx"

.IF "$(ENABLE_GTK)" != ""

PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE : pkg_config.mk

CFLAGS+=-DVERSION=$(EMQ)"$(UPD)$(LAST_MINOR)$(EMQ)"

ATKVERSION:=$(shell @$(PKG_CONFIG) --modversion atk | $(AWK) -v num=true -f $(SOLARENV)$/bin$/getcompver.awk)

.IF "$(ATKVERSION)" >= "000100070000"
CFLAGS+=-DHAS_ATKRECTANGLE
.ENDIF

SLOFILES=\
    $(SLO)$/atkaction.obj \
    $(SLO)$/atkbridge.obj \
    $(SLO)$/atkcomponent.obj \
    $(SLO)$/atkeditabletext.obj \
    $(SLO)$/atkfactory.obj \
    $(SLO)$/atkhypertext.obj \
    $(SLO)$/atkimage.obj \
    $(SLO)$/atklistener.obj \
    $(SLO)$/atkregistry.obj \
    $(SLO)$/atkselection.obj \
    $(SLO)$/atktable.obj \
    $(SLO)$/atktext.obj \
    $(SLO)$/atktextattributes.obj \
    $(SLO)$/atkutil.obj \
    $(SLO)$/atkvalue.obj \
    $(SLO)$/atkwindow.obj \
    $(SLO)$/atkwrapper.obj

.ELSE # "$(ENABLE_GTK)" != ""

dummy:
    @echo GTK disabled - nothing to build
.ENDIF
.ENDIF		# "$(GUIBASE)"!="unx"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

