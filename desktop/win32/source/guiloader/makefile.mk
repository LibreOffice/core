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
TARGET=guiloader
LIBTARGET=NO
TARGETTYPE=GUI
UWINAPILIB=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

APP1TARGET=guiloader
APP1NOSAL=TRUE
APP1ICON=$(SOLARRESDIR)$/icons/ooo-main-app.ico
APP1OBJS=\
    $(OBJ)$/extendloaderenvironment.obj \
    $(OBJ)$/genericloader.obj \
    $(SOLARLIBDIR)$/pathutils-obj.obj
STDLIB1=$(SHLWAPILIB)

APP2TARGET=so$/guiloader
APP2NOSAL=TRUE
APP2ICON=$(SOLARRESDIR)$/icons/so8-main-app.ico
APP2OBJS=\
    $(OBJ)$/extendloaderenvironment.obj \
    $(OBJ)$/genericloader.obj \
    $(SOLARLIBDIR)$/pathutils-obj.obj
STDLIB2=$(SHLWAPILIB)

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

