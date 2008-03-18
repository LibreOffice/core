#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:49:13 $
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

PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = unopkg
TARGETTYPE = GUI
ENABLE_EXCEPTIONS = TRUE
LIBTARGET=NO

PRJINC += ..$/..$/deployment ..$/..
.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

APP1TARGET = so$/unopkg
APP1OBJS = $(OBJFILES)
APP1STDLIBS = $(SALLIB) $(UNOPKGLIB)
APP1DEPN = $(SHL1TARGETN)
APP1NOSAL = TRUE
APP1RPATH = BRAND
.IF "$(OS)" == "WNT"
APP1ICON = $(SOLARRESDIR)$/icons/so8-main-app.ico
APP1LINKRES = $(MISC)$/$(TARGET)1.res
.ENDIF

APP2TARGET = unopkg
APP2OBJS = $(OBJFILES)
APP2STDLIBS = $(SALLIB) $(UNOPKGLIB)
APP2DEPN = $(SHL1TARGETN)
APP2NOSAL = TRUE
APP2RPATH = BRAND
.IF "$(OS)" == "WNT"
APP2ICON = $(SOLARRESDIR)$/icons/ooo-main-app.ico
APP2LINKRES = $(MISC)$/$(TARGET)2.res
.ENDIF

SHL1TARGET = unopkg
SHL1OBJS = $(SLOFILES) $(SLO)$/lockfile.obj
SHL1STDLIBS = \
    $(SALLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(VCLLIB) \
    $(DEPLOYMENTMISCLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = \
    $(SLO)$/unopkg_app.obj \
    $(SLO)$/unopkg_cmdenv.obj \
    $(SLO)$/unopkg_misc.obj

OBJFILES = $(OBJ)$/unopkg_main.obj

.INCLUDE : target.mk

.IF "$(APP1TARGETN)" != "" # not set during depend=x
$(APP1TARGETN) : $(MISC)$/binso_created.flg
.ENDIF			# "$(APP1TARGETN)"!=""

$(MISC)$/binso_created.flg:
    @@-$(MKDIRHIER) $(BIN)$/so && $(TOUCH) $@
