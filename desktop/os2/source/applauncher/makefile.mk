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

PRJNAME=desktop
TARGET=applauncher
LIBTARGET=NO
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/launcher.obj \
    $(OBJ)$/swriter.obj \
    $(OBJ)$/scalc.obj \
    $(OBJ)$/sdraw.obj \
    $(OBJ)$/simpress.obj \
    $(OBJ)$/sbase.obj \
    $(OBJ)$/smath.obj \
    $(OBJ)$/officeloader.obj \
    $(OBJ)$/quickstart.obj

APP1TARGET=swriter
APP1NOSAL=TRUE
APP1LINKRES=$(MISC)$/$(TARGET)1.res
APP1ICON=$(SOLARRESDIR)$/icons$/ooo-writer-app.ico
APP1OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/swriter.obj


APP2TARGET=scalc
APP2NOSAL=TRUE
APP2LINKRES=$(MISC)$/$(TARGET)2.res
APP2ICON=$(SOLARRESDIR)$/icons$/ooo-calc-app.ico
APP2OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/scalc.obj

APP3TARGET=sdraw
APP3NOSAL=TRUE
APP3LINKRES=$(MISC)$/$(TARGET)3.res
APP3ICON=$(SOLARRESDIR)$/icons$/ooo-draw-app.ico
APP3OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sdraw.obj

APP4TARGET=simpress
APP4NOSAL=TRUE
APP4LINKRES=$(MISC)$/$(TARGET)4.res
APP4ICON=$(SOLARRESDIR)$/icons$/ooo-impress-app.ico
APP4OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/simpress.obj

APP5TARGET=sbase
APP5NOSAL=TRUE
APP5LINKRES=$(MISC)$/$(TARGET)5.res
APP5ICON=$(SOLARRESDIR)$/icons$/ooo-base-app.ico
APP5OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sbase.obj

APP6TARGET=smath
APP6NOSAL=TRUE
APP6LINKRES=$(MISC)$/$(TARGET)6.res
APP6ICON=$(SOLARRESDIR)$/icons$/ooo-math-app.ico
APP6OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/smath.obj

APP7TARGET=officeloader
APP7NOSAL=TRUE
APP7LINKRES=$(MISC)$/$(TARGET)7.res
APP7ICON=$(SOLARRESDIR)$/icons$/ooo-main-app.ico
APP7OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/officeloader.obj

APP8TARGET=quickstart
APP8NOSAL=TRUE
APP8LINKRES=$(MISC)$/$(TARGET)8.res
APP8ICON=$(SOLARRESDIR)$/icons$/ooo-main-app.ico
APP8OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/quickstart.obj

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

