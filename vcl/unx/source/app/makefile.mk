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
TARGET=salapp
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="unx"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="unx"

SLOFILES=\
            $(SLO)$/i18n_cb.obj			\
            $(SLO)$/i18n_ic.obj			\
            $(SLO)$/i18n_im.obj			\
            $(SLO)$/i18n_xkb.obj		\
            $(SLO)$/i18n_wrp.obj		\
            $(SLO)$/i18n_status.obj		\
            $(SLO)$/i18n_keysym.obj		\
            $(SLO)$/saldata.obj			\
            $(SLO)$/saltimer.obj		\
            $(SLO)$/saldisp.obj			\
            $(SLO)$/randrwrapper.obj	\
            $(SLO)$/salinst.obj			\
            $(SLO)$/salsys.obj			\
            $(SLO)$/soicon.obj			\
            $(SLO)$/keysymnames.obj		\
            $(SLO)$/wmadaptor.obj

EXCEPTIONSFILES=\
            $(SLO)$/wmadaptor.obj		\
            $(SLO)$/saldata.obj			\
            $(SLO)$/salinst.obj			\
            $(SLO)$/saldisp.obj			\
            $(SLO)$/i18n_status.obj		\
            $(SLO)$/i18n_cb.obj				\
            $(SLO)$/i18n_ic.obj				\
            $(SLO)$/salsys.obj				\
            $(SLO)$/sm.obj


.IF "$(ENABLE_RANDR)" != ""
CDEFS+=-DUSE_RANDR
.IF "$(XRANDR_DLOPEN)" == "FALSE"
CDEFS+=$(XRANDR_CFLAGS)
.ELSE
CDEFS+=-DXRANDR_DLOPEN
.ENDIF
.ENDIF

.IF "$(USE_XINERAMA)" != "NO"
CDEFS+=-DUSE_XINERAMA
.IF "$(USE_XINERAMA_VERSION)" == "Xorg"
CDEFS+=-DUSE_XINERAMA_XORG
.ELIF "$(USE_XINERAMA_VERSION)" == "Xsun"
CDEFS+=-DUSE_XINERAMA_XSUN
.ELSE
# provide sensible default
.IF "$(OS)" != "SOLARIS"
CDEFS+=-DUSE_XINERAMA_XORG
.ELSE
CDEFS+=-DUSE_XINERAMA_XSUN
.ENDIF
.ENDIF
.ENDIF

.ENDIF		# "$(GUIBASE)"!="unx"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk

