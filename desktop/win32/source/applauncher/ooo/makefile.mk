#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:52:55 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
APP1NOSAL=TRUE
APP1LINKRES=$(MISC)$/$(TARGET)1.res
APP1ICON=$(SOLARRESDIR)$/icons/ooo-writer-app.ico
APP1OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/swriter.obj
APP1STDLIBS = $(SHELL32LIB)

APP2TARGET=scalc
APP2NOSAL=TRUE
APP2LINKRES=$(MISC)$/$(TARGET)2.res
APP2ICON=$(SOLARRESDIR)$/icons/ooo-calc-app.ico
APP2OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/scalc.obj
APP2STDLIBS = $(SHELL32LIB)

APP3TARGET=sdraw
APP3NOSAL=TRUE
APP3LINKRES=$(MISC)$/$(TARGET)3.res
APP3ICON=$(SOLARRESDIR)$/icons/ooo-draw-app.ico
APP3OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sdraw.obj
APP3STDLIBS = $(SHELL32LIB)

APP4TARGET=simpress
APP4NOSAL=TRUE
APP4LINKRES=$(MISC)$/$(TARGET)4.res
APP4ICON=$(SOLARRESDIR)$/icons/ooo-impress-app.ico
APP4OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/simpress.obj
APP4STDLIBS = $(SHELL32LIB)

APP5TARGET=smath
APP5NOSAL=TRUE
APP5LINKRES=$(MISC)$/$(TARGET)5.res
APP5ICON=$(SOLARRESDIR)$/icons/ooo-math-app.ico
APP5OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/smath.obj
APP5STDLIBS = $(SHELL32LIB)

APP6TARGET=sbase
APP6NOSAL=TRUE
APP6LINKRES=$(MISC)$/$(TARGET)6.res
APP6ICON=$(SOLARRESDIR)$/icons/ooo-base-app.ico
APP6OBJS = \
        $(OBJ)$/launcher.obj\
       $(OBJ)$/sbase.obj
APP6STDLIBS = $(SHELL32LIB)

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

