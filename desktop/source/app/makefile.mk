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

PRJ=..$/..

PRJNAME=desktop
TARGET=dkt
AUTOSEG=true
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : ../deployment/inc/dp_misc.mk

.IF "$(ENABLE_GNOMEVFS)"=="TRUE"
CFLAGS+=-DGNOME_VFS_ENABLED
.ENDIF

# .IF "$(OS)" == "WNT"
# .IF "$(COM)" == "GCC"
# DEPLOYMENTMISCLIB = -ldeploymentmisc$(DLLPOSTFIX)
# .ELSE
# DEPLOYMENTMISCLIB = ideploymentmisc$(DLLPOSTFIX).lib
# .ENDIF
# .ELIF "$(OS)" == "OS2"
# DEPLOYMENTMISCLIB = ideploymentmisc$(DLLPOSTFIX).lib
# .ELSE
# DEPLOYMENTMISCLIB = -ldeploymentmisc$(DLLPOSTFIX)
# .ENDIF

.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2" || "$(GUIBASE)"=="aqua" || "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
CFLAGS+=-DENABLE_QUICKSTART_APPLET
.ENDIF

SHL1TARGET = sofficeapp
SHL1OBJS = \
    $(SLO)$/app.obj \
    $(SLO)$/appfirststart.obj \
    $(SLO)$/appinit.obj \
    $(SLO)$/appsys.obj \
    $(SLO)$/checkinstall.obj \
    $(SLO)$/check_ext_deps.obj \
    $(SLO)$/cmdlineargs.obj \
    $(SLO)$/cmdlinehelp.obj \
    $(SLO)$/configinit.obj \
    $(SLO)$/desktopcontext.obj \
    $(SLO)$/desktopresid.obj \
    $(SLO)$/dispatchwatcher.obj \
    $(SLO)$/langselect.obj \
    $(SLO)$/lockfile.obj \
    $(SLO)$/lockfile2.obj \
    $(SLO)$/officeipcthread.obj \
    $(SLO)$/sofficemain.obj \
    $(SLO)$/userinstall.obj

SHL1LIBS = $(SLB)$/mig.lib

SHL1STDLIBS = \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(DEPLOYMENTMISCLIB) \
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

SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

OBJFILES = \
    $(OBJ)$/copyright_ascii_ooo.obj \
    $(OBJ)$/main.obj
.IF "$(GUI)" != "OS2"
OBJFILES += \
    $(OBJ)$/copyright_ascii_sun.obj
.ENDIF

SLOFILES = $(SHL1OBJS)

SRS1NAME=	desktop
SRC1FILES=	desktop.src

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

