#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:52:42 $
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

PRJ=..$/..$/..

PRJNAME=desktop
TARGET=applauncher
LIBTARGET=NO
TARGETTYPE=GUI
UWINAPILIB=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CDEFS+=-DUNICODE


OBJFILES= \
    $(OBJ)$/launcher.obj \
    $(OBJ)$/swriter.obj \
    $(OBJ)$/scalc.obj \
    $(OBJ)$/sdraw.obj \
    $(OBJ)$/simpress.obj \
    $(OBJ)$/sbase.obj \
    $(OBJ)$/smath.obj

# SO launcher
.IF "$(BUILD_SPECIAL)"!=""
APP1TARGET=so$/swriter
APP1NOSAL=TRUE
APP1LINKRES=$(MISC)$/$(TARGET)1.res
APP1ICON=$(SOLARRESDIR)$/icons/so8-writer-app.ico
APP1OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/swriter.obj
APP1STDLIBS = $(SHELL32LIB)

APP2TARGET=so$/scalc
APP2NOSAL=TRUE
APP2LINKRES=$(MISC)$/$(TARGET)2.res
APP2ICON=$(SOLARRESDIR)$/icons/so8-calc-app.ico
APP2OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/scalc.obj
APP2STDLIBS = $(SHELL32LIB)

APP3TARGET=so$/sdraw
APP3NOSAL=TRUE
APP3LINKRES=$(MISC)$/$(TARGET)3.res
APP3ICON=$(SOLARRESDIR)$/icons/so8-draw-app.ico
APP3OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sdraw.obj
APP3STDLIBS = $(SHELL32LIB)

APP4TARGET=so$/simpress
APP4NOSAL=TRUE
APP4LINKRES=$(MISC)$/$(TARGET)4.res
APP4ICON=$(SOLARRESDIR)$/icons/so8-impress-app.ico
APP4OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/simpress.obj
APP4STDLIBS = $(SHELL32LIB)

APP5TARGET=so$/sbase
APP5NOSAL=TRUE
APP5LINKRES=$(MISC)$/$(TARGET)5.res
APP5ICON=$(SOLARRESDIR)$/icons/so8-base-app.ico
APP5OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sbase.obj
APP5STDLIBS = $(SHELL32LIB)

APP6TARGET=so$/smath
APP6NOSAL=TRUE
APP6LINKRES=$(MISC)$/$(TARGET)6.res
APP6ICON=$(SOLARRESDIR)$/icons/so8-math-app.ico
APP6OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/smath.obj
APP6STDLIBS = $(SHELL32LIB)

.ENDIF			# "$(BUILD_SPECIAL)"!=""

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

