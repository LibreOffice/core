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

PRJ=..$/..$/..$/..

PRJNAME=desktop
TARGET=applauncher
LIBTARGET=NO
TARGETTYPE=GUI
UWINAPILIB=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

APP1TARGET=swriter
APP1DEPN=verinfo.rc
APP1VERINFO=verinfo.rc
APP1NOSAL=TRUE
APP1LINKRES=$(MISC)$/$(TARGET)1.res
APP1ICON=$(SOLARRESDIR)$/icons/ooo3_writer_app.ico
APP1OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/swriter.obj
APP1STDLIBS = $(SHELL32LIB)
APP1PRODUCTDEF+=-DRES_APP_NAME=$(APP1TARGET)

APP2TARGET=scalc
APP2DEPN=verinfo.rc
APP2VERINFO=verinfo.rc
APP2NOSAL=TRUE
APP2LINKRES=$(MISC)$/$(TARGET)2.res
APP2ICON=$(SOLARRESDIR)$/icons/ooo3_calc_app.ico
APP2OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/scalc.obj
APP2STDLIBS = $(SHELL32LIB)
APP2PRODUCTDEF+=-DRES_APP_NAME=$(APP2TARGET)

APP3TARGET=sdraw
APP3DEPN=verinfo.rc
APP3VERINFO=verinfo.rc
APP3NOSAL=TRUE
APP3LINKRES=$(MISC)$/$(TARGET)3.res
APP3ICON=$(SOLARRESDIR)$/icons/ooo3_draw_app.ico
APP3OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sdraw.obj
APP3STDLIBS = $(SHELL32LIB)
APP3PRODUCTDEF+=-DRES_APP_NAME=$(APP3TARGET)

APP4TARGET=simpress
APP4DEPN=verinfo.rc
APP4VERINFO=verinfo.rc
APP4NOSAL=TRUE
APP4LINKRES=$(MISC)$/$(TARGET)4.res
APP4ICON=$(SOLARRESDIR)$/icons/ooo3_impress_app.ico
APP4OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/simpress.obj
APP4STDLIBS = $(SHELL32LIB)
APP4PRODUCTDEF+=-DRES_APP_NAME=$(APP4TARGET)

APP5TARGET=smath
APP5DEPN=verinfo.rc
APP5VERINFO=verinfo.rc
APP5NOSAL=TRUE
APP5LINKRES=$(MISC)$/$(TARGET)5.res
APP5ICON=$(SOLARRESDIR)$/icons/ooo3_math_app.ico
APP5OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/smath.obj
APP5STDLIBS = $(SHELL32LIB)
APP5PRODUCTDEF+=-DRES_APP_NAME=$(APP5TARGET)

APP6TARGET=sbase
APP6DEPN=verinfo.rc
APP6VERINFO=verinfo.rc
APP6NOSAL=TRUE
APP6LINKRES=$(MISC)$/$(TARGET)6.res
APP6ICON=$(SOLARRESDIR)$/icons/ooo3_base_app.ico
APP6OBJS = \
        $(OBJ)$/launcher.obj\
       $(OBJ)$/sbase.obj
APP6STDLIBS = $(SHELL32LIB)
APP6PRODUCTDEF+=-DRES_APP_NAME=$(APP6TARGET)

APP7TARGET=sweb
APP7DEPN=verinfo.rc
APP7VERINFO=verinfo.rc
APP7NOSAL=TRUE
APP7LINKRES=$(MISC)$/$(TARGET)7.res
APP7ICON=$(SOLARRESDIR)$/icons/ooo3_writer_app.ico
APP7OBJS = \
        $(OBJ)$/launcher.obj\
       $(OBJ)$/sweb.obj
APP7STDLIBS = $(SHELL32LIB)
APP7PRODUCTDEF+=-DRES_APP_NAME=$(APP7TARGET)

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

