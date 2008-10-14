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
# $Revision: 1.42.40.1 $
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

PRJ=..$/..

PRJNAME=desktop
TARGET=dkt
AUTOSEG=true
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

SHL1TARGET = sofficeapp
SHL1OBJS = \
    $(SLO)$/app.obj \
    $(SLO)$/appfirststart.obj \
    $(SLO)$/appinit.obj \
    $(SLO)$/appsys.obj \
    $(SLO)$/cfgfilter.obj \
    $(SLO)$/checkinstall.obj \
    $(SLO)$/cmdlineargs.obj \
    $(SLO)$/cmdlinehelp.obj \
    $(SLO)$/configinit.obj \
    $(SLO)$/desktopcontext.obj \
    $(SLO)$/desktopresid.obj \
    $(SLO)$/dispatchwatcher.obj \
    $(SLO)$/langselect.obj \
    $(SLO)$/lockfile.obj \
    $(SLO)$/lockfile2.obj \
    $(SLO)$/migration.obj \
    $(SLO)$/officeipcthread.obj \
    $(SLO)$/pages.obj \
    $(SLO)$/sofficemain.obj \
    $(SLO)$/userinstall.obj \
    $(SLO)$/wizard.obj
SHL1STDLIBS = \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(I18NISOLANGLIB) \
    $(SALLIB) \
    $(SFXLIB) \
    $(SVLLIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(VCLLIB) \
    $(VOSLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

OBJFILES = \
    $(OBJ)$/copyright_ascii_ooo.obj \
    $(OBJ)$/copyright_ascii_sun.obj \
    $(OBJ)$/main.obj

SLOFILES = $(SHL1OBJS)

SRS1NAME=	desktop
SRC1FILES=	desktop.src

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

