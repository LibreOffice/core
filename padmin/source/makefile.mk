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
# $Revision: 1.24 $
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

.IF "$(GUIBASE)"=="aqua"

dummy:
    @echo "Nothing to build for GUIBASE aqua."

.ELSE

PRJ=..
PRJNAME=padmin
TARGET=padmin
TARGETTYPE=GUI
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES=padialog.src rtsetup.src

RESLIB1NAME=spa
RESLIB1IMAGES=$(PRJ)$/source
RESLIB1SRSFILES= $(SRS)$/padmin.srs
RESLIB1DEPN=\
    padialog.src	\
    padialog.hrc	\
    rtsetup.src		\
    rtsetup.hrc

.IF "$(GUI)" == "UNX"

SLOFILES=\
    $(SLO)$/padialog.obj		\
    $(SLO)$/cmddlg.obj			\
    $(SLO)$/progress.obj		\
    $(SLO)$/newppdlg.obj		\
    $(SLO)$/prtsetup.obj		\
    $(SLO)$/fontentry.obj		\
    $(SLO)$/helper.obj			\
    $(SLO)$/adddlg.obj			\
    $(SLO)$/titlectrl.obj


OBJFILES=\
    $(OBJ)/pamain.obj

SHL1TARGET= spa$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(SVTOOLLIB)					\
    $(VCLLIB)						\
    -lpsp$(DLLPOSTFIX)		\
    $(UNOTOOLSLIB)					\
    $(TOOLSLIB)						\
    $(COMPHELPERLIB)				\
    $(CPPULIB)						\
    $(SALLIB)

APP1TARGET=spadmin.bin
APP1DEPN+=$(SHL1TARGETN)

APP1OBJS=\
    $(OBJ)$/desktopcontext.obj  \
    $(OBJ)/pamain.obj

APP1STDLIBS=	\
    -l$(SHL1TARGET)		\
    $(SVTOOLLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(UCBHELPERLIB)		\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)			\
    -lXext -lX11

UNIXTEXT = $(MISC)$/spadmin.sh

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # GUIBASE==aqua

